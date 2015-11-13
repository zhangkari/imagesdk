/********************************
 * file name:	jniHelper.h
 * author:		Kari.Zhang
 *
 *******************************/

#ifndef __JNI__HELPER__H__
#define __JNI__HELPER__H__

#include <jni.h>

/**
 * Convert jstring to char*
 */
char* jstring2string(JNIEnv *env, jstring jstr);

/*
 * Convert char* to jstring
 */
jstring string2jstring(JNIEnv *env, const char *cmd);

#endif
