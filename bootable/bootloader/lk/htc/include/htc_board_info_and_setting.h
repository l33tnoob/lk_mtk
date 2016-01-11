
#ifndef __HTC_BOARD_INFO_AND_SETTING_H
#define __HTC_BOARD_INFO_AND_SETTING_H

enum {
	BOOTMODE_NORMAL,
	BOOTMODE_REBOOT,
	BOOTMODE_FASTBOOT,
	BOOTMODE_RECOVERY,
	BOOTMODE_FTM1,
	BOOTMODE_FTM2,
	BOOTMODE_FTM3,
	BOOTMODE_DOWNLOAD,
	BOOTMODE_DOWNLOAD_RUU,
	BOOTMODE_RAMDUMP,
	BOOTMODE_DDRTEST
};

enum {
	XA = 0,
	XB,
	XC,
	XD,
	XE,
	XF,
	XG,
	XH,
	PVT = 0x80,
	PVT_A = PVT,
	PVT_B,
	PVT_C,
	PVT_D,
	PVT_E,
	PVT_F,
	PVT_G,
	PVT_H,
	EVM = 0x99,
};

int htc_setting_init(void);
int htc_setting_init_bootmode(void);
int htc_setting_get_bootmode(void);
int htc_setting_cfgdata_get(int id);
int htc_setting_cfgdata_set(int id, int data);
int htc_setting_cfgdata_erase();
int htc_setting_kernelflag_get();
int htc_setting_debugflag_get();
int htc_setting_bootloaderflag_get();
int htc_setting_radioflag_get();
int htc_setting_radioflag_ex1_get();
int htc_setting_radioflag_ex2_get();
int htc_smart_mdlog() ;
const char* htc_setting_get_wipe_done_flag(void);
void htc_setting_set_wipe_done_flag(void);
void htc_setting_reset_wipe_done_flag(void);
unsigned char* htc_setting_get_barcode();
unsigned char htc_setting_get_bomid(void);
unsigned char htc_setting_pcbid(void);
unsigned char htc_setting_pcbid2(void);
void htc_setting_pcbid_write(unsigned char pcbid);
unsigned htc_setting_engineerid(void);
int htc_setting_skuid_read(int id);
int htc_setting_skuid_write(int id, int data, int true_write_flag);
const char* htc_setting_cid(void);
int htc_setting_check_backup_cid_empty(void);
int htc_setting_check_device_is_super_cid(void);
int htc_setting_security(void);
int htc_cmdline_update(void** cmdline);
void setting_invalidate_checksum();
char *htc_setting_get_firmware_main_version();
int get_off_alarm_data(int *data, int secs);
#if TAMPER_DETECT
void setting_get_tamper_info(char *data);
void setting_set_tamper_info(const char* data);
long setting_get_tamper_info_offset(void);
#endif
unsigned int setting_get_force_sec_boot_reason (void);
void setting_set_force_sec_boot_reason (unsigned int reason);

int get_off_alarm_data(int *data, int secs);
#endif
