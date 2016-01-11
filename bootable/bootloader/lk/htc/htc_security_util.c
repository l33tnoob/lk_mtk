#include "htc_security_util.h"
#include "htc_pg_fs.h"
#include "boot_mode.h"
#include <oemkey.h>
#include <htc_crypto.h>
#include <htc_rsa.h>

/* the public key of pg2fs_ship_signkey image signature */
/* e=65537, 2048 bit */
const RSAPublicKey g_pgfs_sigkey_pub_key = {
	64,0x11afc2d3,
	{530193061,170374451,196069786,4260324067,2103740441,1961839979,
	1482077138,2941319886,1787275690,979411763,2602235834,3989302958,
	2449927932,3880034348,1004292489,2450170707,2921898310,2261607963,
	4226736474,1158712228,2370254073,1903292375,1629010489,1268779741,
	2222909444,3616528793,2867387929,2676348128,186303783,2318910760,
	2782584322,3647295436,1013282526,1544977472,3496482828,841616833,
	3336298325,692205059,98668404,1118904724,4209057827,1536312502,
	2065658156,2593758357,3496716868,2431527950,2311036470,2337096443,
	1164126437,1028850090,1099054047,4190953748,2874174609,3855659208,
	1063513066,3449038283,3942676449,2292140779,638628229,2199696937,
	1439857558,1139869674,3459049269,3296446634},
	{3420605967,3256800587,2992936665,1581863832,397492846,2860395114,
	1046466450,3167287524,3965373746,4240272477,2790757331,497168984,
	803533479,1306918261,3283996120,3383640372,1607612336,3267754566,
	3975112308,2979883695,765467469,1671187591,635894871,1870156295,
	3259763705,2401484139,2507044363,1208319974,4142586196,748760662,
	167225454,321808592,372579764,1835503354,372588352,2313209926,
	238557897,3191769255,1178755993,814752361,3838018288,3302983504,
	3201444630,1853417132,3502260849,2635937167,1160777995,3166392225,
	3101434597,1144865121,751663665,392690777,2447713288,3961532544,
	2912378464,2342596971,3725603493,353052923,1769009219,374955877,
	1785297294,2631692478,1236589234,650896263},
	65537
};

const u8 g_mteekey[MTEE_IMG_VFY_PUBK_SZ] = {MTEE_IMG_VFY_PUBK};

static RSAPublicKey g_ship_sign_key = {0};
int Boot_Security_Check_Fail = 0;
const char *SEC_BOOT_WARNING = "*** Security Warning ***";

static unsigned char* verify_partitions_list[] = {"secro", "tee1", "tee2", "lk", "hosd"};


extern u8 g_oemkey[OEM_PUBK_SZ];
extern BOOT_ARGUMENT *g_boot_arg;

extern void seclib_image_buf_init(void);
extern void seclib_image_buf_free(void);
extern u32 seclib_image_check_buf(u8 *buf, u32 size);

struct security_info_t* security_info = NULL;
struct sec_ex_util_info_t* sec_ex_util_info = NULL;
char g_unlock_string[36] = {0};
extern int unlock_status;

int htc_sec_dl_permission_chk(const char *part_name, unsigned int *permitted)
{
	*permitted = 0; // forbidden image download by default

	if (setting_security()) // if S-ON
	{
		if (strcmp(part_name, "hosd") == 0)
		{
			*permitted = 1;
		}
		else
		{
		}
	}
	else
	{
		*permitted = 1; // always can download image while S-OFF
	}
	return 0;
}
int htc_sec_format_permission_chk(const char *part_name, unsigned int *permitted)
{
	if (setting_security()) // if S-ON
	{
		*permitted = 0; // forbidden image download by default
	}
	else
	{
		*permitted = 1; // always can format partition while S-OFF
	}
	return 0;
}


void htc_rid_scramble(unsigned char* rid)
{
	int i = 0;
	unsigned char rid_internal[CHIP_RID_LEN*2] = {0};
	memcpy(rid_internal, rid, CHIP_RID_LEN);
	for (i = 0; i < (CHIP_RID_LEN*2); i++)
	{
		rid[i] = (i+((rid_internal[((CHIP_RID_LEN*2)-1)-i] | 0x7 ) | (rid_internal[i] << 3))) & 0xFF;
	}
}

