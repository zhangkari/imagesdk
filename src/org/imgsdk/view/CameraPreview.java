package org.imgsdk.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.imgsdk.core.ImageSdk;
import org.imgsdk.core.OnEditCompleteListener;

import java.io.IOException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by ws-kari on 15-11-14.
 */
public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {

    final static String TAG = "CameraPreview";

    private Camera mCamera;
    private RefreshThread mThread;

    private SurfaceHolder mHolder;

    public CameraPreview(Context context, Camera camera) {
        super(context);
        mCamera = camera;
        getHolder().addCallback(this);
        getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        mHolder = getHolder();
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        try {
            mCamera.setPreviewDisplay(surfaceHolder);
            mCamera.setPreviewCallback(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(byte[] bytes, Camera camera) {

                }
            });


            final Paint mPaint = new Paint();
            mPaint.setColor(Color.RED);

            mCamera.setFaceDetectionListener(new Camera.FaceDetectionListener() {
                @Override
                public void onFaceDetection(Camera.Face[] faces, Camera camera) {
                    if (null == faces || faces.length < 1) {
                        Log.d(TAG, "No face detect");
                    } else {
                        Log.d(TAG, "face num:" + faces.length);
                        mThread.addFaces(faces);
                    }
                }
            });

            mCamera.startFaceDetection();

        } catch (IOException e) {
            Log.d(TAG, "Error setting camera preview: " + e.getMessage());
        }

        mThread = new RefreshThread(surfaceHolder);
        mThread.start();

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        if (surfaceHolder.getSurface() == null) {
            return;
        }
        try {
            mCamera.stopPreview();
        } catch (Exception e) {
            // ignore: tried to stop a non-existent preview
        }
        try {
            mCamera.setPreviewDisplay(surfaceHolder);
            mCamera.startPreview();
        } catch (Exception e) {
            Log.d(TAG, "Error starting camera preview: " + e.getMessage());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        mThread.exit();
    }

    class RefreshThread extends Thread {
        private SurfaceHolder mHolder;
        private boolean mRunFlag;
        private Paint mPaint;
        private BlockingQueue<Camera.Face> mFaceQueue;

        public RefreshThread(SurfaceHolder holder) {
            mHolder = holder;
            mPaint = new Paint();
            mPaint.setColor(Color.RED);
            mPaint.setStyle(Paint.Style.FILL);
            mFaceQueue = new LinkedBlockingQueue<Camera.Face>(40);
        }

        public void start() {
            mRunFlag = true;
            super.run();
        }

        public void exit() {
            mRunFlag = false;
        }

        public void run() {
            Canvas c = null;
            Camera.Face face = null;

            while (mRunFlag) {
                try {
                    face = mFaceQueue.take();
                    synchronized (mHolder) {
                        c = mHolder.lockCanvas();
                        if (face != null) {
                            c.drawCircle(face.leftEye.x, face.leftEye.y, 40, mPaint);
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    if (c != null) {
                        mHolder.unlockCanvasAndPost(c);
                    }
                }

            } // while()

        } // run()

        public void addFaces(Camera.Face[] faces) {
            if (null != faces) {
                for (Camera.Face face : faces) {
                    mFaceQueue.offer(face);
                }
            }
        }

    }
}
