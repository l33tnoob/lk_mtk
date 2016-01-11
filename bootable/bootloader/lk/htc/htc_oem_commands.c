#include <debug.h>
#include <reg.h>
#include <target.h>
#include <stdlib.h>
#include <../app/mt_boot/fastboot.h>
#include <platform/mmc_core.h>
#include <platform/partition.h>
#include <platform/mmc_wrapper.h>
#include <platform/mt_pmic.h>
#include <htc_board_info_and_setting.h>
#include <htc_reboot_info.h>
#include <htc_security_util.h>
#if _DDR_TEST
#include <htc_ddr_test.h>
#endif

#if TAMPER_DETECT
#include "htc_tamper_detect.h"
#endif


static char fastboot_response[256] = {0};
#define fastboot_printf(type, a...) {\
	snprintf(fastboot_response, sizeof(fastboot_response) - 1, a); \
	fastboot_##type(fastboot_response); \
}

struct skuid_t {
	char * name;
    unsigned id;
};

const struct skuid_t skuidDataTable[] = {
	{"FunctionSKUField", 0x00},
	{"PCBIDField",       0x01},
	{"RFSKUIDField_D0",  0x02},
	{"RFSKUIDField_D1",  0x03},
	{"RFSKUIDField_D2",  0x04},
	{"RFSKUIDField_D3",  0x05},
	{"RFSKUIDField_D4",  0x06},
	{"RFSKUIDField_D5",  0x07},
	{"RFSKUIDField_D6",  0x08},
	{"RFSKUIDField_D7",  0x09},
	{"SKUIDChecksum",    0x0A},
	{"EngineerID",       0x0B},
};

const unsigned long ulNumberOfSkuidData =
(sizeof(skuidDataTable) / sizeof(skuidDataTable[0]));

extern void htc_voprintf(const char *msg, ...);
void cmd_oem_help(const char *arg, void *data, unsigned sz) {
	extern struct fastboot_cmd *cmdlist;
	struct fastboot_cmd *cmd = cmdlist;
	while (cmd) {
		fastboot_printf( info, "%s", cmd->prefix);
		cmd = cmd->next;
	}
	fastboot_okay("");
}


void cmd_oem_readconfig(const char *arg, void *data, unsigned sz) {
	unsigned int id = -1;

	if (strlen(arg) == 0) {
		for(id=0; id <0xC; id++) {
			fastboot_printf( info, "index:0x%X, value:0x%X", id, htc_setting_cfgdata_get(id));
		}
	} else {
		id = -1;
		sscanf(arg, " %x", &id);
		if ( id >= 0 && id < 0xC ) {
			fastboot_printf( info, "index:0x%X, value:0x%X", id, htc_setting_cfgdata_get(id));
		} else {
			fastboot_fail("oem readconfig [id: 0...B]");
			return;
		}
	}
	fastboot_okay("");
}

void cmd_oem_writeconfig(const char *arg, void *data, unsigned sz) {
	unsigned int id = -1;
	unsigned int value = 0x0;

	if (strlen(arg) == 0) {
		fastboot_fail("oem writeconfig <id:0...B> <value>");
		return;
	} else {
		sscanf(arg, " %x %x", &id, &value);
		if ( id >= 0 && id < 0xC ) {
			if ( 0 !=  htc_setting_cfgdata_set(id, value) ) {
				fastboot_fail("oem writeconfig failed!");
				return;
			}
		} else {
			fastboot_fail("oem writeconfig <id:0...B> <value>");
			return;
		}
	}
	fastboot_okay("");
}