int htc_rid_encrypt(unsigned char* buf, int len, int enc)
{
	uint32_t rid[(CHIP_RID_LEN*2) / sizeof(uint32_t)] = {0};
	uint32_t iv[AES_IV_SIZE / sizeof(uint32_t)] = {0};

	*(iv+0x0) = 0x3BFF1A56;
	*(iv+0x1) = 0x92A036C5;
	*(iv+0x2) = 0x567AB3D9;
	*(iv+0x3) = 0xDB3899C4;

	if(! buf)
	{
		HLOGD("Buffer is empty...\r\n");
		return -1;
	}
	if (len % AES_BLOCK_SIZE)
	{
		HLOGD("%s: buffer length must be multiple of 16...\r\n", __func__);
		return -2;
	}
	if(seclib_get_random_id((unsigned char*)rid,CHIP_RID_LEN))
	{
		HLOGD("Can't get RID...\r\n");
		return -3;
	}

	htc_rid_scramble((unsigned char*)rid);

	if (enc == AES_ENCRYPT)
	{
		encrypt_custom(buf,len,(unsigned char*)iv,AES_IV_SIZE,(unsigned char*)rid,CHIP_RID_LEN*2);
	}
	else // decrypt
	{
		decrypt_custom(buf,len,(unsigned char*)iv,AES_IV_SIZE,(unsigned char*)rid,CHIP_RID_LEN*2);
	}
	return 0;
}


int htc_get_security_info(struct security_info_t * sec_info, int size)
{
	unsigned char* tmp = NULL;
	if (!sec_info || size != sizeof(struct security_info_t))
	{
		HLOGE("Invalid argument\r\n");
		return -1;
	}

	tmp = (unsigned char*)alloc_page_aligned(size);
	if (tmp == NULL)
	{
		HLOGE("%s: alloc buf failed\r\n", __FUNCTION__);
		return -2;
	}
	memset(tmp, 0, sizeof(struct security_info_t));
	// read cipher data to temp buffer from "pg2fs_sec_recovery"
	partition_read_pgfs("pg2fs_sec_recovery", 0, tmp, sizeof(struct security_info_t));
	// decrypt cipher data by by RID
	if (htc_rid_encrypt(tmp, sizeof(struct security_info_t), AES_DECRYPT))
	{
		HLOGE("%s: htc_rid_encrypt dec failed\r\n", __FUNCTION__);
		return -3;
	}
	// copy plain data to input buffer
	memcpy(sec_info, tmp, sizeof(struct security_info_t));

	memset(tmp, 0, sizeof(struct security_info_t));
	// read plain data to temp buffer from "pg1fs_security"
	partition_read_pgfs("pg1fs_security", 0, tmp, sizeof(struct security_info_t));
	if (memcmp(sec_info, tmp, sizeof(struct security_info_t)) != 0) // not equal
	{
		HLOGE("%s: pg1fs_security != pg2fs_sec_recovery\r\n", __FUNCTION__);

		memcpy(sec_info, tmp, sizeof(struct security_info_t));
		#ifdef MFG_BUILD // MFG build, HTC_BUILD_MODE=MFG_BUILD
			sec_info->security_level = SECLEVEL_MFG;
		#elif defined (SHIP_BUILD) // SHIP build, HTC_BUILD_MODE=SHIP_BUILD
			sec_info->security_level = SECLEVEL_USER;
		#else // ENG build, HTC_BUILD_MODE=
			sec_info->security_level = SECLEVEL_USER;
		#endif
		htc_set_security_info(sec_info, sizeof(struct security_info_t));
	}

	free(tmp);
	return 0;
}

