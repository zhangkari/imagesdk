/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_imgsdk_core_NativeImageSdk */

#ifndef _Included_org_imgsdk_core_NativeImageSdk
#define _Included_org_imgsdk_core_NativeImageSdk
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    initSDK
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_imgsdk_core_NativeImageSdk_initSDK
  (JNIEnv *, jobject, jobject);

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    freeSDK
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_imgsdk_core_NativeImageSdk_freeSDK
  (JNIEnv *, jobject, jlong);

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    setEffectCmd
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_imgsdk_core_NativeImageSdk_setEffectCmd
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     org_imgsdk_core_NativeImageSdk
 * Method:    executeCmd
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_imgsdk_core_NativeImageSdk_executeCmd
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
