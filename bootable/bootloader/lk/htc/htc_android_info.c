#include <stdio.h>
#include <debug.h>
#include <htc_android_info.h>

static const char MID_title[] = "modelid:";
static const char CID_title[] = "cidnum:";
static const char MainVer_title[] = "mainver:";
static const char PreferDiag_title[] = "preferdiag:";
static const char DelUserData_title[] = "deluserdata:";
static const char DelCache_title[] = "delcache:";
static const char DelDevlog_title[] = "deldevlog:";
static const char DelFat_title[] = "delfat:";
static const char HbootPreUpdate_title[] = "hbootpreupdate:";
static const char EraseConfig_title[] = "eraseconfig:";
static const char EnableRadio8_title[] = "enableradio8:";
static const char BuildType_title[] = "btype:";
static const char AAReport_title[] = "aareport:";

#define DEBUG_LOG(s...) dprintf(CRITICAL, "[android info] "s)

static inline void strtolower(unsigned char* s)
{
	if (!s)
		return;
	for (; *s; ++s)
		*s = tolower(*s);
}

#if 0
static struct htc_android_info = {
	.del_userdata_flag = 2,
	.del_cache_flag = 0,
	.hboot_preupdate_flag = 0,
	.del_devlog_flag = 0,
	.del_fat_flag = 0,
	.erase_config_flag = 0,
	.enable_radio8_flag = 0,
};
#endif

char *strstrip(char *s)
{
	char *end;

	if (!(*s))
		return s;

	while (*s && isspace(*s))
		++s;

	end = s + strlen(s) - 1;
	while (end >= s && isspace(*end))
		--end;

	*(end + 1) = '\0';

	return s;
}

static int get_android_info_tag(char *android_info_buf, char *tag_buf, int tag_buf_len, int tag_buf_max_def, char *tag_title)
{
	char *tmp_s = NULL, *tmp_e = NULL, *tmp_e2 = NULL, *org_tmp_buf = NULL, *tmp_buf = NULL;
	char *strip_ret, *strip_buf;
	int tag_len, count = 0;

	if ((org_tmp_buf = (char *)malloc(strlen(android_info_buf) + 1)) == NULL)
		return -1;

	if ((strip_buf = (char *)malloc(tag_buf_len)) == NULL)
		return -1;

	tmp_buf = org_tmp_buf;
	memcpy(tmp_buf, android_info_buf, (strlen(android_info_buf) + 1));
	strtolower(tmp_buf);

	memset(tag_buf, 0, tag_buf_len*tag_buf_max_def);

	while ((count < tag_buf_max_def) && (tmp_buf < (org_tmp_buf + strlen(android_info_buf)))) {
		// parsing specific tag_title
		if ((tmp_s = strstr(tmp_buf, tag_title)) == NULL) {
			free(strip_buf);
			free(org_tmp_buf);
			if (count == 0)
				return -1;
			else
				return 0;
		}

		tmp_e = strchr(tmp_s, '\n');	// '\n' => LF
		tmp_e2 = strchr(tmp_s, '\r');	// '\r' => CR

		if ((tmp_e != NULL) && (tmp_e2 != NULL)) {
			if (tmp_e2 < tmp_e)
				tmp_e = tmp_e2;
		} else if (tmp_e2 != NULL) {
			tmp_e = tmp_e2;
		}

		if (tmp_e == NULL) {
			tag_len = strlen(tmp_s) - strlen(tag_title);
			tmp_buf = org_tmp_buf + strlen(android_info_buf) + 1;
		} else {
			tag_len = tmp_e - tmp_s - strlen(tag_title);
			tmp_buf = tmp_e + 1;
		}

		if ((tag_len <= 0) || (tag_len >= tag_buf_len)) {
			free(strip_buf);
			free(org_tmp_buf);
			return -1;
		}

		memset(strip_buf, 0, tag_buf_len);
		memcpy(strip_buf, android_info_buf + (tmp_s - org_tmp_buf) + strlen(tag_title), tag_len);

		strip_ret = strstrip(strip_buf);
		if (strip_ret > strip_buf)
			memcpy(strip_buf, strip_ret, strlen(strip_ret) + 1);

		if (strlen(strip_buf) <= 0) {
			free(strip_buf);
			free(org_tmp_buf);
			return -1;
		}

		memcpy(tag_buf+(count*tag_buf_len), strip_buf, strlen(strip_buf));
		count++;

		DEBUG_LOG("parsing_android_info: %s%s\r\n", tag_title, strip_buf);
	}

	free(strip_buf);
	free(org_tmp_buf);

	return 0;
}

