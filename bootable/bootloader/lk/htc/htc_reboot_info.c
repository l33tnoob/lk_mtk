#include <debug.h>
#include <sys/types.h>
#include <platform/partition.h>
#include <htc_reboot_info.h>

typedef struct{
	unsigned int	reboot_reason;
	unsigned char	reboot_string[HTC_REBOOT_INFO_REBOOT_STRING_SIZE];
	unsigned char	reserved[512 - 4 - HTC_REBOOT_INFO_REBOOT_STRING_SIZE];
} htc_reboot_info_struct;

static htc_reboot_info_struct *htc_reboot_info_p = NULL;
static htc_reboot_info_struct *last_htc_reboot_info_p = NULL;

#define REBOOT_STRING(p) \
	(((p) != NULL && ( (p)->reboot_reason == 0x0 || IS_HTC_STOCK_REBOOT_INFO((p)->reboot_reason) || IS_HTC_OEM_REBOOT_INFO((p)->reboot_reason)))?(p)->reboot_string:NULL)

#ifdef ENABLE_HTC_REBOOT_INFO_SAVE_IN_EMMC
typedef struct{
	unsigned int  is_support;
	unsigned int  offset;
	const char    partition_name[256];
} htc_reboot_info_save_emmc_support_struct;

static htc_reboot_info_struct htc_reboot_info_emmc = {0};
static htc_reboot_info_save_emmc_support_struct htc_reboot_info_save_emmc_support = {0};
#endif

static void* _virt_to_phys(void* addr) {
	return addr;
}

static void* _phys_to_virt(void* addr) {
	return addr;
}

static int htc_is_valid_stock_reboot_info() {
	if (htc_reboot_info_p) {
		return IS_HTC_STOCK_REBOOT_INFO(htc_reboot_info_p->reboot_reason)?1:0;
	}
	return 0;
}

static int htc_is_valid_oem_reboot_info() {
	if (htc_reboot_info_p) {
		return IS_HTC_OEM_REBOOT_INFO(htc_reboot_info_p->reboot_reason)?1:0;
	}
	return 0;
}

int htc_is_valid_reboot_info() {
	if (htc_is_valid_stock_reboot_info() || htc_is_valid_oem_reboot_info() ) {
		return 1;
	}
	return 0;
}

