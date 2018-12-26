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
               
#ifndef LIBGAL_H_
#define LIBGAL_H_

#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long

#include <stdint.h>

#define gcvFALSE                0
#define gcvTRUE                 1
#define gcvINFINITE             ((gctUINT32) ~0U)

#define gcvNULL NULL

typedef int gctBOOL;
typedef gctBOOL *gctBOOL_PTR;
typedef int gctINT;

typedef int8_t gctINT8;
typedef int16_t gctINT16;
typedef int32_t gctINT32;
typedef int64_t gctINT64;

typedef gctINT *gctINT_PTR;
typedef gctINT8 *gctINT8_PTR;
typedef gctINT16 *gctINT16_PTR;
typedef gctINT32 *gctINT32_PTR;
typedef gctINT64 *gctINT64_PTR;

typedef unsigned int gctUINT;

typedef uint8_t gctUINT8;
typedef uint16_t gctUINT16;
typedef uint32_t gctUINT32;
typedef uint64_t gctUINT64;

typedef gctUINT *gctUINT_PTR;
typedef gctUINT8 *gctUINT8_PTR;
typedef gctUINT16 *gctUINT16_PTR;
typedef gctUINT32 *gctUINT32_PTR;
typedef gctUINT64 *gctUINT64_PTR;

typedef size_t gctSIZE_T;
typedef gctSIZE_T *gctSIZE_T_PTR;
typedef struct _gcoOS *gcoOS;
typedef void *gctPOINTER;

struct _gcoSURF
{
    int32_t totalSize[176];
};

typedef struct _gcoSURF *gcoSURF;
typedef void* gctSIGNAL;
typedef void *                  gctSHBUF;

typedef struct _gcsSURF_FORMAT_INFO *   gcsSURF_FORMAT_INFO_PTR;
typedef struct _gcoHAL *gcoHAL;

typedef union  _gcuVIDMEM_NODE *        gcuVIDMEM_NODE_PTR;