int parsing_android_info(char *android_info_buf, struct htc_android_info_struct *htc_android_info)
{
	int ret;

	if (htc_android_info != NULL) {
		memset( htc_android_info, 0, sizeof(struct htc_android_info_struct));
		htc_android_info->del_userdata_flag = 2;
	} else {
		return -1;
	}

	// parsing image_mid
	if ((ret = get_android_info_tag(android_info_buf, htc_android_info->image_mid, MIDLeng, MAX_MID_DEF, MID_title)) != 0)
		return ret;

	// parsing image_mainver
	if ((ret = get_android_info_tag(android_info_buf, htc_android_info->image_mainver, MAIN_VERSION_LEN, 1, MainVer_title)) != 0)
		return ret;

	// parsing prefer_diag
	get_android_info_tag(android_info_buf, htc_android_info->prefer_diag, PREFER_DIAG_NAME_LEN, 1, PreferDiag_title);

	// for Ship mode: parsing image_cid
	if ((ret = get_android_info_tag(android_info_buf, htc_android_info->image_cid, CIDLeng, MAX_CID_DEF, CID_title)) != 0)
		return ret;

	char tmp_buf[8] = "";

	// parsing del_userdata_flag
	if ((ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, DelUserData_title)) == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("DelUserData: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 0)
				htc_android_info->del_userdata_flag = 2;	// 0: never del userdata
			else if (atoi(tmp_buf) == 1)
				htc_android_info->del_userdata_flag = 1;	// 1: force to del userdata
			else
				htc_android_info->del_userdata_flag = 0;	// (!0 && !1): delete original userdata if new image contains system image without userdata image
		}
	}

	// parsing del_cache_flag
	if ((ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, DelCache_title)) == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("DelCache: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->del_cache_flag = 1;		// 1: del cache
		}
	}

	/*parsing del_devlog_flag*/
	ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, DelDevlog_title);
	if (ret == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("DelDevlog: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->del_devlog_flag = 1;		/*1: del devlog*/
		}
	}

	/* parsing del_fat_flag */
	ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, DelFat_title);
	if (ret == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("DelFat: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->del_fat_flag = 1;		/* 1: del fat */
		}
	}

	// parsing hboot_preupdate_flag
	if ((ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, HbootPreUpdate_title)) == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("HbootPreUpdate: %s\r\n", tmp_buf);
			switch(atoi(tmp_buf)) {
				case 0:		// 0: force Hboot pre-update to be NOT functional
				case 1:		// 1: Hboot will be pre-updated while the Hboot version in zipped image is newer than the current one
				case 11:		// 11: [Radio will be pre-updated with Hboot] while the Hboot version in zipped image is newer than the current one
				case 2:		// 2: Hboot will be pre-updated while the Hboot version in zipped image is different from the current one
				case 12:		// 12: [Radio will be pre-updated with Hboot] while the Hboot version in zipped image is different from the current one
				case 3:		// 3: force Hboot pre-update to be functional
				case 13:		// 13: force Hboot and Radio to be pre-updated simultaneously to be functional
					htc_android_info->hboot_preupdate_flag = atoi(tmp_buf);
					break;
				default:
					break;
			}
		}
	}

	/* parsing erase_config_flag */
	ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, EraseConfig_title);
	if (ret == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("EraseConfig: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->erase_config_flag = 1;
		}
	}

	/* parsing enable_radio8_flag */
	ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, EnableRadio8_title);
	if (ret == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("EnableRadio8: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->enable_radio8_flag = 1;
		}
	}

	/* parsing build_type */
	htc_android_info->build_type = -1;
	ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, BuildType_title);
	if (ret == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("BuildType: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->build_type = 1;
			else if(atoi(tmp_buf) == 0)
				htc_android_info->build_type = 0;
		}
	}

	/* parsing aa_report */
	htc_android_info->aa_report = -1;
	ret = get_android_info_tag(android_info_buf, tmp_buf, 8, 1, AAReport_title);
	if (ret == 0) {
		if (strlen(tmp_buf) > 0) {
			DEBUG_LOG("AAReport: %s\r\n", tmp_buf);
			if (atoi(tmp_buf) == 1)
				htc_android_info->aa_report = 1;
			else if (atoi(tmp_buf) == 0)
				htc_android_info->aa_report = 0;
		}
	}

	return 0;
}
