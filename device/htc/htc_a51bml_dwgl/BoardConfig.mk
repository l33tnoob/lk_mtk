# Use the non-open-source part, if present
-include vendor/htc/$(TARGET_PRODUCT)/BoardConfigVendor.mk

TARGET_BOARD_PLATFORM := mt6753

# Use the 6735 common part
include device/mediatek/mt6735/BoardConfig.mk

TARGET_SYSTEM_PROP := \
	device/htc/$(TARGET_PRODUCT)/system.prop \
	device/htc/common/system.prop

# Note: Define `HTC_PREBUILT_SUBSYS_PATH' if default prebuilt subsys images
#       path `device/htc/prebuilts/$(TARGET_BOARD_PLATFORM)' is not used.
# HTC_PREBUILT_SUBSYS_PATH :=
HTC_MAIN_PROJECT_PREBUILT_SUBSYS_PATH := device/htc/$(TARGET_PRODUCT)/prebuilt_images

#Config partition size
-include $(MTK_PTGEN_OUT)/partition_size.mk
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 4096

include device/htc/$(TARGET_PRODUCT)/ProjectConfig.mk

MTK_INTERNAL_CDEFS := $(foreach t,$(AUTO_ADD_GLOBAL_DEFINE_BY_NAME),$(if $(filter-out no NO none NONE false FALSE,$($(t))),-D$(t)))
MTK_INTERNAL_CDEFS += $(foreach t,$(AUTO_ADD_GLOBAL_DEFINE_BY_VALUE),$(if $(filter-out no NO none NONE false FALSE,$($(t))),$(foreach v,$(shell echo $($(t)) | tr '[a-z]' '[A-Z]'),-D$(v))))
MTK_INTERNAL_CDEFS += $(foreach t,$(AUTO_ADD_GLOBAL_DEFINE_BY_NAME_VALUE),$(if $(filter-out no NO none NONE false FALSE,$($(t))),-D$(t)=\"$($(t))\"))

COMMON_GLOBAL_CFLAGS += $(MTK_INTERNAL_CDEFS)
COMMON_GLOBAL_CPPFLAGS += $(MTK_INTERNAL_CDEFS)

TARGET_HOSD_FSTAB := device/htc/common/hosd.fstab

ifneq ($(MTK_K64_SUPPORT), yes)
ifeq ($(PRIVATE_RCMS_SKU), HTCCN CHS CT)
BOARD_KERNEL_CMDLINE = bootopt=64S3,32N2,32N2
else
BOARD_KERNEL_CMDLINE = bootopt=64S3,32S1,32S1
endif
else
BOARD_KERNEL_CMDLINE = bootopt=64S3,32N2,64N2
endif

# HTC BT ++
BOARD_HAVE_CUSTOM_BLUETOOTH_BLUEDROID := true
# HTC BT --

# HTC WIFI ++
MTK_WIFI_GET_IMSI_FROM_PROPERTY := yes
# HTC WIFI --

# NFC++
NFC_MW := NCI_NXP
NFC_NCI_NXP_LIB_VERSION_CONTROL := libnfc_nci_nxp_320
NFC_UICC_PLUG_WORKAROUND := true
NFC_RECOVERY_WORKAROUND := true
# NFC--

# HTC Display ++
ifeq ($(HTC_HEP_FLAG), True)
HTC_AUTOBOT_SUPPORT := true
endif
# HTC Display --

#HTC USB ++
BOARD_KERNEL_CMDLINE += androidusb.pid=0x05D9
#HTC USB --

MOCANA_FIPS_MODULE := false

# Motion Sensors ++
BOARD_VENDOR_USE_SENSOR_HAL := sensor_hub_pni_mtk
# Motion Sensors --

#CAMERA
HTC_MFG_CAL := true
CAMERA_ASYNC_SPLIT_MODE := true
CAMERA_LOWLIGHT_NONZSL :=true
# enable check camera feature for 6753 platform
TARGET_ENABLE_CHECK_CAMERA := true
CAMERA_OIS_MOVIE_PANORAMA := true
#CAMERA

# Add by U61_Bill_Yang 
# 16G : Hybrid + Full pre-opt
WITH_ART_HYBRID_MODE := true
ifeq ($(TARGET_BUILD_VARIANT),user)
WITH_DEXPREOPT := true
FORCE_ALL_APP_DEXPREOPT := true
WITH_DEXPREOPT_PIC := true
endif
