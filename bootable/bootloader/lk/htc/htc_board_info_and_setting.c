#include <htc_board_info_and_setting.h>
#include <htc_reboot_info.h>
#include <htc_android_info.h>
#include <htc_version_info.h>
#include <htc_security_util.h>
#include <CFG_HTC_DATA_File.h>
#include <debug.h>
#include <sys/types.h>
#include <platform/partition.h>
#include <platform/boot_mode.h>
#include <dev/keys.h>
#include <bits.h>

//#include "ftm_lib_emmc_api.h"

#define CID_SIZE 8
#define IMEI_SIZE 15
#define MEID_SIZE 14
#define BOARD_INFO_MAGIC "HTC-BOARD-INFO!@"
#define BOARD_INFO_SIZE	1024
#define MAGIC_BOOT_2_KERNEL         0xAABBCCDD
#define CMDLINE_MAX_LEN	1024
//#define RESET_MSG_LENGTH 512

#define RESET_MSG_LENGTH HTC_REBOOT_INFO_REBOOT_STRING_SIZE

#define KERNEL_KMEMLEAK_FLAG  (BIT(4))
#define BOOTLOADER_CRYPTO_CHECK_FLAG (BIT(5))
#define BOOTLOADER_DISABLE_FORCE_DISK_ENCRYPTION  (BIT(19))

static const char* kernelflag_cmdline        = " kernelflag=0x";
static const char* debugflag_cmdline         = " debugflag=0x";
static const char* radioflag_cmdline         = " radioflag=0x";
static const char* radioflagex1_cmdline      = " radioflagex1=0x";
static const char* radioflagex2_cmdline      = " radioflagex2=0x";
static const char* device_hw_rev_cmdline     = " androidboot.devicerev=";
static const char* abnormal_reset_cmdline    = " abnrst=";
static const char *secure_level_cmdline      = " td.sf=";
static const char *atsdebug_cmdline          = " ro.atsdebug=";
static const char *getebd_cmdline            = " ro.getebd=";
extern int get_ebdlog;

static const char *unlock_cmdline            = " androidboot.lb=";
int unlock_status;

/* for calibration tool in CSD */
static const char *root_cmdline	= " ro.secure=";
static const char *op_cmdline		= " ro.op=";
extern int root_magic;

static const char *cid_cmdline = " androidboot.cid=";
static const char *mid_cmdline = " androidboot.mid=";

static const char* bootloader_ver_cmdline    = " androidboot.bootloader=";

#ifdef MFG_BUILD
static const char *meta_auto_nvram_backup = " androidboot.meta.auto_nvram_backup=";
#endif

static const char* maxcpus_cmdline    = " maxcpus=";

#if TAMPER_DETECT
static const char *tpd_td_cmdline            = " td.td=";
static const char *tpd_ofs_cmdline           = " td.ofs=";
static const char *tpd_prd_cmdline           = " td.prd=";
static const char *tpd_dly_cmdline           = " td.dly=";
static const char *tpd_tmo_cmdline           = " td.tmo=";
#endif


HTC_BOARD_INFO* board_info = NULL;

//quote from hboot function chipset_setting_init
void htc_board_info_init()
{
		bool found = false;

		board_info = (HTC_BOARD_INFO *)memalign(4096, sizeof(HTC_BOARD_INFO));
		if(!board_info)
			goto err;

		memset(board_info, 0, sizeof(HTC_BOARD_INFO));

		partition_read("proinfo", HTC_BOARD_INFO_OFFSET, board_info, (board_info->end - board_info->start));

		if(strncmp(board_info->data.magic, BOARD_INFO_MAGIC, sizeof(board_info->data.magic))) {
			dprintf(CRITICAL, "%s: invalid board info\r\n", __func__);
			goto clear;
		}

		return;

	clear:
		memset(board_info, 0, sizeof(HTC_BOARD_INFO));
	err:
		dprintf(CRITICAL, "%s: fail to read board info\r\n", __func__);
	restore:
		memcpy(board_info->data.magic, BOARD_INFO_MAGIC, sizeof(board_info->data.magic));
		partition_write("proinfo", HTC_BOARD_INFO_OFFSET, board_info, (board_info->end - board_info->start));
}

int htc_get_pid()
{
	if(board_info == NULL) {
		dprintf(CRITICAL, "%s: board_info is NULL\r\n", __func__);
		return -1;
	} else {
		return board_info->data.pid;
	}
}

//from hboot include/utils.h
#define HEXSYMBOLCHK(c) (((c) >= '0' && (c) <= '9') ||\
                                                ((c) >= 'A' && (c) <= 'Z') ||\
                                                ((c) >= 'a' && (c) <= 'z') ||\
                                                ((c) == '-' || (c) == '_'))
//from hboot common/utils.c
static int invalid_cid(const char *cid)
{
	int i;
	if (cid == NULL)
		return -1;

	for (i = 0; i < CID_SIZE; i++) {
		if (HEXSYMBOLCHK(cid[i]))
			continue;
		else
			return -1;
	}

	/*must be NULL-terminated string*/
	if (!(cid[i] == NULL))
		return -1;

	return 0;
}

char *htc_chipset_get_cid(void)
{
	char cid[CID_SIZE + 1];

	if (!board_info || !board_info->data.cid)
		return NULL;

	memcpy(cid, board_info->data.cid, CID_SIZE);
	cid[CID_SIZE] = 0;

	if (cid[0] == 0x0 || cid[0] == 0xff || invalid_cid(cid)) {
		dprintf(CRITICAL, "fake cid: 11111111\r\n");
		return "11111111";
	}

	return board_info->data.cid;
}

//function in app/aboot/aboot.c
//extern uint32_t get_page_size();
uint32_t get_page_size() {
	//TODO:
	return 2048;
}

//===== from hboot incluce/setting.h =====
#define DEFAULT_SERIAL_NO "123456789012"
#define MISC_MAGIC "\x46\x4e\x4f\x43"
#define MAX_MFG_AWB_CAL_DATA_SIZE	(8 * 2048) /* Max 16384 bytes. */
#define BOOTLOADER_CONFIG_MAX 512