void cmd_oem_writesku(const char *arg, void *data, unsigned sz)
{
	unsigned uiCount;
	unsigned dwIndex = 0, dwValue = 0;
	char strIndexName[32] = {0};

	if (strlen(arg) ==0 ) {
		fastboot_fail("oem writesku <index> <value>");
		return;
	}
	sscanf(arg, " %x %x", &dwIndex, &dwValue);
	sscanf(arg, " %s %x", &strIndexName, &dwValue);

	// get the index by argument 1
	for (uiCount = 0; uiCount < ulNumberOfSkuidData; uiCount++) {
		if ( 0 == strcmp(strIndexName, skuidDataTable[uiCount].name)) {
				dwIndex = uiCount;
				break;
		} else {
			if (dwIndex == skuidDataTable[uiCount].id) {
				strncpy(strIndexName, sizeof(strIndexName) - 1, skuidDataTable[uiCount].name);
				break;
			}
		}
	}

	// check the index of skuid argument
	if ((uiCount >= ulNumberOfSkuidData))	// SKUID_DATA_NUM-1 for Engineer ID
	{
		fastboot_printf(fail, "Write config: invalid skuid item/index! index:%X value:%X", dwIndex, dwValue);
		return;
	}

#warning Have bug here: dwIndex may be a wrong value
	htc_setting_skuid_write(dwIndex, dwValue, true);
	fastboot_printf(info, "write skuid index:0x%X, value:0x%X", dwIndex, dwValue);

	fastboot_okay("");
	return;
}

void cmd_oem_readsku(const char *arg, void *data, unsigned sz)
{
	unsigned uiCount = 0;
	unsigned dwIndex = 0, dwValue = 0;
	char strIndexName[32] = {0};

	if (strlen(arg) == 0) {
		for (uiCount = 0; uiCount < ulNumberOfSkuidData; uiCount++) {
			dwValue = htc_setting_skuid_read(uiCount);
			fastboot_printf(info, "skuid item:%s \r\nindex:0x%X, value:0x%X\r\n\r\n",
				skuidDataTable[uiCount].name, uiCount,dwValue);
		}
	} else if (0 == strcmp(arg, " all") ){
			fastboot_printf(info, "100\r\n");
			for (uiCount = 0; uiCount < ulNumberOfSkuidData;uiCount++) {
				dwValue = htc_setting_skuid_read(uiCount);
				fastboot_printf(info, "skuid item: %s\r\nindex:0x%X, value:0x%X\r\n\r\n",
					skuidDataTable[uiCount].name, uiCount, dwValue);
			}
	} else {
		sscanf(arg, " %x", &dwIndex);
		sscanf(arg, " %s", &strIndexName);
		for (uiCount = 0; uiCount < ulNumberOfSkuidData; uiCount++) {
			if ( 0 == strcmp(strIndexName, skuidDataTable[uiCount].name) ) {
				dwIndex = skuidDataTable[uiCount].id;
				break;
			} else if (dwIndex == skuidDataTable[uiCount].id) {
				strncpy(strIndexName, sizeof(strIndexName) - 1, skuidDataTable[uiCount].name);
				break;
			}
		}

		// check the index of skuid argument
		if ((uiCount >= ulNumberOfSkuidData)) {
			fastboot_printf(fail, "Read config: invalid config item/index!%s", arg);
			return;
		}

		dwValue = htc_setting_skuid_read(dwIndex);
		fastboot_printf(info, "read skuid index:0x%X, value:0x%X", dwIndex, dwValue);
	}

	fastboot_okay("");
	return;
}

void cmd_oem_readsecureflag(const char *arg, void *data, unsigned sz)
{
	int secure_flag;

	if (strlen(arg) == 0)
	{
		secure_flag = read_security_level();
		fastboot_printf(info, "secure_flag: %d\r\n", secure_flag);
	}
	else
	{
		fastboot_fail("oem readsecureflag has no argument");
		return;
	}
	fastboot_okay("");
}

void cmd_oem_writesecureflag(const char *arg, void *data, unsigned sz)
{
	int secure_flag;
	int rc = 0;

	if (strlen(arg) == 0) {
		fastboot_fail("oem writesecureflag <value:0...3>");
		return;
	}
	else
	{
		sscanf(arg, " %d", &secure_flag);
		rc = write_security_level(secure_flag);
		if (rc)
		{
			fastboot_printf(fail, "oem writesecureflag error = %d", rc);
			return;
		}
		fastboot_okay("");
	}
}

