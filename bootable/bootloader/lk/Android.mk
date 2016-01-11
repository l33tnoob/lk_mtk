LOCAL_PATH := $(call my-dir)
LK_ROOT_DIR := $(PWD)

$(info zzytest LK_ROOT_DIR=$(LK_ROOT_DIR))

ifdef LK_PROJECT
    $(info zzytest LK_PROJECT=$(LK_PROJECT))
    LK_DIR := $(LOCAL_PATH)
    INSTALLED_LK_TARGET := $(PRODUCT_OUT)/lk.bin 
    INSTALLED_LOGO_TARGET := $(PRODUCT_OUT)/logo.bin

LOCAL_BOOTLOADER_ARGS:=

ifeq ($(HTC_AA_HBOOT_MODE),0)
LOCAL_BOOTLOADER_ARGS += HTC_BUILD_MODE=SHIP_BUILD
else ifeq ($(HTC_AA_HBOOT_MODE),1)
LOCAL_BOOTLOADER_ARGS += HTC_BUILD_MODE=MFG_BUILD
else
LOCAL_BOOTLOADER_ARGS += HTC_BUILD_MODE=ENG_BUILD
endif

# Security - tamper detection feature
ifeq ($(shell echo $(HTC_HEP_FLAG) | tr A-Z a-z),true)
LOCAL_BOOTLOADER_ARGS += TAMPER_DETECT=1
else
LOCAL_BOOTLOADER_ARGS += TAMPER_DETECT=0
endif

  ifeq ($(wildcard $(TARGET_PREBUILT_LK)),)
    TARGET_LK_OUT ?= $(if $(filter /% ~%,$(TARGET_OUT_INTERMEDIATES)),,$(LK_ROOT_DIR)/)$(TARGET_OUT_INTERMEDIATES)/BOOTLOADER_OBJ
    BUILT_LK_TARGET := $(TARGET_LK_OUT)/build-$(LK_PROJECT)/lk.bin
    ifeq ($(LK_CROSS_COMPILE),)
    ifeq ($(TARGET_ARCH), arm)
#      LK_CROSS_COMPILE := $(LK_ROOT_DIR)/$(TARGET_TOOLS_PREFIX)
    else ifeq ($(TARGET_2ND_ARCH), arm)
#      LK_CROSS_COMPILE := $(LK_ROOT_DIR)/$($(TARGET_2ND_ARCH_VAR_PREFIX)TARGET_TOOLS_PREFIX)
    endif
    endif
    LK_MAKE_OPTION := $(if $(SHOW_COMMANDS),NOECHO=) $(if $(LK_CROSS_COMPILE),TOOLCHAIN_PREFIX=$(LK_CROSS_COMPILE)) BOOTLOADER_OUT=$(TARGET_LK_OUT) ROOTDIR=$(LK_ROOT_DIR)

$(BUILT_LK_TARGET): $(MTK_PROJECT_CONFIG) FORCE
	$(hide) mkdir -p $(dir $@)
	$(MAKE) -C $(LK_DIR) $(LK_MAKE_OPTION) $(LK_PROJECT) $(LOCAL_BOOTLOADER_ARGS)

$(TARGET_PREBUILT_LK): $(BUILT_LK_TARGET)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(BUILT_LK_TARGET) $@
	$(hide) cp -f $(dir $(BUILT_LK_TARGET))logo.bin $(dir $@)logo.bin

  else
    BUILT_LK_TARGET := $(TARGET_PREBUILT_LK)
  endif#TARGET_PREBUILT_LK

$(INSTALLED_LK_TARGET): $(BUILT_LK_TARGET)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(BUILT_LK_TARGET) $(INSTALLED_LK_TARGET)

$(INSTALLED_LOGO_TARGET): $(BUILT_LK_TARGET)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(dir $(BUILT_LK_TARGET))logo.bin $(INSTALLED_LOGO_TARGET)

.PHONY: lk save-lk clean-lk
droidcore: $(INSTALLED_LK_TARGET) $(INSTALLED_LOGO_TARGET)
all_modules: $(INSTALLED_LK_TARGET) $(INSTALLED_LOGO_TARGET)
lk: $(INSTALLED_LK_TARGET) $(INSTALLED_LOGO_TARGET)
save-lk: $(TARGET_PREBUILT_LK)

clean-lk:
	$(hide) rm -rf $(INSTALLED_LK_TARGET) $(INSTALLED_LOGO_TARGET) $(TARGET_LK_OUT)

droid: check-lk-config
check-mtk-config: check-lk-config
check-lk-config:
	-python device/mediatek/build/build/tools/check_kernel_config.py -c $(MTK_TARGET_PROJECT_FOLDER)/ProjectConfig.mk -l bootable/bootloader/lk/project/$(LK_PROJECT).mk -p $(MTK_PROJECT_NAME)

endif#LK_PROJECT
