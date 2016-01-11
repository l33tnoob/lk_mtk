# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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


# *************************************************************************
# Set shell align with Android build system
# *************************************************************************
SHELL        := /bin/bash
.DELETE_ON_ERROR:

include mediatek/build/Makefile
$(call codebase-path)
#mtk-config-files := $(strip $(call mtk.config.generate-rules,mtk-config-files))
#mtk-custom-files := $(strip $(call mtk.custom.generate-rules,mtk-custom-files))
$(shell $(foreach a,$(CMD_ARGU),$(if $(filter 2,$(words $(subst =, ,$(a)))),$(a))) make -f mediatek/build/custgen.mk > /dev/null)
PRJ_MF       := $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
include $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk

ifdef OPTR_SPEC_SEG_DEF
  ifneq (NONE,$(OPTR_SPEC_SEG_DEF))
    include $(MTK_ROOT_SOURCE_OPERATOR)/OperatorInfo.mk
  endif
endif

#MKTOPDIR      =  $(shell pwd)
ifndef OUT_DIR
  OUT_DIR    :=  out
  LOGDIR      =  $(MKTOPDIR)/out/target/product
else
  LOGDIR      =  $(OUT_DIR)/target/product
  export OUT_DIR
endif


# *************************************************************************
# Set PHONY
# *************************************************************************
.PHONY : new newall remake remakeall clean cleanall \
         preloader kernel android \
         check-modem update-modem sign-image encrypt-image sign-modem check-dep \
         dump-memusage gen-relkey check-appres \
         codegen btcodegen javaoptgen clean-javaoptgen emigen nandgen custgen drvgen ptgen \
         update-api modem-info bindergen clean-modem

S_MODULE_LOG  =  out/target/product/$(PROJECT)_$(CUR_MODULE).log
MODULE_LOG    =  $(LOGDIR)/$(PROJECT)_$(CUR_MODULE).log
S_LOG =  out/target/product/$(PROJECT)_
LOG   =  $(LOGDIR)/$(PROJECT)_
CUSTOM_MEMORY_HDR = mediatek/custom/$(PROJECT)/preloader/inc/custom_MemoryDevice.h
CUSTOM_NAND_HDR = mediatek/custom/$(PROJECT)/common/nand_device_list.h

ifeq ($(MTK_TABLET_DRAM),yes)
MEMORY_DEVICE_XLS = mediatek/build/tools/TabletEmiList/$(MTK_PLATFORM)/TabletMemoryDeviceList_$(MTK_PLATFORM).xls
else
MEMORY_DEVICE_XLS = mediatek/build/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_$(MTK_PLATFORM).xls
endif

USERID        =  $(shell whoami)
PRELOADER_WD  =  mediatek/preloader
LK_WD         =  bootable/bootloader/lk
KERNEL_WD     =  kernel
ANDROID_WD    =  .
ALL_MODULES   =
MAKE_DEBUG    =  --no-print-directory
hide         :=  @
CMD_ARGU2    :=  $(filter-out -j%, $(CMD_ARGU))
REMAKECMD    :=  make -fmediatek/build/makemtk.mk CMD_ARGU=$(CMD_ARGU) $(CMD_ARGU2) $(MAKE_DEBUG)
CPUCORES     :=  $(shell cat /proc/cpuinfo | grep processor | wc -l)
MAKEJOBS     :=  -j$(CPUCORES)
makemtk_temp := $(shell mkdir -p $(LOGDIR))

# Memory partition auto-gen related variable initilization
MEM_PARTITION_GENERATOR   := mediatek/build/tools/ptgen/$(MTK_PLATFORM)/ptgen.pl
MEM_PARTITION_TABLE       := mediatek/build/tools/ptgen/$(MTK_PLATFORM)/partition_table_$(MTK_PLATFORM).xls
PARTITION_HEADER_LOCATION := mediatek/custom/$(PROJECT)/common
BOARDCONFIG_LOCATION 	  := mediatek/config/$(PROJECT)/configs/partition_size.mk
OTA_SCATTER_GENERATOR     := mediatek/build/tools/ptgen/ota_scatter.pl

#ifeq ($(ACTION),update-api)
#   MAKEJOBS :=
#endif
MAKECMD      :=  make $(MAKEJOBS) $(CMD_ARGU) $(MAKE_DEBUG)

SHOWTIMECMD   =  date "+%Y/%m/%d %H:%M:%S"
SHOWRSLT      =  /usr/bin/perl $(MKTOPDIR)/mediatek/build/tools/showRslt.pl

PRELOADER_IMAGES := $(PRELOADER_WD)/bin/preloader_$(PROJECT).bin

LK_IMAGES     := $(LK_WD)/build-$(PROJECT)/lk.bin
LOGO_IMAGES   := $(LK_WD)/build-$(PROJECT)/logo.bin

ifeq ($(strip $(KBUILD_OUTPUT_SUPPORT)),yes)
  KERNEL_IMAGES    := $(KERNEL_WD)/out/kernel_$(PROJECT).bin
else
  KERNEL_IMAGES    := $(KERNEL_WD)/kernel_$(PROJECT).bin
endif
ANDROID_IMAGES   := $(LOGDIR)/$(PROJECT)/system.img \
                    $(LOGDIR)/$(PROJECT)/boot.img \
                    $(LOGDIR)/$(PROJECT)/recovery.img \
                    $(LOGDIR)/$(PROJECT)/secro.img \
                    $(LOGDIR)/$(PROJECT)/userdata.img
