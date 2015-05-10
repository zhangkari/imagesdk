LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := png.c \
                   pngset.c\
                   pngget.c\
                   pngrutil.c\
                   pngtrans.c\
                   pngwutil.c\
                   pngread.c\
                   pngrio.c\
                   pngwio.c\
                   pngwrite.c\
                   pngrtran.c\
                   pngwtran.c\
                   pngmem.c\
                   pngerror.c \
                   pngpread.c



LOCAL_MODULE := libpng
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
