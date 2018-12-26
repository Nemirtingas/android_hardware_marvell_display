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

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <binder/IPCThreadState.h>

#include "gc_gralloc_gr.h"
#include "gralloc_priv.h"
#include "mrvl_pxl_formats.h"

//#include <gc_hal_user.h>
//#include <gc_hal_base.h>

#if HAVE_ANDROID_OS
#if (MRVL_VIDEO_MEMORY_USE_TYPE == gcdMEM_TYPE_ION)
#include <linux/ion.h>
#include <linux/pxa_ion.h>
#else
#include <linux/android_pmem.h>
#endif
#endif

#if MRVL_SUPPORT_DISPLAY_MODEL
#include <dms_if.h>
extern android::sp<android::IDisplayModel> displayModel;
#endif

using namespace android;

extern struct {
    int android_format;
    gceSURF_FORMAT hal_format;
} HAL_PIXEL_FORMAT_TABLE[] =
{
    { HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, gcvSURF_A8B8G8R8},
    { HAL_PIXEL_FORMAT_RGBA_8888             , gcvSURF_A8B8G8R8},
    { HAL_PIXEL_FORMAT_RGBX_8888             , gcvSURF_X8B8G8R8},
    { HAL_PIXEL_FORMAT_RGB_888               , gcvSURF_UNKNOWN },
    { HAL_PIXEL_FORMAT_RGB_565               , gcvSURF_R5G6B5  },
    { HAL_PIXEL_FORMAT_BGRA_8888             , gcvSURF_A8R8G8B8},
    { HAL_PIXEL_FORMAT_YCbCr_420_888         , gcvSURF_NV12    },
    { HAL_PIXEL_FORMAT_YV12                  , gcvSURF_YV12    }, // YCrCb 4:2:0 Planar
    { HAL_PIXEL_FORMAT_YCbCr_422_SP          , gcvSURF_NV16    }, // NV16
    { HAL_PIXEL_FORMAT_YCrCb_420_SP          , gcvSURF_NV21    }, // NV21
    { HAL_PIXEL_FORMAT_YCbCr_420_P           , gcvSURF_I420    },
    { HAL_PIXEL_FORMAT_YCbCr_422_I           , gcvSURF_YUY2    }, // YUY2
    { HAL_PIXEL_FORMAT_CbYCrY_422_I          , gcvSURF_UYVY    },
    { HAL_PIXEL_FORMAT_YCbCr_420_SP_MRVL     , gcvSURF_NV12    },
    { 0x00                                   , gcvSURF_UNKNOWN },
};

static int ion_alloc_buffer()
{

}

