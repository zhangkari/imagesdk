package com.kari.imgsdk;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import org.imgsdk.core.ImageSdk;

public class HomeActivity extends Activity implements View.OnClickListener {
    private Button mBtn;
    private ImageView mImageView;
    private ImageSdk mImageSdk;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        findViews();
        setListeners();
        mImageSdk = new ImageSdk();
        mImageSdk.onCreate(this);
    }

    private void findViews() {
        mBtn = (Button) findViewById(R.id.home_btn_foot);
        mImageView = (ImageView) findViewById(R.id.home_iv_center);
    }

    private void setListeners() {
        mBtn.setOnClickListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mImageSdk.onDestroy();
    }

    @Override
    public void onClick(View view) {
        if (mImageView.getVisibility() == View.GONE) {
            mImageView.setImageResource(R.drawable.bg);
            mImageView.setVisibility(View.VISIBLE);
            mImageSdk.setEffectCmd("cmd = zoom-in | value = 1.2");
            mImageSdk.executeCmd();
        } else {
            mImageView.setVisibility(View.GONE);
            mImageSdk.setEffectCmd("cmd=zoom-out | value = 0.8f ");
            mImageSdk.executeCmd();
        }
    }
}