ifeq (true,$(BUILD_TINY_ANDROID))
  ANDROID_IMAGES := $(filter-out %recovery.img,$(ANDROID_IMAGES))
endif
ifneq ($(ACTION),)
  ANDROID_TARGET_IMAGES :=$(filter %/$(patsubst %image,%.img,$(ACTION)),$(ANDROID_IMAGES))
  ifeq (${ACTION},otapackage)
    ANDROID_TARGET_IMAGES :=$(ANDROID_IMAGES)
  endif
  ifeq (${ACTION},snod)
    ANDROID_TARGET_IMAGES :=$(filter %/system.img,$(ANDROID_IMAGES))
  endif
endif
ifeq (MT6573, $(MTK_PLATFORM))
  ifeq (android, $(CUR_MODULE))
    ANDROID_IMAGES += $(LOGDIR)/$(PROJECT)/DSP_BL
  endif
endif

SCATTER_FILE := mediatek/misc/$(MTK_PLATFORM)_Android_scatter.txt
ifeq ($(strip $(MTK_EMMC_SUPPORT)),yes)
  SCATTER_FILE := mediatek/misc/$(MTK_PLATFORM)_Android_scatter_emmc.txt
endif
ifeq ($(strip $(MTK_YAML_SCATTER_FILE_SUPPORT)),yes)
  SCATTER_FILE := mediatek/misc/$(MTK_PLATFORM)_Android_scatter.txt
endif

#wschen
OTA_SCATTER_FILE := mediatek/misc/ota_scatter.txt

export TARGET_PRODUCT=$(PROJECT)
export FLAVOR=$(FLAVOR)

ifneq ($(ACTION), )
  SHOWBUILD     =  $(ACTION)
else
  SHOWBUILD     =  build
endif
SHOWTIME      =  $(shell $(SHOWTIMECMD))
ifeq ($(ENABLE_TEE), TRUE)
  DEAL_STDOUT := 2>&1 | tee -a $(MODULE_LOG)
  DEAL_STDOUT_CUSTGEN := 2>&1 | tee -a $(LOG)custgen.log
  DEAL_STDOUT_EMIGEN := 2>&1 | tee -a $(LOG)emigen.log
  DEAL_STDOUT_NANDGEN := 2>&1 | tee -a $(LOG)nandgen.log
  DEAL_STDOUT_JAVAOPTGEN := 2>&1 | tee -a $(LOG)javaoptgen.log
  DEAL_STDOUT_IMEJAVAOPTGEN := 2>&1 | tee -a $(LOG)imejavaoptgen.log
  DEAL_STDOUT_SIGN_IMAGE := 2>&1 | tee -a $(LOG)sign-image.log
  DEAL_STDOUT_ENCRYPT_IMAGE := 2>&1 | tee -a $(LOG)encrypt-image.log
  DEAL_STDOUT_DRVGEN := 2>&1 | tee -a $(LOG)drvgen.log
  DEAL_STDOUT_SIGN_MODEM := 2>&1 | tee -a $(LOG)sign-modem.log
  DEAL_STDOUT_CHECK_MODEM := 2>&1 | tee -a $(LOG)check-modem.log
  DEAL_STDOUT_MODEM_INFO := 2>&1 | tee -a $(LOG)modem-info.log
  DEAL_STDOUT_UPDATE_MD := 2>&1 | tee -a $(LOG)update-modem.log
  DEAL_STDOUT_DUMP_MEMUSAGE := 2>&1 | tee -a $(LOG)dump-memusage.log
  DEAL_STDOUT_PTGEN := 2>&1 | tee -a $(LOG)ptgen.log
  DEAL_STDOUT_MM := 2>&1 | tee -a $(LOG)mm.log
  DEAL_STDOUT_CUSTREL := 2>&1 | tee -a $(LOG)rel-cust.log
  DEAL_STDOUT_CHK_APPRES := 2>&1 | tee -a $(LOG)check-appres.log
  DEAL_STDOUT_BINDERGEN := 2>&1 | tee -a $(LOG)bindergen.log
  DEAL_STDOUT_TRUSTZONE := 2>&1 | tee -a $(LOG)trustzone.log
else
  DEAL_STDOUT  := >> $(MODULE_LOG) 2>&1
  DEAL_STDOUT_CUSTGEN := > $(LOG)custgen.log 2>&1
  DEAL_STDOUT_EMIGEN := > $(LOG)emigen.log 2>&1
  DEAL_STDOUT_NANDGEN := > $(LOG)nandgen.log 2>&1
  DEAL_STDOUT_JAVAOPTGEN := > $(LOG)javaoptgen.log 2>&1
  DEAL_STDOUT_IMEJAVAOPTGEN := > $(LOG)imejavaoptgen.log 2>&1
  DEAL_STDOUT_SIGN_IMAGE := > $(LOG)sign-image.log 2>&1
  DEAL_STDOUT_ENCRYPT_IMAGE := > $(LOG)encrypt-image.log 2>&1
  DEAL_STDOUT_SIGN_MODEM := > $(LOG)sign-modem.log 2>&1
  DEAL_STDOUT_CHECK_MODEM := > $(LOG)check-modem.log 2>&1
  DEAL_STDOUT_MODEM_INFO := > $(LOG)modem-info.log 2>&1
  DEAL_STDOUT_DRVGEN := > $(LOG)drvgen.log 2>&1
  DEAL_STDOUT_UPDATE_MD := > $(LOG)update-modem.log 2>&1
  DEAL_STDOUT_DUMP_MEMUSAGE := > $(LOG)dump-memusage.log 2>&1
  DEAL_STDOUT_PTGEN := > $(LOG)ptgen.log 2>&1
  DEAL_STDOUT_MM := > $(LOG)mm.log 2>&1
  DEAL_STDOUT_CUSTREL := > $(LOG)rel-cust.log 2>&1
  DEAL_STDOUT_CHK_APPRES := >> $(LOG)check-appres.log 2>&1
  DEAL_STDOUT_BINDERGEN := > $(LOG)bindergen.log 2>&1
  DEAL_STDOUT_TRUSTZONE := > $(LOG)trustzone.log 2>&1
