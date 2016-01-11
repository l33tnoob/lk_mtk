LOCAL_DIR := $(GET_LOCAL_DIR)

$(info "HTC: Add Module: "$(LOCAL_DIR))

INCLUDES += -I$(LOCAL_DIR)/include -I$(LOCAL_DIR)/include/htc_crypto

INCLUDES += -I$(LOCAL_DIR)/../app/mt_boot

OBJS += \
	$(LOCAL_DIR)/htc_board_info_and_setting.o \
	$(LOCAL_DIR)/htc_key_and_menu.o \
	$(LOCAL_DIR)/htc_menu_eng.o \
	$(LOCAL_DIR)/htc_oem_commands.o \
	$(LOCAL_DIR)/htc_fastboot_policy.o \
	$(LOCAL_DIR)/htc_android_info.o \
	$(LOCAL_DIR)/htc_reboot_info.o \
	$(LOCAL_DIR)/htc_dtb_utility.o \
	$(LOCAL_DIR)/htc_pg_fs.o \
	$(LOCAL_DIR)/htc_security_util.o \
	$(LOCAL_DIR)/htc_crc.o \
    $(LOCAL_DIR)/htc_dump_commands.o \
	$(LOCAL_DIR)/htc_bootloader_log.o \
	$(LOCAL_DIR)/htc_crypto/aes_rijndael.o \
	$(LOCAL_DIR)/htc_crypto/crypto.o \
	$(LOCAL_DIR)/htc_crypto/rsa.o \
	$(LOCAL_DIR)/htc_crypto/sha256.o \
	$(LOCAL_DIR)/htc_crypto/sha.o \
	$(LOCAL_DIR)/htc_tamper_detect.o

