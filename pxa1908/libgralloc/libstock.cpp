#include "libstock.h"

#include "gralloc_priv.h"

#include <cutils/log.h>

libstock::libstock():_lib(dlopen("/system/lib/hw/gralloc.stock.so", RTLD_NOW)),gralloc_close(NULL), gralloc_alloc(NULL),gralloc_free(NULL)
{
    //dlsym(HMI, "HMI");

    *(int*)&HMI = (int)::dlsym(_lib, "HMI");

    gralloc_device_open       = HMI->base.common.methods->open;
    gralloc_register_buffer   = HMI->base.registerBuffer;
    gralloc_unregister_buffer = HMI->base.unregisterBuffer;
    gralloc_lock              = HMI->base.lock;
    gralloc_unlock            = HMI->base.unlock;
    gralloc_perform           = HMI->base.perform;
    gralloc_lock_ycbcr        = HMI->base.lock_ycbcr;

    dlsym(fb_device_open, "_Z14fb_device_openPK11hw_module_tPKcPP11hw_device_t");

    dlsym(gc_gralloc_alloc, "_Z16gc_gralloc_allocP14alloc_device_tiiiiPPK13native_handlePi");
    dlsym(gc_gralloc_wrap, "_Z15gc_gralloc_wrapP16private_handle_tiiiimPv");

    dlsym(gralloc_alloc_framebuffer, "_Z25gralloc_alloc_framebufferP14alloc_device_tiiiiPPK13native_handlePi");

    ALOGE("%s: %p", "gralloc_device_open", gralloc_device_open);
    ALOGE("%s: %p", "gralloc_register_buffer", gralloc_register_buffer);
    ALOGE("%s: %p", "gralloc_unregister_buffer", gralloc_unregister_buffer);
    ALOGE("%s: %p", "gralloc_lock", gralloc_lock);
    ALOGE("%s: %p", "gralloc_unlock", gralloc_unlock);
    ALOGE("%s: %p", "gralloc_perform", gralloc_perform);
    ALOGE("%s: %p", "gralloc_lock_ycbcr", gralloc_lock_ycbcr);
}

libstock::~libstock()
{
    dlclose(_lib);
}

libstock& libstock::Inst()
{
    static libstock lib;
    return lib;
}