endif

MAKECMD    +=  TARGET_PRODUCT=$(PROJECT) GEMINI=$(GEMINI) EVB=$(EVB) FLAVOR=$(FLAVOR)

ifeq ($(BUILD_PRELOADER),yes)
  ALL_MODULES += preloader
endif

ifeq ($(BUILD_LK),yes)
  ALL_MODULES += lk
endif

ifeq ($(BUILD_KERNEL),yes)
  ALL_MODULES += kernel
  KERNEL_ARG = kernel_$(PROJECT).config
endif

ALL_MODULES += android

include mediatek/build/libs/pack_dep_gen.mk
-include mediatek/build/tools/preprocess/preprocess.mk
include mediatek/build/libs/codegen.mk

pregen: emigen nandgen ptgen codegen
codegen: custgen drvgen btcodegen cgen
remake update-api drvgen emigen nandgen: custgen
# TODO: newall: cleanall remakeall; remakeall:
newall: cleanall pregen remakeall
# TODO: some one in $(ALL_MODULES): codegen or emigen
new: clean codegen remake

check-dep: custgen
	$(eval include mediatek/build/addon/core/config.mak)
	$(if $(filter error,$(DEP_ERR_CNT)),\
                  $(error Dependency Check FAILED!!))
#	$(hide) echo " Dependency Check Successfully!!"
#	$(hide) echo "*******************************"

cleanall remakeall:
ifeq ($(filter -k, $(CMD_ARGU)),)
	$(hide) for i in $(ALL_MODULES); do \
	  $(REMAKECMD) CUR_MODULE=$$i $(subst all,,$@); \
	  if [ $${PIPESTATUS[0]} != 0 ]; then exit 1; fi; \
      done
else
	$(hide) let count=0; for i in $(ALL_MODULES); do \
	$(REMAKECMD) CUR_MODULE=$$i $(subst all,,$@); \
	last_return_code=$${PIPESTATUS[0]}; \
	if [ $$last_return_code != 0 ]; then let count=$$count+$$last_return_code; fi; \
	done; \
	exit $$count
endif


ANDROID_NATIVE_TARGETS := \
         update-api \
         cts sdk win_sdk otapackage banyan_addon banyan_addon_x86 dist updatepackage \
         snod bootimage systemimage recoveryimage secroimage target-files-package \
         factoryimage userdataimage userdataimage-nodeps customimage
ANDROID_NATIVE_TARGETS += dump-comp-build-info
ANDROID_NATIVE_TARGETS += dump-products
.PHONY: $(ANDROID_NATIVE_TARGETS)

systemimage: check-modem

$(ANDROID_NATIVE_TARGETS):
	$(hide) \
        $(if $(filter update-api,$@),\
          $(if $(filter true,$(strip $(BUILD_TINY_ANDROID))), \
            echo SKIP $@... \
            , \
            $(if $(filter snod userdataimage-nodeps,$@), \
              , \
              /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT) && \
             ) \
            $(REMAKECMD) ACTION=$@ CUR_MODULE=$@ android \
           ) \
          , \
          $(if $(filter snod userdataimage-nodeps,$@), \
            , \
            /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT) && \
           ) \
          $(if $(filter banyan_addon banyan_addon_x86,$@), \
            $(REMAKECMD) ACTION=sdk_addon CUR_MODULE=sdk_addon android \
            , \
            $(REMAKECMD) ACTION=$@ CUR_MODULE=$@ android \
           ) \
         )

update-api: $(ALLJAVAOPTFILES)
banyan_addon: $(ALLJAVAOPTFILES)
banyan_addon_x86: $(ALLJAVAOPTFILES)
win_sdk: $(ALLJAVAOPTFILES)

ifeq ($(TARGET_PRODUCT),emulator)
   TARGET_PRODUCT := generic
endif
.PHONY: mm
ifeq ($(HAVE_PREPROCESS_FLOW),true)
mm: run-preprocess
endif
mm:
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) (source build/envsetup.sh;cd $(MM_PATH);TARGET_PRODUCT=$(TARGET_PRODUCT) FLAVOR=$(FLAVOR) mm $(MAKEJOBS) $(SNOD) $(DEAL_STDOUT_MM);exit $${PIPESTATUS[0]})  && \
          $(SHOWRSLT) $$? $(LOG)$@.log || \
          $(SHOWRSLT) $$? $(LOG)$@.log


