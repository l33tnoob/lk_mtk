# Inherit for devices that support 64-bit primary and 32-bit secondary zygote startup script
#$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)

# Inherit from those products. Most specific first.
#$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# Set target and base project for flavor build
MTK_TARGET_PROJECT := $(subst full_,,$(TARGET_PRODUCT))
MTK_BASE_PROJECT := $(MTK_TARGET_PROJECT)
MTK_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_BASE_PROJECT))
MTK_TARGET_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_TARGET_PROJECT))

# This is where we'd set a backup provider if we had one
#$(call inherit-product, device/sample/products/backup_overlay.mk)
# Inherit from maguro device
$(call inherit-product, device/htc/$(MTK_TARGET_PROJECT)/device.mk)
#DOLBY START
LOWER_SKU_NAME := $(shell echo $(PRIVATE_SKU_NAME) | tr A-Z a-z)
ifneq ($(wildcard mfg/.patched), )
MFG_ROM := true
endif
ifneq (gep, $(findstring gep, $(LOWER_SKU_NAME)))
ifneq (porting, $(findstring porting, $(LOWER_SKU_NAME)))
ifneq (true, $(MFG_ROM))
ifneq (downloadmode, $(findstring downloadmode, $(LOWER_SKU_NAME)))
#$(call inherit-product, vendor/dolby/ds/dolby-product.mk)
endif
endif
endif
endif
#DOLBY END

# set locales & aapt config.
PRODUCT_LOCALES := zh_CN en_US zh_TW

# Set those variables here to overwrite the inherited values.
PRODUCT_MANUFACTURER := HTC
PRODUCT_NAME := htc_a51bml_dwgl
PRODUCT_DEVICE := htc_a51bml_dwgl
PRODUCT_MODEL := htc_a51bml_dwl
PRODUCT_POLICY := android.policy_phone
PRODUCT_BRAND := HTC

#DOLBY START
ifneq (gep, $(findstring gep, $(LOWER_SKU_NAME)))
ifneq (porting, $(findstring porting, $(LOWER_SKU_NAME)))
ifneq (true, $(MFG_ROM))
ifneq (downloadmode, $(findstring downloadmode, $(LOWER_SKU_NAME)))
DOLBY_DAX_VERSION            := 1
DOLBY_DAP                    := true
DOLBY_DAP2                   := false
DOLBY_DAP_SW                 := true
DOLBY_DAP_HW                 := false
DOLBY_DAP_PREGAIN            := true
DOLBY_DAP_HW_QDSP_HAL_API    := false
DOLBY_DAP_MOVE_EFFECT        := true
DOLBY_DAP_BYPASS_SOUND_TYPES := true
DOLBY_CONSUMER_APP           := false
DOLBY_UDC                    := true
DOLBY_UDC_VIRTUALIZE_AUDIO   := false
DOLBY_MONO_SPEAKER           := false

#include vendor/dolby/ds/dolby-buildspec.mk
PRODUCT_COPY_FILES += \
    device/htc/$(PRODUCT_DEVICE)/dolby/ds1-default.xml:system/vendor/etc/dolby/ds1-default.xml
endif
endif
endif
endif
#DOLBY END

# ACC+
# Need to include ACC Make file before common.mk that properties in ACC Make file can overwrite other original properties.
ACC_MAKEFILE := device/htc/customize/acc/ACCConfig.mk
ACC_MAKEFILE_EXIST := $(shell if test -e $(ACC_MAKEFILE); then echo yes; else echo no; fi)
#use hard-code path to let ACC reconize
ifeq ($(ACC_MAKEFILE_EXIST), yes)
include device/htc/customize/acc/ACCConfig.mk
else
    $(warning No $(ACC_MAKEFILE) found.)
endif
# ACC-

ifeq ($(KERNEL_DEFCONFIG),)
    ifeq ($(HTC_HEP_FLAG), True)
        ifneq ($(HTC_DEBUG_FLAG), DEBUG)
            KERNEL_DEFCONFIG := a51bml_dwgl-perf_defconfig
            ifeq ($(TARGET_BUILD_VARIANT), user)
                $(shell python device/htc/common/pnp/gen_Perf_conf.py a51bml_dwgl $(LOCAL_PATH)/white-list.cfg)
            else
                $(shell python device/htc/common/pnp/gen_Perf_conf.py a51bml_dwgl)
            endif
        endif
    endif
endif
#HTC_WIFI_START
#for wifi enable notification
ifeq (CHS, $(findstring CHS, $(PRIVATE_RCMS_SKU)))
$(warning CHS SKU found.)
PRODUCT_COPY_FILES += \
       frameworks/base/data/wifienablenotifylist/wifienablenotifylist_chs.xml:system/etc/wifienablenotifylist.xml