/*******************************************************************************
**
**  _ConvertAndroid2HALFormat
**
**  Convert android pixel format to Vivante HAL pixel format.
**
**  INPUT:
**
**      int Format
**          Specified android pixel format.
**
**  OUTPUT:
**
**      gceSURF_FORMAT HalFormat
**          Pointer to hold hal pixel format.
*/
gceSTATUS
_ConvertAndroid2HALFormat(
    int Format,
    gceSURF_FORMAT * HalFormat
    )
{
    *HalFormat = gcvSURF_UNKNOWN;
    for( int i = 0; ; ++i )
    {
        if( HAL_PIXEL_FORMAT_TABLE[i].android_format == 0 )
        {
            ALOGE("Unknown ANDROID format: %d", format);
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if( Format == HAL_PIXEL_FORMAT_TABLE[i].android_format )
        {
            *HalFormat = HAL_PIXEL_FORMAT_TABLE[i].hal_format;
            if( *HalFormat == gcvSURF_UNKNOWN )
            {
                ALOGE("Not supported ANDROID format: %d", format);
                return gcvSTATUS_INVALID_ARGUMENT;
            }
            return gcvSTATUS_OK;
        }
    }
}

int _ConvertFormatToSurfaceInfo(
    int Format,
    int Width,
    int Height,
    size_t *Xstride,
    size_t *Ystride,
    size_t *Size
    )
{
    //log_func_entry;
    size_t xstride = 0;
    size_t ystride = 0;
    size_t size = 0;

    switch(Format)
    {
    case HAL_PIXEL_FORMAT_YCbCr_420_SP_MRVL:
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        xstride = _ALIGN(Width, 64 );
        ystride = _ALIGN(Height, 64 );
        size    = xstride * ystride * 3 / 2;
        break;

    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        xstride = _ALIGN(Width, 64);
        ystride = _ALIGN(Height, 64);
        size    = xstride * ystride * 2;
        break;

    case HAL_PIXEL_FORMAT_YCbCr_420_P:
        xstride = _ALIGN(Width, 64 );
        ystride = _ALIGN(Height, 64 );
        size    = xstride * ystride * 3 / 2;
        break;

    case HAL_PIXEL_FORMAT_YV12:
        xstride = _ALIGN(Width, 64 );
        ystride = _ALIGN(Height, 64 );
        size    = xstride * ystride * 3 / 2;
        break;

    case HAL_PIXEL_FORMAT_YCbCr_422_I:
    case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        xstride = _ALIGN(Width, 16);
        ystride = _ALIGN(Height, 32);
        size    = xstride * ystride * 2;
        break;

    /* RGBA format lists*/
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        xstride = _ALIGN(Width, 16);
        ystride = _ALIGN(Height, 4);
        size = xstride * ystride * 4;
        break;

    case HAL_PIXEL_FORMAT_RGB_888:
        xstride = _ALIGN(Width, 16);
        ystride = _ALIGN(Height, 4);
        size = xstride * ystride * 3;
        break;

    case HAL_PIXEL_FORMAT_RGB_565:
#if PLATFORM_SDK_VERSION < 19
    case HAL_PIXEL_FORMAT_RGBA_5551:
    case HAL_PIXEL_FORMAT_RGBA_4444:
#endif
        xstride = _ALIGN(Width, 16);
        ystride = _ALIGN(Height, 4);
        size = xstride * ystride * 2;
        break;

    default:
        return -EINVAL;
    }

    *Xstride = xstride;
    *Ystride = ystride;
    *Size = size;

    return 0;
}

/*******************************************************************************
**
**  _MapBuffer
**
**  Map android native buffer (call gc_gralloc_map).
**
**  INPUT:
**
**      gralloc_module_t const * Module
**          Specified gralloc module.
**
**      buffer_handle_t Handle
**          Specified buffer handle.
**
**  OUTPUT:
**
**      void ** Vaddr
**          Point to save virtual address pointer.
*/
extern int
_MapBuffer(
    gralloc_module_t const * Module,
    private_handle_t * Handle,
    void **Vaddr)
{
    //log_func_entry;
    int retCode = 0;

    retCode = gc_gralloc_map(Handle, Vaddr);
    if( retCode >= 0 && (Handle->surfFormat - gcvSURF_YUY2) > 0x63 &&
       (Handle->flags & (private_handle_t::PRIV_FLAGS_USES_PMEM_ADSP|private_handle_t::PRIV_FLAGS_USES_PMEM)) == private_handle_t::PRIV_FLAGS_USES_PMEM )
    {
        gc_gralloc_flush(Handle, Handle->flags);
    }

    return retCode;
}

/*******************************************************************************
**
**  gc_gralloc_alloc_buffer
**
**  Allocate android native buffer.
**  Will allocate surface types as follows,
**
**  +------------------------------------------------------------------------+
**  | To Allocate Surface(s)        | Linear(surface) | Tile(resolveSurface) |
**  |------------------------------------------------------------------------|
**  |Enable              |  CPU app |      yes        |                      |
**  |LINEAR OUTPUT       |  3D app  |      yes        |                      |
**  |------------------------------------------------------------------------|
**  |Disable             |  CPU app |      yes        |                      |
**  |LINEAR OUTPUT       |  3D app  |                 |         yes          |
**  +------------------------------------------------------------------------+
**
**
**  INPUT:
**
**      alloc_device_t * Dev
**          alloc device handle.
**
**      int Width
**          Specified target buffer width.
**
**      int Hieght
**          Specified target buffer height.
**
**      int Format
**          Specified target buffer format.
**
**      int Usage
**          Specified target buffer usage.
**
**  OUTPUT:
**
**      buffer_handle_t * Handle
**          Pointer to hold the allocated buffer handle.
**
**      int * Stride
**          Pointer to hold buffer stride.
*/
extern int
gc_gralloc_alloc_buffer(
    alloc_device_t * Dev,
    int Width,
    int Height,
    int Format,
    int Usage,
    buffer_handle_t * Handle,
    int * Stride
    )
{
    log_func_entry;

    int err             = 0;
    int fd              = -1;
    int master          = -1;
    int offset          = 0;
    int flags           = 0;
    int width           = 0;
    int height          = 0;

    size_t size         = 0;
    size_t xstride      = 0;
    size_t ystride      = 0;
    int bpr             = 0;
    int physAddr        = 0;
    gctUINT pixelPipes;

    /* Buffer handle. */
    private_handle_t * handle      = NULL;

    gctBOOL isAlloc                   = gcvFALSE;
    gctBOOL isPmemAlloc               = gcvFALSE;
    gctBOOL isGcAlloc                 = gcvFALSE;

    gceSTATUS status                  = gcvSTATUS_OK;
    gctUINT stride                    = 0;

    gceSURF_FORMAT format             = gcvSURF_UNKNOWN;
    gceSURF_FORMAT resolveFormat      = gcvSURF_UNKNOWN;
    (void) resolveFormat;

    /* Linear stuff. */
    gcoSURF surface                   = gcvNULL;
    gcuVIDMEM_NODE_PTR vidNode        = gcvNULL;
    gcePOOL pool                      = gcvPOOL_UNKNOWN;
    gctUINT adjustedSize              = 0U;

    /* Tile stuff. */
    gcoSURF resolveSurface            = gcvNULL;
    gcuVIDMEM_NODE_PTR resolveVidNode = gcvNULL;
    gcePOOL resolvePool               = gcvPOOL_UNKNOWN;
    gctUINT resolveAdjustedSize       = 0U;

    gctSIGNAL signal                  = gcvNULL;
    gctINT clientPID                  = 0;

    /* Binder info. */
    IPCThreadState* ipc               = IPCThreadState::self();

    clientPID = ipc->getCallingPid();

    ALOGE("gc_gralloc_alloc_buffer is not implemented, fallback to stock function!");
    return libstock::Inst().gc_gralloc_alloc(Dev, Width, Height, Format, Usage, Handle, Stride);

    /* Convert to hal pixel format. */
    if( _ConvertAndroid2HALFormat(Format, &format) != gcvSTATUS_OK )
        goto OnError;

    if( gcoHAL_QueryPixelPipesInfo(&pixelPipes,gcvNULL,gcvNULL) < 0 )
        goto OnError;

    if( (Usage & (GRALLOC_USAGE_HW_RENDER|GRALLOC_USAGE_HW_VIDEO_ENCODER) ) != GRALLOC_USAGE_HW_RENDER )
    {
        flags |= private_handle_t::PRIV_FLAGS_USES_PMEM;
        if( Usage & GRALLOC_USAGE_HW_VIDEO_ENCODER )
            flags |= private_handle_t::PRIV_FLAGS_USES_PMEM_ADSP;

        /* FIX ME: Workaround for PMEM path, due to user-allocated surface don't
         * do alignment when created, it may cause resolve onto texture
         * surface failed.
         */
        width = _ALIGN(Width, 16);

        /*the alignment of height is base on HW pixelpipe*/
        height = _ALIGN(Height, 4 * pixelPipes);
        err = _ConvertFormatToSurfaceInfo(Format,
                                           width,
                                           height,
                                           &xstride,
                                           &ystride,
                                           &size);
        if(err < 0)
        {
            ALOGE("not found compatible format!");
            return err;
        }

        size = roundUpToPageSize(size);

        if( !(Usage & GRALLOC_USAGE_PRIVATE_3) )
        {
            master = mvmem_alloc(size, 0x30000|ION_HEAP_TYPE_CARVEOUT, 0x1000);
            isGcAlloc = 0;
        }
        else
        {
            master = mvmem_alloc(size, 0x30000|ION_HEAP_TYPE_SYSTEM_CONTIG, 0x1000);
            isGcAlloc = 1;
            if( master < 0 )
            {
                ALOGW("WARNING: continuous ion memory alloc failed. Try to alloc non-continuous ion memory.");
                master = mvmem_alloc(size, 0x30000|ION_HEAP_TYPE_CARVEOUT, 0x1000);
                isGcAlloc = 0;
            }
        }
        // if fail to alloc buffer from ion, return failed.
        if(master >= 0)
        {
            mvmem_set_name(master, "gralloc");
            if( isGcAlloc )
            {
                if( mvmem_get_phys(master, &physAddr) < 0 )
                {
                    mvmem_free(master);
                    ALOGE("Failed to allocate memory from ION");
                    master = -1;
                }
            }
        }
        else
        {
            ALOGE("Failed to allocate memory from ION");
            err = master;
        }
    }

    if(isPmemAlloc)
    {
        if(master >= 0)
        {
            gcmONERROR(
                gcoSURF_Construct(gcvNULL,
                                  width,
                                  height,
                                  1,
                                  gcvSURF_BITMAP,
                                  format,
                                  gcvPOOL_USER,
                                  &surface));

            /* Now retrieve and store vid mem node attributes. */
            gcmONERROR(
               gcoSURF_QueryVidMemNode(surface,
                                       &vidNode,
                                       &pool,
                                       &adjustedSize));

            /* For CPU apps, we must synchronize lock requests from CPU with the composition.
             * Composition could happen in the following ways. i)2D, ii)3D, iii)CE, iv)copybit 2D.
             * (Note that, we are not considering copybit composition of 3D apps, which also uses
             * linear surfaces. Can be added later if needed.)

             * In all cases, the following mechanism must be used for proper synchronization
             * between CPU and GPU :

             * - App on gralloc::lock
             *      wait_signal(hwDoneSignal);

             * - Compositor on composition
             *      set_unsignalled(hwDoneSignal);
             *      issue composition;
             *      schedule_event(hwDoneSignal, clientPID);

             *  This is a manually reset signal, which is unsignalled by the compositor when
             *  buffer is in use, prohibiting app from obtaining write lock.
             */
            /* Manually reset signal, for CPU/GPU sync. */
            //gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvTRUE, &signal));

            /* Initially signalled. */
            //gcmONERROR(gcoOS_Signal(gcvNULL, signal, gcvTRUE));

            ALOGV("Created signal=%p for hnd=%p", signal, handle);

            /*buffer is allocated successfully.*/
            isAlloc = gcvTRUE;
        }

    }

    /*gc path, allocate memroy from gc driver(continuous or non-continous).*/
    if(!isAlloc && isGcAlloc)
    {
        /*reset width and height*/
        width = Width;
        height = Height;

        /*remove pmem flags if it is set*/
        flags &= ~private_handle_t::PRIV_FLAGS_USES_PMEM;

        /* For 3D app. */
        master = open("/dev/null",O_RDONLY,0);

        /* we're using two fds now */
        fd = open("/dev/null", O_RDONLY, 0);

        /*This flag is used to decide texture surafce use linear or tile. */
#if !gcdGPU_LINEAR_BUFFER_ENABLED
        /* 3D App use tile buffer, non-cached.(gcvSURF_TEXTURE) */

        /* Get the resolve format. */
        gcmONERROR(
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        format,
                                        &resolveFormat));

        /* Construct the resolve target. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              width,
                              height,
                              1,
                              gcvSURF_TEXTURE,
                              resolveFormat,
                              gcvPOOL_DEFAULT,
                              &resolveSurface));

        /* 3D surfaces are bottom-top. */
        //gcmONERROR(gcoSURF_SetOrientation(resolveSurface, gcvORIENTATION_BOTTOM_TOP));

        /* Now retrieve and store vid mem node attributes. */
        gcmONERROR(
            gcoSURF_QueryVidMemNode(resolveSurface,
                                   &resolveVidNode,
                                   &resolvePool,
                                   &resolveAdjustedSize));

        /* Android expects stride in pixels which is returned as alignedWidth. */
        gcmONERROR(
            gcoSURF_GetAlignedSize(resolveSurface,
                                   &stride,
                                   gcvNULL,
                                   gcvNULL));

#else
            /* 3D App use linear buffer, non-cached. (gcvSURF_BITMAP)*/

            /* For HARWARE_RENDER case, allocate non-cacheable Linear Surface from GC reserved. */
#if gcdANDROID_UNALIGNED_LINEAR_COMPOSITION_ADJUST
             gcmONERROR(
                gcoSURF_Construct(gcvNULL,
                                  width,
                                  height,
                                  1,
                                  gcvSURF_FLIP_BITMAP,
                                  format,
                                  gcvPOOL_DEFAULT,
                                  &surface));
#else
             gcmONERROR(
                gcoSURF_Construct(gcvNULL,
                                  width,
                                  height,
                                  1,
                                  gcvSURF_BITMAP,
                                  format,
                                  gcvPOOL_DEFAULT,
                                  &surface));
#endif
            /* Now retrieve and store vid mem node attributes. */
            gcmONERROR(
               gcoSURF_QueryVidMemNode(surface,
                                       &vidNode,
                                       &pool,
                                       &adjustedSize));

            /* Get stride. */
            gcmONERROR(
                gcoSURF_GetAlignedSize(surface,
                                       &stride,
                                       gcvNULL,
                                       gcvNULL));

#endif
            /*buffer is allocated successfully.*/
            isAlloc = gcvTRUE;

            /*buffer is allocated from gc driver but not ion.*/
            isPmemAlloc = gcvFALSE;
    }

    if(err == 0)
    {
        handle = new private_handle_t(fd, size, flags);
        /*
        handle->master              = master;
        handle->width               = (int) width;
        handle->height              = (int) height;
        handle->format              = (int) Format;
        handle->surfFormat          = (int) format;

        // Save tile resolveSurface to buffer handle.
        handle->resolveSurface      = (int) resolveSurface;
        handle->resolveVidNode      = (int) resolveVidNode;
        handle->resolvePool         = (int) resolvePool;
        handle->resolveAdjustedSize = (int) resolveAdjustedSize;
        handle->adjustedSize        = (int) adjustedSize;

        handle->hwDoneSignal        = (int) signal;

        // Record usage to recall later in hwc.
        handle->lockUsage           = 0;
        handle->allocUsage          = Usage;
        handle->clientPID           = clientPID;
        */
        /* Case module. */
        gralloc_module_t *module =
            reinterpret_cast<gralloc_module_t *>(Dev->common.module);

        /*
        if(isPmemAlloc)
        {
            // Public part.
            handle->offset          = offset;
            handle->physAddr        = physAddr;
            handle->usage           = Usage;
            handle->mem_xstride     = xstride;// Aligned width.
            handle->mem_ystride     = ystride;

            // Private part.
            handle->bpr             = bpr;
            handle->surface         = (int) surface;
            handle->pool            = gcvPOOL_USER;
            stride                  = xstride;

            // Map pmem memory to user.
            err = _MapBuffer(module, handle);
            if (err < 0)
            {
                ALOGE(" gc_gralloc_map memory error");
                return -errno;
            }
            gcmONERROR(
                gcoSURF_MapUserSurface(surface,
                                        0,
                                        (void*)handle->base,
                                        ~0));
        }
        else
        {
            //3D App: Save linear surface to buffer handle, possibe this surface
            //is NULL if gcdGPU_LINEAR_BUFFER_ENABLED disabled.
            handle->surface             = (int) surface;
            handle->vidNode             = (int) vidNode;
            handle->pool                = (int) pool;

            // Save correct xstride and ystride.
            if(surface != gcvNULL)
            {
                gcmONERROR(
                    gcoSURF_GetAlignedSize(surface, &xstride, &ystride, gcvNULL));

                handle->mem_xstride     = xstride;
                handle->mem_ystride     = ystride;
            }

            // Map video memory to user.
            err = _MapBuffer(module, handle);
        }
        */
    }

    if(err == 0)
    {
        *Handle = handle;
        *Stride = stride;
    }
    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    return err;

OnError:
    /* Destroy linear surface. */
    if (surface != gcvNULL)
    {
        gcoSURF_Destroy(surface);
        surface = gcvNULL;
    }

    /* Destroy tile resolveSurface. */
    if (resolveSurface != gcvNULL)
    {
        gcoSURF_Destroy(resolveSurface);
        resolveSurface = gcvNULL;
    }

    /* Destroy signal. */
    if (signal)
    {
        gcoOS_DestroySignal(gcvNULL, signal);
        signal = gcvNULL;
    }

    /* Error roll back. */
    if (handle != NULL)
    {
        delete handle;
        handle = gcvNULL;
    }

    ALOGE("failed to allocate, status=%d", status);

    return -EINVAL;
}

