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

#define PLATFORM_SDK_VERSION 22

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

static int android_formats[] =
{
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, gcvSURF_A8B8G8R8,
    HAL_PIXEL_FORMAT_RGBA_8888             , gcvSURF_A8B8G8R8,
    HAL_PIXEL_FORMAT_RGBX_8888             , gcvSURF_X8B8G8R8,
    HAL_PIXEL_FORMAT_RGB_888               , gcvSURF_UNKNOWN ,
    HAL_PIXEL_FORMAT_RGB_565               , gcvSURF_R5G6B5  ,
    HAL_PIXEL_FORMAT_BGRA_8888             , gcvSURF_A8R8G8B8,
    HAL_PIXEL_FORMAT_YCbCr_420_888         , gcvSURF_NV12    ,
    HAL_PIXEL_FORMAT_YV12                  , gcvSURF_YV12    , // YCrCb 4:2:0 Planar
    HAL_PIXEL_FORMAT_YCbCr_422_SP          , gcvSURF_NV16    , // NV16
    HAL_PIXEL_FORMAT_YCrCb_420_SP          , gcvSURF_NV21    , // NV21
    HAL_PIXEL_FORMAT_YCbCr_420_P           , gcvSURF_I420    ,
    HAL_PIXEL_FORMAT_YCbCr_422_I           , gcvSURF_YUY2    , // YUY2
    HAL_PIXEL_FORMAT_CbYCrY_422_I          , gcvSURF_UYVY    ,
    HAL_PIXEL_FORMAT_YCbCr_420_SP_MRVL     , gcvSURF_NV12    ,
    0x00
};

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
static gceSTATUS
_ConvertAndroid2HALFormat(
    int Format,
    gceSURF_FORMAT * HalFormat
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    switch (Format)
    {
    case HAL_PIXEL_FORMAT_RGB_565:
        *HalFormat = gcvSURF_R5G6B5;
        break;

    case HAL_PIXEL_FORMAT_RGBA_8888:
        *HalFormat = gcvSURF_A8R8G8B8;
        break;

    case HAL_PIXEL_FORMAT_RGBX_8888:
        *HalFormat = gcvSURF_X8R8G8B8;
        break;

#if PLATFORM_SDK_VERSION < 19
    case HAL_PIXEL_FORMAT_RGBA_4444:
        *HalFormat = gcvSURF_R4G4B4A4;
        break;

    case HAL_PIXEL_FORMAT_RGBA_5551:
        *HalFormat = gcvSURF_R5G5B5A1;
        break;
#endif

    case HAL_PIXEL_FORMAT_BGRA_8888:
        *HalFormat = gcvSURF_A8B8G8R8;
        break;

    case HAL_PIXEL_FORMAT_YCbCr_420_SP_MRVL:
        /* YUV 420 semi planner: NV12 */
        *HalFormat = gcvSURF_NV12;
        break;

    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        /* YVU 420 semi planner: NV21 */
        *HalFormat = gcvSURF_NV21;
        break;

    case HAL_PIXEL_FORMAT_YCbCr_420_P:
        /* YUV 420 planner: I420 */
        *HalFormat = gcvSURF_I420;
        break;

    case HAL_PIXEL_FORMAT_YV12:
        /* YVU 420 planner: YV12 */
        *HalFormat = gcvSURF_YV12;
        break;

    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        /* YUV 422 package: YUYV, YUY2 */
        *HalFormat = gcvSURF_YUY2;
        break;

    case HAL_PIXEL_FORMAT_CbYCrY_422_I:
        /* UYVY 422 package: UYVY */
        *HalFormat = gcvSURF_UYVY;
        break;


    default:
        *HalFormat = gcvSURF_UNKNOWN;
        status = gcvSTATUS_INVALID_ARGUMENT;

        ALOGE("Unknown format %d", Format);
        break;
    }

    return status;
}