.PHONY: trustzone
TRUSTZONE_PROTECT_PRIVATE_SECURITY_PATH := $(wildcard mediatek/protect-private/security/Android.mk)
TRUSTZONE_PROTECT_PRIVATE_SEC_DRV_PATH := $(wildcard mediatek/protect-private/sec_drv/Android.mk)
TRUSTZONE_PROTECT_PRIVATE_SEC_DRM_PATH := $(wildcard mediatek/protect-private/sec_drm/Android.mk)
TRUSTZONE_PROTECT_PRIVATE_MTEE_PATH := $(wildcard mediatek/protect-private/mtee/Android.mk)
TRUSTZONE_PROTECT_PRIVATE_DRM_PATH := $(wildcard mediatek/protect-private/drm/Android.mk)
TRUSTZONE_PROTECT_PATH := $(wildcard mediatek/protect/trustzone/Android.mk)
TRUSTZONE_PROTECT_PLATFORM_PATH := $(wildcard mediatek/protect/platform/$(call lc,$(MTK_PLATFORM))/trustzone/Android.mk)
TRUSTZONE_PROTECT_BSP_PATH := $(wildcard mediatek/protect-bsp/trustzone/Android.mk)
TRUSTZONE_PROTECT_BSP_PLATFORM_PATH := $(wildcard mediatek/protect-bsp/platform/$(call lc,$(MTK_PLATFORM))/trustzone/Android.mk)
TRUSTZONE_PATH := $(wildcard mediatek/trustzone/Android.mk)
TRUSTZONE_PLATFORM_PATH := $(wildcard mediatek/platform/$(call lc,$(MTK_PLATFORM))/trustzone/Android.mk)
TRUSTZONE_ALL_PATH :="$(TRUSTZONE_PROTECT_PRIVATE_SECURITY_PATH) $(TRUSTZONE_PROTECT_PRIVATE_SEC_DRV_PATH) $(TRUSTZONE_PROTECT_PRIVATE_SEC_DRM_PATH) $(TRUSTZONE_PROTECT_PRIVATE_MTEE_PATH) $(TRUSTZONE_PROTECT_PRIVATE_DRM_PATH) $(TRUSTZONE_PROTECT_PATH) $(TRUSTZONE_PROTECT_PLATFORM_PATH) $(TRUSTZONE_PROTECT_BSP_PATH) $(TRUSTZONE_PROTECT_BSP_PLATFORM_PATH) $(TRUSTZONE_PLATFORM_PATH) $(TRUSTZONE_PATH)"

trustzone:
ifeq ($(ACTION),)
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) (source build/envsetup.sh;TARGET_PRODUCT=$(TARGET_PRODUCT) FLAVOR=$(FLAVOR) ONE_SHOT_MAKEFILE=$(TRUSTZONE_ALL_PATH) m $(MAKEJOBS) $(SNOD) $(DEAL_STDOUT_TRUSTZONE);exit $${PIPESTATUS[0]})  && \
          $(SHOWRSLT) $$? $(LOG)$@.log || \
          $(SHOWRSLT) $$? $(LOG)$@.log
endif

.PHONY: rel-cust
ifeq ($(DUMP),true)
rel-cust: dump_option := -d
endif
rel-cust:
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) python mediatek/build/tools/customRelease.py $(dump_option) ./ $(RELEASE_DEST) $(TARGET_PRODUCT) $(MTK_RELEASE_PACKAGE).xml $(DEAL_STDOUT_CUSTREL) && \
         $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
         $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log

clean:
ifneq ($(strip $(MTK_DEPENDENCY_AUTO_CHECK)), true)
	$(hide) $(REMAKECMD) ACTION=$@ $(CUR_MODULE)
endif

mrproper:
ifneq ($(strip $(MTK_DEPENDENCY_AUTO_CHECK)), true)
	$(hide) $(REMAKECMD) ACTION=$@ $(CUR_MODULE)
endif

remake:
	$(hide) /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT)
	$(hide) $(REMAKECMD) ACTION= $(CUR_MODULE)

#### Remove old modem files under mediatek/custom/out/$project/modem ####
clean-modem:
	$(hide) rm -rf $(strip $(MTK_ROOT_CUSTOM_OUT))/modem
	$(hide) rm -rf $(strip $(LOGDIR)/$(PROJECT))/system/etc/extmddb
	$(hide) rm -rf $(strip $(LOGDIR)/$(PROJECT))/system/etc/mddb

update-modem: clean-modem custgen check-modem sign-modem
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) ./makeMtk $(FULL_PROJECT) mm build/target/board/ snod $(DEAL_STDOUT_UPDATE_MD) && \
         $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
         $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log


drvgen:
# mediatek/custom/$project/kernel/dct/dct/*.h *.c
PRIVATE_DRVGEN_DEPENDENCY := $(shell find -L mediatek/dct/ -type f | sed 's/ /\\ /g')
ifneq ($(MTK_DRVGEN_SUPPORT),no)
ifneq ($(PROJECT),generic)
$(eval $(call mtk-check-dependency,drvgen,$(MTK_DEPENDENCY_OUTPUT)))
$(eval $(call mtk-check-argument,drvgen,$(MTK_DEPENDENCY_OUTPUT),$(PRIVATE_DRVGEN_DEPENDENCY)))
endif
endif
$(MTK_DEPENDENCY_OUTPUT)/drvgen.dep: PRIVATE_CUSTOM_KERNEL_DCT:= $(if $(CUSTOM_KERNEL_DCT),$(CUSTOM_KERNEL_DCT),dct)
$(MTK_DEPENDENCY_OUTPUT)/drvgen.dep:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $(basename $(notdir $@))ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$(basename $(notdir $@)).log
	$(hide) rm -f $(LOG)$(basename $(notdir $@)).log $(LOG)$(basename $(notdir $@)).log_err
	$(hide) mediatek/dct/DrvGen mediatek/custom/$(PROJECT)/kernel/dct/$(PRIVATE_CUSTOM_KERNEL_DCT)/codegen.dws $(DEAL_STDOUT_DRVGEN) && \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	@echo '[Dependency] $(PRIVATE_DRVGEN_DEPENDENCY) mediatek/custom/$(PROJECT)/kernel/dct/$(PRIVATE_CUSTOM_KERNEL_DCT)/codegen.dws' >> $(LOG)$(basename $(notdir $@)).log
	$(call mtk-print-dependency)
	$(call mtk-print-argument,$(PRIVATE_DRVGEN_DEPENDENCY))