void cmd_oem_readsmartcardmagic(const char *arg, void *data, unsigned sz)
{
	int smart_card_magic;

	if (strlen(arg) == 0)
	{
		smart_card_magic = read_sec_ex_magic(MAGIC_TYPE_SMART_SD);
		fastboot_printf(info, "smart_card_magic: %X\r\n", smart_card_magic);
	}
	else
	{
		fastboot_fail("oem readsmartcardmagic has no argument");
		return;
	}
	fastboot_okay("");
}

void cmd_oem_writesmartcardmagic(const char *arg, void *data, unsigned sz)
{
	int smart_card_magic;
	int rc = 0;

	if (strlen(arg) == 0)
	{
		fastboot_fail("oem writesmartcardmagic <value>");
		return;
	}
	else
	{
		sscanf(arg, " %x", &smart_card_magic);
		rc = write_sec_ex_magic(MAGIC_TYPE_SMART_SD, smart_card_magic);
		if (rc)
		{
			fastboot_printf(fail, "oem writesmartcardmagic error = %d", rc);
			return;
		}
		fastboot_okay("");
	}
}

void cmd_oem_reboot_download(const char *arg, void *data, unsigned sz) {
	dprintf(INFO, "rebooting the device to DOWNLOAD mode\n");
	fastboot_okay("");
	htc_set_reboot_reason( HTC_REBOOT_INFO_DOWNLOAD_MODE, NULL);
	mtk_arch_reset(1); //bypass pwr key when reboot
}

void cmd_oem_reboot_ftm(const char *arg, void *data, unsigned sz) {
	dprintf(INFO, "rebooting the device to FTM mode\n");
	fastboot_okay("");
	htc_set_reboot_reason( HTC_REBOOT_INFO_FACTORY_MODE, NULL);
	mtk_arch_reset(1); //bypass pwr key when reboot
}

void cmd_oem_reboot_meta(const char *arg, void *data, unsigned sz) {
	dprintf(INFO, "rebooting the device to META mode\n");
	fastboot_okay("");
	htc_set_reboot_reason( HTC_REBOOT_INFO_META_MODE, NULL);
	mtk_arch_reset(1); //bypass pwr key when reboot
}

void cmd_oem_reboot_ruu(const char *arg, void *data, unsigned sz) {
	dprintf(INFO, "rebooting the device to RUU mode\n");
	fastboot_okay("");
	htc_set_reboot_reason( HTC_REBOOT_INFO_REBOOT_RUU, NULL);
	mtk_arch_reset(1); //bypass pwr key when reboot
}

void cmd_oem_listpartition(const char *arg, void *data, unsigned sz)
{
	partition_dump_fb();
	fastboot_okay("");
}

#if _DDR_TEST
void cmd_oem_ddrtest(const char *arg, void *data, unsigned sz)
{
	unsigned addr, len, pattern, count, mode;
	int ret = -1;
	char info_buffer[128];

	memset(info_buffer, 0, sizeof(info_buffer));
	if (strlen(arg) == 0) {
		fastboot_fail("oem ddrtest <start address> <length> <pattern> <count> <mode>");
		fastboot_fail("<mode>==0, do memory test with cache");
		fastboot_fail("<mode>==1, do memory test without cache");
		return;
	} else {
		sscanf(arg, " %x %x %x %x %x", &addr, &len, &pattern, &count, &mode);
		htc_voprintf("ddrtest start: addr:%08x, len:%08x, pattern:%08x, count:%08x, mode:%08x\n", addr, len, pattern, count, mode);

		if(mode==1){/* Disable D-Cache */
			ddr_test_disbale_dcache();
		}else if(mode==0)
		{/* Enable D-Cache */
			ddr_test_enable_dcache();
		}

		snprintf(info_buffer, sizeof(info_buffer),"ddrtest start: addr:%08x, len:%08x, pattern:%08x, count:%08x, mode:%08x\n", addr, len, pattern, count, mode);
		fastboot_info(info_buffer);
		while(count)
		{
			ret = complex_mem_test(addr, len, 1, pattern);
			if(ret) break;
			count --;
		}

		htc_voprintf("ddrtest: ret:%d, test %s\n", ret, (ret>=0)?"pass":"fail");
		memset(info_buffer, 0, sizeof(info_buffer));
		snprintf(info_buffer, sizeof(info_buffer), "ddrtest: ret:%d, test %s\n", ret, (ret>=0)?"pass":"fail");
		fastboot_info(info_buffer);
	}

	/* after testing, enable D-Cache. */
	ddr_test_enable_dcache();

	fastboot_okay("");
	fastboot_info("DDRTEST");
}
#endif

