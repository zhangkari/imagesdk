LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := libjpeg libpng
LOCAL_MODULE 	:= imgsdk

LOCAL_CFLAGS += -D_ANDROID_

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libpng-1.6.17 \
					$(LOCAL_PATH)/../jpeg-9a
					
LOCAL_SRC_FILES := imgsdk.c \
				   android_main.c \
				   NativeImageSdk.c \
				   jniHelper.c
				   					
# Must enable when BUILD_EXECUTABLE
# And disable when BUILD_SHARED_LIBRARY
#LOCAL_CFLAGS	+= -pie -fPIE

LOCAL_LDLIBS	:= -llog -lGLESv2 -lEGL -landroid -lz

# Must enable when BUILD_EXECUTABLE
# And disable when BUILD_SHARED_LIBRARY
#LOCAL_LDFLAGS   += -pie -fPIE

LOCAL_STATIC_LIBRARIES := android_native_app_glue

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)

$(call import-module, android/native_app_glue)
