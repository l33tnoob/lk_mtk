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
#ifndef HTC_DTB_UTILITY_H_
#define HTC_DTB_UTILITY_H_

#include <htc_board_info_and_setting.h>
#include <stddef.h>
#include <stdint.h>

#define HTC_CONFIG_DATA_PATH "/htc,config-data"
#define HTC_SKU_DATA_PATH "/htc,sku-data"

typedef struct {
	uint32_t index;
	const char* name;
	int (*get_data)();
} htc_config_data_struct;

static const htc_config_data_struct htc_config_data[] = {
	{
		.index = 0x00000005,
		.name = "debugflag",
		.get_data = htc_setting_debugflag_get,
	},
	{
		.index = 0x00000006,
		.name = "kernelflag",
		.get_data = htc_setting_kernelflag_get,
	},
	{
		.index = 0x00000007,
		.name = "bootloaderflag",
		.get_data = htc_setting_bootloaderflag_get,
	},
	{
		.index = 0x00000008,
		.name = "radioflag",
		.get_data = htc_setting_radioflag_get,
	},
	{
		.index = 0x0000000a,
		.name = "radioflag_ex1",
		.get_data = htc_setting_radioflag_ex1_get,
	},
	{
		.index = 0x0000000b,
		.name = "radioflag_ex2",
		.get_data = htc_setting_radioflag_ex2_get,
	},
};

int htc_dtb_config_data_set(void* fdt, int offset);
int htc_dtb_append_config_data(void* fdt);
int htc_dtb_append_board_info(void* fdt);
int htc_dtb_get_dtb_from_dtbs(void *fdt, unsigned int dtb_addr, unsigned int dtb_size, char* buf, unsigned int fb_base_addr);
#endif /* HTC_DTB_UTILITY_H_ */
