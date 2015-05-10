LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := libpng libz
LOCAL_SRC_FILES := imgsdk.c
LOCAL_MODULE := imgsdk
#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