int htc_set_security_info(struct security_info_t * sec_info, int size)
{
	unsigned char* tmp = NULL;
	if (!sec_info || size != sizeof(struct security_info_t))
	{
		HLOGE("Invalid argument\r\n");
		return -1;
	}

	tmp = (unsigned char*)alloc_page_aligned(size);
	if (tmp == NULL)
	{
		HLOGE("%s: alloc buf failed\r\n", __FUNCTION__);
		return -2;
	}
	memcpy(tmp, sec_info, sizeof(struct security_info_t));
	// encrypt plain data by by RID
	if (htc_rid_encrypt(tmp, sizeof(struct security_info_t), AES_ENCRYPT))
	{
		HLOGE("%s: htc_rid_encrypt enc failed\r\n", __FUNCTION__);
		return -3;
	}
	// plain data must store into "pg1fs_security"
	partition_update_pgfs("pg1fs_security", 0, sec_info, sizeof(struct security_info_t));
	// cipher data must store into "pg2fs_sec_recovery"
	partition_update_pgfs("pg2fs_sec_recovery", 0, tmp, sizeof(struct security_info_t));

	free(tmp);
	return 0;
}


void htc_security_init(void)
{
	//get security level from pg1fs security partition
	security_info = (struct security_info_t *)alloc_cache_page_aligned(sizeof(struct security_info_t));
	if(!security_info)
		goto err;

	memset(security_info, 0, sizeof(struct security_info_t));
	if (htc_get_security_info(security_info, sizeof(struct security_info_t)))
	{
		HLOGD("%s: htc_get_security_info fail\r\n", __func__);
		goto err;
	}

	//get security extra utility from pg2fs sec_ex_util partition
	sec_ex_util_info = (struct sec_ex_util_info_t *)alloc_cache_page_aligned(sizeof(struct sec_ex_util_info_t));
	if(!sec_ex_util_info)
		goto err;

	memset(sec_ex_util_info, 0, sizeof(struct sec_ex_util_info_t));

	set_unlock_display_string(security_info->unlock);

	if ( security_info->unlock== 0)
		unlock_status = 0;//LOCKED
	else if (security_info->unlock == HTC_UNLOCK_MAGIC)
		unlock_status = 1;//UNLOCKED
	else if (security_info->unlock == HTC_RELOCK_MAGIC)
		unlock_status = 1;//RELOCKED
	else
		unlock_status = 0;//UNKNOWN

	return;

err:
	HLOGD("%s: fail to read security info\r\n", __func__);
}

int verifySecureBootPartitions(void)
{
	int i = 0;
	for (i = 0; i < sizeof(verify_partitions_list)/sizeof(unsigned char*); i++)
	{
		if (htc_verify_image(verify_partitions_list[i]) != 0)
		{
			HLOGE("Verifying %s failed!\r\n", verify_partitions_list[i]);
			return -1;
		}
		else
		{
			HLOGI("Verifying %s passed!\r\n", verify_partitions_list[i]);
		}
	}
	return 0;
}

int write_security_level(int level)
{
	if (level >= SECLEVEL_MFG && level < SECLEVEL_MAX)
	{
		if (!setting_security() && level >= SECLEVEL_DEVELOPER)
		{
			if (verifySecureBootPartitions())
			{
				HLOGE("writesecureflag: partitions siganture failed\r\n");
				return -1;
			}
			HLOGI("writesecureflag: partitions siganture pass\r\n");
		}

		if (setting_security())
		{
			if (!setting_smart_card()) // if no smart card magic
			{
				HLOGE("writesecureflag: no smart card, permission denied!\r\n");
				return -2;
			}
			else
			{
				HLOGI("writesecureflag: with smart card, permission granted!\r\n");
			}
		}

		if (security_info)
		{
			security_info->security_level = level;
			return htc_set_security_info(security_info, sizeof(struct security_info_t));
		}
		else
		{
			HLOGE("writesecureflag: security_info = NULL!!\r\n");
			return -3;
		}
	}
	else
	{
		HLOGE("writesecureflag: invalid security level!\r\n");
		return -4;
	}
}

int read_security_level(void)
{
	if (security_info)
		return security_info->security_level;

	return -1;

}

int setting_security(void)
{
	if (!security_info)
		return 1;

	if (security_info->security_level > SECLEVEL_ENGINEER)
		return 1;
	else
		return 0;
}

