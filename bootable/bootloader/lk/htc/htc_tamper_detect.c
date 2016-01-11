#include <string.h>
#include "htc_tamper_detect.h"
#include "htc_board_info_and_setting.h"

#define TAMPER_DET_DEBUG	0

#if TAMPER_DET_DEBUG
#define TD_MSG(s...) HLOGD("[TD]" s)
#else
#define TD_MSG(s...) do {}while(0)
#endif

#define TAMPER_INIT_MAGIC           0xD7233658

#define TAMPER_ERR_NO_MEM           -1
#define TAMPER_ERR_RW_FAIL          -2
#define TAMPER_ERR_MGC_FAIL         -3

#define TAMPER_UPT_IMG_BOOT         0x1
#define TAMPER_UPT_IMG_RECOVERY     0x2
#define TAMPER_UPT_IMG_SYSTEM       0x4
#define TAMPER_UPT_IMG_HOSD         0x8

unsigned int td_update_img = 0;

char g_tamper_string[36];

struct tamper_info_t
{
	uint32_t tamper_magic;
	uint32_t tamper_flag;
	uint8_t  reserved[TAMLPER_ENCRYPT_UINT - 8];
};

/*
	read misc: setting_misc_partition_read("misc");   (done by setting_init)
	write misc: setting_save_misc();
*/
static int tamper_read_from_misc(uint32_t *tamper_flag)
{
	struct tamper_info_t *t_info;

	/* Memory allocate */
	t_info = malloc(sizeof(struct tamper_info_t));
	if (t_info == NULL) {
		TD_MSG("[ERR] %s: no mem\r\n", __FUNCTION__);
		return TAMPER_ERR_NO_MEM;
	}

	/* Get encrypted tamper_info from misc */
	setting_get_tamper_info((char *) t_info);

#if TAMPER_DET_DEBUG
{
	int i;
	char *buf = t_info;
	HLOGD("tamper_info:");
	for (i = 0; i < TAMLPER_ENCRYPT_UINT; i++) {
		if (!(i%16))
			HLOGD("\r\n");
		HLOGD("%X ", buf[i]);
	}
	HLOGD("\r\n");
}
#endif

	/* Check value valid */
	if (t_info->tamper_magic != TAMPER_INFO_MAGIC) {
		free(t_info);
		TD_MSG("%s: magic check fail\r\n", __FUNCTION__);
		return TAMPER_ERR_MGC_FAIL;
	}

	*tamper_flag = t_info->tamper_flag;

	free(t_info);
	return 0;
}

static int tamper_read_pgfs(char *tamper_info, int len)
{
	if (!len)
		return 0;

	return partition_read_pgfs("pg2fs_tamper", 0, tamper_info, len);
}

static int tamper_write_pgfs(char *tamper_info, int len)
{
	char *buf = NULL;
	int rlen;

	/* currently we only support max size 1024 */
	if (!len || len > 1024) return -1;

	/* pg2fs write unit is 1024 */
	buf = malloc(1024);
	if (buf == NULL)
		return -1;

	memset(buf, 0, 1024);
	memcpy(buf, tamper_info, len);

	rlen = partition_update_pgfs("pg2fs_tamper", 0, buf, 1024);

	free(buf);
	return (rlen > 0)?0:rlen;
}

/*
	read pg2fs: partition_read_pgfs("pg2fs_tamper", 0, security_info, sizeof(struct security_info_t))
		radio:
	write pg2fs: partition_update_pgfs("pg2fs_tamper", 0, xxx, sizeof(struct xxx_t));
		radio:
*/
static int tamper_read_secure_filesystem(int32_t *tamper_flag)
{
	struct tamper_info_t *t_info;
	int ret;

	/* Memory allocate */
	t_info = malloc(sizeof(struct tamper_info_t));
	if (t_info == NULL) {
		TD_MSG("[ERR] %s: no mem\r\n", __FUNCTION__);
		return TAMPER_ERR_NO_MEM;
	}

	ret = tamper_read_pgfs((char *)t_info, sizeof(struct tamper_info_t ));
	if (!ret) {
		if (t_info->tamper_magic != TAMPER_INFO_MAGIC) {
			TD_MSG("%s: Tamper-Detect do init\r\n", __FUNCTION__);
			tamper_reset_flag();
			t_info->tamper_flag = 0;
		}
	} else {
		TD_MSG("[ERR] %s: read fail\r\n", __FUNCTION__);
		*tamper_flag = 0;
		free(t_info);
		return TAMPER_ERR_RW_FAIL;
	}

	*tamper_flag = t_info->tamper_flag;

	free(t_info);
	return ret;
}