#define MAX_HEAP_TEST_PARAM_ARGC		8
#define MAX_HEAP_TEST_PARAM_ARGV_LEN		16
struct heap_test_param{
	int argc;
	char argv[MAX_HEAP_TEST_PARAM_ARGC][MAX_HEAP_TEST_PARAM_ARGV_LEN];
};

typedef struct {
    char command[32];
    char status[32];
    char recovery[768];
    char stage[32];
    char reserved[224];
} bootloader_message;

struct setting_data_2k_page{
	//misc partition
	//page 0
	char misc_page0_start[0];
	char backup_cid[16];			//0x000
	char auto_enter_bootloader[16];		//0x010
	char cold_boot[16];			//0x020
	char enter_updateloader[16];		//0x030
	char org_sdupdate_protect[16];		//0x040	(original hboot-kant ruu_nbh[])
	char radio_serial_debug[16];		//0x050
	char debug_cable[16];			//0x060
	char radio_usb_debug[16];		//0x070
	char not_use_0[16];			//0x080	(first 8 chars will be set to 0x0 by DIAG before SHIP)
	char org_autodownload[16];			//0x090 /* moved to PRO_INFO */
	char org_firmware_main_version[16];		//0x0A0 /* moved to PRO_INFO */
	char ruu_protect[16];			//0x0B0
	char hboot_preupdate[16];		//0x0C0
	char prefer_diag[16];			//0x0D0
	char logic_partition_update[16]; 	//0x0E0
	char special_ramdump[16];		//0x0F0
	char gift[16];				//0x100
	int offmode_alarm[10];			//0x110
	char vzw_hpst[16];			//0x138 (for vzw multiflash tool)
	char tamper_info[256];			//0x148 (for tamper detection)
	int offmode_alarm_snooze[10];		//0x248
	unsigned int reset_hw_reason;		//0x270

	char enable_log[68];			//0x274 (enable log in release ROM )
	char username[68];			//0x2B8 (for watermark)
	char reset_enable_log[8];		//0x2FC (for reset enable log in release ROM)

	unsigned int gift_files_count;		//0x304 (for gift function to the count of files copy)
	unsigned int gift_userdata_crc;		//0x308 (save the userdata crc before do gift function)
	char keycard_checked[8];		//0x30C (for checking keycard)
	char wipe_data_done[8];			//0x314 (factory reset done, for calibration tool in ASP)
	char rom_info_pre[8];			//0x31C (save previous rom information)
	char rom_info_cur[8];			//0x324 (save current rom information)
	char zip_info[32];			//0X32C (Add hidden information to EXE/Zip files)
	char heap_test[16];			//0x34C (heap test bottom half)
	struct heap_test_param ht_param; //0x35C
	char qsc_imei_update_result[16]; //0x3E0
	char qsc_version[16]; //0x3F0
	char temp_main_version[16];		//0x400
	char gdvoucher_redeemed[16];    //0x410
	int batt_magic_num;				//0x420
	int batt_soc; 					//0x424
	int batt_ocv_uv; 				//0x428
	int batt_cc_uah;				//0x42c
	unsigned long batt_stored_time;			//0x430
	unsigned int gift_files_size;       //0x434(for gift function to the size of files copy)
	int pre_delta_vddmax_mv;				//0x438
	unsigned int force_sec_boot_reason;
	char reserve0[2048-16*23-4*30-256-66-68-8-2-8*4-32-132-4-0x200];    //0x43C
	char reserved_for_htc_reboot_info[0x200];
	char misc_page0_end[0];
	//page 1
	char misc_page1_start[0];
	char command_old[32];
	char status_old[32];
	char recovery_old[1024];
	int ruu_first_boot;
	int gencheckpt_flag;
	int recovery_counter;
	char repartition_state[128];
	char reserve1[2048-1228];
	char misc_page1_end[0];
	//page 2
	char misc_page2_start[0];
	int org_cfgdata[BOOTLOADER_CONFIG_MAX]; /* moved to PRO_INFO */
	char misc_page2_end[0];
	// page 3
	char misc_page3_start[0];
	int reserve3[512];
	char misc_page3_end[0];
	// page 4
	char misc_page4_start[0];
	int AWB_cal_data[512];
	char misc_page4_end[0];
	// page 5
	char misc_page5_start[0];
	int reserve5[512];
	char misc_page5_end[0];
	// page 6
	char misc_page6_start[0];
	char imsi[16];	//USIM lock feature requested by SoftBank
	long timestamp;
	char reserve6[2048-16-4];
	char misc_page6_end[0];

	//internal
	char model_name[64];
	char radio_version[64];
	char cpld_version[16];
	char microp_version[16];
	char tp_version[32];
	char nfc_version[16];
	char cir_version[16];
	char cap_version[16];
	char adsp_version[32];
	char cid[16];
	char lcd_version[48];
	char ois_version[16];
	int bootmode;
	int batt_level_check;
	int panel_type;	// for hero panel use, remove after EVT
	int hboot_preupdate_flag;
	int ps_type[1];
	int cam_type;
	int gy_type[1];
	int compass_type[1];
	unsigned char mtk_barcode[64];

	//PRO_INFO Start
	HTC_MFG	mfg;
	//PRO_INFO End

	//HTC BCB Start
	bootloader_message boot;
	//HTC BCB End
};
//===== EndOf from hboot incluce/setting.h =====

static struct setting_data_2k_page *setting = NULL;
#define SETTING_PAGE_SIZE	(2048)

static int setting_misc_is_valid(void)
{
	/**
	 * We only use first 10 DWORD (0x0 to 0x9) in cfgdata
	 * and cfgdata[255] is for MISC protection (4 bytes: "\x43\x4f\x4e\x46")
	 *
	 * return 1 is valid, 0 is INVALID
	 */

	if (!memcmp(&setting->org_cfgdata[255], MISC_MAGIC, 4))
		return true;

	return false;
}

static int setting_cfgdata_is_valid(void)
{
	/**
	 * We only use first 10 DWORD (0x0 to 0x9) in cfgdata
	 * and cfgdata[255] is for MISC protection (4 bytes: "\x43\x4f\x4e\x46")
	 *
	 * return 1 is valid, 0 is INVALID
	 */

	if (!memcmp(&setting->mfg.data.cfgdata[255], MISC_MAGIC, 4))
		return true;

	return false;
}