/*******************************************************************************
**
**  gc_gralloc_alloc
**
**  General buffer alloc.
**
**  INPUT:
**
**      alloc_device_t * Dev
**          alloc device handle.
**
**      int Width
**          Specified target buffer width.
**
**      int Height
**          Specified target buffer height.
**
**      int Format
**          Specified target buffer format.
**
**      int Usage
**          Specified target buffer usage.
**
**  OUTPUT:
**
**      buffer_handle_t * Handle
**          Pointer to hold the allocated buffer handle.
**
**      int * Stride
**          Pointer to hold buffer stride.
*/
extern int
gc_gralloc_alloc(
    alloc_device_t * Dev,
    int Width,
    int Height,
    int Format,
    int Usage,
    buffer_handle_t * Handle,
    int * Stride)
{
    //log_func_entry;

    int ret;
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;

    if (!Handle || !Stride)
    {
        return -EINVAL;
    }

    /* Must be set current hardwareType to 3D, due to surfaceflinger need tile
     * surface.
     */
    gcoHAL_GetHardwareType(0, &hwtype);
    setHwType71D0(Usage);

    ret = gc_gralloc_alloc_buffer(Dev, Width, Height, Format, Usage, Handle, Stride);

    gcoHAL_SetHardwareType(0, hwtype);

    return ret;
}


