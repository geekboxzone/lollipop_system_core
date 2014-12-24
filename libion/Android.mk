LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := ion.c
LOCAL_MODULE := libion
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/kernel-headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include $(LOCAL_PATH)/kernel-headers
LOCAL_CFLAGS := -Werror

ifeq ($(TARGET_BOARD_HARDWARE),rk30board)
LOCAL_CFLAGS := -DTARGET_ROCHCHIP_ION=true
else
LOCAL_CFLAGS := -DTARGET_ROCHCHIP_ION=false
endif
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := ion.c ion_test.c
LOCAL_MODULE := iontest
LOCAL_MODULE_TAGS := optional tests
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/kernel-headers
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_CFLAGS := -Werror
ifeq ($(TARGET_BOARD_HARDWARE),rk30board)
LOCAL_CFLAGS := -DTARGET_ROCHCHIP_ION=true
else
LOCAL_CFLAGS := -DTARGET_ROCHCHIP_ION=false
endif

include $(BUILD_EXECUTABLE)

include $(call first-makefiles-under,$(LOCAL_PATH))
