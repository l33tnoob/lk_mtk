/* common/htc_pg_fs.c
 *
 * This is a simple parser for htc partition group file system
 *
 */

#ifdef QCT_PLATFORM
#ifdef HTC_PG_TOOL
#include "htc_pg_tool.h"
#else
#include "common.h"
#include <string.h>
#include "memory.h"
#include "htc_pg_fs.h"
#include "partition.h"
#include "partition_tb.h"
#if SUPPORT_HTC_TZ
#include "tz.h"
#endif
#include "hal/sd.h"
#endif
#include "errno.h"
#include "delay.h"
#else
#include "htc_pg_fs.h"

#define PTBL_PGFS_OUTER	0x20
#endif

typedef struct {
	char *name;
	int size;
} pgfs_partition_t;

pgfs_partition_t pg1fs_partition[] = {
    {"security", 4 * 1024},
    {NULL, 0}
};

pgfs_partition_t pg2fs_partition[] = {
    {"sec_recovery",  4 * 1024},
    {"atsdebug_info", 1 * 1024},
    {"sec_ex_util",   1 * 1024},
    {"tamper",        1 * 1024},
    {"ship_signkey", 20 * 1024},
    {NULL, 0}
};


#define HTC_PGFS_DEBUG	0

extern unsigned Crc32CheckSum(unsigned dwStartAddr, unsigned dwTotalLen, unsigned crc);
extern int strncmp(char const *cs, char const *ct, size_t count);
extern void * memset(void *s, int c, size_t count);
extern int atoi(const char *num);
extern int sscanf(char *str, char const *fmt0, ...);

int emmc_init(void)
{
	return 0;
}

int sd_read_sector(unsigned char * pBuffer, unsigned sector_no, unsigned sector_cnt)
{
	part_t* p = (part_t*)get_part_2((unsigned long)sector_no);
	int size = 0;
	if (p == NULL)
	{
		return -1;
	}
	size = partition_read(p->info->name, (sector_no-p->start_sect)*SECTOR_SIZE, pBuffer, sector_cnt*SECTOR_SIZE);
	if (size != sector_cnt*SECTOR_SIZE)
	{
		PGERR("partition_read: size %x != %x\r\n", size, sector_cnt*SECTOR_SIZE);
		return -1;
	}
	return 0;
}

int sd_write_sector(unsigned char * pBuffer, unsigned sector_no, unsigned sector_cnt)
{
	part_t* p = (part_t*)get_part_2((unsigned long)sector_no);
	int size = 0;
	if (p == NULL)
	{
		return 0;
	}
	size = partition_write(p->info->name, (sector_no-p->start_sect)*SECTOR_SIZE, pBuffer, sector_cnt*SECTOR_SIZE);
	if (size != sector_cnt*SECTOR_SIZE)
	{
		PGERR("partition_write: size %x != %x\r\n", size, sector_cnt*SECTOR_SIZE);
		return 0;
	}
	return size;
}

int32 htc_pg_sanity_check(uint32 pg_sector, uint32 clean)
{
	uint32 i, crc;
	int rc;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr_list = NULL;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	pg_hdr = alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	rc = htc_pg_hdr_get(pg_sector, pg_hdr);
	if ((rc == PG_ERR_NO_PG_HDR) || clean) {
		memset(pg_hdr, 0, HTC_PG_HEADER_LEN);
		pg_hdr->magic = HTC_PG_HEADER_MAGIC;
		pg_hdr->total_size = HTC_PG_DEF_TOTAL_SIZE;
		pg_hdr->max_part_num = HTC_PG_DEF_MAX_PART_NUM;
		pg_hdr->max_part_size = HTC_PG_DEF_MAX_PART_SIZE;
		pg_hdr->group_size = HTC_PG_DEF_GROUP_SIZE;
		pg_hdr->sector_size = HTC_PG_DEF_SECTOR_SIZE;

#ifdef eMMC_SQN
		if (emmc_erase(pg_sector, HTC_PG_DEF_TOTAL_SIZE / SECTOR_SIZE)) {
			PGERR("emmc_erase failed\r\n");
			free(pg_hdr);
			return PG_ERR_EMMC_ERASE;
		}
#endif
		rc = htc_pg_hdr_set(pg_sector, pg_hdr);
	} else if ((rc == PG_ERR_NONE) && pg_hdr->part_num) {
		/* check CRC of each partition */
		part_hdr_list = alloc_cache_page_aligned(HTC_PART_HEADER_LEN * pg_hdr->part_num);
		if (part_hdr_list == NULL) {
			PGERR("part_hdr_list allocate failed\r\n");
			free(pg_hdr);
			return PG_ERR_ALLOC;
		}

		rc = sd_read_sector((unsigned char *)part_hdr_list,
				pg_sector + HTC_PG_HEADER_LEN / SECTOR_SIZE,
				(HTC_PART_HEADER_LEN / SECTOR_SIZE) * pg_hdr->part_num);
		if (rc) {
			PGERR("sd_read_sector failed\r\n");
			free(pg_hdr);
			free(part_hdr_list);
			return PG_ERR_EMMC_ACCESS;
		}

		for (i = 0; i < pg_hdr->part_num; i++) {
			HLOGD("[partition %d] %s\r\n", i, part_hdr_list[i].name);
			crc = htc_pg_part_crc(pg_sector, part_hdr_list[i].name);
			if (crc != part_hdr_list[i].checksum)
				PGERR("pg %s: calculated checksum 0x%X is mismatched (header checksum 0x%X)\r\n",
					part_hdr_list[i].name, crc, part_hdr_list[i].checksum);
			else
				PGDBG("pg %s: calculated checksum 0x%X is matched \r\n",
					part_hdr_list[i].name, crc);
		}
		free(part_hdr_list);
	}

	free(pg_hdr);
	return rc;
}

