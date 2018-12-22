# Prebuild stock libraries
display-hals := libGAL libgcu libHWComposerGC
# HAL Libraries
display-hals += libhwcomposer libgralloc
display-hals += libmemtrack
#ifneq ($(TARGET_PROVIDES_LIBLIGHT),true)
#display-hals += liblight
#endif

include $(call all-named-subdir-makefiles,$(display-hals))