#if TAMPER_DETECT
void cmd_oem_get_tamper_flag(const char *arg, void *data, unsigned sz)
{
	unsigned int tamper_flag;
	char response[128];

	tamper_get_flag(&tamper_flag);

	sprintf(response, "\tTamper_Flag: 0x%X", tamper_flag);
	fastboot_info(response);

	fastboot_okay("");
}

void cmd_oem_get_force_sec_boot_reason(const char *arg, void *data, unsigned sz)
{
	char response[128];

	sprintf(response, "\tForce Sec Boot Reason: 0x%X", setting_get_force_sec_boot_reason());
	fastboot_info(response);

	fastboot_okay("");
}
#endif //!TAMPER_DETECT

void cmd_oem_read_mmc(const char *arg, void *data, unsigned sz)
{
	char * temp_buf = NULL;
	uint64_t start_addr  = 0;
	unsigned int total_length = 0, read_blocks = 0, read_unit = 0;
	unsigned int is_show = 0, block_size = 0, shift_sector = 0;
	uint8_t *data_buffer, *temp_ptr, i_count;
	unsigned int bit_count = 0, line_count = 0;
	char print_buffer[64] = {0x00};
	unsigned int part_id = EMMC_PART_UNKNOWN;

	/* get emmc block size, always is 512 */
	block_size = htc_mmc_get_device_blocksize();

	/* chech emmc string*/
	temp_buf = strtok(arg, " ");
	if (!strncmp(temp_buf, "emmc_boot", 9))
		part_id = EMMC_PART_BOOT1;
	else if (strncmp(temp_buf, "emmc", 4))
		goto err_input;

	/* read start address */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		start_addr = atoul(temp_buf);
	else
		goto err_input;

	/* read total length */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		total_length = atol(temp_buf);
	else
		goto err_input;

	/* read read blocks */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		read_blocks = atol(temp_buf);
	else
		goto err_input;

	/* total length == 0; fail; */
	if (!read_blocks || !total_length)
		goto err_input;

	/* check print data on PC */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		is_show = atol(temp_buf);
	else
		goto err_input;

	/*check total length >= read blocks*/
	if (total_length < read_blocks)
		read_blocks = total_length;

	/* read data from emmc */
	data_buffer = (uint8_t *)memalign(CACHE_LINE, ROUNDUP(read_blocks * block_size, CACHE_LINE));
	if (!data_buffer) {
		fastboot_fail("Could not allocate memory for fastboot buffer");
		ASSERT(0);
		goto end;
	}
	memset(data_buffer, 0xA, read_blocks * block_size);

	while (total_length) {
		read_unit = read_blocks < total_length ? read_blocks : total_length;
		if (part_id != EMMC_PART_UNKNOWN) {
			if (htc_mmc_read2(start_addr * block_size, (unsigned int *)data_buffer, read_unit * block_size, part_id)) {
				fastboot_fail("ERROR: Cannot read emmc by partition");
				goto end;
			}
		} else {
			if (htc_mmc_read(start_addr * block_size, (unsigned int *)data_buffer, read_unit * block_size)) {
				fastboot_fail("ERROR: Cannot read emmc");
				goto end;
			}
		}

		snprintf(print_buffer, sizeof(print_buffer), "Read from the %llu sector and length is %u",
			start_addr, read_unit);
		fastboot_info(print_buffer);

		/* Print data to PC  */
		memset(print_buffer, 0, sizeof(print_buffer));
		shift_sector = 0;
		for(i_count = 0; i_count < read_unit; i_count++)
		{
			temp_ptr = data_buffer + i_count * block_size;
			snprintf(print_buffer, sizeof(print_buffer), "Current Sector at %llu",
				start_addr + shift_sector++);
			fastboot_info(print_buffer);
			for(line_count = 0; is_show && (line_count < 32); line_count++, bit_count++) {
				snprintf(print_buffer, sizeof(print_buffer),
					"[%03X]%02X %02X %02X %02X %02X %02X %02X %02X"
					"\t%02X %02X %02X %02X %02X %02X %02X %02X", bit_count * 16,
					*temp_ptr, *(temp_ptr+1), *(temp_ptr+2), *(temp_ptr+3),
					*(temp_ptr+4), *(temp_ptr+5), *(temp_ptr+6), *(temp_ptr+7),
					*(temp_ptr+8), *(temp_ptr+9), *(temp_ptr+10), *(temp_ptr+11),
					*(temp_ptr+12), *(temp_ptr+13), *(temp_ptr+14), *(temp_ptr+15));
				fastboot_info(print_buffer);
				temp_ptr += 16;
			}
		}
		start_addr += read_unit;
		total_length -= read_unit;
	}
	fastboot_okay("");

end:
	free(data_buffer);
	return ;
err_input:
	fastboot_info("emmc [start] [#total blocks!=0] [#read length!=0] [show]");
	fastboot_fail("");
}

