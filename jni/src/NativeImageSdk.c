/********************************
 * file name:	NativeImageSdk.c
 * author:		Kari.Zhang
 *
 *******************************/

#include <stdint.h>
#include "NativeImageSdk.h"
#include "imgsdk.h"
#include "jniHelper.h"

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    initSDK
 * Signature: ()J
 */
jlong JNICALL Java_org_imgsdk_core_NativeImageSdk_initSDK
  (JNIEnv *env, jobject thiz)
{
	Log("++++ initSDK() in native method ++++\n");

    LogE("We need Context to get AssetManager\n");
    // TODO

	SdkEnv *sdk = newDefaultSdkEnv();
	if (NULL == sdk) {
		LogE("Failed newDefaultSdkEnv()\n");
        return 0;
	}

	jlong ptr = (jlong)((intptr_t)sdk);
	Log("---- initSDK() in native method ----\n");
	return ptr;
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    freeSDK
 * Signature: (J)V
 */
void JNICALL Java_org_imgsdk_core_NativeImageSdk_freeSDK
  (JNIEnv *env, jobject thiz, jlong ptr)
{
	Log("++++ freeSDK() in native method ++++\n");

    SdkEnv *sdk = (SdkEnv *)((intptr_t) ptr);
    if (NULL != sdk) {
        freeSdkEnv(sdk);
    }

	Log("---- freeSDK() in native method ----\n");
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    setEffectCmd
 * Signature: (JLjava/lang/String;)V
 */
void JNICALL Java_org_imgsdk_core_NativeImageSdk_setEffectCmd
  (JNIEnv *env, jobject thiz, jlong ptr, jstring jcommand)
{
	Log("++++ setEffectCmd() in native method ++++\n");

	SdkEnv* sdk = (SdkEnv *)((intptr_t)ptr);
	if (NULL == sdk) {
		LogE("NULL pointer exception\n");
		return;
	}

	char *cmd = jstring2string(env, jcommand);
    if (NULL == cmd) {
        LogE("Effect cmd is NULL\n");
        return;
    }

    Log("EffectCmd = %s\n", cmd);

	setEffectCmd(sdk, cmd);
	Log("---- setEffectCmd() in native method ----\n");
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    executeCmd
 * Signature: (J)V
 */
void JNICALL Java_org_imgsdk_core_NativeImageSdk_executeCmd
  (JNIEnv *env, jobject thiz, jlong ptr)
{
	Log("++++ executeCmd() in native method ++++\n");

	SdkEnv *sdk = (SdkEnv *) ((intptr_t) ptr);
	if (NULL == env) {
		LogE("NULL pointer exception\n");
		return;
	}

	// TODO

	Log("---- executeCmd() in native method ----\n");
}


/***
 * I am hesitated about how to register native methods;
 * I should choose this dynamic way in the future.
 * Now, I just use the static way
 */
#if 0
static JNINativeMethod gMethods[] = {
	{ 
		"Java_org_imgsdk_core_NativeImageSdk_initSDK",
		"()J;",
		Java_org_imgsdk_core_NativeImageSdk_initSDK
	},
	{ 
		"Java_org_imgsdk_core_NativeImageSdk_freeSDK",
		"(J)V;",
		Java_org_imgsdk_core_NativeImageSdk_executeCmd
	},
	{ 
		"Java_org_imgsdk_core_NativeImageSdk_setEffectCmd",
		"(JLjava/lang/String;)V;",
		Java_org_imgsdk_core_NativeImageSdk_setEffectCmd
	},
	{ 
		"Java_org_imgsdk_core_NativeImageSdk_executeCmd",
		"(J)V;",
		Java_org_imgsdk_core_NativeImageSdk_executeCmd
	},
};
#endif

