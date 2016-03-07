package org.imgsdk.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.imgsdk.core.ImageSdk;

/**
 * Created by ws-kari on 15-11-14.
 */
public class RenderSurfaceView extends SurfaceView implements android.view.SurfaceHolder.Callback {
    final static String TAG = "RenderSurfaceView";
    private ImageSdk mImageSdk;

    public RenderSurfaceView(Context context) {
        super(context);
        init();
    }

    public RenderSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public RenderSurfaceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    private void init() {
        getHolder().addCallback(this);
        Log.d(TAG, "thread id:" + Thread.currentThread().getId());
    }

    public ImageSdk getRenderer() {
        return mImageSdk;
    }

    @Override
    public void invalidate() {
        super.invalidate();
        mImageSdk.swapBuffer();
        mImageSdk.invalidate();
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        Log.d(TAG, "surfaceCreated thread id:"+Thread.currentThread().getId());
        mImageSdk = new ImageSdk(getContext(), surfaceHolder.getSurface());
        mImageSdk.onCreate();
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        mImageSdk.onDestroy();
    }
}