endif
#HTC_WIF_END


KERNEL_DEFCONFIG ?= a51bml_dwgl_defconfig

# MFG ROM P_SENSOR CALIBRATION.
ifneq ($(wildcard mfg/.patched), )
  $(shell echo -e "\nCONFIG_HUB_SENTRAL_MFG=y" >>  kernel-3.10/arch/arm64/configs/$(KERNEL_DEFCONFIG))
endif

ifeq ($(PRIVATE_RCMS_SKU), HTCCN CHS CT)
    $(shell cp kernel-3.10/arch/arm64/configs/$(KERNEL_DEFCONFIG)  kernel-3.10/arch/arm64/configs/a51bml_dwgl-ct_defconfig)
    $(shell echo -e "\nCONFIG_MTK_MD_SBP_CUSTOM_VALUE=\"9\"" >>  kernel-3.10/arch/arm64/configs/a51bml_dwgl-ct_defconfig)
    KERNEL_DEFCONFIG := a51bml_dwgl-ct_defconfig
else
    $(shell cp kernel-3.10/arch/arm64/configs/$(KERNEL_DEFCONFIG)  kernel-3.10/arch/arm64/configs/a51bml_dwgl-om_defconfig)
    $(shell echo -e "\nCONFIG_MTK_TC7_FEATURE=y" >>  kernel-3.10/arch/arm64/configs/a51bml_dwgl-om_defconfig)
    $(shell echo -e "\nCONFIG_MTK_MD_SBP_CUSTOM_VALUE=\"\"" >>  kernel-3.10/arch/arm64/configs/a51bml_dwgl-om_defconfig)
    KERNEL_DEFCONFIG := a51bml_dwgl-om_defconfig
endif

PRELOADER_TARGET_PRODUCT ?= htc_a51bml
LK_PROJECT ?= htc_a51bml

# HTC KEY - copy file for key ++ #
PRODUCT_COPY_FILES += \
	device/htc/common/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl
# HTC KEY - copy file for key -- #

PRODUCT_COPY_FILES += \
    device/htc/common/rootdir/init.htc.storage.rc:root/init.htc.storage.rc \
    device/htc/common/rootdir/init.mocana.sh:system/etc/init.mocana.sh

PRODUCT_COPY_FILES += \
	device/htc/$(PRODUCT_DEVICE)/init.hosd.mt6735.rc:root/init.hosd.mt6735.rc \
	device/mediatek/build/build/tools/CheckSum_Generate/CheckSum_Gen:CheckSum_Gen \
	device/mediatek/build/build/tools/CheckSum_Generate/libflashtool.so:libflashtool.so

# Camera res_ctrl file
  PRODUCT_COPY_FILES += \
          vendor/mediatek/proprietary/custom/mt6735/hal/D1/pic_sizes/res_ctrl_common.conf:system/etc/res_ctrl.conf

# HTC touch
include device/htc/common/touch/project/a51bml/touch.mk

# HTC PnPMgr service
TARGET_HTC_PNPMGR := true

# HTC BT 20141124 ++
PRODUCT_PACKAGES += \
    FMRadioService
# HTC BT 20141124 --

# HTC Wake Lock Detector configuration
PRODUCT_COPY_FILES += \
	device/htc/$(PRODUCT_DEVICE)/wld_config.xml:system/etc/wld_config.xml
PRODUCT_COPY_FILES += \
    device/htc/$(PRODUCT_DEVICE)/wld_config.xml:data/data/htc/cupd/libwld.so

# HTC: don't enable full ramdump when user build, this function eat too much eMMC
ifeq ($(TARGET_BUILD_VARIANT), user)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += persist.mtk.aee.allocate=0
else
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += persist.mtk.aee.allocate=fullmem
endif

# HTC Thermal configuration
PRODUCT_COPY_FILES += \
	device/htc/$(PRODUCT_DEVICE)/thermal.conf:system/etc/.tp/thermal.conf
# copy file for hall sensor
PRODUCT_COPY_FILES += \
    device/htc/$(PRODUCT_DEVICE)/AK8789_HALL_SENSOR.kl:system/usr/keylayout/AK8789_HALL_SENSOR.kl \
    device/htc/$(PRODUCT_DEVICE)/com.htc.sensor.hallsensor.xml:system/etc/permissions/com.htc.sensor.hallsensor.xml

PRODUCT_COPY_FILES += \
	device/htc/$(PRODUCT_DEVICE)/thermal.off.conf:system/etc/.tp/thermal.off.conf