extern int setHwType71D0(int AllocUsage)
{
    //log_func_entry;
    static gceHARDWARE_TYPE hwtype = gcvHARDWARE_INVALID;
    int buffer[80];

    if( hwtype == gcvHARDWARE_INVALID )
    {
        buffer[0] = 39;
        gcoOS_DeviceControl(0, 30000, buffer, sizeof(buffer), buffer, sizeof(buffer));
        for ( int i = 0; i < buffer[8]; ++i )
        {
            switch ( buffer[i+9] )
            {
                case gcvHARDWARE_3D:
                    hwtype = gcvHARDWARE_3D;
                    break;
                case gcvHARDWARE_3D2D:
                    hwtype = gcvHARDWARE_3D2D;
                    break;
                case gcvHARDWARE_2D:
                    if ( !(hwtype & ~gcvHARDWARE_VG) )
                        hwtype = gcvHARDWARE_2D;
                    break;
                case gcvHARDWARE_VG:
                    if ( hwtype == gcvHARDWARE_INVALID )
                        hwtype = gcvHARDWARE_VG;
                    break;
                default:
                    continue;
            }
        }
        if ( !hwtype )
            ALOGE("Failed to get hardware types");
    }
    if( (AllocUsage & (GRALLOC_USAGE_RENDERSCRIPT|GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_MRVL_PRIVATE_1)) == (GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_MRVL_PRIVATE_1) )
        return gcoHAL_SetHardwareType(0, gcvHARDWARE_VG);
    else
        return gcoHAL_SetHardwareType(0, hwtype);
}

