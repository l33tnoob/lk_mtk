#include <malloc.h>
#ifdef MTK_GPT_SCHEME_SUPPORT
#include <platform/partition.h>
#else
#include <mt_partition.h>
#endif
#include <printf.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <video.h>
#include <dev/mrdump.h>
#include <platform/mtk_key.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_gpt.h>
#include <target/cust_key.h>
#include <platform/boot_mode.h>
#include <platform/ram_console.h>
#include <arch/ops.h>

#include "aee.h"
#include "kdump.h"

extern BOOT_ARGUMENT *g_boot_arg;

#define MRDUMP_DELAY_TIME 10

/* HTC++ */
#if _MAKE_HTC_LK
#include <htc_board_info_and_setting.h>
#include <target/cust_usb.h>
#include<target.h>
#include <target/cust_usb.h>
#include <dev/udc.h>
#include "../fastboot.h"
#include <reg.h>
#include <htc_ramdump.h>

#define DEFAULT_SERIAL_NUM "0123456789ABCDEF"

/*
 * Support read barcode from /dev/pro_info to be serial number.
 * Then pass the serial number from cmdline to kernel.
 */

#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL) || (defined(MTK_SECURITY_SW_SUPPORT) && defined(MTK_SEC_FASTBOOT_UNLOCK_SUPPORT))
#define SERIALNO_LEN    38      /* from preloader */
static char sn_buf[SN_BUF_LEN+1] = ""; /* will read from EFUSE_CTR_BASE */
#else
#define SERIALNO_LEN    38
static char sn_buf[SN_BUF_LEN+1] = FASTBOOT_DEVNAME;
#endif

static struct udc_device surf_udc_device = {
        .vendor_id      = USB_VENDORID,
        .product_id     = USB_PRODUCTID,
        .version_id     = USB_VERSIONID,
        .manufacturer   = USB_MANUFACTURER,
        .product        = USB_PRODUCT_NAME,
};

const ramdump_e ramdump_regions[] =
{
        {"No_Delete.rdmp", RAMDUMP_ND_RDMP},
};

struct ramdump_file_list filelist[] =
{
        {"No_Delete.rdmp", 0},
};


int ramdump_regions_num = ARRAY_SIZE(ramdump_regions);

AEE2EMMC_STATUS aee2emmc_status = AEE2EMMC_UNSTART ;

#endif
/* HTC-- */
#define GREEN_C       0x00FF00
struct mrdump_cblock_result *cblock_result = NULL;

static void voprintf(char type, const char *msg, va_list ap)
{
    char msgbuf[128], *p;

    p = msgbuf;
    if (msg[0] == '\r') {
        *p++ = msg[0];
        msg++;
    }

    *p++ = type;
    *p++ = ':';
    vsnprintf(p, sizeof(msgbuf) - (p - msgbuf), msg, ap);
    switch (type) {
    case 'H':	
    case 'I':
    case 'W':
    case 'E':
#if _MAKE_HTC_LK
	display_sd_debug_info(msgbuf, GREEN_C);
#else
	video_printf("%s", msgbuf);
#endif
        break;
    }

    dprintf(CRITICAL,"%s", msgbuf);
    
    /* Write log buffer */
    p = msgbuf;
    while ((*p != 0) && (cblock_result->log_size < sizeof(cblock_result->log_buf))) {
	cblock_result->log_buf[cblock_result->log_size] = *p++;
	cblock_result->log_size++;
    }
}

void voprintf_verbose(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('V', msg, ap);
    va_end(ap);
}

void voprintf_debug(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('D', msg, ap);
    va_end(ap);
}

void voprintf_info(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('I', msg, ap);
    va_end(ap);
}

void voprintf_warning(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('W', msg, ap);
    va_end(ap);
}

void voprintf_error(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('E', msg, ap);
    va_end(ap);
}

void vo_show_progress(int sizeM)
{
    video_set_cursor((video_get_rows() / 4) * 3, (video_get_colums() - 22)/ 2);
    video_printf("=====================\n");
    video_set_cursor((video_get_rows() / 4) * 3 + 1, (video_get_colums() - 22)/ 2);
    video_printf(">>> Written %4dM <<<\n", sizeM);
    video_set_cursor((video_get_rows() / 4) * 3 + 2, (video_get_colums() - 22)/ 2);
    video_printf("=====================\n");
    video_set_cursor(video_get_rows() - 1, 0);

    dprintf(CRITICAL,"... Written %dM\n", sizeM);
}

