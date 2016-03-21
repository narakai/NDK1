LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := com_example_leon_ndk1_Store

LOCAL_SRC_FILES := \
    com_example_leon_ndk1_Store.cpp \
    Store.cpp

include $(BUILD_SHARED_LIBRARY)
