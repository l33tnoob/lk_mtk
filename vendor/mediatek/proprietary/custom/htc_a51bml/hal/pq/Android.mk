LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    cust_pq_main.cpp \
    cust_tdshp.cpp \
    cust_pqds.cpp \
    cust_pqdc.cpp \
    cust_color.cpp \
    cust_gamma.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \

LC_MTK_PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(LC_MTK_PLATFORM)/hardware/pq \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(LC_MTK_PLATFORM)/kernel/drivers/dispsys \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/inc \


LOCAL_MODULE:= libpq_cust

include $(BUILD_SHARED_LIBRARY)