int32 htc_pg_hdr_get(uint32 pg_sector, htc_pg_hdr *pg_hdr)
{
	int rc;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	if (pg_hdr == NULL) {
		PGERR("NULL pg_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	rc = sd_read_sector((unsigned char *)pg_hdr, pg_sector, HTC_PG_HEADER_LEN / SECTOR_SIZE);
	if (rc) {
		PGERR("sd_read_sector error\r\n");
		return PG_ERR_EMMC_ACCESS;
	}

	if (pg_hdr->magic != HTC_PG_HEADER_MAGIC)
		return PG_ERR_NO_PG_HDR;

	return PG_ERR_NONE;
}

int32 htc_pg_hdr_set(uint32 pg_sector, htc_pg_hdr *pg_hdr)
{
	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	if (pg_hdr == NULL) {
		PGERR("NULL pg_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	if (sd_write_sector((unsigned char *)pg_hdr, pg_sector, HTC_PG_HEADER_LEN / SECTOR_SIZE) == false)
		return PG_ERR_EMMC_ACCESS;
	else
		return PG_ERR_NONE;
}

int32 htc_pg_part_hdr_get(uint32 pg_sector, int8 *name, htc_part_hdr *part_hdr)
{
	uint32 i;
	int rc;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr_list = NULL;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	if (part_hdr == NULL) {
		PGERR("NULL part_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	pg_hdr = alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_hdr_get(pg_sector, pg_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		free(pg_hdr);
		return rc;
	}

	part_hdr_list = alloc_cache_page_aligned(HTC_PART_HEADER_LEN * pg_hdr->part_num);
	if (part_hdr_list == NULL) {
		PGERR("part_hdr_list allocate failed\r\n");
		free(pg_hdr);
		return PG_ERR_ALLOC;
	}

	rc = sd_read_sector((unsigned char *)part_hdr_list,
			pg_sector + HTC_PG_HEADER_LEN / SECTOR_SIZE,
			(HTC_PART_HEADER_LEN / SECTOR_SIZE) * pg_hdr->part_num);
	if (rc) {
		PGERR("sd_read_sector failed\r\n");
		free(pg_hdr);
		free(part_hdr_list);
		return PG_ERR_EMMC_ACCESS;
	}

	rc = PG_ERR_NOT_FOUND;
	for (i = 0; i < pg_hdr->part_num; i++)
		if (!strcmp((const char *)part_hdr_list[i].name, (const char *)name)) {
			memcpy(part_hdr, part_hdr_list + i, HTC_PART_HEADER_LEN);
			rc = PG_ERR_NONE;
			break;
		}

	free(pg_hdr);
	free(part_hdr_list);
	return rc;
}

static int32 htc_pg_alloc_map(htc_pg_hdr *pg_hdr, htc_part_hdr *part_hdr_list, uint8 **palloc_map)
{
	uint8 *alloc_map;
	htc_pg_link *pg_link;
	uint32 i, j, start, end, unit, max_link;

	if (pg_hdr == NULL) {
		PGERR("NULL pg_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	if (part_hdr_list == NULL) {
		PGERR("NULL part_hdr_list\r\n");
		return PG_ERR_PARAMETER;
	}

	if (palloc_map == NULL) {
		PGERR("NULL palloc_map\r\n");
		return PG_ERR_PARAMETER;
	}

	j = PG_DATA_SECTOR_COUNT(pg_hdr);
	alloc_map = alloc_cache_page_aligned(j);
	if (alloc_map == NULL) {
		PGERR("alloc_map allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	memset(alloc_map, 0, j);
	unit = pg_hdr->group_size / pg_hdr->sector_size;
	for (i = 0; i < pg_hdr->part_num; i++)
		if (part_hdr_list[i].attr & HTC_PG_ATTR_LINK) {
			max_link = sizeof(part_hdr_list[i].dl.link) / sizeof(htc_pg_link);
			if (part_hdr_list[i].link_num < max_link)
				max_link = part_hdr_list[i].link_num;
			pg_link = part_hdr_list[i].dl.link;
			for (j = 0; j < max_link; ) {
				switch (pg_link[j].SI) {
					case HTC_PG_LINK_GROUP_ALL:
						start = pg_link[j].GI * unit;
						end = start + unit;
						j++;
						break;
					case HTC_PG_LINK_RANGE_DEF:
						if ((j + 2) >= max_link) {
							PGERR("range definition is not enough, part %d, index %d\r\n", i, j);
							free(alloc_map);
							return PG_ERR_LINK;
						}
						start = pg_link[j + 1].GI * unit + pg_link[j + 1].SI;
						end = pg_link[j + 2].GI * unit + pg_link[j + 2].SI + 1;
						j += 3;
						break;
					default:
						if (pg_link[j].SI > HTC_PG_LINK_RANGE_DEF) {
							PGERR("invalid single link, part %d, index %d\r\n", i, j);
							free(alloc_map);
							return PG_ERR_LINK;
						}
						start = pg_link[j].GI * unit + pg_link[j].SI;
						end = start + 1;
						j++;
						break;
				}

				memset(alloc_map + start, 1, end - start);
			}
		}

	*palloc_map = alloc_map;
	return PG_ERR_NONE;
}

static int32 htc_pg_find_best_alloc(uint8 *alloc_map, uint32 total, uint32 req_num, uint32 *head, uint32 *tail, uint32 aligned_sector)
{
	uint32 i, j;
	uint32 start = 0, end = 0, prev_num = 0, num, found = 0;

	if ((alloc_map == NULL) || (head == NULL) || (tail == NULL))
		return PG_ERR_PARAMETER;

	/* sector number alignment? */
	if (aligned_sector && (req_num % aligned_sector))
		return PG_ERR_PARAMETER;

	PGDBG("total %d, req_num = %d, aligned_sector = %d\r\n", total, req_num, aligned_sector);
	for (i = 0; i < total; ) {
		/* sector number alignment */
		if (aligned_sector && (i % aligned_sector))
			i += aligned_sector - (i % aligned_sector);
		if (i >= total)
			break;

		if (!alloc_map[i]) {
			start = end = i;
			for (j = i + 1; j < total; j++)
				if (alloc_map[j]) {
					end = j - 1;
					break;
				}
			if (j >= total)
				end = total - 1;
			i = j + 1;

			num = end - start + 1;
			if (num) {
				if (found) {
					if (num < req_num) {
						if ((prev_num < req_num) && (num > prev_num)) {
							*head = start;
							*tail = end;
							prev_num = num;
						}
					} else {
						if ((prev_num < req_num) || (num < prev_num)) {
							*head = start;
							*tail = end;
							prev_num = num;
						}
					}
				} else {
					*head = start;
					*tail = end;
					prev_num = num;
					found = 1;
					break;
				}

				if (prev_num == req_num)
					break;
			}
		} else
			i++;
	}

	PGDBG("prev_num %d, head %d, tail %d\r\n", prev_num, *head, *tail);
	/* adjust allocation according to req_num*/
	if (prev_num > req_num) {
		prev_num = req_num;
		*tail = start + prev_num - 1;
		PGDBG("adjusted prev_num %d, head %d, tail %d\r\n", prev_num, *head, *tail);
	}
	if (prev_num > 0) {
		/* sector number alignment? */
		if (aligned_sector && (prev_num < req_num))
			return PG_ERR_FULL;
		/* mark allocation */
		memset(alloc_map + *head, 1, *tail - *head + 1);
		return PG_ERR_NONE;
	} else
		return PG_ERR_FULL;
}

static int32 htc_pg_alloc(htc_pg_hdr *pg_hdr, htc_part_hdr *part_hdr_list, uint32 index, uint32 size)
{
	htc_part_hdr *part_hdr;
	uint8 *alloc_map = NULL;
	uint32 i, num, max_link, total, start = 0, end = 0, gunit;
	uint32 used_sector = 0;
	int rc = PG_ERR_NONE;

	if ((rc = htc_pg_alloc_map(pg_hdr, part_hdr_list, &alloc_map)) != PG_ERR_NONE) {
		PGERR("htc_pg_alloc_map error\r\n");
		return rc;
	}

	PGDBG("htc_pg_alloc: pg %s, size %d\r\n", part_hdr_list[index].name, size);
	gunit = pg_hdr->group_size / pg_hdr->sector_size;
	part_hdr = part_hdr_list + index;
	total = PG_DATA_SECTOR_COUNT(pg_hdr);
	for (i = 0; i < total; i++)
		if (alloc_map[i])
			used_sector++;
	PGDBG("used_sector = %d\r\n", used_sector);

	num = size / pg_hdr->sector_size;
	if (num > (total - used_sector)) {
		PGERR("pg sectors are not enough: free %d, request %d\r\n",
			total - used_sector, num);
		rc = PG_ERR_FULL;
		goto pg_alloc_ret;
	}

	max_link = sizeof(part_hdr->dl.link) / sizeof(htc_pg_link);
	PGDBG("max_link = %d\r\n", max_link);
	for (i = part_hdr->link_num; num && ( (i+2) < max_link); ) {
		/* try group allocation first if the requestd size is equal to the group size */
		if (num == gunit) {
			rc = htc_pg_find_best_alloc(alloc_map, total, num, &start, &end, gunit);
			if (rc == PG_ERR_NONE) {
				part_hdr->dl.link[i].GI = start / gunit;
				part_hdr->dl.link[i].SI = HTC_PG_LINK_GROUP_ALL;
				PGDBG("pg %s, link %d, allocate num %d, group (%d, %d)\r\n",
					part_hdr->name, i, num,
					part_hdr->dl.link[i].GI, part_hdr->dl.link[i].SI);
				num = 0;
				i++;
				continue;
			}
		}

		rc = htc_pg_find_best_alloc(alloc_map, total, num, &start, &end, 0);
		if (rc != PG_ERR_NONE) {
			PGERR("pg %s, link %d, allocate %d failed\r\n", part_hdr->name, i, num);
			goto pg_alloc_ret;
		}

		part_hdr->dl.link[i].GI = 0;
		part_hdr->dl.link[i].SI = HTC_PG_LINK_RANGE_DEF;
		part_hdr->dl.link[i + 1].GI = start / gunit;
		part_hdr->dl.link[i + 1].SI = start % gunit;
		part_hdr->dl.link[i + 2].GI = end / gunit;
		part_hdr->dl.link[i + 2].SI = end % gunit;
		PGDBG("pg %s, link %d, allocate num %d, range (%d, %d) ~ (%d, %d)\r\n",
			part_hdr->name, i, num,
			part_hdr->dl.link[i + 1].GI, part_hdr->dl.link[i + 1].SI,
			part_hdr->dl.link[i + 2].GI, part_hdr->dl.link[i + 2].SI);

		num -= end - start + 1;
		i += 3;
	}

	part_hdr->link_num = i;
	PGDBG("link_num = %d\r\n", part_hdr->link_num);

pg_alloc_ret:
	free(alloc_map);
	return rc;
}

static int32 htc_pg_part_reduce_size(htc_pg_hdr *pg_hdr, htc_part_hdr *part_hdr, uint32 req_size)
{
	uint32 j, start, end, unit, count;
	htc_pg_link *pg_link;

	if (pg_hdr == NULL) {
		PGERR("NULL pg_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	if (part_hdr == NULL) {
		PGERR("NULL part_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	if (req_size >= part_hdr->size) {
		PGERR("req_size is larger\r\n");
		return PG_ERR_PARAMETER;
	}

	PGDBG("pg %s: reduce size to %d\r\n", part_hdr->name, req_size);
	pg_link = part_hdr->dl.link;
	unit = pg_hdr->group_size / pg_hdr->sector_size;
	req_size /= pg_hdr->sector_size;
	for (j = 0; req_size && (j < part_hdr->link_num); ) {
		switch (pg_link[j].SI) {
			case HTC_PG_LINK_GROUP_ALL:
				if (unit < req_size) {
					req_size -= unit;
					j++;
				} else if (unit == req_size) {
					PGDBG("group link %d is the last one\r\n", j);
					req_size = 0;
					j++;
				} else {
					PGDBG("modify group link %d from (%d, %d)\r\n",
						j, pg_link[j].GI, pg_link[j].SI);
					pg_link[j].SI = HTC_PG_LINK_RANGE_DEF;
					pg_link[j + 1].GI = pg_link[j].GI;
					pg_link[j + 1].SI = 0;
					pg_link[j + 2].GI = pg_link[j].GI;
					pg_link[j + 2].SI = req_size;
					pg_link[j].GI = 0;
					PGDBG("to range link (%d, %d) ~ (%d, %d)\r\n",
						pg_link[j + 1].GI, pg_link[j + 1].SI,
						pg_link[j + 2].GI, pg_link[j + 2].SI);
					req_size = 0;
					j += 3;
				}
				break;
			case HTC_PG_LINK_RANGE_DEF:
				if ((j + 2) >= part_hdr->link_num) {
					PGERR("range definition is not enough, index %d\r\n", j);
					return PG_ERR_LINK;
				}
				start = pg_link[j + 1].GI * unit + pg_link[j + 1].SI;
				end = pg_link[j + 2].GI * unit + pg_link[j + 2].SI;
				count = end - start + 1;
				if (count < req_size)
					req_size -= count;
				else if (count == req_size) {
					PGDBG("range link %d is the last one\r\n", j);
					req_size = 0;
				} else {
					PGDBG("modify range link %d, from (%d, %d) ~ (%d, %d)\r\n",
						j, pg_link[j + 1].GI, pg_link[j + 1].SI,
						pg_link[j + 2].GI, pg_link[j + 2].SI);
					end = start + req_size - 1;
					pg_link[j + 2].GI = end / unit;
					pg_link[j + 2].SI = end % unit;
					PGDBG("to range link (%d, %d) ~ (%d, %d)\r\n",
						pg_link[j + 1].GI, pg_link[j + 1].SI,
						pg_link[j + 2].GI, pg_link[j + 2].SI);
					req_size = 0;
				}
				j += 3;
				break;
			default:
				if (pg_link[j].SI > HTC_PG_LINK_RANGE_DEF) {
					PGERR("invalid single link, index %d\r\n", j);
					return PG_ERR_LINK;
				}
				if (req_size > 1)
					req_size--;
				else {
					PGDBG("single link %d is the last one\r\n", j);
					req_size = 0;
				}
				j++;
				break;
		}
	}

	if (req_size) {
		PGERR("link size is smaller\r\n");
		return PG_ERR_LINK;
	}

	part_hdr->link_num = j;
	memset(pg_link + j, 0, sizeof(part_hdr->dl) - j * sizeof(htc_pg_link));
	return PG_ERR_NONE;
}


int32 htc_pg_fix_part_hdr_add(uint32 pg_sector, int8 *name, uint32 sector_start, uint32 sector_size)
{
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr = NULL;
	htc_pg_link *pg_link;
	uint32 data_start, start;
	uint32 g_unit, s_unit;
	int rc;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	pg_hdr = alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	rc = htc_pg_hdr_get(pg_sector, pg_hdr);
	if (rc != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		free(pg_hdr);
		return rc;
	}

	part_hdr = alloc_cache_page_aligned(HTC_PART_HEADER_LEN);
	if (part_hdr == NULL) {
		PGERR("part_hdr allocate failed\r\n");
		free(pg_hdr);
		return PG_ERR_ALLOC;
	}

	memset(part_hdr, 0, HTC_PART_HEADER_LEN);
	pg_link = part_hdr->dl.link;
	strncpy(part_hdr->name, name, sizeof(part_hdr->name)-1);
	part_hdr->attr = HTC_PG_ATTR_LINK;

	g_unit = pg_hdr->group_size / SECTOR_SIZE;
	s_unit = pg_hdr->sector_size / SECTOR_SIZE;
	data_start = pg_sector +
			(HTC_PG_HEADER_LEN + HTC_PART_HEADER_LEN * pg_hdr->max_part_num) / SECTOR_SIZE;

	if (sector_start < data_start) {
		PGERR("sector start range error\r\n");
		free(pg_hdr);
		return PG_ERR_PARAMETER;
	}

	start = sector_start - data_start;

	pg_link[0].SI = HTC_PG_LINK_RANGE_DEF;
	pg_link[1].GI = (uint8)(start / g_unit);
	pg_link[1].SI = (uint8)((start % g_unit) / s_unit);
	pg_link[2].GI = (uint8)((start + sector_size - 1) / g_unit);
	pg_link[2].SI = (uint8)(((start + sector_size - 1) % g_unit) / s_unit);
	part_hdr->link_num = 3;
	part_hdr->size = sector_size * SECTOR_SIZE;

	PGDBG("[link] = (0x%02x, 0x%02x) ~ (0x%02x, 0x%02x)\r\n",
						pg_link[1].GI, pg_link[1].SI,
						pg_link[2].GI, pg_link[2].SI);

	rc = htc_pg_part_hdr_set(pg_sector, name, part_hdr, 0);

	free(pg_hdr);
	free(part_hdr);

	return rc;
}

int32 htc_pg_part_hdr_set(uint32 pg_sector, int8 *name, htc_part_hdr *part_hdr, uint32 pg_realloc)
{
	uint32 i;
	int rc, new_part = 0, fix_part = 0;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr_list = NULL;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	if (part_hdr == NULL) {
		PGERR("NULL part_hdr\r\n");
		return PG_ERR_PARAMETER;
	}

	pg_hdr = alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_hdr_get(pg_sector, pg_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		free(pg_hdr);
		return rc;
	}

	if (!(part_hdr->attr & HTC_PG_ATTR_LINK) && (part_hdr->size > sizeof(part_hdr->dl))) {
		PGERR("no HTC_PG_ATTR_LINK, and part_hdr->size is too large\r\n");
		free(pg_hdr);
		return PG_ERR_PARAMETER;
	}

	if (part_hdr->size > pg_hdr->max_part_size) {
		PGERR("part_hdr->size is too large\r\n");
		free(pg_hdr);
		return PG_ERR_PARAMETER;
	}

	part_hdr_list = alloc_cache_page_aligned(HTC_PART_HEADER_LEN * pg_hdr->max_part_num);
	if (part_hdr_list == NULL) {
		PGERR("part_hdr_list allocate failed\r\n");
		free(pg_hdr);
		return PG_ERR_ALLOC;
	}

	rc = sd_read_sector((unsigned char *)part_hdr_list,
			pg_sector + HTC_PG_HEADER_LEN / SECTOR_SIZE,
			(HTC_PART_HEADER_LEN / SECTOR_SIZE) * pg_hdr->max_part_num);
	if (rc) {
		PGERR("sd_read_sector failed\r\n");
		free(pg_hdr);
		free(part_hdr_list);
		return PG_ERR_EMMC_ACCESS;
	}

	for (i = 0; i < pg_hdr->part_num; i++)
		if (!strcmp((const char *)part_hdr_list[i].name, (const char *)name)) {
			if (part_hdr_list[i].attr != part_hdr->attr) {
				PGDBG("attr mismatched: old (0x%X), new (0x%X)\r\n",
					part_hdr_list[i].attr, part_hdr->attr);
				if (part_hdr->attr & HTC_PG_ATTR_LINK)
					pg_realloc = 1;
				else {
					if (part_hdr->size > sizeof(part_hdr->dl)) {
							PGERR("pg direct data size error\r\n");
							free(pg_hdr);
							free(part_hdr_list);
							return PG_ERR_PARAMETER;
					}
				}
				part_hdr->link_num = 0;
				memset(part_hdr->dl.link, 0, sizeof(part_hdr->dl));
			} else if (part_hdr->attr & HTC_PG_ATTR_LINK) {
				if (part_hdr_list[i].size > part_hdr->size) {
					PGDBG("reduce pg %s size: from %d to %d\r\n",
						name, part_hdr_list[i].size, part_hdr->size);
					/* reduce pg partition size */
					if (!pg_realloc) {
						rc = htc_pg_part_reduce_size(pg_hdr, part_hdr_list + i, part_hdr->size);
						if (rc != PG_ERR_NONE) {
							PGERR("htc_pg_part_reduce_size error\r\n");
							free(pg_hdr);
							free(part_hdr_list);
							return rc;
						}
						/* update link */
						part_hdr->link_num = part_hdr_list[i].link_num;
						memcpy(part_hdr->dl.link, part_hdr_list[i].dl.link,
							sizeof(part_hdr->dl));
					}
				} else if (part_hdr_list[i].size < part_hdr->size) {
					PGDBG("increase pg %s size: from %d to %d\r\n",
						name, part_hdr_list[i].size, part_hdr->size);
					/* increase pg partition size */
					if (!pg_realloc) {
						rc = htc_pg_alloc(pg_hdr, part_hdr_list, i,
								part_hdr->size - part_hdr_list[i].size);
						if (rc != PG_ERR_NONE) {
							PGERR("htc_pg_alloc error\r\n");
							free(pg_hdr);
							free(part_hdr_list);
							return rc;
						}

						/* update link */
						part_hdr->link_num = part_hdr_list[i].link_num;
						memcpy(part_hdr->dl.link, part_hdr_list[i].dl.link,
								sizeof(part_hdr->dl));
					}
				}
			}

			if (part_hdr_list[i].checksum != part_hdr->checksum)
				PGDBG("checksum mismatched: old (0x%X), new (0x%X)\r\n",
					part_hdr_list[i].checksum, part_hdr->checksum);

			memcpy(part_hdr_list + i, part_hdr, HTC_PART_HEADER_LEN);
			break;
		}

	if (i >= pg_hdr->part_num) {
		if (i >= pg_hdr->max_part_num) {
			PGERR("pg_hdr->part_num should not exceed the maximum number\r\n");
			free(pg_hdr);
			free(part_hdr_list);
			return PG_ERR_FULL;
		} else {
			if (part_hdr->link_num != 0) {
				fix_part = 1;
				memcpy(part_hdr_list + i, part_hdr, HTC_PART_HEADER_LEN);
				pg_hdr->part_num++;
				PGDBG("fixed pg part %s is generated, index %d\r\n", part_hdr->name, i);
			} else {
				new_part = 1;
				memcpy(part_hdr_list + i, part_hdr, HTC_PART_HEADER_LEN);
				memset(&(part_hdr_list[i].dl), 0, sizeof(part_hdr_list[i].dl));
				pg_hdr->part_num++;
				PGDBG("new pg part %s is generated, index %d\r\n", part_hdr->name, i);
			}
		}
	} else if (pg_realloc) {
		part_hdr_list[i].link_num = 0;
		memset(&(part_hdr_list[i].dl), 0, sizeof(part_hdr_list[i].dl));
	}

	if (new_part || pg_realloc) {
		/* allocate the sectors for the new partition */
		rc = htc_pg_alloc(pg_hdr, part_hdr_list, i, part_hdr->size);
		if (rc != PG_ERR_NONE) {
			PGERR("htc_pg_alloc error\r\n");
			free(pg_hdr);
			free(part_hdr_list);
			return rc;
		}
	}

	rc = sd_write_sector((unsigned char *)(part_hdr_list + i),
			pg_sector + (HTC_PG_HEADER_LEN / SECTOR_SIZE) + (HTC_PART_HEADER_LEN / SECTOR_SIZE) * i,
			(HTC_PART_HEADER_LEN / SECTOR_SIZE));

	if (rc == false) {
		PGERR("sd_write_sector failed\r\n");
		free(pg_hdr);
		free(part_hdr_list);
		return PG_ERR_EMMC_ACCESS;
	} else
		rc = PG_ERR_NONE;

	if (new_part | fix_part)
		rc = htc_pg_hdr_set(pg_sector, pg_hdr);

	free(pg_hdr);
	free(part_hdr_list);
	return rc;
}

static int32 htc_pg_part_traverse(uint32 pg_sector, int8 *name, uint32 offset, void *buf, int len, htc_pg_traverse_fn fn, uint32 argu, uint32 *ret, int32 check_ret)
{
	htc_pg_hdr *pg_hdr;
	htc_part_hdr *part_hdr;
	htc_pg_link *pg_link;
	uint32 link1, link2;
	uint32 max_link, data_start, start, size, count;
	uint32 i, rc = PG_ERR_NONE;

	PGDBG("PG %s, sector %d, offset %d, len %d, check_ret %d\r\n", name,
			pg_sector, offset, len, check_ret);

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	if (buf == NULL) {
		PGDBG("NULL buf pointer\r\n");
	}

	if ((fn != NULL) && (ret == NULL)) {
		PGDBG("NULL ret pointer\r\n");
		return PG_ERR_PARAMETER;
	}

	pg_hdr = (htc_pg_hdr *)alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_hdr_get(pg_sector, pg_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		goto free_pg_hdr;
	}

	part_hdr = (htc_part_hdr *)alloc_cache_page_aligned(HTC_PART_HEADER_LEN);
	if (part_hdr == NULL) {
		PGERR("part_hdr allocate failed\r\n");
		rc = PG_ERR_ALLOC;
		goto free_pg_hdr;
	}

	if ((rc = htc_pg_part_hdr_get(pg_sector, name, part_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_part_hdr_get failed\r\n");
		goto free_part_hdr;
	}

	if ((offset + len) > part_hdr->size) {
		PGERR("invalid traverse range\r\n");
		rc = PG_ERR_PARAMETER;
		goto free_part_hdr;
	}

	if (part_hdr->attr & HTC_PG_ATTR_LINK) {
		pg_link = part_hdr->dl.link;
		max_link = sizeof(part_hdr->dl.link) / sizeof(htc_pg_link);
		if (part_hdr->link_num < max_link)
			max_link = part_hdr->link_num;
		data_start = pg_sector +
			(HTC_PG_HEADER_LEN + HTC_PART_HEADER_LEN * pg_hdr->max_part_num) / SECTOR_SIZE;
		for (i = 0; (i < max_link) && (len > 0); len -= (count * SECTOR_SIZE)) {
			start = data_start;
			count = 0;
			PGDBG("[%d] len = %d\r\n", i, len);
			switch (pg_link[i].SI) {
				case HTC_PG_LINK_GROUP_ALL:
					PGDBG("  group %d\r\n", pg_link[i].GI);
					if (pg_hdr->group_size > offset) {
						start += pg_link[i].GI * (pg_hdr->group_size / SECTOR_SIZE) +
									offset / SECTOR_SIZE;
						count = (pg_hdr->group_size - offset) / SECTOR_SIZE;
						offset = 0;
					} else
						offset -= pg_hdr->group_size;
					i++;
					break;
				case HTC_PG_LINK_RANGE_DEF:
					if ((i + 2) >= max_link) {
						PGERR("range definition is not enough, index %d\r\n", i);
						rc = PG_ERR_LINK;
						goto free_part_hdr;
					}

					PGDBG("  range (%d, %d) ~ (%d, %d)\r\n",
							pg_link[i + 1].GI, pg_link[i + 1].SI,
							pg_link[i + 2].GI, pg_link[i + 2].SI
							);
					link1 = start +
								pg_link[i + 1].GI * (pg_hdr->group_size / SECTOR_SIZE) +
								pg_link[i + 1].SI * (pg_hdr->sector_size / SECTOR_SIZE);
					link2 = start +
								pg_link[i + 2].GI * (pg_hdr->group_size / SECTOR_SIZE) +
								pg_link[i + 2].SI * (pg_hdr->sector_size / SECTOR_SIZE);
					size = link2 + (pg_hdr->sector_size / SECTOR_SIZE) - link1;

					if ((size * SECTOR_SIZE) > offset) {
						start = link1 + offset / SECTOR_SIZE;
						count = size - offset / SECTOR_SIZE;
						offset = 0;
					} else
						offset -= size * SECTOR_SIZE;
					i += 3;
					break;
				default:
					PGDBG("  sector (%d, %d)\r\n",
							pg_link[i].GI, pg_link[i].SI
							);
					if (pg_link[i].SI > HTC_PG_LINK_RANGE_DEF) {
						PGERR("invalid single link, index %d\r\n", i);
						rc = PG_ERR_LINK;
						goto free_part_hdr;
					}
					if (pg_hdr->sector_size > offset) {
						start += pg_link[i].GI * (pg_hdr->group_size / SECTOR_SIZE) +
									pg_link[i].SI * (pg_hdr->sector_size / SECTOR_SIZE) +
									offset / SECTOR_SIZE;
						count = (pg_hdr->sector_size - offset) / SECTOR_SIZE;
						offset = 0;
					} else
						offset -= pg_hdr->sector_size;
					i++;
					break;
			}

			if (count) {
				PGDBG("read sector %d, count %d\r\n", start, count);
				if (len < (int)(count * SECTOR_SIZE))
					count = (len + SECTOR_SIZE - 1) / SECTOR_SIZE;
				if (buf) {
					rc = sd_read_sector(buf, start, count);
					if (rc) {
						PGERR("sd_read_sector error\r\n");
						rc = PG_ERR_EMMC_ACCESS;
						break;
					}
				}
				if (fn) {
					*ret = fn(start, count, (uint32)buf, count * SECTOR_SIZE, argu);
					if (check_ret && (*ret != PG_ERR_NONE)) {
						PGERR("fn return error\r\n");
						rc = PG_ERR_FN_RET;
						break;
					}
				}
				if (buf)
					buf = (uint8 *)buf + count * SECTOR_SIZE;
			}
		}
	} else {
		PGDBG("no link, len = %d\r\n", part_hdr->size);
		if (buf)
			memcpy(buf, part_hdr->dl.data + offset, len);
		if (fn) {
			*ret = fn(0, 0, (uint32)(part_hdr->dl.data + offset), len, argu);
			if (check_ret && (*ret != PG_ERR_NONE)) {
				PGERR("fn return error\r\n");
				rc = PG_ERR_FN_RET;
			}
		}
	}

free_part_hdr:
	free(part_hdr);
free_pg_hdr:
	free(pg_hdr);
	return rc;
}

static uint32 htc_pg_link_size(uint32 sector, uint32 count, uint32 addr, uint32 len, uint32 argu)
{
	uint32 size;

	(void)sector;
	(void)count;
	(void)addr;

	size = *((uint32 *)argu) + len;
	PGDBG("htc_pg_link_size: %d\r\n", size);
	return size;
}

static uint32 htc_pg_part_update(uint32 sector, uint32 count, uint32 addr, uint32 len, uint32 argu)
{
	uint8 *buf = *((uint8 **)argu);
	int rc;

	(void)addr;

	PGDBG("htc_pg_part_update: buf 0x%X, sector %d, count %d\r\n", (unsigned int)buf, sector, count);
	rc = sd_write_sector(buf, sector, count);
	if (rc == false) {
		PGERR("sd_write_sector failed\r\n");
		return PG_ERR_EMMC_ACCESS;
	}

	/* update buffer pointer */
	*((uint8 **)argu) = buf + len;
	return PG_ERR_NONE;
}

static uint32 htc_pg_part_clear(uint32 sector, uint32 count, uint32 addr, uint32 len, uint32 argu)
{
#ifdef eMMC_SQN
	int rc;
#endif

	(void)addr;
	(void)len;
	(void)argu;

	PGDBG("htc_pg_part_clear: sector %d, count %d\r\n", sector, count);
#ifdef eMMC_SQN
	rc = emmc_erase(sector, count);
	if (rc) {
		PGERR("emmc_erase failed\r\n");
		return PG_ERR_EMMC_ERASE;
	}
#endif

	return PG_ERR_NONE;
}

int32 htc_pg_part_read(uint32 pg_sector, int8 *name, uint32 offset, void *buf, uint32 len)
{
	uint32 size = 0, rc;
	char *buf2;
	uint32 sec, ofs;

	sec = offset / SECTOR_SIZE;
	ofs = offset % SECTOR_SIZE;
	buf2 = (char *)alloc_page_aligned(len + SECTOR_SIZE);
	if (buf2 == NULL) {
		PGERR("pgfs read buffer allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

    /* clean the memory before using it */
    memset(buf2 + ofs, 0, len);

    /* buf2 will remains the same if there is nothing to read */
    /* thus, we clean the memory first, and return the result if successed */
	rc = htc_pg_part_traverse(pg_sector, name, sec * SECTOR_SIZE, buf2,
			len + ofs, htc_pg_link_size, (uint32)&size, &size, 0);

    /* won't copy out to buf if traverse failed */
	if (rc) {
		PGERR("htc_pg_part_traverse failed\r\n");
        free(buf2);
		return rc;
	} else {
        memcpy(buf, buf2 + ofs, len);
        free(buf2);
        return size;
    }

}


int32 htc_pg_update_crc(uint32 pg_sector, int8 *name)
{
	int rc;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr = NULL;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	pg_hdr = (htc_pg_hdr *)alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	rc = htc_pg_hdr_get(pg_sector, pg_hdr);
	if (rc != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		free(pg_hdr);
		return rc;
	}

	part_hdr = (htc_part_hdr *)alloc_cache_page_aligned(HTC_PART_HEADER_LEN);
	if (part_hdr == NULL) {
		PGERR("part_hdr allocate failed\r\n");
		free(pg_hdr);
		return PG_ERR_ALLOC;
	}

	rc = htc_pg_part_hdr_get(pg_sector, name, part_hdr);
	if (rc != PG_ERR_NONE) {
		PGERR("htc_pg_part_hdr_get failed\r\n");
		free(pg_hdr);
		free(part_hdr);
		return rc;
	}

	part_hdr->checksum = htc_pg_part_crc(pg_sector, part_hdr->name);
	rc = htc_pg_part_hdr_set(pg_sector, name, part_hdr, 0);

	free(pg_hdr);
	free(part_hdr);
	return rc;


}

static int32 htc_pg_part_modify(uint32 pg_sector, int8 *name, uint32 offset, void *buf, uint32 len, uint32 is_erase, uint32 update_crc)
{
	int rc;
	uint32 ret;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr = NULL;

	PGDBG("htc_pg_part_modify: part %s, offset %d, len %d, is_erase %d, update_crc %d\r\n",
		name, offset, len, is_erase, update_crc);
	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	pg_hdr = (htc_pg_hdr *)alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_hdr_get(pg_sector, pg_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		free(pg_hdr);
		return rc;
	}

	part_hdr = (htc_part_hdr *)alloc_cache_page_aligned(HTC_PART_HEADER_LEN);
	if (part_hdr == NULL) {
		PGERR("part_hdr allocate failed\r\n");
		free(pg_hdr);
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_part_hdr_get(pg_sector, name, part_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_part_hdr_get failed\r\n");
		free(pg_hdr);
		free(part_hdr);
		return rc;
	}

	if (is_erase) {
		offset = 0;
		len = part_hdr->size;
	}

	if (part_hdr->attr & HTC_PG_ATTR_LINK) {
		if ((offset + len) > part_hdr->size) {
			PGERR("Partition address is out of range, offset:0x%x, len:%d, part_hdr->size:%d \r\n", offset, len, part_hdr->size);
			rc = PG_ERR_PARAMETER;
		} else {
			if (is_erase)
				rc = htc_pg_part_traverse(pg_sector, name, offset, NULL, len,
						htc_pg_part_clear,
						0, &ret, 1);
			else
				rc = htc_pg_part_traverse(pg_sector, name, offset, NULL, len,
						htc_pg_part_update,
						(uint32)&buf, &ret, 1);
			/* update checksum */
			if ((rc == PG_ERR_NONE) && update_crc) {
				part_hdr->checksum = htc_pg_part_crc(pg_sector, part_hdr->name);
				rc = htc_pg_part_hdr_set(pg_sector, name, part_hdr, 0);
			}
			if (rc == PG_ERR_NONE)
				rc = len;
		}
	} else {
		if ((offset + len) > sizeof(part_hdr->dl)) {
			PGERR("Partition address is out of range, offset:0x%x, len:%d, sizeof(part_hdr->dl):%d \r\n", offset, len, sizeof(part_hdr->dl));
			rc = PG_ERR_PARAMETER;
		} else {
			if (is_erase)
				memset(part_hdr->dl.data + offset, 0, len);
			else
				memcpy(part_hdr->dl.data + offset, buf, len);
			rc = htc_pg_part_hdr_set(pg_sector, name, part_hdr, 0);
			/* update checksum */
			if ((rc == PG_ERR_NONE) && update_crc) {
				part_hdr->checksum = htc_pg_part_crc(pg_sector, part_hdr->name);
				rc = htc_pg_part_hdr_set(pg_sector, name, part_hdr, 0);
			}
			if (rc == PG_ERR_NONE)
				rc = len;
		}
	}

	free(pg_hdr);
	free(part_hdr);
	return rc;
}

int32 htc_pg_part_write(uint32 pg_sector, int8 *name, uint32 offset, void *buf, uint32 len, uint32 update_crc)
{
	return htc_pg_part_modify(pg_sector, name, offset, buf, len, 0, update_crc);
}

int32 htc_pg_part_erase(uint32 pg_sector, int8 *name, uint32 update_crc)
{
	return htc_pg_part_modify(pg_sector, name, 0, NULL, 0, 1, update_crc);
}

int32 htc_pg_free_size(uint32 pg_sector)
{
	uint32 i, total, total2, size = 0;
	int rc = PG_ERR_NONE;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr_list = NULL;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	pg_hdr = (htc_pg_hdr *)alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
	if (pg_hdr == NULL) {
		PGERR("pg_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_hdr_get(pg_sector, pg_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_hdr_get failed\r\n");
		free(pg_hdr);
		return rc;
	}

	part_hdr_list = (htc_part_hdr *)alloc_cache_page_aligned(HTC_PART_HEADER_LEN * pg_hdr->max_part_num);
	if (part_hdr_list == NULL) {
		PGERR("part_hdr_list allocate failed\r\n");
		free(pg_hdr);
		return PG_ERR_ALLOC;
	}

	rc = sd_read_sector((unsigned char *)part_hdr_list,
			pg_sector + HTC_PG_HEADER_LEN / SECTOR_SIZE,
			(HTC_PART_HEADER_LEN / SECTOR_SIZE) * pg_hdr->part_num);
	if (rc) {
		PGERR("sd_read_sector failed\r\n");
		free(pg_hdr);
		free(part_hdr_list);
		return PG_ERR_EMMC_ACCESS;
	}

	total = total2 = PG_DATA_SIZE(pg_hdr);
	PGDBG("pgfs total size %d\r\n", total);
	for (i = 0; i < pg_hdr->part_num; i++)
		if (part_hdr_list[i].attr & HTC_PG_ATTR_LINK)
			total -= part_hdr_list[i].size;

	for (i = 0; i < pg_hdr->part_num; i++) {
		if (part_hdr_list[i].attr & HTC_PG_ATTR_LINK) {
			rc = htc_pg_part_traverse(pg_sector, part_hdr_list[i].name, 0, NULL,
					part_hdr_list[i].size, htc_pg_link_size, (uint32)&size, &size, 0);
			if (rc) {
				PGERR("htc_pg_part_traverse failed\r\n");
				break;
			}
		}

		PGDBG("[%d] pgfs accumulated size %d\r\n", i, size);
	}
	if (rc == PG_ERR_NONE) {
		total2 -= size;
		if (total2 != total) {
			PGERR("free partition size mismatched\r\n");
			rc = PG_ERR_LINK;
		}
	}

	free(pg_hdr);
	free(part_hdr_list);
	PGDBG("total %d, total2 %d\r\n", total, total2);
	return total2;
}

static uint32 htc_pg_crc32(uint32 sector, uint32 count, uint32 addr, uint32 len, uint32 argu)
{
	(void)sector;
	(void)count;

	return Crc32CheckSum(addr, len, *((uint32 *)argu));
}

int32 htc_pg_part_crc(uint32 pg_sector, int8 *name)
{
	htc_part_hdr *part_hdr;
	uint8 *pg_buf;
	int32 rc;
	uint32 crc = 0, size;

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	part_hdr = (htc_part_hdr *)alloc_cache_page_aligned(HTC_PART_HEADER_LEN);
	if (part_hdr == NULL) {
		PGERR("part_hdr allocate failed\r\n");
		return PG_ERR_ALLOC;
	}

	if ((rc = htc_pg_part_hdr_get(pg_sector, name, part_hdr)) != PG_ERR_NONE) {
		PGERR("htc_pg_part_hdr_get failed\r\n");
		free(part_hdr);
		return rc;
	}

	size = part_hdr->size;

	pg_buf = (uint8 *)alloc_cache_page_aligned(size);
	if (pg_buf == NULL) {
		PGERR("pg_buf allocate failed\r\n");
		free(part_hdr);
		return PG_ERR_ALLOC;
	}

	rc = htc_pg_part_traverse(pg_sector, name, 0, pg_buf, size, htc_pg_crc32, (uint32)&crc, &crc, 0);
	PGDBG("[partition] checksum 0x%X\r\n", crc);
	part_hdr->checksum = 0;
	if (part_hdr->attr & HTC_PG_ATTR_LINK)
		size = (uint32)(&(part_hdr->dl.link[part_hdr->link_num])) - (uint32)part_hdr;
	else
		size = (uint32)(&(part_hdr->dl.data[0])) - (uint32)part_hdr;
	crc = Crc32CheckSum((uint32)part_hdr, size, crc);
	PGDBG("[partition + header] checksum 0x%X (header size %d)\r\n", crc, size);
	free(part_hdr);
	free(pg_buf);
	return crc;
}


#ifndef HTC_PG_TOOL

#ifdef QCT_PLATFORM
	extern struct partition_t *partition_tb;
	extern pgfs_partition_t *pgfs_partition[];
#else
	pgfs_partition_t *pgfs_partition[] = {
		pg1fs_partition,
		pg2fs_partition,
		NULL
	};

	struct pg_partition_t {
		char name[PART_META_INFO_NAMELEN];
		int start_block;
		int size;
		int flag;
	};

	extern part_t *partition;
#endif

int32 htc_check_pgfs(void)
{
#ifdef QCT_PLATFORM
	struct partition_t *p;
#else
	struct pg_partition_t pg_partition = {{0}, 0};
	struct pg_partition_t *p = &pg_partition;
	
#endif
	int rc, idx, g, pg_sector;
	htc_pg_hdr *pg_hdr = NULL;
	htc_part_hdr *part_hdr = NULL;
	int build_new;
	pgfs_partition_t *part;

	dprintf(CRITICAL, "zzytest, htc_check_pgfs begin\n");
#ifdef QCT_PLATFORM
	for (p = &partition_tb[0]; p->name != 0; p++) {
		if (p->format != DF_PGFS)
			continue;
#else
	part_t *tmp_partition = NULL;;
	for (tmp_partition=partition; tmp_partition->nr_sects; tmp_partition++) 
	{
		if (tmp_partition->info)
		{
			if (strncmp((char *)tmp_partition->info->name, PGFS_PREFIX, strlen(PGFS_PREFIX)))  // must co-work with emmc: only pg*fs partition had prefix "pg"
			{
				continue;
		 	}
			memset(p, 0, sizeof(struct pg_partition_t));
			p->start_block = (u32)tmp_partition->start_sect;
			p->size = tmp_partition->nr_sects;
			p->flag = 0;
			strncpy(p->name, (char *)tmp_partition->info->name, PART_META_INFO_NAMELEN);
#endif
		HLOGD("check %s\r\n", p->name);
		build_new = 0;
		pg_sector = p->start_block;
		pg_hdr = (htc_pg_hdr *)alloc_cache_page_aligned(HTC_PG_HEADER_LEN);
		if (pg_hdr == NULL) {
			PGERR("pg_hdr allocate failed\r\n");
			return PG_ERR_ALLOC;
		}

		rc = htc_pg_hdr_get(pg_sector, pg_hdr);
		if ((rc != PG_ERR_NONE) && (rc != PG_ERR_NO_PG_HDR)) {
			PGERR("htc_pg_hdr_get failed\r\n");
			free(pg_hdr);
			return rc;
		}

		if (pg_hdr->magic != HTC_PG_HEADER_MAGIC) {
			/* format pgfs  */
			memset(pg_hdr, 0, HTC_PG_HEADER_LEN);

			if (p->flag & PTBL_PGFS_OUTER) {
				pg_hdr->total_size = HTC_PG_DEF_TOTAL_SIZE;
				pg_hdr->sector_size = HTC_PG_DEF_OUTER_SECTOR_SIZE;
			} else {
				pg_hdr->total_size = p->size * SECTOR_SIZE;
				pg_hdr->sector_size = HTC_PG_DEF_SECTOR_SIZE;
			}
			pg_hdr->magic = HTC_PG_HEADER_MAGIC;
			pg_hdr->max_part_num = HTC_PG_DEF_MAX_PART_NUM;
			pg_hdr->max_part_size = HTC_PG_DEF_MAX_PART_SIZE;
			pg_hdr->group_size = HTC_PG_DEF_GROUP_SIZE;

			HLOGD("%s init %d %d %d\r\n", p->name, pg_sector,
											pg_hdr->total_size, pg_hdr->sector_size);

			rc = htc_pg_hdr_set(pg_sector, pg_hdr);
			if (rc != PG_ERR_NONE) {
				free(pg_hdr);
				return rc;
			}
			build_new = 1;
		} else {
			if (pg_hdr->part_num == 0)
				build_new = 1;
		}

		/* check partition */
		part_hdr = (htc_part_hdr *)alloc_cache_page_aligned(HTC_PART_HEADER_LEN);
		if (part_hdr == NULL) {
			PGERR("pg_hdr allocate failed\r\n");
			free(pg_hdr);
			return PG_ERR_ALLOC;
		}
		idx = (int)(p->name[2] - '1');
		part = pgfs_partition[idx];
		g = 0;
		while (part[g].name != NULL) {
			if (!build_new)
				rc = htc_pg_part_hdr_get(pg_sector, (int8 *)part[g].name, part_hdr);
			else
				rc = PG_ERR_NOT_FOUND;

			if (rc == PG_ERR_NOT_FOUND) {
				if (part[g].size > 0) {
					memset(part_hdr, 0, HTC_PART_HEADER_LEN);
					HLOGD("%s add %s %d\r\n", p->name, part[g].name, part[g].size);
					strncpy(part_hdr->name, part[g].name, sizeof(part_hdr->name)-1);
					part_hdr->size = part[g].size;
					part_hdr->attr = HTC_PG_ATTR_LINK;
					rc = htc_pg_part_hdr_set(pg_sector, (int8 *)part[g].name, part_hdr, 0);
				} else {
					if (p->start_block > 0) {
						HLOGD("%s add %s %d %d\r\n", p->name, part[g].name,
													partition_start_block(part[g].name),
													partition_size(part[g].name));
						rc = htc_pg_fix_part_hdr_add(pg_sector, (int8 *)part[g].name,
													partition_start_block(part[g].name),
													partition_size(part[g].name));
					}
				}
			} else {
				/* check pgfs size  */
				if ((part[g].size > 0) && (part_hdr->size > 0) && (part[g].size != (int)part_hdr->size)) {
					HLOGD("%s_%s change size from %d to %d\r\n", p->name, part[g].name, part_hdr->size, part[g].size);
					memset(part_hdr, 0, HTC_PART_HEADER_LEN);
					strncpy(part_hdr->name, part[g].name, sizeof(part_hdr->name)-1);
					part_hdr->size = part[g].size;
					part_hdr->attr = HTC_PG_ATTR_LINK;
					rc = htc_pg_part_hdr_set(pg_sector, (int8 *)part[g].name, part_hdr, 0);
				}
			}
			if (rc != PG_ERR_NONE) {
				PGERR("%s add %s error\r\n", p->name, part[g].name);
				#if 0
				free(pg_hdr);
				free(part_hdr);
				return rc;
				#endif
			}
			g++;
		}

		free(pg_hdr);
		free(part_hdr);
	}
#ifdef QCT_PLATFORM
#else
	}
#endif

#if SUPPORT_HTC_TZ
	if (secure_get_simlock_magic() == SIMLOCK_MAGIC_2) {
		partition_format_pgfs("pg1fs_simlock");
		partition_format_pgfs("pg1fs_simunlock");
	}
#endif

	return 0;
}



#endif

int32 htc_pg_process(int argc, char *argv[])
{
	int32 rc;
	uint32 i;
	htc_pg_hdr pg_hdr;
	htc_part_hdr part_hdr;
#ifdef HTC_PG_TOOL
	uint32 len;
	FILE *fp;
	uint8 *dbuf;
#endif

	if (emmc_init()) {
		PGERR("EMMC Init eror\r\n");
		return PG_ERR_EMMC_INIT;
	}

	if (argc <= 1) {
		HLOGD("command format:\r\n");
		HLOGD("\t%s check [start sector] [clean flag: 0 | 1]\r\n", argv[0]);
		HLOGD("\t%s init [start sector] [total] [max part num] [max part size] [part num] [group size] [sector size]\r\n", argv[0]);
		HLOGD("\t%s info [start sector]\r\n", argv[0]);
		HLOGD("\t%s add [start sector] [name] [realloc flag: 0 | 1] [size] [attr]\r\n", argv[0]);
		HLOGD("\t%s addfix [start sector] [name] [sector start] [size]\r\n", argv[0]);
		HLOGD("\t%s erase [start sector] [name]\r\n", argv[0]);
		HLOGD("\t%s get [start sector] [name]\r\n", argv[0]);
		HLOGD("\t%s free [start sector]\r\n", argv[0]);
		HLOGD("\t%s crc [start sector] [name]\r\n", argv[0]);
		HLOGD("\t%s update_crc [start sector] [name]\r\n", argv[0]);
#if HTC_PGFS_DEBUG
		HLOGD("\t%s dump [name] [offset] [size]\r\n", argv[0]);
#endif
#ifdef HTC_PG_TOOL
		HLOGD("\t%s part_write [start sector] [name] [file name]\r\n", argv[0]);
		HLOGD("\t%s part_read [start sector] [name] [file name]\r\n", argv[0]);
#endif
	} else {
		if (!strcmp(argv[1], "check")) {
			rc = htc_pg_sanity_check(atoi(argv[2]), atoi(argv[3]));
			HLOGD("ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "init")) {
			memset(&pg_hdr, 0, sizeof(pg_hdr));
			pg_hdr.magic = HTC_PG_HEADER_MAGIC;
			sscanf(argv[3], "%X", &(pg_hdr.total_size));
			sscanf(argv[4], "%d", &(pg_hdr.max_part_num));
			sscanf(argv[5], "%X", &(pg_hdr.max_part_size));
			sscanf(argv[6], "%d", &(pg_hdr.part_num));
			sscanf(argv[7], "%X", &(pg_hdr.group_size));
			sscanf(argv[8], "%X", &(pg_hdr.sector_size));
			rc = htc_pg_hdr_set(atoi(argv[2]), &pg_hdr);
			HLOGD("ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "info")) {
			rc = htc_pg_hdr_get(atoi(argv[2]), &pg_hdr);
			HLOGD("magic = 0x%X\r\n", pg_hdr.magic);
			HLOGD("total_size = 0x%X\r\n", pg_hdr.total_size);
			HLOGD("max_part_num = %d\r\n", pg_hdr.max_part_num);
			HLOGD("max_part_size = 0x%X\r\n", pg_hdr.max_part_size);
			HLOGD("part_num = %d\r\n", pg_hdr.part_num);
			HLOGD("group_size = 0x%X\r\n", pg_hdr.group_size);
			HLOGD("sector_size = 0x%X\r\n", pg_hdr.sector_size);
			HLOGD("ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "add")) {
			memset(&part_hdr, 0, sizeof(part_hdr));
			strncpy(part_hdr.name, argv[3], sizeof(part_hdr.name)-1);
			sscanf(argv[5], "%X", &(part_hdr.size));
			sscanf(argv[6], "%X", &(part_hdr.attr));
			rc = htc_pg_part_hdr_set(atoi(argv[2]), (int8 *)argv[3], &part_hdr, atoi(argv[4]));
			HLOGD("htc_pg_part_hdr_set ret = %d\r\n", rc);
			rc = htc_pg_part_erase(atoi(argv[2]), (int8 *)argv[3], 1);
			HLOGD("htc_pg_part_hdr_set ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "addfix")) {
			rc = htc_pg_fix_part_hdr_add(atoi(argv[2]), (int8 *)argv[3], atoi(argv[4]), atoi(argv[5]));
			HLOGD("ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "erase")) {
			rc = htc_pg_part_erase(atoi(argv[2]), (int8 *)argv[3], 1);
			HLOGD("ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "get")) {
			int unit, start, end;
			memset(&pg_hdr, 0, sizeof(pg_hdr));
			rc = htc_pg_hdr_get(atoi(argv[2]), &pg_hdr);
			memset(&part_hdr, 0, sizeof(part_hdr));
			rc = htc_pg_part_hdr_get(atoi(argv[2]), (int8 *)argv[3], &part_hdr);
			HLOGD("name = %s\r\n", part_hdr.name);
			HLOGD("size = 0x%X\r\n", part_hdr.size);
			HLOGD("checksum = 0x%X\r\n", part_hdr.checksum);
			HLOGD("attr = 0x%X\r\n", part_hdr.attr);
			HLOGD("link_num = %d\r\n", part_hdr.link_num);
			for (i = 0; i < part_hdr.link_num; i++) {
				HLOGD("[link %d] = (0x%X, 0x%X)\r\n", i,
					part_hdr.dl.link[i].GI, part_hdr.dl.link[i].SI);
				switch (part_hdr.dl.link[i].SI) {
				case HTC_PG_LINK_GROUP_ALL:
					HLOGD("  group %d\r\n", part_hdr.dl.link[i].GI);
					i++;
					break;
				case HTC_PG_LINK_RANGE_DEF:
					HLOGD("  range (%d, %d) ~ (%d, %d)\r\n",
							part_hdr.dl.link[i + 1].GI, part_hdr.dl.link[i + 1].SI,
							part_hdr.dl.link[i + 2].GI, part_hdr.dl.link[i + 2].SI
							);
					unit = pg_hdr.group_size / pg_hdr.sector_size;
					start = part_hdr.dl.link[i + 1].GI * unit + part_hdr.dl.link[i + 1].SI;
					end = part_hdr.dl.link[i + 2].GI * unit + part_hdr.dl.link[i + 2].SI + 1;
					HLOGD("  sector %d - %d\r\n", start, end);
					i += 2;
					break;
				default:
					HLOGD("  sector (%d, %d)\r\n",
							part_hdr.dl.link[i].GI, part_hdr.dl.link[i].SI
							);
				}
			}
			HLOGD("ret = %d\r\n", rc);
		} else if (!strcmp(argv[1], "free")) {
			rc = htc_pg_free_size(atoi(argv[2]));
			HLOGD("ret = 0x%X(%d)\r\n", rc, rc);
		} else if (!strcmp(argv[1], "crc")) {
			rc = htc_pg_part_crc(atoi(argv[2]), (int8 *)argv[3]);
			HLOGD("ret = 0x%X(%d)\r\n", rc, rc);
		} else if (!strcmp(argv[1], "update_crc")) {
		    rc = htc_pg_update_crc(atoi(argv[2]), (int8 *)argv[3]);
			HLOGD("ret = 0x%X(%d)\r\n", rc, rc);
#if HTC_PGFS_DEBUG
		} else if (!strcmp(argv[1], "dump")) {
			int offset, size;
			char *buf;

			sscanf(argv[3], "%d", &offset);
			sscanf(argv[4], "%d", &size);
			HLOGD("dump %s ofs:%d size:%d\r\n", offset, size);
			buf = alloc_page_aligned(size);
			if (buf != NULL) {
				partition_read_pgfs(argv[2], offset, buf, size);
				dump_buffer(buf, 0, size);
				free(buf);
			}
#endif
#ifdef HTC_PG_TOOL
		} else if (!strcmp(argv[1], "part_write")) {
			fp = fopen(argv[4], "rb");
			if (fp == NULL)
				HLOGD("%s could not be opened\r\n", argv[4]);
			else {
				if (fseek(fp, 0, SEEK_END))
					HLOGD("fseek failed\r\n");
				else if ((len = ftell(fp)) < 0)
					HLOGD("ftell failed\r\n");
				else if (fseek(fp, 0, SEEK_SET))
					HLOGD("fseek failed\r\n");
				else {
					dbuf = malloc(len);
					if (dbuf == NULL)
						HLOGD("dbuf allocation failed\r\n");
					else {
						rc = fread(dbuf, 1, len, fp);
						if (rc != len)
							HLOGD("file length mismatched: %d, %d\r\n", len, rc);
						else {
							rc = htc_pg_part_erase(atoi(argv[2]), argv[3], 0);
							HLOGD("htc_pg_part_erase ret = 0x%X(%d)\r\n", rc, rc);
							rc = htc_pg_part_write(atoi(argv[2]), argv[3], 0, dbuf, len, 1);
							HLOGD("htc_pg_part_write ret = 0x%X(%d)\r\n", rc, rc);
						}
						free(dbuf);
					}
				}
				fclose(fp);
			}
		} else if (!strcmp(argv[1], "part_read")) {
			fp = fopen(argv[4], "wb");
			if (fp == NULL)
				HLOGD("%s could not be opened\r\n", argv[4]);
			else {
				rc = htc_pg_part_hdr_get(atoi(argv[2]), argv[3], &part_hdr);
				HLOGD("ret = 0x%X(%d)\r\n", rc, rc);
				if (rc == PG_ERR_NONE) {
					len = part_hdr.size;
					dbuf = malloc(len);
					if (dbuf == NULL)
						HLOGD("dbuf allocation failed\r\n");
					else {
						rc = htc_pg_part_read(atoi(argv[2]), argv[3], 0, dbuf, len);
						HLOGD("ret = 0x%X(%d)\r\n", rc, rc);
						if ((rc = fwrite(dbuf, 1, len, fp)) != len)
							HLOGD("fwrite failed: %d, %d\r\n", len, rc);
						free(dbuf);
					}

				}
				fclose(fp);
			}
#endif
		}
	}

	return 0;
}
