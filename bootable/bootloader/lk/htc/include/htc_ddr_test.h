#ifndef __HTC_DDR_TEST_H
#define __HTC_DDR_TEST_H


#if 0
#include <platform/mt_reg_base.h>
#include <platform/mt_disp_drv.h>
#include <debug.h>

#define LK_START MEMBASE
#define LK_LEN   0x100000
#define FB_START 0xf1e00000
#define FB_LEN   0x4000000
#endif

typedef struct {
        unsigned int start;
        unsigned int len;
        int test_in_bldr;
} MEM_TEST_INFO;


#if 0 /* move to platform/mt6795/platform.c */
MEM_TEST_INFO mem_test_parition[]=
{
#if (6795 == MACH_TYPE)
    {DRAM_PHY_ADDR, (LK_START-DRAM_PHY_ADDR),  1},
    {LK_START, LK_LEN, 0}, /* 0x41F00000, 0x100000 */
    {(LK_START+LK_LEN), (BL_LOG_PHYS-(LK_START+LK_LEN)), 1},
    {BL_LOG_PHYS, BL_LOG_SIZE, 0},  /* 0x0x83B00000, 0x80000 */
    {(BL_LOG_PHYS+BL_LOG_SIZE), (FB_START-(BL_LOG_PHYS+BL_LOG_SIZE)), 1},
    {FB_START, FB_LEN, 0},
#endif	
    {0, 0, 0},
};
#endif

struct htc_ddrtest_device{
        MEM_TEST_INFO *mem_test_partition;
	int map_count;
        void (*display_info)(const char *msg, ...);
        void (*disable_watchdog)(void);
        void (*disable_dcache)(void);
        void (*set_ddr_rate)(void);
        void (*enable_bat_charging)(void);
};


int
complex_mem_test (unsigned int start, unsigned int len, int block, unsigned int pattern);


#endif
