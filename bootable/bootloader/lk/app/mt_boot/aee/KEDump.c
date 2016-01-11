#include <stdio.h>
#include <string.h>
#ifdef MTK_GPT_SCHEME_SUPPORT
#include <platform/partition.h>
#else
#include <mt_partition.h>
#endif
#include <platform/boot_mode.h>
#include <platform/ram_console.h>
#include <platform/mt_reg_base.h>
#include "elf.h"
#include "KEHeader.h"

/**************************/
/* ----------------	  */
/* RAM_CONSOLE_DRAM_ADDR  */
/* (1M align)		  */
/* +RAM_CONSOLE_DRAM_SIZE */
/* 			  */
/* ----------------	  */
/* +0xf0000		  */
/* 			  */
/* ----------------	  */
/* KE_RESERVED_MEM_ADDR	  */
/* 			  */
/* ----------------	  */
/* RAMDISK_LOAD_ADDR	  */
/**************************/
#define KE_RESERVED_MEM_ADDR (RAM_CONSOLE_DRAM_ADDR + 0xf0000)

#define MSM_RTB_ADDR 0x83E00000  /*last io*/
#define MSM_RTB_SIZE 0x100000
#define HTC_DEBUG_RTB_ADDR 0x83C10000    /*htc_debug_rtb*/
#define HTC_DEBUG_RTB_SIZE 0x20000
#define HTC_DEBUG_RTB_MAGIC 0x5254424D /* RTBM */

struct htc_debug_rtb {
  unsigned int magic;
  unsigned int phys;
  unsigned int cpu_idx[];
};

struct ke_dev {
	part_dev_t *dev;
	uint  part_id;
	u64 ptn;
	u64 part_size;
};

static struct ke_dev dev;

struct elfhdr {
	void *start;
	unsigned int e_machine;
	unsigned int e_phoff;
	unsigned int e_phnum;
};
	
extern BOOT_ARGUMENT *g_boot_arg;
#define SZLOG 4096
static char logbuf[SZLOG];

static int is_lk_resetted() ;

extern void ram_console_addr_size(unsigned long *addr, unsigned long *size);

int sLOG(char *fmt, ...)
{
	va_list args;
	static int pos = 0;
	va_start(args, fmt);
	if (pos < SZLOG - 1) /* vsnprintf bug */
		pos += vsnprintf(logbuf + pos, SZLOG - pos - 1, fmt, args);
	va_end(args);
	return 0;
}

