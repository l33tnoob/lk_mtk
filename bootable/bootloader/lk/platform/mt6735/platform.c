/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <video.h>
#include <dev/uart.h>
#include <arch/arm.h>
#include <arch/arm/mmu.h>
#include <arch/ops.h>
#include <target/board.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_disp_drv.h>
#include <platform/disp_drv.h>
#include <platform/boot_mode.h>
#include <platform/mt_logo.h>
#include <platform/partition.h>
#include <platform/env.h>
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#include <platform/mt_pmic_wrap_init.h>
#include <platform/mt_i2c.h>
#include <platform/mtk_key.h>
#include <platform/mt_rtc.h>
#include <platform/mt_leds.h>
#include <platform/upmu_common.h>
#include <platform/mtk_wdt.h>
#if _MAKE_HTC_LK
#include <htc_version_info.h>
#include <htc_board_info_and_setting.h>
#include <htc_reboot_info.h>
#include <htc_security_util.h>
#endif
#include <platform/disp_drv_platform.h>
#if _MAKE_HTC_LK
#include <htc_board_info_and_setting.h>
#include <htc_reboot_info.h>
#endif

#ifdef LK_DL_CHECK
/*block if check dl fail*/
#undef LK_DL_CHECK_BLOCK_LEVEL
#endif
//#define MT_SRAM_REPAIR_SUPPORT

extern void platform_early_init_timer();
extern void jump_da(u32 addr, u32 arg1, u32 arg2);
extern int i2c_hw_init(void);
extern int mboot_common_load_logo(unsigned long logo_addr, char* filename);
extern int sec_func_init(int dev_type);
extern int sec_usbdl_enabled (void);
extern int sec_usbdl_verify_da(unsigned char*, unsigned int, unsigned char*, unsigned int);
extern void mtk_wdt_disable(void);
extern void platform_deinit_interrupts(void);
extern int mmc_get_dl_info(void);
extern int mmc_legacy_init(int);
#ifdef MT_SRAM_REPAIR_SUPPORT
extern int repair_sram(void);
#endif
#ifdef MTK_EFUSE_WRITER_SUPPORT
extern void mt_efuse_get(void);
#endif
extern bool g_boot_menu;

#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
extern kal_bool is_low_battery(kal_int32 val);
extern int hw_charging_get_charger_type(void);
#endif

#ifdef MTK_MT8193_SUPPORT
extern int mt8193_init(void);
#endif

void platform_uninit(void);
void config_shared_SRAM_size(void);
extern int dev_info_nr_cpu(void);

#if _MAKE_HTC_LK
extern void vib_turn_on(int delay_ms);
#endif
BOOT_ARGUMENT *g_boot_arg;
BOOT_ARGUMENT boot_addr;
int g_nr_bank;
BI_DRAM bi_dram[MAX_NR_BANK];
unsigned int g_fb_base;
unsigned int g_fb_size;

static int g_dram_init_ret;

#if defined(MTK_TC7_FEATURE)
extern char ramdump_boot;
#endif

