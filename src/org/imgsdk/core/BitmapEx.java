package org.imgsdk.core;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

/**
 * Created by ws-kari on 15-12-10.
 */
public final class BitmapEx {

    private Bitmap bitmap;
    private long mPointer;

    public BitmapEx() {
        bitmap = null;
        mPointer = 0;
    }

    public Bitmap getBitmap() {
        return bitmap;
    }

    public void create(int w, int h) {
        if (w < 1 || h < 1 || w > 4096 || h > 4096) {
            throw new IllegalArgumentException("width and height must between 1 and 4096");
        }

        int size = w * h * 4;
        mPointer = allocate(size);
        if (0 == mPointer) {
            throw new RuntimeException("Failed allocate native memory");
        }

        byte[] colors = convert2ByteArray(mPointer, size);
        bitmap = BitmapFactory.decodeByteArray(colors, 0, size);

    }

    public void release() {
        if (mPointer != 0) {
            release(mPointer);
        }

        if (null != bitmap) {
            bitmap.recycle();
            bitmap = null;
        }
    }

    static {
        System.loadLibrary("imgsdk");
    }

    // native methods
    private final native long allocate(int size);

    private final native byte[] convert2ByteArray(long pointer, int size);

    private final native void release(long pointer);
}
