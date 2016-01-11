/*
 * HTC Corporation Proprietary Rights Acknowledgment
 *
 * Copyright (C) 2013 HTC Corporation
 * All Rights Reserved.
 *
 * The information contained in this work is the exclusive property of
 * HTC Corporation ("HTC"). Only the user who is legally authorized by
 * HTC ("Authorized User") has right to employ this work within the scope of
 * this statement. Nevertheless, the Authorized User shall not use this work for
 * any purpose other than the purpose agreed by HTC.
 *
 * Any and all addition or modification to this work shall be unconditionally
 * granted back to HTC and such addition or modification shall be solely
 * owned by HTC.
 *
 * No right is granted under this statement, including but not limited to,
 * distribution, reproduction, and transmission, except as otherwise provided in
 * this statement.
 *
 * Any other usage of this work shall be subject to the further written
 * consent of HTC.
 *
 * Author: Daniel Tsai
 */

#include <stdlib.h>
#include <htc_dtb_utility.h>
#include <htc_board_info_and_setting.h>
#include <libfdt.h>
#include <debug.h>

#define FDT_BUFF_SIZE  1024
#define FDT_BUFF_PATTERN  "BUFFEND"


int htc_dtb_sku_data_set(void* fdt, int offset)
{
	int ret = 0;
	unsigned int sku_data[12];

	dprintf(CRITICAL,"Appending htc sku data info FDT\r\n");
	for (unsigned i = 0; i < ARRAY_SIZE(sku_data); ++i) {
		sku_data[i] = cpu_to_fdt32(htc_setting_skuid_read(i));
		dprintf(CRITICAL,"sku[%d]: %08X\r\n", i, htc_setting_skuid_read(i));
	}

	ret = fdt_setprop(fdt, offset, "sku-data", sku_data, sizeof(sku_data));

	if (ret) {
		dprintf(CRITICAL,"%s: Failed (%d) to add htc sku data\r\n", __func__, ret);
	}

	return ret;
}

int htc_dtb_append_sku_data(void* fdt)
{
	int32_t ret = -1, offset;

	/* Get offset of the htc,sku-data node */
	offset = fdt_path_offset(fdt, HTC_SKU_DATA_PATH);
	if (offset < 0) {
		dprintf(CRITICAL,"Could not find "HTC_SKU_DATA_PATH" node.\r\n");
		return offset;
	}
	else {
		ret = htc_dtb_sku_data_set(fdt, offset);
	}

	if (ret) {
		dprintf(CRITICAL,"Cannot update " HTC_SKU_DATA_PATH " node\r\n");
	}

	return ret;
}

int htc_dtb_config_data_set(void* fdt, int offset)
{
	int ret = 0;
	unsigned int config_data[6];

	dprintf(CRITICAL,"Appending htc config data info FDT\r\n");
	for (unsigned i = 0; i < ARRAY_SIZE(htc_config_data); ++i) {
		config_data[i] = cpu_to_fdt32(htc_config_data[i].get_data());
		dprintf(CRITICAL,"config \'%s\' (index:%08X, value:%08X) added\r\n",
				htc_config_data[i].name, htc_config_data[i].index,
				htc_config_data[i].get_data());
	}

	ret = fdt_setprop(fdt, offset, "config-data", config_data, sizeof(config_data));

	if (ret) {
		dprintf(CRITICAL,"%s: Failed (%d) to add htc config data\r\n", __func__, ret);
	}

	return ret;
}

int htc_dtb_append_config_data(void* fdt)
{
	int32_t ret = -1, offset;

	/* Get offset of the htc,config-data node */
	offset = fdt_path_offset(fdt, HTC_CONFIG_DATA_PATH);
	if (offset < 0) {
		dprintf(CRITICAL,"Could not find "HTC_CONFIG_DATA_PATH" node.\r\n");
		return offset;
	}
	else {
		ret = htc_dtb_config_data_set(fdt, offset);
	}

	if (ret) {
		dprintf(CRITICAL,"Cannot update " HTC_CONFIG_DATA_PATH " node\r\n");
	}

	return ret;
}

int htc_dtb_append_board_info(void* fdt)
{
	int32_t ret=-1, offset;
	unsigned int board_info[2];

	board_info[0] = cpu_to_fdt32(htc_get_pid());
	board_info[1] = cpu_to_fdt32(htc_setting_skuid_read(1));
	offset = fdt_path_offset(fdt, "/htc_board_info");
	if (offset >= 0) {
		ret = fdt_setprop(fdt, offset, "htc_pid,htc_sku1", board_info, sizeof(board_info));
		dprintf(CRITICAL,"%s: htc_sku1 = 0x%08x\n", __func__, htc_setting_skuid_read(1));
	}
	else{
		dprintf(CRITICAL,"Cannot find htc_board_info offset\n");
	}

	if (ret) {
		dprintf(CRITICAL,"Cannot update htc board info\r\n");
	}

	return ret;
}

int htc_dtb_get_dtb_from_dtbs(void *fdt, unsigned int dtb_addr, unsigned int dtb_size, char* buf, unsigned int fb_base_addr) {
	int ret = -1;
	int offset = 0;
	int len = 0;
	int fdt_count = 0;
	int *board_info = NULL;
	unsigned int magic_number = 0;

	do{
		memcpy(&magic_number, (void*) dtb_addr, 4);
		if(fdt32_to_cpu(magic_number) == FDT_MAGIC)
		{
			memcpy(&dtb_size, (void*) (dtb_addr+0x4), 4);
			dtb_size = fdt32_to_cpu(dtb_size);
		}
		else
		{
			if (0 == fdt_count){
				dprintf(CRITICAL,"Can't find device tree. Please check your kernel image\n");
				while(1);
				//dprintf(CRITICAL,"use default device tree\n");
				//dtb_addr = (unsigned int)&device_tree;
				//dtb_size = device_tree_size;
			}else{
				break;
			}
		}
		dprintf(CRITICAL,"dtb_addr = 0x%08X, dtb_size = 0x%08X\n", dtb_addr, dtb_size);

		if(((unsigned int)fdt + dtb_size) > fb_base_addr)
		{
			dprintf(CRITICAL,"[ERROR] dtb end address (0x%08X) is beyond the memory (0x%08X).\n", (unsigned int)fdt+dtb_size, fb_base_addr);
			return -1;
		}
		memcpy(fdt, (void *)dtb_addr, dtb_size);

		strcpy(&buf[FDT_BUFF_SIZE], FDT_BUFF_PATTERN);

		ret = fdt_open_into(fdt, fdt, MIN(0x100000, (fb_base_addr-(unsigned int)fdt)));
		if (ret) return -1;
		ret = fdt_check_header(fdt);
		if (ret) return -1;

		offset = fdt_path_offset(fdt, "/htc_board_info");

		if (offset >= 0) {
			board_info = fdt_getprop(fdt, offset, "htc_pid,htc_sku1", &len);
			if(board_info){
				dprintf(CRITICAL, "htc_pid = 0x%x, htc_pcbid=0x%x from device tree\n", fdt32_to_cpu(*(board_info)), fdt32_to_cpu(*(board_info+1)));
				if(fdt32_to_cpu(*(board_info+1)) == htc_setting_pcbid())
					break;
				}else {
					dprintf(CRITICAL, "Get htc_pid htc_pcbid property fail\n");
				}
		}
		else{
			dprintf(CRITICAL,"Cannot find htc_board_info offset\n");
		}

		fdt_count ++;
		dtb_addr = dtb_addr + dtb_size;
	}while(1);
	return ret;
}
