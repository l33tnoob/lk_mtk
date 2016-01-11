LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= htcsecure.c

LOCAL_SHARED_LIBRARIES := libcutils \
			libc

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libhtcsecure

include $(BUILD_SHARED_LIBRARY)
include $(BUILD_BSP_TO_PDK_LIB)