custgen:
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) make -f mediatek/build/custgen.mk $(DEAL_STDOUT_CUSTGEN) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log


JAVAOPTFILEPATH := mediatek/frameworks/common/src/com/mediatek/common/featureoption
JAVAOPTFILE := $(JAVAOPTFILEPATH)/FeatureOption.java

$(JAVAOPTFILE): mediatek/build/tools/javaoptgen.pl $(PRJ_MF) $(OPTR_MF) mediatek/build/tools/javaoption.pm
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) gen $@ ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)javaoptgen.log
	$(hide) rm -f $(LOG)javaoptgen.log $(LOG)javaoptgen.log_err
	$(hide) perl mediatek/build/tools/javaoptgen.pl $(PRJ_MF) $(OPTR_MF) $(DEAL_STDOUT_JAVAOPTGEN)

JAVAIMEOPTFILE := $(JAVAOPTFILEPATH)/IMEFeatureOption.java
$(JAVAIMEOPTFILE): mediatek/build/tools/gen_java_ime_definition.pl $(PRJ_MF)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) gen $@ ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)imejavaoptgen.log
	$(hide) rm -f $(LOG)imejavaoptgen.log $(LOG)imejavaoptgen.log_err
	$(hide) perl mediatek/build/tools/gen_java_ime_definition.pl $(PRJ_MF) $(DEAL_STDOUT_IMEJAVAOPTGEN)

ALLJAVAOPTFILES := $(JAVAIMEOPTFILE) $(JAVAOPTFILE)
clean-javaoptgen:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
ifneq ($(strip $(MTK_DEPENDENCY_AUTO_CHECK)), true)
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo clean $(ALLJAVAOPTFILES)
	$(hide) rm -rf $(ALLJAVAOPTFILES)
endif

javaoptgen: $(ALLJAVAOPTFILES)
ifneq ($(filter javaoptgen,$(MAKECMDGOALS)),)
	$(hide) echo Done java optgen
endif


sign-image:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) perl mediatek/build/tools/SignTool/SignTool.pl $(PROJECT) $(FULL_PROJECT) $(MTK_SEC_SECRO_AC_SUPPORT) $(MTK_NAND_PAGE_SIZE) $(DEAL_STDOUT_SIGN_IMAGE) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log

encrypt-image:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) perl mediatek/build/tools/encrypt_image.pl $(PROJECT) $(DEAL_STDOUT_ENCRYPT_IMAGE) && \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log


sign-modem: custgen
ifeq ($(filter generic banyan_addon banyan_addon_x86,$(PROJECT)),)
ifneq ($(MTK_SIGNMODEM_SUPPORT),no)
$(eval $(call mtk-check-dependency,sign-modem,$(MTK_DEPENDENCY_OUTPUT)))
endif
endif
$(MTK_DEPENDENCY_OUTPUT)/sign-modem.dep:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $(basename $(notdir $@))ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$(basename $(notdir $@)).log
	$(hide) rm -f $(LOG)$(basename $(notdir $@)).log $(LOG)$(basename $(notdir $@)).log_err
	$(hide) perl mediatek/build/tools/sign_modem.pl \
                     $(FULL_PROJECT) \
                     $(MTK_SEC_MODEM_ENCODE) \
                     $(MTK_SEC_MODEM_AUTH) \
                     $(PROJECT) \
                     $(MTK_SEC_SECRO_AC_SUPPORT) \
                     $(DEAL_STDOUT_SIGN_MODEM) && \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(call mtk-print-dependency)
	$(hide) perl $(MTK_DEPENDENCY_SCRIPT) --overwrite $@ $@  mediatek/build/tools/SignTool/dep/ "\.dep" mediatek/build/tools/SignTool/
# workaround: check-modem will fail after sign-modem
	$(hide) touch -c $(MTK_DEPENDENCY_OUTPUT)/check-modem.dep

MODEM_INFO_FLAG := $(foreach f, $(CUSTOM_MODEM), $(wildcard $(MTK_ROOT_CUSTOM)/common/modem/$(f)/modem*.info))

modem-info: clean-modem custgen
modem-info: PRIVATE_MODEM_PATH := $(strip $(MTK_ROOT_CUSTOM_OUT))/modem
modem-info: PRIVATE_CHK_MD_TOOL := mediatek/build/tools/checkMD.pl
modem-info:
	$(hide) echo MODEM_INFO_FLAG = $(MODEM_INFO_FLAG)
ifneq ($(strip $(MODEM_INFO_FLAG)),)
   ifeq ($(strip $(MTK_ENABLE_MD1)),yes)
      ifeq ($(strip $(MTK_MD1_SUPPORT)),1)
	$(hide) echo ==== Modem info. of MD1 2G===
	$(hide) cat $(PRIVATE_MODEM_PATH)/modem_1_2g_n.info
	$(hide) echo ""
      endif
      ifeq ($(strip $(MTK_MD1_SUPPORT)),3)
	$(hide) echo ==== Modem info. of MD1 WG===
	$(hide) cat $(PRIVATE_MODEM_PATH)/modem_1_wg_n.info
	$(hide) echo ""
      endif
      ifeq ($(strip $(MTK_MD1_SUPPORT)),4)
	$(hide) echo ==== Modem info. of MD1 TG===
	$(hide) cat $(PRIVATE_MODEM_PATH)/modem_1_tg_n.info
	$(hide) echo ""
      endif
   endif
   ifeq ($(strip $(MTK_ENABLE_MD2)),yes)
	$(hide) echo ==== Modem info. of MD2 ===
	$(hide) cat $(PRIVATE_MODEM_PATH)/modem_sys2.info
	$(hide) echo ""
   endif
