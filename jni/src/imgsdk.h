/***************************
 * file name:   imgsdk.h
 * description:
 * author:      kari.zhang
 * date:        2015-05-09
 *
 ***************************/

#ifndef __IMGSDK__H__
#define __IMGSDK__H__

#define OK               0x0000 
#define NULL_POINTER    -0x0001
#define FILE_NOT_EXIST  -0x0002
#define INVALID_PARAMS  -0x0003
#define ERR_ALLOC_MEM   -0x0004

#include <stdio.h>
#include <android/log.h>

#define VALIDATE_NOT_NULL(X) 						\
    do { 											\
        if (NULL == X) { 							\
            LogE(#X" = NULL in %s", __FUNCTION__);	\
            return ;								\
        } 											\
    } while (0)
	
#define VALIDATE_NOT_NULL2(X, Y)    \
    do {                            \
        VALIDATE_NOT_NULL(X);       \
        VALIDATE_NOT_NULL(Y);       \
    } while (0)
	
#define VALIDATE_NOT_NULL3(X, Y, Z)             \
    do {                                        \
        VALIDATE_NOT_NULL2(X, Y);               \
        VALIDATE_NOT_NULL(X);                   \
    } while (0)


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

struct SdkEnv;
typedef struct SdkEnv SdkEnv;

#define INFO  0
#define DEBUG 1
#define WARN  2
#define ERROR 3

#define TAG "ImageSDK"
/*
#define Log(...) ((void)__android_log_print(INFO, TAG, __VA_ARGS__))
#define LogD(...) ((void)__android_log_print(DEBUG, TAG, __VA_ARGS__))
#define LogE(...) ((void)__android_log_print(ERROR, TAG, __VA_ARGS__))
*/

#define Log(...) ((void)printf( __VA_ARGS__))
#define LogD(...) ((void)printf( __VA_ARGS__))
#define LogE(...) ((void)printf( __VA_ARGS__))


#define VERT_SHADER_FILE "vert.shdr"
#define FRAG_SHADER_FILE "frag.shdr"

#define SURFACE_MAX_WIDTH  2048 
#define SURFACE_MAX_HEIGHT 2048 

int read_png(const char *path, Bitmap_t *mem);
int write_png(const char *path, Bitmap_t *mem);
void freeBitmap(Bitmap_t *mem);

SdkEnv* defaultSdkEnv();
int freeSdkEnv(SdkEnv *env);

int setEffectCmd(SdkEnv* env, const char* cmd);
int parseEffectCmd(SdkEnv* env);
int readFile(const char* path, char** mem);
int createProgram(const char* vertSource, const char* fragSource);

#endif