/*******************************************************************************
**
**  gc_gralloc_free
**
**  General buffer free.
**
**  INPUT:
**
**      alloc_device_t * Dev
**          alloc device handle.
**
**      buffer_handle_t Handle
**          Specified target buffer to free.
**
**  OUTPUT:
**
**      Nothing.
*/
extern int
gc_gralloc_free(
    alloc_device_t * Dev,
    buffer_handle_t Handle
    )
{
    //log_func_entry;
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;

    if( private_handle_t::validate(Handle) )
    {
        return -EINVAL;
    }

    /* Cast private buffer handle. */
    private_handle_t * hnd = (private_handle_t*)Handle;

    gcoHAL_GetHardwareType(0, &hwtype);
    setHwType71D0(hnd->allocUsage);
    if( hnd->base )
        gc_gralloc_unmap(hnd);

    if( hnd->surface )
    {
        if( (hnd->allocUsage & (GRALLOC_USAGE_MRVL_PRIVATE_1|GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_RENDERSCRIPT)) == (GRALLOC_USAGE_MRVL_PRIVATE_1|GRALLOC_USAGE_FOREIGN_BUFFERS) )
        {
            setHwType71D0(GRALLOC_USAGE_SW_READ_NEVER);
            gcoSURF_Unlock(hnd->surface, 0);
            setHwType71D0(hnd->allocUsage);
        }
        gcoSURF_Destroy(hnd->surface);
    }
    if( hnd->signal )
    {
        gcoOS_DestroySignal(0, hnd->signal);
        hnd->signal = NULL;
        hnd->signalHigh32Bits = 0;
    }
    hnd->clientPID = 0;
    gcoHAL_Commit(0,gcvFALSE);
    if( hnd->flags & private_handle_t::PRIV_FLAGS_USES_PMEM )
    {
        mvmem_free(hnd->master);
        if( hnd->fd >= 0 )
            close(hnd->fd);
    }
    else
    {
        close(hnd->fd);
        close(hnd->master);
    }

    hnd->magic = 0;
    delete hnd;
    gcoHAL_SetHardwareType(0, hwtype);

    return 0;
}

