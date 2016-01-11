# Inherit for devices that support 64-bit primary and 32-bit secondary zygote startup script
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# Set target and base project for flavor build
MTK_TARGET_PROJECT := $(subst full_,,$(TARGET_PRODUCT))
MTK_BASE_PROJECT := $(MTK_TARGET_PROJECT)
MTK_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_BASE_PROJECT))
MTK_TARGET_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_TARGET_PROJECT))

# This is where we'd set a backup provider if we had one
#$(call inherit-product, device/sample/products/backup_overlay.mk)
# Inherit from maguro device
$(call inherit-product, device/htc/$(MTK_TARGET_PROJECT)/device.mk)

# set locales & aapt config.
PRODUCT_LOCALES := zh_CN en_US zh_TW

# Set those variables here to overwrite the inherited values.
PRODUCT_MANUFACTURER := alps
PRODUCT_NAME := full_irmn53_a50_64_op01
PRODUCT_DEVICE := irmn53_a50_64_op01
PRODUCT_MODEL := irmn53_a50_64_op01
PRODUCT_POLICY := android.policy_phone
PRODUCT_BRAND := alps

ifeq ($(TARGET_BUILD_VARIANT), eng)
KERNEL_DEFCONFIG ?= irmn53_a50_64_op01_debug_defconfig
else
KERNEL_DEFCONFIG ?= irmn53_a50_64_op01_defconfig
endif
PRELOADER_TARGET_PRODUCT ?= irmn53_a50_64_op01
LK_PROJECT ?= irmn53_a50_64_op01