static void mrdump_status(const char *status, const char *fmt, va_list ap)
{
    if (cblock_result != NULL) {
        char *dest = strcpy(cblock_result->status, status);
        dest += strlen(dest);
        *dest++ = '\n';
    
        vsnprintf(dest, sizeof(cblock_result->status) - (dest - cblock_result->status), fmt, ap);
    }
}

void mrdump_status_ok(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mrdump_status("OK", fmt, ap);
    va_end(ap);
}

void mrdump_status_none(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mrdump_status("NONE", fmt, ap);
    va_end(ap);
}

void mrdump_status_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mrdump_status("FAILED", fmt, ap);
    va_end(ap);
}

uint32_t g_aee_mode = AEE_MODE_MTK_ENG;

const const char *mrdump_mode2string(uint8_t mode)
{
  switch (mode) {
  case AEE_REBOOT_MODE_NORMAL:
    return "NORMAL-BOOT";

  case AEE_REBOOT_MODE_KERNEL_OOPS:
    return "KERNEL-OOPS";

  case AEE_REBOOT_MODE_KERNEL_PANIC:
    return "KERNEL-PANIC";

  case AEE_REBOOT_MODE_NESTED_EXCEPTION:
    return "NESTED-CPU-EXCEPTION";

  case AEE_REBOOT_MODE_WDT:
    return "HWT";

  case AEE_REBOOT_MODE_EXCEPTION_KDUMP:
    return "MANUALDUMP";

#if _MAKE_HTC_LK
  case AEE_REBOOT_MODE_MODEM_FATAL:
    return "Modem Fatal";
  case AEE_REBOOT_MODE_RAMDUMP:
    return "Force ramdump or unknown";
#endif

  default:
    return "UNKNOWN-BOOT";
  }
}

static int kdump_ui(struct mrdump_control_block *mrdump_cblock)
{
    int ret = -1;
    video_clean_screen();
    video_set_cursor(0, 0);

    mrdump_status_error("Unknown error\n");
    voprintf_info("Kdump triggerd by '%s'\n", mrdump_mode2string(mrdump_cblock->crash_record.reboot_mode));

    struct aee_timer elapse_time;
    aee_timer_init(&elapse_time);

    uint32_t total_dump_size = memory_size();
    
    aee_timer_start(&elapse_time);
    switch (mrdump_cblock->machdesc.output_device) {
    case MRDUMP_DEV_NULL:
        ret = kdump_null_output(mrdump_cblock, total_dump_size);
        break;
#if 0
    case MRDUMP_DEV_SDCARD:
        ret = kdump_sdcard_output(mrdump_cblock, total_dump_size);
        break;
#endif
    case MRDUMP_DEV_EMMC:
        ret = kdump_emmc_output(mrdump_cblock, total_dump_size);
        break;

    default:
        voprintf_error("Unknown device id %d\n", mrdump_cblock->machdesc.output_device);
    }
    aee_mrdump_flush_cblock(mrdump_cblock);
    aee_timer_stop(&elapse_time);
    
    voprintf_info("Reset count down %d ...\n", MRDUMP_DELAY_TIME);
    mtk_wdt_restart();

    int timeout = MRDUMP_DELAY_TIME;
    while(timeout-- >= 0) {
        mdelay(1000);
        mtk_wdt_restart();
	voprintf_info("\rsec %d", timeout);
    }

    video_clean_screen();
    video_set_cursor(0, 0);

   return ret;
}

#if _MAKE_HTC_LK
#include <kernel/thread.h>

/* HTC: implement fastboot mode for ramdump to USB */
extern int fastboot_init(void *base, unsigned size);
extern void htc_ramdump_menu_init(void);
extern void ramdump_menu_init(void);

void fastboot_mode();
extern void display_sd_debug_init();

