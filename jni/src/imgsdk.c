/***************************************************************
 * file name:   imgsdk.c
 * description: a simple toolkit for processing image use GPU
 * author:      kari.zhang
 * create date: 2015-05-09
 *
 * modification 
 *	1: Revivew code at 2015-11-23 21:12 by kari.zhang. 
 *		add const qualifier for input parameters & add comments
 ***************************************************************/

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include "chrbuf.h"
#include "comm.h"
#include "eftcmd.h"
#include "imgsdk.h"
#include "jpeglib.h"
#include "png.h"
#include "utility.h"

// user cmd default capability
#define USER_CMD_CAPABILITY 1024

/**
 * Indicate whether  userData.param or userData.inputPath is active
 */
typedef enum ActiveType {
    ACTIVE_NONE  = 0,		// NO active
    ACTIVE_PARAM = 1,		// param is active
    ACTIVE_PATH = 2,		// input path is active
} ActiveType;

typedef enum ImageType {
	IMAGE_UNKNOWN = 0,	// unknown image type
	IMAGE_PNG,			// png format
	IMAGE_JPG			// jpg format
} ImageType;

/**
 * User's image information
 */
typedef struct {
    GLuint			width;			// user's image width
    GLuint			height;			// user's image height
    GLchar			*param;			// user's parameter
    GLchar			*inputPath;		// input image path
	ImageType		inputImageType;	// input image type
    GLchar			*outputPath;	// output image path
    ActiveType		active;			// which is active ( param or pixel) 
    PixForm_e		pixfmt;			// pixel format
    PlatformType	platform;		// 0 Android, 1 iOS
    const void		*platformData;	// AssetManager* in Android
    char            *vertSource;    // vertex shader source
    int             nVertSource;    // length of vertSource
    char            *fragSource;    // fragment shader source
    int             nFragSource;    // length of fragSource
} UserData;

/**
 * Shader variable handle
 * You may update me When you process image
 */
typedef struct CommHandle {
    GLuint program;				// program handler
    GLuint vertShader;			// vertex shader handle
    GLuint fragShader;			// fragment shader handle
    GLint positionIdx;		    // attribute vec3 aPosition
    GLuint texture1Idx;         // texture handle
    GLuint texture2Idx;         // texture handle
    GLuint sampler2dIdx;        // sampler handler
    GLuint texCoordIdx;         // texture coordinate
    GLuint colorIdx;            // attribute color
    GLuint fboIdx;				// framebuffer id
} CommHandle;

/**
 * EGL environment
 */
typedef struct eglEnv {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLint     width;			// used for glViewport
    EGLint     height;			// used for glViewport
    EGLNativeWindowType window;	// ignore in off-screen render)
} eglEnv;

/**
 * ImageSDK callback function
 */
typedef void (*CallbackFunc)(SdkEnv *);

/**
 * Indicate sdk status
 */
enum {
    SDK_STATUS_ERR  = 0,
    SDK_STATUS_OK   = 1,
};

/**
 * Indicate sdk type
 */
enum {
    ON_SCREEN_RENDER  = 0,
    OFF_SCREEN_RENDER = 1
}; 

typedef struct SdkEnv {
    // sdk type
    int type;

    // sdk status
    int status;

    // native Activity
    struct android_app* app; 

    // EGL
    eglEnv egl;

    // openGL ES2 shader variable handle 
    CommHandle handle;

    // user data
    UserData userData;

    // user cmd
    chrbuf_t *userCmd;

    // effect cmd
    eftcmd_t effectCmd;

    // callback	
    CallbackFunc onCreate;
    CallbackFunc onDraw;
    CallbackFunc onDestroy;
} SdkEnv;

static void onCreate(SdkEnv *env);
static void onDraw(SdkEnv *env);
static void onRender(SdkEnv *env);
static void onDestroy(SdkEnv *env);

int sdkMain(SdkEnv *env)
{
    VALIDATE_NOT_NULL(env);

    env->onCreate  = onCreate;
    //env->onDraw	   = onDraw;
    env->onDraw	   = onRender;
    env->onDestroy = onDestroy;

    onSdkCreate(env);
}

/*
 * Console application entry
 * Usage:
 *		imgsdk input output
 * Notice:
 *	1. input & onput support *.jpg or *.png
 *	2. support input and output image type are not same 
 *	3. vert.shdr & frag.shdr must be prepared 
 */
int main(int argc, char **argv) {
    if (3 != argc) {
        Log("Usage:\n");
        Log("  %s input output\n", argv[0]);
        return -1;
    }

    SdkEnv *env = newDefaultSdkEnv();
    if (NULL == env) {
        LogE("Failed get SdkEnv instance\n");
    }
    setInputImagePath (env, argv[1]);
    setOutputImagePath (env, argv[2]);

    sdkMain (env);
    setEffectCmd (env, "{\"effect\":\"Normal\"}");
    onSdkDraw (env);

    Bitmap_t *img = (Bitmap_t *) env->userData.param;
    uint32_t begin_t = getCurrentTime();

    glBindTexture(GL_TEXTURE, env->handle.texture2Idx);
    memset (img->base, 0, img->width * img->height * img->form);

    // copy pixels from GPU memory to CPU memory
    int x = 0;
    int y = 0;
	GLint fmt = GL_RGBA;
	if (IMAGE_JPG == env->userData.inputImageType) {
		fmt = GL_RGB;
	}
    glReadPixels(x, y, img->width, img->height, fmt, GL_UNSIGNED_BYTE, img->base);
    int errCode = glGetError ();
    if (GL_NO_ERROR != errCode ) { 
        Log ("Failed read pixles, error code:0x%04x\n", errCode);
    }
    uint32_t finish_t = getCurrentTime();
    LogD("Read pixel data cost %d ms\n", (finish_t - begin_t));

    begin_t = getCurrentTime();
    if (NULL != env->userData.outputPath) {
        if (saveImage (env->userData.outputPath, img) < 0) {
            LogE ("Failed saveImage\n");
        }
		else { 
            finish_t = getCurrentTime();
            LogD("Save %s cost %d ms\n", 
                    env->userData.outputPath,
                    (finish_t - begin_t));
        }
    }

    freeSdkEnv(env);
    //onSdkDestroy(env);
}

