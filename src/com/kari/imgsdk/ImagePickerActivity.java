package com.kari.imgsdk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;

import com.kari.imgsdk.adapter.ImagePickerAdapter;
import com.kari.imgsdk.adapter.PickerItemDecoration;
import com.kari.imgsdk.config.EventCode;
import com.kari.imgsdk.data.ImageCollector;

import java.lang.ref.WeakReference;
import java.util.List;

public class ImagePickerActivity extends Activity implements ImageCollector.OnCollectedListener {

    final static String TAG = "ImagePickerActivity";

    private ImagePickerAdapter mAdapter;
    private MyHandler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_picker);
        mHandler = new MyHandler(this);
        findViews();
        ImageCollector.init(this);
        ImageCollector.getInstance().collectImage(this);
    }

    private void findViews() {
        RecyclerView recyclerView = (RecyclerView) findViewById(R.id.picker_recycler_view);
        mAdapter = new ImagePickerAdapter(this);
        String path = "drawable://" + R.drawable.camera;
        Message msg = new Message();
        msg.what = EventCode.EVENT_ADD_DEFAULT_ICON;
        msg.obj = path;
        mHandler.sendMessage(msg);
        mAdapter.setItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                String path = mAdapter.getDataSource().get(i);
                Log.d(TAG, "path=" + path);
                if (i == 0) {
                    startActivity(new Intent(ImagePickerActivity.this, CameraActivity.class));
                    finish();
                } else {
                    Intent intent = new Intent();
                    path = path.replace("file://", "");
                    Log.d(TAG, "path=" + path);
                    intent.putExtra("path", path);
                    setResult(RESULT_OK, intent);
                    finish();
                }
            }
        });
        recyclerView.setAdapter(mAdapter);
        recyclerView.setLayoutManager(new GridLayoutManager(this, 3));
        recyclerView.setItemAnimator(new DefaultItemAnimator());
        recyclerView.addItemDecoration(new PickerItemDecoration(this));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        ImageCollector.destroy();
        mHandler = null;
    }

    @Override
    public void onProgress(String path) {
        Message message = Message.obtain();
        message.what = EventCode.EVENT_COLLECT_IMAGE_PROGRESS;
        message.obj = path;
        mHandler.sendMessage(message);
    }

    @Override
    public void onComplete(List<String> images) {
        Log.d(TAG, "onComplete() images size = " + images.size());
        mHandler.sendEmptyMessage(EventCode.EVENT_COLLECT_IMAGE_COMPLETE);
    }

    @Override
    public void onError(String message) {
        Log.d(TAG, "onError() " + message);
        mHandler.sendEmptyMessage(EventCode.EVENT_COLLECT_IMAGE_ERROR);
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
                    case EventCode.EVENT_COLLECT_IMAGE_COMPLETE:
                        break;

                    case EventCode.EVENT_COLLECT_IMAGE_ERROR:
                        break;

                    case EventCode.EVENT_COLLECT_IMAGE_PROGRESS:
                        String path = (String) message.obj;
                        activity.mAdapter.getDataSource().add("file://" + path);
                        int lastIndex = activity.mAdapter.getItemCount() - 1;
                        activity.mAdapter.notifyItemChanged(lastIndex);
                        break;

                    case EventCode.EVENT_ADD_DEFAULT_ICON:
                        activity.mAdapter.getDataSource().add((String)(message.obj));
                        int index = activity.mAdapter.getItemCount() - 1;
                        activity.mAdapter.notifyItemChanged(index);
                        break;
                }
            }

            super.handleMessage(message);
        }
    }
}
