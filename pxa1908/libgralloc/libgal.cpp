#include "libgal.h"

#include <dlfcn.h>

struct __libgal
{
    void *_lib;

    __libgal():_lib(dlopen("libGAL.so", RTLD_NOW))
    {
        *(int*)gcoHAL_GetHardwareType = (int)dlsym(_lib, "gcoHAL_GetHardwareType");
        *(int*)gcoHAL_SetHardwareType = (int)dlsym(_lib, "gcoHAL_SetHardwareType");
        *(int*)gcoHAL_Commit = (int)dlsym(_lib, "gcoHAL_Commit");
        *(int*)gcoHAL_ImportVideoMemory = (int)dlsym(_lib, "gcoHAL_ImportVideoMemory");
        *(int*)gcoHAL_QueryPixelPipesInfo = (int)dlsym(_lib, "gcoHAL_QueryPixelPipesInfo");
        *(int*)gcoHAL_NameVideoMemory = (int)dlsym(_lib, "gcoHAL_NameVideoMemory");

        *(int*)gcoOS_ModuleConstructor = (int)dlsym(_lib, "gcoOS_ModuleConstructor");
        *(int*)gcoOS_GetBaseAddress = (int)dlsym(_lib, "gcoOS_GetBaseAddress");
        *(int*)gcoOS_DeviceControl = (int)dlsym(_lib, "gcoOS_DeviceControl");
        *(int*)gcoOS_DestroySignal = (int)dlsym(_lib, "gcoOS_DestroySignal");
        *(int*)gcoOS_UnmapSignal = (int)dlsym(_lib, "gcoOS_UnmapSignal");
        *(int*)gcoOS_WaitSignal = (int)dlsym(_lib, "gcoOS_WaitSignal");
        *(int*)gcoOS_MapSignal = (int)dlsym(_lib, "gcoOS_MapSignal");

        *(int*)gcoSURF_Lock = (int)dlsym(_lib, "gcoSURF_Lock");
        *(int*)gcoSURF_Unlock = (int)dlsym(_lib, "gcoSURF_Unlock");
        *(int*)gcoSURF_Destroy = (int)dlsym(_lib, "gcoSURF_Destroy");
        *(int*)gcoSURF_UpdateTimeStamp = (int)dlsym(_lib, "gcoSURF_UpdateTimeStamp");
        *(int*)gcoSURF_PushSharedInfo = (int)dlsym(_lib, "gcoSURF_PushSharedInfo");
        *(int*)gcoSURF_CPUCacheOperation = (int)dlsym(_lib, "gcoSURF_CPUCacheOperation");
        *(int*)gcoSURF_GetAlignedSize = (int)dlsym(_lib, "gcoSURF_GetAlignedSize");
        *(int*)gcoSURF_QueryFormat = (int)dlsym(_lib, "gcoSURF_QueryFormat");
        *(int*)gcoSURF_Construct = (int)dlsym(_lib, "gcoSURF_Construct");
        *(int*)gcoSURF_SetBuffer = (int)dlsym(_lib, "gcoSURF_SetBuffer");
        *(int*)gcoSURF_SetWindow = (int)dlsym(_lib, "gcoSURF_SetWindow");
        *(int*)gcoSURF_SetFlags = (int)dlsym(_lib, "gcoSURF_SetFlags");
        *(int*)gcoSURF_AllocShBuffer = (int)dlsym(_lib, "gcoSURF_AllocShBuffer");
        *(int*)gcoSURF_BindShBuffer = (int)dlsym(_lib, "gcoSURF_BindShBuffer");
        *(int*)gcoSURF_MapUserSurface = (int)dlsym(_lib, "gcoSURF_MapUserSurface");
        *(int*)gcoSURF_SetSamples = (int)dlsym(_lib, "gcoSURF_SetSamples");
        *(int*)gcoSURF_QueryVidMemNode = (int)dlsym(_lib, "gcoSURF_QueryVidMemNode");
        *(int*)gcoSURF_SetRect = (int)dlsym(_lib, "gcoSURF_SetRect");

        *(int*)gcoTEXTURE_GetClosestFormat = (int)dlsym(_lib, "gcoTEXTURE_GetClosestFormat");
    }

    ~__libgal()
    {
        dlclose(_lib);
    }
} libgal;