else
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) perl $(PRIVATE_CHK_MD_TOOL) \
                     PROJECT=$(PROJECT) \
                     PRIVATE_MODEM_PATH=$(PRIVATE_MODEM_PATH) \
                     MTK_PLATFORM=$(MTK_PLATFORM) \
                     MTK_MD1_SUPPORT=$(MTK_MD1_SUPPORT) \
                     MTK_MD2_SUPPORT=$(MTK_MD2_SUPPORT) \
                     MTK_GET_BIN_INFO=$@ \
                     $(DEAL_STDOUT_MODEM_INFO) && \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log
endif #$(MODEMINFO_Exist_Flag)

check-modem:
ifeq ($(filter generic banyan_addon banyan_addon_x86,$(PROJECT)),)
  ifneq ($(MTK_PLATFORM),MT8135)
-include $(MTK_DEPENDENCY_OUTPUT)/./check-modem.dep
check-modem: $(MTK_DEPENDENCY_OUTPUT)/check-modem.dep
  endif
endif
$(MTK_DEPENDENCY_OUTPUT)/check-modem.dep: PRIVATE_CHK_MD_TOOL := mediatek/build/tools/checkMD.pl
$(MTK_DEPENDENCY_OUTPUT)/check-modem.dep: PRIVATE_MODEM_PATH := $(strip $(MTK_ROOT_CUSTOM_OUT))/modem
$(MTK_DEPENDENCY_OUTPUT)/check-modem.dep:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $(basename $(notdir $@))ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$(basename $(notdir $@)).log
	$(hide) rm -f $(LOG)$(basename $(notdir $@)).log $(LOG)$(basename $(notdir $@)).log_err
	$(hide) perl $(PRIVATE_CHK_MD_TOOL) \
                     PROJECT=$(PROJECT) \
                     PRIVATE_MODEM_PATH=$(PRIVATE_MODEM_PATH) \
                     MTK_PLATFORM=$(MTK_PLATFORM) \
                     MTK_MD1_SUPPORT=$(MTK_MD1_SUPPORT) \
                     MTK_MD2_SUPPORT=$(MTK_MD2_SUPPORT) \
                     $(DEAL_STDOUT_CHECK_MODEM) && \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
          $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(call mtk-print-dependency)


emigen:
ifneq (,$(filter MT8135 MT6575 MT6577 MT6589 MT8135 MT6582 MT6572,$(MTK_PLATFORM)))
ifneq ($(PROJECT), generic)
$(eval $(call mtk-check-dependency,emigen,$(MTK_DEPENDENCY_OUTPUT)))
endif
endif
$(MTK_DEPENDENCY_OUTPUT)/emigen.dep:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $(basename $(notdir $@))ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$(basename $(notdir $@)).log
	$(hide) rm -f $(LOG)$(basename $(notdir $@)).log $(LOG)$(basename $(notdir $@)).log_err
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/emigen.pl $(CUSTOM_MEMORY_HDR) \
                     $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(PROJECT) $(DEAL_STDOUT_EMIGEN) && \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(call mtk-print-dependency)


nandgen:
ifneq ($(PROJECT), generic)
$(eval $(call mtk-check-dependency,nandgen,$(MTK_DEPENDENCY_OUTPUT)))
endif
$(MTK_DEPENDENCY_OUTPUT)/nandgen.dep:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $(basename $(notdir $@))ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$(basename $(notdir $@)).log
	$(hide) rm -f $(LOG)$(basename $(notdir $@)).log $(LOG)$(basename $(notdir $@)).log_err
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/nandgen.pl \
                     $(CUSTOM_NAND_HDR) \
                     $(MEMORY_DEVICE_XLS) \
                     $(MTK_PLATFORM) \
                     $(PROJECT) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT_NANDGEN) && \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(call mtk-print-dependency)


dump-memusage: MEM_USAGE_LABEL := $(if $(LABEL),$(LABEL),$(shell date +%Y-%m-%d_%H:%M:%S))
dump-memusage: MEM_USAGE_GENERATOR := mediatek/build/tools/memmon/rommon.pl
dump-memusage: PRIVATE_PROJECT := $(if $(filter emulator, $(PROJECT)),generic,$(PROJECT))
dump-memusage: MEM_USAGE_DATA_LOCATION := mediatek/build/tools/memmon/data
dump-memusage: IMAGE_LOCATION := $(OUT_DIR)/target/product/$(PRIVATE_PROJECT)
dump-memusage:
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) perl $(MEM_USAGE_GENERATOR) \
                     $(MEM_USAGE_LABEL) \
                     $(PRIVATE_PROJECT) \
                     $(FLAVOR) \
                     $(MEM_USAGE_DATA_LOCATION) \
                     $(IMAGE_LOCATION) \
                     $(DEAL_STDOUT_DUMP_MEMUSAGE) && \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
                $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log


