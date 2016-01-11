#ifndef __HTC_REBOOT_INFO_H__
#define __HTC_REBOOT_INFO_H__

#define HTC_REBOOT_INFO_REBOOT_STRING_SIZE	252

#define HTC_OEM_REBOOT_INFO_MAGIC	(0x6F656D00)
#define HTC_STOCK_REBOOT_INFO_MAGIC	(0x77665500)


#define HTC_OEM_REBOOT_INFO_MASK	(0xFFFFFF00)
#define HTC_STOCK_REBOOT_INFO_MASK	(0xFFFFFF00)

#define IS_HTC_OEM_REBOOT_INFO(reset_reason) \
		((reset_reason & HTC_OEM_REBOOT_INFO_MASK) == HTC_OEM_REBOOT_INFO_MAGIC)

#define IS_HTC_STOCK_REBOOT_INFO(reset_reason) \
		((reset_reason & HTC_STOCK_REBOOT_INFO_MASK) == HTC_STOCK_REBOOT_INFO_MAGIC)

#define SET_HTC_OEM_REBOOT_REASON(reset_reason) \
		(HTC_OEM_REBOOT_INFO_MAGIC | (((unsigned int)reset_reason) & ~HTC_OEM_REBOOT_INFO_MASK ))

#define SET_HTC_STOCK_REBOOT_REASON(reset_reason) \
		(HTC_STOCK_REBOOT_INFO_MAGIC | (((unsigned int)reset_reason) & ~HTC_STOCK_REBOOT_INFO_MASK ))


#define HTC_REBOOT_INFO_BOOTLOADER		SET_HTC_STOCK_REBOOT_REASON(0x00)
#define HTC_REBOOT_INFO_REBOOT			SET_HTC_STOCK_REBOOT_REASON(0x01)
#define HTC_REBOOT_INFO_RECOVERY		SET_HTC_STOCK_REBOOT_REASON(0x02)
#define HTC_REBOOT_INFO_RAMDUMP			SET_HTC_STOCK_REBOOT_REASON(0xAA)

#define HTC_REBOOT_INFO_REBOOT_RUU		SET_HTC_OEM_REBOOT_REASON(0x78)
#define HTC_REBOOT_INFO_NORMAL			SET_HTC_OEM_REBOOT_REASON(0x88)
#define HTC_REBOOT_INFO_DOWNLOAD_MODE		SET_HTC_OEM_REBOOT_REASON(0xE0)
#define HTC_REBOOT_INFO_FTM_1			SET_HTC_OEM_REBOOT_REASON(0xF1)
#define HTC_REBOOT_INFO_FTM_2			SET_HTC_OEM_REBOOT_REASON(0xF2)
#define HTC_REBOOT_INFO_DDRTEST			SET_HTC_OEM_REBOOT_REASON(0xFF)

#define HTC_REBOOT_INFO_META_MODE		HTC_REBOOT_INFO_FTM_1
#define HTC_REBOOT_INFO_FACTORY_MODE	HTC_REBOOT_INFO_FTM_2

void htc_reboot_info_init(void* phys_addr, const char* emmc_part_name, unsigned int emmc_offset);
int htc_is_valid_reboot_info();
int htc_set_reboot_reason(unsigned int reboot_reason, const unsigned char* reboot_string);
unsigned int htc_get_reboot_reason();
unsigned char* htc_get_reboot_string();
void htc_flush_reboot_info(void);

#endif //__HTC_REBOOT_INFO_H__
