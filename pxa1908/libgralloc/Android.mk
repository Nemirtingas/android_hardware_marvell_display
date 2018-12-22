#
#  Copyright (C) 2016 Android For Marvell Project <ctx.xda@gmail.com>
#  Copyright 2006, The Android Open Source Project
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.  
#

LOCAL_PATH	:= $(call my-dir)

# Setting LOCAL_PATH will mess up all-subdir-makefiles, so do it beforehand.
#SAVE_MAKEFILES := $(call all-subdir-makefiles)

#
# gralloc.<property>.so
#

include $(LOCAL_PATH)/../common.mk

ifeq (0,1)
include $(CLEAR_VARS)

LOCAL_ADDITIONAL_DEPENDENCIES := $(common_deps) $(kernel_deps)
LOCAL_COPY_HEADERS_TO         := $(common_header_export_path)

LOCAL_SRC_FILES := \
    libstock.cpp \
	gc_gralloc_fb.cpp \
	gc_gralloc_alloc.cpp \
    gc_gralloc_map.cpp \
	gralloc.cpp

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := $(common_libs) libbinder libmvmem libGAL
LOCAL_C_INCLUDES       := $(common_includes) $(kernel_includes) device/samsung/pxa1908-common/mvmem

# See hardware/libhardware/modules/README.android to see how this is named.

LOCAL_MODULE := gralloc.mrvl

# With front buffer rendering, gralloc always provides the same buffer 
# when GRALLOC_USAGE_HW_FB. Obviously there is no synchronization with the display.
# Can be used to test non-VSYNC-locked rendering.
LOCAL_CFLAGS := \
	-DLOG_TAG=\"v_gralloc\" \
	-DDISABLE_FRONT_BUFFER \
    -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
	-DFRAMEBUFFER_PIXEL_FORMAT=$(FRAMEBUFFER_PIXEL_FORMAT)

LOCAL_CFLAGS += $(common_flags) -DUSE_ION 

include $(BUILD_SHARED_LIBRARY)
endif

# We need stock library till we reverse the last functionnality
include $(CLEAR_VARS)
LOCAL_SRC_FILES := gralloc.stock.so

LOCAL_MODULE := gralloc.stock
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_TAG := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := gralloc.mrvl.so

LOCAL_MODULE := gralloc.mrvl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_TAG := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
include $(BUILD_PREBUILT)
