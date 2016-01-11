#ifndef __HTC_ANDROID_INFO_H__
#define __HTC_ANDROID_INFO_H__

#define MAX_MID_DEF					16
#define MIDLeng       				32
#define MAIN_MID_LEN        		4
#define MAX_MID_IGNORE  			5
#define CHECK_MID_LEN 				9
#define MAIN_SD_FILENAME_HEADER_LEN	4

#define MAX_CID_DEF					32
#define CIDLeng           			32
#define MAIN_CID_LEN        		5
#define MAX_CID_IGNORE      		3
#define CHECK_CID_LEN 				8

#define MAIN_VERSION_LEN			16
#define PREFER_DIAG_NAME_LEN		16

struct htc_android_info_struct{
	int del_userdata_flag;
	int del_cache_flag;
	int hboot_preupdate_flag;
	int del_devlog_flag;
	int del_fat_flag;
	int erase_config_flag;
	int enable_radio8_flag;
	int build_type;
	int aa_report;
	char image_cid[MAX_CID_DEF][CIDLeng];
	char image_mid[MAX_MID_DEF][MIDLeng];
	char image_mainver[MAIN_VERSION_LEN];
	char prefer_diag[PREFER_DIAG_NAME_LEN];
};

int parsing_android_info(char *android_info_buf, struct htc_android_info_struct *htc_android_info);
#endif
