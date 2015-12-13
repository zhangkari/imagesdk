/****************************************
 *
 * file name:	android_main.c
 * description: android application entry
 * author:		kari.zhang
 *
 *
 ***************************************/

#include <android/log.h>
#include <android_native_app_glue.h>
#include <stdlib.h>
#include <time.h>
#include "imgsdk.h"

/*
 * HandleCommand()
 * Android callback for onAppCmd
 */
static void HandleCommand(struct android_app *pApp, int32_t cmd) {
	SdkEnv *sdk = (SdkEnv *) pApp->userData;
	switch ( cmd )
	{
		case APP_CMD_SAVE_STATE:
            LogD("save state");
			// the OS asked us to save the state of the app
			break;

        case APP_CMD_INIT_WINDOW:
            LogD("init window");
            setEglNativeWindow(sdk, pApp->window);
            if (initSdkEnv(sdk) < 0) {
                LogE("Failed initSdkEnv\n");
            }
            sdkMain(sdk);
			break;

		case APP_CMD_TERM_WINDOW:
            LogD("term window");
			onSdkDestroy(sdk);
			break;

		case APP_CMD_LOST_FOCUS:
            LogD("lost focus");
			break;

		case APP_CMD_GAINED_FOCUS:
            LogD("gain focus");
            onSdkDraw(sdk);
            swapEglBuffers(sdk);
			break;
	}
}


/*
 * android_main()
 * Main entrypoint for Android application
 */
void android_main(struct android_app *pApp)
{
	// Make sure glue isn't stripped.
	app_dummy();

	// Not use default SdkEnv
	// default is used to render off screen
	//	SdkEnv *sdkEnv = newDefaultSdkEnv();
	SdkEnv *sdkEnv = newBlankSdkEnv(PLATFORM_ANDROID);
	void *assetMgr = (void *)pApp->activity->assetManager;
	setPlatformData(sdkEnv, assetMgr);
	
	pApp->onAppCmd = HandleCommand;
	pApp->userData = sdkEnv;
	sdkMain(sdkEnv);
	while (1) {
		int ident;
		int events;
		struct android_poll_source *pSource;
		while ((ident = ALooper_pollAll (0, NULL, &events, (void **)&pSource)) >= 0) {
			if (pSource != NULL) {
				pSource->process(pApp, pSource);
			}
			if (pApp->destroyRequested != 0) {
				return;
			}
		}

	}
}
