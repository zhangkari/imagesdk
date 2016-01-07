package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import org.imgsdk.core.ImageSdk;
import org.imgsdk.core.OnEditCompleteListener;

public class HomeActivity extends Activity implements View.OnClickListener, OnEditCompleteListener {

    final static String TAG = "HomeActivity";

    private Button mBtnPicker;
    private Button mBtnLoad;
    private Button mBtnGo;
    private ImageView mImageView;
    private ImageSdk mImageSdk;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        findViews();
        setListeners();
        mImageSdk = new ImageSdk(this);
        mImageSdk.onCreate();
    }

    private void findViews() {
        mBtnPicker = (Button) findViewById(R.id.home_btn_pick);
        mBtnLoad = (Button) findViewById(R.id.home_btn_load);
        mBtnGo = (Button) findViewById(R.id.home_btn_go);
        mImageView = (ImageView) findViewById(R.id.home_iv_center);
    }

    private void setListeners() {
        mBtnPicker.setOnClickListener(this);
        mBtnLoad.setOnClickListener(this);
        mBtnGo.setOnClickListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mImageSdk.onDestroy();
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.home_btn_pick:
                mImageSdk.onDestroy();
                startActivity(new Intent(this, ImagePickerActivity.class));
                break;

            case R.id.home_btn_load:
                if (mImageView.getVisibility() == View.GONE) {
                    mImageView.setImageResource(R.drawable.bg);
                    mImageView.setVisibility(View.VISIBLE);
                    mImageSdk.setEffectCmd("cmd = zoom-in | value = 1.2");
                    mImageSdk.executeCmd(this, null);
                } else {
                    mImageView.setVisibility(View.GONE);
                    mImageSdk.setEffectCmd("cmd=zoom-out | value = 0.8f ");
                    mImageSdk.executeCmd(this, null);
                }
                break;

            case R.id.home_btn_go:
                mImageSdk.onDestroy();
                startActivity(new Intent(this, RenderActivity.class));
                break;
        }
    }

    @Override
    public void onSuccess(String path, Object param) {
        Log.d(TAG, "onSuccess(): path = " + path);
    }

    @Override
    public void onError(Throwable error, Object param) {
        Log.d(TAG, "oError(): error = " + error.getMessage());
    }
}
