package com.kari.imgsdk;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import org.imgsdk.core.OnEditCompleteListener;
import org.imgsdk.view.RenderSurfaceView;

public class RenderActivity extends Activity {
    final String TAG = "RenderActivity";
    private RenderSurfaceView mSurfaceView;
    private Button mBtnRun;
    private boolean hasSetInputPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_render);
        findViews();
        setListeners();

        Log.d(TAG, "thread id:" + Thread.currentThread().getId());
    }

    private void findViews() {
        mSurfaceView = (RenderSurfaceView) findViewById(R.id.render_surface_view);
        mBtnRun = (Button) findViewById(R.id.render_btn_run);
    }

    private void setListeners() {
        mBtnRun.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (!hasSetInputPath) {
                    mSurfaceView.getRenderer().setInputPath("/sdcard/input.jpg");
                    mSurfaceView.getRenderer().setOutputPath("/sdcard/output.jpg");
                    hasSetInputPath = true;
                }
                Log.d(TAG, "thread id:" + Thread.currentThread().getId());
                mSurfaceView.getRenderer().setEffectCmd("{\"effect\":\"Normal\"}");
                mSurfaceView.getRenderer().executeCmd(mCompleteListener, null);
                mSurfaceView.invalidate();
            }
        });
    }

    private OnEditCompleteListener mCompleteListener = new OnEditCompleteListener() {
        @Override
        public void onSuccess(String path, Object param) {
            Log.d(TAG, "onSuccess() path=" + path + ", thread id = " + Thread.currentThread().getId());
        }

        @Override
        public void onError(Throwable error, Object param) {
            Log.d(TAG, "onSuccess");
        }
    };

}
