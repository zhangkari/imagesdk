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

#define INFO  0
#define DEBUG 1
#define WARN  2
#define ERROR 3
static void LogD(const char* tag, const char* msg) {
	__android_log_write(DEBUG, tag, msg);
}

static void LogE(const char* tag, const char* msg) {
	__android_log_write(ERROR, tag, msg);
}


int defaultShader();
int readFile(const char* path, char** mem);
int loadShader(int shaderType, const char* source);

#endif
