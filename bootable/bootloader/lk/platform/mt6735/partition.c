#include <stdint.h>
#include <printf.h>
#include <malloc.h>
#include <string.h>
#include <platform/errno.h>
#include <platform/mmc_core.h>
#include <platform/partition.h>
#include <target.h>
#if _MAKE_HTC_LK
#include "htc_pg_fs.h"
#endif

#define TAG "[PART_LK]"

#define LEVEL_ERR   (0x0001)
#define LEVEL_INFO  (0x0004)

#define DEBUG_LEVEL (LEVEL_ERR | LEVEL_INFO)   

#define part_err(fmt, args...)   \
do {    \
    if (DEBUG_LEVEL & LEVEL_ERR) {  \
        dprintf(CRITICAL, fmt, ##args); \
    }   \
} while (0)

#define part_info(fmt, args...)  \
do {    \
    if (DEBUG_LEVEL & LEVEL_INFO) {  \
        dprintf(CRITICAL, fmt, ##args);    \
    }   \
} while (0)


part_t tempart;
struct part_meta_info temmeta;

part_t *partition;
part_t *partition_all;

static part_dev_t *mt_part_dev;

#if defined(MTK_EMMC_SUPPORT) || defined(MTK_NAND_SUPPORT)
static uchar *mt_part_buf;
#endif

part_t* get_part(char *name)
{
    part_t *part = partition;
    part_t *ret = NULL;
	
	part_info("%s[%s] %s\n", TAG, __FUNCTION__, name);
	while (part->nr_sects) {
        if (part->info) {
		    if (!strcmp(name, (char *)part->info->name)) {
                memcpy(&tempart, part, sizeof(part_t));
                memcpy(&temmeta, part->info, sizeof(struct part_meta_info));
                tempart.info = &temmeta;
                ret = &tempart;
                break;
            }
        }
		part++;
		
        //part_info("%s[%s] 0x%lx\n", TAG, __FUNCTION__, tempart.start_sect);
	}
    return ret;
}

void put_part(part_t *part)
{
    if (!part) {
        return;
    }

    if (part->info) {
        free(part->info);
    }
    free(part);
}


part_t *mt_part_get_partition(char *name)
{
    if (!strcmp(name, "PRELOADER") || !strcmp(name, "preloader")) {
        tempart.start_sect = 0x0;
        tempart.nr_sects = 0x200;
        tempart.part_id = 1;
        return &tempart;
    }
    return get_part(name);
}

#ifdef MTK_EMMC_SUPPORT
void mt_part_dump(void)
{
    part_t *part = partition;
    
	dprintf(CRITICAL, "zzytest, mt_part_dump begin\n");
    part_info("Part Info.(1blk=%dB):\n", BLK_SIZE);
    while (part->nr_sects) {
    	 part_info("[0x%016llx-0x%016llx] (%.8ld blocks): \"%s\"\n", 
               (u64)part->start_sect * BLK_SIZE, 
              (u64)(part->start_sect + part->nr_sects) * BLK_SIZE - 1, 
                               part->nr_sects, (part->info) ? part->info->name : "unknown");
        part++;
    }
    part_info("\n");
}

int mt_part_generic_read(part_dev_t *dev, u64 src, uchar *dst, int size, unsigned int part_id)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
    u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        part_err("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) { 
		return 0;
    }

	end = src + size;
	
	part_start    = src &  ((u64)BLK_SIZE - 1);
	part_end      = end &  ((u64)BLK_SIZE - 1);
	aligned_start = src & ~((u64)BLK_SIZE - 1);
	aligned_end   = end & ~((u64)BLK_SIZE - 1);
 
	if (part_start) {
		blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
        	if (part_len > (u64)size) {
            		part_len = size;
        	}
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf, part_id)) != 1) {
			return -EIO;
        	}
		memcpy(dst, buf + part_start, part_len);
		dst += part_len;
        	src += part_len;
	}
	aligned_start = src & ~((u64)BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;
	
	if ((blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst), part_id)) != blkcnt) {
		return -EIO;
	}

	src += (blkcnt << BLK_BITS);
	dst += (blkcnt << BLK_BITS);

	if (part_end && src < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf, part_id)) != 1) {
			return -EIO;
        }
		memcpy(dst, buf, part_end);
	}
	return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, u64 dst, int size, unsigned int part_id)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
    u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        part_err("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) {
		return 0;
    }

	end = dst + size;
	
	part_start    = dst &  ((u64)BLK_SIZE - 1);
	part_end      = end &  ((u64)BLK_SIZE - 1);
	aligned_start = dst & ~((u64)BLK_SIZE - 1);
	aligned_end   = end & ~((u64)BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if (part_len > (u64)size) {
			part_len = size;
		}
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf, part_id)) != 1) {
			return -EIO;
        }
	memcpy(buf + part_start, src, part_len);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf, part_id)) != 1) {
        	return -EIO;
        }
	dst += part_len;
        src += part_len;
    }
    
	aligned_start = dst & ~((u64)BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

	if ((blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src), part_id)) != blkcnt)
		return -EIO;
    
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && dst < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf, part_id)) != 1) {
			return -EIO;
		}
		memcpy(buf, src, part_end);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf, part_id)) != 1) {
            return -EIO;
    	}
	}
	return size;
}

