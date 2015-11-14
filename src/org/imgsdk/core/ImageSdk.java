package org.imgsdk.core;

import android.content.Context;

/**
 * Created by ws-kari on 15-11-13.
 */
public class ImageSdk {
    private long mPointer;

    static {
        System.loadLibrary("imgsdk");
    }

    public void onCreate(Context context) {
        mPointer = NativeImageSdk.initSDK(context);
    }

    public void onDestroy() {
        NativeImageSdk.freeSDK(mPointer);
    }

    public void setEffectCmd(String cmd) {
        NativeImageSdk.setEffectCmd(mPointer, cmd);
    }

    public void executeCmd() {
        NativeImageSdk.executeCmd(mPointer);
    }
}
