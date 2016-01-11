#include <stdint.h>
#include <printf.h>
#include <malloc.h>
#include <string.h>
#include <target.h>
#include <platform/errno.h>
#include <platform/mmc_core.h>
#include <platform/partition.h>
#include <platform/mmc_wrapper.h>

#if _MAKE_HTC_LK

/* HTC emmc vendor info*/
#define MMC_HOST_ID                 0
#define MMC_BLK_SZ			512

struct emmc_info {
	char *name;
	unsigned mid;
};

const struct emmc_info emmcDataTable[] = {
	{"Sandisk", 0x02},
	{"Sandisk", 0x45},
	{"Hynix", 0x90},
	{"Micron", 0xFE},
	{"Samsung", 0x15},
	{"Kingston", 0x70},
	{"Toshiba", 0x11},
	{"unkonwn", 0xff},
};

static u32 unstuff_bits(u32 *resp, u32 start, u32 size)
{
    const u32 __mask = (1 << (size)) - 1;
    const int __off = 3 - ((start) / 32);
    const int __shft = (start) & 31;
    u32 __res;

    __res = resp[__off] >> __shft;
    if ((size) + __shft >= 32)
        __res |= resp[__off-1] << (32 - __shft);
    return __res & __mask;
}

/* Return the eMMC block size */
uint32_t htc_mmc_get_device_blocksize()
{
	struct mmc_card *card;
	card = mmc_get_card(0);
	return card->blklen;
}

/* Return the eMMC raw data */
uint32_t htc_mmc_read(uint64_t data_addr, uint32_t *out, uint32_t data_len)
{
	int size = 0;
	part_dev_t *dev = NULL;

	dprintf(CRITICAL, "[mmc_read] addr:%llu, len:%lu\n", data_addr, data_len);

	dev = mt_part_get_device();
	if (dev == NULL) {
		dprintf(CRITICAL, "Failed Getting device\n");
		return -1;
	}

	size = dev->read(dev, data_addr, out, data_len, EMMC_PART_USER);
	if (size != data_len) {
		dprintf(CRITICAL, "Failed Reading size %x != %x\n", size, data_len);
		return -1;
	}
	return 0;
}

uint32_t htc_mmc_read2(uint64_t data_addr, uint32_t *out, uint32_t data_len, uint32_t part_id)
{
	int size = 0;
	part_dev_t *dev = NULL;

	dprintf(CRITICAL, "[mmc_read2] addr:%llu, len:%lu, pid:%u\n", data_addr, data_len, part_id);

	dev = mt_part_get_device();
	if (dev == NULL) {
		dprintf(CRITICAL, "Failed Getting device\n");
		return -1;
	}

	size = dev->read(dev, data_addr, out, data_len, part_id);
	if (size != data_len) {
		dprintf(CRITICAL, "Failed Reading size %x != %x\n", size, data_len);
		return -1;
	}
	return 0;
}

uint32_t htc_mmc_write(uint64_t data_addr, uint32_t data_len, void *in)
{
	int size = 0;
	part_dev_t *dev = NULL;

	dprintf(CRITICAL, "[mmc_write] addr:%llu, len:%lu\n", data_addr, data_len);

	dev = mt_part_get_device();
	if (dev == NULL) {
		dprintf(CRITICAL, "Failed Getting device\n");
		return -1;
	}

	size = dev->write(dev, in, data_addr, data_len, EMMC_PART_USER);
	if (size != data_len) {
		dprintf(CRITICAL, "Failed Writing size %x != %x\n", size, data_len);
		return -1;
	}
	return 0;
}

uint32_t htc_mmc_write2(uint64_t data_addr, uint32_t data_len, void *in, uint32_t part_id)
{
	int size = 0;
	part_dev_t *dev = NULL;

	dprintf(CRITICAL, "[mmc_write2] addr:%llu, len:%lu, pid:%u\n", data_addr, data_len, part_id);

	dev = mt_part_get_device();
	if (dev == NULL) {
		dprintf(CRITICAL, "Failed Getting device\n");
		return -1;
	}

	size = dev->write(dev, in, data_addr, data_len, part_id);
	if (size != data_len) {
		dprintf(CRITICAL, "Failed Writing size %x != %x\n", size, data_len);
		return -1;
	}
	return 0;
}

/*
 * Function: mmc get erase unit size
 * Arg     : None
 * Return  : Returns the erase unit size of the storage
 * Flow    : Get the erase unit size from the card
 */
uint32_t htc_mmc_get_eraseunit_size()
{
	uint32_t erase_unit_sz = 0;
	uint32_t wp_unit_sz = 0;
	struct mmc_card *card;
	u8 *ext_csd;
	u32 *csd;

	card = mmc_get_card(MMC_HOST_ID);

	if (mmc_card_mmc(card))
	{
		/*
		 * Calculate the erase unit size as per the emmc specification v4.5
		 */
		ext_csd = &card->raw_ext_csd[0];
		csd = card->raw_csd;

		if (ext_csd[EXT_CSD_ERASE_GRP_DEF]) {
			erase_unit_sz = card->ext_csd.hc_erase_grp_sz / MMC_BLK_SZ;
			wp_unit_sz = card->ext_csd.hc_wp_grp_sz / MMC_BLK_SZ;
		} else {
			erase_unit_sz = card->csd.erase_sctsz;
			wp_unit_sz = card->csd.erase_sctsz * (card->csd.write_prot_grpsz + 1);
		}
	}

	dprintf(CRITICAL, "eMMC use %s memory setting.\n", (ext_csd[EXT_CSD_ERASE_GRP_DEF] & 0x1) ? "HC" : "normal");
	dprintf(CRITICAL, "-- ERASE_GRP_SIZE: %u blocks\n", erase_unit_sz);
	dprintf(CRITICAL, "-- WP_GRP_SIZE:    %u blocks\n", wp_unit_sz);

	return wp_unit_sz;
}

char * htc_mmc_get_vendor(uint32_t *vender_id)
{
	uint32_t i = 0;
	void *dev;
	struct mmc_card *card;
	u32 *resp;
	unsigned int mid = 0;

	card = mmc_get_card(MMC_HOST_ID);
	resp = card->raw_cid;

	mid = unstuff_bits(resp, 120, 8);

	for (i = 0; i < 8; i++) {
		if (emmcDataTable[i].mid == mid) {
			*vender_id = mid;
			return emmcDataTable[i].name;
			break;
		}
	}
}

void * htc_get_mmc_ext_csd()
{
	void *dev;
	struct mmc_card *card;

	card = mmc_get_card(MMC_HOST_ID);

	return (void*)&card->raw_ext_csd[0];
}

#endif //_MAKE_HTC_LK
