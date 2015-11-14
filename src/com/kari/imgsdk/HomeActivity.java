package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import org.imgsdk.core.ImageSdk;

public class HomeActivity extends Activity implements View.OnClickListener {
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
        mBtnLoad = (Button) findViewById(R.id.home_btn_load);
        mBtnGo = (Button) findViewById(R.id.home_btn_go);
        mImageView = (ImageView) findViewById(R.id.home_iv_center);
    }

    private void setListeners() {
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
            case R.id.home_btn_load:
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
                break;

            case R.id.home_btn_go:
                mImageSdk.onDestroy();
                startActivity(new Intent(this, RenderActivity.class));
                break;
        }
    }
}
