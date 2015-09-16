# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	builtins.c \
	init.c \
	devices.c \
	property_service.c \
	util.c \
	parser.c \
	keychords.c \
	signal_handler.c \
	init_parser.c \
	ueventd.c \
	ueventd_parser.c \
	watchdogd.c

LOCAL_CFLAGS    += -Wno-unused-parameter

ifeq ($(strip $(INIT_BOOTCHART)),true)
LOCAL_SRC_FILES += bootchart.c
LOCAL_CFLAGS    += -DBOOTCHART=1
endif

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DALLOW_LOCAL_PROP_OVERRIDE=1 -DALLOW_DISABLE_SELINUX=1
endif

# Enable ueventd logging
#LOCAL_CFLAGS += -DLOG_UEVENTS=1

ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3288)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_RK3288
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), sofia3gr)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_SOFIA3GR
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk3368)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_RK3368
endif
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), rk312x)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_RK312x
endif

ifeq ($(BUILD_WITH_NAND_EMMC),true)
LOCAL_CFLAGS += -DNAND_EMMC
endif
ifeq ($(strip $(BUILD_WITH_USER_PTEST)),true)
LOCAL_CFLAGS += -DUSER_PTEST
endif
LOCAL_MODULE:= init

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

LOCAL_STATIC_LIBRARIES := \
	libfs_mgr \
	liblogwrap \
	libcutils \
	liblog \
	libc \
	libselinux \
	libmincrypt \
	libext4_utils_static \
	libsparse_static \
	libz

LOCAL_ADDITIONAL_DEPENDENCIES += $(LOCAL_PATH)/Android.mk

include $(BUILD_EXECUTABLE)

# Make a symlink from /sbin/ueventd and /sbin/watchdogd to /init
SYMLINKS := \
	$(TARGET_ROOT_OUT)/sbin/ueventd \
	$(TARGET_ROOT_OUT)/sbin/watchdogd

$(SYMLINKS): INIT_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> ../$(INIT_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf ../$(INIT_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)
