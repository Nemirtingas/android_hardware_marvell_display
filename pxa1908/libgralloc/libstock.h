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
               
#ifndef __LIBSTOCK_H
#define __LIBSTOCK_H

struct hw_module_t;
struct private_module_t;
struct hw_device_t;
struct native_handle;
typedef native_handle native_handle_t;
typedef const native_handle_t* buffer_handle_t;
struct gralloc_module_t;
struct android_ycbcr;

#include <dlfcn.h>

class libstock
{
    void *_lib;

    private_module_t *HMI;

    libstock();

    template<typename T>
    void dlsym(T& func, const char* name)
    {
        func = (T)::dlsym(_lib, name);
    }

public:
    ~libstock();
    static libstock& Inst();

    int (*gralloc_device_open)(const hw_module_t* module, const char* name,
    hw_device_t** device);

    int (*gralloc_close)(struct hw_device_t * dev);

    int (*gralloc_alloc)(struct alloc_device_t* dev, int w, int h, int format, int usage, buffer_handle_t* handle, int* stride);

    int (*gralloc_alloc_framebuffer)(alloc_device_t *device, int w, int h, int format, int usage, buffer_handle_t* pHandle, int *pStride);

    int (*gralloc_free)(struct alloc_device_t* dev,
    buffer_handle_t handle);

    int (*gralloc_register_buffer)(gralloc_module_t const* module,
        buffer_handle_t handle);

    int (*gralloc_unregister_buffer)(gralloc_module_t const* module,
        buffer_handle_t handle);

    int (*gralloc_lock)(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr);

    int (*gralloc_lock_ycbcr)(gralloc_module_t const* module, buffer_handle_t handle,
        int usage, int l, int t, int w, int h, android_ycbcr *ycbcr );

    int (*gralloc_unlock)(gralloc_module_t const* module,
        buffer_handle_t handle);

    int (*gralloc_perform)(struct gralloc_module_t const* module,
        int operation, ... );


    int (*fb_device_open)(const hw_module_t* module, const char* name, hw_device_t** device);
    int (*fb_close)(struct hw_device_t * Dev);
    int (*fb_post)(struct framebuffer_device_t * Dev, buffer_handle_t Buffer);
    int (*fb_setSwapInterval)(struct framebuffer_device_t * Dev,int Interval);


    int (*gc_gralloc_alloc)(alloc_device_t * Dev,int Width,int Height,int Format,int Usage,buffer_handle_t * Handle,int * Stride);
    int (*gc_gralloc_wrap)(buffer_handle_t Handle, int w, int h, int format, int stride, int offset, void *vaddr);
    int (*gc_gralloc_lock)(gralloc_module_t const* Module, buffer_handle_t Handle,int Usage, int Left, int Top, int Width, int Height, void ** Vaddr);
    int (*gc_gralloc_lock_ycbcr)(gralloc_module_t const * Module,  buffer_handle_t Handle, int Usage, int Left, int Top, int Width, int Height, android_ycbcr *ycbcr);
};

#endif // __LIBSTOCK_H
