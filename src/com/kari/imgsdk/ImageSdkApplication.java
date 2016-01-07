package com.kari.imgsdk;

import android.app.Application;
import android.graphics.Bitmap;
import android.util.Log;

import com.kari.imgsdk.config.CacheConfig;
import com.nostra13.universalimageloader.cache.disc.impl.UnlimitedDiskCache;
import com.nostra13.universalimageloader.cache.memory.impl.UsingFreqLimitedMemoryCache;
import com.nostra13.universalimageloader.core.DisplayImageOptions;
import com.nostra13.universalimageloader.core.ImageLoader;
import com.nostra13.universalimageloader.core.ImageLoaderConfiguration;
import com.nostra13.universalimageloader.core.assist.ImageScaleType;
import com.nostra13.universalimageloader.core.display.FadeInBitmapDisplayer;
import com.nostra13.universalimageloader.core.display.RoundedBitmapDisplayer;
import com.nostra13.universalimageloader.utils.StorageUtils;

import java.io.File;

/**
 * Created by ws-kari on 16-1-7.
 */
public class ImageSdkApplication extends Application {

    final static String TAG = "ImageSdkApplication";

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate()");
        super.onCreate();

        initImageLoader();
    }

    private void initImageLoader() {
        File cacheDir = StorageUtils.getOwnCacheDirectory(getApplicationContext(), CacheConfig.CACHE_PATH);

        DisplayImageOptions displayOptions = new DisplayImageOptions.Builder()
                .showImageOnLoading(R.drawable.progress)
                .showImageForEmptyUri(R.drawable.empty)
                .showImageOnFail(R.drawable.failure)
                .cacheInMemory(true)
                .cacheOnDisk(true)
                .considerExifParams(true)
                .imageScaleType(ImageScaleType.EXACTLY_STRETCHED)
                .bitmapConfig(Bitmap.Config.RGB_565)
                .resetViewBeforeLoading(true)
                .displayer(new RoundedBitmapDisplayer(20))
                .displayer(new FadeInBitmapDisplayer(500))
                .build();

        ImageLoaderConfiguration configuration = new ImageLoaderConfiguration.Builder(this)
                .diskCache(new UnlimitedDiskCache(cacheDir))
                .threadPoolSize(3)
                .threadPriority(Thread.NORM_PRIORITY - 2)
                .memoryCache(new UsingFreqLimitedMemoryCache(2 * 1024 * 1024))
                .memoryCacheSize(2 * 1024 * 1024)
                .diskCacheSize(64 * 1024 * 1024)
                .diskCacheFileCount(CacheConfig.CACHE_COUNT)
                .defaultDisplayImageOptions(displayOptions)
                .writeDebugLogs()
                .build();

        ImageLoader.getInstance().init(configuration);
    }

    @Override
    public void onTerminate() {
        Log.d(TAG, "onTerminate()");
        super.onTerminate();
    }
}
