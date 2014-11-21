
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := mkbootimg.c
LOCAL_STATIC_LIBRARIES := libmincrypt
LOCAL_CFLAGS := -Werror

LOCAL_MODULE := mkbootimg

ifneq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS := -DTARGET_ROCHCHIP_RECOVERY=true
else
LOCAL_CFLAGS := -DTARGET_ROCHCHIP_RECOVERY=false
endif

include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,dist_files,$(LOCAL_BUILT_MODULE))