static int setting_misc_set_valid(void)
{
	memcpy(&setting->org_cfgdata[255], MISC_MAGIC, 4);

	return 0;
}

static int setting_cfgdata_set_valid(void)
{
	memcpy(&setting->mfg.data.cfgdata[255], MISC_MAGIC, 4);

	return 0;
}

static int setting_page_range_check(void)
{
#if defined(ENG_BUILD)
	/* The page size in misc setting should be equal to 2K */
	if (((setting->misc_page0_end - setting->misc_page0_start) != SETTING_PAGE_SIZE) ||
		((setting->misc_page1_end - setting->misc_page1_start) != SETTING_PAGE_SIZE) ||
		((setting->misc_page2_end - setting->misc_page2_start) != SETTING_PAGE_SIZE) ||
		((setting->misc_page3_end - setting->misc_page3_start) != SETTING_PAGE_SIZE) ||
		((setting->misc_page4_end - setting->misc_page4_start) != SETTING_PAGE_SIZE) ||
		((setting->misc_page5_end - setting->misc_page5_start) != SETTING_PAGE_SIZE) ||
		((setting->misc_page6_end - setting->misc_page6_start) != SETTING_PAGE_SIZE)) {
		dprintf(CRITICAL, "[ERROR] Loop here, some page size in MISC setting is not equal to %d...\r\n", SETTING_PAGE_SIZE);
		while (1) ;
	}
#endif

	return 0;
}

static int setting_misc_partition_read(const char *name)
{
	int page_size;
	int mtk_env_end_offset = get_env_offset() + get_env_size();
	page_size = get_page_size();

	//BCB Read Sart
	partition_read(name, 0, &setting->boot, sizeof(setting->boot));
	//BCB Read End

	partition_read(name, mtk_env_end_offset + 0, setting->misc_page0_start, (setting->misc_page0_end - setting->misc_page0_start));
	partition_read(name, mtk_env_end_offset + page_size, setting->misc_page1_start, (setting->misc_page1_end - setting->misc_page1_start));
	partition_read(name, mtk_env_end_offset + (2 * page_size), setting->misc_page2_start, (setting->misc_page2_end - setting->misc_page2_start));
	partition_read(name, mtk_env_end_offset + (3 * page_size), setting->misc_page3_start, (setting->misc_page3_end - setting->misc_page3_start));
	partition_read(name, mtk_env_end_offset + (4 * page_size), setting->misc_page4_start, (setting->misc_page4_end - setting->misc_page4_start));
	partition_read(name, mtk_env_end_offset + (5 * page_size), setting->misc_page5_start, (setting->misc_page5_end - setting->misc_page5_start));
	partition_read(name, mtk_env_end_offset + (6 * page_size), setting->misc_page6_start, (setting->misc_page6_end - setting->misc_page6_start));

	return 0;
}

static int setting_misc_partition_update(const char *name)
{
	int page_size;
	int size;
	char *pmisc;
	int ret = 0;
	int mtk_env_end_offset = get_env_offset() + get_env_size();

	page_size = get_page_size();
	size = sizeof(struct setting_data_2k_page);
	pmisc = (char *)memalign(4096, size);

	if (pmisc == NULL) {
		dprintf(CRITICAL, "Heap allocating for saving MISC is failed!\r\n");
		return -1;
	}
	memset(pmisc, 0, size);

	partition_read(name, mtk_env_end_offset, pmisc, size);

	memcpy(pmisc, setting->misc_page0_start, (setting->misc_page0_end - setting->misc_page0_start));
	memcpy(pmisc + page_size, setting->misc_page1_start, (setting->misc_page1_end - setting->misc_page1_start));
	memcpy(pmisc + (2 * page_size), setting->misc_page2_start, (setting->misc_page2_end - setting->misc_page2_start));
	memcpy(pmisc + (3 * page_size), setting->misc_page3_start, (setting->misc_page3_end - setting->misc_page3_start));
	memcpy(pmisc + (4 * page_size), setting->misc_page4_start, (setting->misc_page4_end - setting->misc_page4_start));
	memcpy(pmisc + (5 * page_size), setting->misc_page5_start, (setting->misc_page5_end - setting->misc_page5_start));
	memcpy(pmisc + (6 * page_size), setting->misc_page6_start, (setting->misc_page6_end - setting->misc_page6_start));

	//BCB Save Sart
	partition_write(name, 0, &setting->boot, sizeof(setting->boot));
	//BCB Save End

	//===== write partition ======================================
	partition_write(name, mtk_env_end_offset, pmisc, size);
	//===== end of write partition ===============================

free_and_return:

	free(pmisc);
	return 0;
}

static int setting_save_misc(void)
{
	setting_misc_partition_update("para");

	if (partition_get_size("misc3") > 0) { // MISC3 should be not all bad blocks
		// Also save data to MISC3
		setting_misc_partition_update("misc3");
	} else if (partition_get_size("misc2") > 0) { // MISC2 should be not all bad blocks
		// Also save data to MISC2
		setting_misc_partition_update("misc2");
	}

	return 0;
}

static int setting_restore_misc(void)
{
	dprintf(INFO, "Restore data from original MISC...\r\n");

	if (partition_get_size("misc3") > 0) { // MISC3 should be not all bad blocks
		// Read data from MISC3
		setting_misc_partition_read("misc3");
	} else if (partition_get_size("misc2") > 0) { // MISC2 should be not all bad blocks
		// Read data from MISC2
		setting_misc_partition_read("misc2");
	}

	// Save data to MISC
	setting_misc_set_valid();
	// Prevent restoring recovery command.
	memset(setting->boot.command, 0, sizeof(setting->boot.command));
	memset(setting->boot.status, 0, sizeof(setting->boot.status));
	memset(setting->boot.recovery, 0, sizeof(setting->boot.recovery));
	setting_save_misc();
	return 0;
}

static int setting_save_mfg(void)
{
	partition_write("proinfo", HTC_MFG_OFFSET, &setting->mfg, sizeof(setting->mfg));
	return 0;
}