int write_sec_ex_magic(int magic_type, int magic_num)
{
	if (magic_type <= MAGIC_TYPE_RESERVED_1 || magic_type >= MAGIC_TYPE_MAX)
	{
		HLOGE("%s: Invalid magic_type %d!!(%d)\n", __FUNCTION__, magic_type);
		return -1;
	}
	if (sec_ex_util_info)
	{
		partition_read_pgfs("pg2fs_sec_ex_util", 0, sec_ex_util_info, sizeof(struct sec_ex_util_info_t));
		// Should decrypt the "sec_ex_util_info" by RID first
		if (htc_rid_encrypt(sec_ex_util_info, sizeof(struct sec_ex_util_info_t), AES_DECRYPT))
		{
			HLOGE("%s: htc_rid_encrypt dec failed\r\n", __FUNCTION__);
			return -1;
		}
		 ((unsigned int*)sec_ex_util_info)[magic_type] = magic_num;
		// Should encrypt the "sec_ex_util_info" by RID here...
		if (htc_rid_encrypt(sec_ex_util_info, sizeof(struct sec_ex_util_info_t), AES_ENCRYPT))
		{
			HLOGE("%s: htc_rid_encrypt enc failed\r\n", __FUNCTION__);
			return -1;
		}
		partition_update_pgfs("pg2fs_sec_ex_util", 0, sec_ex_util_info, sizeof(struct sec_ex_util_info_t));
		return 0;
	}
	return -1;
}

int read_sec_ex_magic(int magic_type)
{
	if (magic_type <= MAGIC_TYPE_RESERVED_1 || magic_type >= MAGIC_TYPE_MAX)
	{
		HLOGE("%s: Invalid magic_type %d!!(%d)\n", __FUNCTION__, magic_type);
		return -1;
	}
	if (sec_ex_util_info)
	{
		partition_read_pgfs("pg2fs_sec_ex_util", 0, sec_ex_util_info, sizeof(struct sec_ex_util_info_t));
		// Should decrypt the "sec_ex_util_info" by RID here...
		if (htc_rid_encrypt(sec_ex_util_info, sizeof(struct sec_ex_util_info_t), AES_DECRYPT))
		{
			HLOGE("%s: htc_rid_encrypt dec failed\r\n", __FUNCTION__);
			return -1;
		}
		return ((unsigned int*)sec_ex_util_info)[magic_type];
	}
	return -1;
}

int setting_smart_card(void)
{
	if (!sec_ex_util_info)
	{
		return 0;
	}
	if (read_sec_ex_magic(MAGIC_TYPE_SMART_SD) == MAGIC_SMART_CARD_FOUND)
	{
		return 1;
	}
	return 0;
}

int setting_one_time_root(void)
{
	if (!sec_ex_util_info)
	{
		return 0;
	}
	if (read_sec_ex_magic(MAGIC_TYPE_ONE_TIME_ROOT) == MAGIC_ONE_TIME_ROOT)
	{
		return 1;
	}
	return 0;
}

/******** unlock bootloader ********/
int write_security_unlock(int unlock)
{
	if (security_info)
	{
		security_info->unlock = unlock;
		return htc_set_security_info(security_info, sizeof(struct security_info_t));
	}

	return -1;
}

int read_security_unlock(void)
{
	if (security_info)
		return security_info->unlock;

	return -1;

}

int get_unlock_status(void)
{
	int unlock;

	unlock = read_security_unlock();

	return (unlock == HTC_UNLOCK_MAGIC);
}

void set_unlock_display_string(int status)
{
	memset(g_unlock_string, 0, sizeof(g_unlock_string));

	if (status == 0)
		sprintf(g_unlock_string, "*** LOCKED ***");
	else if (status == HTC_UNLOCK_MAGIC)
		sprintf(g_unlock_string, "*** UNLOCKED ***");
	else if (status == HTC_RELOCK_MAGIC)
		sprintf(g_unlock_string, "*** RELOCKED ***");
	else
		sprintf(g_unlock_string, "*** UNKNOWN ***");
}

