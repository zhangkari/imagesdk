package org.imgsdk.core;

import android.content.Context;
import android.view.Surface;

import java.lang.Object;

/**
 * Created by ws-kari on 15-11-13.
 */
public final class ImageSdk {
    private long mPointer;
    private NativeImageSdk nativeImageSdk;
    private Context mContext;
    private Surface mSurface;

    static {
        System.loadLibrary("imgsdk");
    }

    public ImageSdk(Context context) {
        nativeImageSdk = new NativeImageSdk();
        mContext = context;
    }

    public ImageSdk(Context context, Surface surface) {
        nativeImageSdk = new NativeImageSdk();
        mContext = context;
        mSurface = surface;
    }

    public void onCreate() {
        if (null == mContext) {
            throw new NullPointerException("context = null");
        }

        if (null == mSurface) {
            mPointer = nativeImageSdk.initSDK(mContext);
        } else {
            mPointer = nativeImageSdk.initRenderSDK(mContext, mSurface);
        }
    }

    public void onDestroy() {
        nativeImageSdk.freeSDK(mPointer);
    }

    public void setInputPath(String path) {
        nativeImageSdk.setInputPath(mPointer, path);
    }

    public void setOutputPath(String path) {
        nativeImageSdk.setOutputPath(mPointer, path);
    }

    public void setEffectCmd(String cmd) {
        nativeImageSdk.setEffectCmd(mPointer, cmd);
    }

    public void executeCmd(OnEditCompleteListener listener, Object param) {
        nativeImageSdk.executeCmd(mPointer, listener, param);
    }

    public void swapBuffer() {
        nativeImageSdk.swapBuffer(mPointer);
    }

    public void invalidate() {
        nativeImageSdk.invalidate(mPointer);
    }
}