void setting_invalidate_checksum()
{
	if(setting->gift_userdata_crc != 0) {
		dprintf(CRITICAL, "%s: set gift_userdata_crc from 0x%8X to 0x0\r\n", __func__, setting->gift_userdata_crc);
		setting->gift_userdata_crc = 0;
		setting_save_misc();
	}
}

void htc_setting_set_firmware_main_version(const unsigned char *version)
{
	memset(setting->mfg.data.firmware_main_version, 0, sizeof(setting->mfg.data.firmware_main_version));
	strncpy(setting->mfg.data.firmware_main_version, version, (sizeof(setting->mfg.data.firmware_main_version) - 1));

	setting_save_mfg();

	return;
}

char *htc_setting_get_firmware_main_version()
{
	setting->mfg.data.firmware_main_version[sizeof(setting->mfg.data.firmware_main_version) - 1] = 0;
	return setting->mfg.data.firmware_main_version;
}

static int setting_check_and_update_firmware_main_version()
{
	char* p_reserve_buf = NULL;
	struct htc_android_info_struct htc_android_info;
	size_t reserve_partition_size = (size_t)partition_get_size_by_name("reserve");

	if ( (long)reserve_partition_size < 0 ) {
		dprintf(CRITICAL, "get 'reserve' partition size error!\r\n");
		return -1;
	}

	if (reserve_partition_size > 4096*2) {
		reserve_partition_size = 4096*2;
	}

	if (NULL == (p_reserve_buf = (char *)memalign(4096, reserve_partition_size))) {
		dprintf(CRITICAL, "memalign for reserve_partition_size(%lu) failed!\r\n", reserve_partition_size);
		return -1;
	}

	if (0 >= partition_read("reserve", 0, p_reserve_buf, reserve_partition_size)) {
		dprintf(CRITICAL, "[setting] partition read reserve partition failed! size:%lu bytes\r\n", reserve_partition_size);
		goto exit;
	}

	p_reserve_buf[reserve_partition_size - 1] = '\0';

	if ( strlen(p_reserve_buf) != 0 ) {
		memset(&htc_android_info, 0, sizeof(struct htc_android_info_struct));

		if (0 == parsing_android_info( p_reserve_buf, &htc_android_info) ) {
			if ( strlen(htc_android_info.image_mainver) > 0) {
				dprintf(CRITICAL, "[setting] htc_android_info.image_mainver:%s\r\n",
									htc_android_info.image_mainver);
				htc_setting_set_firmware_main_version(htc_android_info.image_mainver);
			}
		}

		dprintf(CRITICAL, "[setting] clean reserve partition\r\n");
		memset(p_reserve_buf, 0, reserve_partition_size);
		partition_write("reserve", 0, p_reserve_buf, reserve_partition_size);
	}

exit:
	if (p_reserve_buf != NULL) {
		free(p_reserve_buf);
	}

	return 0;
}

static void htc_setting_get_modelid(char *pszModelIDBuf)
{
	int i;

	pszModelIDBuf[0] = 0;

	//convert 2 byte wide character to one byte character
	if(setting->mfg.data.model_id[0])	{
		for(i = 0; i < sizeof(setting->mfg.data.model_id)/2; i++)
			pszModelIDBuf[i] = setting->mfg.data.model_id[i*2];

		pszModelIDBuf[sizeof(setting->mfg.data.model_id)/2] = 0;
	}
#if 0 //TODO: LK Not support board_project_name() and BOARD IOCTL
	if ((pszModelIDBuf[0] == 0x00) || (pszModelIDBuf[0] == 0xFF)) {
		pszModelIDBuf[0] = 0;
		if (strlen(board_project_name()) >= board_ioctl(GET_MID_LEN)) {
			memcpy(pszModelIDBuf, board_project_name(), board_ioctl(GET_MID_LEN));
			pszModelIDBuf[board_ioctl(GET_MID_LEN)] = 0;
		}else {
			memcpy(pszModelIDBuf, board_project_name(), strlen(board_project_name()));
			pszModelIDBuf[strlen(board_project_name())] = 0;
			for (i=0; i<(board_ioctl(GET_MID_LEN)-strlen(board_project_name())); i++)
				strcat(pszModelIDBuf, "*");
		}

		for (i=board_ioctl(GET_MID_LEN); i<CHECK_MID_LEN; i++)
			strcat(pszModelIDBuf, "*");

		SECMSG("GetModelName- Default[%s]\r\n", pszModelIDBuf);
	}
#endif
}

#if TAMPER_DETECT
void setting_get_tamper_info(char *data)
{
	memcpy(data, setting->tamper_info, sizeof(setting->tamper_info));
}

void setting_set_tamper_info(const char* data)
{
	memcpy(setting->tamper_info, data, sizeof(setting->tamper_info));
	setting_save_misc();
	return;
}

long setting_get_tamper_info_offset(void)
{
	return (ssize_t) setting->tamper_info - (ssize_t) setting->misc_page0_start;
}

/* Initial display tamper string */
int setting_init_tamper_status()
{
	tamper_get_status();

	return 0;
}
#endif //!TAMPER_DETECT

unsigned int setting_get_force_sec_boot_reason (void)
{
	return setting->force_sec_boot_reason;
}

void setting_set_force_sec_boot_reason (unsigned int reason)
{
#if TAMPER_DETECT
	setting->force_sec_boot_reason = reason;
	setting_save_misc();
#endif
}

