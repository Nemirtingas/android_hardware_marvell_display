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

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include "gralloc_priv.h"
#include "gc_gralloc_gr.h"


/*****************************************************************************/

struct gralloc_context_t
{
alloc_device_t device;
};

extern int format;

/*****************************************************************************/

extern int fb_device_open(const hw_module_t* module, const char* name,
    hw_device_t** device);

extern int gralloc_alloc_framebuffer(alloc_device_t * Dev, int Width, int Height,
    int Format, int Usage, buffer_handle_t * Handle, int * Stride);

extern int gralloc_free_framebuffer(alloc_device_t * Dev, buffer_handle_t handle);

extern int gralloc_device_open(const hw_module_t* module, const char* name,
    hw_device_t** device);

extern int gralloc_register_buffer(gralloc_module_t const* module,
    buffer_handle_t handle);

extern int gralloc_unregister_buffer(gralloc_module_t const* module,
    buffer_handle_t handle);

extern int gralloc_lock(gralloc_module_t const* module,
    buffer_handle_t handle, int usage,
    int l, int t, int w, int h,
    void** vaddr);

extern int gralloc_lock_ycbcr(gralloc_module_t const* module, buffer_handle_t handle,
    int usage, int l, int t, int w, int h, android_ycbcr *ycbcr );

extern int gralloc_unlock(gralloc_module_t const* module,
    buffer_handle_t handle);

extern int gralloc_perform(struct gralloc_module_t const* module,
    int operation, ... );


/*****************************************************************************/

static struct hw_module_methods_t gralloc_module_methods =
{
    open: gralloc_device_open
};

struct private_module_t HAL_MODULE_INFO_SYM =
{
    base:
    {
        common:
        {
            tag: HARDWARE_MODULE_TAG,
            version_major: 1,
            version_minor: 0,
            id: GRALLOC_HARDWARE_MODULE_ID,
            name: "Graphics Memory Allocator Module",
            author: "Nemirtingas (Maxime P)",
            methods: &gralloc_module_methods
        },

        registerBuffer: gralloc_register_buffer,
        unregisterBuffer: gralloc_unregister_buffer,
        lock: gralloc_lock,
        unlock: gralloc_unlock,
        perform: gralloc_perform,
        lock_ycbcr: gralloc_lock_ycbcr,
    },

    framebuffer: 0,
    flags: 0,
    numBuffers: 0,
    bufferMask: 0,
    lock: PTHREAD_MUTEX_INITIALIZER,
    currentBuffer: 0,
};


/*****************************************************************************/

extern int gralloc_alloc(struct alloc_device_t* dev,
    int w, int h, int format, int usage,
    buffer_handle_t* handle, int* stride)
{
    //log_func_entry;
    if ( gcoOS_ModuleConstructor() )
    {
        ALOGE("failed to module construct!");
        return -EINVAL;
    }

    if ( usage & GRALLOC_USAGE_HW_FB )
        return gralloc_alloc_framebuffer(dev, w, h, format, usage, handle, stride);

    return gc_gralloc_alloc(dev, w, h, format, usage, handle, stride);
}

extern int gralloc_free(struct alloc_device_t* dev,
    buffer_handle_t handle)

{
    //log_func_entry;

    private_handle_t *Handle = (private_handle_t*)handle;

    if ( Handle->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER )
        return gralloc_free_framebuffer(dev, handle);

    return gc_gralloc_free(dev, handle);
}

extern int gralloc_close(struct hw_device_t * dev)
{
    //log_func_entry;

    if( dev )
        free(dev);
    return 0;
}

extern int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    //log_func_entry;

    if (!strcmp(name, GRALLOC_HARDWARE_GPU0))
    {
        // Open alloc device.
        gralloc_context_t *dev = (gralloc_context_t *) malloc(sizeof (*dev));

        if( dev == NULL )
            return -ENOMEM;

        // Initialize our state here
        memset(dev, 0, sizeof (*dev));

        // initialize the procs
        dev->device.common.tag     = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module  = const_cast<hw_module_t *>(module);
        dev->device.common.close   = gralloc_close;
        dev->device.alloc          = gralloc_alloc;
        dev->device.free           = gralloc_free;

        *device = (hw_device_t*)dev;
        return 0;
    }

    if( !strncmp(name, GRALLOC_HARDWARE_FB0, 2) )
    {
        // Open framebuffer device.
        return fb_device_open(module, name, device);
    }

    ALOGE("Invalid device name");
    return -EINVAL;
}