/*
 * Read png file and store in memory
 * Return:
 *		 0		  OK
 *      negative  ERROR
 */
int read_png(const char *path, Bitmap_t *mem)
{
    VALIDATE_NOT_NULL2(path, mem);
    FILE *fp = fopen(path, "rb");
    if (NULL == fp) {
        return FILE_NOT_EXIST;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
            0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr,info_ptr);
    int type = png_get_color_type(png_ptr, info_ptr);
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    int bpp = 0;
    switch (type) { 
        case PNG_COLOR_TYPE_GRAY:
            bpp = 1;
            break;

        case PNG_COLOR_TYPE_RGB:
            bpp = 3;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            bpp = 4;
            break;
    }

    if (bpp == 0) {
        LogE("Failed get pixel format");
    }

    mem->form = bpp;
    mem->width = width;
    mem->height = height;

    Log("[%s %d x %d bpp=%d]\n", path, width, height, bpp);

    int size = width * height * bpp;
    if (mem->base == NULL) {
        mem->base = (char *)calloc(size, 1);
        if (mem->base == NULL) {
            LogE("Failed calloc mem\n");
        }
    }

    char *start = mem->base;
    int i, j;
    for (i = 0; i < height; ++i) {
        memcpy(start, row_pointers[i], width * bpp);
        start += width * bpp;
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(fp);
    return size;
}

/**
 * Write png to file
 * Return:
 *		0		 OK
 *		negative ERROR
 */
int write_png(const char *path, const Bitmap_t *mem)
{
    VALIDATE_NOT_NULL2(path, mem);
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;

    fp = fopen(path, "wb");
    if (NULL == fp) {
        LogE("Failed open file\n");
        return -1;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (NULL == png_ptr) {
        fclose (fp);
        return -1;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (NULL == info_ptr) {
        fclose (fp);
        png_destroy_write_struct(&png_ptr, NULL);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        LogE("write file occur error");
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return -1;
    }

    int form = PNG_COLOR_TYPE_RGB_ALPHA;
    int depth = 8;
    switch (mem->form) {
        case RGBA32:
            form = PNG_COLOR_TYPE_RGBA;
            break;

        case RGB24:
            form = PNG_COLOR_TYPE_RGB;
            break;

        case GRAY:
            form = PNG_COLOR_TYPE_GRAY;
            depth = 1;
            break;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, mem->width, mem->height, depth, form, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    png_bytep row_pointers[mem->height];
    int k;
    for (k = 0; k < mem->height; ++k) {
        row_pointers[k] = mem->base + k * mem->width * mem->form;
    }
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose (fp);

    return 0;
}

/**
 *	Read the text file to memory
 *  Parameters:
 *			path:	file path
 *			mem:	[OUT] store file content
 *	Return:
 *			if OK return file's length
 *			if ERROR return -1
 *	Notice:
 *		space of mem is 1 more byte than file length
 */
int readFile(const char* path, char** mem)
{
    VALIDATE_NOT_NULL2(path, mem);
    FILE *fp = fopen(path, "r");
    if (NULL == fp) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (NULL == *mem) {
        (*mem) = (char *)calloc(1, size + 1);
        if (NULL == *mem) {
            LogE("Failed calloc mem for shader file\n");
            close (fp);
            return -1;
        }
    }

    if (fread(*mem, 1, size, fp) != size) {
        LogE("Failed read shader to mem\n");
        close(fp);
        free (*mem);
        *mem = NULL;
        return -1;
    }

    return size;
}

// Used for glsl log
#define LOG_BUFF_SIZE 1024
static char sLogBuff[LOG_BUFF_SIZE];

/**
 * Create a glsh shader
 * Parameters:
 *		type:	Just support GL_VERTEX_SHADER and GL_FRAGMENT_SHADER
 *		source:	shader source code
 * Return:
 *		if failed, return 0
 *		if success, return the handle of shader
 */
static int loadShader(GLenum type, const char* source) {
    VALIDATE_NOT_NULL(source);
    GLuint shader = glCreateShader(type);
    if (!shader) {
        LogE("Invalid shader\n");
        return 0;
    }
    GLint length = strlen(source);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);
    int compiled, len;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        memset(sLogBuff, 0, LOG_BUFF_SIZE);
        glGetShaderInfoLog(shader, LOG_BUFF_SIZE, &len, sLogBuff);
        if (len > 0) {
            Log("compile error:%s\n", sLogBuff);
        } else {
            LogE("Failed get compile log\n");
        }
        shader = 0;
        glDeleteShader(shader);
    }

    return shader;
}

/**
 * Create program and store shader handle in SdkEnv
 * Return:
 *		0  OK
 *     -1 ERROR
 */
static int createProgram(SdkEnv *env, const char* vertexSource, const char* fragSource) {
    VALIDATE_NOT_NULL3(env, vertexSource, fragSource);

    // Make sure to reset them
    env->handle.program		= 0;
    env->handle.vertShader	= 0;
    env->handle.fragShader	= 0;

    int program = 0;
    int vertShader = 0;
    int fragShader = 0;

    do {
        vertShader = loadShader(GL_VERTEX_SHADER, vertexSource);
        if (!vertShader) {
            LogE("Failed load vertex shader\n");
            break;
        }

        fragShader = loadShader(GL_FRAGMENT_SHADER, fragSource);
        if (!fragShader) {
            LogE("Failed load fragment sahder\n");
            break;
        }

        // Mark to delete, fre automaticlly when do not use any longer
        //glDeleteShader(vertShader);
        //glDeleteShader(fragShader);

        program = glCreateProgram();
        if (!program) {
            LogE("Failed create program\n");
            break;
        }

        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        GLint len;
        if (!linkStatus) {
            memset(sLogBuff, 0, LOG_BUFF_SIZE);
            glGetProgramInfoLog(program, LOG_BUFF_SIZE, &len, sLogBuff); 
            if (len > 0) {
                Log("link error log:%s\n", sLogBuff);
            } else {
                LogE("Failed get link log\n");
            }
            break;
        }

        glValidateProgram (program);
        GLint success;
        glGetProgramiv (program, GL_VALIDATE_STATUS, &success);
        if (!success) {
            memset (sLogBuff, 0, LOG_BUFF_SIZE);
            glGetProgramInfoLog (program, LOG_BUFF_SIZE, &len, sLogBuff);
            if (len > 0) {
                Log("program is invalidate:%s\n", sLogBuff); 
            } 
			else {
                LogE("Failed get program status\n");
            }
            break;
        }

        env->handle.program		= program;
        env->handle.vertShader	= vertShader;
        env->handle.fragShader	= fragShader;

        return 0;

    } while (0);

    if (program) {
        glDeleteProgram(program);
    }

    if (vertShader) {
        glDeleteShader(vertShader);
    }

    if (fragShader) {
        glDeleteShader(fragShader);
    }

    return -1;
}

void freeBitmap(Bitmap_t *mem)
{
    if (NULL != mem) {
        if (NULL != mem->base) {
            free (mem->base);
            mem->base = NULL;
        }
    }
}

/**
 * Initialize the default EGL
 * Create a Pbuffer Surface for off-screen render
 */
static int initDefaultEGL(SdkEnv *env) {
    VALIDATE_NOT_NULL(env);
	env->egl.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (EGL_NO_DISPLAY == env->egl.display ||
			EGL_SUCCESS != eglGetError()) {
		LogE("Failed eglGetDisplay\n");
		return -1;
	}

    EGLint major, minor;
    if (!eglInitialize(env->egl.display, &major, &minor) || EGL_SUCCESS != eglGetError()) {
        LogE("Failed eglInitialize\n");
        return -1;
    }
    Log("EGL %d.%d\n", major, minor);

    EGLConfig configs[2];
    EGLint numConfigs;
    if (eglGetConfigs(env->egl.display, configs, 2, &numConfigs) == EGL_FALSE || EGL_SUCCESS != eglGetError()) {
        LogE("Failed eglGetConfigs\n");
        return -1;
    }

    EGLint cfg_attrs[] = { 
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_NONE
    };
    if (!eglChooseConfig(env->egl.display, cfg_attrs, configs, 2, &numConfigs) || numConfigs < 1) {
        LogE("Failed eglChooseConfig\n");
        return -1;
    }

#define SURFACE_MAX_WIDTH  2048
#define SURFACE_MAX_HEIGHT 2048
    EGLint surface_attrs[] = {
        EGL_WIDTH,				SURFACE_MAX_WIDTH,
        EGL_HEIGHT,				SURFACE_MAX_HEIGHT,

        /*
           EGL_TEXTURE_FORMAT,		EGL_TEXTURE_RGAB,
           EGL_TEXTURE_TARGET,		EGL_TEXTURE_2D,
         */

        EGL_LARGEST_PBUFFER,	EGL_TRUE,
        EGL_NONE
    };
    env->egl.surface = eglCreatePbufferSurface(env->egl.display, configs[0], surface_attrs);

    if (EGL_NO_SURFACE == env->egl.surface) {
        LogE("Failed create Pbuffer Surface, error code:%x\n", eglGetError());
        return -1;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        LogE("Failed eglBindAPI\n");
        return -1;
    }

    EGLint context_attrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    env->egl.context = eglCreateContext(env->egl.display, configs[0], EGL_NO_CONTEXT, context_attrs);
    if (EGL_NO_CONTEXT == env->egl.context) {
        LogE("Failed create context, error code:%x\n", eglGetError() );
        return -1;
    }

    if (!eglMakeCurrent(env->egl.display, env->egl.surface, env->egl.surface, env->egl.context)) {
        LogE("Failed eglMakeCurrent, error code:%x\n", eglGetError() );
        return -1;
    }

    eglQuerySurface(env->egl.display, env->egl.surface, EGL_WIDTH, &env->egl.width);
    eglQuerySurface(env->egl.display, env->egl.surface, EGL_HEIGHT, &env->egl.height);
    Log("EGL Pbuffer Surface %d x %d\n", env->egl.width, env->egl.height);

    env->type = OFF_SCREEN_RENDER;

    return 0;
}

/**
 * Release shader & program
 */
static int releaseShader(SdkEnv *env) {
    VALIDATE_NOT_NULL(env);

    glDetachShader(env->handle.program, env->handle.vertShader);
    glDetachShader(env->handle.program, env->handle.fragShader);
    glDeleteShader(env->handle.vertShader);
    glDeleteShader(env->handle.fragShader);
    glDeleteProgram(env->handle.program);
    glReleaseShaderCompiler();
}

static void freeEffectCmd(eftcmd_t *cmd) {
    if (NULL == cmd) {
        return;
    }
    cmd->capacity = 0;
    cmd->valid = false;
    cmd->cmd = ec_NORMAL;
    cmd->count = 0;
    if (NULL != cmd->params) {
        free(cmd->params);
        cmd->params = NULL;
    }
}

/**
 * Free SdkEnv resource
 */
int freeSdkEnv(SdkEnv *env)
{
    if (NULL == env) {
        return -1;
    }

    if (env->egl.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(env->egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (env->egl.context != EGL_NO_CONTEXT) {
            eglDestroyContext(env->egl.display, env->egl.context);
        }
        if (env->egl.surface != EGL_NO_SURFACE) {
            eglDestroySurface(env->egl.display, env->egl.surface);
        }
        eglTerminate(env->egl.display);
        eglReleaseThread();
    }

    env->egl.display = EGL_NO_DISPLAY;
    env->egl.context = EGL_NO_CONTEXT;
    env->egl.surface = EGL_NO_SURFACE;
    if (ACTIVE_PATH == env->userData.active 
            && NULL != env->userData.inputPath) {
        free (env->userData.inputPath);
        env->userData.inputPath = NULL;
    }

    if (ACTIVE_PARAM == env->userData.active 
            && NULL != env->userData.param) {
        Bitmap_t *img = (Bitmap_t *) env->userData.param;
        freeBitmap (img);
        env->userData.param = NULL;
    }

    if (NULL != env->userCmd) {
        freeChrbuf (env->userCmd);
        env->userCmd = NULL;
    }

    if (NULL != env->userData.inputPath) {
        free (env->userData.inputPath);
        env->userData.inputPath = NULL;
    }

    if (NULL != env->userData.outputPath) {
        free (env->userData.outputPath);
        env->userData.outputPath = NULL;
    }

    releaseShader(env);

    if (OFF_SCREEN_RENDER == env->type) {
        glDeleteFramebuffers (1, &env->handle.fboIdx);
        glDeleteTextures (1, &env->handle.texture2Idx);
    }

    freeEffectCmd(&env->effectCmd);

    free (env);
}

/**
 * Find all variables handle in shader
 */
static int findShaderHandles(SdkEnv *env) {
    if (NULL == env) {
        return -1;
    }

    if (0 == env->handle.program) {
        return -1;
    }

    env->handle.positionIdx = glGetAttribLocation(env->handle.program, "aPosition");
    if (env->handle.positionIdx < 0) {
        LogE("Failed get aPosition location\n");
    }

    env->handle.sampler2dIdx = glGetUniformLocation(env->handle.program, "uSampler2D");
    if (env->handle.sampler2dIdx < 0) {
        LogE("Failed get uSampler2D location\n");
    }

    env->handle.texCoordIdx = glGetAttribLocation(env->handle.program, "aTexCoord");
    if (env->handle.texCoordIdx < 0) {
        LogE("Failed get aTexCoord location\n");
    }

    env->handle.colorIdx = glGetAttribLocation(env->handle.program, "aColor");
    if (env->handle.colorIdx < 0) {
        LogE("Failed get aColor location\n");
    }


    return 0;
}

/**
 * Create a default SdkEnv instance
 */
static int attachShader(SdkEnv *env, 
        const char *vertSource, 
        const char *fragSource) {

    VALIDATE_NOT_NULL3(env, vertSource, fragSource);

    // log openGL information
    const GLubyte* version = glGetString(GL_VERSION);
    Log ("Version:%s\n", version);

    const GLubyte* renderer = glGetString(GL_RENDERER);
    Log ("Render:%s\n", renderer);
	
    const GLubyte* vendor = glGetString(GL_VENDOR);
    Log ("Vendor:%s\n", vendor);

	GLboolean shaderCompiler;
	glGetBooleanv(GL_SHADER_COMPILER, &shaderCompiler);
	if (shaderCompiler) {
		Log ("Support shader compiler\n");
	} else {
		Log ("Do not support shader compiler\n");
	}

	GLint max_text;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_text);
	Log ("Max texture size:%d\n", max_text);

    const GLubyte* extension = glGetString (GL_EXTENSIONS);
    Log ("Extensions:\n%s\n\n", extension);

    if(createProgram(env, vertSource, fragSource) < 0) {
        LogE("Failed createProgram\n");
        return -1;
    }

    if (findShaderHandles(env) < 0) {
        LogE("Failed findShaderHandles\n");
        return -1;
    }

    return 0;
}


/**
 * Create a blank SdkEnv instance
 * Parameters:
 *		platform: Just support Android & iOS
 * Return:
 *			if failed return NULL
 *			otherwise return an valid pointer
 */
SdkEnv* newBlankSdkEnv(int platform) 
{
    if (platform <= PLATFORM_OLD || platform >= PLATFORM_NEW) {
        LogE("Invalid platform\n");
        return NULL;
    }

    SdkEnv *env = (SdkEnv *)calloc(1, sizeof(SdkEnv));
    return env;
}

/**
 * Initialize opengl buffers such as framebuffer
 * texture .etc
 */
static int initGlBuffers (SdkEnv *env) {
    VALIDATE_NOT_NULL (env);

    glGenFramebuffers (1, &env->handle.fboIdx);
    glGenTextures(1, &env->handle.texture1Idx);
    glGenTextures(1, &env->handle.texture2Idx);

    glBindTexture (GL_TEXTURE_2D, env->handle.texture1Idx);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return 0;
}

/**
 * Create a default SdkEnv instance
 */
SdkEnv* newDefaultSdkEnv() 
{
    SdkEnv *env = (SdkEnv *)calloc(1, sizeof(SdkEnv));
    if (NULL == env){
        return NULL;
    }
    if (initDefaultEGL(env) < 0) {
        LogE("Failed initDefaultEGL\n");
        freeSdkEnv(env);
        return NULL;
    }

    chrbuf_t *userCmd = newChrbuf (USER_CMD_CAPABILITY);
    if (NULL == userCmd) {
        LogE ("Failed newChrbuf for userCmd\n");
        freeSdkEnv (env);
        return NULL;
    }
    env->userCmd = userCmd;

    char *vertSource = NULL;
    int count = readFile(VERT_SHADER_FILE, &vertSource);
    if (count < 0) {
        LogE("Failed open vertex shader file:%s\n", VERT_SHADER_FILE);
        freeSdkEnv(env);
        return NULL;
    }
    Log("Read %s OK.\n", VERT_SHADER_FILE);
    env->userData.nVertSource = count;
    env->userData.vertSource = vertSource;

    char *fragSource = NULL;
    count = readFile(FRAG_SHADER_FILE, &fragSource);
    if (count < 0) {
        LogE("Failed read fragment shader file:%s\n", FRAG_SHADER_FILE);
        free (vertSource);
        freeSdkEnv(env);
        return NULL;
    }
    env->userData.nFragSource = count;
    env->userData.fragSource = fragSource;
    Log("Read %s OK.\n", FRAG_SHADER_FILE);

    if (attachShader(env, vertSource, fragSource) < 0) {
        LogE("Failed attachShader\n");
        free (vertSource);
        free (fragSource);
        freeSdkEnv(env);
        return NULL;
    }

    if (initGlBuffers (env) < 0) {
        LogE ("Failed init OpenGL buffers.\n");
        return NULL;
    }

    env->status = SDK_STATUS_OK;

    return env;
}

/**
 * Initialize EGL by specified params
 * Create a Window Surface for on-screen render
 */
int initEGL(SdkEnv *env)
{
    LOG_ENTRY;

    VALIDATE_NOT_NULL(env);
    VALIDATE_NOT_NULL(env->egl.window);

    env->egl.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == env->egl.display || EGL_SUCCESS != eglGetError()) {
        LogE("Failed eglGetDisplay\n");
        return -1;
    }
    EGLint major, minor;
    if (!eglInitialize(env->egl.display, &major, &minor) || EGL_SUCCESS != eglGetError()) {
        LogE("Failed eglInitialize\n");
        return -1;
    }
    Log("EGL %d.%d\n", major, minor);

    EGLConfig configs[2];
    EGLint numConfigs;
    if (eglGetConfigs(env->egl.display, configs, 2, &numConfigs) == EGL_FALSE || EGL_SUCCESS != eglGetError()) {
        LogE("Failed eglGetConfigs\n");
        return -1;
    }

    EGLint cfg_attrs[] = { 
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE,       8,
        EGL_GREEN_SIZE,      8,
        EGL_RED_SIZE,        8,
        EGL_NONE
    };
    if (!eglChooseConfig(env->egl.display, cfg_attrs, configs, 2, &numConfigs) || numConfigs < 1) {
        LogE("Failed eglChooseConfig\n");
        return -1;
    }

    EGLint surface_attrs[] = {
        EGL_NONE
    };

    env->egl.surface = eglCreateWindowSurface(env->egl.display, configs[0], env->egl.window, surface_attrs);
    if (EGL_NO_SURFACE == env->egl.surface) {
        LogE("Failed create Window Surface, error code:%x\n", eglGetError());
        return -1;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        LogE("Failed eglBindAPI\n");
        return -1;
    }

    EGLint context_attrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    env->egl.context = eglCreateContext(env->egl.display, configs[0], EGL_NO_CONTEXT, context_attrs);
    if (EGL_NO_CONTEXT == env->egl.context) {
        LogE("Failed create context, error code:%x\n", eglGetError() );
        return -1;
    }

    if (!eglMakeCurrent(env->egl.display, env->egl.surface, env->egl.surface, env->egl.context)) {
        LogE("Failed eglMakeCurrent, error code:%x\n", eglGetError() );
        return -1;
    }

    eglQuerySurface(env->egl.display, env->egl.surface, EGL_WIDTH, &env->egl.width);
    eglQuerySurface(env->egl.display, env->egl.surface, EGL_HEIGHT, &env->egl.height);
    Log("EGL Window Surface %d x %d\n", env->egl.width, env->egl.height);

    env->type = ON_SCREEN_RENDER;

    LOG_EXIT;

    return 0;
}

/**
 * Read file in assets
 * Return 
 *		  -1 ERROR
 *      >= 0 file length
 */
static int readFileFromAssets(const SdkEnv *sdk, 
        const char *fname, 
        char **mem) {

    VALIDATE_NOT_NULL2(sdk, sdk->userData.platformData);
    VALIDATE_NOT_NULL2(fname, mem);

    AAssetManager *assetMgr = (AAssetManager *)(sdk->userData.platformData);
    AAsset *asset = AAssetManager_open(assetMgr, fname, AASSET_MODE_UNKNOWN);
    if (NULL == asset) {
        LogE("Failed open %s by asset manger\n", fname);
        return -1;
    }
    size_t size = AAsset_getLength(asset);
    if (size <= 0) {
        LogE("%s length is invalid (file length must > 0)\n", fname);
        AAsset_close (asset);
        return -1;
    }
    *mem = calloc(1, size + 1);
    if (NULL == *mem) {
        LogE("Failed calloc memory for %s\n", fname);
        AAsset_close (asset);
        return -1;
    }

    if (AAsset_read(asset, *mem, size) != size) {
        LogE("Failed read %s\n", fname);
        AAsset_close(asset);
        free (*mem);
        *mem = NULL;
        return -1;
    }

    AAsset_close(asset);
    return size + 1;
}

static bool initEffectCmd(eftcmd_t *cmd) {
   if (NULL == cmd) {
        return false;
   }

#define MAX_EFFECT_PARAM_COUNT 8
    cmd->capacity = MAX_EFFECT_PARAM_COUNT;
    cmd->valid = true;
    cmd->cmd = ec_NORMAL;
    cmd->count = 0;
    cmd->params = (int *)calloc(cmd->capacity, sizeof(int));
    assert(NULL != cmd->params);

    return true;
}

/**
 * Initialize the specified SdkEnv instance
 */
int initSdkEnv(SdkEnv *env) 
{
    VALIDATE_NOT_NULL(env);

	Log("initSdkEnv pthead self = %lx\n", pthread_self());
	Log("initSdkEnv gettid = %x\n", gettid());

    uint32_t t_begin, t_finish;

    if (env->type == SDK_STATUS_OK) {
        LogE("SDK is already initialized\n");
        return -1;
    }

    if (NULL == env->userData.platformData) {
        LogE("Please call setPlatformData first!\n");
        return -1;
    }

	chrbuf_t *userCmd = newChrbuf (USER_CMD_CAPABILITY);
    if (NULL == userCmd) {
        LogE ("Failed newChrbuf for userCmd\n");
        freeSdkEnv (env);
        return -1;
    }
    env->userCmd = userCmd;

    bool result = initEffectCmd(&env->effectCmd);
    assert(result);

    if (NULL != env->egl.window) {	// On-screen render
        t_begin = getCurrentTime();
        if (initEGL(env) < 0) {
            LogE("Failed initEGL\n");
            return -1;
        }
        t_finish = getCurrentTime();
        Log("Initialize EGL OK cost %d ms.\n", t_finish - t_begin);

        ANativeWindow *window = env->egl.window;
        env->userData.width = ANativeWindow_getWidth(window);
        env->userData.height = ANativeWindow_getHeight(window);
        Log ("Native window %d x %d\n", 
                env->userData.width, env->userData.height);

        if (env->egl.width != env->userData.width ||
                env->egl.height != env->userData.height) {
            LogE("EGL and Native window size are not equal\n");
            return -1;
        }
    } else {	// Off-screen render
        t_begin = getCurrentTime();
        if (initDefaultEGL(env) < 0) {
            LogE("Failed initDefaultEGL\n");
            return -1;
        }
        t_finish = getCurrentTime();
        Log("Initialize Default EGL OK cost %d ms.\n", t_finish - t_begin);
    }

#define VERT_SHDR_NAME "vert.shdr"
    t_begin = getCurrentTime();
    char *vertSource = NULL;
    int nVertSource = readFileFromAssets (env, VERT_SHDR_NAME, &vertSource);
    if (nVertSource < 0) {
        LogE ("Failed read assets file:%s\n", VERT_SHDR_NAME);
    }
    env->userData.nVertSource = nVertSource;
    env->userData.vertSource = vertSource; 
    t_finish = getCurrentTime();
    Log("Load %s OK cost %d ms.\n", VERT_SHDR_NAME, t_finish - t_begin);

#define FRAG_SHDR_NAME "frag.shdr"
    char *fragSource = NULL;
    int nFragSource = readFileFromAssets (env, FRAG_SHDR_NAME, &fragSource);
    if (nFragSource < 0) {
        LogE("Failed read assets file:%s\n", FRAG_SHDR_NAME);
    }
    env->userData.nFragSource = nFragSource;
    env->userData.fragSource = fragSource;
    t_finish = getCurrentTime();
    Log("Load %s OK cost %d ms.\n", FRAG_SHDR_NAME, t_finish - t_begin);

    if (attachShader(env, vertSource, fragSource) < 0) {
        LogE("Failed attachShader\n");
        return -1;
    }

    if (initGlBuffers (env) < 0) {
        LogE ("Failed init OpenGL buffers\n");
    }

    env->status = SDK_STATUS_OK;

    return 0;
}

/*
 * Set input image path
 * Parameters:
 *		env:	sdk context
 *		path:	input image path
 * Return:
 *		 0 OK
 *		-1 ERROR
 * Notice:
 *		userData.active & inputPath & inputImageType  may changed
 */
int setInputImagePath (SdkEnv* env, const char* path)
{
    VALIDATE_NOT_NULL2 (env, path);
	if (env->userData.inputPath != NULL &&
			strcmp(env->userData.inputPath, path) == 0) {
		Log ("Same input image path\n");
		return 0;
	}

    if (NULL != env->userData.inputPath) {
        free (env->userData.inputPath);
    }
    env->userData.inputPath = strdup (path);
    env->userData.active = ACTIVE_PATH;
	const char const *postfix = getFilePostfix (path);
	if (strcasecmp (postfix, "jpg") == 0) {
		env->userData.inputImageType = IMAGE_JPG;
	}
	else if (strcasecmp (postfix, "png") == 0) {
		env->userData.inputImageType = IMAGE_PNG;
	}
	else {
		env->userData.inputImageType = IMAGE_UNKNOWN;
	}
    return 0;
}

/*
 * Set output image path
 * Parameters:
 *              env:    sdk context
 *              path:   output image path
 *  Return:
 *               0  OK
 *              -1  ERROR
 */
int setOutputImagePath (SdkEnv* env, const char* path)
{
    VALIDATE_NOT_NULL2 (env, path);
    if (NULL != env->userData.outputPath) {
        free (env->userData.outputPath);
    }
    env->userData.outputPath = strdup(path);
    return 0;
}

/*
 * Get output image path
 * Parameters:
 *              env:    sdk context
 *  Return:
 *              NULL	ERROR
 */
char* getOutputImagePath (SdkEnv* env)
{
	VALIDATE_NOT_NULL(env);
	return env->userData.outputPath;
}


/*
 * Set image effect command
 * Parameters:
 *		env:	sdk context
 *		cmd:	user command
 * Return:
 *		-1 ERROR
 *		 0 OK
 */
int setEffectCmd(SdkEnv* env, const char* cmd)
{
    LOG_ENTRY;
    VALIDATE_NOT_NULL2(env, cmd);

	Log("ptheadself = %lx\n", pthread_self());
	Log("gettid = %x\n", gettid());

    env->effectCmd.valid = false;
    if (parseEffectCmd (cmd, &env->effectCmd) < 0) {
        LogE ("Failed parseEffectCmd FIXME! \n");
    }

    if (NULL == env->userData.inputPath &&
            NULL == env->userData.param) {
        LogE ("No input image\n");
        return -1;
    }

    Log ("effect cmd:%s\n", cmd);

	if (NULL != env->userCmd &&
			env->userCmd->base != NULL &&
			strcmp(cmd, env->userCmd->base) == 0) {
		Log ("The same effect cmd\n");
	}

    // input image by specified path
    if (ACTIVE_PATH == env->userData.active) {

		Log ("ACTIVE PATH\n"); 

        if (NULL == env->userData.inputPath) {
            LogE ("Please setImagePath first\n");
            return -1;
        }

        Bitmap_t *img = (Bitmap_t *) calloc(1, sizeof(Bitmap_t));
        if (NULL == img) {
            Log("Failed calloc mem or bitmap\n");
            return -1;
        }

        uint32_t begin_t = getCurrentTime();
        if(loadImage (env->userData.inputPath, img) < 0) {
            LogE("Failed loadImage\n");
            return -1;
        }
        uint32_t end_t = getCurrentTime();
        Log("Load %s cost %d ms\n", env->userData.inputPath, end_t - begin_t);
    

//#ifdef _DEBUG_
#if 0
        ///////////////////////////////////////
        // save iamge for test
        // == begin test
        begin_t = getCurrentTime ();
        char destname[64];
        memset (destname, 0, 64);
        strcpy (destname, "test_");
        strcat (destname, env->userData.outputPath);
        if(saveImage (destname, img) < 0) {
            LogE("Failed saveImage\n");
        }
        end_t = getCurrentTime();
        Log("Save %s cost %d ms\n", destname, end_t - begin_t);
        // == end test
        //////////////////////////////////////
#endif

        // In off-screen render, We must reAssign value.
        // Otherwise, the elemets will be invisible
        if (OFF_SCREEN_RENDER == env->type) {
            env->egl.width = img->width;
            env->egl.height = img->height;
        }

        clearChrbuf (env->userCmd);
        appendChrbuf (env->userCmd, cmd);
        env->userData.param = (void *) img;
        env->userData.active = ACTIVE_PARAM;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, env->handle.texture1Idx);
		GLint fmt = GL_RGBA;
		if (IMAGE_JPG == env->userData.inputImageType) {
			fmt = GL_RGB;
		}

        int level = 0;
#define BORDER 0
        glTexImage2D(GL_TEXTURE_2D, level, fmt, img->width, img->height, BORDER, fmt, GL_UNSIGNED_BYTE, img->base);

        glBindTexture(GL_TEXTURE_2D, env->handle.texture2Idx);
        glTexImage2D(GL_TEXTURE_2D, level, fmt, img->width, img->height, BORDER, fmt, GL_UNSIGNED_BYTE, NULL);


    }
    // reUse image in memory
    else if (ACTIVE_PARAM == env->userData.active) {

#if 0
        if (NULL == env->userData.param) {
            LogE ("No reuse image\n");
            return -1;
        }
#endif

        Log ("Reuse image\n");
    }

    LOG_EXIT;
    return 0;
}

