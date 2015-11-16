/********************************
 * file name:	jniHelper.c
 * author:		Kari.Zhang
 *
 *******************************/

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "comm.h"
#include "jniHelper.h"

/**
 * Convert jstring to char*
 */
char* jstring2string(JNIEnv *env, jstring jstr) 
{
	if (NULL == env || NULL == jstr) {
		return NULL;
	}

	char *cmd = NULL;
	jclass clsStr = (jclass) (*env)->FindClass(env, "java/lang/String");
	jstring strEncode = (jstring) (*env)->NewStringUTF(env, "UTF-8");
	jmethodID mid = (jmethodID) (*env)->GetMethodID(env, clsStr, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray byteArr = (jbyteArray) (*env)->CallObjectMethod(env, jstr, mid, strEncode);
	jsize len = (*env)->GetArrayLength(env, byteArr);
	jbyte *bp = (jbyte *) (*env)->GetByteArrayElements(env, byteArr, JNI_FALSE);
	if (len > 0) {
		cmd = (char *)calloc(1, len + 1);
		memcpy(cmd, bp, len);
	} else {
		LogE("Failed env->GetByteArrayElements\n");
	}

	(*env)->ReleaseByteArrayElements (env, byteArr, bp, 0);
	(*env)->DeleteLocalRef (env, strEncode);

	return cmd;
}

/*
 * Convert char* to jstring
 */
jstring string2jstring(JNIEnv *env, const char *cmd) 
{
	if (NULL == env || NULL == cmd) {
		return NULL;
	}

	jclass clsStr = (jclass) (*env)->FindClass(env, "java/lang/String");
	jmethodID mid = (jmethodID) (*env)->GetMethodID(env, clsStr, "<init>", "([BLjava/lang/String;)V");
	jstring strEncode = (jstring) (*env)->NewStringUTF(env, "UTF-8");

	int len = strlen(cmd);
	jbyteArray byteArr = (jbyteArray) (*env)->NewByteArray(env, len);
	(*env)->SetByteArrayRegion (env, byteArr, 0, len, (jbyte *)cmd);

	(*env)->DeleteLocalRef (env, strEncode);

	return (jstring) (*env)->NewObject(clsStr, mid, byteArr, strEncode);
}