ptgen:
ifneq ($(PROJECT), generic)
ifneq ($(MTK_PTGEN_SUPPORT),no)
$(eval $(call mtk-check-dependency,ptgen,$(MTK_DEPENDENCY_OUTPUT)))
ptgen: $(MTK_DEPENDENCY_OUTPUT)/ptgen.dep $(OTA_SCATTER_FILE)
endif
endif
$(MTK_DEPENDENCY_OUTPUT)/ptgen.dep:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $(basename $(notdir $@))ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$(basename $(notdir $@)).log
	$(hide) rm -f $(LOG)$(basename $(notdir $@)).log $(LOG)$(basename $(notdir $@)).log_err
	$(hide) perl $(MEM_PARTITION_GENERATOR) \
                     MTK_PLATFORM=$(MTK_PLATFORM) \
                     PROJECT=$(PROJECT) \
                     FULL_PROJECT=$(FULL_PROJECT) \
                     MTK_LCA_SUPPORT=$(MTK_LCA_SUPPORT) \
                     MTK_NAND_PAGE_SIZE=$(MTK_NAND_PAGE_SIZE) \
                     MTK_EMMC_SUPPORT=$(MTK_EMMC_SUPPORT) \
                     EMMC_CHIP=$(EMMC_CHIP) \
                     MTK_LDVT_SUPPORT=$(MTK_LDVT_SUPPORT) \
                     TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) \
                     MTK_EMMC_OTP_SUPPORT=$(MTK_EMMC_SUPPORT_OTP) \
                     $(DEAL_STDOUT_PTGEN) && \
                     $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
                     $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(hide) mkdir -p $(LOGDIR)/$(PROJECT)
  ifeq ($(strip $(MTK_EMMC_SUPPORT)),yes)
	$(hide) cp -f mediatek/misc/EBR* $(LOGDIR)/$(PROJECT)
	$(hide) cp -f mediatek/misc/MBR $(LOGDIR)/$(PROJECT)
     ifeq ($(strip $(MTK_YAML_SCATTER_FILE_SUPPORT)),yes)
	$(hide) cp -f mediatek/misc/$(MTK_PLATFORM)_Android_scatter.txt $(LOGDIR)/$(PROJECT)
     else
	$(hide) cp -f mediatek/misc/$(MTK_PLATFORM)_Android_scatter_emmc.txt $(LOGDIR)/$(PROJECT)
     endif
  else
	$(hide) cp -f mediatek/misc/$(MTK_PLATFORM)_Android_scatter.txt $(LOGDIR)/$(PROJECT)
  endif
	$(call mtk-print-dependency)


ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(OTA_SCATTER_FILE)
endif
$(OTA_SCATTER_FILE): $(SCATTER_FILE) $(OTA_SCATTER_GENERATOR)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) perl $(OTA_SCATTER_GENERATOR) $< $@


gen-relkey: PRIVATE_KEY_GENERATOR := development/tools/make_key
gen-relkey: PRIVATE_KEY_LOCATION := build/target/product/security/$(TARGET_PRODUCT)
gen-relkey: PRIVATE_KEY_LIST := releasekey media shared platform
gen-relkey: PRIVATE_SIGNATURE_SUBJECT := $(strip $(SIGNATURE_SUBJECT))
gen-relkey:
	$(hide) echo "Generating release key/certificate..."
	$(hide) if [ ! -d $(PRIVATE_KEY_LOCATION) ]; then \
                  mkdir $(PRIVATE_KEY_LOCATION); \
                fi
	$(hide) for key in $(PRIVATE_KEY_LIST); do \
                  $(PRIVATE_KEY_GENERATOR) $(strip $(PRIVATE_KEY_LOCATION))/$$key '$(PRIVATE_SIGNATURE_SUBJECT)' < /dev/null; \
                done

# check unused application resource
check-appres: PRIVATE_SCANNING_FOLDERS := packages/apps
check-appres: PRIVATE_CHECK_TOOL := mediatek/build/tools/FindDummyRes.py
check-appres:
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -rf $(LOG)$@.log*
	$(hide) for d in $(PRIVATE_SCANNING_FOLDERS); do \
                  $(PRIVATE_CHECK_TOOL) -d $$d \
                  $(DEAL_STDOUT_CHK_APPRES); \
                done
	$(hide) $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log

ifeq ($(BUILD_PRELOADER),yes)
  ifeq ($(ACTION), )
    ifneq (,$(filter MT6516 MT6575 MT6577 MT6573 MT6589 MT8135 MT6572 MT6582,$(MTK_PLATFORM)))
preloader: emigen
    endif
preloader: nandgen ptgen
  endif
endif
preloader:
ifeq ($(BUILD_PRELOADER),yes)
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
	$(hide) rm -f $(MODULE_LOG) $(MODULE_LOG)_err
  ifneq ($(ACTION), )
	$(hide) cd $(PRELOADER_WD) && \
	  (make clean $(DEAL_STDOUT) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION)) && cd $(MKTOPDIR)
  else
	$(hide) cd $(PRELOADER_WD) && \
	  (./build.sh $(PROJECT) $(ACTION) $(DEAL_STDOUT) && \
	  cd $(MKTOPDIR) && \
          $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(PRELOADER_IMAGES),$(DEAL_STDOUT),&&) \
          $(if $(strip $(ACTION)),:,$(call copytoout,$(PRELOADER_IMAGES),$(LOGDIR)/$(PROJECT))) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) || \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG))
  endif
else
	$(hide) echo Not support $@.
endif

ifeq ($(BUILD_LK),yes)
  ifneq ($(ACTION),clean)
    ifneq (,$(filter MT6516 MT6575 MT6577 MT6573,$(MTK_PLATFORM)))
lk: emigen
    endif
lk: nandgen ptgen
lk: 
  endif
