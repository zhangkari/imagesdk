/***************************
 * file name:   imgsdk.h
 * description:
 * author:      kari.zhang
 * date:        2015-05-09
 *
 ***************************/

#ifndef __IMGSDK__H__
#define __IMGSDK__H__

#include <android/log.h>
#include <EGL/egl.h>
#include "comm.h"

/*
 * pixel color format
 */
typedef enum {
	GRAY   = 1,			// gray scale
	RGB16  = 2,			// rgb 5-6-6
	RGB24  = 3,			// rgb 8-8-8
	RGBA32 = 4			// rgba 8-8-8-8
} PixForm_e;

/**
 * Used for storage image in memory. The available space
 * is width * height * form
 */
typedef struct {
	PixForm_e form;		// pixel color format
	int width;			// image width
	int height;			// image height
	char* base;			// base address
} Bitmap_t;

/**
 * The platform supported currently
 */
typedef enum PlatformType {
	PLATFORM_OLD     = -1,		// old platform
	PLATFORM_ANDROID =  0,		// Android platform
	PLATFORM_IOS     =  1,		// iOS platform
	PLATFORM_NEW     =  2		// latest platform
} PlatformType;

/**
 * Sdk context defined in imgsdk.c. It's not visible to user
 */
struct SdkEnv;
typedef struct SdkEnv SdkEnv;

#define VERT_SHADER_FILE "vert.shdr"
#define FRAG_SHADER_FILE "frag.shdr"

/**
 * Load image
 * Parameters:
 *      path:   input path 
 *      mem:    memory for decoding
 * Return:
 *           0  OK
 *          -1  ERROR
 */
int loadImage (const char *path, Bitmap_t *mem);

/**
 * Save image
 * Parameters:
 *      path:   output path 
 *      mem:    bitmap in memory
 * Return:
 *           0  OK
 *          -1  ERROR
 */
int saveImage (const char *path, const Bitmap_t *mem);


/**
 * Read png file to memory
 * Return:
 *		 0 OK
 *		-1 error
 */
int read_png(const char *path, Bitmap_t *mem);

/**
 * Write png data from memory to file
 * Return:
 *		 0 OK
 *		-1 error
 */
int write_png(const char *path, const Bitmap_t *mem);

/**
 * Read jpeg file to memory
 * Return:
 *		 0 OK
 *		-1 error
 */
int read_jpeg(const char *path, Bitmap_t *mem);

/**
 * Write jpeg data from memory to file
 * Return:
 *		 0 OK
 *		-1 error
 */
int write_jpeg(const char *path, const Bitmap_t *mem);


/*
 * Free bitmap
 */
void freeBitmap(Bitmap_t *mem);

/**
 * Create an uninitialized SdkEnv instance, must call initSdkEnv next
 * Params:
 *		platfomr: Must specify the platform. Now just support Android & iOS
 */
SdkEnv* newBlankSdkEnv(int platform);

/**
 * Initialize SdkEnv instance. Ignore me if created by newDefaultSdkEnv
 * SdkEnv is used to render on-screen
 */ int initSdkEnv(SdkEnv *env);

/**
 * Create a default SdkEnv instance. Do not call initSdkEnv next 
 * Default SdkEnv is used to render off-screen
 */
SdkEnv* newDefaultSdkEnv();

/**
 * Free SdkEnv instance
 */
int freeSdkEnv(SdkEnv *env);

/**
 * Run sdk
 */
int sdkMain(SdkEnv *env);

/**
 * Pass the platform related window to SdkEnv
 */
int setEglNativeWindow(SdkEnv *env, const EGLNativeWindowType window);

/**
 * Pass the platform related data to SdkEnv
 * Params:
 *		env:  SdkEnv instance
 *		data: ANativeAssetManager (AssetManager in java) instance in Android
 */
int setPlatformData(SdkEnv *env, const void *data);

/**
 * Swap buffers
 */
void swapEglBuffers(const SdkEnv *env);

/**
 * Create sdk
 */
void onSdkCreate(SdkEnv *env);

/**
 * Draw frames
 */
void onSdkDraw(SdkEnv *env);

/**
 * Destroy sdk
 */
void onSdkDestroy(SdkEnv *env);

/**
 * Set effect command
 */
int setEffectCmd(SdkEnv* env, const char* cmd);

/*
 * Set input image path
 */
int setInputImagePath (SdkEnv* env, const char* path);

/*
 * Set output image path
 */
int setOutputImagePath (SdkEnv* env, const char* path);

/*
 * Get output image path
 * Parameters:
 *              env:    sdk context
 *  Return:
 *              NULL	ERROR
 */
char* getOutputImagePath (SdkEnv* env);

/**
 * Read file to memory. Do not forget to free memory
 */
int readFile(const char* path, char** mem);

/**
 * Get current time in milliseconds
 */
uint32_t getCurrentTime();

#endif
