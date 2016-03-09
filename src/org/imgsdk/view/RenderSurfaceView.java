package org.imgsdk.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.imgsdk.core.ImageSdk;
import org.imgsdk.core.OnEditCompleteListener;

/**
 * Created by ws-kari on 15-11-14.
 */
public class RenderSurfaceView extends SurfaceView implements android.view.SurfaceHolder.Callback {
    final static String TAG = "RenderSurfaceView";
    private ImageSdk mImageSdk;
    private String mInputPath;
    private String mOutputPath;
    private String mEffectCmd;
    private Object mParam;
    private OnEditCompleteListener mListener;

    public RenderSurfaceView(Context context) {
        super(context);
    }

    public RenderSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public RenderSurfaceView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public void init() {
        getHolder().addCallback(this);
        Log.d(TAG, "thread id:" + Thread.currentThread().getId());
    }

    public void setInputPath(String path) {
        if (null == path) {
            throw new NullPointerException("path = null");
        }
        mInputPath = path;
    }

    public void setEffectCmd(String cmd) {
        if (null == cmd) {
            throw new NullPointerException("cmd = null");
        }
        mEffectCmd = cmd;
    }

    public void setOutputPath(String path) {
        if (null == path) {
            throw new NullPointerException("path = null");
        }
        mOutputPath = path;
    }

    public void executeCmd(OnEditCompleteListener listener, Object param) {
        if (null == listener) {
            throw new NullPointerException("listener = null");
        }

        mListener = listener;
        mParam = param;
    }

    @Override
    public void invalidate() {
        super.invalidate();
        if (null != mImageSdk) {
            mImageSdk.swapBuffer();
            mImageSdk.invalidate();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        Log.d(TAG, "surfaceCreated thread id:" + Thread.currentThread().getId());
        mImageSdk = new ImageSdk(getContext(), surfaceHolder.getSurface());

        new Thread(new Runnable() {
            @Override
            public void run() {
                mImageSdk.onCreate();
                mImageSdk.setInputPath(mInputPath);
                mImageSdk.setOutputPath(mOutputPath);
                mImageSdk.setEffectCmd(mEffectCmd);
                mImageSdk.executeCmd(mListener, mParam);
            }
        }).start();

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        mImageSdk.onDestroy();
    }
}