extern int read_gpt(part_t *part);

int mt_part_register_device(part_dev_t *dev)
{
    part_t *part_ptr;

	part_info("[mt_part_register_device]\n");
    if (!mt_part_dev) {
        if (!dev->read) {
		    dev->read = mt_part_generic_read;
        }
		if (!dev->write) {
		    dev->write = mt_part_generic_write;
        }
		mt_part_dev = dev;

	    mt_part_buf = (uchar*)malloc(BLK_SIZE * 2);
	    //part_info("[mt_part_register_device]malloc %d : %x\n",(BLK_SIZE * 2), mt_part_buf);

        part_ptr = calloc(128 + 1, sizeof(part_t));
        if (!part_ptr) {
            return 0;
        }

        partition_all = part_ptr;
        partition = part_ptr + 1;

        read_gpt(partition);
        
        partition_all[0].start_sect = 0x0;
        partition_all[0].nr_sects = 0x200;
        partition_all[0].part_id = 1;
        partition_all[0].info = malloc(sizeof(struct part_meta_info));
        if (partition_all[0].info) {
            sprintf(partition_all[0].info->name, "%s", "preloader");
        }
    }
    return 0;
}

#elif defined(MTK_NAND_SUPPORT)

void mt_part_dump(void)
{
    part_t *part = &partition_layout[0];
    
    part_info("\nPart Info from compiler.(1blk=%dB):\n", BLK_SIZE);
    part_info("\nPart Info.(1blk=%dB):\n", BLK_SIZE);
    while (part->name) {
        part_info("[0x%.8x-0x%.8x] (%.8ld blocks): \"%s\"\n", 
               part->start_sect * BLK_SIZE, 
              (part->start_sect + part->nr_sects) * BLK_SIZE - 1, 
			   part->nr_sects, part->name);
        part++;
    }
    part_info("\n");
}

int mt_part_generic_read(part_dev_t *dev, ulong src, uchar *dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        part_err("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) {
		return 0;
    }

	end = src + size;
	
	part_start    = src &  (BLK_SIZE - 1);
	part_end      = end &  (BLK_SIZE - 1);
	aligned_start = src & ~(BLK_SIZE - 1);
	aligned_end   = end & ~(BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf), part_id) != 1) {
			return -EIO;
        }
		memcpy(dst, buf + part_start, part_len);
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = src & ~(BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;
	
	if ((blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst), part_id)) != blkcnt) {
		return -EIO;
    }

    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && src < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf)) != 1) {
			return -EIO;
        }
		memcpy(dst, buf, part_end);
	}
	return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, ulong dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        part_err("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) {
		return 0;
    }

	end = dst + size;
	
	part_start    = dst &  (BLK_SIZE - 1);
	part_end      = end &  (BLK_SIZE - 1);
	aligned_start = dst & ~(BLK_SIZE - 1);
	aligned_end   = end & ~(BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf), part_id) != 1)
			return -EIO;
		memcpy(buf + part_start, src, part_len);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf), part_id) != 1)
        	return -EIO;
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = dst & ~(BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

	if ((blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src), part_id)) != blkcnt) {
		return -EIO;
    }
    
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && dst < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf), part_id) != 1) {
			return -EIO;
		}
		memcpy(buf, src, part_end);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf), part_id) != 1) {
            return -EIO;
    	}
	}
	return size;
}

