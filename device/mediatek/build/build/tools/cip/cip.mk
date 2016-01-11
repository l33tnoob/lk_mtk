# To generate Customer Image Partition image.
TARGET_CUSTOM_OUT := $(PRODUCT_OUT)/custom
include device/*/$(PROJECT)/partition_size.mk

.PHONY: customimage
customimage:
	$(build-customimage-target)                  
                     
define build-customimage-target
mkdir -p $(TARGET_CUSTOM_OUT)

mkuserimg.sh -s $(PRODUCT_OUT)/custom $(PRODUCT_OUT)/custom.img ext4 custom $(strip $(BOARD_CUSTOMIMAGE_PARTITION_SIZE)) $(PRODUCT_OUT)/root/file_contexts

endef