long long zzy_test_flags = 0;
int dram_init(void)
{
    int i;
    struct boot_tag *tags;

    /* Get parameters from pre-loader. Get as early as possible
     * The address of BOOT_ARGUMENT_LOCATION will be used by Linux later
     * So copy the parameters from BOOT_ARGUMENT_LOCATION to LK's memory region
     */
    g_boot_arg = &boot_addr;

    if(*(unsigned int *)BOOT_ARGUMENT_LOCATION == BOOT_ARGUMENT_MAGIC)
    {
        memcpy(g_boot_arg, (void*)BOOT_ARGUMENT_LOCATION, sizeof(BOOT_ARGUMENT));
    }
    else
    {
        zzy_test_flags = zzy_test_flags || 0x1;
        g_boot_arg->maggic_number = BOOT_ARGUMENT_MAGIC;
	    for (tags = (void *)BOOT_ARGUMENT_LOCATION; tags->hdr.size; tags = boot_tag_next(tags))
	    {
		    switch(tags->hdr.tag)
		    {
                case BOOT_TAG_BOOT_REASON:
                    g_boot_arg->boot_reason = tags->u.boot_reason.boot_reason;
                    break;
                case BOOT_TAG_BOOT_MODE:
                    g_boot_arg->boot_mode = tags->u.boot_mode.boot_mode;
                    break;
                case BOOT_TAG_META_COM:
                    g_boot_arg->meta_com_type = tags->u.meta_com.meta_com_type;
                    g_boot_arg->meta_com_id = tags->u.meta_com.meta_com_id;
                    g_boot_arg->meta_uart_port = tags->u.meta_com.meta_uart_port;
                    break;
                case BOOT_TAG_LOG_COM:
                    g_boot_arg->log_port = tags->u.log_com.log_port;
                    g_boot_arg->log_baudrate = tags->u.log_com.log_baudrate;
                    g_boot_arg->log_enable = tags->u.log_com.log_enable;
                    break;
                case BOOT_TAG_MEM:
                    g_boot_arg->dram_rank_num = tags->u.mem.dram_rank_num;
                    for (i = 0; i < tags->u.mem.dram_rank_num; i++) {
                        g_boot_arg->dram_rank_size[i] = tags->u.mem.dram_rank_size[i];
                    }
                    g_boot_arg->mblock_info = tags->u.mem.mblock_info;
                    g_boot_arg->orig_dram_info = tags->u.mem.orig_dram_info;
                    g_boot_arg->lca_reserved_mem = tags->u.mem.lca_reserved_mem;
                    g_boot_arg->tee_reserved_mem = tags->u.mem.tee_reserved_mem;
                    break;
                case BOOT_TAG_MD_INFO:
                    for (i = 0; i < 4; i++) {
                        g_boot_arg->md_type[i] = tags->u.md_info.md_type[i];
                    }
                    break;
                case BOOT_TAG_BOOT_TIME:
                    g_boot_arg->boot_time = tags->u.boot_time.boot_time;
                    break;
                case BOOT_TAG_DA_INFO:
                    memcpy(&g_boot_arg->da_info, &tags->u.da_info.da_info, sizeof(da_info_t));
                    break;
                case BOOT_TAG_SEC_INFO:
                    memcpy(&g_boot_arg->sec_limit, &tags->u.sec_info.sec_limit, sizeof(SEC_LIMIT));
                    break;
                case BOOT_TAG_PART_NUM:
                    g_boot_arg->part_num = tags->u.part_num.part_num;
                    break;
                case BOOT_TAG_PART_INFO:
                    g_boot_arg->part_info = tags->u.part_info.part_info;  /* only copy the pointer but the contains*/
                    break;
                case BOOT_TAG_EFLAG:
                    g_boot_arg->e_flag = tags->u.eflag.e_flag;
                    break;
                case BOOT_TAG_DDR_RESERVE:
                    g_boot_arg->ddr_reserve_enable = tags->u.ddr_reserve.ddr_reserve_enable;
                    g_boot_arg->ddr_reserve_success = tags->u.ddr_reserve.ddr_reserve_success;
                    break;
                case BOOT_TAG_DRAM_BUF:
                    g_boot_arg->dram_buf_size = tags->u.dram_buf.dram_buf_size;
                    break;
                case BOOT_TAG_BOOT_OPT:
                    g_boot_arg->smc_boot_opt = tags->u.boot_opt.smc_boot_opt;
                    g_boot_arg->lk_boot_opt = tags->u.boot_opt.lk_boot_opt;
                    g_boot_arg->kernel_boot_opt = tags->u.boot_opt.kernel_boot_opt;
                    break;
                case BOOT_TAG_SRAM_INFO:
                    g_boot_arg->non_secure_sram_addr = tags->u.sram_info.non_secure_sram_addr;
                    g_boot_arg->non_secure_sram_size = tags->u.sram_info.non_secure_sram_size;
                    break;
                case BOOT_TAG_PTP:                   
                    memcpy(&g_boot_arg->ptp_volt_info, &tags->u.ptp_volt.ptp_volt_info, sizeof(ptp_info_t));
                    break;
	// HTC_CSP_START, #21487(Dybert_Wang), Add
	#if _MAKE_HTC_LK
		case BOOT_TAG_PL_INFO:
                    memset(g_boot_arg->htc_pl_version, 0, sizeof(g_boot_arg->htc_pl_version));
                    memset(g_boot_arg->htc_pl_commit_id, 0, sizeof(g_boot_arg->htc_pl_commit_id));
                    memcpy(g_boot_arg->htc_pl_version, &tags->u.htc_pl_info.htc_pl_version, sizeof(g_boot_arg->htc_pl_version)-1);
                    memcpy(g_boot_arg->htc_pl_commit_id, &tags->u.htc_pl_info.htc_pl_commit_id, sizeof(g_boot_arg->htc_pl_commit_id)-1);

                    memset(g_boot_arg->meid, 0, sizeof(g_boot_arg->meid));
                    memcpy(g_boot_arg->meid, &tags->u.htc_pl_info.meid, sizeof(g_boot_arg->meid));

                    memset(g_boot_arg->htc_sign_key_id, 0, sizeof(g_boot_arg->htc_sign_key_id));
                    memcpy(g_boot_arg->htc_sign_key_id, &tags->u.htc_pl_info.htc_sign_key_id, sizeof(g_boot_arg->htc_sign_key_id)-1);

                    break;
	#endif
	// HTC_CSP_END

                default:
                    break;
		    }
	    }
    }


#ifdef MACH_FPGA
    g_nr_bank = 2;
    bi_dram[0].start = DRAM_PHY_ADDR + RIL_SIZE;
    bi_dram[0].size = (256 * 1024 * 1024) - RIL_SIZE;
    bi_dram[1].start = bi_dram[0].start + bi_dram[0].size;
    bi_dram[1].size = (256 * 1024 * 1024);
#else
    g_nr_bank = g_boot_arg->dram_rank_num;

    if (g_nr_bank == 0 || g_nr_bank > MAX_NR_BANK)
    {
        g_dram_init_ret = -1;
        //dprintf(CRITICAL, "[LK ERROR] DRAM bank number is not correct!!!");
        //while (1) ;
        return -1;
    }

/* return the actual DRAM info */
    bi_dram[0].start = DRAM_PHY_ADDR + RIL_SIZE;
    bi_dram[0].size = g_boot_arg->dram_rank_size[0] - RIL_SIZE;
    for (i = 1; i < g_nr_bank; i++)
    {
        bi_dram[i].start = bi_dram[i-1].start + bi_dram[i-1].size;
        bi_dram[i].size = g_boot_arg->dram_rank_size[i];
    }

#endif

    return 0;
}

