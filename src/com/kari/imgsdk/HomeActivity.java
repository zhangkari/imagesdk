package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import com.nostra13.universalimageloader.core.ImageLoader;

import org.imgsdk.core.ImageSdk;
import org.imgsdk.core.OnEditCompleteListener;

public class HomeActivity extends Activity implements View.OnClickListener, OnEditCompleteListener {

    final static String TAG = "HomeActivity";

    final static int REQUEST_PICK_PATH = 0x4000;

    private View mBtnPicker;
    private View mBtnLoad;
    private View mBtnGo;

    private ImageView mImageView;
    private ImageSdk mImageSdk;

    private String mInputPath;

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
        mBtnPicker = findViewById(R.id.home_btn_pick);
        mBtnLoad = findViewById(R.id.home_btn_load);
        mBtnLoad.setVisibility(View.GONE);
        mBtnGo = findViewById(R.id.home_btn_go);
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
                startActivityForResult(new Intent(this, ImagePickerActivity.class), REQUEST_PICK_PATH);
                break;

            case R.id.home_btn_load:
                if (mInputPath == null) {
                    Toast.makeText(this, "请先选择图片", Toast.LENGTH_SHORT).show();
                    return;
                }
                mImageSdk.setInputPath(mInputPath);
                mImageSdk.setOutputPath("/sdcard/output.jpg");
                mImageSdk.setEffectCmd("{\"effect\":\"Rotate\",\"degree\":90}");
                mImageSdk.executeCmd(this, null);
                break;

            case R.id.home_btn_go:
                if (null == mInputPath) {
                    Toast.makeText(this, "请先选择图片", Toast.LENGTH_SHORT).show();
                    return;
                }
                startActivity(new Intent(this, RenderActivity.class).putExtra("path", mInputPath));
                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_PICK_PATH && resultCode == RESULT_OK) {
            String path = data.getStringExtra("path");
            ImageLoader.getInstance().displayImage("file://" + path, mImageView);
            mImageView.setVisibility(View.VISIBLE);
            mBtnLoad.setVisibility(View.VISIBLE);
            mInputPath = path;
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
