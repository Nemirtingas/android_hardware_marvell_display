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

#ifndef __HW_BASELAY_COMPOSER_H__
#define __HW_BASELAY_COMPOSER_H__
#include <string.h>
#include <hardware/hwcomposer.h>

namespace android {
/*
 * HWBaselayComposer is the wrapper class to forward HWComposer calling
 * to Vivante HWComposer to process Layers except Overlays.
 * Vivante HWComposer will composer layers into Baselay now.
 */
class HWBaselayComposer {
public:
    HWBaselayComposer();
    ~HWBaselayComposer();
    int open(const char * name, struct hw_device_t** device);
    int close(struct hw_device_t* dev);
    int prepare(hwc_composer_device_1_t *dev, size_t numDisplays, hwc_display_contents_1_t** displays);
    int set(hwc_composer_device_1_t *dev, size_t numDisplays, hwc_display_contents_1_t** displays);
    int blank(hwc_composer_device_1_t *dev, int disp, int blk);
private:
    hwc_composer_device_1_t*  mHwc;
};
};
#endif //__HW_BASELAY_MANAGER_H__