extern int gc_gralloc_notify_change(buffer_handle_t Handle)
{
    //log_func_entry;
    private_handle_t *hnd = (private_handle_t*)Handle;
    if( private_handle_t::validate(hnd) )
        return -EINVAL;

    if( hnd->surface == NULL )
        return -EINVAL;

    gcoSURF_UpdateTimeStamp(hnd->surface);
    gcoSURF_PushSharedInfo(hnd->surface);

    return 0;
}

extern int setHwType71D4()
{
    //log_func_entry;
    static gceHARDWARE_TYPE hwtype = gcvHARDWARE_INVALID;
    int buffer[80];

    if( hwtype == 0 )
    {
        buffer[0] = 39;
        gcoOS_DeviceControl(0, 30000, buffer, sizeof(buffer), buffer, sizeof(buffer));
        for( int i = 0; i < buffer[8]; ++i )
        {
            switch( buffer[i+9] )
            {
                case gcvHARDWARE_3D: hwtype = gcvHARDWARE_3D; break;
                case gcvHARDWARE_3D2D: hwtype = gcvHARDWARE_3D2D; break;
                case gcvHARDWARE_2D:
                    if( (hwtype & ~gcvHARDWARE_VG) == 0 )
                        hwtype = gcvHARDWARE_2D;
                    break;
                case gcvHARDWARE_VG:
                    if( hwtype == gcvHARDWARE_INVALID )
                        hwtype = gcvHARDWARE_VG;
                    break;
            }
        }
        if( hwtype == gcvHARDWARE_INVALID )
            ALOGE("Failed to get hardware types");
    }
    return gcoHAL_SetHardwareType(0, hwtype);
}

