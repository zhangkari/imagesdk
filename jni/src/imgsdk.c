/***************************
 * file name:   imgsdk.c
 * description:
 * author:      kari.zhang
 * date:        2015-05-09
 *
 ***************************/

#include <malloc.h>
#include <string.h>
#include <sys/time.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>
#include "png.h"
#include "imgsdk.h"

/**
 * User's command
 */
typedef struct {
    GLchar  *cmd;
    GLuint  cmdLen;
    GLuint  cmdSize;
    GLuint  paramCount;
} UserCmd;

typedef enum ActiveType {
	ACTIVE_NONE  = 0,
	ACTIVE_PARAM = 1,
	ACTIVE_PIXEL = 2,
} ActiveType;

/**
 * User's image information
 */
typedef struct {
    GLuint			width;			// user's image width
    GLuint			height;			// user's image height
    GLchar			*param;			// user's parameter
    GLchar			*pixel;			// user's image pixel
	ActiveType		active;			// which is active ( param or pixel) 
	PixForm_e		pixfmt;			// pixel format
	PlatformType	platform;		// 0 Android, 1 iOS
	void			*platformData;	// AssetManager in Android
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
    GLint position;				// attribute vec3 aPosition
	GLint color;				// uniform vec4 uColor
    GLuint textureId;           // texture handle
    GLuint sampler2D;           // sampler handler
    GLuint texCoord;            // texture coordinate
} CommHandle;

/**
 * EGL environment
 */
typedef struct eglEnv {
	EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLint     width;
    EGLint     height;
    EGLNativeWindowType window;
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
    UserCmd userCmd;
    // callback	
	CallbackFunc onCreate;
    CallbackFunc onDraw;
	CallbackFunc onDestroy;
} SdkEnv;

static void onCreate(SdkEnv *env);
static void onDraw(SdkEnv *env);
static void onDestroy(SdkEnv *env);

int sdkMain(SdkEnv *env)
{
	VALIDATE_NOT_NULL(env);

	env->onCreate  = onCreate;
	env->onDraw	   = onDraw;
	env->onDestroy = onDestroy;

	onSdkCreate(env);
}

int main(int argc, char **argv) {
    if (2 != argc) {
        Log("Usage:\n");
        Log("  %s path\n", argv[0]);
        return -1;
    }

    SdkEnv *env = newDefaultSdkEnv();
    if (NULL == env) {
        LogE("Failed get SdkEnv instance\n");
    }

	sdkMain (env);
    setEffectCmd (env, "test cmd ");
    onSdkDraw (env);

	uint32_t begin_t = getCurrentTime();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // copy pixels from GPU memory to CPU memory
	int x = 0;
	int y = 0;
    glReadPixels(x, y, env->userData.width, env->userData.height, GL_RGBA, GL_UNSIGNED_BYTE, env->userData.pixel);

	uint32_t finish_t = getCurrentTime();
	LogD("Read pixel data cost %d ms\n", (finish_t - begin_t));

	begin_t = getCurrentTime();

#define DEST_PATH  "2.png"
    if (write_png(DEST_PATH, (Bitmap_t *)env->userData.param) < 0) {
        LogE("Failed write png\n");
    }

	finish_t = getCurrentTime();
	LogD("Save %s cost %d ms\n", DEST_PATH, (finish_t - begin_t));

    freeSdkEnv(env);
	//onSdkDestroy(env);
}

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

