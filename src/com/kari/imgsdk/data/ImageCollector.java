package com.kari.imgsdk.data;

import android.content.Context;
import android.database.Cursor;
import android.provider.MediaStore;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by ws-kari on 16-1-7.
 */
public class ImageCollector {
    final static String TAG = "ImageCollector";
    private static ImageCollector sInstance;
    private Context context;

    private ImageCollector(Context context) {
        this.context = context;
    }

    public synchronized static void init(Context context) {
        if (sInstance == null) {
            sInstance = new ImageCollector(context);
        }
    }

    public static ImageCollector getInstance() {
        return sInstance;
    }

    public void collectImage(final OnCollectedListener listener) {
        if (null == listener) {
            throw new NullPointerException("listener = null");
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                final String[] columns = {MediaStore.Images.Media.DATA, MediaStore.Images.Media._ID};
                final String orderBy = MediaStore.Images.Media.DATE_TAKEN;
                Cursor cursor = context.getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, columns, null, null, orderBy + " DESC");
                if (cursor == null) {
                    listener.onError("cursor is null");
                    return;
                }

                List<String> list = new ArrayList<String>(cursor.getColumnCount());
                cursor.moveToFirst();
                while (cursor.moveToNext()) {
                    int idx = cursor.getColumnIndex(MediaStore.Images.Media.DATA);
                    String path = cursor.getString(idx);
                    list.add(path);
                }
                cursor.close();

                listener.onComplete(list);
            }
        }).start();
    }

    public interface OnCollectedListener {
        void onComplete(List<String> images);

        void onError(String message);
    }
}
