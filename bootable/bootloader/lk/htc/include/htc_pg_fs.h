#ifndef __HTCPGFS_H__
#define __HTCPGFS_H__

#ifdef QCT_PLATFORM
#else
#include "debug.h"
#include "partition.h"


#define PGFS_PREFIX	"pg"
#define SECTOR_SIZE	BLK_SIZE
#define alloc_cache_page_aligned(size)	memalign(4096, size)
#define alloc_page_aligned(size) 			memalign(4096, size)

#define HLOGD printf
#define HLOGI printf
#define HLOGE printf
#endif

#define HTC_PG_DEBUG		0

#if HTC_PG_DEBUG
#define PGDBG(s...)	do {\
					HLOGD("[PG] %s(%d):", __FUNCTION__, __LINE__); \
					HLOGD(" "s); \
					} while (0)
#else
#define PGDBG(s...)	 do {} while (0)
#endif
#define PGERR(s...)	do {\
					HLOGD("[PG_ERROR] %s(%d):", __FUNCTION__, __LINE__); \
					HLOGD(" "s); \
					} while (0)

#define HTC_PG_HEADER_MAGIC			0x53464750			/* PGFS */
#define HTC_PG_DEF_TOTAL_SIZE		(1 << 25)			/* 32MB */
#define HTC_PG_DEF_MAX_PART_NUM		32
#define HTC_PG_DEF_MAX_PART_SIZE	(31 << 20)			/* 31MB */
#define HTC_PG_DEF_GROUP_SIZE		(1 << 17)			/* 128KB */
#define HTC_PG_DEF_SECTOR_SIZE		(1 << 10)			/* 1KB */
#define HTC_PG_DEF_OUTER_SECTOR_SIZE (1 << 9)			/* 512B */


#define PG_ERR_NONE				0
#define PG_ERR_EMMC_ACCESS		-1
#define PG_ERR_EMMC_INIT		-2
#define PG_ERR_PARAMETER		-3
#define PG_ERR_ALLOC			-4
#define PG_ERR_NO_PG_HDR		-5
#define PG_ERR_NOT_FOUND		-6
#define PG_ERR_FULL				-7
#define PG_ERR_EMMC_ERASE		-8
#define PG_ERR_LINK				-9
#define PG_ERR_LINK_FULL		-10
#define PG_ERR_FN_RET			-11

#define HTC_PG_NAME_MAX_LEN		32
#define HTC_PG_HEADER_LEN		1024
#define HTC_PART_HEADER_LEN		1024

#define HTC_PG_LINK_RANGE_DEF	0x80
#define HTC_PG_LINK_GROUP_ALL	0xFF

#define HTC_PG_ATTR_LINK		(1 << 0)	/* partition is defined with links */

#define HTC_PG_FS_INTER			0
#define HTC_PG_FS_OUTER			1

struct htc_pg_link_t {
	uint8	GI;			/* group index */
	uint8	SI;			/* sector index */
} __attribute__((packed));

typedef struct htc_pg_link_t htc_pg_link;

struct htc_part_hdr_t {
	int8	name[HTC_PG_NAME_MAX_LEN];		/* partition name */
	uint32	size;							/* partition size */
	uint32	checksum;						/* partition header and content checksum */
	uint32	attr;							/* partition attribute */
	uint32	link_num;						/* number of link */
	union {
		uint8 data[HTC_PART_HEADER_LEN - HTC_PG_NAME_MAX_LEN - 16];
		htc_pg_link link[(HTC_PART_HEADER_LEN - HTC_PG_NAME_MAX_LEN - 16) / 2];
	} dl;
} __attribute__((packed));

typedef struct htc_part_hdr_t htc_part_hdr;

struct htc_pg_hdr_t {
	uint32	magic;				/* partition group header starting magic */
	uint32	total_size;			/* partition group total size */
	uint32	max_part_num;		/* maximum partition number */
	uint32	max_part_size;		/* maximum partition size */
	uint32	part_num;			/* partition number */
	uint32	group_size;			/* allocation group size */
	uint32	sector_size;		/* allocation unit size */
	uint8	reserved[HTC_PG_HEADER_LEN - 28];			/* unused */
	htc_part_hdr part[0];
} __attribute__((packed));

#define PG_DATA_SIZE(pg) (pg->total_size - HTC_PG_HEADER_LEN - pg->max_part_num * HTC_PART_HEADER_LEN)
#define PG_DATA_SECTOR_COUNT(pg) (PG_DATA_SIZE(pg) / pg->sector_size)

typedef struct htc_pg_hdr_t htc_pg_hdr;

typedef uint32 htc_pg_traverse_fn(uint32 sector, uint32 count, uint32 addr, uint32 len, uint32 argu);

int32 htc_pg_sanity_check(uint32 pg_sector, uint32 clean);
int32 htc_pg_hdr_get(uint32 pg_sector, htc_pg_hdr *pg_hdr);
int32 htc_pg_hdr_set(uint32 pg_sector, htc_pg_hdr *pg_hdr);
int32 htc_pg_part_hdr_get(uint32 pg_sector, int8 *name, htc_part_hdr *part_hdr);
int32 htc_pg_part_hdr_set(uint32 pg_sector, int8 *name, htc_part_hdr *part_hdr, uint32 pg_realloc);
int32 htc_pg_part_read(uint32 pg_sector, int8 *name, uint32 offset, void *buf, uint32 len);
int32 htc_pg_part_write(uint32 pg_sector, int8 *name, uint32 offset, void *buf, uint32 len, uint32 update_crc);
int32 htc_pg_part_erase(uint32 pg_sector, int8 *name, uint32 update_crc);
int32 htc_pg_free_size(uint32 pg_sector);
int32 htc_pg_part_crc(uint32 pg_sector, int8 *name);
int32 htc_pg_process(int argc, char *argv[]);
int32 check_pgfs(void);


#endif /*__HTCPGFS_H__*/