int htc_setting_init(void)
{
	int page_size;

	page_size = get_page_size();

	setting = (struct setting_data_2k_page *)memalign(4096, sizeof(struct setting_data_2k_page));

	if (!setting) {
		dprintf(CRITICAL, "Allocte heap for setting struct error!\r\n");
		return -2;
	}

	memset(setting, 0, sizeof(struct setting_data_2k_page));
	setting_page_range_check();

	setting_misc_partition_read("para");

	if (setting_misc_is_valid() == false) { // Read failed OR org_cfgdata[] is invalid
		// Restore data from original MISC
		setting_restore_misc();
	}

	//DEBUG MISC Size RANGE
	#if 0
	memset(setting->backup_cid, '1', sizeof(setting->backup_cid)/sizeof(char));
	memset(setting->reserve0, 0xE0, sizeof(setting->reserve0));
	memset(setting->reserve6, 0xE6, sizeof(setting->reserve6));
	setting_save_misc();
	#endif

	partition_read("proinfo", HTC_MFG_OFFSET, &setting->mfg, sizeof(setting->mfg));
	if (setting_cfgdata_is_valid() == false) { // Read failed OR cfgdata[] is invalid
		setting_cfgdata_set_valid();
		setting_save_mfg();
	}

	if(htc_chipset_get_cid()) {
		memset(setting->cid, 0, sizeof(setting->cid));
		memcpy(setting->cid, htc_chipset_get_cid(), ((sizeof(setting->cid) - 1) < CID_SIZE)? (sizeof(setting->cid) - 1): CID_SIZE);
	}
#if 1
	partition_read("proinfo", MTK_BARCODE_OFFSET, &setting->mtk_barcode, sizeof(setting->mtk_barcode));

	if ((setting->mtk_barcode[0] == 0xFF) || (setting->mtk_barcode[0] == 0x0)) {
		dprintf(INFO, "Adopting default serial NO. (%s)\r\n", DEFAULT_SERIAL_NO);
		memset(setting->mtk_barcode, 0, sizeof(setting->mtk_barcode));
		strncpy(setting->mtk_barcode, DEFAULT_SERIAL_NO, (sizeof(setting->mtk_barcode) - 1));
	} else {
		setting->mtk_barcode[sizeof(setting->mtk_barcode) - 1] = 0;
	}

#else
	if ((setting->mfg.data.serrial_no[0] == 0xFF) || (setting->mfg.data.serrial_no[0] == 0x0)) {
		dprintf(INFO, "Adopting default serial NO. (%s)\r\n", DEFAULT_SERIAL_NO);
		memset(setting->mfg.data.serrial_no, 0, sizeof(setting->mfg.data.serrial_no));
		strncpy(setting->mfg.data.serrial_no, DEFAULT_SERIAL_NO, (sizeof(setting->mfg.data.serrial_no) - 1));
	} else {
		setting->mfg.data.serrial_no[sizeof(setting->mfg.data.serrial_no) - 1] = 0;
	}
#endif
	setting_check_and_update_firmware_main_version();

	setting->batt_level_check = 1;

#if TAMPER_DETECT
		setting_init_tamper_status();
#endif

	return 0;
}

//bootmode
static const char enter_fastboot_str[] = "EnterFastboot";
static const char reboot_str[] = "Reboot";
//revovery command
const char boot_recovery_str[] = "boot-recovery";
const char update_zip_str[] = "update-zip";

#define MAX_TIMES_TO_TRY_ENTER_RECOVERY (10)
static int setting_if_enter_recovery_mode(void)
{
	int if_enter_recovery = 0;

	if (!memcmp(setting->boot.command, boot_recovery_str, sizeof(boot_recovery_str))) {
		if (setting->recovery_counter >= MAX_TIMES_TO_TRY_ENTER_RECOVERY) {
			/* too many times to try to enter recovery mode in a row
			 * clear the command
			 */
			memset(setting->boot.command, 0, sizeof(setting->boot.command));
			memset(setting->boot.status, 0, sizeof(setting->boot.status));
			memset(setting->boot.recovery, 0, sizeof(setting->boot.recovery));
			setting->recovery_counter = 0;
			setting_save_misc();
		} else {
			setting->recovery_counter++;
			setting_save_misc();
			if_enter_recovery = 1;
		}
	} else {
		if (setting->recovery_counter) {
			setting->recovery_counter = 0;
			setting_save_misc();
		}
	}

	return if_enter_recovery;
}

static int setting_check_bootmode(void)
{
		if (setting_if_enter_recovery_mode())
				return BOOTMODE_RECOVERY;

		if (!memcmp(setting->auto_enter_bootloader, enter_fastboot_str, sizeof(enter_fastboot_str))) {
				memset(setting->auto_enter_bootloader, 0, sizeof(setting->auto_enter_bootloader));
				setting_save_misc();
				return BOOTMODE_FASTBOOT;
		}

		if (!memcmp(setting->auto_enter_bootloader, reboot_str, sizeof(reboot_str))) {
				memset(setting->auto_enter_bootloader, 0, sizeof(setting->auto_enter_bootloader));
				setting_save_misc();
				return BOOTMODE_REBOOT;
		}

		if(!memcmp(setting->boot.command, update_zip_str, sizeof(update_zip_str)))
		{
			return BOOTMODE_DOWNLOAD_RUU ;
		}

		return BOOTMODE_NORMAL;
}

static int chipset_bootmode(void)
{
	unsigned int reset_reason = htc_get_reboot_reason();

	dprintf(CRITICAL,"%s(): reset_reason=0x%08X\r\n", __func__, reset_reason);

	//save_kernel_abnormal_reset_reason(reset_reason);

	if ((reset_reason & 0xFFFFFF00) == 0x77665500){
		switch (reset_reason & 0xFF){
		case 0x00:	//stay in bootloader, fastboot mode
		case 0xCC:	//for DDR2G ramdump/ramtest bottom-half
			return BOOTMODE_FASTBOOT;

		case 0x01:	//boot boot.img
			return BOOTMODE_REBOOT;

		case 0x02:	//boot recovery.img
			return BOOTMODE_RECOVERY;

		default:	//boot boot.img
			return BOOTMODE_REBOOT;
		}
	}else if ((reset_reason & 0xFFFFFF00) == 0x6F656D00){
		//oem reset reason, TBD
		switch (reset_reason & 0xFF) {
		case 0x11:  // oem-11, skip ram dump and boot to kernel
			// NOT IMPEMENTED YET
			break;

#if TAMPER_DETECT
		case 0x68:
			dprintf(INFO, "Tamper-Detect do sync!\r\n");
			tamper_sync_from_misc();
			break;
#endif //! TAMPER_DETECT

		case 0x76:  // oem-76,
			// NOT IMPEMENTED YET
			break;

		case 0x78:  // oem-78, rebootRUU
			return BOOTMODE_DOWNLOAD_RUU;

		case 0x88:  // use "adb shell reboot oem-88" to trigger kernel rebooting to normal mode
			return BOOTMODE_NORMAL;

		case 0x8A: // trigger remote kill, only do this on s-off devices
			if (read_security_level() == SECLEVEL_MFG)
			{
				partition_format_emmc("hosd");
				partition_format_emmc("system");
				partition_format_emmc("userdata");
				reboot_device(0);
			}
			return BOOTMODE_NORMAL;

		case 0xE0:	// oem-E0, download mode
			return BOOTMODE_DOWNLOAD;

		case 0xF1:	// oem-F1, for FTM1 /* MTK META_BOOT */
			return BOOTMODE_FTM1;
		case 0xF2:  // oem-F2, for FTM2 /* MTK FACTORY_BOOT */
			return BOOTMODE_FTM2;
                case 0xFF:
                        return BOOTMODE_DDRTEST;
#ifdef MFG_BUILD
		case 0xF3:  // oem-F3, for FTM3 /* MTK META_BOOT and auto Backup NVRAM then reboot to OS */
			return BOOTMODE_FTM3;
#endif
		default:
			return BOOTMODE_NORMAL;
		}
	}

	return BOOTMODE_NORMAL;
}