int mt_part_register_device(part_dev_t *dev)
{
    part_info("[mt_part_register_device]\n");
    if (!mt_part_dev) {
        if (!dev->read) {
            dev->read = mt_part_generic_read;
        }
        if (!dev->write) {
            dev->write = mt_part_generic_write;
        }
        mt_part_dev = dev;
		mt_part_buf = (uchar*)malloc(BLK_SIZE * 2);
    	part_info("[mt_part_register_device]malloc %d : %x\n", (BLK_SIZE * 2), mt_part_buf);
        
#ifdef PMT
	    part_init_pmt(BLK_NUM(1 * GB), dev);
#else
        mt_part_init(BLK_NUM(1 * GB));
#endif 
    }
	
    return 0;
}

#else

void mt_part_dump(void)
{
}


#endif

part_dev_t *mt_part_get_device(void)
{
    if (mt_part_dev && !mt_part_dev->init && mt_part_dev->init_dev) {
        mt_part_dev->init_dev(mt_part_dev->id);
        mt_part_dev->init = 1;
    }
    return mt_part_dev;
}

/**/
/*fastboot*/
/**/
unsigned int write_partition(unsigned size, unsigned char *partition)
{
    return 0;
}

int partition_get_index(const char * name)
{
    int i;

    for (i = 0; i < 129; i++) {
        if (!partition_all[i].info)
            continue;
        if (!strcmp(name, partition_all[i].info->name)) {
            printf("[%s]find %s index %d\n", __FUNCTION__, name, i);
            return i;
        }
    }

    return -1;
}

unsigned int partition_get_region(int index)
{
    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    return (u64)partition_all[index].part_id;
}   

u64 partition_get_offset(int index)
{
    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    return (u64)partition_all[index].start_sect * BLK_SIZE;
}

u64 partition_get_size(int index)
{
    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    return (u64)partition_all[index].nr_sects * BLK_SIZE;
}

int partition_get_type(int index, char **p_type)
{
    int i, loops;

    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    if (!partition_all[index].info)
        return -1;

    loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

    for (i = 0; i < loops; i++) {
	if (!g_part_name_map[i].fb_name[0])
		break;
        if (!strcmp(partition_all[index].info->name, g_part_name_map[i].r_name)) {
            *p_type = g_part_name_map[i].partition_type;
            return 0;
        }
    }
    return -1;
}

int partition_get_name(int index, char **p_name)
{
    int i, loops;

    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    if (!partition_all[index].info)
        return -1;

    *p_name = partition_all[index].info->name;
    return 0;
}

int is_support_erase(int index)
{
    int i, loops;

    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    if (!partition_all[index].info)
        return -1;

    loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

    for (i = 0; i < loops; i++) {
	if (!g_part_name_map[i].fb_name[0])
		break;
        if (!strcmp(partition_all[index].info->name, g_part_name_map[i].r_name)) {
            return g_part_name_map[i].is_support_erase;
        }
    }

    return -1;
}

int is_support_flash(int index)
{
    int i, loops;

    if (index < 0 || index >= 129) {
        return -1;
    }

    if (!partition_all[index].nr_sects)
        return -1;

    if (!partition_all[index].info)
        return -1;

    loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

    for (i = 0; i < loops; i++) {
	if (!g_part_name_map[i].fb_name[0])
		break;
        if (!strcmp(partition_all[index].info->name, g_part_name_map[i].r_name)) {
            return g_part_name_map[i].is_support_dl;
        }
    }

    return -1;
}

#ifdef MTK_EMMC_SUPPORT
u64 emmc_write(u32 part_id, u64 offset, void *data, u64 size)
{
	part_dev_t *dev = mt_part_get_device();
	return (u64)dev->write(dev,data,offset,(int)size, part_id);
}

