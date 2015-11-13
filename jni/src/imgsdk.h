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

typedef enum {
	GRAY   = 1,
	RGB16  = 2,
	RGB24  = 3,
	RGBA32 = 4
} PixForm_e;

typedef struct {
	PixForm_e form;
	int width;
	int height;
	char* base;
} Bitmap_t;

typedef enum PlatformType {
	PLATFORM_OLD     = -1,		// old platform
	PLATFORM_ANDROID =  0,		// Android platform
	PLATFORM_IOS     =  1,		// iOS platform
	PLATFORM_NEW     =  2		// latest platform
} PlatformType;

struct SdkEnv;
typedef struct SdkEnv SdkEnv;

#define VERT_SHADER_FILE "vert.shdr"
#define FRAG_SHADER_FILE "frag.shdr"

#define SURFACE_MAX_WIDTH  2048 
#define SURFACE_MAX_HEIGHT 2048 

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
int write_png(const char *path, Bitmap_t *mem);

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
int setEglNativeWindow(SdkEnv *env, EGLNativeWindowType window);

/**
 * Pass the platform related data to SdkEnv
 * Params:
 *		env:  SdkEnv instance
 *		data: ANativeAssetManager (AssetManager in java) instance in Android
 */
int setPlatformData(SdkEnv *env, void *data);

/**
 * Swap buffers
 */
void swapEglBuffers(SdkEnv *env);

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

/**
 * Parse user's effect command
 */
int parseEffectCmd(SdkEnv* env);

/**
 * Read file to memory. Do not forget to free memory
 */
int readFile(const char* path, char** mem);

/**
 * Get current time in milliseconds
 */
uint32_t getCurrentTime();

#endif
