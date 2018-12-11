/****************************************************************************
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
*****************************************************************************/

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

static int gralloc_alloc_framebuffer(alloc_device_t * Dev, int Width, int Height,
    int Format, int Usage, buffer_handle_t * Handle, int * Stride);

static int gralloc_free_framebuffer(alloc_device_t * Dev, buffer_handle_t handle);

static int gralloc_device_open(const hw_module_t* module, const char* name,
    hw_device_t** device);

static int gralloc_register_buffer(gralloc_module_t const* module,
    buffer_handle_t handle);

static int gralloc_unregister_buffer(gralloc_module_t const* module,
    buffer_handle_t handle);

static int gralloc_lock(gralloc_module_t const* module,
    buffer_handle_t handle, int usage,
    int l, int t, int w, int h,
    void** vaddr);

static int gralloc_lock_ycbcr(gralloc_module_t const* module, buffer_handle_t handle,
    int usage, int l, int t, int w, int h, android_ycbcr *ycbcr );

static int gralloc_unlock(gralloc_module_t const* module,
    buffer_handle_t handle);

static int gralloc_perform(struct gralloc_module_t const* module,
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
            author: "Vivante Corporation",
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

static int gralloc_alloc(struct alloc_device_t* dev,
    int w, int h, int format, int usage,
    buffer_handle_t* handle, int* stride)
{
    int rel;

    if( gcoOS_ModuleConstructor() )
    {
        ALOGE("failed to module construct!");
        rel = -EINVAL;
    }
    else if( usage & 0x1000 )
    {
        rel = gralloc_alloc_framebuffer(dev, w, h, format, usage, handle, stride);
    }
    else
    {
        rel = gc_gralloc_alloc(dev, w, h, format, usage, handle, stride);
    }

    return rel;
}

static int gralloc_free(struct alloc_device_t* dev,
    buffer_handle_t handle)

{
    int rel;
    private_handle_t *hnd = (private_handle_t*)handle;
    if( hnd->flags & 1 )
        rel = gralloc_free_framebuffer(dev, handle);
    else
        rel = gc_gralloc_free(dev, handle);

    return rel;
}

static int gralloc_close(struct hw_device_t * dev)
{
    gralloc_context_t * ctx =
        reinterpret_cast<gralloc_context_t *>(dev);

    if (ctx != NULL)
    {
        free(ctx);
    }

    return 0;
}

int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    int status = -EINVAL;

    if (!strcmp(name, GRALLOC_HARDWARE_GPU0))
    {
        /* Open alloc device. */
        gralloc_context_t * dev;
        dev = (gralloc_context_t *) malloc(sizeof (*dev));

        /* Initialize our state here */
        memset(dev, 0, sizeof (*dev));

        /* initialize the procs */
        dev->device.common.tag     = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module  = const_cast<hw_module_t *>(module);
        dev->device.common.close   = gralloc_close;
        dev->device.alloc          = gralloc_alloc;
        dev->device.free           = gralloc_free;

        *device = &dev->device.common;
        status = 0;
    }
    else
    {
        /* Open framebuffer device. */
        status = fb_device_open(module, name, device);
    }

    return status;
}


/*****************************************************************************/

int gralloc_alloc_framebuffer(alloc_device_t *device, int w, int h, int format, int usage, buffer_handle_t* pHandle, int *pStride)
{
    int numBytes;
    int lineSize;

    if( pHandle == NULL || pStride == NULL )
        return -EINVAL;

    switch( format )
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            numBytes = 4; break;
        case HAL_PIXEL_FORMAT_RGB_888:
            numBytes = 3; break;
        case HAL_PIXEL_FORMAT_RGB_565:
            numBytes = 2; break;
        default: return -EINVAL;
    }

    private_module_t *m = (private_module_t*)device->common.module;
    lineSize = _ALIGN(numBytes*w, 4);
    pthread_mutex_lock(&m->lock);

    if (m->framebuffer == NULL) {
        // initialize the framebuffer, the framebuffer is mapped once
        // and forever.
        int err = mapFrameBufferLocked(m);
        if (err < 0) {
            return err;
        }
    }

    const uint32_t bufferMask = m->bufferMask;
    const uint32_t numBuffers = m->numBuffers;
    const size_t bufferSize = m->finfo.line_length * (m->info.yres_virtual / numBuffers);
    if (bufferMask >= ((1LU<<numBuffers)-1)) {
        // We ran out of buffers.
        pthread_mutex_unlock(&m->lock);
        return -ENOMEM;
    }

    // create a "fake" handles for it
    intptr_t vaddr = intptr_t(m->framebuffer->base);
    private_handle_t* hnd = new private_handle_t(dup(m->framebuffer->fd), h*lineSize,
            private_handle_t::PRIV_FLAGS_FRAMEBUFFER);

    // find a free slot
    for (uint32_t i=0 ; i<numBuffers ; i++) {
        if ((bufferMask & (1LU<<i)) == 0) {
            m->bufferMask |= (1LU<<i);
            break;
        }
        vaddr += bufferSize;
    }

    hnd->base = vaddr;
    hnd->offset = vaddr - intptr_t(m->framebuffer->base);
    *pHandle = hnd;

    pthread_mutex_unlock(&m->lock);

    *pStride = lineSize / numBytes;
    *pStride = m->info.xres_virtual;

    if( gc_gralloc_wrap(*pHandle, w, h, ::format, *pStride, hnd->offset + m->finfo.smem_start, (void*)hnd->base) )
    {
        ALOGE("%s: failed to wrap", "gralloc_alloc_framebuffer");
    }

    return 0;
}

int gralloc_free_framebuffer(alloc_device_t *device, buffer_handle_t handle)
{
    private_module_t *module = (private_module_t*)device->common.module;
    private_handle_t *hnd = (private_handle_t*)handle;

    if( private_handle_t::validate(handle) )
        return -EINVAL;

    module->bufferMask &= ~(1 <<
                (hnd->base - module->framebuffer->base) /
                (module->finfo.line_length * module->info.yres)
                );

    gc_gralloc_unwrap(hnd);
    close(hnd->fd);
    delete hnd;

    return 0;
}

int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    private_handle_t *hnd = (private_handle_t*)handle;
    int rel;

    if( hnd->flags & 1 )
        rel = -EINVAL;
    else
        rel = gc_gralloc_register_buffer(module, handle);

    return rel;
}

int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    private_handle_t *hnd = (private_handle_t*)handle;
    int rel;

    if( hnd->flags & 1 )
        rel = -EINVAL;
    else
        rel = gc_gralloc_unregister_buffer(module, handle);

    return rel;
}

int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr)
{
    private_handle_t *hnd = (private_handle_t*)handle;
    int rel;

    if( (hnd->flags & 1) == 0 )
        return gc_gralloc_lock(module, handle, usage, l, t, w, h, vaddr);

    if( vaddr )
        *vaddr = (void*)hnd->base;

    return 0;
}

int gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    private_handle_t *hnd = (private_handle_t*)handle;
    int rel;

    if( hnd->flags & 1 )
        rel = 0;
    else
        rel = gc_gralloc_unlock(module, handle);

    return rel;
}

// https://android.googlesource.com/platform/system/core/+/master/libsystem/include/system/graphics.h for struct android_ycbcr*
int gralloc_lock_ycbcr(gralloc_module_t const* module, buffer_handle_t handle, int usage, int l, int t, int w, int h, android_ycbcr *ycbcr )
{
    return gc_gralloc_lock_ycbcr(module, handle, usage, l, t, w, h, ycbcr);
}

int gralloc_perform(struct gralloc_module_t const* module,
        int operation, ... )
{
    return 0;
}

