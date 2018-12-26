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
               
#include "libstock.h"

#include "gralloc_priv.h"

#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <linux/fb.h>
#include <cutils/log.h>

struct gralloc_context_t
{
alloc_device_t device;
};

libstock::libstock():_lib(dlopen("/system/lib/hw/gralloc.stock.so", RTLD_NOW)),gralloc_close(NULL), gralloc_alloc(NULL),gralloc_free(NULL)
{
    gralloc_context_t *dev;
    framebuffer_device_t *fbdev;

    dlsym(HMI, "HMI");

    gralloc_device_open       = HMI->base.common.methods->open;
    gralloc_register_buffer   = HMI->base.registerBuffer;
    gralloc_unregister_buffer = HMI->base.unregisterBuffer;
    gralloc_lock              = HMI->base.lock;
    gralloc_unlock            = HMI->base.unlock;
    gralloc_perform           = HMI->base.perform;
    gralloc_lock_ycbcr        = HMI->base.lock_ycbcr;

    dlsym(fb_device_open, "_Z14fb_device_openPK11hw_module_tPKcPP11hw_device_t");

    /*
    private_module_t module;

    module.framebuffer = (private_handle_t*)1;
    fb_device_open((hw_module_t*)&module, GRALLOC_HARDWARE_FB0, (hw_device_t**)&fbdev);

    fb_close = fbdev->common.close;
    fb_post = fbdev->post;
    fb_setSwapInterval = fbdev->setSwapInterval;

    fb_close((hw_device_t*)fbdev);
    */

    dlsym(gc_gralloc_alloc, "_Z16gc_gralloc_allocP14alloc_device_tiiiiPPK13native_handlePi");
    dlsym(gc_gralloc_wrap, "_Z15gc_gralloc_wrapP16private_handle_tiiiimPv");
    dlsym(gc_gralloc_lock_ycbcr, "_Z21gc_gralloc_lock_ycbcrPK16gralloc_module_tPK13native_handleiiiiiP13android_ycbcr");
    dlsym(gc_gralloc_lock, "_Z15gc_gralloc_lockPK16gralloc_module_tPK13native_handleiiiiiPPv");

    dlsym(gralloc_alloc_framebuffer, "_Z25gralloc_alloc_framebufferP14alloc_device_tiiiiPPK13native_handlePi");

    gralloc_device_open(0, "gpu0", (hw_device_t**)&dev);

    gralloc_close = dev->device.common.close;

    gralloc_alloc = dev->device.alloc;

    gralloc_free = dev->device.free;

    gralloc_close((hw_device_t*)dev);
}

libstock::~libstock()
{
    dlclose(_lib);
}

libstock& libstock::Inst()
{
    static libstock lib;
    return lib;
}