void htc_ramdump2USB()
{
	thread_t *thr = NULL;


	/* HTC: reset reboot reason */
	struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
	if (mrdump_cblock != NULL) {
			writel(AEE_REBOOT_MODE_NORMAL, &(mrdump_cblock->crash_record.reboot_mode));
			arch_sync_cache_range(&(mrdump_cblock->crash_record.reboot_mode), sizeof(uint32_t));
	}

	thr = thread_create("fastboot", fastboot_mode, 0, DEFAULT_PRIORITY, 4096);
        if (!thr)
        {
                dprintf(CRITICAL, "%s: failed to create thread radump_menu\r\n", __func__);
                return;
        }
	thread_resume(thr);
}

void htc_ramdump2eMMC()
{
	int dump_file_num = ramdump_regions_num;
  int ret ;
	struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();

	mrdump_status_none("ramdump2eMMC\n");
	//display_sd_info_clr_all();
	//display_print_ramdump_files(filelist, dump_file_num);
	//display_update_screen();
	aee2emmc_status = AEE2EMMC_RUNNING ;
	ret = kdump_ui(mrdump_cblock);
  if(ret < 0)
    aee2emmc_status = AEE2EMMC_FAIL ;
  else
	  aee2emmc_status = AEE2EMMC_DONE ;
}

void fastboot_mode()
{
        unsigned usb_init = 0;
        unsigned sz = 0;

	htc_setting_set_bootmode(BOOTMODE_FASTBOOT);
	simple_menu_init(); /* display menu of fastboot according to setting->bootmode. */

        /* HTC: get SN by htc_setting_get_barcode() */
        memset( sn_buf, 0, sizeof(sn_buf));
        strncpy(sn_buf, htc_setting_get_barcode(), sizeof(sn_buf));

        sn_buf[SN_BUF_LEN] = '\0';
        surf_udc_device.serialno = sn_buf;

        target_fastboot_init();
        if(!usb_init)
                /*Hong-Rong: wait for porting*/
                udc_init(&surf_udc_device);
        sz = target_get_max_flash_size();
        fastboot_init(target_get_scratch_address(), sz);
        udc_start();

        while (1) {
                thread_sleep(1);
        }

}

unsigned char *aee_get_reboot_string()
{
	struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
	if (mrdump_cblock == NULL) 
		return NULL;
	return mrdump_cblock->crash_record.msg;
}


int mrdump_detection(void)
{
    struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
    if (mrdump_cblock == NULL) {
        return 0;
    }

    int boot_reason;
    if (g_boot_arg->maggic_number == BOOT_ARGUMENT_MAGIC){
	boot_reason = g_boot_arg->boot_reason;
	if(boot_reason == BR_POWER_KEY || boot_reason == BR_USB || boot_reason == BR_RTC)
	{
		 voprintf_debug("%s: cold boot. return. %d\n", __func__, boot_reason);
		return 0;
	}
    }

    /* return if no writeconfig 8 8 */
    if(!(htc_setting_radioflag_get()&0x8))
        return 0;

#if 0
    /* return if HTC maigc is wrong */
    unsigned int htc_reboot_reason = htc_get_reboot_reason();
    if((htc_reboot_reason != 0x776655AA) && (htc_reboot_reason != 0xAABBCCDD)){
        return 0;
    }
#endif
    memset(&mrdump_cblock->result, 0, sizeof(struct mrdump_cblock_result));
    cblock_result = &mrdump_cblock->result;

    uint8_t reboot_mode = mrdump_cblock->crash_record.reboot_mode;
    voprintf_debug("sram record with mode %d\n", reboot_mode);
    voprintf_debug("reset_string of aee: %s\n",  mrdump_cblock->crash_record.msg);	
    switch (reboot_mode) {
    case AEE_REBOOT_MODE_NORMAL:  {
        mrdump_status_none("Normal boot\n");
        return 0;
    }
    case AEE_REBOOT_MODE_KERNEL_OOPS:
    case AEE_REBOOT_MODE_KERNEL_PANIC:
    case AEE_REBOOT_MODE_NESTED_EXCEPTION:
    case AEE_REBOOT_MODE_WDT:
    case AEE_REBOOT_MODE_EXCEPTION_KDUMP:
    /* HTC defined:  */
    case AEE_REBOOT_MODE_MODEM_FATAL:
    case AEE_REBOOT_MODE_RAMDUMP:
        return 1;
    }
    return 0;
}

#else