static void onRender(SdkEnv *env) {

    if (NULL == env) {
        LogE("sdk context = NULL in onRender()\n");
        return;
    }

    Bitmap_t *bmp = (Bitmap_t *)env->userData.param;
    Log("image:%d x %d\n", bmp->width, bmp->height);
    Log("surface:%d x %d\n", env->egl.width, env->egl.height);
    float imgRatio = bmp->height / (float)bmp->width;
    float winRatio = env->egl.height / (float) env->egl.width;

    int width, height;
    if (imgRatio >= winRatio) {
        height = env->egl.height;
        width = (int)(height / (float)imgRatio);
    } else {
        width = env->egl.width;
        height = (int)(width * imgRatio);
    }
    int left = (env->egl.width - width) / 2;
    int top = (env->egl.height - height) / 2;
    Log("view port: (%d, %d, %d, %d\n", left, top, width, height);
    glViewport(left, top, width, height);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
    glUseProgram(env->handle.program);

    if ( OFF_SCREEN_RENDER == env->type ) {
        glBindFramebuffer (GL_FRAMEBUFFER, env->handle.fboIdx);

        int level = 0;
        glFramebufferTexture2D (
                GL_FRAMEBUFFER, 
                GL_COLOR_ATTACHMENT0,	 
                GL_TEXTURE_2D,
                env->handle.texture2Idx,
                level);

        GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LogE ("Framebuffer not ready. status code:0x%04x\n", status);
        }
    }

    float vertex[] = {
        -1.0,  1.0, 0,
        -1.0, -1.0, 0,
         1.0, -1.0, 0,
         1.0,  1.0, 0
    };

    float texCoord[] = {
        0, 1,
        0, 0,
        1, 0,
        1, 1
    };

    float colors[] = {
        0.0f, 1.0f, 0.2f
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, env->handle.texture1Idx);
    glUniform1i(env->handle.sampler2dIdx, 0);

    glEnableVertexAttribArray(env->handle.positionIdx);
    glVertexAttribPointer(env->handle.positionIdx, 3, GL_FLOAT, GL_FALSE, 0, vertex);

    glEnableVertexAttribArray(env->handle.texCoordIdx);
    glVertexAttribPointer(env->handle.texCoordIdx, 2, GL_FLOAT, GL_FALSE, 0, texCoord);


    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Only rendering with OpenGL ES 2.0, so
    // I simply call glFinish()
    uint32_t t1 = getCurrentTime ();	
    glFinish();
    uint32_t t2 = getCurrentTime ();
    Log ("Render cost %d ms\n", (t2 - t1) );

    if (ON_SCREEN_RENDER == env->type ) {
        Log ("On screen render\n");
        swapEglBuffers (env);
    } else if (OFF_SCREEN_RENDER == env->type) {
        Log ("Off screen render\n");
    }
}