u64 emmc_read(u32 part_id, u64 offset, void *data, u64 size)
{
	part_dev_t *dev = mt_part_get_device();
	return (u64)dev->read(dev,offset,data,(int)size, part_id);
}

int emmc_erase(u32 part_id, u64 offset, u64 size)
{
	return mmc_do_erase(0,offset,size, part_id);
}
#endif
#if _MAKE_HTC_LK
int partition_read(char *name, u64 offset, void *buf, u64 len)
{
    long ret = 0;

#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;
#else
	unsigned long start_addr;
#endif
    part_t *part;
    part_dev_t *dev;

    dev = mt_part_get_device();
    if (!dev)
    {
        return -ENODEV;
    }

    part = mt_part_get_partition(name);
    if (!part)
    {
		return -ENOENT;
    }
#ifdef MTK_EMMC_SUPPORT
    start_addr = (u64)part->start_sect * BLK_SIZE;
#else
    start_addr = part->startblk * BLK_SIZE;
#endif

#ifdef MTK_EMMC_SUPPORT
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    ret = dev->read(dev, start_addr + offset,(uchar*)buf, len, part->part_id);
#else
    ret = dev->read(dev, start_addr + offset,(uchar*)buf, len);
#endif
#else
    ret = dev->read(dev, start_addr + offset,(uchar*)buf, len);
#endif
    if (ret < 0)
    {
        ret = -EIO;
        goto exit;
    }

exit:
    return ret;
}

int partition_write(char *name, u64 offset, void *buf, u64 len)
{
    long ret = 0;

#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;
#else
	unsigned long start_addr;
#endif
    part_t *part;
    part_dev_t *dev;

    dev = mt_part_get_device();
    if (!dev)
    {
        return -ENODEV;
    }

    part = mt_part_get_partition(name);
    if (!part)
    {
		return -ENOENT;
    }
#ifdef MTK_EMMC_SUPPORT
    start_addr = (u64)part->start_sect * BLK_SIZE;
#else
    start_addr = part->startblk * BLK_SIZE;
#endif

#ifdef MTK_EMMC_SUPPORT
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    ret = dev->write(dev, (uchar*)buf, start_addr + offset, len, part->part_id);
#else
    ret = dev->write(dev, (uchar*)buf, start_addr + offset, len);
#endif
#else
    ret = dev->write(dev, (uchar*)buf, start_addr + offset, len);
#endif
    if (ret < 0)
    {
        ret = -EIO;
        goto exit;
    }

exit:
    return ret;
}

unsigned long long partition_get_size_by_name(char* name)
{
	part_t *p = NULL;

	p = mt_part_get_partition(name);
	if (p == NULL) {
		return -1;
    }
	return (unsigned long long)p->nr_sects * BLK_SIZE;
}

unsigned long partition_size(const char* name)
{
	part_t *p = NULL;

	p = mt_part_get_partition(name);
	if (p == NULL) {
		return -1;
    }
	return (unsigned long)p->nr_sects; // return sector number of the specific partition
}

unsigned long partition_start_block(const char *name)
{
	part_t *p = NULL;

	p = mt_part_get_partition(name);
	if (p == NULL) {
		return -1;
    }
	return (unsigned long)p->start_sect;
}

part_t* get_part_2(unsigned long sector_num)
{
	part_t *part = partition;
	part_t *ret = NULL;

	part_info("%s[%s] 0x%X\n", TAG, __FUNCTION__, sector_num);
	while (part->nr_sects)
	{
	        if (part->info)
		{
			if (sector_num >= part->start_sect &&  sector_num < part->start_sect+part->nr_sects)
			{
		                memcpy(&tempart, part, sizeof(part_t));
		                memcpy(&temmeta, part->info, sizeof(struct part_meta_info));
		                tempart.info = &temmeta;
		                ret = &tempart;
		                break;
	            }
	        }
		part++;

        //part_info("%s[%s] 0x%lx\n", TAG, __FUNCTION__, tempart.start_sect);
	}
	return ret;
}