extern int
gc_gralloc_unwrap(buffer_handle_t Handle)
{
    //log_func_entry;
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;
    private_handle_t *hnd = (private_handle_t*)Handle;
    if( private_handle_t::validate(hnd) )
        return -EINVAL;

    gcoHAL_GetHardwareType(0, &hwtype);
    setHwType71D4();
    gcoSURF_Unlock(hnd->surface, 0);
    gcoSURF_Destroy(hnd->surface);
    gcoHAL_Commit(0, gcvTRUE);
    memset(&hnd->surface, 0, 108);
    gcoHAL_SetHardwareType(0, hwtype);
    return 0;
}

extern int
gc_gralloc_wrap(buffer_handle_t Handle, int w, int h, int format, int stride, int offset, void *vaddr)
{
   //log_func_entry;
    private_handle_t *hnd = (private_handle_t*)Handle;
    int status;
    gcoSURF surface = 0;
    gceSURF_FORMAT surfFormat;
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;
    gctUINT32 Address;
    gctUINT32 physical = offset;
    gctUINT32 base_addr;
    gctSHBUF shbuf;
    gcsSURF_FORMAT_INFO_PTR formatInfo;

    if( private_handle_t::validate(hnd) )
        return -EINVAL;

    if( vaddr == NULL)
    {
        ALOGE("%s Invalid virtual address", __FUNCTION__);
        return -EINVAL;
    }

    if( _ConvertAndroid2HALFormat(format, &surfFormat) != gcvSTATUS_OK )
    {
        return -EINVAL;
    }

    gcoHAL_GetHardwareType(0, &hwtype);
    status = gcoSURF_QueryFormat(surfFormat, &formatInfo);
    if( status >= 0 )
    {
        if( (surfFormat-gcvSURF_YUY2) <= 9 )
        {
            if( (1 << (surfFormat+12)) & 0x303 )
            {
                stride *= 2;
            }
            else if( (1 << (surfFormat+12)) & 0xFC )
            {
            }
            else
            {
                stride *= formatInfo->interleaved >> 3;
            }
        }
        else
        {
            stride *= formatInfo->interleaved >> 3;
        }
        memset(&hnd->surface, 0, 108);
        setHwType71D4();
        if( offset != -1 )
        {
            status = gcoOS_GetBaseAddress(0, &base_addr);
            if( status < 0 )
            {
                if( surface )
                    gcoSURF_Destroy(surface);
                gcoHAL_SetHardwareType(0, hwtype);
                return -EFAULT;
            }
            physical = offset - base_addr;
            if( (signed int)(physical) < 0 )
            {
                physical = -1;
            }
            else if( (signed int)(physical + h * stride - 1) < 0 )
            {
                physical = -1;
            }
        }
        status = gcoSURF_Construct(0, w, h, 1, gcvSURF_BITMAP, surfFormat, gcvPOOL_USER, &surface);
        if( status >= 0 )
        {
            status = gcoSURF_SetBuffer(surface, gcvSURF_BITMAP, surfFormat, stride, vaddr, physical);
            if( status >= 0 )
            {
                status = gcoSURF_SetWindow(surface, 0, 0, w, h);
                if( status >= 0 )
                {
                    status = gcoSURF_Lock(surface, &Address, (gctPOINTER*)&base_addr);
                    if( status >= 0 )
                    {
                        status = gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE);
                        if( status >= 0 )
                        {
                            status = gcoSURF_AllocShBuffer(surface, &shbuf);
                            if( status >= 0 )
                            {
                                hnd->stride = stride;
                                hnd->surface = surface;
                                hnd->surfaceHigh32Bits = 0;
                                hnd->lockAddr = Address;
                                hnd->dirtyHeight = h;
                                hnd->dirtyWidth = w;
                                hnd->format = format;
                                hnd->surfFormat = surfFormat;
                                hnd->shAddr = shbuf;
                                hnd->shAddrHight32Bits = 0;
                                gcoHAL_SetHardwareType(0, hwtype);
                                return 0;
                            }
                            else
                                ALOGE("%s: Failed gcoSURF_AllocShBuffer", __FUNCTION__);
                        }
                        else
                            ALOGE("%s: Failed gcoSURF_SetFlags", __FUNCTION__);
                    }
                    else
                        ALOGE("%s: Failed gcoSURF_Lock", __FUNCTION__);
                }
                else
                    ALOGE("%s: Failed gcoSURF_SetWindow", __FUNCTION__);
            }
            else
                ALOGE("%s: Failed gcoSURF_Buffer", __FUNCTION__);
        }
        else
            ALOGE("%s: Failed gcoSURF_Construct", __FUNCTION__);
    }

    ALOGE("failed to wrap handle=%p", hnd);

    if( surface )
        gcoSURF_Destroy(surface);

    gcoHAL_SetHardwareType(0, hwtype);

    return -EFAULT;
}

