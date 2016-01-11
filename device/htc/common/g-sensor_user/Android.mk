ifneq ($(TARGET_DEVICE),$(filter $(TARGET_DEVICE),huangshan huashan))

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

ifeq (test,test)
LOCAL_SRC_FILES:= main.c \
	FileIO.c \
	IOMessage.c \
	$(NULL)
else
LOCAL_SRC_FILES:= lib_test.c
endif

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libstdc++ \
	libc \
	$(NULL)

LOCAL_MODULE:= bma150_usr

#include $(SSD_BUILD_DAEMON)
include $(BUILD_EXECUTABLE)

endif