typedef enum _gceSTATUS
{
    gcvSTATUS_OK                    =   0,
    gcvSTATUS_FALSE                 =   0,
    gcvSTATUS_TRUE                  =   1,
    gcvSTATUS_NO_MORE_DATA          =   2,
    gcvSTATUS_CACHED                =   3,
    gcvSTATUS_MIPMAP_TOO_LARGE      =   4,
    gcvSTATUS_NAME_NOT_FOUND        =   5,
    gcvSTATUS_NOT_OUR_INTERRUPT     =   6,
    gcvSTATUS_MISMATCH              =   7,
    gcvSTATUS_MIPMAP_TOO_SMALL      =   8,
    gcvSTATUS_LARGER                =   9,
    gcvSTATUS_SMALLER               =   10,
    gcvSTATUS_CHIP_NOT_READY        =   11,
    gcvSTATUS_NEED_CONVERSION       =   12,
    gcvSTATUS_SKIP                  =   13,
    gcvSTATUS_DATA_TOO_LARGE        =   14,
    gcvSTATUS_INVALID_CONFIG        =   15,
    gcvSTATUS_CHANGED               =   16,
    gcvSTATUS_NOT_SUPPORT_DITHER    =   17,
    gcvSTATUS_EXECUTED              =   18,
    gcvSTATUS_TERMINATE             =   19,

    gcvSTATUS_INVALID_ARGUMENT      =   -1,
    gcvSTATUS_INVALID_OBJECT        =   -2,
    gcvSTATUS_OUT_OF_MEMORY         =   -3,
    gcvSTATUS_MEMORY_LOCKED         =   -4,
    gcvSTATUS_MEMORY_UNLOCKED       =   -5,
    gcvSTATUS_HEAP_CORRUPTED        =   -6,
    gcvSTATUS_GENERIC_IO            =   -7,
    gcvSTATUS_INVALID_ADDRESS       =   -8,
    gcvSTATUS_CONTEXT_LOSSED        =   -9,
    gcvSTATUS_TOO_COMPLEX           =   -10,
    gcvSTATUS_BUFFER_TOO_SMALL      =   -11,
    gcvSTATUS_INTERFACE_ERROR       =   -12,
    gcvSTATUS_NOT_SUPPORTED         =   -13,
    gcvSTATUS_MORE_DATA             =   -14,
    gcvSTATUS_TIMEOUT               =   -15,
    gcvSTATUS_OUT_OF_RESOURCES      =   -16,
    gcvSTATUS_INVALID_DATA          =   -17,
    gcvSTATUS_INVALID_MIPMAP        =   -18,
    gcvSTATUS_NOT_FOUND             =   -19,
    gcvSTATUS_NOT_ALIGNED           =   -20,
    gcvSTATUS_INVALID_REQUEST       =   -21,
    gcvSTATUS_GPU_NOT_RESPONDING    =   -22,
    gcvSTATUS_TIMER_OVERFLOW        =   -23,
    gcvSTATUS_VERSION_MISMATCH      =   -24,
    gcvSTATUS_LOCKED                =   -25,
    gcvSTATUS_INTERRUPTED           =   -26,
    gcvSTATUS_DEVICE                =   -27,
    gcvSTATUS_NOT_MULTI_PIPE_ALIGNED =   -28,

    /* Linker errors. */
    gcvSTATUS_GLOBAL_TYPE_MISMATCH              =   -1000,
    gcvSTATUS_TOO_MANY_ATTRIBUTES               =   -1001,
    gcvSTATUS_TOO_MANY_UNIFORMS                 =   -1002,
    gcvSTATUS_TOO_MANY_VARYINGS                 =   -1003,
    gcvSTATUS_UNDECLARED_VARYING                =   -1004,
    gcvSTATUS_VARYING_TYPE_MISMATCH             =   -1005,
    gcvSTATUS_MISSING_MAIN                      =   -1006,
    gcvSTATUS_NAME_MISMATCH                     =   -1007,
    gcvSTATUS_INVALID_INDEX                     =   -1008,
    gcvSTATUS_UNIFORM_MISMATCH                  =   -1009,
    gcvSTATUS_UNSAT_LIB_SYMBOL                  =   -1010,
    gcvSTATUS_TOO_MANY_SHADERS                  =   -1011,
    gcvSTATUS_LINK_INVALID_SHADERS              =   -1012,
    gcvSTATUS_CS_NO_WORKGROUP_SIZE              =   -1013,
    gcvSTATUS_LINK_LIB_ERROR                    =   -1014,
    gcvSTATUS_SHADER_VERSION_MISMATCH           =   -1015,
    gcvSTATUS_TOO_MANY_INSTRUCTION              =   -1016,
    gcvSTATUS_SSBO_MISMATCH                     =   -1017,
    gcvSTATUS_TOO_MANY_OUTPUT                   =   -1018,
    gcvSTATUS_TOO_MANY_INPUT                    =   -1019,
    gcvSTATUS_NOT_SUPPORT_CL                    =   -1020,
    gcvSTATUS_NOT_SUPPORT_INTEGER               =   -1021,
    gcvSTATUS_UNIFORM_TYPE_MISMATCH             =   -1022,
    gcvSTATUS_TOO_MANY_SAMPLER                  =   -1023,
    /* Compiler errors. */
    gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR    =   -2000,
    gcvSTATUS_COMPILER_FE_PARSER_ERROR          =   -2001,

    /* Recompilation Errors */
    gcvSTATUS_RECOMPILER_CONVERT_UNIMPLEMENTED  =   -3000,
}
gceSTATUS;

typedef enum _gceCACHEOPERATION
{
  gcvCACHE_CLEAN = 0x1,
  gcvCACHE_INVALIDATE = 0x2,
  gcvCACHE_FLUSH = 0x3,
  gcvCACHE_MEMORY_BARRIER = 0x4,
} gceCACHEOPERATION;