/*
 * Render with OpenGLES shader
 */
static void onDraw(SdkEnv *env) {
    if (NULL == env) {
        LogE ("NULL pointer exception in onDraw()\n");
        return;
    }
    Log("onDraw()\n");

	Log("ptheadself = %lx\n", pthread_self());
	Log("gettid = %x\n", gettid());

#define POINT_COUNT 5
    // start vertex
    GLfloat vertex[POINT_COUNT * 3];
    int i = 0;
    int start_angle = 18;
    int delta_angle = 72;
    float factor = (float) env->egl.width / (float)env->egl.height;
    while ( i < POINT_COUNT * 3 ) {
        float angle = start_angle + (i / 3) * delta_angle;
		if (factor <= 1.0f) {
			vertex[i] = cos ( M_PI * angle / 180.0f);
			vertex[i+1] = sin (M_PI * angle / 180.0f) * factor;
		} else {
			vertex[i+1] = sin (M_PI * angle / 180.0f);
			vertex[i] = cos ( M_PI * angle / 180.0f) / factor;
		}
        vertex[i+2] = 0;
        i += 3;
    }

    GLfloat color[] = {
        1.0f, 0.0f, 1.0f, 1.0f
    };

    glViewport(0, 0, env->egl.width, env->egl.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
    glUseProgram(env->handle.program);

    if ( OFF_SCREEN_RENDER == env->type ) {
        glBindFramebuffer (GL_FRAMEBUFFER, env->handle.fboIdx);

        int level = 1;
        glFramebufferTexture2D (
                GL_FRAMEBUFFER, 
                GL_COLOR_ATTACHMENT0,	 
                GL_TEXTURE_2D,
                env->handle.texture2Idx,
                level);

        GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LogE ("Framebuffer not ready. status code:0x%04x\n", status);
        }
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertex);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, POINT_COUNT);

    // Only rendering with OpenGL ES 2.0, so
    // I simply call glFinish()
    uint32_t t1 = getCurrentTime ();	
    glFinish();
    uint32_t t2 = getCurrentTime ();
    Log ("Render cost %d ms\n", (t2 - t1) );

    if (ON_SCREEN_RENDER == env->type ) {
        Log ("On screen render\n");
        swapEglBuffers (env);
    } else if (OFF_SCREEN_RENDER == env->type) {
        Log ("Off screen render\n");
    }
}

