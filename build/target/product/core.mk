#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
#
# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Base configuration for communication-oriented android devices
# (phones, tablets, etc.).  If you want a change to apply to ALMOST ALL
# devices (including non-phones and non-tablets), modify
# core_minimal.mk instead. If you care about wearables, you need to modify
# core_tiny.mk in addition to core_minimal.mk.

PRODUCT_PACKAGES += \
    BasicDreams \
    Browser \
    Calculator \
    Calendar \
    CalendarProvider \
    CaptivePortalLogin \
    CertInstaller \
    Contacts \
    DeskClock \
    DocumentsUI \
    DownloadProviderUi \
    Email \
    Exchange2 \
    ExternalStorageProvider \
    FusedLocation \
    InputDevices \
    KeyChain \
    Keyguard \
    LatinIME \
    Launcher2 \
    ManagedProvisioning \
    PicoTts \
    PacProcessor \
    libpac \
    PrintSpooler \
    ProxyHandler \
    QuickSearchBox \
    Settings \
    SharedStorageBackup \
    Telecom \
    TeleService \
    VpnDialogs \
    MmsService \
    libhtcflag-jni

# DRM related modules
PRODUCT_PACKAGES += \
    libhtcomaplugin

# MirrorLink related module
PRODUCT_PACKAGES += \
    libdrmmirrorlinkplugin

# DriveActivator DRM plugin modules
PRODUCT_PACKAGES += \
    libdrmdriveactivatorplugin

$(call inherit-product, $(SRC_TARGET_DIR)/product/core_base.mk)

#HtcLegacy
PRODUCT_PACKAGES += \
           HtcLegacy \

#Google Play feature filter
PRODUCT_PACKAGES += \
    com.htc.software.market.xml

# include app_core.mk to add PRODUCT_PACKAGES
-include packages/apps/CustomizationsData/config/app_core.mk
-include external/HtcBuildFlag/build_tools/common_core.mk
# GMS build in
$(call inherit-product-if-exists, packages/apps/GMS/products/gms.mk)
$(call inherit-product-if-exists, packages/apps/GMS_Base/products/gms.mk)

#``USBNET related modules
PRODUCT_PACKAGES += \
    usbnet \
    libusbnetjni \
    netsharing \
    libnetsharing \
    udhcpd

#//++[Optimization][paul.wy_wang][2014/10/22][AAPT] Setup PRODUCT_AAPT_CONFIG by density.
#//: Setup PRODUCT_AAPT_CONFIG by density.
density = $(strip $(shell cat device/htc/$(TARGET_PRODUCT)/system.prop | grep ro.sf.lcd_density | cut -d "=" -f 2))
# [Emulator]
ifneq ($(TARGET_PRODUCT), full)
# Not match all of below density, trigger build break
ifeq ($(findstring $(density), 160 240 320 480 640), )
$(error density not found in device/htc/$(TARGET_PRODUCT)/system.prop)
endif
endif #[Emulator]

ifeq ($(density) , 160)
PRODUCT_AAPT_CONFIG += mdpi
#/// [Framework] begin, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
#Dalvik inherit heap related settings +
$(call inherit-product, frameworks/native/build/phone-hdpi-1024-dalvik-heap.mk)
#Dalvik inherit heap related settings -
#/// [Framework] end, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
endif

ifeq ($(density) , 240)
PRODUCT_AAPT_CONFIG += hdpi mdpi
#/// [Framework] begin, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
#Dalvik inherit heap related settings +
$(call inherit-product, frameworks/native/build/phone-hdpi-1024-dalvik-heap.mk)
#Dalvik inherit heap related settings -
#/// [Framework] end, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
endif

ifeq ($(density) , 320)
PRODUCT_AAPT_CONFIG += xhdpi hdpi mdpi
#/// [Framework] begin, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
#Dalvik inherit heap related settings +
$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)
#Dalvik inherit heap related settings -
#/// [Framework] end, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
endif

ifeq ($(density) , 480)
PRODUCT_AAPT_CONFIG += xxhdpi xhdpi hdpi
#/// [Framework] begin, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
#Dalvik inherit heap related settings +
$(call inherit-product, frameworks/native/build/phone-xhdpi-2048-dalvik-heap.mk)
#Dalvik inherit heap related settings -
#/// [Framework] end, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
endif

ifeq ($(density) , 640)
PRODUCT_AAPT_CONFIG += xxxhdpi xxhdpi xhdpi hdpi
#/// [Framework] begin, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
#Dalvik inherit heap related settings +
$(call inherit-product, frameworks/native/build/phone-xxhdpi-2048-dalvik-heap.mk)
#Dalvik inherit heap related settings -
#/// [Framework] end, bibby_lin, 2015/02/09, [ART] auto inherit heap setting
endif
#//++[Optimization][paul.wy_wang][2014/10/22][AAPT] Setup PRODUCT_AAPT_CONFIG by density.
