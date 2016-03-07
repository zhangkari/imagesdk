package org.imgsdk.core;

import android.content.Context;
import android.view.Surface;

public final class NativeImageSdk {
    public native long initSDK(Context context);
    public native long initRenderSDK(Context context, Surface surface);
    public native void freeSDK(long pointer);
    public native void swapBuffer(long pointer);
    public native void invalidate(long pointer);
    public native void setInputPath(long pointer, String path);
    public native void setOutputPath(long pointer, String path);
    public native void setEffectCmd(long pointer, String cmd);
    public native void executeCmd(long pointer, OnEditCompleteListener callback, Object param);
}