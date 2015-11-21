LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE 	:= libjson
LOCAL_SRC_FILES := cJSON.c 
include $(BUILD_STATIC_LIBRARY)