static void onCreate(SdkEnv *env) {

}

static void onDestroy(SdkEnv *env) {

}

/**
 * Assign the native window to sdk context 
 * Notice:
 *		Never check if window is really a native window
 *		The user ensure window is valid.
 */

int setEglNativeWindow(SdkEnv *env, const EGLNativeWindowType window)
{
    VALIDATE_NOT_NULL2(env, window);
    env->egl.window = window;
}

/**
 * Swap back buffer and front buffer
 * Please call me in on-screen render,
 * And ignore me in off-screen render.
 */
void swapEglBuffers(const SdkEnv *env)
{
    if (NULL != env) {

        // Only rendering with OpenGL ES 2.0, so
        // No need call funtions below to synchronize
        /*
           eglWaitClient ();
           eglWaitNative (EGL_CORE_NATIVE_ENGINE);
         */

        eglSwapBuffers (env->egl.display, env->egl.surface);
    }
}

/**
 * Assign an variable which type is AssetManager * to sdk context in 
 * Android platform. It does not define in other platforms
 * Notice:
 *		Never check if data is really valid.
 *		The user ensure data is valid.
 */
int setPlatformData(SdkEnv *env, const void *data)
{
    VALIDATE_NOT_NULL2(env, data);
    env->userData.platformData = data;
	return 0;
}