//use a dummy function for the time being
static int board_bootmode(void)
{
	return BOOTMODE_NORMAL;
}

static int bootmode_key_check(void)
{
	if (keys_get_state(KEY_VOLUMEUP) && keys_get_state(KEY_VOLUMEDOWN)) {
		return BOOTMODE_FASTBOOT;
	} else if(keys_get_state(KEY_VOLUMEUP)) {
		return BOOTMODE_FTM2;
	} else if(keys_get_state(KEY_VOLUMEDOWN)) {
		return BOOTMODE_DOWNLOAD;
	}

	return BOOTMODE_NORMAL;
}

int htc_setting_init_bootmode(void)
{
	setting->bootmode = setting_check_bootmode();

	if(setting->bootmode == BOOTMODE_NORMAL)
		setting->bootmode = chipset_bootmode();

	if(setting->bootmode == BOOTMODE_NORMAL)
		setting->bootmode = board_bootmode();

	if(setting->bootmode == BOOTMODE_NORMAL)
		setting->bootmode = bootmode_key_check();

	return 0;
}

int htc_setting_get_bootmode(void)
{
	return setting->bootmode;
}

void htc_setting_set_bootmode(int bootmode)
{
        setting->bootmode = bootmode;
}

int htc_setting_cfgdata_get(int id)
{
	if (id < BOOTLOADER_CONFIG_MAX) {
		return setting->mfg.data.cfgdata[id];
	}
	return -1;
}

int htc_setting_cfgdata_set(int id, int data)
{
	if (id < BOOTLOADER_CONFIG_MAX) {
		setting->mfg.data.cfgdata[id] = data;
		setting_save_mfg();
		return 0;
	}
	return -1;
}

int htc_setting_cfgdata_erase()
{
	memset(setting->mfg.data.cfgdata,0,sizeof(setting->mfg.data.cfgdata));
	setting_cfgdata_set_valid();
	setting_save_mfg();
	return 0;
}

int htc_setting_kernelflag_get()
{
	return htc_setting_cfgdata_get(0x6);
}

int htc_setting_bootloaderflag_get()
{
	return htc_setting_cfgdata_get(0x7);
}

int htc_setting_debugflag_get()
{
	return htc_setting_cfgdata_get(0x5);
}

int htc_setting_radioflag_get()
{
	return htc_setting_cfgdata_get(0x8);
}

int htc_setting_radioflag_ex1_get()
{
	return htc_setting_cfgdata_get(0xa);
}

int htc_setting_radioflag_ex2_get()
{
	return htc_setting_cfgdata_get(0xb);
}

/*writeconfig 8 0x10*/
int htc_smart_mdlog()
{
	if(htc_setting_radioflag_get() & 0x10)
		return 1 ;
	else
		return 0 ;
}

unsigned char* htc_setting_get_barcode() {
	return setting->mtk_barcode;
}

unsigned char htc_setting_get_bomid(void)
{
	return setting->mfg.data.skuid.bom_id;
}

/* get MFG revision high byte for driver use.*/
unsigned char htc_setting_pcbid(void)
{
	return setting->mfg.data.skuid.pcb_id;
}

/* get MFG revision low byte for OS use.*/
unsigned char htc_setting_pcbid2(void)
{
	return setting->mfg.data.skuid.pcb_id2;
}

void htc_setting_pcbid_write(unsigned char pcbid)
{
	setting->mfg.data.skuid.pcb_id = pcbid;

	setting_save_mfg();
}

unsigned htc_setting_engineerid(void)
{
	return (*(unsigned *)setting->mfg.data.skuid.engineer_id) & ~0xFFFFFFF0;
}

char * htc_setting_skuid(void)
{
	return (char *)&setting->mfg.data.skuid;
}

int htc_setting_skuid_read(int id)
{
	int *skuid = (int *)&setting->mfg.data.skuid;
	return skuid[id];
}

int htc_setting_skuid_write(int id, int data, int true_write_flag)
{
	int *skuid = (int *)&setting->mfg.data.skuid;
	skuid[id] = data;

	if (true_write_flag)
		setting_save_mfg();

	return 0;
}

char* htc_setting_color_ID_read(void)
{
	return (char*)setting->mfg.data.color_ID;
}

char* get_reset_msg(void)
{
	return htc_get_reboot_string();
}

static int is_kernel_abnormal_reset(void)
{
	int ret = 0;
	unsigned reboot_mode = 0;
	char buf[RESET_MSG_LENGTH];

	reboot_mode = check_reboot_mode();

	if (0x776655AA == reboot_mode) {
		// read reset message
		memcpy(buf, get_reset_msg(), RESET_MSG_LENGTH);

		// force-hard, force-dog-bark are normal reset. Powerkey Hard reset will set to 2
		if ((!strncmp(buf, "force-hard", 10)) || (!strncmp(buf, "force-dog-bark", 14)))
			ret = 0;
		else if(!strncmp(buf, "Powerkey Hard Reset", 19))
			ret = 2;
		else if (!strncmp(buf, "TZ: NON_SECURE_WDT", 18) || !strncmp(buf, "Unknown", 7))
			ret = 3;
		else
			ret = 1;
	} else if (0x6F656D99 == reboot_mode)
		ret = 1;

	return ret;
}