typedef enum _gceHARDWARE_TYPE
{
  gcvHARDWARE_INVALID = 0x0,
  gcvHARDWARE_3D = 0x1,
  gcvHARDWARE_2D = 0x2,
  gcvHARDWARE_VG = 0x4,
  gcvHARDWARE_3D2D = 0x3,
} gceHARDWARE_TYPE;

typedef struct _gcsFORMAT_COMPONENT
{
    gctUINT8                    start;
    gctUINT8                    width;
}
gcsFORMAT_COMPONENT;

typedef enum _gceSURF_FORMAT
{
  gcvSURF_UNKNOWN = 0x0,
  gcvSURF_INDEX1 = 0x64,
  gcvSURF_INDEX4 = 0x65,
  gcvSURF_INDEX8 = 0x66,
  gcvSURF_A2R2G2B2 = 0xC8,
  gcvSURF_R3G3B2 = 0xC9,
  gcvSURF_A8R3G3B2 = 0xCA,
  gcvSURF_X4R4G4B4 = 0xCB,
  gcvSURF_A4R4G4B4 = 0xCC,
  gcvSURF_R4G4B4A4 = 0xCD,
  gcvSURF_X1R5G5B5 = 0xCE,
  gcvSURF_A1R5G5B5 = 0xCF,
  gcvSURF_R5G5B5A1 = 0xD0,
  gcvSURF_R5G6B5 = 0xD1,
  gcvSURF_R8G8B8 = 0xD2,
  gcvSURF_X8R8G8B8 = 0xD3,
  gcvSURF_A8R8G8B8 = 0xD4,
  gcvSURF_R8G8B8A8 = 0xD5,
  gcvSURF_G8R8G8B8 = 0xD6,
  gcvSURF_R8G8B8G8 = 0xD7,
  gcvSURF_X2R10G10B10 = 0xD8,
  gcvSURF_A2R10G10B10 = 0xD9,
  gcvSURF_X12R12G12B12 = 0xDA,
  gcvSURF_A12R12G12B12 = 0xDB,
  gcvSURF_X16R16G16B16 = 0xDC,
  gcvSURF_A16R16G16B16 = 0xDD,
  gcvSURF_A32R32G32B32 = 0xDE,
  gcvSURF_R8G8B8X8 = 0xDF,
  gcvSURF_R5G5B5X1 = 0xE0,
  gcvSURF_R4G4B4X4 = 0xE1,
  gcvSURF_A4B4G4R4 = 0x12C,
  gcvSURF_A1B5G5R5 = 0x12D,
  gcvSURF_B5G6R5 = 0x12E,
  gcvSURF_B8G8R8 = 0x12F,
  gcvSURF_B16G16R16 = 0x130,
  gcvSURF_X8B8G8R8 = 0x131,
  gcvSURF_A8B8G8R8 = 0x132,
  gcvSURF_A2B10G10R10 = 0x133,
  gcvSURF_X16B16G16R16 = 0x134,
  gcvSURF_A16B16G16R16 = 0x135,
  gcvSURF_B32G32R32 = 0x136,
  gcvSURF_X32B32G32R32 = 0x137,
  gcvSURF_A32B32G32R32 = 0x138,
  gcvSURF_B4G4R4A4 = 0x139,
  gcvSURF_B5G5R5A1 = 0x13A,
  gcvSURF_B8G8R8X8 = 0x13B,
  gcvSURF_B8G8R8A8 = 0x13C,
  gcvSURF_X4B4G4R4 = 0x13D,
  gcvSURF_X1B5G5R5 = 0x13E,
  gcvSURF_B4G4R4X4 = 0x13F,
  gcvSURF_B5G5R5X1 = 0x140,
  gcvSURF_X2B10G10R10 = 0x141,
  gcvSURF_DXT1 = 0x190,
  gcvSURF_DXT2 = 0x191,
  gcvSURF_DXT3 = 0x192,
  gcvSURF_DXT4 = 0x193,
  gcvSURF_DXT5 = 0x194,
  gcvSURF_CXV8U8 = 0x195,
  gcvSURF_ETC1 = 0x196,
  gcvSURF_YUY2 = 0x1F4,
  gcvSURF_UYVY = 0x1F5,
  gcvSURF_YV12 = 0x1F6,
  gcvSURF_I420 = 0x1F7,
  gcvSURF_NV12 = 0x1F8,
  gcvSURF_NV21 = 0x1F9,
  gcvSURF_NV16 = 0x1FA,
  gcvSURF_NV61 = 0x1FB,
  gcvSURF_YVYU = 0x1FC,
  gcvSURF_VYUY = 0x1FD,
  gcvSURF_D16 = 0x258,
  gcvSURF_D24S8 = 0x259,
  gcvSURF_D32 = 0x25A,
  gcvSURF_D24X8 = 0x25B,
  gcvSURF_A4 = 0x2BC,
  gcvSURF_A8 = 0x2BD,
  gcvSURF_A12 = 0x2BE,
  gcvSURF_A16 = 0x2BF,
  gcvSURF_A32 = 0x2C0,
  gcvSURF_A1 = 0x2C1,
  gcvSURF_L4 = 0x320,
  gcvSURF_L8 = 0x321,
  gcvSURF_L12 = 0x322,
  gcvSURF_L16 = 0x323,
  gcvSURF_L32 = 0x324,
  gcvSURF_L1 = 0x325,
  gcvSURF_A4L4 = 0x384,
  gcvSURF_A2L6 = 0x385,
  gcvSURF_A8L8 = 0x386,
  gcvSURF_A4L12 = 0x387,
  gcvSURF_A12L12 = 0x388,
  gcvSURF_A16L16 = 0x389,
  gcvSURF_L6V5U5 = 0x3E8,
  gcvSURF_V8U8 = 0x3E9,
  gcvSURF_X8L8V8U8 = 0x3EA,
  gcvSURF_Q8W8V8U8 = 0x3EB,
  gcvSURF_A2W10V10U10 = 0x3EC,
  gcvSURF_V16U16 = 0x3ED,
  gcvSURF_Q16W16V16U16 = 0x3EE,
  gcvSURF_R8 = 0x44C,
  gcvSURF_X8R8 = 0x44D,
  gcvSURF_G8R8 = 0x44E,
  gcvSURF_X8G8R8 = 0x44F,
  gcvSURF_A8R8 = 0x450,
  gcvSURF_R16 = 0x451,
  gcvSURF_X16R16 = 0x452,
  gcvSURF_G16R16 = 0x453,
  gcvSURF_X16G16R16 = 0x454,
  gcvSURF_A16R16 = 0x455,
  gcvSURF_R32 = 0x456,
  gcvSURF_X32R32 = 0x457,
  gcvSURF_G32R32 = 0x458,
  gcvSURF_X32G32R32 = 0x459,
  gcvSURF_A32R32 = 0x45A,
  gcvSURF_RG16 = 0x45B,
  gcvSURF_R16F = 0x4B0,
  gcvSURF_X16R16F = 0x4B1,
  gcvSURF_G16R16F = 0x4B2,
  gcvSURF_X16G16R16F = 0x4B3,
  gcvSURF_B16G16R16F = 0x4B4,
  gcvSURF_X16B16G16R16F = 0x4B5,
  gcvSURF_A16B16G16R16F = 0x4B6,
  gcvSURF_R32F = 0x4B7,
  gcvSURF_X32R32F = 0x4B8,
  gcvSURF_G32R32F = 0x4B9,
  gcvSURF_X32G32R32F = 0x4BA,
  gcvSURF_B32G32R32F = 0x4BB,
  gcvSURF_X32B32G32R32F = 0x4BC,
  gcvSURF_A32B32G32R32F = 0x4BD,
  gcvSURF_A16F = 0x4BE,
  gcvSURF_L16F = 0x4BF,
  gcvSURF_A16L16F = 0x4C0,
  gcvSURF_A16R16F = 0x4C1,
  gcvSURF_A32F = 0x4C2,
  gcvSURF_L32F = 0x4C3,
  gcvSURF_A32L32F = 0x4C4,
  gcvSURF_A32R32F = 0x4C5,
} gceSURF_FORMAT;