#define LOG(fmt, ...)			\
	do {						\
	sLOG(fmt, ##__VA_ARGS__);	\
	printf(fmt, ##__VA_ARGS__); \
	}while(0);

#define LOGD(fmt, ...)				\
	sLOG(fmt, ##__VA_ARGS__)

#define elf_note	elf32_note
#define PHDR_PTR(ehdr, phdr, mem)		\
	(ehdr->e_machine == EM_ARM ? ((struct elf32_phdr*)phdr)->mem : ((struct elf64_phdr*)phdr)->mem)

#define PHDR_TYPE(ehdr, phdr) PHDR_PTR(ehdr, phdr, p_type)
#define PHDR_ADDR(ehdr, phdr) PHDR_PTR(ehdr, phdr, p_paddr)
#define PHDR_SIZE(ehdr, phdr) PHDR_PTR(ehdr, phdr, p_filesz)
#define PHDR_OFF(ehdr, phdr) PHDR_PTR(ehdr, phdr, p_offset)
#define PHDR_INDEX(ehdr, i)			\
	(ehdr->e_machine == EM_ARM ? ehdr->start + ehdr->e_phoff + sizeof(struct elf32_phdr) * i : ehdr->start + ehdr->e_phoff + sizeof(struct elf64_phdr) *i)

static void lastpc_dump()
{
  volatile unsigned int mcu_base = MCUCFG_BASE + 0x410;
	int ret = -1, i;
  int cnt = 8 ;
	__u64 pc_value;
	__u64 fp_value;
	__u64 sp_value;
	__u64 pc_value_h;
	__u64 fp_value_h;
	__u64 sp_value_h;
	int cluster, cpu_in_cluster;

  LOG("last pc base=0x%lx\n", mcu_base) ;
  for (i = 0; i < cnt; i++) {
		cluster = i / 4;
		cpu_in_cluster = i % 4;
		pc_value_h = DRV_Reg32((mcu_base+0x4) + (cpu_in_cluster << 5) + (0x100 * cluster));
		pc_value = (pc_value_h << 32) |  DRV_Reg32((mcu_base+0x0) + (cpu_in_cluster << 5) + (0x100 * cluster));
		fp_value_h = DRV_Reg32((mcu_base+0x14) + (cpu_in_cluster << 5) + (0x100 * cluster));
		fp_value = (fp_value_h <<32) | DRV_Reg32((mcu_base+0x10) + (cpu_in_cluster << 5) + (0x100 * cluster));
		sp_value_h = DRV_Reg32((mcu_base+0x1c) + (cpu_in_cluster << 5) + (0x100 * cluster));
		sp_value = (sp_value_h << 32) | DRV_Reg32((mcu_base+0x18) + (cpu_in_cluster << 5) + (0x100 * cluster));

		LOG("[LAST PC] CORE_%d PC = 0x%llx, FP = 0x%llx, SP = 0x%llx\n", i, pc_value, fp_value, sp_value);
	}
}

static struct elfhdr* kedump_elf_hdr(void)
{
	char *ei;
	static struct elfhdr kehdr;
	static struct elfhdr *ehdr = (void*)-1;
	if (ehdr != (void*)-1)
		return ehdr;
	ehdr = NULL;
        kehdr.start = (void*)(KE_RESERVED_MEM_ADDR);
	LOGD("kedump: KEHeader %p\n", kehdr.start);
	if (kehdr.start) {
		ei = (char*)kehdr.start; //elf_hdr.e_ident
        /* valid elf header */
		if (ei[0] == 0x7f && ei[1] == 'E' && ei[2] == 'L' && ei[3] == 'F') {
			kehdr.e_machine = ((struct elf32_hdr*)(kehdr.start))->e_machine;
			if (kehdr.e_machine == EM_ARM) {
				kehdr.e_phnum = ((struct elf32_hdr*)(kehdr.start))->e_phnum;
				kehdr.e_phoff = ((struct elf32_hdr*)(kehdr.start))->e_phoff;
				ehdr = &kehdr;
			} else if (kehdr.e_machine == EM_AARCH64) {
				kehdr.e_phnum = ((struct elf64_hdr*)(kehdr.start))->e_phnum;
				kehdr.e_phoff = ((struct elf64_hdr*)(kehdr.start))->e_phoff;
				ehdr = &kehdr;
			}
		}
		if(ehdr == NULL)
			LOG("kedump: invalid header[0x%x%x%x%x]\n", ei[0], ei[1], ei[2], ei[3]);
        }
	LOGD("kedump: mach[%x], phnum[%x], phoff[%x]", kehdr.e_machine, kehdr.e_phnum, kehdr.e_phoff);
	return ehdr;
}

/*kedump_done set ehdr[0] to 0x81*/
static int is_lk_resetted()
{
	char *ehdr = (char*)(KE_RESERVED_MEM_ADDR) ;
  if(ehdr[0] == 0x81 && ehdr[1] == 'E' && ehdr[2] == 'L' && ehdr[3] == 'F')
  {
    LOG("kedump lk is resetted in previous\n");
    return 1 ;
  }
  else
    return 0 ;
}

static int kedump_dev_open(void)
{
	int index;
        index = partition_get_index(AEE_IPANIC_PLABLE);
	dev.dev = mt_part_get_device();
        if(index == -1 || dev.dev == NULL) {
		LOG("kedump: no %s partition[%d]\n", AEE_IPANIC_PLABLE, index);
                return -1;
        }
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        dev.part_id = partition_get_region(index);
#endif
        dev.ptn = partition_get_offset(index);
	dev.part_size = partition_get_size(index);
	LOG("kedump: partiton %d[%llx - %llx]\n", index, dev.ptn, dev.part_size);
        
        return 0;
}

static unsigned long long kedump_dev_write (unsigned long long offset, void *data, unsigned long sz)
{
        unsigned long long size_wrote = 0;
	
	if (offset >= dev.part_size || sz > dev.part_size - offset) {
		LOG("kedump: write oversize %lx -> %llx > %lx\n", sz, offset, dev.part_size);
		return 0;
	}

#ifdef MTK_EMMC_SUPPORT
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        size_wrote = dev.dev->write(dev.dev, data, dev.ptn + offset, sz, dev.part_id);
#else
        size_wrote = dev.dev->write(dev.dev, data, dev.ptn + offset, sz);
#endif
#elif defined(MTK_NAND_SUPPORT)
	size_wrote = dev.dev->write(dev.dev, data, (unsigned long)dev.ptn + offset, sz);
#endif

	if ((long long)size_wrote < 0) {
		LOG("kedump: write failed(%llx/%llx), %lx@%p -> %llx\n", size_wrote, sz, data, offset);
		size_wrote = 0;
	}
        return size_wrote;
}

static void kedump_dev_close(void)
{
        return;
}

/* the min offset reserved for the header's size. */
static unsigned long kedump_mrdump_header_size (struct elfhdr *ehdr)
{
	void *phdr = PHDR_INDEX(ehdr, 1);
	return ALIGN(PHDR_OFF(ehdr, phdr) + PHDR_SIZE(ehdr, phdr), PAGE_SIZE);
}

static unsigned int kedump_mini_rdump(struct elfhdr *ehdr, unsigned long long offset)
{
	void *phdr;
	unsigned long addr, size;
	unsigned int i;
	unsigned int total = 0;
	unsigned long elfoff = kedump_mrdump_header_size(ehdr);
	unsigned long sz_header = elfoff;
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = PHDR_INDEX(ehdr, i);
		LOGD("kedump: PT[%d] %x@%llx -> %x(%x)\n", PHDR_TYPE(ehdr, phdr), PHDR_SIZE(ehdr, phdr), PHDR_ADDR(ehdr, phdr), elfoff, (unsigned int)PHDR_OFF(ehdr, phdr));
		if (PHDR_TYPE(ehdr, phdr) != PT_LOAD)
			continue;
		addr = PHDR_ADDR(ehdr, phdr);
		size = PHDR_SIZE(ehdr, phdr);
		if (ehdr->e_machine == EM_ARM)
			((struct elf32_phdr*)phdr)->p_offset = elfoff;
		else
			((struct elf64_phdr*)phdr)->p_offset = elfoff;
		if (size != 0 && elfoff != 0)
			total += kedump_dev_write(offset + elfoff, (void*)addr, size);
		elfoff += size;
	}
	total += kedump_dev_write(offset, ehdr->start, sz_header);
	return total;
}

static unsigned int kedump_misc(unsigned int addr, unsigned int start, unsigned int size, unsigned long long offset)
{
	unsigned int total;
	LOG("kedump: misc data %x@%x+%x\n", size, addr, start);
	if (start >= size)
		start = start % size;
	total = kedump_dev_write(offset, (void*)addr + start, size - start);
	if (start)
		total += kedump_dev_write(offset + total, (void*)addr, start);
	return total;
}

static unsigned int kedump_misc32(struct mrdump_mini_misc_data32 *data, unsigned long long offset)
{
	unsigned int addr = data->paddr;
	unsigned int start = data->start ? *(unsigned int*)(data->start) : 0;
	unsigned int size = data->size;
	return kedump_misc(addr, start, size, offset);
}

static unsigned int kedump_misc64(struct mrdump_mini_misc_data64 *data, unsigned long long offset)
{
	unsigned int addr = (unsigned int)data->paddr;
	unsigned int start = data->start ? *(unsigned int*)(unsigned long)(data->start) : 0;
	unsigned int size = (unsigned int)data->size;
	return kedump_misc(addr, start, size, offset);
}

struct ipanic_header panic_header;
static void kedump_add2hdr(unsigned int offset, unsigned int size, unsigned datasize, char *name)
{
	struct ipanic_data_header *pdata;
	int i;
	for (i = 0; i < IPANIC_NR_SECTIONS; i++) {
		pdata = &panic_header.data_hdr[i];
		if (pdata->valid == 0)
			break;
	}
	LOG("kedump add: %s[%d] %x/%x@%x\n", name, i, datasize, size, offset);
	if (i < IPANIC_NR_SECTIONS) {
		pdata->offset = offset;
		pdata->total = size;
		pdata->used = datasize;
		strcpy((char*)pdata->name, name);
		pdata->valid = 1;
	}
}

static int kedump_to_expdb(void)
{
	struct elfhdr *ehdr;
	void *phdr_misc;
	struct elf_note *misc, *miscs;
	char *m_name;
	unsigned long sz_misc, addr_misc;
	void *m_data;
	unsigned long long offset = 0;
	unsigned int size, datasize;
	char name[32];
	unsigned int i;

  lastpc_dump() ;

	if (kedump_dev_open() != 0)
                return -1;

        ehdr = kedump_elf_hdr();
  if(ehdr)
  {
  /* reserve space in expdb for panic header */
    offset = ALIGN(sizeof(panic_header), BLK_SIZE);
    datasize = kedump_mini_rdump(ehdr, offset);
    size = datasize;
    kedump_add2hdr(offset, size, datasize, "SYS_MINI_RDUMP");
    offset += datasize;

    phdr_misc = PHDR_INDEX(ehdr, 1);
    miscs = (struct elf_note*)(ehdr->start + PHDR_OFF(ehdr, phdr_misc));
    LOGD("kedump: misc[%p] %llx@%llx\n", phdr_misc, PHDR_SIZE(ehdr, phdr_misc), PHDR_OFF(ehdr, phdr_misc));
    sz_misc = sizeof(struct elf_note) + miscs->n_namesz + miscs->n_descsz;
    LOGD("kedump: miscs[%p], size %x\n", miscs, sz_misc);
    for (i = 0; i < (PHDR_SIZE(ehdr, phdr_misc)) / sz_misc; i++) {
      misc = (struct elf_note*)((void*)miscs + sz_misc * i);
      m_name = (char*)misc + sizeof(struct elf_note);
      if (m_name[0] == 'N' && m_name[1] == 'A' && m_name[2] == '\0')
                          break;
      m_data = (void*)misc + sizeof(struct elf_note) + misc->n_namesz;
      if (misc->n_descsz == sizeof(struct mrdump_mini_misc_data32)) {
        datasize = kedump_misc32((struct mrdump_mini_misc_data32*)m_data, offset);
        size = ((struct mrdump_mini_misc_data32*)m_data)->size;
      } else {
        datasize = kedump_misc64((struct mrdump_mini_misc_data64*)m_data, offset);
        size = ((struct mrdump_mini_misc_data64*)m_data)->size;
      }
      /* [SYS_]MISC[_RAW] */
                  if (m_name[0] == '_')
                          strcpy (name, "SYS");
      else
        name[0] = 0;
                  strcat (name, m_name);
                  if (m_name[strlen(m_name)-1] == '_')
                          strcat (name,  "RAW");
      kedump_add2hdr(offset, size, datasize, name);
      offset += datasize;
    }
  }
	/* ram_console raw log */
	ram_console_addr_size(&addr_misc, &sz_misc);
	if (addr_misc && sz_misc) {
		datasize = kedump_misc((unsigned int)addr_misc, 0, (unsigned int)sz_misc, offset);
		kedump_add2hdr(offset, (unsigned int)sz_misc, datasize, "SYS_RAMCONSOLE_RAW");
		offset += datasize;
	}

   /* LK raw log */
   addr_misc = BL_LOG_PHYS;
   sz_misc = BL_LOG_SIZE;
   if (addr_misc && sz_misc) {
           datasize = kedump_misc((unsigned int)addr_misc, 0, (unsigned int)sz_misc, offset);
           kedump_add2hdr(offset, (unsigned int)sz_misc, datasize, "SYS_LK_LOG_RAW");
           offset += datasize;
   }

   /* LK last raw log */
   addr_misc = LAST_BL_LOG_PHYS;
   sz_misc = BL_LOG_SIZE;
   if (addr_misc && sz_misc) {
           datasize = kedump_misc((unsigned int)addr_misc, 0, (unsigned int)sz_misc, offset);
           kedump_add2hdr(offset, (unsigned int)sz_misc, datasize, "SYS_LK_LAST_LOG_RAW");
           offset += datasize;
   }

  /* msm_rtb raw data*/
  datasize = kedump_misc((unsigned int)MSM_RTB_ADDR, 0, (unsigned int)MSM_RTB_SIZE, offset) ;
  kedump_add2hdr(offset, (unsigned int)MSM_RTB_SIZE, datasize, "SYS_RTB_RAW") ;
  offset += datasize ;
  /* per cpu rtb index*/
  if(((struct htc_debug_rtb*)HTC_DEBUG_RTB_ADDR)->magic == HTC_DEBUG_RTB_MAGIC )
  {
    int i = 0 ;
    struct htc_debug_rtb *rtb_info = (struct htc_debug_rtb*)HTC_DEBUG_RTB_ADDR ;
    while(i<8)
    {
      LOG("rtb_index: cpu[%d], addr[%x], index[0x%x]\n", i, rtb_info->cpu_idx[i], *((unsigned int*)rtb_info->cpu_idx[i])) ;
      i++ ;
    }
  }

	/* save KEdump flow logs */
	datasize = kedump_dev_write(offset, logbuf, SZLOG);
	kedump_add2hdr(offset, SZLOG, datasize, "ZAEE_LOG");
	offset += datasize;

	/* finally write the ipanic header. */
	panic_header.magic = AEE_IPANIC_MAGIC;
	panic_header.version = AEE_IPANIC_PHDR_VERSION;
	panic_header.size = sizeof(panic_header);
	panic_header.blksize = BLK_SIZE;
	panic_header.partsize = dev.part_size;
        kedump_dev_write(0, (void*)(&panic_header), sizeof(panic_header));
        return 0; 
}

static int kedump_skip(void)
{
	unsigned int boot_reason = g_boot_arg->boot_reason;
	static int kedump_dumped = 0;
	LOG("kedump: boot_reason(%d)\n", boot_reason);
	ram_console_init();
	/* this flow should be executed once only */
	if (kedump_dumped == 0) {
		kedump_dumped = 1;
		if (ram_console_is_abnormal_boot())
		    return 0;
	}
	return 1;
}

static int kedump_avail(void)
{
        struct elfhdr *ehdr;
        ehdr = kedump_elf_hdr();
        if (0 == ehdr) {
                return -1;
        }
	return 0;
}

static int kedump_done(void)
{
	struct elfhdr* ehdr;
        ehdr = kedump_elf_hdr();
	if (0 == ehdr) {
                return -1;
        }
	((char*)ehdr->start)[0] = 0x81; //->e_ident[0]
	return 0;
}
/* Dump KE infomation to expdb */
/*  0 success , -1 failed*/
int kedump_mini(void)
{
	LOG("kedump mini start\n");
        if (kedump_skip())
                return 0;

        if (is_lk_resetted())
            return 0 ;

  /*kedump_skip detect abnormal_boot, don't check elfhdr again
   * because sometimes ehdr is wrong due to unkown reason*/
#if 0
        if (kedump_avail())
                return 0;
#endif

	kedump_to_expdb();

	kedump_done();
	LOG("kedump mini done\n");
        return 1;
}