/*
 * Just export env->onCreate() to user
 */
void onSdkCreate(SdkEnv *env)
{
    if (NULL != env && NULL != env->onCreate) {
        env->onCreate(env);
    }
}

/*
 * Just export env->onDraw() to user
 */
void onSdkDraw(SdkEnv *env)
{
    if (NULL != env && NULL != env->onDraw) {
        env->onDraw(env);
    }
}

/*
 * Just export env->onDestroy() to user
 */
void onSdkDestroy(SdkEnv *env)
{
    if (NULL != env && NULL != env->onDestroy) {
        env->onDestroy(env);
    }
}

/**
 * Get current time in milliseconds
 * Return:
 *		 > 0  OK
 *		   0  ERROR
 */
uint32_t getCurrentTime() 
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        return 0;
    }
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/**
 * Read jpeg file to memory
 * Return:
 *		 0 OK
 *		-1 error
 */
int read_jpeg(const char *path, Bitmap_t *mem)
{
    VALIDATE_NOT_NULL2 (path, mem);

    FILE *fp = fopen (path, "rb");
    if (NULL == fp) {
        Log("Failed open %s\n", path);
        return -1;
    }

    struct jpeg_decompress_struct jds;
    struct jpeg_error_mgr jerr;
    jds.err = jpeg_std_error (&jerr);
    jpeg_create_decompress (&jds);
    jpeg_stdio_src (&jds, fp);
    jpeg_read_header (&jds, TRUE);

    Log("[%s %d x %d %d]\n", path, jds.image_width, jds.image_height, jds.num_components);

    mem->width = jds.image_width;
    mem->height = jds.image_height;
    mem->form =  jds.num_components;
    mem->base = (char *) calloc (jds.image_width * jds.image_height * jds.num_components, 1);
    assert (NULL != mem->base);

    jpeg_start_decompress (&jds); 
    JSAMPROW row_pointer[1];
    while (jds.output_scanline < jds.output_height) {
        row_pointer[0] = (JSAMPROW)(mem->base + (jds.output_height - jds.output_scanline - 1) * jds.image_width * jds.num_components);
        jpeg_read_scanlines (&jds, row_pointer, 1);
    }
    jpeg_finish_decompress (&jds);
    jpeg_destroy_decompress (&jds);
    fclose (fp);

    return 0;
}

