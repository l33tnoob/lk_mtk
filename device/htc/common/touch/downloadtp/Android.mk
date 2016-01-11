LOCAL_PATH:= $(call my-dir)

#BUILD EXECUTABLE
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := downloadtp
LOCAL_COPY_HEADERS := \
	downloadtp.h

LOCAL_SRC_FILES:= \
	downloadtp.c

LOCAL_CFLAGS:= -DLOG_TAG=\"downloadtp\"

LOCAL_C_INCLUDES += bionic

LOCAL_LDLIBS := -llog

LOCAL_STATIC_LIBRARIES := \
	libc \
	liblog \
	libcutils

LOCAL_MODULE := downloadtp
#LOCAL_MODULE_STEM_64 := downloadtp64

#LOCAL_MODULE_PATH := $(TARGET_HOSD_ROOT_OUT)/sbin
#LOCAL_MODULE_PATH := $(TARGET_OUT)/bin

LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

