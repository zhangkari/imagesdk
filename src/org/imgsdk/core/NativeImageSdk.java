package org.imgsdk.core;
public class NativeImageSdk {
    public native static long initSDK();
    public native static void freeSDK(long pointer);
    public native static void setEffectCmd(long pointer, String cmd);
    public native static void executeCmd(long pointer);
}