/**
 * Write jpeg data from memory to file
 * Return:
 *		 0 OK
 *		-1 error
 */
int write_jpeg(const char *path, const Bitmap_t *mem)
{
    VALIDATE_NOT_NULL3 (path, mem, mem->base);

    FILE *fp = fopen (path, "wb");
    if (NULL == fp) {
        return -1;
    }

    struct jpeg_compress_struct jcs;
    struct jpeg_error_mgr jerr;
    jcs.err = jpeg_std_error (&jerr);
    jpeg_create_compress (&jcs);
    jpeg_stdio_dest (&jcs, fp);
    jcs.image_width = mem->width;
    jcs.image_height = mem->height;

#ifdef _DEBUG_ 
    assert (mem->form >= GRAY && mem->form <= RGBA32);
    assert (mem->form != RGB16);
#endif

    int colorSpace = JCS_GRAYSCALE;
    int components = mem->form;
    if (components >= 3) {
        components = 3;
        colorSpace = JCS_RGB;
    }
    jcs.input_components = components;
    jcs.in_color_space = colorSpace;

    jpeg_set_defaults (&jcs);
#define QUALITY 80
    jpeg_set_quality (&jcs, QUALITY, TRUE);
    jpeg_start_compress (&jcs, TRUE);
    JSAMPROW row_pointer[1];
    int row_stride = mem->width * mem->form;

    while ( jcs.next_scanline < jcs.image_height ) {
        row_pointer[0] = (JSAMPROW)(mem->base + jcs.next_scanline * row_stride);
        jpeg_write_scanlines (&jcs, row_pointer, 1);
    }
    jpeg_finish_compress (&jcs);
    jpeg_destroy_compress (&jcs);
    fclose (fp);
    return 0;
}