void cmd_oem_write_mmc(const char *arg, void *data, unsigned sz)
{
	char * temp_buf = NULL;
	uint64_t start_addr  = 0, write_blocks = 0, write_unit = 0;
	unsigned int total_length = 0;
	unsigned int write_value = 0, block_size = 0, shift_sector = 0;
	uint8_t *data_buffer, *temp_ptr, i_count;
	unsigned int bit_count = 0, line_count = 0;
	char print_buffer[64] = {0x00};
	unsigned int part_id = EMMC_PART_UNKNOWN;

	/* get emmc block size, always is 512 */
	block_size = htc_mmc_get_device_blocksize();

	/* chech emmc string*/
	temp_buf = strtok(arg, " ");
	if (!strncmp(temp_buf, "emmc_boot", 9))
		part_id = EMMC_PART_BOOT1;
	else if (strncmp(temp_buf, "emmc", 4))
		goto err_input;

	/* start address */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		start_addr = atoul(temp_buf);
	else
		goto err_input;

	/* total length */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		total_length = atol(temp_buf);
	else
		goto err_input;

	/* write blocks */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		write_blocks = atol(temp_buf);
	else
		goto err_input;

	/* total length == 0; fail; */
	if (!write_blocks || !total_length)
		goto err_input;

	/* write data value */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		write_value = atol(temp_buf);
	else
		goto err_input;

	/* check total length >= read write_blocks*/
	if (total_length < write_blocks)
		write_blocks = total_length;

	/* write data to emmc alignment */
	data_buffer = (uint8_t *)memalign(CACHE_LINE, ROUNDUP(write_blocks * block_size, CACHE_LINE));
	if (!data_buffer) {
		fastboot_fail("Could not allocate memory for fastboot buffer");
		ASSERT(0);
		goto end;
	}

	memset(data_buffer, write_value, write_blocks * block_size);
	while (total_length) {
		write_unit = write_blocks < total_length ? write_blocks : total_length;
		if (part_id != EMMC_PART_UNKNOWN) {
			if (htc_mmc_write2(start_addr * block_size, write_unit * block_size,
			   (unsigned int *)data_buffer, part_id)) {
			   fastboot_fail("ERROR: Cannot write emmc by partitions");
			   goto end;
			}
		} else {
			if (htc_mmc_write(start_addr * block_size, write_unit * block_size,
				(unsigned int *)data_buffer)) {
				fastboot_fail("ERROR: Cannot write emmc");
				goto end;
			}
		}
		snprintf(print_buffer, sizeof(print_buffer), "Read from the %llu sector and length is %u",
			start_addr, write_unit);
		fastboot_info(print_buffer);
		start_addr += write_unit;
		total_length -= write_unit;
	}
	fastboot_okay("");

end:
	free(data_buffer);
	return ;
err_input:
	fastboot_info("emmc [start] [#total blocks!=0] [#write length!=0] [Value]");
	fastboot_fail("");
}

