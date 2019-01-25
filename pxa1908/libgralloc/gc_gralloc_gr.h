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

#ifndef __gc_gralloc_gr_h_
#define __gc_gralloc_gr_h_

#include <stdint.h>
// #include <asm/page.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <hardware/gralloc.h>
#include <pthread.h>
#include <errno.h>
#include <cutils/native_handle.h>

#include <mvmem.h>

#include <gc_hal_enum.h>
#include <gc_hal_base.h>
#include <gc_hal_engine.h>
#include "gralloc_priv.h"

struct private_module_t;
struct private_handle_t;

#define _ALIGN( n, align_dst ) ( (n + (align_dst-1)) & ~(align_dst-1) )

inline size_t
roundUpToPageSize(size_t x)
{
    return (x + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
}

/* Framebuffer pixel format. */
extern int format;

int
mapFrameBufferLocked(
    struct private_module_t * Module
    );

int
gc_gralloc_map(
    buffer_handle_t Handle,
    void** Vaddr
    );

int
gc_gralloc_unmap(
    buffer_handle_t Handle
    );

int
gc_gralloc_alloc(
    alloc_device_t * Dev,
    int Width,
    int Height,
    int Format,
    int Usage,
    buffer_handle_t * Handle,
    int * Stride
    );

int
gc_gralloc_free(
    alloc_device_t * Dev,
    buffer_handle_t Handle
    );

int
gc_gralloc_register_buffer(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    );

int
gc_gralloc_unregister_buffer(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    );

int
gc_gralloc_lock(
    gralloc_module_t const * Module,
    buffer_handle_t Handle,
    int Usage,
    int Left,
    int Top,
    int Width,
    int Height,
    void ** Vaddr
    );

int
gc_gralloc_lock_ycbcr(
    gralloc_module_t const * Module,
    buffer_handle_t Handle,
    int Usage,
    int Left,
    int Top,
    int Width,
    int Height,
    android_ycbcr *ycbcr
    );

int
gc_gralloc_unlock(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    );

int
gc_gralloc_perform(
    gralloc_module_t const * Module,
    int Operation,
    ...
    );

int
gc_gralloc_flush(
    buffer_handle_t Handle,
    uint32_t flags
    );

int
gc_gralloc_wrap(buffer_handle_t handle, int w, int h, int format, int stride, int offset, void *base);

int
gc_gralloc_unwrap(buffer_handle_t Handle);

int
gc_gralloc_register_wrap(private_handle_t *Handle, size_t offset, void* base);

int
gc_gralloc_notify_change(buffer_handle_t Handle);

int setHwType71D0(int AllocUsage);

#endif /* __gc_gralloc_gr_h_ */