typedef struct _gcsFORMAT_CLASS_TYPE_DEPTH
{
    gcsFORMAT_COMPONENT         depth;
    gcsFORMAT_COMPONENT         stencil;
}
gcsFORMAT_CLASS_TYPE_DEPTH;


typedef enum _gceFORMAT_CLASS
{
    gcvFORMAT_CLASS_RGBA        = 4500,
    gcvFORMAT_CLASS_YUV,
    gcvFORMAT_CLASS_INDEX,
    gcvFORMAT_CLASS_LUMINANCE,
    gcvFORMAT_CLASS_BUMP,
    gcvFORMAT_CLASS_DEPTH,
    gcvFORMAT_CLASS_ASTC,
    gcvFORMAT_CLASS_OTHER
}
gceFORMAT_CLASS;

typedef enum _gceSURF_SWIZZLE
{
  gcvSURF_NOSWIZZLE = 0x0,
  gcvSURF_ARGB = 0x1,
  gcvSURF_ABGR = 0x2,
  gcvSURF_RGBA = 0x3,
  gcvSURF_BGRA = 0x4,
} gceSURF_SWIZZLE;

typedef struct _gcsFORMAT_CLASS_TYPE_BUMP
{
  gcsFORMAT_COMPONENT alpha;
  gcsFORMAT_COMPONENT l;
  gcsFORMAT_COMPONENT v;
  gcsFORMAT_COMPONENT u;
  gcsFORMAT_COMPONENT q;
  gcsFORMAT_COMPONENT w;
} gcsFORMAT_CLASS_TYPE_BUMP;

