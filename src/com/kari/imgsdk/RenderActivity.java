package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import org.imgsdk.core.OnEditCompleteListener;
import org.imgsdk.view.RenderSurfaceView;

public class RenderActivity extends Activity {
    final String TAG = "RenderActivity";

    private RenderSurfaceView mSurfaceView;
    private View mSwitchView;
    private View mBackView;

    private boolean hasSetInputPath;

    private String mInputPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_render);
        getInputPath(savedInstanceState);
        findViews();
        setListeners();

        Log.d(TAG, "thread id:" + Thread.currentThread().getId());
    }

    private boolean getInputPath(Bundle bundle) {
        Intent intent = getIntent();
        if (intent == null) {
            Log.e(TAG, "getIntent() return null");
            return false;
        }

        mInputPath = intent.getStringExtra("path");
        if (mInputPath == null) {
            Log.e(TAG, "getIntent() return null");
            return false;
        }

        return true;
    }

    private void findViews() {
        mSurfaceView = (RenderSurfaceView) findViewById(R.id.render_surface_view);
        mSwitchView = findViewById(R.id.render_switch_view);
        mBackView = findViewById(R.id.render_back_view);
    }

    private void setListeners() {
        mSwitchView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (!hasSetInputPath) {
                    mSurfaceView.getRenderer().setInputPath(mInputPath);
                    mSurfaceView.getRenderer().setOutputPath("/sdcard/output.jpg");
                    hasSetInputPath = true;
                }
                Log.d(TAG, "thread id:" + Thread.currentThread().getId());
                mSurfaceView.getRenderer().setEffectCmd("{\"effect\":\"Normal\"}");
                mSurfaceView.getRenderer().executeCmd(mCompleteListener, null);
                mSurfaceView.invalidate();
            }
        });

        mBackView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                finish();
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