void  htc_reboot_info_init(void* phys_addr, const char* emmc_part_name, unsigned int emmc_offset) {
	dprintf(CRITICAL, "zzytest, htc_reboot_info_init begin\n");
	if (htc_reboot_info_p != NULL) {
		//htc_reboot_info_p has been inited already!
		return;
	}

	htc_reboot_info_p = (htc_reboot_info_struct*) _phys_to_virt(phys_addr);
	last_htc_reboot_info_p = (htc_reboot_info_struct*) ((unsigned char*)_phys_to_virt(phys_addr) + sizeof(htc_reboot_info_struct));

	htc_reboot_info_p->reboot_string[HTC_REBOOT_INFO_REBOOT_STRING_SIZE - 1] = 0;
	last_htc_reboot_info_p->reboot_string[HTC_REBOOT_INFO_REBOOT_STRING_SIZE - 1] = 0;

	dprintf(CRITICAL, "%s: ddr reboot_reason:      0x%08X\n", __func__, htc_reboot_info_p->reboot_reason);
	dprintf(CRITICAL, "%s: ddr reboot_string:      %s\n", __func__, REBOOT_STRING(htc_reboot_info_p));
	dprintf(CRITICAL, "%s: ddr last reboot_reason: 0x%08X\n", __func__, last_htc_reboot_info_p->reboot_reason);
	dprintf(CRITICAL, "%s: ddr last reboot_string: %s\n", __func__, REBOOT_STRING(last_htc_reboot_info_p));

#ifdef ENABLE_HTC_REBOOT_INFO_SAVE_IN_EMMC
	if (emmc_part_name != NULL) {
		if (0 >= partition_read(emmc_part_name, emmc_offset, &htc_reboot_info_emmc, sizeof(htc_reboot_info_struct))) {
			dprintf(CRITICAL, "%s: partition_read failed!\n", __func__);
			memset(&htc_reboot_info_emmc, 0, sizeof(htc_reboot_info_struct));
			htc_reboot_info_save_emmc_support.is_support = 0;
		} else {
			htc_reboot_info_save_emmc_support.is_support = 1;
			htc_reboot_info_save_emmc_support.offset = emmc_offset;
			strncpy(htc_reboot_info_save_emmc_support.partition_name, emmc_part_name, sizeof(htc_reboot_info_save_emmc_support.partition_name));
		}
		dprintf(CRITICAL, "%s: emmc reboot_reason: 0x%08X\n", __func__, htc_reboot_info_emmc.reboot_reason);
		dprintf(CRITICAL, "%s: emmc reboot_string: %s\n", __func__, REBOOT_STRING(&htc_reboot_info_emmc));
	}

	if (!htc_is_valid_reboot_info() && htc_reboot_info_save_emmc_support.is_support) {
		memcpy(htc_reboot_info_p, &htc_reboot_info_emmc, sizeof(htc_reboot_info_struct));
	}
#endif

	if (!htc_is_valid_reboot_info() && htc_reboot_info_p->reboot_reason != 0x0 ) {
		memset(htc_reboot_info_p, 0, sizeof(htc_reboot_info_struct));
	}
	htc_reboot_info_p->reboot_string[HTC_REBOOT_INFO_REBOOT_STRING_SIZE - 1] = 0;
	memcpy(last_htc_reboot_info_p, htc_reboot_info_p, sizeof(htc_reboot_info_struct));

	/* clean current reboot_info */
	memset(htc_reboot_info_p, 0, sizeof(htc_reboot_info_struct));

#ifdef ENABLE_HTC_REBOOT_INFO_SAVE_IN_EMMC
	if (emmc_part_name != NULL && htc_reboot_info_save_emmc_support.is_support) {
		if ( 0 >= partition_write(emmc_part_name, emmc_offset, htc_reboot_info_p, sizeof(htc_reboot_info_struct))) {
			dprintf(CRITICAL, "%s: partition_write failed!\n", __func__);
		}
	}
#endif
}

int htc_set_reboot_reason(unsigned int reboot_reason, const unsigned char* reboot_string) {
	if (htc_reboot_info_p) {
		memset(htc_reboot_info_p, 0, sizeof(htc_reboot_info_struct));
		htc_reboot_info_p->reboot_reason = reboot_reason;
		dprintf(CRITICAL, "%s: reboot_reason: 0x%08X\n", __func__, reboot_reason);
		dprintf(CRITICAL, "%s: reboot_string: %s\n", __func__, reboot_string);
		if (reboot_string != NULL) {
			strncpy(htc_reboot_info_p->reboot_string, reboot_string, sizeof(htc_reboot_info_p->reboot_string)-1);
		}

#ifdef ENABLE_HTC_REBOOT_INFO_SAVE_IN_EMMC
		if (htc_reboot_info_save_emmc_support.is_support) {
			memset(&htc_reboot_info_emmc, 0, sizeof(htc_reboot_info_struct));
			if (reboot_reason != 0xAABBCCDD) {
				memcpy(&htc_reboot_info_emmc, htc_reboot_info_p, sizeof(htc_reboot_info_struct));
			}

			if ( 0 >= partition_write(
						htc_reboot_info_save_emmc_support.partition_name,
						htc_reboot_info_save_emmc_support.offset,
						&htc_reboot_info_emmc,
						sizeof(htc_reboot_info_struct))) {
				dprintf(CRITICAL, "%s: partition_write failed!\n", __func__);
			}
		}
#endif

		return 0;
	}
	return -1;
}

void htc_flush_reboot_info(void)
{
	if(htc_reboot_info_p)
		arch_clean_cache_range(htc_reboot_info_p, sizeof(htc_reboot_info_struct));
}


unsigned int htc_get_reboot_reason() {
	return last_htc_reboot_info_p->reboot_reason;
}

unsigned char* htc_get_reboot_string() {
	return REBOOT_STRING(last_htc_reboot_info_p);
}
