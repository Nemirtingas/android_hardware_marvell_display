display-hals := libGAL libgralloc
#display-hals := libgralloc libcopybit libvirtual
#display-hals += libhwcomposer liboverlay libqdutils libexternal libqservice
display-hals += libmemtrack
#ifneq ($(TARGET_PROVIDES_LIBLIGHT),true)
#display-hals += liblight
#endif

include $(call all-named-subdir-makefiles,$(display-hals))
