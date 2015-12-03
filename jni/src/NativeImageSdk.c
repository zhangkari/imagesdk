/********************************
 * file name:	NativeImageSdk.c
 * author:		Kari.Zhang
 *
 *******************************/

#include <pthread.h>
#include <stdint.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include "NativeImageSdk.h"
#include "imgsdk.h"
#include "jniHelper.h"
#include "utility.h"

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    initSDK
 * Signature: ()J
 */
jlong JNICALL Java_org_imgsdk_core_NativeImageSdk_initSDK
  (JNIEnv *env, jobject thiz, jobject jcontext)
{
	LOG_ENTRY;
	if (NULL == jcontext) {
		LogE("NULL pointer exception:context = NULL\n");
		return 0;
	}

	Log("initSDK pthread id:%lx\n", pthread_self() );

	jclass cls = (*env)->GetObjectClass(env, jcontext);
	jmethodID mid = (*env)->GetMethodID(env, cls, 
			"getAssets", 
			"()Landroid/content/res/AssetManager;");
	jobject jassetMgr = (*env)->CallObjectMethod(env, jcontext, mid);
	if (NULL == jassetMgr) {
		LogE("Failed get java asset manager\n");
		return 0;
	}

	AAssetManager *assetMgr = AAssetManager_fromJava(env, jassetMgr);
	if (NULL == assetMgr) {
		LogE("Failed get native asset manager\n");
		return 0;
	}

	SdkEnv *sdk = newBlankSdkEnv(PLATFORM_ANDROID);
	if (NULL == sdk) {
		LogE("Failed newBlankSdkEnv\n");
        return 0;
	}

	setPlatformData(sdk, assetMgr);
	if (initSdkEnv (sdk) < 0) {
		LogE("Failed initSdkEnv \n");
		freeSdkEnv (sdk);
		return 0;
	}

	jlong ptr = (jlong) ( (intptr_t)sdk );
	LOG_EXIT;
	return ptr;
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    initRenderSDK
 * Signature: ()J
 */
jlong JNICALL Java_org_imgsdk_core_NativeImageSdk_initRenderSDK
  (JNIEnv *env, jobject thiz, jobject jcontext, jobject jsurface)
{
	LOG_ENTRY;
	if (NULL == jcontext) {
		LogE("NULL pointer exception:context = NULL\n");
		return 0;
	}

	Log("initRenderSDK pthread id:%lx\n", pthread_self() );

	jclass cls = (*env)->GetObjectClass(env, jcontext);
	jmethodID mid = (*env)->GetMethodID(env, cls, 
			"getAssets", 
			"()Landroid/content/res/AssetManager;");
	jobject jassetMgr = (*env)->CallObjectMethod(env, jcontext, mid);
	if (NULL == jassetMgr) {
		LogE("Failed get java asset manager\n");
		return 0;
	}

	AAssetManager *assetMgr = AAssetManager_fromJava(env, jassetMgr);
	if (NULL == assetMgr) {
		LogE("Failed get native asset manager\n");
		return 0;
	}

	if (NULL == jsurface) {
		LogE("NULL pointer exception:surface = NULL\n");
		return 0;
	}
	ANativeWindow *window = ANativeWindow_fromSurface(env, jsurface);
	if (NULL == window) {
		LogE("Failed get native window\n");
		return 0;
	}

	SdkEnv *sdk = newBlankSdkEnv(PLATFORM_ANDROID);
	if (NULL == sdk) {
		LogE("Failed newBlankSdkEnv\n");
        return 0;
	}

	setEglNativeWindow(sdk, window);
	setPlatformData(sdk, assetMgr);
	if (initSdkEnv (sdk) < 0) {
		LogE("Failed initSdkEnv \n");
		freeSdkEnv (sdk);
		return 0;
	}

	//
	// Run shader program default
	//
	sdkMain (sdk);

	jlong ptr = (jlong) ( (intptr_t)sdk );
	LOG_EXIT;
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
	LOG_ENTRY;

	Log("freeSDK pthread id:%lx\n", pthread_self() );

    SdkEnv *sdk = (SdkEnv *)((intptr_t) ptr);
    if (NULL != sdk) {
        freeSdkEnv(sdk);
    }
	LOG_EXIT;
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    setInputPath
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_imgsdk_core_NativeImageSdk_setInputPath
  (JNIEnv *env, jobject thiz, jlong ptr, jstring jpath)
{
	LOG_ENTRY;
	SdkEnv* sdk = (SdkEnv *)((intptr_t)ptr);
	if (NULL == sdk) {
		LogE("NULL pointer exception\n");
		return;
	}

	if (NULL == jpath) {
		LogE("NULL pointer exception\n");
		return;
	}

	char *path = jstring2string(env, jpath);
    if (NULL == path) {
        LogE("input path is NULL\n");
        return;
    }

	Log("input path:%s\n", path);
	setInputImagePath(sdk, path);

	LOG_EXIT;
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    setOutputPath
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_imgsdk_core_NativeImageSdk_setOutputPath
  (JNIEnv *env, jobject thiz, jlong ptr, jstring jpath)
{
	LOG_ENTRY;
	SdkEnv* sdk = (SdkEnv *)((intptr_t)ptr);
	if (NULL == sdk) {
		LogE("NULL pointer exception\n");
		return;
	}

	if (NULL == jpath) {
		LogE("NULL pointer exception\n");
		return;
	}

	char *path = jstring2string(env, jpath);
    if (NULL == path) {
        LogE("output path is NULL\n");
        return;
    }

    Log("output path:%s\n", path);
	setOutputImagePath(sdk, path);

	LOG_EXIT;
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    setEffectCmd
 * Signature: (JLjava/lang/String;)V
 */
void JNICALL Java_org_imgsdk_core_NativeImageSdk_setEffectCmd
  (JNIEnv *env, jobject thiz, jlong ptr, jstring jcommand)
{
	LOG_ENTRY;
	SdkEnv* sdk = (SdkEnv *)((intptr_t)ptr);
	if (NULL == sdk) {
		LogE("NULL pointer exception\n");
		return;
	}

	if (NULL == jcommand) {
		LogE("NULL pointer exception\n");
		return;
	}

	Log("setEffectCmd pthread id:%lx\n", pthread_self() );

	char *cmd = jstring2string(env, jcommand);
    if (NULL == cmd) {
        LogE("Effect cmd is NULL\n");
        return;
    }

    Log("EffectCmd = %s\n", cmd);
	setEffectCmd (sdk, cmd);

	LOG_EXIT;
}

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    executeCmd
 * Signature: (J)V
 */
void JNICALL Java_org_imgsdk_core_NativeImageSdk_executeCmd
  (JNIEnv *env, jobject thiz, jlong ptr, jobject jlistener, jobject param)
{
	LOG_ENTRY;
	SdkEnv *sdk = (SdkEnv *) ((intptr_t) ptr);
	if (NULL == sdk) {
		LogE("NULL pointer exception\n");
		return;
	}

	Log("executeCmd pthread id:%lx\n", pthread_self() );

	swapEglBuffers(sdk);
    onSdkDraw (sdk);

	if (NULL != jlistener) {
		jclass cls = (jclass)(*env)->GetObjectClass(env, jlistener);
		if (NULL == cls) {
			LogE("Not found OnEditCompleteListener\n");
			return;
		}

		jmethodID mid = (jmethodID)(*env)->GetMethodID(env, cls, "onSuccess", "(Ljava/lang/String;Ljava/lang/Object;)V");
		if (mid == NULL) {
			LogE("Not found onSuccess(String, Object\n");
			return;
		}

		char *path = getOutputImagePath(sdk);
		Log("output path:%s\n", path);
		jstring jpath = NULL;
		if (NULL != path) {
			jpath = string2jstring(env, path);
		}
		(*env)->CallVoidMethod(env, jlistener, mid, jpath, param);
		if (NULL != jpath) {
			(*env)->DeleteGlobalRef(env, jpath);
		}
	}

	LOG_EXIT;
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