static int tamper_write_secure_filesystem(uint32_t tamper_flag)
{
	struct tamper_info_t *t_info;
	int ret;

	/* Memory allocate */
	t_info = malloc(sizeof(struct tamper_info_t));
	if (t_info == NULL) {
		TD_MSG("[ERR] %s: no mem\r\n", __FUNCTION__);
		return TAMPER_ERR_NO_MEM;
	}

	memset(t_info, 0, sizeof(struct tamper_info_t));
	t_info->tamper_magic = TAMPER_INFO_MAGIC;
	t_info->tamper_flag = tamper_flag;

	ret = tamper_write_pgfs((char *)t_info, sizeof(struct tamper_info_t ));
	if (ret) {
		TD_MSG("[ERR] %s: radio r/w fail (maybe no mem)\r\n", __FUNCTION__);
		free(t_info);
		return TAMPER_ERR_RW_FAIL;
	} else {
		memset(t_info, 0, sizeof(struct tamper_info_t));
		tamper_read_pgfs((char *)t_info, sizeof(struct tamper_info_t ));

		if (t_info->tamper_flag == tamper_flag && t_info->tamper_magic == TAMPER_INFO_MAGIC)
			TD_MSG("Write to radio successfully\r\n");
		else
			TD_MSG("write to radio fail...\r\n");
	}

	free(t_info);

#if defined(REBOOT_TAMPER_DEVICE)
	/* Need to reboot device for radio master platform */
	/* Reset to make things happen */
	TD_MSG("Reset!!!\r\n");
	reboot_device(0);
#endif

	return ret;
}

int tamper_set_flag(uint32_t tamper_flag)
{
	uint32_t tamper_type;
	int ret;

	tamper_type = tamper_flag & TAMPER_TYPE_MASK;
	if (tamper_type)
		TD_MSG("%s: Tampered! type: 0x%X\r\n", __FUNCTION__, tamper_type);
	else
		TD_MSG("%s: Reset Tampered!\r\n", __FUNCTION__);

	ret = tamper_write_secure_filesystem(tamper_flag);
	if (ret) {
		TD_MSG("[ERR] %s: radio r/w fail (maybe no mem)\r\n", __FUNCTION__);
		return TAMPER_ERR_RW_FAIL;
	}

	tamper_set_display_string(tamper_type);

	return 0;
}

int tamper_reset_flag(void)
{
	return tamper_set_flag(0);
}

int tamper_get_flag(uint32_t *tamper_flag)
{
	int ret;

	ret = tamper_read_secure_filesystem(tamper_flag);
	if (ret) {
		TD_MSG("[ERR] %s: radio r/w fail (maybe no mem)\r\n", __FUNCTION__);
		return TAMPER_ERR_RW_FAIL;
	}

	TD_MSG("%s: Get tamper flag: 0x%X\r\n", __FUNCTION__, *tamper_flag);

	return 0;
}