/*****************************************************************************/

extern int gralloc_alloc_framebuffer(alloc_device_t *device, int w, int h, int format, int usage, buffer_handle_t* pHandle, int *pStride)
{
    //log_func_entry;

    private_module_t *m = (private_module_t*)device->common.module;
    private_handle_t *hnd;
    int sizeofPixel;
    int sizeofLine;
    int bufferSize;
    int vaddr;

    if( pHandle == NULL || pStride == NULL )
        return -EINVAL;

    switch ( format )
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            sizeofPixel = 4;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            sizeofPixel = 3;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            sizeofPixel = 2;
            break;
        default:
            return -EINVAL;
    }

    sizeofLine = _ALIGN(sizeofPixel * w, 4);
    pthread_mutex_lock(&m->lock);
    /* Allocate the framebuffer. */
    if (m->framebuffer == NULL)
    {
        /* Initialize the framebuffer, the framebuffer is mapped once
        ** and forever. */
        int err = mapFrameBufferLocked(m);
        if (err < 0)
        {
            pthread_mutex_unlock(&m->lock);
            return err;
        }
    }

    if (m->bufferMask >= ((1LU << m->numBuffers) - 1))
    {
        /* We ran out of buffers. */
        ALOGE("Out of buffers");
        return -ENOMEM;
    }

    bufferSize = m->finfo.line_length * (m->info.yres_virtual / m->numBuffers);

    hnd = new private_handle_t(dup(m->framebuffer->fd), h * sizeofLine, private_handle_t::PRIV_FLAGS_FRAMEBUFFER);

    vaddr = m->framebuffer->base;
    for( int i = 0; i < m->numBuffers; ++i )
    {
        if( !((m->bufferMask >> i) & 1) )
        {
            m->bufferMask |= 1<<i;
            break;
        }
        vaddr += bufferSize;
    }
    hnd->master = -1;
    hnd->base = vaddr;
    hnd->offset = vaddr - m->framebuffer->base;
    *pHandle = (buffer_handle_t)hnd;

    pthread_mutex_unlock(&m->lock);
    *pStride = sizeofLine / sizeofPixel;
    *pStride = m->info.xres_virtual;

    if( gc_gralloc_wrap(*pHandle, w, h, ::format, *pStride, hnd->offset + m->finfo.smem_start, (void*)hnd->base) )
        ALOGE("failed to wrap");

    return 0;
}

extern int gralloc_free_framebuffer(alloc_device_t *device, buffer_handle_t handle)
{
    //log_func_entry;

    private_handle_t *hnd = (private_handle_t*)handle;
    private_module_t *module = (private_module_t*)device->common.module;

    if( private_handle_t::validate(hnd) )
        return -EINVAL;

    module->bufferMask &= ~(1 << (hnd->base - module->framebuffer->base)
                            / (module->finfo.line_length
                             * module->info.yres ));

    gc_gralloc_unwrap(hnd);
    close(hnd->fd);
    delete hnd;
    return 0;
}

extern int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    //log_func_entry;

    private_handle_t *hnd = (private_handle_t*)handle;
    if( hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER )
        return -EINVAL;

    return gc_gralloc_register_buffer(module, handle);

}

extern int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    //log_func_entry;

    private_handle_t *hnd = (private_handle_t*)handle;
    if( hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER )
        return -EINVAL;

    return gc_gralloc_unregister_buffer(module, handle);
}

extern int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr)
{
    //log_func_entry;

    private_handle_t *hnd = (private_handle_t*)handle;
    if( !(hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) )
        return gc_gralloc_lock(module, handle, usage, l, t, w, h, vaddr);

    if( vaddr )
        *vaddr = (void*)hnd->base;

    return 0;
}

extern int gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    //log_func_entry;

    private_handle_t *hnd = (private_handle_t*)handle;

    if( hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER )
        return 0;

    return gc_gralloc_unlock(module, handle);
}

// https://android.googlesource.com/platform/system/core/+/master/libsystem/include/system/graphics.h for struct android_ycbcr*
extern int gralloc_lock_ycbcr(gralloc_module_t const* module, buffer_handle_t handle, int usage, int l, int t, int w, int h, android_ycbcr *ycbcr )
{
    //log_func_entry;

    return gc_gralloc_lock_ycbcr(module, handle, usage, l, t, w, h, ycbcr);
}

extern int gralloc_perform(struct gralloc_module_t const* module,
        int operation, ... )
{
    //log_func_entry;
    return 0;
}