# Motion Sensors ++
PRODUCT_COPY_FILES += \
       frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
       frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
       frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
       frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
       frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:system/etc/permissions/android.hardware.sensor.stepdetector.xml \
       frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:system/etc/permissions/android.hardware.sensor.stepcounter.xml \
       frameworks/native/data/etc/com.htc.sensor.sensorhub.xml:system/etc/permissions/com.htc.sensor.sensorhub.xml \
       device/htc/$(PRODUCT_DEVICE)/sentral.fw:system/etc/firmware/sentral.fw

# Build shub_dbg daemon
TARGET_USES_SENSOR_HUB_PNI_DBG_DAEMON := true
ifeq ($(TARGET_USES_SENSOR_HUB_PNI_DBG_DAEMON), true)
PRODUCT_PACKAGES += \
       shub_dbg
endif #TARGET_USES_SENSOR_HUB_PNI_DBG_DAEMON

# Motion Sensor --


#NFC++
# NFCEE access control
$(call inherit-product-if-exists,packages/apps/Nfc/NFCEE/nfcee.mk)
# NFC packages
PRODUCT_PACKAGES += \
   libnfc-nci-nxp \
   libnfc_nci_nxp_jni \
   nfc_nci_pn547.default \
   NfcNciNxp \
   Tag \
   com.android.nfc_extras


# file that declares the MIFARE NFC constant
# Commands to migrate prefs from com.android.nfc3 to com.android.nfc
# NFC access control + feature files + configuration
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
	frameworks/native/data/etc/android.hardware.nfc.hce.xml:system/etc/permissions/android.hardware.nfc.hce.xml \
	frameworks/native/data/etc/com.nxp.mifare.xml:system/etc/permissions/com.nxp.mifare.xml \
	frameworks/native/data/etc/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml \
    device/htc/common/nfc/firmware/so/libpn547_fw_08_01_24_64.so:system/vendor/firmware/libpn547_fw.so


ifneq ($(HTC_PROTECT_CONFIDENTIAL_FLAG),true)
        PRODUCT_COPY_FILES += \
                device/htc/common/nfc/configuration/nxp/pn547/R320/debug/libnfc-nxp.a31bml.conf:system/etc/libnfc-nxp.conf \
                device/htc/common/nfc/configuration/nxp/pn547/R320/debug/libnfc-brcm.conf:system/etc/libnfc-brcm.conf
else
        PRODUCT_COPY_FILES += \
                device/htc/common/nfc/configuration/nxp/pn547/R320/release/libnfc-nxp.a31bml.conf:system/etc/libnfc-nxp.conf \
                device/htc/common/nfc/configuration/nxp/pn547/R320/release/libnfc-brcm.conf:system/etc/libnfc-brcm.conf
endif #($(HTC_PROTECT_CONFIDENTIAL_FLAG),true)

#NFC--

####################
#  exFAT binary tool (Enable if exfat is enabled in defconfg)
####################
PRODUCT_COPY_FILES += \
    device/htc/common/exfat/exfat_mt6753/bin/exfatck:/system/bin/exfatck \
    device/htc/common/exfat/exfat_mt6753/bin/exfatdebug:/system/bin/exfatdebug \
    device/htc/common/exfat/exfat_mt6753/bin/exfatinfo:/system/bin/exfatinfo \
    device/htc/common/exfat/exfat_mt6753/bin/exfatlabel:/system/bin/exfatlabel \
    device/htc/common/exfat/exfat_mt6753/bin/mkexfat:/system/bin/mkexfat


#HTC FOTA Firmware udpate ++
PRODUCT_FOTA_FIRMWARE_LIST += \
    hosd_verified.img \
    lk_verified.bin \
    logo_verified.bin \
    preloader.bin \
    secro_verified.img \
    tee1.img \
    tee2.img \
    boot_verified.img  \
    recovery_verified.img \
    gpt_main_16g.img \
    gpt_main_32g.img \
    gpt_main_64g.img \
    tp_HMX852XE_MAIN.img \
    tp_HMX852XE_SECOND.img
#HTC FOTA Firmware udpate --


# HTC Automotive - BEGIN
ifeq ($(HTC_HEP_FLAG), True)
PRODUCT_PACKAGES += cand
ifneq ($(TARGET_BUILD_PREBUILT),true)
PRODUCT_COPY_FILES += vendor/htc/can/hsml.key:system/etc/hsml.key
endif
endif
# HTC Automotive - END

# Security Tamper Detection
ifeq ($(HTC_HEP_FLAG), True)
PRODUCT_PACKAGES += tpd_daemon
endif
PRODUCT_PACKAGES += \
clockd

