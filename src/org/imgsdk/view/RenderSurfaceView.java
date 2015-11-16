package org.imgsdk.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.imgsdk.core.ImageSdk;

/**
 * Created by ws-kari on 15-11-14.
 */
public class RenderSurfaceView extends SurfaceView implements android.view.SurfaceHolder.Callback {

    private ImageSdk mImageSdk;

    public RenderSurfaceView(Context context) {
        super(context);
        getHolder().addCallback(this);
    }

    public RenderSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
    }

    public RenderSurfaceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        getHolder().addCallback(this);
    }

    @Override
    public void invalidate() {
        mImageSdk.executeCmd();
        super.invalidate();
    }

    public void setEffectCmd(String cmd) {
        mImageSdk.setEffectCmd(cmd);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
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
