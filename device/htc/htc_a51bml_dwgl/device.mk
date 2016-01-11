
include device/htc/$(TARGET_PRODUCT)/ProjectConfig.mk

######################################################

# PRODUCT_COPY_FILES overwrite
# Please add flavor project's PRODUCT_COPY_FILES here.
# It will overwrite base project's PRODUCT_COPY_FILES.
# PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/init.project.rc:root/init.project.rc
# PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/meta_init.project.rc:root/meta_init.project.rc
# PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/factory_init.project.rc:root/factory_init.project.rc

# property overwrite
PRODUCT_PROPERTY_OVERRIDES += ro.telephony.default_network=4,0
# overlay has priorities. high <-> low.
DEVICE_PACKAGE_OVERLAYS += device/htc/$(MTK_TARGET_PROJECT)/overlay

#######################################################

# PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/egl.cfg:system/lib/egl/egl.cfg
# PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/ueventd.mt6735.rc:root/ueventd.mt6735.rc

PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/factory_init.project.rc:root/factory_init.project.rc
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/init.project.rc:root/init.project.rc
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/meta_init.project.rc:root/meta_init.project.rc

# USB
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/init.mt6735.usb.rc:root/init.mt6735.usb.rc

ifeq ($(MTK_SMARTBOOK_SUPPORT),yes)
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/sbk-kpd.kl:system/usr/keylayout/sbk-kpd.kl \
                      device/htc/$(TARGET_PRODUCT)/sbk-kpd.kcm:system/usr/keychars/sbk-kpd.kcm
endif

# Add FlashTool needed files
#PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/EBR1:EBR1
#ifneq ($(wildcard device/htc/$(MTK_TARGET_PROJECT)/EBR2),)
#  PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/EBR2:EBR2
#endif
#PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/MBR:MBR
#PRODUCT_COPY_FILES += device/htc/$(MTK_TARGET_PROJECT)/MT6735_Android_scatter.txt:MT6735_Android_scatter.txt

#HTC Debug
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/sfc:root/sbin/sfc

# alps/vendor/mediatek/proprietary/binary/3rd-party/free/SRS_AudioEffect/srs_processing/Android.mk
ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
  PRODUCT_COPY_FILES += vendor/mediatek/proprietary/binary/3rd-party/free/SRS_AudioEffect/srs_processing/license/srsmodels.lic:system/data/srsmodels.lic
endif

# HTC_AUD_START
ifeq ($(strip $(ENABLE_DTS_EFFECT)),yes)
  PRODUCT_COPY_FILES +=  frameworks/av/services/dts_processing/license/dts.lic:system/etc/soundimage/dts.lic
  #Audio srs
  PRODUCT_COPY_FILES += \
	device/htc/$(TARGET_PRODUCT)/audio/srsfx_trumedia_int.cfg:system/etc/soundimage/srsfx_trumedia_int.cfg \
	device/htc/$(TARGET_PRODUCT)/audio/srsfx_trumedia_ext.cfg:system/etc/soundimage/srsfx_trumedia_ext.cfg \
	device/htc/$(TARGET_PRODUCT)/audio/srsfx_trumedia_ext_HS250.cfg:system/etc/soundimage/srsfx_trumedia_ext_HS250.cfg \
	device/htc/$(TARGET_PRODUCT)/audio/srsfx_trumedia_ext_MAX300.cfg:system/etc/soundimage/srsfx_trumedia_ext_MAX300.cfg
#	device/htc/$(TARGET_PRODUCT)/audio/srsfx_trumedia_ext_MIDTIER.cfg:system/etc/soundimage/srsfx_trumedia_ext_MIDTIER.cfg
endif

PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/audio/audio_device.xml:system/etc/audio_device.xml
# HTC_AUD_END

# alps/vendor/mediatek/proprietary/external/GeoCoding/Android.mk
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/external/GeoCoding/geocoding.db:system/etc/geocoding.db

# alps/vendor/mediatek/proprietary/frameworks-ext/native/etc/Android.mk
# sensor related xml files for CTS
ifneq ($(strip $(CUSTOM_KERNEL_ACCELEROMETER)),)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml
endif

ifneq ($(strip $(CUSTOM_KERNEL_MAGNETOMETER)),)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml
endif

ifneq ($(strip $(CUSTOM_KERNEL_ALSPS)),)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml
else
  ifneq ($(strip $(CUSTOM_KERNEL_PS)),)
    PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml
  endif
  ifneq ($(strip $(CUSTOM_KERNEL_ALS)),)
    PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml
  endif
endif

ifneq ($(strip $(CUSTOM_KERNEL_GYROSCOPE)),)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml
endif

