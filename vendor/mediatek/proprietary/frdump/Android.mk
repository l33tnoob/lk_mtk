# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.


#ifeq ($(TARGET_BUILD_PDK),)

ifneq ($(TARGET_PRODUCT),generic)
ifneq ($(TARGET_SIMULATOR),true)
#ifeq ($(TARGET_ARCH),arm)
ifneq ($(MTK_EMULATOR_SUPPORT),yes)
ifeq ($(MTK_TC7_FEATURE), yes)

# factory program
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

GENERIC_CUSTOM_PATH := $(MTK_ROOT)/custom/generic/factory
HAVE_CUST_FOLDER := $(shell test -d $(MTK_PATH_CUSTOM)/factory && echo yes)

ifeq ($(HAVE_CUST_FOLDER),yes)
CUSTOM_PATH := $(MTK_PATH_CUSTOM)/factory
else
CUSTOM_PATH := $(GENERIC_CUSTOM_PATH)
endif

commands_factory_local_path := $(LOCAL_PATH)

CORE_SRC_FILES := \
	src/frdump.c \
	src/item.c \
	src/util/utils.c \
	src/util/iniconfig.c

TEST_SRC_FILES := \
	src/test/ftm.c\
	src/test/ftm_mods.c\
	src/test/ftm_touch.c\

TEST_SRC_FILES += \
	src/test/ftm_frdump.c

HAVE_CUST_INC_PATH := $(shell test -d $(MTK_PATH_CUSTOM)/factory/inc && echo yes)

ifeq ($(HAVE_CUST_INC_PATH),yes)
  $(info Apply factory custom include path for $(TARGET_DEVICE))
else
  $(info No factory custom include path for $(TARGET_DEVICE))
endif

ifeq ($(HAVE_CUST_INC_PATH),yes)
	LOCAL_CUST_INC_PATH := $(CUSTOM_PATH)/inc
else
	LOCAL_CUST_INC_PATH := $(GENERIC_CUSTOM_PATH)/inc
endif

include $(LOCAL_PATH)/src/miniui/font.mk

LOCAL_SRC_FILES := \
	$(CORE_SRC_FILES)\
	$(TEST_SRC_FILES)

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/inc/ \
	$(MTK_PATH_COMMON)/factory/inc \
	$(LOCAL_CUST_INC_PATH) \
	$(TOP)/kernel

LOCAL_MODULE := frdump

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminiui libpixelflinger_static libpng libz libcutils

LOCAL_MODULE_CLASS := EXECUTABLES
frdump_intermediates := $(call local-intermediates-dir)
export frdump_intermediates

LOCAL_C_INCLUDES += $(frdump_intermediates)/inc

LOCAL_SHARED_LIBRARIES:= libc libcutils libdl libhwm libfile_op 

LOCAL_CFLAGS += -D$(MTK_PLATFORM)

LOCAL_SHARED_LIBRARIES += libutils

include $(BUILD_EXECUTABLE)

endif    # MTK_TC7_FEATURE
#endif   # TARGET_ARCH == arm
endif	# !TARGET_SIMULATOR
endif
endif

#endif # ifeq ($(TARGET_BUILD_PDK),)