/*******************************************************
 * Routine: memory_size
 * Description: return DRAM size to LCM driver
 ******************************************************/
u64 physical_memory_size(void)
{
    int i;
    unsigned long long size = 0;

    for (i = 0; i < g_nr_bank; i++)
        size += bi_dram[i].size;
    size += RIL_SIZE;

    return size;
}

u32 memory_size(void)
{
    unsigned long long size = physical_memory_size();

    while (((unsigned long long)DRAM_PHY_ADDR + size) > 0x100000000ULL)
    {
        size -= (unsigned long long)(1024*1024*1024);
    }

    return (unsigned int)size;
}

void sw_env()
{

    dprintf(CRITICAL, "[%s:%d] starts\n", __func__, __LINE__);

#ifdef LK_DL_CHECK
#ifdef MTK_EMMC_SUPPORT
    int dl_status = 0;
    dl_status = mmc_get_dl_info();
    dprintf(INFO, "mt65xx_sw_env--dl_status: %d\n", dl_status);
    if (dl_status != 0)
    {
        video_printf("=> TOOL DL image Fail!\n");
        dprintf(CRITICAL, "TOOL DL image Fail\n");
#ifdef LK_DL_CHECK_BLOCK_LEVEL
        dprintf(CRITICAL, "uboot is blocking by dl info\n");
        while (1) ;
#endif
    }
#endif
#endif

#ifndef USER_BUILD
    switch (g_boot_mode)
    {
        case META_BOOT:
            video_printf(" => META MODE\n");
            break;
        case FACTORY_BOOT:
#ifdef MTK_TC7_FEATURE
            if(ramdump_boot)
                video_printf(" => RAMDUMP MODE\n");
            else
                video_printf(" => FACTORY MODE\n");
#else
            video_printf(" => FACTORY MODE\n");
#endif
            break;
        case RECOVERY_BOOT:
            video_printf(" => RECOVERY MODE\n");
            break;
        case SW_REBOOT:
            //video_printf(" => SW RESET\n");
            break;
        case NORMAL_BOOT:
            //if(g_boot_arg->boot_reason != BR_RTC && get_env("hibboot") != NULL && atoi(get_env("hibboot")) == 1)
            if(!g_boot_menu && get_env("hibboot") != NULL && atoi(get_env("hibboot")) == 1)
                video_printf(" => HIBERNATION BOOT\n");
            else
                video_printf(" => NORMAL BOOT\n");
            break;
        case ADVMETA_BOOT:
            video_printf(" => ADVANCED META MODE\n");
            break;
        case ATE_FACTORY_BOOT:
            video_printf(" => ATE FACTORY MODE\n");
            break;
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
        case KERNEL_POWER_OFF_CHARGING_BOOT:
            video_printf(" => POWER OFF CHARGING MODE\n");
            break;
        case LOW_POWER_OFF_CHARGING_BOOT:
            video_printf(" => LOW POWER OFF CHARGING MODE\n");
            break;
#endif
        case ALARM_BOOT:
            video_printf(" => ALARM BOOT\n");
            break;
        case FASTBOOT:
            video_printf(" => FASTBOOT mode...\n");
            break;
#if _MAKE_HTC_LK
		case HTC_DOWNLOAD:
			video_printf(" => DOWNLOAD mode...\n");
			break;
		case HTC_DOWNLOAD_RUU:
			video_printf(" => RUU mode...\n");
			break;
#endif
        default:
            video_printf(" => UNKNOWN BOOT\n");
    }
    return;
#endif

#ifdef USER_BUILD
    if(g_boot_mode == FASTBOOT)
        video_printf(" => FASTBOOT mode...\n");
    return;
#endif
}

void platform_init_mmu_mappings(void)
{
    /* configure available RAM banks */
    dram_init();

    /* Enable D-cache  */
#if 1
    unsigned int addr;
    unsigned int dram_size = 0;

    dram_size = physical_memory_size();

    for (addr = 0; addr < dram_size; addr += (1024*1024))
    {
        /*virtual to physical 1-1 mapping*/
#if _MAKE_HTC_LK
        if (
                (bi_dram[0].start+addr) >= HTC_MEM_RESERVED_PHYS_ADDR &&
                bi_dram[0].start+addr < (HTC_MEM_RESERVED_PHYS_ADDR + HTC_MEM_RESERVED_SIZE) )
        {
	        arm_mmu_map_section(
                                    bi_dram[0].start+addr,bi_dram[0].start+addr,
                                    MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE | MMU_MEMORY_AP_READ_WRITE
            );
        } else {
            arm_mmu_map_section(
                                    bi_dram[0].start+addr,bi_dram[0].start+addr,
                                    MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE | MMU_MEMORY_AP_READ_WRITE);
        }
#else
        arm_mmu_map_section(bi_dram[0].start+addr,bi_dram[0].start+addr, MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE | MMU_MEMORY_AP_READ_WRITE);
#endif
    }
#endif
}