static int _ConvertFormatToSurfaceInfo(
    int Format,
    int Width,
    int Height,
    size_t *Xstride,
    size_t *Ystride,
    size_t *Size
    )
{
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
int
_MapBuffer(
    gralloc_module_t const * Module,
    private_handle_t * Handle,
    void **Vaddr)
{
    int retCode = 0;

    retCode = gc_gralloc_map(Handle, Vaddr);
    if( retCode && (Handle->surfFormat - gcvSURF_YUY2) > 99 &&
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
**  |Diable              |  CPU app |      yes        |                      |
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
static int
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
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;
    gceSURF_FORMAT surfFormat = gcvSURF_UNKNOWN;
    gceSTATUS status;
    gctUINT32 pixelPipes;
    int fd = open("/dev/null", 0, 0);
    int masterFd;
    int buffFlags;
    int v15;
    int v21;
    int v22;
    int v24;
    int v25;
    int v26;
    int v27;
    int v28;
    int v29;
    int v30;
    int v32;
    int v34;
    int v35;
    int v36;
    int v42;
    int v62;
    int v65;
    int v68;
    int v69;
    int v70;
    int v81;
    int v89;
    int v91;
    gctUINT32 v93;
    gctUINT32 v94;
    gctUINT alignedWidth, alignedWidth2;
    gctUINT alignedHeight, alignedHeight2;
    int v101;
    int v103;
    int v104;
    int dirtyWidthAligned16;
    int lineSize;
    int callingPID;
    int dirtyWidth;
    int dirtyHeight;
    void *Vaddr;
    void *v76;
    gcoSURF surface;
    gcePOOL Pool;
    gctUINT Bytes;
    gctSHBUF shAddr;
    gcuVIDMEM_NODE_PTR Node;

    private_handle_t *hnd;

    /* Binder info. */
    IPCThreadState* ipc               = IPCThreadState::self();
    callingPID = ipc->getCallingPid();

    gcoHAL_GetHardwareType(0, &hwtype);
    setHwType71D0(Usage);

    dirtyHeight = Height;
    dirtyWidth = Width;

    for( int i = 0; ; i += 2 )
    {
        if( android_formats[i] == 0 )
        {
            ALOGE("Unknown ANDROID format: %d", format);
            break;
        }
        if( (gceSURF_FORMAT)android_formats[i+1] == gcvSURF_UNKNOWN )
        {
            ALOGE("Not supported ANDROID format: %d", format);
            break;
        }

        if( format == android_formats[i] )
        {
            surfFormat = (gceSURF_FORMAT)android_formats[i+1];
            break;
        }
    }

    if( surfFormat != gcvSURF_UNKNOWN )
    {
        status = gcoHAL_QueryPixelPipesInfo(&pixelPipes, 0, 0);
        if( status != gcvSTATUS_OK )
        {
            surfFormat = gcvSURF_UNKNOWN;
            v15 = 0;
LABEL_97:
            masterFd = -1;
            goto ON_ERROR;
        }

        if( (Usage & (GRALLOC_USAGE_HW_VIDEO_ENCODER|GRALLOC_USAGE_HW_RENDER)) != GRALLOC_USAGE_HW_RENDER )
        {
            if( Usage & GRALLOC_USAGE_HW_VIDEO_ENCODER  )
                buffFlags = private_handle_t::PRIV_FLAGS_USES_PMEM_ADSP|private_handle_t::PRIV_FLAGS_USES_PMEM;
            else
                buffFlags = private_handle_t::PRIV_FLAGS_USES_PMEM;

            v21 = dirtyWidth + 15;
            v22 = (4 * pixelPipes - 1 + dirtyHeight) & ~3 * pixelPipes;
            v91 = 8;
            dirtyWidthAligned16 = _ALIGN(dirtyWidth, 16);
            v24 = (Usage >> 2) & 1;
            if( Format != HAL_PIXEL_FORMAT_YCrCb_420_SP )
            {
                if( Format <= HAL_PIXEL_FORMAT_YCrCb_420_SP )
                {
                    if( Format == HAL_PIXEL_FORMAT_RGB_888 )
                    {
                        v28 = v21 & ~0xF;
                        v29 = _ALIGN(v22, 4);
                        // 3bytes, 1R, 1G, 1B
                        lineSize = 3 * _ALIGN(dirtyWidth, 16);
                        v30 = v29 * lineSize;
                        goto LABEL_71;
                    }
                    if( Format > HAL_PIXEL_FORMAT_RGB_888 )
                    {
                        if( Format != HAL_PIXEL_FORMAT_BGRA_8888 )
                        {
                            if( Format < HAL_PIXEL_FORMAT_BGRA_8888 )// HAL_PIXEL_FORMAT_RGB_565
                            {
                                v28 = v21 & ~0xF;
                                v29 = _ALIGN(v22, 4);
                            }
                            else
                            {
                                if( Format != HAL_PIXEL_FORMAT_YCbCr_422_SP )
                                    goto LABEL_96;
                                v21 = v22 + 63;
                                v28 = _ALIGN(dirtyWidthAligned16, 64);
                                v29 = _ALIGN(v22, 64);
                                if( v24 )
                                {
                                    v30 = 2 * v28 * v29;
                                    goto LABEL_71;
                                }
                            }
                            lineSize = 2 * v28;
                            v30 = v29 * lineSize;
                            goto LABEL_71;
                        }
                    }
                    else if( Format < HAL_PIXEL_FORMAT_RGBA_8888 )
                    {
                        goto LABEL_96;
                    }
LABEL_66:
                    v28 = v21 & ~0xF;
                    v29 = _ALIGN(v22, 4);
                    lineSize = 4 * dirtyWidthAligned16;
                    v30 = v29 * lineSize;
                    goto LABEL_71;
                }
                if( Format == HAL_PIXEL_FORMAT_CbYCrY_422_I )
                {
LABEL_64:
                    v29 = _ALIGN(v22, 32);
                    v28 = dirtyWidthAligned16;
                    v30 = v29 * 2 * dirtyWidthAligned16;
                    if( v24 )
                        v30 = _ALIGN(v30, 4096);
LABEL_71:
                    v81 = _ALIGN(v30, 4096);
                    v32 = v81 + 4095;
                    if( (Usage & GRALLOC_USAGE_PRIVATE_3) )
                    {
                        masterFd = mvmem_alloc(v32 & ~4095, 0x30001, 0x1000);
                        if( masterFd >= 0 )
                        {
                            v35 = 1;
                            goto LABEL_78;
                        }
                        ALOGW("WARNING: continuous ion memory alloc failed. Try to alloc non-continuous ion memory.");
                    }
                    masterFd = mvmem_alloc(v32 & ~4095, 0x30002, 0x1000);
                    if( masterFd < 0 )
                    {
                        v34 = -errno;
LABEL_82:
                        ALOGE("Failed to allocate memory from ION");
                        masterFd = v34;
                        goto LABEL_83;
                    }
                    v35 = 0;
LABEL_78:
                    mvmem_set_name(masterFd, "gralloc");
                    if( v35 )
                    {
                        v34 = mvmem_get_phys(masterFd, &v101);
                        if( v34 < 0 )
                        {
                            mvmem_free(masterFd);
                            goto LABEL_82;
                        }
                    }
                    else
                    {
                        v101 = 0;
                    }
LABEL_83:
                    v36 = 0x2000006;
                    if( !v24 )
                        v36 = 6;
                    status = gcoSURF_Construct(0, dirtyWidthAligned16, v22, 1, (gceSURF_TYPE)v36, surfFormat, gcvPOOL_SYSTEM, &surface);
                    if( status != gcvSTATUS_OK )
                    {
LABEL_89:
                        hnd = 0;
                        v15 = 1;
                        goto ON_ERROR;
                    }
                    status = gcoSURF_GetAlignedSize(surface, &alignedWidth, &alignedHeight, 0);
                    if( status != gcvSTATUS_OK )
                    {
                        goto LABEL_89;
                    }
                    status = gcoSURF_QueryVidMemNode(surface, &Node, &Pool, &Bytes);
                    if( status != gcvSTATUS_OK )
                    {
                        goto LABEL_89;
                    }
                    if( dirtyWidthAligned16 != dirtyWidth || v22 != dirtyHeight )
                    {
                        status = gcoSURF_SetRect(surface, dirtyWidth, dirtyHeight);
                        if( status != gcvSTATUS_OK )
                        {
                            goto LABEL_89;
                        }
                    }
                    dirtyHeight = v22;
                    v25 = 0;
                    dirtyWidth = _ALIGN(dirtyWidth, 16);
                    v15 = 1;
                    v26 = 6;
LABEL_151:
                    status = gcoSURF_AllocShBuffer(surface, &shAddr);
                    if( status != gcvSTATUS_OK )
                    {
                        hnd = 0;
                        goto ON_ERROR;
                    }
                    hnd = new private_handle_t(fd, v89 * dirtyHeight, buffFlags);
                    hnd->format = Format;
                    hnd->surfType = (gceSURF_TYPE)v26;
                    hnd->samples = v25;
                    hnd->dirtyWidth = dirtyWidth;
                    hnd->dirtyHeight = dirtyHeight;
                    hnd->surface = surface;
                    hnd->surfaceHigh32Bits = 0;
                    hnd->surfFormat = (gceSURF_FORMAT)v62;
                    hnd->size = surface->totalSize[22];
                    hnd->clientPID = callingPID;
                    hnd->allocUsage = Usage;
                    hnd->signal = 0;
                    hnd->signalHigh32Bits = 0;
                    hnd->lockUsage = 0;
                    hnd->master = masterFd;
                    if( v15 )
                    {
                        hnd->lockAddr = v101;
                        hnd->pool = (gcePOOL)surface->totalSize[27];
                        hnd->infoB2 = surface->totalSize[39];
                        hnd->infoA2 = surface->totalSize[90];
                        hnd->infoB3 = surface->totalSize[102];
                        hnd->field_34 = v28;
                        hnd->field_38 = v29;
                        hnd->size = v81;
                        v65 = alignedWidth;
                        v42 = _MapBuffer((gralloc_module_t*)Dev, hnd, &Vaddr);
                        if( v42 < 0 )
                        {
                            ALOGE("gc_gralloc_map memory error");
LABEL_162:
                            fd = -1;
                            goto ON_ERROR;
                        }
                        status = gcoSURF_MapUserSurface(surface, 0, (gctPOINTER)hnd->base, -1);
                        if( status != gcvSTATUS_OK )
                        {
LABEL_169:
                            fd = -1;
                            goto ON_ERROR;
                        }
                    }
                    else
                    {
                        v68 = surface->totalSize[41];
                        alignedWidth2 = 0;
                        alignedHeight2 = 0;
                        v93 = v68;
                        gcoHAL_NameVideoMemory(v68, &v93);
                        hnd->infoA1 = v93;
                        hnd->pool = (gcePOOL)surface->totalSize[27];
                        hnd->infoB2 = surface->totalSize[39];
                        v70 = surface->totalSize[104];
                        v94 = v70;
                        if( v70 )
                            gcoHAL_NameVideoMemory(v70, &v94);
                        hnd->infoB1 = v94;
                        hnd->infoA2 = surface->totalSize[90];
                        hnd->infoB3 = surface->totalSize[102];
                        status = gcoSURF_GetAlignedSize(surface, &alignedWidth2, &alignedHeight2, 0);
                        if( status != gcvSTATUS_OK )
                        {
                            goto LABEL_162;
                        }
                        hnd->field_34 = alignedWidth2;
                        hnd->field_38 = alignedHeight2;
                        v42 = _MapBuffer((gralloc_module_t*)Dev, hnd, &Vaddr);
                    }
                    if( v42 == 0 )
                    {
                        v76 = Vaddr;
                        *Handle = (buffer_handle_t)hnd;
                        *Stride = v65;
                        if( v76 && (Usage & GRALLOC_USAGE_HW_RENDER) == 0 )
                            memset(v76, 0, hnd->size);
                        goto LABEL_109;
                    }
                    status = (gceSTATUS)-13;
                    goto LABEL_169;
                }
                if( Format <= HAL_PIXEL_FORMAT_CbYCrY_422_I )
                {
                    if( Format != HAL_PIXEL_FORMAT_YCbCr_420_P )
                    {
                        if( Format != HAL_PIXEL_FORMAT_YCbCr_422_I )
                            goto LABEL_96;
                        goto LABEL_64;
                    }
                    goto LABEL_61;
                }
                if( Format != HAL_PIXEL_FORMAT_YCbCr_420_SP_MRVL )
                {
                    if( Format != HAL_PIXEL_FORMAT_YV12 )
                    {
                        if( Format != HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED )
                        {
LABEL_96:
                            hnd = 0;
                            v15 = 1;
                            goto LABEL_97;
                        }
                        goto LABEL_66;
                    }
LABEL_61:
                        v28 = _ALIGN(dirtyWidthAligned16, 64);
                        v29 = _ALIGN(v22, 64);
                        if( v24 )
                        {
                            v30 = _ALIGN((v28*v29)/2,4096) + v28*v29;
                            goto LABEL_71;
                        }
LABEL_63:
                        v30 = (v29 * v28 * 3) / 2;
                        goto LABEL_71;
                }
                v25 = Usage;
                if( (Usage & GRALLOC_USAGE_SW_WRITE_OFTEN) == GRALLOC_USAGE_SW_WRITE_OFTEN
                 || (Usage & GRALLOC_USAGE_SW_READ_OFTEN) == GRALLOC_USAGE_SW_READ_OFTEN )
                {
                    v25 = 0;
                    Pool = gcvPOOL_CONTIGUOUS;
                    v26 = 1030;
                    goto LABEL_111;
                }
                goto LABEL_44;
            }
            v27 = Usage & (GRALLOC_USAGE_RENDERSCRIPT|GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_MRVL_PRIVATE_1);
            if( v27 == GRALLOC_USAGE_RENDERSCRIPT )
            {
                Pool = gcvPOOL_DEFAULT;
LABEL_47:
                v26 = 4;
                goto LABEL_111;
            }
            switch( v27 )
            {
                case GRALLOC_USAGE_FOREIGN_BUFFERS:
                    Pool = gcvPOOL_DEFAULT;
                    v26 = 260;
                    break;
                case GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_RENDERSCRIPT:
                    Pool = gcvPOOL_DEFAULT;
                    v26 = 0x40004;
                    break;
                case GRALLOC_USAGE_MRVL_PRIVATE_1:
                    Pool = gcvPOOL_DEFAULT;
                    v25 = 4;
                    goto LABEL_47;
                default:
                    if( v27 != (GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_MRVL_PRIVATE_1)
                     && (v27 == (GRALLOC_USAGE_MRVL_PRIVATE_1|GRALLOC_USAGE_FOREIGN_BUFFERS)
                         || Usage & (GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_RENDER)
                         || !(Usage & GRALLOC_USAGE_HW_TEXTURE)
                        )
                      )
                    {
 LABEL_44:
                        v25 = 0;
                        Pool = gcvPOOL_DEFAULT;
                        v26 = 6;
                        break;
                    }
                    v25 = 0;
                    Pool = gcvPOOL_DEFAULT;
                    v26 = 3;
                    break;
            }
LABEL_111:
            masterFd = open("/dev/null", 0, 0);
            if( v26 == 4 )
            {
                if( surfFormat != gcvSURF_A8R8G8B8 )
                {
                    if ( surfFormat > gcvSURF_A8R8G8B8 )
                    {
                        if ( surfFormat != gcvSURF_A8B8G8R8 )
                        {
                            if ( surfFormat > gcvSURF_A8B8G8R8 )
                            {
                                if ( (unsigned int)(surfFormat - gcvSURF_YUY2) <= 7 )
                                {
                                    surfFormat = gcvSURF_YUY2;
                                    goto LABEL_130;
                                }
                            }
                            else if ( surfFormat == gcvSURF_X8B8G8R8 )
                            {
                                surfFormat = gcvSURF_X8R8G8B8;
                                goto LABEL_130;
                            }
                        }
                    }
                    else
                    {
                        switch( surfFormat )
                        {
                            case gcvSURF_R5G5B5A1:
                                surfFormat = gcvSURF_A1R5G5B5;
                                goto LABEL_130;

                            case gcvSURF_R5G6B5:
                                goto LABEL_130;

                            case gcvSURF_R4G4B4A4:
                                surfFormat = gcvSURF_A4R4G4B4;
                                goto LABEL_130;
                        }
                    }
                }
                surfFormat = gcvSURF_A8R8G8B8;
            }
            else if( v26 == 3 )
            {
                status = gcoTEXTURE_GetClosestFormat(0, surfFormat, &surfFormat);
                if( status != gcvSTATUS_OK )
                {
                    goto LABEL_149;
                }
            }
LABEL_130:
            if( Usage & GRALLOC_USAGE_PROTECTED )
            {
                v26 |= 0x8000;
            }
            status = gcoSURF_Construct(0, dirtyWidth, dirtyHeight, 1, (gceSURF_TYPE)v26, surfFormat, Pool, &surface);
            if( status >= 0 || Pool == gcvPOOL_CONTIGUOUS
               && (Pool = gcvPOOL_VIRTUAL, gcoSURF_Construct(0, dirtyWidth, dirtyHeight, 1, (gceSURF_TYPE)v26, surfFormat, Pool, &surface)) >= 0 )
            {
                if( (Usage & (GRALLOC_USAGE_RENDERSCRIPT|GRALLOC_USAGE_FOREIGN_BUFFERS|GRALLOC_USAGE_MRVL_PRIVATE_1)) == (GRALLOC_USAGE_MRVL_PRIVATE_1|GRALLOC_USAGE_FOREIGN_BUFFERS) )
                {
                    status = gcoSURF_Unlock(surface, 0);
                    if( status != gcvSTATUS_OK )
                    {
                        goto LABEL_149;
                    }
                    setHwType71D0(0);
                    status = gcoSURF_Lock(surface, 0, 0);
                    if( status != gcvSTATUS_OK )
                    {
                        goto LABEL_149;
                    }
                    setHwType71D0(Usage);
                }
                if( v25 <= 1 || gcoSURF_SetSamples(surface, 4) >= 0 )
                {
                    status = gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE);
                    if( status != gcvSTATUS_OK )
                    {
                        v28 = 0;
                        v29 = 0;
                        v15 = 0;
                        v81 = 0;
                        surface->totalSize[50] = 1;
                        buffFlags = 0;
                        goto LABEL_151;
                    }
                }
            }
        }
LABEL_149:
        hnd = 0;
        v15 = 0;
        goto ON_ERROR;
    }

    v15 = 0;
    ALOGE("error: HAL_PIXEL_FORMAT %x\n", Format);
    status = (gceSTATUS)-13;
    masterFd = -1;

ON_ERROR:
    if( surface )
        gcoSURF_Destroy(surface);

    if( hnd )
        delete hnd;

    if( fd > 0 )
        close(fd);

    if( masterFd > 0 )
    {
        if( v15 )
            mvmem_free(masterFd);
        else
            close(masterFd);
    }

    v42 = -EINVAL;

LABEL_109:
    gcoHAL_SetHardwareType(0, hwtype);

    return v42;
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
int
gc_gralloc_alloc(
    alloc_device_t * Dev,
    int Width,
    int Height,
    int Format,
    int Usage,
    buffer_handle_t * Handle,
    int * Stride)
{
    int err;

    if (!Handle || !Stride)
    {
        return -EINVAL;
    }

    err = gc_gralloc_alloc_buffer(Dev, Width, Height, Format, Usage, Handle, Stride);

    return err;
}


int setHwType71D0(int AllocUsage)
{
    static gceHARDWARE_TYPE hwtype = gcvHARDWARE_INVALID;
    int buffer[81];

    if( hwtype == 0 )
    {
        buffer[0] = 39;
        gcoOS_DeviceControl(NULL, 30000, buffer, 80*sizeof(int), buffer, 80*sizeof(int));
        for( int i = 0; i < buffer[8]; ++i )
        {
            switch( buffer[i+9] )
            {
                case gcvHARDWARE_3D: hwtype = gcvHARDWARE_3D; break;
                case gcvHARDWARE_3D2D: hwtype = gcvHARDWARE_3D2D; break;
                case gcvHARDWARE_2D:
                    if( (hwtype & ~gcvHARDWARE_VG) )
                        hwtype = gcvHARDWARE_2D;
                    break;
                case gcvHARDWARE_VG:
                    if( hwtype == gcvHARDWARE_INVALID )
                        hwtype = gcvHARDWARE_VG;
            }
        }
        if( hwtype == gcvHARDWARE_INVALID )
            ALOGE("Failed to get hardware types");
    }
    if( (AllocUsage & 0x700000) == 0x600000 )
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
int
gc_gralloc_free(
    alloc_device_t * Dev,
    buffer_handle_t Handle
    )
{
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;

    if (private_handle_t::validate(Handle) < 0)
    {
        return -EINVAL;
    }

    /* Cast private buffer handle. */
    private_handle_t * hnd =
        const_cast<private_handle_t*>(reinterpret_cast<private_handle_t const *>(Handle));

    gcoHAL_GetHardwareType(0, &hwtype);
    setHwType71D0(hnd->allocUsage);
    if( hnd->base )
        gc_gralloc_unmap(hnd);

    if( hnd->surface )
    {
        if( (hnd->allocUsage & 0x700000) == 0x600000 )
        {
            setHwType71D0(0);
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
        if( hnd->fd )
            close(hnd->fd);
    }
    else
    {
        close(hnd->fd);
        close(hnd->master);
    }

    delete hnd;
    gcoHAL_SetHardwareType(0, hwtype);

    return 0;
}

int gc_gralloc_notify_change(buffer_handle_t Handle)
{
    private_handle_t *hnd = (private_handle_t*)Handle;
    if( private_handle_t::validate(hnd) )
        return -EINVAL;

    if( hnd->surface == NULL )
        return -EINVAL;

    gcoSURF_UpdateTimeStamp(hnd->surface);
    gcoSURF_PushSharedInfo(hnd->surface);

    return 0;
}

int setHwType71D4()
{
    static gceHARDWARE_TYPE hwtype = gcvHARDWARE_INVALID;
    int buffer[81];

    if( hwtype == 0 )
    {
        buffer[0] = 39;
        gcoOS_DeviceControl(0, 30000, buffer, 80*sizeof(int), buffer, 80*sizeof(int));
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

int
gc_gralloc_unwrap(buffer_handle_t Handle)
{
    gceHARDWARE_TYPE hwtype;
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

int
gc_gralloc_wrap(buffer_handle_t Handle, int w, int h, int format, int stride, int offset, void *vaddr)
{
    private_handle_t *hnd = (private_handle_t*)Handle;
    gceSTATUS status;
    gcoSURF surface = 0;
    gceSURF_FORMAT surfFormat;
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;
    gctUINT32 Address;
    gctUINT32 base_addr;
    gctSHBUF shbuf;
    gcsSURF_FORMAT_INFO_PTR formatInfo;

    if( private_handle_t::validate(hnd) )
        return -EINVAL;

    if( vaddr == NULL)
    {
        ALOGE("Invalid virtual address");
        return -EINVAL;
    }

    for( int i = 0; ; i += 2 )
    {
        if( android_formats[i] == 0 )
        {
            ALOGE("Unknown ANDROID format: %d", format);
            return -EINVAL;
        }
        if( (gceSURF_FORMAT)android_formats[i+1] == gcvSURF_UNKNOWN )
        {
            ALOGE("Not supported ANDROID format: %d", format);
            return -EINVAL;
        }

        if( format == android_formats[i] )
        {
            surfFormat = (gceSURF_FORMAT)android_formats[i+1];
            break;
        }
    }

    gcoHAL_GetHardwareType(0, &hwtype);
    status = gcoSURF_QueryFormat(surfFormat, &formatInfo);
    if( (status & 0x80000000) == 0 )
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
            if( (status&0x80000000) != gcvSTATUS_OK )
            {
                if( surface )
                    gcoSURF_Destroy(surface);
                gcoHAL_SetHardwareType(0, hwtype);
                return -EFAULT;
            }
            if( (offset - (int32_t)base_addr) < 0 )
            {
                offset = -1;
            }
            else if( ((offset - (int32_t)base_addr) + h * stride - 1) < 0 )
            {
                offset = -1;
            }
            else
            {
                offset -= (int32_t)base_addr;
            }
        }
        status = gcoSURF_Construct(0, w, h, 1, gcvSURF_BITMAP, surfFormat, gcvPOOL_USER, &surface);
        if( (status & 0x80000000) == gcvSTATUS_OK )
        {
            status = gcoSURF_SetBuffer(surface, gcvSURF_BITMAP, surfFormat, stride, vaddr, offset);
            if( (status & 0x80000000) == gcvSTATUS_OK )
            {
                status = gcoSURF_SetWindow(surface, 0, 0, w, h);
                if( (status & 0x80000000) == gcvSTATUS_OK )
                {
                    status = gcoSURF_Lock(surface, &Address, (gctPOINTER*)&base_addr);
                    if( (status & 0x80000000) == gcvSTATUS_OK )
                    {
                        status = gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE);
                        if( (status & 0x80000000) == gcvSTATUS_OK )
                        {
                            status = gcoSURF_AllocShBuffer(surface, &shbuf);
                            if( (status & 0x80000000) == gcvSTATUS_OK )
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

int
gc_gralloc_register_wrap(private_handle_t *Handle, int32_t offset, void* Vaddr)
{
    gceHARDWARE_TYPE hwtype = gcvHARDWARE_3D;
    gctUINT32 Memory;
    gceSTATUS status;
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
    if( offset == -1 )
    {
        status = gcoOS_GetBaseAddress(0, &Memory);
        if( status != gcvSTATUS_OK )
        {
            if( surface )
            {
                gcoSURF_Destroy(surface);
                gcoHAL_Commit(0, gcvFALSE);
            }
            gcoHAL_SetHardwareType(0, hwtype);
            return -EFAULT;
        }
        offset -= (int32_t)Memory;
    }
    status = gcoSURF_Construct(0, Handle->dirtyWidth, Handle->dirtyHeight, 1, gcvSURF_BITMAP, Handle->surfFormat, gcvPOOL_USER, &surface);
    if( status != gcvSTATUS_OK )
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
    if( status != gcvSTATUS_OK )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_SetWindow(surface, 0, 0, Handle->dirtyWidth, Handle->dirtyHeight);
    if( status != gcvSTATUS_OK )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_Lock(surface, &Address, (void**)&Memory);
    if( status != gcvSTATUS_OK )
    {
        gcoSURF_Destroy(surface);
        gcoHAL_Commit(0, gcvFALSE);
        gcoHAL_SetHardwareType(0, hwtype);
        return -EFAULT;
    }

    status = gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE);
    if( status != gcvSTATUS_OK )
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