int sd_get_pg2fs_sign_key(RSAPublicKey *sign_key)
{
	signkey_header_mtk *header = NULL;
	unsigned p_count = 0;
	unsigned keycount = 0;
	unsigned key_id;
	unsigned char *key_image = NULL;
	RSAPublicKey *tmp_key;
	unsigned char *key_sig;
	unsigned char digest[DIGEST256_SIZE];
	int ret = 0, i;
	const char* pgfs_name = "pg2fs_ship_signkey";
	RSAPublicKey *pgfs_sigkey_pub_key = &g_pgfs_sigkey_pub_key;
	unsigned char *mtk_product_name = g_boot_arg->htc_sign_key_id;

	if (sign_key == NULL) {
		HLOGD("%s: sign_key memory is NULL\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	header = (signkey_header_mtk *)malloc(sizeof(signkey_header_mtk));
	if (!header) {
		HLOGD("%s: allocate memory of header failed\r\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	/* read signkey image header */
	ret = partition_read_pgfs(pgfs_name, 0, (void *)header, sizeof(signkey_header_mtk));
	if (ret < 0) {
		HLOGD("%s: read pg2fs partition failed\r\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	/* check header magic */
	if (strcmp(header->magic, SIGNKEY_MAGIC)) {
		HLOGD("%s: sign key header is not correct\r\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	p_count = header->p_count;

	keycount = header->keycount;

	/* show key image version */
	HLOGD("key image chipset: %s\n", header->chipset);
	HLOGD("key image version: %s\n", header->version);

	key_image = (unsigned char *)malloc(header->total_len + SIGNATURE_SIZE);
	if (!key_image) {
		HLOGD("%s: allocate memory of key_image failed\r\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	/* read signkey image data and signature */
	ret = partition_read_pgfs(pgfs_name, 0, (void *)key_image, header->total_len + SIGNATURE_SIZE);
	if (ret < 0) {
		HLOGD("%s: read pg2fs partition failed\r\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	/* verify signature */
	compute_digest_sha256(key_image, (unsigned)header->total_len, digest);

	key_sig = (unsigned char *)(key_image + header->total_len);

	ret = is_signature_okay_ext(digest, HASH_SHA256, key_sig, (void *)pgfs_sigkey_pub_key);
	if (!ret) {
		HLOGD("%s: sign key signature verify failed\r\n", __FUNCTION__);
		ret = -1;
		goto free_buff;
	}

	HLOGD("%s: check htc_sign_key_id: %s\r\n", __FUNCTION__, mtk_product_name);

	/* looking for correct key slot */
	for (i = 0; i < (int)p_count; i++) {
		HLOGD("%s: key table: %s\r\n", __FUNCTION__, header->key_table[i].product_name);

		if (!strcmp(header->key_table[i].product_name, mtk_product_name) && (key_id = header->key_table[i].key_id) <= keycount) {

			HLOGD("%s: meet sign key product name: %s, kid: %d\r\n", __FUNCTION__, mtk_product_name, key_id);

			tmp_key = (RSAPublicKey *)(key_image + sizeof(signkey_header_mtk) + (key_id - 1) * sizeof(RSAPublicKey));

			memcpy(sign_key, tmp_key, sizeof(RSAPublicKey));

			tmp_key = (RSAPublicKey *)sign_key;

			HLOGD("%s: key data is len: %d, n0inv: %x, n[0]: %u, rr[0]: %u, expo: %d\r\n",
				__FUNCTION__, tmp_key->len, tmp_key->n0inv, tmp_key->n[0], tmp_key->rr[0], tmp_key->exponent);

			ret = 0;
			goto free_buff;
		}
	}

	ret = -1;
	HLOGD("%s: did not find out correct sign key\r\n", __FUNCTION__);

free_buff:
	if (header)
		free(header);
	if (key_image)
		free(key_image);
	return ret;
}

int htc_convert_RSAPublicKey(RSAPublicKey* sign_key, unsigned char* oem_key, unsigned int oem_key_buf_sz)
{
	// usage:
	// 	unsigned char g_oemkey[OEM_PUBK_SZ] = {OEM_PUBK};;
	// 	htc_convert_RSAPublicKey(&g_ship_sign_key, g_oemkey, 256 or OEM_PUBK_SZ)
	int i = 0, index = 0;
	uint32_t tmp_value = 0;
	if (sign_key == NULL || oem_key == NULL)
	{
		HLOGD("%s: rsa_pubkey or oem_key was NULL, error...\r\n", __FUNCTION__);
		return -1;
	}
	if (oem_key_buf_sz != RSANUMBYTES)
	{
		HLOGD("%s: oem key buffer size %d error, should be %d...\r\n", __FUNCTION__, oem_key_buf_sz, RSANUMBYTES);
		return -2;
	}

	if (sign_key->exponent != 0x10001) // should be 65537
	{
		HLOGD("%s: rsa key exponent(%d) should be 65537, error...\r\n", __FUNCTION__, sign_key->exponent);
		return -3;
	}

	memset(oem_key, 0, oem_key_buf_sz);
	for (i = sign_key->len-1; i >= 0; i--)
	{
		tmp_value = sign_key->n[i];
		index = (RSANUMWORDS-i-1)*sizeof(uint32_t);
		oem_key[index] = (unsigned char)((tmp_value & 0xFF000000) >> 24);
		oem_key[index+1] = (unsigned char)((tmp_value & 0x00FF0000) >> 16);
		oem_key[index+2] = (unsigned char)((tmp_value & 0x0000FF00) >> 8);
		oem_key[index+3] = (unsigned char)((tmp_value & 0x000000FF) );
	}

	HLOGD("%s:oem key: [0]%x, [1]%x, [2]%x [%d]%x [%d]%x [%d]%x\r\n",
					__FUNCTION__, oem_key[0], oem_key[1], oem_key[2],
					RSANUMBYTES-3, oem_key[RSANUMBYTES-3],
					RSANUMBYTES-2, oem_key[RSANUMBYTES-2],
					RSANUMBYTES-1, oem_key[RSANUMBYTES-1]
					);
	return 0;
}

int setOEMkeyfromPGFS()
{
	int ret = 0;

	/* check if didn't load key before */
	if (g_ship_sign_key.len == 0) {

		ret = sd_get_pg2fs_sign_key(&g_ship_sign_key);

		if (ret) {
			HLOGD("%s: get sign key failed\r\n", __FUNCTION__);
			return -1;
		}
	}

	HLOGD("%s: get RSA public key: n0inv: %x, n[0]: %u, rr[0]: %u, expo: %d\r\n",
		__FUNCTION__, g_ship_sign_key.n0inv, g_ship_sign_key.n[0], g_ship_sign_key.rr[0], g_ship_sign_key.exponent);

	/* convert to MTK key format */
	ret = htc_convert_RSAPublicKey(&g_ship_sign_key, g_oemkey, OEM_PUBK_SZ);
	if (ret) {
		HLOGD("%s: convert RSA public key failed\r\n", __FUNCTION__);
		return -1;
	}

	/* set MTK oem key */
	ret = seclib_set_oemkey(g_oemkey, OEM_PUBK_SZ);
	if (ret) {
		HLOGD("%s: seclib_set_oemkey failed %d\r\n",__FUNCTION__, ret);
		return -1;
	}

	return 0;
}

int htc_verify_image(unsigned char* p_img_name)
{
	int rc = 0;
#ifdef MTK_SECURITY_SW_SUPPORT
	int img_size = 0;
	int atf_img_size = 0, tee_img_size = 0;
	part_hdr_t part_hdr = {0};
	unsigned char* buffer = NULL;

	HLOGD("%s: verifying %s image\r\n...", __func__, p_img_name);
	if (strncmp(p_img_name, TEE_IMG_NAME_PREFIX, strlen(TEE_IMG_NAME_PREFIX)) == 0)
	{
		rc = seclib_set_oemkey(g_mteekey, MTEE_IMG_VFY_PUBK_SZ);
	}
	else
	{
		rc = setOEMkeyfromPGFS();
	}

	if (rc)
	{
		HLOGD("seclib_set_oemkey failed, rc = %d!\r\n", rc);
		return -1;
	}
	if (strcmp(p_img_name, BOOT_IMG_NAME) == 0 ||
		strcmp(p_img_name, HOSD_IMG_NAME) == 0 ||
		strcmp(p_img_name, RECOVERY_IMG_NAME) == 0 )
	{
		// boot/hosd/recovery image
		seclib_image_buf_init();
		rc = seclib_image_check((U8*)p_img_name, TRUE);
		seclib_image_buf_free();
	}
	else
	{
		if (strcmp(p_img_name, SECRO_IMG_NAME) == 0)
		{
			img_size = SECRO_IMG_SIZE;
		}
		else if (strcmp(p_img_name, LK_IMG_NAME) == 0 ) // lk
		{
			partition_read(p_img_name, 0, &part_hdr, sizeof(part_hdr));
			img_size = MTK_IMG_ALIGNMENT*(1 + (sizeof(part_hdr)+part_hdr.info.dsize-1)/MTK_IMG_ALIGNMENT) + SIGNATURE_SIZE;
		}
		else if (strncmp(p_img_name, TEE_IMG_NAME_PREFIX, strlen(TEE_IMG_NAME_PREFIX)) == 0)
		{
			// parse ATF header
			partition_read(p_img_name, 0, &part_hdr, sizeof(part_hdr));
			atf_img_size = sizeof(part_hdr)+part_hdr.info.dsize;
			// parse TEE header
			partition_read(p_img_name, atf_img_size, &part_hdr, sizeof(part_hdr));
			tee_img_size = sizeof(part_hdr)+part_hdr.info.dsize;
			img_size = atf_img_size + tee_img_size;
			HLOGI("%s image: atf size = %d, tee size = %d\r\n", p_img_name, atf_img_size, tee_img_size);
		}
		else
		{
			HLOGE("Invalid image %s, error...\r\n", p_img_name);
			return -2;
		}
		HLOGI("%s image size = %d\r\n", p_img_name, img_size);
		buffer = (unsigned char*)alloc_page_aligned(img_size);
		if (buffer == NULL)
		{
			HLOGE("%d byte buffer allocation failed!\r\n", img_size);
			return -3;
		}
		partition_read(p_img_name, 0, buffer, img_size);
		rc = seclib_image_check_buf((u8 *)buffer, (u32)img_size);

		if (rc)
		{
			HLOGE("Image %s verification failed, rc = %d!\r\n", p_img_name, rc);
		}
		else
		{
			HLOGI("Image %s verification passed!\r\n", p_img_name);
		}
		if (buffer != NULL)
		{
			free(buffer);
		}
	}
#else
	HLOGD("MTK_SECURITY_SW_SUPPORT NOT enabled, bypass image verification!\r\n");
#endif
	return rc;
}

int htc_sec_boot_check (void)
{
	unsigned char* p_img_name = NULL;

	switch (g_boot_mode)
	{
	case NORMAL_BOOT:
	case META_BOOT:
	case ADVMETA_BOOT:
	case SW_REBOOT:
	case ALARM_BOOT:
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	case KERNEL_POWER_OFF_CHARGING_BOOT:
	case LOW_POWER_OFF_CHARGING_BOOT:
#endif
	case FACTORY_BOOT:
	case ATE_FACTORY_BOOT:
		// boot.img
		p_img_name = "boot";
		break;

	case RECOVERY_BOOT:
		// recovery.img
		p_img_name = "recovery";
		break;

	case HTC_DOWNLOAD:
	case HTC_DOWNLOAD_RUU:
		// hosd.img
		p_img_name = "hosd";
		break;

	case FASTBOOT:
	case DOWNLOAD_BOOT:
	case UNKNOWN_BOOT:
		break;
	}

	if (p_img_name)
	{
		if (htc_verify_image(p_img_name) != 0)
		{
			HLOGE("<ASSERT> %s: verifying %s image failed!\r\n", __func__, p_img_name);
			return -1;

		}
		else
		{
			HLOGI("Verifying %s passed!\r\n", p_img_name);
		}
	}

	return 0;
}

int boot_get_Security_Check_Fail_flag()
{
	return Boot_Security_Check_Fail;
}