int mrdump_detection(void)
{
    struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
    if (mrdump_cblock == NULL) {
        return 0;
    }

    memset(&mrdump_cblock->result, 0, sizeof(struct mrdump_cblock_result));
    cblock_result = &mrdump_cblock->result;

    if (strcmp(mrdump_cblock, MRDUMP_VERSION) == 0) {
	voprintf_debug("Cold boot or kernel not support MT-RAMDUMP\n");
	mrdump_status_none("Cold boot or kernel not support MT-RAMDUMP\n");
	return 0;
    }

    if (!g_boot_arg->ddr_reserve_enable) {
	voprintf_debug("DDR reserve mode disabled\n");
	mrdump_status_none("DDR reserve mode disabled\n");
	return 0;
    }

    if (!g_boot_arg->ddr_reserve_success) {
	voprintf_debug("DDR reserve mode failed\n");
	mrdump_status_none("DDR reserve mode failed\n");
	return 0;
    }
    uint8_t reboot_mode = mrdump_cblock->crash_record.reboot_mode;
    if (mrdump_cblock->machdesc.nr_cpus == 0) {
	voprintf_debug("Runtime disabled\n");
	mrdump_status_none("Runtime disabled\n");
	return 0;
    }

    voprintf_debug("sram record with mode %d\n", reboot_mode);
    switch (reboot_mode) {
    case AEE_REBOOT_MODE_NORMAL:  {
	if (!ram_console_is_abnormal_boot()) {
	    mrdump_status_none("Normal boot\n");
	    return 0;
	}
	else {
	    /* SoC trigger HW REBOOT */
	    mrdump_cblock->crash_record.reboot_mode = AEE_REBOOT_MODE_WDT;
	    return 1;
	}
    }
    case AEE_REBOOT_MODE_KERNEL_OOPS:
    case AEE_REBOOT_MODE_KERNEL_PANIC:
    case AEE_REBOOT_MODE_NESTED_EXCEPTION:
    case AEE_REBOOT_MODE_WDT:
    case AEE_REBOOT_MODE_EXCEPTION_KDUMP:
      return 1;
    }
    return 0;
}
#endif /* _MAKE_HTC_LK */


#if _MAKE_HTC_LK
int mrdump_run2(void)
{
    struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
    int ret = -1; /* ramdump is invalid */

    if (mrdump_cblock != NULL) {

	/* HTC: set ramdump mode for htc */
	htc_setting_set_bootmode(BOOTMODE_RAMDUMP);
	display_sd_debug_init();

	/* HTC: for ramdump2EMMC */
	if(htc_setting_cfgdata_get(0x7)&0x8000){
		ramdump_menu_init();
        	ret = kdump_ui(mrdump_cblock);
	}else{
	/* HTC: for ramdump2USB */
		mtk_wdt_disable(); /* disable watchdog. */
		htc_ramdump_menu_init();
		while (aee2emmc_status != AEE2EMMC_DONE && aee2emmc_status != AEE2EMMC_FAIL){
			thread_sleep(1);
		}
    if(aee2emmc_status == AEE2EMMC_DONE)
		  ret = 0 ;
	}
        return ret;
    }
    return -1;
}
#else
int mrdump_run2(void)
{
    struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
    if (mrdump_cblock != NULL) {
        kdump_ui(mrdump_cblock);
	return 1;
    }
    return 0;
}
#endif

void aee_timer_init(struct aee_timer *t)
{
    memset(t, 0, sizeof(struct aee_timer));
}

void aee_timer_start(struct aee_timer *t)
{
    t->start_ms = get_timer_masked();
}

void aee_timer_stop(struct aee_timer *t)
{
    t->acc_ms += (get_timer_masked() - t->start_ms);
    t->start_ms = 0;
}

void *kdump_core_header_init(const struct mrdump_control_block *kparams, uint64_t kmem_address, uint64_t kmem_size)
{
    voprintf_info("kernel page offset %llu\n", kparams->machdesc.page_offset);
    if (kparams->machdesc.page_offset <= 0xffffffffULL) {
	voprintf_info("32b kernel detected\n");
        return kdump_core32_header_init(kparams, kmem_address, kmem_size);
    }
    else {
	voprintf_info("64b kernel detected\n");
        return kdump_core64_header_init(kparams, kmem_address, kmem_size);
    }
}
