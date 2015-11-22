/***************************
 * file name:   comm.h
 * description:
 * author:      kari.zhang
 * date:        2015-05-09
 *
 ***************************/

#ifndef __COMM__H__
#define __COMM__H__

#include <android/log.h>
#include <stdio.h>

#define OK               0x0000 
#define NULL_POINTER    -0x0001
#define FILE_NOT_EXIST  -0x0002
#define INVALID_PARAMS  -0x0003
#define ERR_ALLOC_MEM   -0x0004


#define VALIDATE_NOT_NULL(X) 						\
    do { 											\
        if (NULL == X) { 							\
            LogE(#X" = NULL in %s\n", __FUNCTION__);	\
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


#define DEBUG 3
#define INFO  4
#define WARN  5
#define ERROR 6

#define TAG "ImageSDK"

#ifdef _ANDROID_
// Android log
#define Log(...) ((void)__android_log_print(INFO, TAG, __VA_ARGS__))
#define LogD(...) ((void)__android_log_print(DEBUG, TAG, __VA_ARGS__))
#define LogE(...) ((void)__android_log_print(ERROR, TAG, __VA_ARGS__))
#else
// output to stdout
#define Log(...) ((void)printf( __VA_ARGS__))
#define LogD(...) ((void)printf( __VA_ARGS__))
#define LogE(...) ((void)printf( __VA_ARGS__))
#endif

#endif
