package org.imgsdk.core;

import android.content.Context;

public class NativeImageSdk {
    public native static long initSDK(Context context);
    public native static void freeSDK(long pointer);
    public native static void setEffectCmd(long pointer, String cmd);
    public native static void executeCmd(long pointer);
}