typedef struct _gcsFORMAT_CLASS_TYPE_LUMINANCE
{
  gcsFORMAT_COMPONENT alpha;
  gcsFORMAT_COMPONENT value;
} gcsFORMAT_CLASS_TYPE_LUMINANCE;

typedef struct _gcsFORMAT_CLASS_TYPE_INDEX
{
  gcsFORMAT_COMPONENT value;
} gcsFORMAT_CLASS_TYPE_INDEX;

typedef struct _gcsFORMAT_CLASS_TYPE_YUV
{
  gcsFORMAT_COMPONENT y;
  gcsFORMAT_COMPONENT u;
  gcsFORMAT_COMPONENT v;
} gcsFORMAT_CLASS_TYPE_YUV;

typedef struct _gcsFORMAT_CLASS_TYPE_RGBA
{
  gcsFORMAT_COMPONENT alpha;
  gcsFORMAT_COMPONENT red;
  gcsFORMAT_COMPONENT green;
  gcsFORMAT_COMPONENT blue;
} gcsFORMAT_CLASS_TYPE_RGBA;

typedef enum _gceSURF_FLAG
{
    /* None flag */
    gcvSURF_FLAG_NONE                = 0x0,
    /* content is preserved after swap */
    gcvSURF_FLAG_CONTENT_PRESERVED   = 0x1,
    /* content is updated after swap*/
    gcvSURF_FLAG_CONTENT_UPDATED     = 0x2,
    /* content is y inverted */
    gcvSURF_FLAG_CONTENT_YINVERTED   = 0x4,
    /* content is protected */
    gcvSURF_FLAG_CONTENT_PROTECTED   = 0x8,
    /* surface is contiguous. */
    gcvSURF_FLAG_CONTIGUOUS          = (1 << 4),
}
gceSURF_FLAG;