ifneq ($(strip $(CUSTOM_KERNEL_BAROMETER)),)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.sensor.barometer.xml:system/etc/permissions/android.hardware.sensor.barometer.xml
endif

# touch related file for CTS
ifeq ($(strip $(CUSTOM_KERNEL_TOUCHPANEL)),generic)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml
else
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.faketouch.xml:system/etc/permissions/android.hardware.faketouch.xml
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml
endif

# USB OTG
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml

# GPS relative file
ifeq ($(MTK_GPS_SUPPORT),yes)
  PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml
endif

# alps/external/libnfc-opennfc/open_nfc/hardware/libhardware/modules/nfcc/nfc_hal_microread/Android.mk
# PRODUCT_COPY_FILES += external/libnfc-opennfc/open_nfc/hardware/libhardware/modules/nfcc/nfc_hal_microread/driver/open_nfc_driver.ko:system/lib/open_nfc_driver.ko

# alps/frameworks/av/media/libeffects/factory/Android.mk
PRODUCT_COPY_FILES += frameworks/av/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf

# alps/mediatek/config/$project
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml

# alps/mediatek/external/sip/
ifeq ($(MTK_SIP_SUPPORT),yes)
  PRODUCT_COPY_FILES += vendor/mediatek/proprietary/external/sip/enable_sip/android.software.sip.xml:system/etc/permissions/android.software.sip.xml
  PRODUCT_COPY_FILES += vendor/mediatek/proprietary/external/sip/enable_sip/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml
else
  PRODUCT_COPY_FILES += vendor/mediatek/proprietary/external/sip/disable_sip/android.software.sip.xml:system/etc/permissions/android.software.sip.xml
  PRODUCT_COPY_FILES += vendor/mediatek/proprietary/external/sip/disable_sip/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml
endif

# HTC BT 20150407 ++
# Set default coredump property to yes
ifneq ($(HTC_PROTECT_CONFIDENTIAL_FLAG), true)
PRODUCT_PROPERTY_OVERRIDES += persist.bt.logstandalone=1
else
PRODUCT_PROPERTY_OVERRIDES += persist.bt.logstandalone=0
endif #($(HTC_PROTECT_CONFIDENTIAL_FLAG), true)
# HTC BT 20150407 --

#HTC Message Framework
# Set default message property to india
ifeq ($(PRIVATE_RCMS_SKU), hTC Asia India)
PRODUCT_PROPERTY_OVERRIDES += persist.c2k.om.area=in
endif #($(PRIVATE_RCMS_SKU), hTC Asia India)

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += persist.sys.usb.config=mtp
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += persist.service.acm.enable=0
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += ro.mount.fs=EXT4

# meta tool
PRODUCT_PROPERTY_OVERRIDES += ro.mediatek.chip_ver=S01
PRODUCT_PROPERTY_OVERRIDES += ro.mediatek.version.release=$(strip $(MTK_BUILD_VERNO))
PRODUCT_PROPERTY_OVERRIDES += ro.mediatek.platform=MT6735

# set Telephony property - SIM count
SIM_COUNT := 2
PRODUCT_PROPERTY_OVERRIDES += ro.telephony.sim.count=$(SIM_COUNT)
PRODUCT_PROPERTY_OVERRIDES += persist.radio.default.sim=0

ifeq ($(GEMINI),yes)
  ifeq ($(MTK_DT_SUPPORT),yes)
    PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=dsda
  else
    ifeq ($(MTK_SVLTE_SUPPORT),yes)
      PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=dsda
    else
      PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=dsds
    endif
  endif
else
  PRODUCT_PROPERTY_OVERRIDES += persist.radio.multisim.config=ss
endif

# Audio Related Resource
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/custom/$(TARGET_PRODUCT)/factory/res/sound/testpattern1.wav:system/res/sound/testpattern1.wav
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/custom/$(TARGET_PRODUCT)/factory/res/sound/ringtone.wav:system/res/sound/ringtone.wav

# Keyboard layout
PRODUCT_COPY_FILES += device/mediatek/mt6735/ACCDET.kl:system/usr/keylayout/ACCDET.kl
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/mtk-kpd.kl:system/usr/keylayout/mtk-kpd.kl

# Microphone
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/android.hardware.microphone.xml:system/etc/permissions/android.hardware.microphone.xml

# Camera
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml

# Audio Policy
PRODUCT_COPY_FILES += device/htc/$(TARGET_PRODUCT)/audio_policy.conf:system/etc/audio_policy.conf

