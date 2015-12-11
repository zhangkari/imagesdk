package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import org.imgsdk.core.BitmapEx;

public class HomeActivity extends Activity implements View.OnClickListener {
    private Button mBtnLoad;
    private Button mBtnGo;
    private ImageView mImageView;
//    private ImageSdk mImageSdk;

    BitmapEx bitmapEx;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        findViews();
        setListeners();
//        mImageSdk = new ImageSdk(this);
//        mImageSdk.onCreate();
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
//        mImageSdk.onDestroy();

        if (null != bitmapEx) {
            bitmapEx.release();
            bitmapEx = null;
        }
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.home_btn_load:
                if (bitmapEx != null) {
                    bitmapEx.release();
                }

                bitmapEx = new BitmapEx();
                bitmapEx.create(2048, 2048);
                if (null != bitmapEx.getBitmap()) {
                    mImageView.setImageBitmap(bitmapEx.getBitmap());
                } else {
                    Toast.makeText(this, "bitmap = null", Toast.LENGTH_LONG).show();
                }
                break;

            case R.id.home_btn_go:
//                mImageSdk.onDestroy();
                startActivity(new Intent(this, RenderActivity.class));
                break;
        }
    }
}