typedef enum _gceSURF_TYPE
{
  gcvSURF_TYPE_UNKNOWN = 0x0,
  gcvSURF_INDEX = 0x1,
  gcvSURF_VERTEX = 0x2,
  gcvSURF_TEXTURE = 0x3,
  gcvSURF_RENDER_TARGET = 0x4,
  gcvSURF_DEPTH = 0x5,
  gcvSURF_BITMAP = 0x6,
  gcvSURF_TILE_STATUS = 0x7,
  gcvSURF_IMAGE = 0x8,
  gcvSURF_MASK = 0x9,
  gcvSURF_SCISSOR = 0xA,
  gcvSURF_HIERARCHICAL_DEPTH = 0xB,
  gcvSURF_NUM_TYPES = 0xC,
  gcvSURF_NO_TILE_STATUS = 0x100,
  gcvSURF_NO_VIDMEM = 0x200,
  gcvSURF_CACHEABLE = 0x400,
  gcvSURF_RENDER_TARGET_NO_TILE_STATUS = 0x104,
  gcvSURF_DEPTH_NO_TILE_STATUS = 0x105,
  gcvSURF_BITMAP_NO_VIDMEM = 0x206,
  gcvSURF_TEXTURE_NO_VIDMEM = 0x203,
  gcvSURF_CACHEABLE_BITMAP_NO_VIDMEM = 0x606,
  gcvSURF_CACHEABLE_BITMAP = 0x406,
} gceSURF_TYPE;



typedef struct _gcsSURF_FORMAT_INFO
{
    gceSURF_FORMAT format;
    gceFORMAT_CLASS fmtClass;
    gctUINT8 bitsPerPixel;
    gceSURF_SWIZZLE swizzle;
    gctUINT8 interleaved;
    union
    {
        gcsFORMAT_CLASS_TYPE_BUMP bump;
        gcsFORMAT_CLASS_TYPE_RGBA rgba;
        gcsFORMAT_CLASS_TYPE_YUV yuv;
        gcsFORMAT_CLASS_TYPE_LUMINANCE lum;
        gcsFORMAT_CLASS_TYPE_INDEX index;
        gcsFORMAT_CLASS_TYPE_DEPTH depth;
    } u;
} gcsSURF_FORMAT_INFO;

typedef enum _gcePOOL
{
    gcvPOOL_UNKNOWN = 0,
    gcvPOOL_DEFAULT,
    gcvPOOL_LOCAL,
    gcvPOOL_LOCAL_INTERNAL,
    gcvPOOL_LOCAL_EXTERNAL,
    gcvPOOL_UNIFIED,
    gcvPOOL_SYSTEM,
    gcvPOOL_VIRTUAL,
    gcvPOOL_USER,
    gcvPOOL_CONTIGUOUS,
    gcvPOOL_DEFAULT_FORCE_CONTIGUOUS,
    gcvPOOL_DEFAULT_FORCE_CONTIGUOUS_CACHEABLE,
#if (MRVL_VIDEO_MEMORY_USE_TYPE != gcdMEM_TYPE_NONE)
    gcvPOOL_PMEM,
#endif
    gcvPOOL_CMDBUF,
    gcvPOOL_NUMBER_OF_POOLS
}
gcePOOL;

typedef gcsSURF_FORMAT_INFO *gcsSURF_FORMAT_INFO_PTR;