int tamper_sync_from_misc(void)
{
	uint32_t tamper_flag, orig_tamper, check_flag;
	int ret, i, cnt;

	/* Read from misc */
	ret = tamper_read_from_misc(&tamper_flag);
	/* Check invalid tamper flag (modified by hacker?!) */
	if (ret == TAMPER_ERR_MGC_FAIL)
		return tamper_set_flag(TAMPER_TYPE_VALUE_INVALID);

	if (!ret) {
		/* Check invalid tamper flag (modified by hacker?!) */
		if (!tamper_flag || (tamper_flag&(~TAMPER_TYPE_VALID_MASK)))
			return tamper_set_flag(TAMPER_TYPE_VALUE_INVALID);

		cnt = 0; check_flag = tamper_flag & TAMPER_TYPE_VALID_MASK;
		for ( i = 0; i < 32; i++) {
			if(check_flag & 0x1)
				cnt++;
			check_flag >>= 1;
		}
		if (cnt > 1)
			return tamper_set_flag(TAMPER_TYPE_VALUE_INVALID);

		/* for trace flag change */
		ret = tamper_read_secure_filesystem(&orig_tamper);
		if (!ret && (orig_tamper != tamper_flag))
			TD_MSG("%s: Tampered status change! from 0x%X to 0x%X\r\n", __FUNCTION__, orig_tamper, tamper_flag);

		/* Write to radio/pg2fs */
		ret = tamper_write_secure_filesystem(tamper_flag);
		if (ret) {
			TD_MSG("[ERR] %s: radio r/w fail (maybe no mem)\r\n", __FUNCTION__);
			return TAMPER_ERR_RW_FAIL;
		}
	} else {
		TD_MSG("[ERR] %s: misc r/w fail (maybe no mem)\r\n", __FUNCTION__);
		return TAMPER_ERR_RW_FAIL;
	}

	return 0;
}

void tamper_rec_update_image(const char *image_name)
{

	TD_MSG("%s: updated images: %s...\r\n", __FUNCTION__, image_name);

	if (!strncmp(image_name, "boot", strlen("boot")))
		td_update_img |= TAMPER_UPT_IMG_BOOT;
	else if (!strncmp(image_name, "recovery", strlen("recovery")))
		td_update_img |= TAMPER_UPT_IMG_RECOVERY;
	else if (!strncmp(image_name, "system", strlen("system")))
		td_update_img |= TAMPER_UPT_IMG_SYSTEM;
	else if (!strncmp(image_name, "hosd", strlen("hosd")))
		td_update_img |= TAMPER_UPT_IMG_HOSD;

}

void tamper_chk_update_image(void)
{
	uint32_t tamper_flag;

	TD_MSG("%s: total updated images: 0x%X...\r\n", __FUNCTION__, td_update_img);

	tamper_get_flag(&tamper_flag);
	if (!setting_security() || !get_unlock_status()) {
		/* When S-OFF or S-ON with Locked bootloader, means we are updating authentic images */
		/* only after updating all boot & recovery & system images, we can reset the tampered flag */
		if (tamper_flag && (td_update_img ==
			(TAMPER_UPT_IMG_BOOT | TAMPER_UPT_IMG_RECOVERY |
			TAMPER_UPT_IMG_SYSTEM | TAMPER_UPT_IMG_HOSD))) {
#if defined(REBOOT_TAMPER_DEVICE)
			/* Need to reboot device for radio master platform */
			htc_setting_set_bootmode(BOOTMODE_FASTBOOT);
#endif
			tamper_reset_flag();
		}
	} else {
		/* if unlock bootloader & trying to update system image, set tampered flag */
		if (!(tamper_flag&TAMPER_TYPE_UNLOCK_UPDATE_SYS) && (td_update_img&TAMPER_UPT_IMG_SYSTEM))
			tamper_set_flag(TAMPER_TYPE_UNLOCK_UPDATE_SYS);
	}

}

/*---------------------Tamper information for tpd daemon---------------------------*/

int tamper_get_status(void)
{
	uint32_t tamper_flag;
	int ret;

	ret = tamper_get_flag(&tamper_flag);

	tamper_set_display_string(tamper_flag);

	if (!ret && tamper_flag)
		return 1;
	else
		return 0;
}

long tamper_get_partition_offset(void)
{
	return setting_get_tamper_info_offset();
}

int tamper_get_periof(void)
{
	return 1;
}

int tamper_get_delay(void)
{
	return 0;
}

int tamper_get_timeout(void)
{
	return 300;
}

void tamper_set_display_string(uint32_t status)
{
	memset(g_tamper_string, 0, sizeof(g_tamper_string));

	if (status)
		sprintf(g_tamper_string, "*** Software status: Modified ***");
	else
		sprintf(g_tamper_string, "*** Software status: Official ***");
}
