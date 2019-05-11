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

#include "IDisplayEngine.h"
#include "FramebufferOverlay.h"
#include "V4L2Overlay.h"
#include "FakeOverlay.h"

namespace android{

enum OVERLAYDEVICETYPE{
    OVERLAY_DEVICE_UNKNOWN = 0,
    OVERLAY_DEVICE_FB      = 1,
    OVERLAY_DEVICE_V4L2    = 2,
    OVERLAY_DEVICE_FAKE    = 3,
};

#if 0
IDisplayEngine* CreateOverlayEngine(uint32_t idx, DMSInternalConfig const * dms_config)
{
    static char const * const *fb_overlay_device_template = dms_config->OVLY.fb_dev_name;
    static char const * const *v4l2_overlay_device_template = dms_config->OVLY.v4l2_dev_name;

    //  static uint32_t idx = 0;
    static OVERLAYDEVICETYPE tOvlyType = OVERLAY_DEVICE_UNKNOWN;

    if(OVERLAY_DEVICE_UNKNOWN == tOvlyType){
        int32_t fd = open(fb_overlay_device_template[idx], O_RDWR, 0);
        if(fd > 0){
            tOvlyType = OVERLAY_DEVICE_FB;
            close(fd);
        }else{
            tOvlyType = OVERLAY_DEVICE_V4L2;
        }
    }

    if(NULL == v4l2_overlay_device_template[idx]){
        tOvlyType = OVERLAY_DEVICE_FAKE;
    }

    IDisplayEngine* pOverlay = NULL;
    if(OVERLAY_DEVICE_V4L2 == tOvlyType){
        ALOGD("create v4l2 overlay! ---------- idx == %d ----------", idx);
        pOverlay = new V4L2OverlayRef(v4l2_overlay_device_template[idx]);
    }else if (OVERLAY_DEVICE_FB == tOvlyType){
        ALOGD("create fb overlay! ---------- idx == %d ----------", idx);
        pOverlay = new FBOverlayRef(fb_overlay_device_template[idx]);
    }else if (OVERLAY_DEVICE_FAKE == tOvlyType){
        ALOGD("create fake overlay! ---------- idx == %d ----------", idx);
        pOverlay = new FakeOverlayRef(NULL);
    }

    return pOverlay;
}
#endif

}
