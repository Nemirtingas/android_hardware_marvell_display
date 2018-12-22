# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH:= $(call my-dir)
# HAL module implemenation, not prelinked and stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.board.platform>.so



include $(CLEAR_VARS)
LOCAL_SRC_FILES := libHWComposerGC.so
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := hardware/libhardware/include
LOCAL_CFLAGS := -DENABLE_HWC_GC_PATH -DINTEGRATED_WITH_MARVELL -g

LOCAL_SHARED_LIBRARIES := $(common_libs) \
        libEGL \
        libbinder \
        libGAL \
        libsync

LOCAL_MODULE := libHWComposerGC
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_TAG := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_PRELINK_MODULE := false
include $(BUILD_PREBUILT)