#ifdef __cplusplus
extern "C" {
#endif

gceSTATUS gcoOS_ModuleConstructor();
gceSTATUS gcoOS_DestroySignal(gcoOS Os, gctSIGNAL Signal);
gceSTATUS gcoOS_WaitSignal(gcoOS Os, gctSIGNAL Signal, gctUINT32 Wait);
gceSTATUS gcoOS_MapSignal(gctSIGNAL  RemoteSignal, gctSIGNAL * LocalSignal);
gceSTATUS gcoOS_UnmapSignal(gctSIGNAL Signal);
gceSTATUS gcoOS_DeviceControl(gcoOS Os, gctUINT32 IoControlCode, gctPOINTER InputBuffer, gctSIZE_T InputBufferSize, gctPOINTER OutputBuffer, gctSIZE_T OutputBufferSize);
gceSTATUS gcoOS_GetBaseAddress(gcoOS Os, gctUINT32_PTR BaseAddress);

gceSTATUS gcoSURF_Construct(gcoHAL Hal, gctUINT Width, gctUINT Height, gctUINT Depth, gceSURF_TYPE Type, gceSURF_FORMAT Format, gcePOOL Pool, gcoSURF * Surface);
gceSTATUS gcoSURF_Lock(gcoSURF Surface, gctUINT32 * Address, gctPOINTER * Memory);
gceSTATUS gcoSURF_Unlock(gcoSURF Surface, gctPOINTER Memory);
gceSTATUS gcoSURF_Destroy(gcoSURF Surface);
gceSTATUS gcoSURF_UpdateTimeStamp(gcoSURF Surface);
gceSTATUS gcoSURF_PushSharedInfo(gcoSURF Surface);
gceSTATUS gcoSURF_QueryFormat(gceSURF_FORMAT Format, gcsSURF_FORMAT_INFO_PTR *Info);
gceSTATUS gcoSURF_CPUCacheOperation(gcoSURF Surface, gceCACHEOPERATION op);
gceSTATUS gcoSURF_GetAlignedSize(gcoSURF Surface,gctUINT * Width,gctUINT * Height,gctINT * Stride);
gceSTATUS gcoSURF_SetBuffer(gcoSURF Surface, gceSURF_TYPE Type, gceSURF_FORMAT Format, gctUINT Stride, gctPOINTER Logical, gctUINT32 Physical);
gceSTATUS gcoSURF_SetWindow(gcoSURF Surface, gctUINT X, gctUINT Y, gctUINT Width, gctUINT Height);
gceSTATUS gcoSURF_SetFlags(gcoSURF Surface,gceSURF_FLAG Flag,gctBOOL Value);
gceSTATUS gcoSURF_AllocShBuffer(gcoSURF Surface,gctSHBUF * ShBuf);
gceSTATUS gcoSURF_BindShBuffer(gcoSURF Surface,gctSHBUF ShBuf);
gceSTATUS gcoSURF_MapUserSurface(gcoSURF Surface,gctUINT Alignment,gctPOINTER Logical,gctUINT32 Physical);
gceSTATUS gcoSURF_SetSamples(gcoSURF Surface,gctUINT Samples);
gceSTATUS gcoSURF_QueryVidMemNode(gcoSURF Surface,gcuVIDMEM_NODE_PTR * Node,gcePOOL * Pool,gctUINT_PTR Bytes);
gceSTATUS gcoSURF_SetRect(gcoSURF Surface, gctUINT Width, gctUINT Height);

gceSTATUS gcoHAL_GetHardwareType(gcoHAL Hal,  gceHARDWARE_TYPE *hwtype);
gceSTATUS gcoHAL_SetHardwareType(gcoHAL Hal,  gceHARDWARE_TYPE hwtype);
gceSTATUS gcoHAL_ImportVideoMemory(gctUINT32 Name,gctUINT32 * Handle);
gceSTATUS gcoHAL_Commit(gcoHAL Hal, gctBOOL Stall);
gceSTATUS gcoHAL_QueryPixelPipesInfo(gctUINT32 * pixelPipes,gctUINT32 * resolveAlignmentX,gctUINT32 * resolveAlignmentY);
gceSTATUS gcoHAL_NameVideoMemory(gctUINT32 Handle,gctUINT32 * Name);

gceSTATUS gcoTEXTURE_GetClosestFormat(gcoHAL Hal,gceSURF_FORMAT InFormat,gceSURF_FORMAT* OutFormat);

#define gcmIS_ERROR(status)         (status < 0)
#define gcmNO_ERROR(status)         (status >= 0)
#define gcmIS_SUCCESS(status)       (status == gcvSTATUS_OK)

#define gcmONERROR(func) \
    do \
    { \
        status = func; \
        if (gcmIS_ERROR(status)) \
        { \
            goto OnError; \
        } \
    } \
    while (gcvFALSE)

#ifdef __cplusplus
}
#endif

#endif
