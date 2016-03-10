package com.kari.imgsdk;

import android.app.Activity;
import android.hardware.Camera;
import android.media.FaceDetector;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import org.imgsdk.view.CameraPreview;

public class CameraActivity extends Activity {

    final static String TAG = "CameraActivity";

    private View mSwitchView;
    private View mBackView;

    private Camera mCamera;
    private CameraPreview mCameraPreview;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /*
        setContentView(R.layout.activity_camera);
        findViews();
        setListeners();
        */

        prepareCamera();
    }

    private void findViews() {
        mSwitchView = findViewById(R.id.camera_switch_view);
        mBackView = findViewById(R.id.camera_back_view);
    }

    private void setListeners() {
        mSwitchView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

        mBackView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });
    }

    private void prepareCamera() {
        int cameraId = -1;
        int num = Camera.getNumberOfCameras();
        if (num > 0) {
            cameraId = 0;
        }
        for (int i = 0; i < num; ++i) {
            Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
            Camera.getCameraInfo(i, cameraInfo);
            if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                cameraId = i;
                break;
            }
        }

        if (cameraId < 0) {
            Toast.makeText(this, "No camera found", Toast.LENGTH_SHORT).show();
            return;
        }

        mCamera = Camera.open(cameraId);
        mCamera.setDisplayOrientation(90);

        mCameraPreview = new CameraPreview(this, mCamera);
        setContentView(mCameraPreview);
    }

    @Override
    public void onResume() {
        super.onResume();
        mCamera.startPreview();
    }

    @Override
    public void onPause() {
        super.onPause();

        if (null != mCamera) {
            mCamera.stopPreview();
            mCamera.stopFaceDetection();
        }

        if (null != mCamera) {
            mCamera.release();
        }
    }
}
