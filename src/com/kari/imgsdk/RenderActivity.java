package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.nostra13.universalimageloader.core.ImageLoader;

import org.imgsdk.core.OnEditCompleteListener;
import org.imgsdk.view.RenderSurfaceView;

public class RenderActivity extends Activity {
    final String TAG = "RenderActivity";

    private View mCenterLayout;
    private ImageView mCompareView;
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
        mCenterLayout = findViewById(R.id.render_center_layout);
        mCompareView = (ImageView) findViewById(R.id.render_compare_view);
        ImageLoader.getInstance().displayImage("file://" + mInputPath, mCompareView);
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
                    mSurfaceView.getRenderer().setEffectCmd("{\"effect\":\"Normal\"}");
                    mSurfaceView.getRenderer().executeCmd(mCompleteListener, null);
                    hasSetInputPath = true;
                }
                Log.d(TAG, "thread id:" + Thread.currentThread().getId());
                mSurfaceView.invalidate();
            }
        });

        mBackView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                finish();
            }
        });

        mCenterLayout.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                int action = motionEvent.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    mCompareView.setVisibility(View.VISIBLE);
                } else if (action == MotionEvent.ACTION_UP) {
                    mCompareView.setVisibility(View.GONE);
                }

                return true;
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
