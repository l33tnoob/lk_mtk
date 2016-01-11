LOCAL_PATH:= $(call my-dir)

###############################################################################
# SEC STATIC LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libsbchk
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libsbchk.a
LOCAL_SRC_FILES_arm64 := lib64/libsbchk.a
LOCAL_MODULE_SUFFIX := .a
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

ifneq ($(CUSTOM_SEC_AUTH_SUPPORT),yes)
include $(CLEAR_VARS)
LOCAL_MODULE := libauth
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libauth.a
LOCAL_SRC_FILES_arm64 := lib64/libauth.a
LOCAL_MODULE_SUFFIX := .a
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)
endif

###############################################################################
# SEC DYNAMIC LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libsec
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libsec.so
LOCAL_SRC_FILES_arm64 := lib64/libsec.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsecdl_static 
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libsecdl_static.a
LOCAL_SRC_FILES_arm64 := lib64/libsecdl_static.a
LOCAL_MODULE_SUFFIX := .a
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libsecdl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libsecdl.so
LOCAL_SRC_FILES_arm64 := lib64/libsecdl.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_MULTILIB := both 
include $(BUILD_PREBUILT)

###############################################################################
# HEVC (OR SIMILIAR) DYNAMIC LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libmtk_cipher
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libmtk_cipher.so
LOCAL_SRC_FILES_arm64 := lib64/libmtk_cipher.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)
