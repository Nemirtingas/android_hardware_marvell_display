#include "libgal.h"

#include <dlfcn.h>

#include <cutils/log.h>

libgal::libgal():_lib(dlopen("libGAL.so", RTLD_NOW))
{
    if( _lib == NULL ) ALOGE("libGAL.so is NULL");

    *(int*)&gcoHAL_GetHardwareType = (int)dlsym(_lib, "gcoHAL_GetHardwareType");
    *(int*)&gcoHAL_SetHardwareType = (int)dlsym(_lib, "gcoHAL_SetHardwareType");
    *(int*)&gcoHAL_Commit = (int)dlsym(_lib, "gcoHAL_Commit");
    *(int*)&gcoHAL_ImportVideoMemory = (int)dlsym(_lib, "gcoHAL_ImportVideoMemory");
    *(int*)&gcoHAL_QueryPixelPipesInfo = (int)dlsym(_lib, "gcoHAL_QueryPixelPipesInfo");
    *(int*)&gcoHAL_NameVideoMemory = (int)dlsym(_lib, "gcoHAL_NameVideoMemory");
    *(int*)&gcoOS_ModuleConstructor = (int)dlsym(_lib, "gcoOS_ModuleConstructor");

    *(int*)&gcoOS_GetBaseAddress = (int)dlsym(_lib, "gcoOS_GetBaseAddress");
    *(int*)&gcoOS_DeviceControl = (int)dlsym(_lib, "gcoOS_DeviceControl");
    *(int*)&gcoOS_DestroySignal = (int)dlsym(_lib, "gcoOS_DestroySignal");
    *(int*)&gcoOS_UnmapSignal = (int)dlsym(_lib, "gcoOS_UnmapSignal");
    *(int*)&gcoOS_WaitSignal = (int)dlsym(_lib, "gcoOS_WaitSignal");
    *(int*)&gcoOS_MapSignal = (int)dlsym(_lib, "gcoOS_MapSignal");

    *(int*)&gcoSURF_Lock = (int)dlsym(_lib, "gcoSURF_Lock");
    *(int*)&gcoSURF_Unlock = (int)dlsym(_lib, "gcoSURF_Unlock");
    *(int*)&gcoSURF_Destroy = (int)dlsym(_lib, "gcoSURF_Destroy");
    *(int*)&gcoSURF_UpdateTimeStamp = (int)dlsym(_lib, "gcoSURF_UpdateTimeStamp");
    *(int*)&gcoSURF_PushSharedInfo = (int)dlsym(_lib, "gcoSURF_PushSharedInfo");
    *(int*)&gcoSURF_CPUCacheOperation = (int)dlsym(_lib, "gcoSURF_CPUCacheOperation");
    *(int*)&gcoSURF_GetAlignedSize = (int)dlsym(_lib, "gcoSURF_GetAlignedSize");
    *(int*)&gcoSURF_QueryFormat = (int)dlsym(_lib, "gcoSURF_QueryFormat");
    *(int*)&gcoSURF_Construct = (int)dlsym(_lib, "gcoSURF_Construct");
    *(int*)&gcoSURF_SetBuffer = (int)dlsym(_lib, "gcoSURF_SetBuffer");
    *(int*)&gcoSURF_SetWindow = (int)dlsym(_lib, "gcoSURF_SetWindow");
    *(int*)&gcoSURF_SetFlags = (int)dlsym(_lib, "gcoSURF_SetFlags");
    *(int*)&gcoSURF_AllocShBuffer = (int)dlsym(_lib, "gcoSURF_AllocShBuffer");
    *(int*)&gcoSURF_BindShBuffer = (int)dlsym(_lib, "gcoSURF_BindShBuffer");
    *(int*)&gcoSURF_MapUserSurface = (int)dlsym(_lib, "gcoSURF_MapUserSurface");
    *(int*)&gcoSURF_SetSamples = (int)dlsym(_lib, "gcoSURF_SetSamples");
    *(int*)&gcoSURF_QueryVidMemNode = (int)dlsym(_lib, "gcoSURF_QueryVidMemNode");
    *(int*)&gcoSURF_SetRect = (int)dlsym(_lib, "gcoSURF_SetRect");

    *(int*)&gcoTEXTURE_GetClosestFormat = (int)dlsym(_lib, "gcoTEXTURE_GetClosestFormat");




    if( gcoHAL_GetHardwareType     == NULL ) ALOGE("gcoHAL_GetHardwareType is NULL");
    if( gcoHAL_SetHardwareType     == NULL ) ALOGE("gcoHAL_SetHardwareType is NULL");
    if( gcoHAL_Commit              == NULL ) ALOGE("gcoHAL_Commit is NULL");
    if( gcoHAL_ImportVideoMemory   == NULL ) ALOGE("gcoHAL_ImportVideoMemory is NULL");
    if( gcoHAL_QueryPixelPipesInfo == NULL ) ALOGE("gcoHAL_QueryPixelPipesInfo is NULL");
    if( gcoHAL_NameVideoMemory     == NULL ) ALOGE("gcoHAL_NameVideoMemory is NULL");

    if( gcoOS_ModuleConstructor   == NULL ) ALOGE("gcoOS_ModuleConstructor is NULL");
    if( gcoOS_GetBaseAddress      == NULL ) ALOGE("gcoOS_GetBaseAddress is NULL");
    if( gcoOS_DeviceControl       == NULL ) ALOGE("gcoOS_DeviceControl is NULL");
    if( gcoOS_DestroySignal       == NULL ) ALOGE("gcoOS_DestroySignal is NULL");
    if( gcoOS_UnmapSignal         == NULL ) ALOGE("gcoOS_UnmapSignal is NULL");
    if( gcoOS_WaitSignal          == NULL ) ALOGE("gcoOS_WaitSignal is NULL");
    if( gcoOS_MapSignal           == NULL ) ALOGE("gcoOS_MapSignal is NULL");

    if( gcoSURF_Lock              == NULL ) ALOGE("gcoSURF_Lock is NULL");
    if( gcoSURF_Unlock            == NULL ) ALOGE("gcoSURF_Unlock is NULL");
    if( gcoSURF_Destroy           == NULL ) ALOGE("gcoSURF_Destroy is NULL");
    if( gcoSURF_UpdateTimeStamp   == NULL ) ALOGE("gcoSURF_UpdateTimeStamp is NULL");
    if( gcoSURF_PushSharedInfo    == NULL ) ALOGE("gcoSURF_PushSharedInfo is NULL");
    if( gcoSURF_CPUCacheOperation == NULL ) ALOGE("gcoSURF_CPUCacheOperation is NULL");
    if( gcoSURF_GetAlignedSize    == NULL ) ALOGE("gcoSURF_GetAlignedSize is NULL");
    if( gcoSURF_QueryFormat       == NULL ) ALOGE("gcoSURF_QueryFormat is NULL");
    if( gcoSURF_Construct         == NULL ) ALOGE("gcoSURF_Construct is NULL");
    if( gcoSURF_SetBuffer         == NULL ) ALOGE("gcoSURF_SetBuffer is NULL");
    if( gcoSURF_SetWindow         == NULL ) ALOGE("gcoSURF_SetWindow is NULL");
    if( gcoSURF_SetFlags          == NULL ) ALOGE("gcoSURF_SetFlags is NULL");
    if( gcoSURF_AllocShBuffer     == NULL ) ALOGE("gcoSURF_AllocShBuffer is NULL");
    if( gcoSURF_BindShBuffer      == NULL ) ALOGE("gcoSURF_BindShBuffer is NULL");
    if( gcoSURF_MapUserSurface    == NULL ) ALOGE("gcoSURF_MapUserSurface is NULL");
    if( gcoSURF_SetSamples        == NULL ) ALOGE("gcoSURF_SetSamples is NULL");
    if( gcoSURF_QueryVidMemNode   == NULL ) ALOGE("gcoSURF_QueryVidMemNode is NULL");
    if( gcoSURF_SetRect           == NULL ) ALOGE("gcoSURF_SetRect is NULL");

    if( gcoTEXTURE_GetClosestFormat == NULL ) ALOGE("gcoTEXTURE_GetClosestFormat is NULL");
}

libgal::~libgal()
{
    dlclose(_lib);
}
