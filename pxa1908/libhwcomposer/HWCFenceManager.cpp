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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// system/core/libsync
#include <sw_sync.h>
#include "HWCFenceManager.h"

namespace android{
HWCFenceManager::HWCFenceManager() : m_bRunning(true)
                                   , m_pEventThread(NULL)
{
    m_pEventThread = new HWCFenceTimerThread();
#if 0
    if(NO_ERROR == m_pEventThread->run("HWCFenceTimerThread", PRIORITY_URGENT_DISPLAY)){
        m_bRunning = true;
    }
#endif
}

HWCFenceManager::~HWCFenceManager()
{
    m_pEventThread.clear();
    m_bRunning = false;
}

int32_t HWCFenceManager::createFence(int64_t nFenceId)
{
    if(!m_bRunning)
        return -1;

    return m_pEventThread->createFence(nFenceId);
}

void HWCFenceManager::signalFence(int64_t nFenceId)
{
    return m_pEventThread->signalFence(nFenceId);
}

void HWCFenceManager::reset()
{
    m_pEventThread->reset();
}

void HWCFenceManager::dump(String8& result, char* buffer, int size)
{
    sprintf(buffer, "Current Fence is %lld.\n", m_pEventThread->m_nCurrentStamp);
    result.append(buffer);
}

HWCFenceTimerThread::HWCFenceTimerThread() : m_nSyncTimeLineFd(-1)
                                           , m_nCurrentStamp(0)
{
    //open("/dev/sw_sync", O_RDWR);
    reset();
}

HWCFenceTimerThread::~HWCFenceTimerThread()
{
    close(m_nSyncTimeLineFd);
}

int32_t HWCFenceTimerThread::createFence(int64_t nFenceId)
{
    if(m_nSyncTimeLineFd < 0){
        return -1;
    }

    char str[256];
    sprintf(str, "test_fence with time %lld", m_nCurrentStamp + 1);
    return sw_sync_fence_create(m_nSyncTimeLineFd, str, nFenceId);
}

void HWCFenceTimerThread::signalFence(int64_t nFenceId)
{
    if(m_nSyncTimeLineFd < 0){
        return;
    }

    int32_t nStep = nFenceId - m_nCurrentStamp;
    // already signaled!
    if(nStep <= 0)
        return;

    // time forward to signal this fence.
    int32_t err = sw_sync_timeline_inc(m_nSyncTimeLineFd, nStep);
    if (err < 0) {
        ALOGE("can't increment sync obj:");
        return;
    }

    m_nCurrentStamp += nStep;

}

void HWCFenceTimerThread::reset()
{
    m_nCurrentStamp = 0;

    if(m_nSyncTimeLineFd >= 0)
        close(m_nSyncTimeLineFd);

    m_nSyncTimeLineFd = sw_sync_timeline_create();
    if (m_nSyncTimeLineFd < 0) {
        ALOGE("ERROR: can't create sw_sync_timeline:");
    }
}

bool HWCFenceTimerThread::threadLoop()
{
    /*TODO: 
     * polling on VSYNC to update fence status.
     */
    return true;
}

}

