package org.imgsdk.core;

/**
 * Created by ws-kari on 15-11-13.
 */
public class ImageSdk {
    private long mPointer;

    static {
        System.loadLibrary("imgsdk");
    }

    public void onCreate() {
        mPointer = NativeImageSdk.initSDK();
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