void platform_init_mmu(void)
{
    unsigned long long addr;
    unsigned int vaddr;
    unsigned long long dram_size;

    /* configure available RAM banks */
    dram_init();

    dram_size = physical_memory_size();

    if(((unsigned long long)DRAM_PHY_ADDR + dram_size) <= 0x100000000ULL)
    {
        arm_mmu_init();

        for (addr = 0; addr < dram_size; addr += (1024*1024))
        {
            /*virtual to physical 1-1 mapping*/
            arm_mmu_map_section(bi_dram[0].start+addr, bi_dram[0].start+addr, MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE | MMU_MEMORY_AP_READ_WRITE);
        }
    }
    else
    {
        arm_mmu_lpae_init();

        for (addr = 0; addr < dram_size; addr += (unsigned long long)(1024*1024*1024))
        {
            vaddr = (bi_dram[0].start + addr < 0x100000000ULL) ? (unsigned int)(bi_dram[0].start + addr) : (0xC0000000);
            arm_mmu_map_block(bi_dram[0].start+addr, vaddr, LPAE_MMU_MEMORY_TYPE_NORMAL_WRITE_BACK);
        }

        arch_enable_mmu();
    }
}


void platform_k64_check(void)
{
    dprintf(CRITICAL, "kernel_boot_opt=%d\n", g_boot_arg->kernel_boot_opt);

    switch(g_boot_arg->kernel_boot_opt){
    case BOOT_OPT_64S3:
    case BOOT_OPT_64S1:
    case BOOT_OPT_64N2:
    case BOOT_OPT_64N1:
        g_is_64bit_kernel = 1;
        dprintf(CRITICAL, "64Bit Kernel\n");
        break;

    case BOOT_OPT_32S3:
    case BOOT_OPT_32S1:
    case BOOT_OPT_32N2:
    case BOOT_OPT_32N1:
        /* maybe need to do something in the feature*/
    default:
        g_is_64bit_kernel = 0;
        dprintf(CRITICAL, "32Bit Kernel\n");
        break;
    }
}

// boot vibration
#if _MAKE_HTC_LK
static void htc_boot_on_vibrate()
{
    vib_turn_on(200);
}
#endif

void platform_early_init(void)
{
#ifdef MT_SRAM_REPAIR_SUPPORT
    int repair_ret;
#endif
#ifdef LK_PROFILING
#ifdef MT_SRAM_REPAIR_SUPPORT
    unsigned int time_repair_sram;
#endif
    unsigned int time_wdt_early_init;
    unsigned int time_led_init;
    unsigned int time_pmic_init;
    unsigned int time_platform_early_init;
#endif
    zzytest_printf("zzytest, begin platform_early_init\n");
    platform_init_interrupts();

    platform_early_init_timer();

#if defined(MACH_FPGA) || defined(SB_LK_BRINGUP)
    mtk_timer_init();  // GPT4 will be initialized at PL after
#endif

#ifdef LK_PROFILING
    time_platform_early_init = get_timer(0);
#endif

#ifndef MACH_FPGA
#ifndef DUMMY_AP
    mt_gpio_set_default();
#endif
#endif

    /* initialize the uart */
    uart_init_early();
    dprintf(CRITICAL, "zzytest, after uart_init_early in function: platform_early_init");
#ifdef MT_SRAM_REPAIR_SUPPORT
    dprintf(CRITICAL, "zzytest, MT_SRAM_REPAIR_SUPPORT\n");
#endif

#if 0
    {
        int idx;
        unsigned int *ptr;
        for(ptr=(unsigned int *)BOOT_ARGUMENT_LOCATION, idx=0; idx < 512; ptr++, idx+=4)
        {
            if(idx%32==0) printf("\n");
            printf("0x%08X ", *ptr);
        }
        printf("\n");

        printf("size of BOOT_ARGUMENT = %u\n", sizeof(BOOT_ARGUMENT));
        for(ptr=(unsigned int *)g_boot_arg, idx=0; idx < (sizeof(BOOT_ARGUMENT)+3); ptr++, idx+=4)
        {
            if(idx%32==0) printf("\n");
            printf("0x%08X ", *ptr);
        }
        printf("\n");
    }
#endif

    platform_k64_check();

#ifdef LK_PROFILING
    time_wdt_early_init = get_timer(0);
#endif
    mtk_wdt_init();
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- WDT Init  takes %d ms -------- \n", (int)get_timer(time_wdt_early_init));
#endif

    if(g_dram_init_ret < 0)
    {
	    dprintf(CRITICAL, "[LK ERROR] DRAM bank number is not correct!!!\n");
	    while(1) ;
    }

    //i2c_v1_init();

#ifdef MT_SRAM_REPAIR_SUPPORT
#ifdef LK_PROFILING
    time_repair_sram = get_timer(0);
#endif
    repair_ret = repair_sram();
    if(repair_ret != 0)
    {
        dprintf(CRITICAL, "Sram repair failed %d\n", repair_ret);
        while(1);
    }
#ifdef LK_PROFILING
        dprintf(INFO, "[PROFILE] ------- Repair SRAM takes %d ms -------- \n", (int)get_timer(time_repair_sram));
#endif
#endif

    //i2c init
    i2c_hw_init();

#if defined(MACH_FPGA) || defined(SB_LK_BRINGUP)
    //mtk_timer_init();  // GPT4 will be initialized at PL after
    mtk_wdt_disable();  // WDT will be triggered when uncompressing linux image on FPGA
    *(volatile unsigned int *)0x10212000 = 0x22000064;
#endif

#if MTK_MT8193_SUPPORT
    mt8193_init();
#endif

#ifndef MACH_FPGA
#ifdef LK_PROFILING
    time_led_init = get_timer(0);
#endif
    leds_init();
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- led init takes %d ms -------- \n", (int)get_timer(time_led_init));
#endif
#endif
    // Workaround by Peng
    //pwrap_init_lk();
    //pwrap_init_for_early_porting();

#ifdef LK_PROFILING
    time_pmic_init = get_timer(0);
#endif
    pmic_init();
/*
    // Workaround by Weiqi
    mt6331_upmu_set_rg_vgp1_en(1);
    mt6331_upmu_set_rg_vcam_io_en(1);
*/
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- pmic_init takes %d ms -------- \n", (int)get_timer(time_pmic_init));
#endif

#if _MAKE_HTC_LK    // boot vibration
    htc_boot_on_vibrate();
#endif

#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- platform_early_init takes %d ms -------- \n", (int)get_timer(time_platform_early_init));
#endif
}