/**
 * Load image
 * Parameters:
 *      path:   input path 
 *      mem:    memory for decoding
 * Return:
 *           0  OK
 *          -1  ERROR
 */
int loadImage (const char *path, Bitmap_t *mem) 
{
    VALIDATE_NOT_NULL2 (path, mem);
    const char const *postfix = getFilePostfix (path);
    if (NULL == postfix) {
        LogE ("Failed getFilePostfix in loadImage\n");
        return -1;
    }

	if (strcasecmp (postfix, "jpg") == 0 ||
			strcasecmp (postfix, "jpeg") == 0) {
		return read_jpeg (path, mem);
	}
    else if (strcasecmp (postfix, "png") == 0) {
        return read_png (path, mem);
    }
    else {
        LogE ("Invalid postfix name (%s) in loadImage\n", postfix);
        return -1;
    }
}

/**
 * Save image
 * Parameters:
 *      path:   output path 
 *      mem:    bitmap in memory
 * Return:
 *           0  OK
 *          -1  ERROR
 */
int saveImage (const char *path, const Bitmap_t *mem) 
{
    VALIDATE_NOT_NULL2 (path, mem);
    const char const *postfix = getFilePostfix (path);
    if (NULL == postfix) {
        LogE ("Failed getFilePostfix in saveImage\n");
        return -1;
    }

    if (strcasecmp (postfix, "jpg") == 0) {
        return write_jpeg (path, mem);
    }
    else if (strcasecmp (postfix, "png") == 0) {
        return write_png (path, mem);
    }
    else {
        LogE ("Invalid postfix name (%s) in saveImage\n", postfix);
        return -1;
    }
}