static void clean_kernel_abnormal_reset()
{
	//uint32_t restart_reason_addr;

	//restart_reason_addr = RESTART_REASON_ADDR;
	//writel(MAGIC_BOOT_2_KERNEL, restart_reason_addr);
	htc_set_reboot_reason(MAGIC_BOOT_2_KERNEL, NULL);
}

int get_and_clean_kernel_abnormal_reset()
{
	int ret=0;

	ret = is_kernel_abnormal_reset();
	clean_kernel_abnormal_reset();

	return ret;
}

const char* htc_setting_cid(void)
{
	return setting->cid;
}

char * htc_setting_get_backup_cid(void)
{
	if (CID_SIZE < sizeof(setting->backup_cid))
		setting->backup_cid[CID_SIZE] = 0;

	return setting->backup_cid;
}

int htc_setting_check_backup_cid_empty(void)
{
	int i, ub;

	ub = (CID_SIZE < (sizeof(setting->backup_cid)-1))? CID_SIZE: (sizeof(setting->backup_cid) - 1);
	for (i = 0; i < ub; i++) {
		if ((setting->backup_cid[i] != 0x00) && (setting->backup_cid[i] != 0xFF))
			return false;
	}
	return true;
}

int htc_setting_check_device_is_super_cid(void)
{
	char s2 = setting->cid[0];
	int i;

	for (i=0; i<strlen(setting->cid); i++) {
		if ((setting->cid[i] == 0x0) || (setting->cid[i] == 0xFF) || (setting->cid[i] != s2)) {
			dprintf(CRITICAL, "Device CID is not super CID\r\n");
			return false;
		}
	}
	dprintf(CRITICAL, "Device CID is super CID\r\n");
	return true;
}

/* for calibration tool in ASP */
void htc_setting_set_wipe_done_flag(void)
{
        memset(setting->wipe_data_done, 0, sizeof(setting->wipe_data_done));
        setting_save_misc();
}

void htc_setting_reset_wipe_done_flag(void)
{
        memcpy(setting->wipe_data_done, "WIPEDONE", sizeof("WIPEDONE"));
        setting_save_misc();
}

const char* htc_setting_get_wipe_done_flag(void)
{
    return setting->wipe_data_done;
}

static char *get_cid_cmdline(void)
{
	char *cid;

	if (htc_setting_check_device_is_super_cid()) {
		/* CID is super CID */

		if (htc_setting_check_backup_cid_empty()){
			/* Backup CID is empty */
			cid = htc_setting_cid();
		} else {
			cid = htc_setting_get_backup_cid();
		}
	} else {
		cid = htc_setting_cid();
	}

	return cid;
}

static char *get_mid_cmdline(void)
{
	#define MIDLeng 32
	static char mid[MIDLeng + 1] = {0};

	htc_setting_get_modelid(mid);

	return &mid[0];
}

static char *get_bootloader_ver_cmdline(void)
{
	return (char*)HTC_LK_VERSION;
}

static int get_maxcpus_cmdline()
{
	switch (g_boot_mode) {
	case META_BOOT:
	case FACTORY_BOOT:
	case ADVMETA_BOOT:
	case ATE_FACTORY_BOOT:
		/* If MFG calibration mode, limit max 4 core */
		return 4;
#if _MAKE_HTC_LK
	case HTC_DOWNLOAD:
	case HTC_DOWNLOAD_RUU:
#endif
	case DOWNLOAD_BOOT:
		/* If download mode, limit max 4 core */
		return 4;
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	case KERNEL_POWER_OFF_CHARGING_BOOT:
	case LOW_POWER_OFF_CHARGING_BOOT:
		return 2;
#endif
	case RECOVERY_BOOT:
		/* If recovery mode, limit max 4 core */
		return 4;
	default:
		/* By default, no limit max cpus */
		return -1;
	}
}



int get_off_alarm_data(int *data, int secs)
{
	int i, ret =0;
	dprintf(CRITICAL,"Secs = %d\n", secs);
	if(secs > 0)
	{
		for(i = 0; i < 10; i ++)
		{
			dprintf(CRITICAL,"offalarm[%d] = %d\n", i, setting->offmode_alarm[i]);
			if(setting->offmode_alarm[i] == secs){
				*data = setting->offmode_alarm[i];
				ret = i + 1;
				break;
			}
		}
		if(ret == 0)
		{
			for(i = 0; i < 10; i ++)
			{
				dprintf(CRITICAL,"offalarm_snooze[%d] = %d\n", i, setting->offmode_alarm_snooze[i]);
				if(setting->offmode_alarm_snooze[i] == secs){
					*data = setting->offmode_alarm_snooze[i];
					ret = i + 1;
					break;
				}
			}
		}
	}
	return ret;
}


static void set_cmdline_str(char** dest, const char* const final, const char* key, const char* value)
{
	if (final != *dest) --(*dest);
	while ((*(*dest)++ = *key++));
	if (final != *dest) --(*dest);
	while ((*(*dest)++ = *value++));
}