extern void mt65xx_bat_init(void);
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)

int kernel_charging_boot(void)
{
	if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_TRUE)
	{
		dprintf(INFO,"[%s] Kernel Power Off Charging with Charger/Usb \n", __func__);
		return  1;
	}
	else if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_FALSE)
	{
		dprintf(INFO,"[%s] Kernel Power Off Charging without Charger/Usb \n", __func__);
		return -1;
	}
	else
		return 0;
}
#endif

void platform_init(void)
{
#ifdef LK_PROFILING
    unsigned int time_nand_emmc;
    unsigned int time_env;
    unsigned int time_disp_init;
    unsigned int time_load_logo;
    unsigned int time_backlight;
    unsigned int time_boot_mode;
#ifdef MTK_SECURITY_SW_SUPPORT
    unsigned int time_security_init;
#endif
    unsigned int time_bat_init;
    unsigned int time_RTC_boot_Check;
    unsigned int time_show_logo;
    unsigned int time_sw_env;
    unsigned int time_platform_init;
    time_platform_init = get_timer(0);
#endif

    dprintf(CRITICAL, "zzytest, platform_init begin\n");

#ifdef DUMMY_AP
    dummy_ap_entry();
#endif

#ifdef LK_PROFILING
    time_nand_emmc = get_timer(0);
#endif
#ifdef MTK_EMMC_SUPPORT
    mmc_legacy_init(1);
#else
#ifndef MACH_FPGA
    nand_init();
    nand_driver_test();
#endif
#endif
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- NAND/EMMC init takes %d ms -------- \n", (int)get_timer(time_nand_emmc));
#endif

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    if((g_boot_arg->boot_reason == BR_USB) && (upmu_is_chr_det() == KAL_FALSE))
    {
        dprintf(INFO, "[%s] Unplugged Charger/Usb between Pre-loader and Uboot in Kernel Charging Mode, Power Off \n", __func__);
        mt6575_power_off();
    }
#endif

#ifdef LK_PROFILING
    time_env = get_timer(0);
#endif
    env_init();
    print_env();
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- ENV init takes %d ms -------- \n", (int)get_timer(time_env));
#endif

#if _MAKE_HTC_LK
// HTC_CSP_START, #21487(Dybert_Wang), Add
// move from below section
#ifdef MTK_SECURITY_SW_SUPPORT
#ifdef LK_PROFILING
    time_security_init = get_timer(0);
#endif
		/* initialize security library */
#ifdef MTK_EMMC_SUPPORT
    #ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        sec_func_init(3);
    #else
        sec_func_init(1);
    #endif
#else
    sec_func_init(0);
#endif
#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- Security init takes %d ms -------- \n", (int)get_timer(time_security_init));
#endif
#endif // MTK_SECURITY_SW_SUPPORT
// HTC_CSP_END

	/* print HTC LK Version Info */
	dprintf(CRITICAL,"HTC PL version: %s\n", g_boot_arg->htc_pl_version);
	dprintf(CRITICAL,"HTC PL commit-id: %s\n", g_boot_arg->htc_pl_commit_id);
	dprintf(CRITICAL,"HTC LK version: %s\n", HTC_LK_VERSION);
	dprintf(CRITICAL,"HTC LK build mode: %s\n", HTC_LK_BUILD_MODE);
	dprintf(CRITICAL,"HTC LK commit-id: %s\n", HTC_LK_COMMIT_ID);
    
    // HTC_CSP_START, #21487(Dybert_Wang), Add
    dprintf(CRITICAL,"HTC MEID: %X %X %X %X\n", g_boot_arg->meid[0], g_boot_arg->meid[1], g_boot_arg->meid[2], g_boot_arg->meid[3]);
    dprintf(CRITICAL,"HTC sign key id = %s\n", g_boot_arg->htc_sign_key_id);
    // sanity check pg1fs and pg2fs, reconstruct them if any problem
    dprintf(CRITICAL, "starting checking pgfs in htc_check_pgfs()\n");
    htc_check_pgfs();
    htc_security_init();
    dprintf(CRITICAL, "htc security flag: %d, smart card magic = %X\n", read_security_level(), read_sec_ex_magic(MAGIC_TYPE_SMART_SD));
    // HTC_CSP_END

    /* HTC: initialize board_info before ramdump mdoe */
    htc_board_info_init();
    dprintf(CRITICAL, "The htc pid is [%d]\n", htc_get_pid());


    htc_reboot_info_init(HTC_REBOOT_INFO_PHYS_ADDR, HTC_REBOOT_INFO_EMMC_PART_NAME, HTC_REBOOT_INFO_EMMC_PART_OFFSET);
    dprintf(CRITICAL, "htc_reboot_reason: 0x%08X\n", htc_get_reboot_reason());
    dprintf(CRITICAL, "htc_reboot_string: %s\n", htc_get_reboot_string());

    htc_setting_init();
    dprintf(CRITICAL, "The htc pcb id is [%d]\n", htc_setting_pcbid());

    htc_setting_init_bootmode();
    dprintf(CRITICAL, "main firmware version: %s\n", htc_setting_get_firmware_main_version());
#if defined(HTC_BANK28_SUPPORT)
	htc_open_b28();
	dprintf(CRITICAL, "htc_open_B28\n");
#else
	dprintf(CRITICAL, "htc_unopen_B28\n");
#endif
#endif


#ifdef LK_PROFILING
    time_disp_init = get_timer(0);
#endif

/* initialize the frame buffet information */
#ifndef MACH_FPGA_NO_DISPLAY
    g_fb_size = mt_disp_get_vram_size();
#else
    g_fb_size = 0x1000000;
#endif
#if 0
    g_fb_base = memory_size() - g_fb_size + DRAM_PHY_ADDR;
#else

#if 0
    if (g_is_64bit_kernel) {
        g_fb_base = mblock_reserve(&g_boot_arg->mblock_info, g_fb_size, 0x200000, 0x100000000, RANKMAX);
        g_fb_base = ALIGN_TO(g_fb_base,0x200000); // size 2MB align
    }
    else {
        g_fb_base = mblock_reserve(&g_boot_arg->mblock_info, g_fb_size, 0x100000, 0x100000000, RANKMAX);
    }
#else
     g_fb_base = mblock_reserve(&g_boot_arg->mblock_info, g_fb_size, 0x10000, 0x100000000, RANKMAX);
#endif

    if (!g_fb_base) {
        /* ERROR */
    }
#endif

  dprintf(CRITICAL, "FB base = 0x%x, FB size = %d\n", g_fb_base, g_fb_size);

#ifndef MACH_FPGA_NO_DISPLAY
    mt_disp_init((void *)g_fb_base);
    /* show black picture fisrtly in case of  backlight is on before nothing is drawed*/
    //mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
    //mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- disp init takes %d ms -------- \n", (int)get_timer(time_disp_init));
#endif

#ifdef LK_PROFILING
    time_load_logo = get_timer(0);
#endif
    drv_video_init();

    mboot_common_load_logo((unsigned long)mt_get_logo_db_addr_pa(), "logo");
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- load_logo takes %d ms -------- \n", (int)get_timer(time_load_logo));
#endif
#endif

    /*for kpd pmic mode setting*/
    set_kpd_pmic_mode();

#ifdef MTK_EFUSE_WRITER_SUPPORT 
    mt_efuse_get();
#endif

#ifndef MACH_FPGA
#ifdef LK_PROFILING
    time_boot_mode = get_timer(0);
#endif
    boot_mode_select();

#if _MAKE_HTC_LK
	/** sync hTC BOOT MODE to MTK BOOT MODE **/
	if (g_boot_mode == NORMAL_BOOT) {
		switch(htc_setting_get_bootmode()) {
		case BOOTMODE_FASTBOOT:      g_boot_mode = FASTBOOT;break;
		case BOOTMODE_RECOVERY:      g_boot_mode = RECOVERY_BOOT;break;
		case BOOTMODE_FTM1:          g_boot_mode = META_BOOT;break;
		case BOOTMODE_FTM2:          g_boot_mode = FACTORY_BOOT;break;
		case BOOTMODE_FTM3:          g_boot_mode = META_BOOT;break;
		case BOOTMODE_DOWNLOAD:      g_boot_mode = HTC_DOWNLOAD;break;
		case BOOTMODE_DOWNLOAD_RUU:  g_boot_mode = HTC_DOWNLOAD_RUU;break;
		}
	}
#endif
	
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- boot mode select takes %d ms -------- \n", (int)get_timer(time_boot_mode));
#endif
#endif

// HTC_CSP_START, #21487(Dybert_Wang), Modify
#if _MAKE_HTC_LK
    // do nothing, move below section to above
#else
#ifdef MTK_SECURITY_SW_SUPPORT
#ifdef LK_PROFILING
    time_security_init = get_timer(0);
#endif
    /* initialize security library */
#ifdef MTK_EMMC_SUPPORT
    #ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        sec_func_init(3);
    #else
        sec_func_init(1);
    #endif
#else
    sec_func_init(0);
#endif
#ifdef LK_PROFILING
    dprintf(INFO,"[PROFILE] ------- Security init takes %d ms -------- \n", (int)get_timer(time_security_init));
#endif
#endif
#endif // #if _MAKE_HTC_LK
// HTC_CSP_END

    /*Show download logo & message on screen */
    if (g_boot_arg->boot_mode == DOWNLOAD_BOOT)
    {
        dprintf(CRITICAL, "[LK] boot mode is DOWNLOAD_BOOT\n");

#ifdef MTK_SECURITY_SW_SUPPORT
        /* verify da before jumping to da*/
	// HTC_CSP_START, #21487(Dybert_Wang), Modify
	#if _MAKE_HTC_LK
	if (setting_security()) { // if S-on
	#else
        if (sec_usbdl_enabled()) {
	#endif
	// HTC_CSP_END
            u8  *da_addr = (u8*)g_boot_arg->da_info.addr;
            u32 da_len   = g_boot_arg->da_info.len;
            u32 sig_len  = g_boot_arg->da_info.sig_len;
            u8  *sig_addr = (unsigned char *)da_addr + (da_len - sig_len);

            if (da_len == 0 || sig_len == 0) {
                dprintf(INFO, "[LK] da argument is invalid\n");
                dprintf(INFO, "da_addr = 0x%x\n", (int)da_addr);
                dprintf(INFO, "da_len  = 0x%x\n", da_len);
                dprintf(INFO, "sig_len = 0x%x\n", sig_len);
            }

            if (sec_usbdl_verify_da(da_addr, (da_len - sig_len), sig_addr, sig_len)) {
                /* da verify fail */
                video_printf(" => Not authenticated tool, download stop...\n");
                while(1); /* fix me, should not be infinite loop in lk */
            }
        }
        else
#endif
        {
            dprintf(INFO, " DA verification disabled...\n");
        }

#ifndef MACH_FPGA_NO_DISPLAY
        mt_disp_show_boot_logo();
#endif
        video_printf(" => Downloading...\n");
        dprintf(CRITICAL, "enable backlight after show bootlogo! \n");
        mt65xx_backlight_on();

        mtk_wdt_disable(); //Disable wdt before jump to DA
        platform_uninit();
#ifdef HAVE_CACHE_PL310
        l2_disable();
#endif
        arch_disable_cache(UCACHE);
        arch_disable_mmu();
#ifdef ENABLE_L2_SHARING
        config_shared_SRAM_size();
#endif

        jump_da(g_boot_arg->da_info.addr, g_boot_arg->da_info.arg1, g_boot_arg->da_info.arg2);
    }

#ifdef LK_PROFILING
    time_bat_init = get_timer(0);
#endif
    mt65xx_bat_init();
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- battery init takes %d ms -------- \n", (int)get_timer(time_bat_init));
#endif

#ifndef CFG_POWER_CHARGING
#ifdef LK_PROFILING
    time_RTC_boot_Check = get_timer(0);
#endif
    /* NOTE: if define CFG_POWER_CHARGING, will rtc_boot_check() in mt65xx_bat_init() */
    rtc_boot_check(false);
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- RTC boot check Init  takes %d ms -------- \n", (int)get_timer(time_RTC_boot_Check));
#endif
#endif

#ifdef LK_PROFILING
    time_show_logo = get_timer(0);
#endif
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if(kernel_charging_boot() == 1)
	{
		#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
		CHARGER_TYPE CHR_Type_num = CHARGER_UNKNOWN;
		CHR_Type_num = hw_charging_get_charger_type();
		if ((g_boot_mode != LOW_POWER_OFF_CHARGING_BOOT) ||
		((CHR_Type_num != STANDARD_HOST) && (CHR_Type_num != NONSTANDARD_CHARGER)))
		{
		#endif
		mt_disp_power(TRUE);
		mt_disp_show_low_battery();
		mt65xx_leds_brightness_set(6, 110);
		#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
		}
		#endif
	}
	else if(g_boot_mode != KERNEL_POWER_OFF_CHARGING_BOOT && g_boot_mode != LOW_POWER_OFF_CHARGING_BOOT)
	{
		if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
		{
#ifndef MACH_FPGA_NO_DISPLAY
			mt_disp_show_boot_logo();
#endif
		}
	}
#else
    if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
    {
#ifndef MACH_FPGA_NO_DISPLAY
        mt_disp_show_boot_logo();
#endif
    }
#endif
#ifdef LK_PROFILING
    time_backlight = get_timer(0);
#endif

#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
    if(!is_low_battery(0))
    {
#endif
        mt65xx_backlight_on();
#ifndef MACH_FPGA_NO_DISPLAY
        //pwm need display sof
        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
#endif
#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
    }
#endif

#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- backlight takes %d ms -------- \n", (int)get_timer(time_backlight));
#endif

#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- show logo takes %d ms -------- \n", (int)get_timer(time_show_logo));
#endif

#ifndef MACH_FPGA
#ifdef LK_PROFILING
    time_sw_env = get_timer(0);
#endif
    sw_env();
#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- sw_env takes %d ms -------- \n", (int)get_timer(time_sw_env));
#endif
#endif

#ifdef LK_PROFILING
    dprintf(INFO, "[PROFILE] ------- platform_init takes %d ms -------- \n", (int)get_timer(time_platform_init));
#endif
}

void platform_uninit(void)
{
    platform_deinit_interrupts();
    return;
}

#ifdef ENABLE_L2_SHARING
#define ADDR_CA7L_CACHE_CONFIG_MP(x) (CA7MCUCFG_BASE + 0x200 * x)
#define L2C_SIZE_CFG_OFFSET  8
#define L2C_SHARE_EN_OFFSET  12
/* 4'b1111: 2048KB(not support)
 * 4'b0111: 1024KB(not support)
 * 4'b0011: 512KB
 * 4'b0001: 256KB
 * 4'b0000: 128KB (not support)
 */

int is_l2_need_config(void)
{
    volatile unsigned int cache_cfg, addr;

    addr = ADDR_CA7L_CACHE_CONFIG_MP(0);
    cache_cfg = DRV_Reg32(addr);
    cache_cfg = cache_cfg >> L2C_SIZE_CFG_OFFSET;

    /* only read 256KB need to be config.*/
    if((cache_cfg &(0x7)) == 0x1)
    {
        return 1;
    }
    return 0;
}

void cluster_l2_share_enable(int cluster)
{
    volatile unsigned int cache_cfg, addr;

    addr = ADDR_CA7L_CACHE_CONFIG_MP(cluster);
    /* set L2C size to 256KB */
    cache_cfg = DRV_Reg32(addr);
    cache_cfg &= (~0x7) << L2C_SIZE_CFG_OFFSET;
    cache_cfg |= 0x1 << L2C_SIZE_CFG_OFFSET;

    /* enable L2C_share_en. Sram only for other to use*/
    cache_cfg |= (0x1 << L2C_SHARE_EN_OFFSET);
    DRV_WriteReg32(addr, cache_cfg);
}

void cluster_l2_share_disable(int cluster)
{
    volatile unsigned int cache_cfg, addr;

    addr = ADDR_CA7L_CACHE_CONFIG_MP(cluster);
    /* set L2C size to 512KB */
    cache_cfg = DRV_Reg32(addr);
    cache_cfg &= (~0x7) << L2C_SIZE_CFG_OFFSET;
    cache_cfg |= 0x3 << L2C_SIZE_CFG_OFFSET;
    DRV_WriteReg32(addr, cache_cfg);

    /* disable L2C_share_en. Sram only for cpu to use*/
    cache_cfg &= ~(0x1 << L2C_SHARE_EN_OFFSET);
    DRV_WriteReg32(addr, cache_cfg);
}

/* config L2 cache and sram to its size */
void config_L2_size(void)
{
    int cluster;

    if(is_l2_need_config())
    {
        /*
         * Becuase mcu config is protected.
         * only can write in secutity mode
         */

        if(dev_info_nr_cpu() == 6)
        {
            cluster_l2_share_disable(0);
            cluster_l2_share_enable(1);
        }

        else
        {
            for(cluster = 0; cluster < 2; cluster++)
            {
                cluster_l2_share_disable(cluster);
            }
        }
    }
}

/* config SRAM back from L2 cache for DA relocation */
void config_shared_SRAM_size(void)
{
    int cluster;

    if(is_l2_need_config())
    {
        /*
         * Becuase mcu config is protected.
         * only can write in secutity mode
         */

        for(cluster = 0; cluster < 2; cluster++)
        {
            cluster_l2_share_enable(cluster);
        }
    }
}
#endif

#ifdef NEW_MEMORY_RESERVED_MODEL
/*
 * reserve a memory from mblock
 * @mblock_info: address of mblock_info
 * @size: size of memory
 * @align: alignment, not implemented
 * @limit: address limit. Must higher than return address + size
 * @rank: preferable rank, the returned address is in rank or lower ranks
 * It returns as high rank and high address as possible. (consider rank first)
 */
u64 mblock_reserve(mblock_info_t *mblock_info, u64 size, u64 align, u64 limit,
                   enum reserve_rank rank)
{
	int i, max_rank, target = -1;
	u64 start, sz, max_addr = 0;

	if (size & (0x200000 - 1)) {
		dprintf(CRITICAL,"warning: size is not 2MB aligned\n");
	}

	if (rank == RANK0) {
		/* reserve memory from rank 0 */
		max_rank = 0;
	} else {
		/* reserve memory from any possible rank */
		/* mblock_num >= nr_ranks is true */
		max_rank = mblock_info->mblock_num - 1;
	}

	for (i = 0; i < mblock_info->mblock_num; i++) {
		start = mblock_info->mblock[i].start;
		sz = mblock_info->mblock[i].size;
		dprintf(CRITICAL,"mblock[%d].start: 0x%llx, sz: 0x%llx, limit: 0x%llx, "
				"max_addr: 0x%llx, max_rank: %d, target: %d, "
				"mblock[].rank: %d\n",
				i, start, sz, limit, max_addr, max_rank,
				target, mblock_info->mblock[i].rank);
		dprintf(CRITICAL,"mblock_reserve dbg[%d]: %d, %d, %d, %d\n",
				i, (start + sz <= limit),
				(mblock_info->mblock[i].rank <= max_rank),
				(start + sz > max_addr),
                (sz >= size));
		if ((start + sz <= limit) &&
			(mblock_info->mblock[i].rank <= max_rank) &&
			(start + sz > max_addr) &&
			(sz >= size)) {
			max_addr = start + sz;
			target = i;
		}
	}

	if (target < 0) {
		dprintf(CRITICAL,"mblock_reserve error\n");
		return 0;
	}

	mblock_info->mblock[target].size -= size;

	dprintf(CRITICAL,"mblock_reserve: %llx - %llx from mblock %d\n",
			(mblock_info->mblock[target].start
             + mblock_info->mblock[target].size),
			(mblock_info->mblock[target].start
             + mblock_info->mblock[target].size + size),
			target);


	return mblock_info->mblock[target].start +
		mblock_info->mblock[target].size;
}
#endif //NEW_MEMORY_RESERVED_MODEL

int platform_skip_hibernation(void)
{
	if (g_boot_menu == true)
		return 1;

	switch (g_boot_arg->boot_reason) {
#if 0 // let schedule power on to go hiberantion bootup process
	case BR_RTC:
#endif
	case BR_WDT:
	case BR_WDT_BY_PASS_PWK:
	case BR_WDT_SW:
	case BR_WDT_HW:
		return 1;
	}

	return 0;
}