void cmd_oem_test_emmc(const char *arg, void *data, unsigned sz)
{
	char test_pattern[4] = {0xAA, 0x55, 0xA5, 0x5A}, info[60], val;
	char *temp_buf;
	uint8_t *wptr = NULL, *rptr = NULL;
	uint64_t w_sector, r_sector;
	uint32_t test_times=0, num, times, count;
	int rc=0, j;
	const uint32_t SECTOR_SIZE = 512;
	const uint32_t MAX_RW_SECTORS = 256;

	/* prepare buffer */
	wptr = (uint8_t *)memalign(CACHE_LINE, ROUNDUP(MAX_RW_SECTORS*SECTOR_SIZE, CACHE_LINE));
	if (!wptr) {
		sprintf(wptr, "Could not allocate memory for fastboot buffer\n.");
		fastboot_info(wptr);
		ASSERT(0);
	}
	rptr = (uint8_t *)memalign(CACHE_LINE, ROUNDUP(MAX_RW_SECTORS*SECTOR_SIZE, CACHE_LINE));
	if (!rptr) {
		sprintf(rptr, "Could not allocate memory for fastboot buffer\n.");
		fastboot_info(rptr);
		ASSERT(0);
	}

	/* start address */
	temp_buf = strtok(arg, " ");
	if (temp_buf) {
		w_sector = atoul(temp_buf);
		r_sector = w_sector;
	} else
		goto err_input;

	/* total sectors */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		num = atol(temp_buf);
	else
		goto err_input;

	/* test times */
	temp_buf = strtok(NULL, " ");
	if (temp_buf)
		times = atol(temp_buf);
	else
		goto err_input;

	while (times) {
		val = test_pattern[times%4];
		memset(info, 0, 60);
		sprintf(info, "writing sector %llu ~ %llu, value  %02X\r\n", w_sector, w_sector + num - 1, val & 0xFF);
		fastboot_info(info);
		j = 0;
		memset(wptr, val & 0xFF, MAX_RW_SECTORS * SECTOR_SIZE);
		while (num) {
			count = (num > MAX_RW_SECTORS) ? MAX_RW_SECTORS : num;
			rc =htc_mmc_write(w_sector * SECTOR_SIZE, count * SECTOR_SIZE, (uint32_t *)wptr);
			if (rc == 0) {
				num -= count;
				w_sector += count;
				j += count;
			} else {
				memset(info, 0, 60);
				sprintf(info, "write sector fail num = %u r_sector = %llu\r\n", num, r_sector);
				fastboot_info(info);
				goto err;
			}
		}

		memset(info, 0, 60);
		sprintf(info, "write patter  %02X done\r\n", val);
		fastboot_info(info);
		fastboot_info("start to read and compare\r\n");
		num = j;
		while (num) {
			count = (num > MAX_RW_SECTORS) ? MAX_RW_SECTORS : num;
			rc =htc_mmc_read(r_sector * SECTOR_SIZE, (uint32_t *)rptr, count*SECTOR_SIZE);
			if (rc == 0) {
				if (memcmp(rptr, wptr, count * SECTOR_SIZE)) {
					memset(info, 0, 60);
					sprintf(info, "compare mismatch at sector %llu\r\n", r_sector);
					fastboot_info(info);
					goto err;
				}
				num -= count;
				memset(rptr, 0x00, MAX_RW_SECTORS * SECTOR_SIZE);
				r_sector += count;
			} else {
				memset(info, 0, 60);
				sprintf(info, "reading sector %llu fail\r\n", r_sector);
				fastboot_info(info);
				goto err;
			}
		}

		test_times++;
		memset(info, 0, 60);
		sprintf(info, "%uth test finished and ok\r\n", test_times);
		fastboot_info(info);
		r_sector -= j;
		w_sector -= j;
		num = j;
		times--;
	}

	fastboot_okay("");

err_input:
	fastboot_info("[start] [num] [iterations]");

err:
	if (rptr) {
		free(rptr);
		rptr = NULL;
	}

	if (wptr) {
		free(wptr);
		wptr = NULL;
	}

	fastboot_fail("");
}

