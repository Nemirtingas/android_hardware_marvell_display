#include "libstock.h"

#include "gralloc_priv.h"

#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <cutils/log.h>

struct gralloc_context_t
{
alloc_device_t device;
};

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
    dlsym(gc_gralloc_lock_ycbcr, "_Z21gc_gralloc_lock_ycbcrPK16gralloc_module_tPK13native_handleiiiiiP13android_ycbcr");
    dlsym(gc_gralloc_lock, "_Z15gc_gralloc_lockPK16gralloc_module_tPK13native_handleiiiiiPPv");

    dlsym(gralloc_alloc_framebuffer, "_Z25gralloc_alloc_framebufferP14alloc_device_tiiiiPPK13native_handlePi");

    gralloc_context_t *dev;
    gralloc_device_open(0, "gpu0", (hw_device_t**)&dev);

    gralloc_close = dev->device.common.close;

    gralloc_alloc = dev->device.alloc;

    gralloc_free = dev->device.free;

    gralloc_close((hw_device_t*)dev);
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
