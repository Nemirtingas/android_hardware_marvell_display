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

#ifndef __HW_OVERLAY_COMPOSER_H__
#define __HW_OVERLAY_COMPOSER_H__

#include <ui/Rect.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include <utils/String8.h>
#include <utils/Mutex.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include "OverlayDevice.h"
#include "GcuEngine.h"


namespace android{


class OverlaySettings : public RefBase
{
    //TODO:
    // deal with parameters like device number, device node, format list, resolution, etc..
};

class HWOverlayComposer {
public:

    /*Ctor
     *
     */
    HWOverlayComposer();

    /*Dtor
     *
     */
    ~HWOverlayComposer();

    /*prepare
     *
     */
    void prepare(size_t numDisplays, hwc_display_contents_1_t** displays);

    /*set
     *
     */
    void set(size_t numDisplays, hwc_display_contents_1_t** displays);

    /*dump
     *
     */
    void dump(String8& result, char* buffer, int size, bool dumpManager = true);

    /*finishCompose
     *
     */
    void finishCompose();

    /*set FB info.
     */
    void setSourceDisplayInfo(const fb_var_screeninfo* info){
        m_pDefaultDisplayInfo = info;
    }

    bool hasOverlayComposition(){
        return m_bRunning;
    }

private:
    bool readyToRun(size_t numDisplays, hwc_display_contents_1_t** displays);

    bool traverse(uint32_t nType, hwc_display_contents_1_t* layers);

    void allocateOverlay(uint32_t nType = 0);

    bool isYuv(uint32_t format);

    bool isScale(hwc_layer_1_t * layer);

    uint32_t getPixelFormat(const hwc_layer_1_t * layer);

    bool isPhyConts(hwc_layer_1_t * layer);

    // check if rgb layer qualified to go overLay
    bool isQualifiedRGBLayer(hwc_layer_1_t *layer);

    //void checkAndAllocate(hwc_layer_1_t* layer);

    bool isOverlayCandidate(hwc_layer_1_t* layer);

    void setOverlayRegion(const Rect& rect);

    ///< current support only 0x0 and 0xFF.
    void transparentizeFrameBuffer(uint32_t nAlpha);

    ///< color fill framebuffer, color : ARGB
    void colorFillFrameBuffer(uint32_t nColor);

    void colorFillLayer(hwc_layer_1_t* pLayer, const Rect& rect, uint32_t nColor);

private:
    class SortedOverlayVector : public SortedVector< hwc_layer_1_t* > {
    public:
        SortedOverlayVector()
        {}

        SortedOverlayVector(const SortedOverlayVector& rhs) : SortedVector<hwc_layer_1_t*>(rhs)
        {}

        ~SortedOverlayVector()
        {}

        virtual int do_compare(const void* lhs, const void*rhs) const {
            const hwc_layer_1_t* const &l(*reinterpret_cast<const hwc_layer_1_t* const * >(lhs));
            const hwc_layer_1_t* const &r(*reinterpret_cast<const hwc_layer_1_t* const *>(rhs));

            private_handle_t *plhandle = private_handle_t::dynamicCast(l->handle);
            private_handle_t *prhandle = private_handle_t::dynamicCast(r->handle);
            ///< we assume YUV format enum is larger than RGB format enum.
            if(plhandle->format != prhandle->format)
                return (plhandle->format - prhandle->format);

            uint32_t lz = (l->displayFrame.left - l->displayFrame.right)
                            * (l->displayFrame.bottom - l->displayFrame.top);
            uint32_t rz = (r->displayFrame.left - r->displayFrame.right)
                            * (r->displayFrame.bottom - r->displayFrame.top);
            // sorted by display frame size
            return lz - rz;
        }
    };

    typedef Vector< hwc_layer_1_t*> DrawingOverlayVector;

private:

    class DisplayData : public RefBase{
    private:
        DisplayData(uint32_t nType) : m_nOverlayDevices(1)
                                    , m_pOverlayDevice(NULL)
                                    , m_pOverlaySettings(NULL)
                                    , m_drawingOverlayRect()
                                    , m_currentOverlayRect()
        {
            m_pOverlayDevice = new OverlayDevice(nType);
        }

        ~DisplayData(){
            m_pOverlayDevice.clear();
            m_pOverlaySettings.clear();
        }
        
        friend class HWOverlayComposer;
    private:
        ///< the overlay device number in this channel.
        const uint32_t m_nOverlayDevices;

        ///< device abstraction.
        sp<OverlayDevice>  m_pOverlayDevice;

        ///< overlay settings.
        sp<OverlaySettings> m_pOverlaySettings;

        ///< drawing overlays.
        DrawingOverlayVector m_vDrawingOverlay;

        ///< drawing overlay rects.
        Rect m_drawingOverlayRect;

        ///< current overlays.
        DrawingOverlayVector m_vCurrentOverlay;

        ///< current overlay rects.
        Rect m_currentOverlayRect;
    };

private:
    ///< how many path have overlay.
    const uint32_t m_nOverlayChannel;

    ///< running status.
    bool m_bRunning;

    ///< defer close
    bool m_bDeferredClose;

    ///< DisplayData array for primary & HDMI which have overlay support.
    Vector<sp<DisplayData> > m_vDisplayData;

    ///< talk to base layer.
    sp<IDisplayEngine> m_pBaseDisplayEngine;

    ///< fb info, for resolution etc.
    const fb_var_screeninfo* m_pDefaultDisplayInfo;

    ///< current compositor target layer.
    hwc_layer_1_t* m_pPrimaryFbLayer;

    ///< gcu engine interface
    GcuEngine* m_pGcuEngine;

    ///< lock to protect sf thread and event call back thread (from dms to indicate overlay caps change)
    Mutex mLock;

    ///< debug use: clear the base to see flickering easily.
    bool m_bDebugClear;
};

}

#endif