void cmd_oem_check_emmc_mid(const char *arg, void *data, unsigned sz)
{
	int vender_id = 0;
	char info_buffer[64];
	char *ptr = NULL;
	memset(info_buffer, 0, sizeof(info_buffer));
	ptr = htc_mmc_get_vendor(&vender_id);
	snprintf(info_buffer, sizeof(info_buffer), "eMMC Manufacturer ID: 0x%02X", vender_id);
	fastboot_info(info_buffer);
	snprintf(info_buffer, sizeof(info_buffer), "eMMC should be %s", ptr);
	fastboot_info(info_buffer);
	fastboot_okay("");
}

void cmd_oem_get_ext_csd_emmc(const char *arg, void *data, unsigned sz)
{
	fastboot_info("eMMC ext csd :");
	fastboot_prhex(htc_get_mmc_ext_csd(), 512);
	fastboot_okay("");
}

void cmd_get_wp_info_emmc(const char *arg, void *data, unsigned sz)
{
	char info_buffer[64];

	memset(info_buffer, 0, sizeof(info_buffer));
	snprintf(info_buffer, sizeof(info_buffer), "eMMC write protection group size = %d"
			, htc_mmc_get_eraseunit_size());
	fastboot_info(info_buffer);
	fastboot_okay("");
}

void htc_oem_command_register()
{
	fastboot_register("oem ?", cmd_oem_help, 0);
	fastboot_register("oem readconfig", cmd_oem_readconfig, 0);
	fastboot_register("oem writeconfig", cmd_oem_writeconfig, 0);

	fastboot_register("oem readsku", cmd_oem_readsku, 0);
	fastboot_register("oem writesku", cmd_oem_writesku, 0);

	fastboot_register("oem readsecureflag", cmd_oem_readsecureflag, 0);
	fastboot_register("oem readsmartcardmagic", cmd_oem_readsmartcardmagic, 0);
	// Must disable "oem writesecureflag" before SHIIP due to LK did NOT have
	// the image verification functionality so that S_ON in LK should be forbidden
	fastboot_register("oem writesecureflag", cmd_oem_writesecureflag, 0);
	fastboot_register("oem writesmartcardmagic", cmd_oem_writesmartcardmagic, 0);

	//oem reboot commands
	fastboot_register("oem reboot-download", cmd_oem_reboot_download, 0);
	fastboot_register("oem reboot-ftm", cmd_oem_reboot_ftm, 0);
	fastboot_register("oem reboot-meta", cmd_oem_reboot_meta, 0);
	fastboot_register("oem rebootRUU", cmd_oem_reboot_ruu, 0);
	fastboot_register("oem listpartition", cmd_oem_listpartition, 0);
	fastboot_register("oem read_mmc", cmd_oem_read_mmc, 0);
	fastboot_register("oem write_mmc", cmd_oem_write_mmc, 0);
	fastboot_register("oem test_emmc", cmd_oem_test_emmc, 0);
	fastboot_register("oem check_emmc_mid", cmd_oem_check_emmc_mid, 0);
	fastboot_register("oem get_ext_csd_emmc", cmd_oem_get_ext_csd_emmc,0);
	fastboot_register("oem get_wp_info_emmc", cmd_get_wp_info_emmc,0);
#if _DDR_TEST
	fastboot_register("oem ddrtest", cmd_oem_ddrtest, 0);
#endif

#if TAMPER_DETECT
	fastboot_register("oem get_tamper_flag", cmd_oem_get_tamper_flag, 0);
	fastboot_register("oem get_force_sec_boot_reason", cmd_oem_get_force_sec_boot_reason, 0);
#endif //!TAMPER_DETECT

}
