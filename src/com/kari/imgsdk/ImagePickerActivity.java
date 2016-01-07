package com.kari.imgsdk;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;

import com.kari.imgsdk.adapter.ImagePickerAdapter;
import com.kari.imgsdk.config.EventCode;
import com.kari.imgsdk.data.ImageCollector;

import java.lang.ref.WeakReference;
import java.util.List;

public class ImagePickerActivity extends Activity implements ImageCollector.OnCollectedListener {

    final static String TAG = "ImagePickerActivity";

    private RecyclerView mRecyclerView;
    private ImagePickerAdapter mAdapter;
    private MyHandler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_image_picker);
        mHandler = new MyHandler(this);
        findViews();
        ImageCollector.init(this);
        ImageCollector.getInstance().collectImage(this);
    }

    private void findViews() {
        mRecyclerView = (RecyclerView) findViewById(R.id.picker_recycler_view);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mHandler = null;
    }

    @Override
    public void onComplete(List<String> images) {
        Log.d(TAG, "onComplete() images size = " + images.size());
        mAdapter = new ImagePickerAdapter(this, images);
        mHandler.sendEmptyMessage(EventCode.EVENT_COLLECT_IMAGE_OK);
    }

    @Override
    public void onError(String message) {
        Log.d(TAG, "onError() " + message);
    }

    static class MyHandler extends Handler {
        WeakReference<ImagePickerActivity> mRef;

        public MyHandler(ImagePickerActivity activity) {
            mRef = new WeakReference<ImagePickerActivity>(activity);
        }

        @Override
        public void handleMessage(Message message) {
            ImagePickerActivity activity = mRef.get();
            if (null == activity) {
                Log.e(TAG, "mRef.get() return null in handleMessage");
            } else {
                switch (message.what) {
                    case EventCode.EVENT_COLLECT_IMAGE_OK:
                        activity.mRecyclerView.setAdapter(activity.mAdapter);
                        activity.mRecyclerView.setLayoutManager(new GridLayoutManager(activity, 4));
                        break;

                }
            }

            super.handleMessage(message);
        }
    }
}