extern int
gc_gralloc_register_wrap(private_handle_t *Handle, int32_t offset, void* Vaddr)
{
    //log_func_entry;
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;
    gctUINT32 Memory;
    int status;
    gcoSURF surface = 0;
    gctUINT32 Address;

    if( private_handle_t::validate(Handle) )
        return -EINVAL;

    if( Vaddr == NULL )
    {
        ALOGE("Invalid virtual address");
        return -EINVAL;
    }

    if( Handle->surface == NULL )
        return -EINVAL;

    if( Handle->dirtyHeight == 0 )
        return -EINVAL;
    if( Handle->dirtyWidth == 0 )
        return -EINVAL;

    if( Handle->format == gcvSURF_UNKNOWN )
        return -EINVAL;

    if( Handle->surfFormat == 0 )
        return -EINVAL;

    gcoHAL_GetHardwareType(0, &hwtype);
    setHwType71D4();
    if( offset != -1 )
    {
        status = gcoOS_GetBaseAddress(0, &Memory);
        if( status < 0 )
        {
            gcoHAL_SetHardwareType(0, hwtype);
            return -EFAULT;
        }
        offset -= (uint32_t)Memory;
    }
    status = gcoSURF_Construct(0, Handle->dirtyWidth, Handle->dirtyHeight, 1, gcvSURF_BITMAP, Handle->surfFormat, gcvPOOL_USER, &surface);
    if( status < 0 )
    {
        if( surface )
        {
            gcoSURF_Destroy(surface);
            gcoHAL_Commit(0, gcvFALSE);
        }
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_SetBuffer(surface, gcvSURF_BITMAP, Handle->surfFormat, Handle->stride, Vaddr, offset);
    if( status < 0 )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_SetWindow(surface, 0, 0, Handle->dirtyWidth, Handle->dirtyHeight);
    if( status < 0 )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_Lock(surface, &Address, (void**)&Memory);
    if( status < 0 )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE);
    if( status < 0 )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    if( Handle->shAddr )
        gcoSURF_BindShBuffer(surface, Handle->shAddr);

    Handle->surfaceHigh32Bits = 0;
    Handle->surface = surface;
    Handle->lockAddr = Address;

    gcoHAL_SetHardwareType(0, hwtype);
    return 0;
}