int htc_cmdline_update(void** cmdline)
{
#define SET_BOOTARGS_KEYVAL(_key, _val) \
	do { \
		const char* tmp1 = _key; \
		const char* tmp2 = _val; \
		const char* key = ((tmp1) ? (_key) : ""); \
		const char* val = ((tmp2) ? (_val) : ""); \
		const size_t keyvalsz = strlen(key) + strlen(val); \
		\
		if (cmdline_len + keyvalsz < sz) { \
			set_cmdline_str(&dst, cmdline_final, key, val); \
			cmdline_len += keyvalsz; \
		} else { \
			dprintf(CRITICAL, "bootargs out of size (max: %u)\r\n", sz); \
			return -1; \
		} \
	} while(0)

#define SET_BOOTARGS_STR(_str) SET_BOOTARGS_KEYVAL((_str), "")

#define SET_BOOTARGS_INT(_key, __val) \
	do { \
		char _val[sizeof(int) * 3] = {0}; /* Imprecise algorithm */ \
		itoa((__val), _val, sizeof(_val), 10); \
		SET_BOOTARGS_KEYVAL((_key), _val); \
	} while(0)

#define SET_BOOTARGS_HEX(_key, __val) \
	do { \
		char _val[(sizeof(int) << 2) + 1] = {0}; \
		itoa((__val), _val, sizeof(_val), 16); \
		SET_BOOTARGS_KEYVAL((_key), _val); \
	} while(0)

	/* Generate cmdline */
	size_t sz = strlen(*cmdline) + CMDLINE_MAX_LEN;
	char *tmp_buf = (char*) malloc(sz);
	size_t cmdline_len = 0;
	char *dst = (char*) tmp_buf;
	char* cmdline_final = dst;
	char *cid;
	int rominfo = 0;
	int keycardid = 0;
	char tmp[64] = {0};

	if (*tmp_buf == NULL || sz == 0) {
		dprintf(CRITICAL, "%s: NULL buffer or zero buffer size\r\n", __func__);
	}

	/* for existing cmdline, dst points to '\0' for later processing. */
	cmdline_len = strlen(*cmdline);
	memcpy(tmp_buf, *cmdline, cmdline_len);

	if (cmdline_len >= sz) {
		dprintf(CRITICAL, "bootargs out of size (max: %u)\r\n", sz);
		return -1;
	}

	if (cmdline_len > 0 )
		dst += cmdline_len + 1;

	/* kernelflag */
	SET_BOOTARGS_HEX(kernelflag_cmdline, htc_setting_kernelflag_get());

	/* debugflag */
	SET_BOOTARGS_HEX(debugflag_cmdline, htc_setting_debugflag_get());

	/* devicerev */
	SET_BOOTARGS_INT(device_hw_rev_cmdline, htc_setting_pcbid2());

	/* abnormal reset */
	SET_BOOTARGS_INT(abnormal_reset_cmdline, get_and_clean_kernel_abnormal_reset());

	/* radioflag */
	SET_BOOTARGS_HEX(radioflag_cmdline, htc_setting_radioflag_get());

	/* radioflagext1 */
	SET_BOOTARGS_HEX(radioflagex1_cmdline, htc_setting_radioflag_ex1_get());

	/* radioflagext2 */
	SET_BOOTARGS_HEX(radioflagex2_cmdline, htc_setting_radioflag_ex2_get());

	/* BIT(4): 0x0000 0010 */
	if (htc_setting_kernelflag_get() & KERNEL_KMEMLEAK_FLAG)
		SET_BOOTARGS_STR(" kmemleak=on");
	else    /* Default */
		SET_BOOTARGS_STR(" kmemleak=off");

#if defined (MFG_BUILD)
               dprintf(CRITICAL, "disable default disk encryption in MFG hboot\r\n");
               SET_BOOTARGS_STR(" force_fde=off");
#else
               if (!setting_security() && (htc_setting_bootloaderflag_get() & BOOTLOADER_DISABLE_FORCE_DISK_ENCRYPTION)) {
                       dprintf(CRITICAL, "disable default disk encryption due to [7][80000]\r\n");
                       SET_BOOTARGS_STR(" force_fde=off");
               }
#endif

	if(strlen(htc_setting_color_ID_read()) > 0) {
		SET_BOOTARGS_STR(" color_ID=");
		SET_BOOTARGS_STR(htc_setting_color_ID_read());
	}

	/* androidboot.cid */
	SET_BOOTARGS_KEYVAL(cid_cmdline, get_cid_cmdline());

	/* androidboot.mid */
	SET_BOOTARGS_KEYVAL(mid_cmdline, get_mid_cmdline());

	/* androidboot.bootloader */
	SET_BOOTARGS_KEYVAL(bootloader_ver_cmdline, get_bootloader_ver_cmdline());

	/* for calibration tool in CSD */
	if(root_magic==0)
	{
		SET_BOOTARGS_INT(op_cmdline, 1);
		SET_BOOTARGS_INT(root_cmdline, 0);
		SET_BOOTARGS_INT(secure_level_cmdline, 0);
	} else if (read_security_level() == SECLEVEL_DEVELOPER) {
		/* ATS debug flag*/
		SET_BOOTARGS_INT(atsdebug_cmdline, 1);
		SET_BOOTARGS_INT(root_cmdline, 0);
		SET_BOOTARGS_INT(secure_level_cmdline, 1);
	} else {
		SET_BOOTARGS_INT(op_cmdline, 0);
		/* htc security flag */
		if (read_security_level() == SECLEVEL_USER || (htc_setting_bootloaderflag_get() & BOOTLOADER_CRYPTO_CHECK_FLAG)) {
			SET_BOOTARGS_INT(secure_level_cmdline, 1);
		} else if (read_security_level() < SECLEVEL_DEVELOPER || read_security_level() >= SECLEVEL_MFG) {
			SET_BOOTARGS_INT(secure_level_cmdline, 0);
		} else {
			SET_BOOTARGS_INT(secure_level_cmdline, 1);
		}
	}

	/* get ebd log */
	if (get_ebdlog == 1) {
		SET_BOOTARGS_INT(getebd_cmdline, 1);
	}

#if TAMPER_DETECT
		/* tamper detection */
		SET_BOOTARGS_INT(tpd_td_cmdline, tamper_get_status());
		SET_BOOTARGS_INT(tpd_ofs_cmdline, tamper_get_partition_offset());
		SET_BOOTARGS_INT(tpd_prd_cmdline, tamper_get_periof());
		SET_BOOTARGS_INT(tpd_dly_cmdline, tamper_get_delay());
		SET_BOOTARGS_INT(tpd_tmo_cmdline, tamper_get_timeout());
#endif //!TAMPER_DETECT

    SET_BOOTARGS_INT(unlock_cmdline, unlock_status);


	/* Assign the updated buffer to cmdline */
	*cmdline = tmp_buf;

	return cmdline_len;

#undef SET_BOOTARGS_INT
#undef SET_BOOTARGS_HEX
#undef SET_BOOTARGS_STR
#undef SET_BOOTARGS_KEYVAL
}
