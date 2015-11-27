package org.imgsdk.core;

/**
 * Created by ws-kari on 15-11-26.
 */
public interface OnEditCompleteListener {
    void onSuccess(String path, Object param);

    void onError(Throwable error, Object param);
}
