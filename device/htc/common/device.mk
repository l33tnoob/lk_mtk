# Hosd
PRODUCT_COPY_FILES += device/htc/common/hosd.fstab:system/etc/hosd.fstab
PRODUCT_COPY_FILES += device/htc/common/hosd-rootdir/init.hosd.common.rc:root/init.hosd.common.rc

#++ 2015.04.07 USB Team, PCN00026 ++
PRODUCT_COPY_FILES += \
    device/htc/common/rootdir/init.htc.usb.rc:root/init.htc.usb.rc
#-- 2015.04.07 USB Team, PCN00026 --
#++ 2015/04/23, USB Team, PCN00006 ++
PRODUCT_COPY_FILES += \
        device/htc/common/projector_input.idc:system/usr/idc/projector_input.idc \
        device/htc/common/hsml_touchscreen.idc:system/usr/idc/hsml_touchscreen.idc \
        device/htc/common/projector-Keypad.kl:system/usr/keylayout/projector-Keypad.kl
#-- 2015/04/23, USB Team, PCN00006 --

#HTC core dump ++
ifneq ($(HTC_PROTECT_CONFIDENTIAL_FLAG),true)
PRODUCT_COPY_FILES += \
    device/htc/common/dbg_corefile.sh:system/bin/dbg_corefile.sh
endif
#HTC core dump --

#remote kill
PRODUCT_PACKAGES += libhtc_flash

#[FM] Enable HTC FMRadioService
PRODUCT_PACKAGES += FMRadioService
# HTC: use dlmalloc instead of jemalloc for mallocs
MALLOC_IMPL := dlmalloc

# HTC Google Drive
PRODUCT_PACKAGES += libgdvoucher

# HTC Mirrorlink
PRODUCT_PACKAGES += libdapcert \
                    libdaptpm
# HTC libscreen
PRODUCT_PACKAGES += screen.default

#camera lib/tools
PRODUCT_PACKAGES += lsc_camera \
                    awb_camera \
                    flash_camera