int write_png(const char *path, Bitmap_t *mem)
{
    VALIDATE_NOT_NULL2(path, mem);
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;

    fp = fopen(path, "wb");
    if (NULL == fp) {
        LogE("Failed open file");
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

    return 0;
}

#define LOG_BUFF_SIZE 1024
static char gLogBuff[LOG_BUFF_SIZE];

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
		memset(gLogBuff, 0, LOG_BUFF_SIZE);
        glGetShaderInfoLog(shader, LOG_BUFF_SIZE, &len, gLogBuff);
        if (len > 0) {
            Log("compile error:%s\n", gLogBuff);
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
static int createProgram(SdkEnv *env, const char* vertexSource, const char* fragSource)
{
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
			memset(gLogBuff, 0, LOG_BUFF_SIZE);
            glGetProgramInfoLog(program, LOG_BUFF_SIZE, &len, gLogBuff); 
            if (len > 0) {
                Log("link error log:%s\n", gLogBuff);
            } else {
                LogE("Failed get link log\n");
            }
            break;
        }

		glValidateProgram(program);
		GLint success;
		glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
		if (!success) {
			memset(gLogBuff, 0, LOG_BUFF_SIZE);
			glGetProgramInfoLog(program, LOG_BUFF_SIZE, &len, gLogBuff);
			if (len > 0) {
				Log("program is invalidate:%s\n", gLogBuff); 
			} else {
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

static int initDefaultEGL(SdkEnv *env) {
    VALIDATE_NOT_NULL(env);
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
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
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
		EGL_WIDTH, SURFACE_MAX_WIDTH,   // pBuffer needs at least 1 x 1
		EGL_HEIGHT,SURFACE_MAX_HEIGHT,  // pBuffer needs at least 1 x 1 
		EGL_TEXTURE_TARGET, EGL_NO_TEXTURE,
		EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE,
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
        LogE("Failed create context\n");
        Log("error code:%x\n", eglGetError());
        return -1;
    }

    if (!eglMakeCurrent(env->egl.display, env->egl.surface, env->egl.surface, env->egl.context)) {
        LogE("Failed eglMakeCurrent\n");
        EGLint errcode = eglGetError();
        LogE("error code:%x\n", errcode);
        return -1;
    }

    eglQuerySurface(env->egl.display, env->egl.surface, EGL_WIDTH, &env->egl.width);
    eglQuerySurface(env->egl.display, env->egl.surface, EGL_HEIGHT, &env->egl.height);
    Log("EGL Pbuffer Surface %d x %d\n", env->egl.width, env->egl.height);

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
    }

    env->egl.display = EGL_NO_DISPLAY;
    env->egl.context = EGL_NO_CONTEXT;
    env->egl.surface = EGL_NO_SURFACE;
	if (ACTIVE_PIXEL == env->userData.active 
			&& NULL != env->userData.pixel) {
		free (env->userData.pixel);
		env->userData.pixel = NULL;
	}

	if (ACTIVE_PARAM == env->userData.active 
			&& NULL != env->userData.param) {
		free (env->userData.param);
		env->userData.param = NULL;
	}

	if (NULL != env->userCmd.cmd) {
		free (env->userCmd.cmd);
		env->userCmd.cmd = NULL;
	}

	releaseShader(env);

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

	env->handle.position = glGetAttribLocation(env->handle.program, "aPosition");
	if (env->handle.position < 0) {
		LogE("Failed get aPosition location\n");
	}

	env->handle.color = glGetUniformLocation(env->handle.program, "uColor");
	if (env->handle.color < 0) {
		LogE("Failed get uColor location\n");
	}

	env->handle.sampler2D = glGetUniformLocation(env->handle.program, "uSampler2D");
	if (env->handle.sampler2D < 0) {
		LogE("Failed get uSampler2D location\n");
	}

    env->handle.texCoord = glGetAttribLocation(env->handle.program, "aTexCoord");
    if (env->handle.texCoord < 0) {
        LogE("Failed get aTexCoord location\n");
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
    Log("%s\n", version);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    Log("%s\n", renderer);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    Log("%s\n", vendor);

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

	env->status = SDK_STATUS_OK;

    return env;
}

/**
 * Initialize EGL by specified params
 */
int initEGL(SdkEnv *env)
{
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
        LogE("Failed create context\n");
        Log("error code:%x\n", eglGetError());
        return -1;
    }

    if (!eglMakeCurrent(env->egl.display, env->egl.surface, env->egl.surface, env->egl.context)) {
        LogE("Failed eglMakeCurrent\n");
        EGLint errcode = eglGetError();
        LogE("error code:%x\n", errcode);
        return -1;
    }

    eglQuerySurface(env->egl.display, env->egl.surface, EGL_WIDTH, &env->egl.width);
    eglQuerySurface(env->egl.display, env->egl.surface, EGL_HEIGHT, &env->egl.height);
    Log("EGL Window Surface %d x %d\n", env->egl.width, env->egl.height);

    return 0;
}

/**
 * Initialize the specified SdkEnv instance
 */
int initSdkEnv(SdkEnv *env) 
{
	VALIDATE_NOT_NULL(env);

	if (env->type == SDK_STATUS_OK) {
		LogE("SDK is already initialized\n");
		return -1;
	}

	if (NULL == env->userData.platformData) {
		LogE("Please call setPlatformData first!\n");
		return -1;
	}

    if (NULL != env->egl.window) {
        if (initEGL(env) < 0) {
            LogE("Failed initEGL\n");
            return -1;
        }
        Log("Initialize EGL OK.\n");
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

    } 
    else {
        if (initDefaultEGL(env) < 0) {
            LogE("Failed initDefaultEGL\n");
            return -1;
        }
        Log("Initialize Default EGL OK.\n");

        env->userData.width = env->egl.width;
        env->userData.height = env->egl.height;
    }

  
    AAssetManager *assetMgr = (AAssetManager *)(env->userData.platformData);
#define VERT_SHDR_NAME "vert.shdr"
    AAsset *asset = AAssetManager_open(assetMgr, VERT_SHDR_NAME, AASSET_MODE_UNKNOWN);
    if (NULL == asset) {
        LogE("Failed open %s by asset manger\n", VERT_SHDR_NAME);
        return -1;
    }
    size_t size = AAsset_getLength(asset);
    if (size <= 0) {
        LogE("%s length is invalid\n", VERT_SHDR_NAME);
        return -1;
    }
    env->userData.nVertSource = size + 1;
    char *vertSource = (char *)calloc(1, size + 1);
    if (NULL == vertSource) {
        LogE("Failed calloc memory for vertSource\n");
        AAsset_close(asset);
        return -1;
    }
    
    if (AAsset_read(asset, vertSource, size) != size) {
        LogE("Failed read %s\n", VERT_SHDR_NAME);
        AAsset_close(asset);
        free (vertSource);
        return -1;
    }
    AAsset_close(asset);
    env->userData.vertSource = vertSource; 
    Log("Load %s OK.\n", VERT_SHDR_NAME);

#define FRAG_SHDR_NAME "frag.shdr"
    asset = AAssetManager_open(assetMgr, FRAG_SHDR_NAME, AASSET_MODE_UNKNOWN);
    if (NULL == asset) {
        LogE("Failed open %s by asset manger\n", FRAG_SHDR_NAME);
        return -1;
    }
    size = AAsset_getLength(asset);
    if (size <= 0) {
        LogE("%s length is invalid\n", FRAG_SHDR_NAME);
        return -1;
    }
    env->userData.nFragSource = size + 1;
    char *fragSource = (char *)calloc(1, size + 1);
    if (NULL == fragSource) {
        LogE("Failed calloc memory for fragSource\n");
        AAsset_close(asset);
        return -1;
    }
    
    if (AAsset_read(asset, fragSource, size) != size) {
        LogE("Failed read %s\n", FRAG_SHDR_NAME);
        AAsset_close(asset);
        free (fragSource);
        return -1;
    }
    AAsset_close(asset);
    env->userData.fragSource = fragSource;
    Log("Load %s OK.\n", FRAG_SHDR_NAME);

	if (attachShader(env, vertSource, fragSource) < 0) {
		LogE("Failed attachShader\n");
		return -1;
	}

	env->status = SDK_STATUS_OK;

    return 0;
}

int setEffectCmd(SdkEnv* env, const char* cmd)
{
    VALIDATE_NOT_NULL2(env, cmd);

    // TODO
    // parseEffectCmd

    Log ("efffect command:%s\n", cmd);

    // TODO

    if (NULL == env->userCmd.cmd || strcmp(cmd, env->userCmd.cmd) != 0) {
        Bitmap_t *img = (Bitmap_t *) calloc(1, sizeof(Bitmap_t));
        if (NULL == img) {
            Log("Failed calloc mem or bitmap\n");
            return -1;
        }

        uint32_t begin_t = getCurrentTime();

#define PATH "/data/local/tmp/bg.png"

        if(read_png(PATH, img) < 0) {
            LogE("Failed read png\n");
            return -1;
        }
        uint32_t end_t = getCurrentTime();
        Log("Read %s cost %d ms\n", PATH, end_t - begin_t);

        if (NULL != env->userCmd.cmd) {
            free (env->userCmd.cmd);
            env->userCmd.cmd = NULL;
        }
        env->userCmd.cmd = strdup (cmd);
        env->userData.param = (void *) img;
        env->userData.active = ACTIVE_PARAM;

        // TODO
        GLuint texture;
        glGenTextures(1, &texture);
        int level = 0;
#define BORDER 0
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, img->width, img->height, BORDER, GL_RGBA, GL_UNSIGNED_BYTE, img->base);
        env->handle.textureId = texture;
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    } else {
        Log ("Cmd is the same\n");
    }

    return 0;
}

int parseEffectCmd(SdkEnv* env)
{
    VALIDATE_NOT_NULL(env);
    
    return -1;
}

static void onDraw(SdkEnv *env) {

    /*
	GLfloat vertex[] = {
		 0.0,   0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f
	};
    */
    GLfloat vertex[] = {
        -0.9,  0.9, 0.0f,
        -0.9, -0.9, 0.0f,
        0.9, -0.9, 0,
        0.9,  0.9, 0.0f
    };

    GLfloat texCoord[] = {
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0
    };

	GLfloat color[] = {
		0.0f, 0.0f, 1.0f, 1.0f
	};
  
	glViewport(0, 0, env->egl.width, env->egl.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(env->handle.program);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, env->handle.textureId);
    glUniform1i (env->handle.sampler2D, 0);


	glVertexAttribPointer(env->handle.position, 3, GL_FLOAT, GL_FALSE, 0, vertex);
	glEnableVertexAttribArray(env->handle.position);

	glVertexAttribPointer(env->handle.texCoord, 2, GL_FLOAT, GL_FALSE, 0, texCoord);
	glEnableVertexAttribArray(env->handle.texCoord);

	glUniform4fv(env->handle.color, 1, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(env->handle.position);

}

static void onCreate(SdkEnv *env) {

}

static void onDestroy(SdkEnv *env) {

}

int setEglNativeWindow(SdkEnv *env, EGLNativeWindowType window)
{
	VALIDATE_NOT_NULL2(env, window);
	env->egl.window = window;
}

void swapEglBuffers(SdkEnv *env)
{
	if (NULL != env) {
        eglWaitClient();
		eglSwapBuffers(env->egl.display, env->egl.surface);
	}
}

int setPlatformData(SdkEnv *env, void *data)
{
	VALIDATE_NOT_NULL2(env, data);
	env->userData.platformData = data;
}

void onSdkCreate(SdkEnv *env)
{
	if (NULL != env && NULL != env->onCreate) {
		env->onCreate(env);
	}
}

void onSdkDraw(SdkEnv *env)
{
	if (NULL != env && NULL != env->onDraw) {
		env->onDraw(env);
	}
}

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
