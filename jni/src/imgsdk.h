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

#include <android/log.h>
#include <GLES2/gl2.h>

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

#define INFO  0
#define DEBUG 1
#define WARN  2
#define ERROR 3

/*
static void Log(const char* tag, const char* fmt, ...) {
	__android_log_print(DEBUG, tag, fmt);
}
*/

static void LogD(const char* tag, const char* msg) {
	__android_log_write(DEBUG, tag, msg);
}

static void LogE(const char* tag, const char* msg) {
	__android_log_write(ERROR, tag, msg);
}

int read_png(const char *path, Bitmap_t *mem);
int write_png(const char *path, Bitmap_t *mem);
void freeBitmap(Bitmap_t *mem);

int readFile(const char* path, char** mem);
int executeVertexShader(const char* source);
int executeFragmentShader(const char* source);

#endif
