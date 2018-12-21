/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <errno.h>

#include <hardware/memtrack.h>

#include "memtrack_mrvl.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern struct memtrack_record record_templates[] = {
    { 0x000, MEMTRACK_FLAG_NONSECURE|MEMTRACK_FLAG_PRIVATE|MEMTRACK_FLAG_SMAPS_ACCOUNTED },
    { 0x000, MEMTRACK_FLAG_NONSECURE|MEMTRACK_FLAG_PRIVATE|MEMTRACK_FLAG_SMAPS_UNACCOUNTED },
};

extern int read_gcmem_alloc(int pid, int *rsize)
{
    char gcmem_file[128];
    char column[128];
    char line[1024];
    int size;
    FILE *file;

    snprintf(gcmem_file, 128, "/proc/driver/gcmem/gcmem-%d", pid);
    file = fopen(gcmem_file, "r");
    if( file == NULL )
        return -errno;

    do
    {
        if( !fgets(line, sizeof(line), file) )
        {
            fclose(file);
            return -1;
        }
    }
    while( sscanf(line, " %*s %20s %zu %*s\n", column, &size) != 2 || strcmp(column, "Sum") );

    *rsize = size;
    fclose(file);
    return 0;
}

extern int read_ion_debug(int pid, const char *filename, int *rsize)
{
    FILE *file;
    int res = -1;
    char line[1024];
    char *subline;
    int i;
    int sizeKB;
    int readPID;
    int offset;

    file = fopen(filename, "r");
    if( file == NULL )
        return -errno;

    while( fgets(line, sizeof(line), file) )
    {
        i = 0;
        offset = 0;
        // Find the size_KB column and the offset to pid
        sscanf(line, "%*x %zu %*x %*u %*s%n ", &sizeKB, &offset);
        if( offset )
        {
            subline = &line[offset];
            int found = 0;
            while( 1 )
            {
                offset = 0;
                sscanf(subline, " [%d %*d %*d %*[^]]]%n", &readPID, &offset);
                if( offset == 0 )
                    break;
                ++i;
                subline += offset;
                if( readPID == pid )
                    found = 1;
            }
            if( found )
            {
                res = offset;
                *rsize = sizeKB/i;
            }
        }
    }

    return res;
}

extern int mrvl_memtrack_init(const struct memtrack_module *module)
{
    if(!module)
        return -1;
    return 0;
}

extern int mrvl_memtrack_get_memory(const struct memtrack_module *module,
                                pid_t pid,
                                int type,
                                struct memtrack_record *records,
                                size_t *num_records)
{
    int nrecords;
    int res;
    int rsize = 0;
    if( !module )
        return -1;
    if( type > MEMTRACK_TYPE_GRAPHICS )
        return -EINVAL;

    nrecords = *num_records;
    if( nrecords > 2 )
        nrecords = 2;

    *num_records = 2;
    if( nrecords )
    {
        memcpy(records, &record_templates, sizeof(struct memtrack_record) * nrecords);
        if( type == MEMTRACK_TYPE_GL )
        {
            res = read_gcmem_alloc(pid, &rsize);
        }
        else
        {
            if( type == MEMTRACK_TYPE_GRAPHICS )
            {
                res = read_ion_debug(pid, "/sys/kernel/debug/ion/heaps/system_heap", &rsize);
            }
            else
            {
                res = read_ion_debug(pid, "/sys/kernel/debug/ion/heaps/carveout_heap", &rsize);
            }
        }
        if( res )
            rsize = 0;
        else
            rsize *= 1024; // Size is in KB, so convert it to B
        records[0].size_in_bytes = 0;
        if( nrecords == 2 )
            records[1].size_in_bytes = rsize;
    }
    return 0;
}

static struct hw_module_methods_t memtrack_module_methods = {
    .open = NULL,
};

struct memtrack_module HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        module_api_version: MEMTRACK_MODULE_API_VERSION_0_1,
        hal_api_version: HARDWARE_HAL_API_VERSION,
        id: MEMTRACK_HARDWARE_MODULE_ID,
        name: "Marvell Memory Tracker HAL",
        author: "The Android Open Source Project",
        methods: &memtrack_module_methods,
    },

    init: mrvl_memtrack_init,
    getMemory: mrvl_memtrack_get_memory,
};

