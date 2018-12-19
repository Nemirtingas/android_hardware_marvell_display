LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libGAL.so
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE := libGAL
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_TAG := optional
LOCAL_MODULE_PATH := system/lib
LOCAL_PRELINK_MODULE := false
include $(BUILD_PREBUILT)
