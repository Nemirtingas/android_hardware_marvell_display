#Common headers
common_includes := $(LOCAL_PATH)/../libgralloc
#common_includes += $(LOCAL_PATH)/../liboverlay
#common_includes += $(LOCAL_PATH)/../libqdutils
common_includes += $(LOCAL_PATH)/../libhwcomposer
#common_includes += $(LOCAL_PATH)/../libexternal
#common_includes += $(LOCAL_PATH)/../libqservice
#common_includes += $(LOCAL_PATH)/../libvirtual

common_header_export_path := marvell/display

#Common libraries external to display HAL
common_libs := liblog libutils libcutils libhardware

#Common C flags
common_flags := -DDEBUG_CALC_FPS -Wno-missing-field-initializers
#TODO: Add -Werror back once all the current warnings are fixed
common_flags += -Wconversion -Wall -fvisibility=hidden -Wunused

ifeq ($(BOARD_GRAPHICS_ENABLE_3D),true)
common_flags += -DgcdENABLE_3D
endif

common_deps  :=
kernel_includes :=

#ifeq ($(call is-vendor-board-platform,mrvl),true)
# This check is to pick the kernel headers from the right location.
# If the macro above is defined, we make the assumption that we have the kernel
# available in the build tree.
# If the macro is not present, the headers are picked from hardware/mrvl/msmXXXX
# failing which, they are picked from bionic.
    common_deps += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
    kernel_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include hardware/marvell/pxa1908/original-kernel-headers 
#endif