#Images for LCD test in factory mode
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/custom/$(TARGET_PRODUCT)/factory/res/images/lcd_test_00.png:system/res/images/lcd_test_00.png
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/custom/$(TARGET_PRODUCT)/factory/res/images/lcd_test_01.png:system/res/images/lcd_test_01.png
PRODUCT_COPY_FILES += vendor/mediatek/proprietary/custom/$(TARGET_PRODUCT)/factory/res/images/lcd_test_02.png:system/res/images/lcd_test_02.png

#Audio TFA9895
# SmartPA parameters
ifeq ($(strip $(NXP_SMARTPA_SUPPORT)),htc)
PRODUCT_COPY_FILES += \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/tfa9895.speaker:system/etc/tfa/tfa9895.speaker \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/tfa9895.patch:system/etc/tfa/tfa9895.patch \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/tfa9895.config:system/etc/tfa/tfa9895.config \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playback.preset:system/etc/tfa/playback.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playback.eq:system/etc/tfa/playback.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playback.drc:system/etc/tfa/playback.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/fm.preset:system/etc/tfa/fm.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/fm.eq:system/etc/tfa/fm.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/fm.drc:system/etc/tfa/fm.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voice.preset:system/etc/tfa/voice.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voice.eq:system/etc/tfa/voice.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voiceWB.eq:system/etc/tfa/voiceWB.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voice.drc:system/etc/tfa/voice.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcv.preset:system/etc/tfa/rcv.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcv.eq:system/etc/tfa/rcv.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcvWB.eq:system/etc/tfa/rcvWB.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcv.drc:system/etc/tfa/rcv.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voip.preset:system/etc/tfa/voip.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voip.eq:system/etc/tfa/voip.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voip.drc:system/etc/tfa/voip.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/Rec_Video.preset:system/etc/tfa/Rec_Video.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/Rec_Video.eq:system/etc/tfa/Rec_Video.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/Rec_Video.drc:system/etc/tfa/Rec_Video.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/tfa9895_l.speaker:system/etc/tfa/tfa9895_l.speaker \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playback_l.preset:system/etc/tfa/playback_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playback_l.eq:system/etc/tfa/playback_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playback_l.drc:system/etc/tfa/playback_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/fm_l.preset:system/etc/tfa/fm_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/fm_l.eq:system/etc/tfa/fm_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/fm_l.drc:system/etc/tfa/fm_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voice_l.preset:system/etc/tfa/voice_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voice_l.eq:system/etc/tfa/voice_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voiceWB_l.eq:system/etc/tfa/voiceWB_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voice_l.drc:system/etc/tfa/voice_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcv_l.preset:system/etc/tfa/rcv_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcv_l.eq:system/etc/tfa/rcv_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcvWB_l.eq:system/etc/tfa/rcvWB_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/rcv_l.drc:system/etc/tfa/rcv_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voip_l.preset:system/etc/tfa/voip_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voip_l.eq:system/etc/tfa/voip_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/voip_l.drc:system/etc/tfa/voip_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/Rec_Video_l.preset:system/etc/tfa/Rec_Video_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/Rec_Video_l.eq:system/etc/tfa/Rec_Video_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/Rec_Video_l.drc:system/etc/tfa/Rec_Video_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/tfa9895MFG.patch:system/etc/tfa/tfa9895MFG.patch \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG.preset:system/etc/tfa/playbackMFG.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG.eq:system/etc/tfa/playbackMFG.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG.config:system/etc/tfa/playbackMFG.config \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG.drc:system/etc/tfa/playbackMFG.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG_l.preset:system/etc/tfa/playbackMFG_l.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG_l.eq:system/etc/tfa/playbackMFG_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG_l.config:system/etc/tfa/playbackMFG_l.config \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackMFG_l.drc:system/etc/tfa/playbackMFG_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackwoofer.eq:system/etc/tfa/playbackwoofer.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackwoofer.drc:system/etc/tfa/playbackwoofer.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackwoofer.preset:system/etc/tfa/playbackwoofer.preset \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackwoofer_l.eq:system/etc/tfa/playbackwoofer_l.eq \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackwoofer_l.drc:system/etc/tfa/playbackwoofer_l.drc \
 device/htc/$(TARGET_PRODUCT)/audio/tfa/playbackwoofer_l.preset:system/etc/tfa/playbackwoofer_l.preset
PRODUCT_PACKAGES += \
    climax
endif

DEVICE_PACKAGE_OVERLAYS += device/htc/$(TARGET_PRODUCT)/overlay

ifeq ($(strip $(OPTR_SPEC_SEG_DEF)),NONE)
    PRODUCT_PACKAGES += DangerDash
endif

# inherit 6752 platform
$(call inherit-product, device/mediatek/mt6735/device.mk)

$(call inherit-product-if-exists, vendor/mediatek/libs/$(TARGET_PRODUCT)/device-vendor.mk)