endif
lk:
ifeq ($(BUILD_LK),yes)
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
	$(hide) rm -f $(MODULE_LOG) $(MODULE_LOG)_err
  ifneq ($(ACTION), )
	$(hide) cd $(LK_WD) && \
	  (PROJECT=$(PROJECT) make clean $(DEAL_STDOUT) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION)) && cd $(MKTOPDIR)
  else
	$(hide) cd $(LK_WD) && \
	  (FULL_PROJECT=$(FULL_PROJECT) make $(MAKEJOBS) $(PROJECT) $(ACTION) $(DEAL_STDOUT) && \
	  cd $(MKTOPDIR) && \
          $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(LK_IMAGES),$(DEAL_STDOUT) &&) \
          $(if $(strip $(ACTION)),:,$(call copytoout,$(LK_IMAGES),$(LOGDIR)/$(PROJECT))) && \
          $(if $(strip $(ACTION)),:,$(call copytoout,$(LOGO_IMAGES),$(LOGDIR)/$(PROJECT))) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) || \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG))
  endif
else
	$(hide) echo Not support $@.
endif


kernel: nandgen
ifeq ($(BUILD_KERNEL),yes)
  ifneq ($(ACTION),clean)
#  ifneq ($(PROJECT), generic)
#  ifneq ($(MTK_PTGEN_SUPPORT),no)
# TODO: review in phase two
kernel: $(MTK_DEPENDENCY_OUTPUT)/ptgen.dep
# DFO
kernel: 
kernel: 
#  endif
#  endif
  endif
endif
kernel:
ifeq ($(BUILD_KERNEL),yes)
  ifneq ($(KMOD_PATH),)
	$(hide)	echo building kernel module KMOD_PATH=$(KMOD_PATH)
	$(hide) cd $(KERNEL_WD) && \
	(KMOD_PATH=$(KMOD_PATH) ./build.sh $(ACTION) $(KERNEL_ARG) ) && cd $(MKTOPDIR)
  else
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
	$(hide) rm -f $(MODULE_LOG) $(MODULE_LOG)_err
	$(hide) cd $(KERNEL_WD) && \
	  (MAKEJOBS=$(MAKEJOBS) ./build.sh $(ACTION) $(PROJECT) $(DEAL_STDOUT) && \
	   cd $(MKTOPDIR) && \
	   $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(if $(strip $(ACTION)),,$(KERNEL_IMAGES)),$(DEAL_STDOUT),&&) \
           $(if $(strip $(ACTION)),:,$(call copytoout,$(KERNEL_IMAGES),$(LOGDIR)/$(PROJECT))) && \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION))
  endif
else
	$(hide) echo Not support $@.
endif


ifneq ($(ACTION),clean)
  ifneq ($(MTK_SIGNMODEM_SUPPORT),no)
android: check-modem sign-modem
  else
android: check-modem
  endif
android: 
android: 
android: 
android: 
android: 
android: 
else
# workaround for clean
#android: $(MTK_ROOT_CONFIG_OUT)/BoardConfig.mk
android: clean-javaoptgen
endif
ifeq ($(HAVE_PREPROCESS_FLOW),true)
  ifeq ($(ACTION),clean)
android: clean-preprocessed
  else
android: run-preprocess
  endif
endif
android: CHECK_IMAGE := $(ANDROID_TARGET_IMAGES)
android:
ifeq ($(ACTION), )
	$(hide) /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT)
endif
ifneq ($(DR_MODULE),)
   ifneq ($(ACTION), clean)
	$(hide) echo building android module MODULE=$(DR_MODULE)
	$(MAKECMD) $(DR_MODULE)
   else
	$(hide) echo cleaning android module MODULE=$(DR_MODULE)
	$(hide) $(MAKECMD) clean-$(DR_MODULE)
   endif
else
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
	$(hide) rm -f $(MODULE_LOG) $(MODULE_LOG)_err
	$(hide) ($(MAKECMD) $(ACTION) $(DEAL_STDOUT);exit $${PIPESTATUS[0]}) && \
	  $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(if $(strip $(ACTION)),$(CHECK_IMAGE),$(ANDROID_IMAGES)),$(DEAL_STDOUT),&&) \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $${PIPESTATUS[0]} $(MODULE_LOG) $(ACTION)
endif


define chkImgSize
$(if $(filter no,$(MTK_CHKIMGSIZE_SUPPORT)), \
     echo "Check Img size process disabled due to MTK_CHKIMGSIZE_SUPPORT is set to no" $(5) $(6),\
     $(call chkImgSize1,$(1),$(2),$(3),$(4),$(5),$(6)) \
)
endef
##############################################################
# function:  chkImgSize1
# arguments: $(ACTION) $(PROJECT) $(SCATTER_FILE) $(IMAGES) $(DEAL_STDOUT) &&
#############################################################
define chkImgSize1
$(if $(strip $(1)), \
     $(if $(strip $(4)), \
          $(if $(filter generic, $(2)),, \
               perl mediatek/build/tools/chkImgSize.pl $(3) $(2) $(4) $(5) $(6) \
           ) \
      ), \
     $(if $(filter generic, $(2)),, \
         perl mediatek/build/tools/chkImgSize.pl $(3) $(2) $(4) $(5) $(6) \
      ) \
 )
endef

#############################################################
# function: copytoout
# arguments: $(SOURCE) $(TARGET)
#############################################################
define copytoout
mkdir -p $(2);cp -f $(1) $(2)
endef

bindergen:
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -f $(LOG)$@.log $(LOG)$@.log_err
	$(hide) mediatek/build/tools/bindergen/bindergen.pl $(DEAL_STDOUT_BINDERGEN) && \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log || \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$@.log