static int get_pg_sector(const char *name)
{
	if ((strlen(name) > 6)) {
		if (!strncmp(name, "pg1fs_", 6))
			return partition_start_block("pg1fs");
		else if (!strncmp(name, "pg2fs_", 6))
			return partition_start_block("pg2fs");
		else
			return -1;
	}
	return -1;
}

int partition_size_pgfs(const char *name)
{
	int ret, pg_sector;
	htc_part_hdr part_hdr;
	char *pg_name;

	pg_sector = get_pg_sector(name);
	if (pg_sector < 0)
		return -1;

	pg_name = name + 6;
	ret = htc_pg_part_hdr_get(pg_sector, pg_name, &part_hdr);
	if (ret != PG_ERR_NONE)
		return -1;

	return part_hdr.size;
}

int partition_read_pgfs(const char *name, int offset, void *buf, int len)
{
	int ret, pg_sector;
	htc_part_hdr part_hdr;
	char *pg_name;

	HLOGD("[PGFS] partition_read_pgfs: %s\r\n", name);

	pg_sector = get_pg_sector(name);
	if (pg_sector < 0)
		return -1;
	pg_name = name + 6;
	ret = htc_pg_part_hdr_get(pg_sector, pg_name, &part_hdr);
	if (ret != PG_ERR_NONE)
		return -1;
	ret = htc_pg_part_read(pg_sector, pg_name, offset, buf, len);
	if (ret < 0)
		return -1;

	return 0;
}


int partition_update_pgfs(const char *name, int offset, void *buf, int len)
{
	int ret, pg_sector;
	htc_part_hdr part_hdr;
	char *pg_name;
	char *dec_buf = NULL;

	HLOGD("[PGFS] partition_update_pgfs: %s\r\n", name);

	pg_sector = get_pg_sector(name);
	if (pg_sector < 0)
		return -1;
	pg_name = name + 6;
	ret = htc_pg_part_hdr_get(pg_sector, pg_name, &part_hdr);
	if (ret != PG_ERR_NONE)
		return -1;

	if (len > part_hdr.size)
	{
		HLOGD("[PGFS] partition_update_pgfs: PGFS update partition size (%d) is larger than eMMC partition size (%d)\r\n", part_hdr.size, len);
		len = part_hdr.size;
	}

	ret = htc_pg_part_write(pg_sector, pg_name, offset, buf, len, 1);
	if (dec_buf != NULL)
		free(dec_buf);

	return ret;
}

int partition_format_pgfs(const char *name)
{
	int ret, pg_sector;
	char *pg_name;

	HLOGD("[PGFS] partition_format_pgfs: %s\r\n", name);

	pg_sector = get_pg_sector(name);
	if (pg_sector < 0)
		return -1;
	pg_name = name + 6;
	ret = htc_pg_part_erase(pg_sector, pg_name, 1);
	return ret;
}

int partition_format_emmc(char *name)
{
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
	unsigned int part_id;
#endif
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	int index = -1; //INVALID_PTN;
	int erase_ret = MMC_ERR_NONE;
	index = partition_get_index(name);
	if (index == -1)
	{
		return index;
	}
	setting_invalidate_checksum();

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
	part_id = partition_get_region(index);
#endif
	ptn = partition_get_offset(index);
	size = partition_get_size(index);

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
	erase_ret = emmc_erase(part_id, ptn, size);
#else
	erase_ret = emmc_erase(ptn, size);
#endif

	return erase_ret;
}

#endif //_MAKE_HTC_LK

void partition_dump_fb(void)
{
	int i = 0, size = 0, offset = 0;
	char *name = NULL;
	char *type = NULL;
	char info_buffer[64] = {0x00};

	fastboot_info(info_buffer);

	for (i = 0; i < PART_MAX_COUNT; i++) {
		size = partition_get_size(i) / BLK_SIZE;
		if(size == -1) break;
		offset = partition_get_offset(i) / BLK_SIZE;
		partition_get_name(i, &name);
		partition_get_type(i, &type);
		sprintf(info_buffer, "[%d][%s](%s) Addr=%d ~ %d (%d KB)",
			i, name, type, offset, size, size/2);
		fastboot_info(info_buffer);
	}
}
