/*
 * Copyright (C) 2016 The CyanogenMod Project
 *               2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HWC_GC_INTERFACE_H__
#define __HWC_GC_INTERFACE_H__
#include <stdlib.h>
#include <hardware/hwcomposer.h>

#ifdef INTEGRATED_WITH_MARVELL
    #define HWC_RENAME(x) x##_gc
    #define hwc_set             HWC_RENAME(hwc_set)
    #define hwc_prepare         HWC_RENAME(hwc_prepare)
    #define hwc_eventControl    HWC_RENAME(hwc_eventControl)
    #define hwc_blank           HWC_RENAME(hwc_blank)
    #define hwc_device_open     HWC_RENAME(hwc_device_open)
    #define hwc_device_close    HWC_RENAME(hwc_device_close)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int
hwc_prepare(
    hwc_composer_device_1_t * dev,
    size_t numDisplays,
    hwc_display_contents_1_t** displays
    );

int
hwc_set(
    hwc_composer_device_1_t * dev,
    size_t numDisplays,
    hwc_display_contents_1_t** displays
    );

int
hwc_eventControl(
    hwc_composer_device_1_t * dev,
    int disp,
    int event,
    int enabled
    );

int
hwc_blank(
    struct hwc_composer_device_1 * dev,
    int disp,
    int blank
    );

int
hwc_query(
    hwc_composer_device_1_t * dev,
    int what,
    int* value
    );

int
hwc_getDisplayConfigs(
    struct hwc_composer_device_1* dev,
    int disp,
    uint32_t* configs,
    size_t* numConfigs
    );

int
hwc_getDisplayAttributes(
    struct hwc_composer_device_1* dev,
    int disp,
    uint32_t config,
    const uint32_t* attributes,
    int32_t* values
    );

int
hwc_device_close(
    struct hw_device_t * dev
    );

int
hwc_device_open(
    const struct hw_module_t * module,
    const char * name,
    struct hw_device_t ** device
    );
#ifdef __cplusplus
}
#endif
#endif //__HWC_GC_INTERFACE_H__
