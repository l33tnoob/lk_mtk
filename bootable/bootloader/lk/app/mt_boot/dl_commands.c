#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <arch/ops.h>
#include <target.h>
#include <platform.h>
#include <platform/mt_reg_base.h>
#include <platform/boot_mode.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_rtc.h>
#include <platform/bootimg.h>
#ifdef MTK_GPT_SCHEME_SUPPORT
#include <platform/partition.h>
#else
#include <mt_partition.h>
#endif

#if _MAKE_HTC_LK
#include <htc_board_info_and_setting.h>
#ifdef MTK_SECURITY_SW_SUPPORT
#include "oemkey.h"
#include "htc_security_util.h"
#endif
#if TAMPER_DETECT
#include <htc_tamper_detect.h>
#endif
#endif

#include <platform/mtk_nand.h>

/*For image write*/
#include "sparse_format.h"
#include "dl_commands.h"
#include <platform/mmc_core.h>
#include <platform/mt_gpt.h>

#include "fastboot.h"
#include "mt_pmic.h"

#define MODULE_NAME "FASTBOOT_DOWNLOAD"
#define MAX_RSP_SIZE 64

extern void *download_base;
#if defined(MTK_MLC_NAND_SUPPORT)
extern unsigned long long download_max;
#else
extern unsigned download_max;
#endif
extern unsigned download_size;
extern unsigned fastboot_state;

/*LXO: !Download related command*/

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define INVALID_PTN -1
//For test: Display info on boot screen
#define DISPLAY_INFO_ON_LCM
#if defined(MTK_EMMC_SUPPORT)  //use another macro.
#define EMMC_TYPE
#else
#define NAND_TYPE
#endif
extern void video_printf (const char *fmt, ...);
extern int video_get_rows(void);
extern void video_set_cursor(int row, int col);
extern void video_clean_screen(void);
#if defined(MTK_MLC_NAND_SUPPORT)
extern int nand_write_img(u64 addr, void *data, u32 img_sz,u64 partition_size,int partition_type);
extern int nand_write_img_ex(u64 addr, void *data, u32 length,u64 total_size, u32 *next_offset, u64 partition_start,u64 partition_size, int img_type);
#else
extern int nand_write_img(u32 addr, void *data, u32 img_sz,u32 partition_size,int partition_type);
extern int nand_write_img_ex(u32 addr, void *data, u32 length,u32 total_size, u32 *next_offset, u32 partition_start,u32 partition_size, int img_type);
#endif
extern u32 gpt4_tick2time_ms (u32 tick);

unsigned start_time_ms;
#define TIME_STAMP gpt4_tick2time_ms(gpt4_get_current_tick())
#define TIME_START {start_time_ms = gpt4_tick2time_ms(gpt4_get_current_tick());}
#define TIME_ELAPSE (gpt4_tick2time_ms(gpt4_get_current_tick()) - start_time_ms)

extern int usb_write(void *buf, unsigned len);
extern int usb_read(void *buf, unsigned len);


extern int sec_dl_permission_chk(const char *part_name, unsigned int *permitted);
extern int sec_format_permission_chk(const char *part_name, unsigned int *permitted);

// HTC_CSP_START, #21487(Dybert_Wang), Add
#if _MAKE_HTC_LK
#ifdef MTK_SECURITY_SW_SUPPORT
extern u32 seclib_image_check_buf(u8 *buf, u32 size);
extern u8 g_oemkey[OEM_PUBK_SZ];
extern const u8 g_mteekey[MTEE_IMG_VFY_PUBK_SZ];
#define DBG_MSG_SIZE	128
#endif
#endif
// HTC_CSP_END


/* todo: give lk strtoul and nuke this */
static unsigned hex2unsigned(const char *x)
{
    unsigned n = 0;

    while(*x) {
        switch(*x) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                n = (n << 4) | (*x - '0');
                break;
            case 'a': case 'b': case 'c':
            case 'd': case 'e': case 'f':
                n = (n << 4) | (*x - 'a' + 10);
                break;
            case 'A': case 'B': case 'C':
            case 'D': case 'E': case 'F':
                n = (n << 4) | (*x - 'A' + 10);
                break;
            default:
                return n;
        }
        x++;
    }

    return n;
}

static void init_display_xy()
{
#if defined(DISPLAY_INFO_ON_LCM)
    video_clean_screen();
    video_set_cursor(video_get_rows()/2, 0);
    //video_set_cursor(1, 0);
#endif
}

static void display_info(const char* msg)
{
#if defined(DISPLAY_INFO_ON_LCM)
    if(msg == 0)
    {
        return;
    }
    video_printf("%s\n", msg);
#endif
}

static void display_progress(const char* msg_prefix, unsigned size, unsigned totle_size)
{
#if defined(DISPLAY_INFO_ON_LCM)
    unsigned vel = 0;
    u64 prog = 0;
    unsigned time = TIME_ELAPSE;


    if(msg_prefix == 0)
    {
        msg_prefix = "Unknown";
    }

    if(time != 0)
    {
        vel = (unsigned)(size / time); //approximate  1024/1000
        time /= 1000;
    }
    if(totle_size != 0)
    {
        prog = (u64)size*100/totle_size;
    }
    video_printf("%s > %3d%% Time:%4ds Vel:%5dKB/s", msg_prefix, (unsigned)prog, time, vel);
#endif
}

static void display_speed_info(const char* msg_prefix, unsigned size)
{
#if defined(DISPLAY_INFO_ON_LCM)
    unsigned vel = 0;
    unsigned time = TIME_ELAPSE;

    if(msg_prefix == 0)
    {
        msg_prefix = "Unknown";
    }

    if(time != 0)
    {
        vel = (unsigned)(size / time); //approximate  1024/1000
    }
    video_printf("%s  Time:%dms Vel:%dKB/s \n", msg_prefix, time, vel);
#endif
}

static void fastboot_fail_wrapper(const char* msg)
{
    display_info(msg);
    fastboot_fail(msg);
}

static void fastboot_ok_wrapper(const char* msg, unsigned data_size)
{
    display_speed_info(msg, data_size);
    fastboot_okay("");
}
void cmd_install_sig(const char *arg, void *data, unsigned sz)
{
    fastboot_fail_wrapper("Signature command not supported");
}


bool power_check()
{
    //3500 mV. shut down threshold 3.45V
    if(get_bat_sense_volt(5) < 3500)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void cmd_download(const char *arg, void *data, unsigned sz)
{
    char response[MAX_RSP_SIZE];
    unsigned len = hex2unsigned(arg);
    int r;

    if(!power_check())
    {
        fastboot_fail("low power, need battery charging.");
        return;
    }
    init_display_xy();
    download_size = 0;
    if (len > download_max) {
        fastboot_fail_wrapper("data is too large");
        return;
    }

    snprintf(response, MAX_RSP_SIZE, "DATA%08x", len);
    if (usb_write(response, strlen(response)) < 0)
    {
        return;
    }
    display_info("USB Transferring... ");
    TIME_START;
    r = usb_read(download_base, len);
    if ((r < 0) || ((unsigned) r != len)) 
    {
        fastboot_fail_wrapper("Read USB error");
        fastboot_state = STATE_ERROR;
        return;
    }
    download_size = len;

    fastboot_ok_wrapper("USB Transmission OK", len);
}

#ifdef NAND_TYPE
static int get_nand_image_type(const char* arg)
{
    int img_type = 0;
    if (!strcmp(arg, "system") ||
            !strcmp(arg, "userdata") ||
            !strcmp(arg, "fat") )
    {
#ifdef MTK_NAND_UBIFS_SUPPORT   
        img_type = UBIFS_IMG;	
#else
        img_type = YFFS2_IMG;	
#endif
    }
    else
    {
        img_type = RAW_DATA_IMG;	    
    }
    return img_type;
}
#endif

void cmd_flash_mmc_img(const char *arg, void *data, unsigned sz)
{
    unsigned long long ptn = 0;
    unsigned long long size = 0;
    int index = INVALID_PTN;

    // HTC_CSP_START, #21487(Dybert_Wang), Add
    #if _MAKE_HTC_LK
    int erase_ret = MMC_ERR_NONE;
    #endif
    // HTC_CSP_END
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    unsigned int part_id;
#endif


    if (!strcmp(arg, "partition"))
    {
        dprintf(ALWAYS, "Attempt to write partition image.\n");
        fastboot_fail_wrapper("Do not support this operation.");
        return;
    }
    else
    {
        index = partition_get_index(arg);
        ptn = partition_get_offset(index);
        if(ptn == (unsigned long long)(-1)) {
            fastboot_fail("partition table doesn't exist");
            return;
        }
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        part_id = partition_get_region(index);
#endif

        if (!strcmp(arg, "boot") || !strcmp(arg, "recovery")) {
            if (memcmp((void *)data, BOOT_MAGIC, BOOT_MAGIC_SIZE-1)) {
                fastboot_fail_wrapper("image is not a boot image");
                return;
            }
        }
        if (!strcmp(arg, "preloader")) {
            //maybe open later.
            fastboot_fail_wrapper("flash preloader is not permitted.");
            return;
        }

        size = partition_get_size(index);
        if (ROUND_TO_PAGE(sz,511) > size)
        {
            fastboot_fail_wrapper("size too large");
            return;
        }

// HTC_CSP_START, #21487(Dybert_Wang), Add
#if _MAKE_HTC_LK
#ifdef MTK_SECURITY_SW_SUPPORT
	int img_verified_status = 0;
	int rc = 0;
	unsigned char msg[DBG_MSG_SIZE] = {0};
	if (strncmp(arg, TEE_IMG_NAME_PREFIX, strlen(TEE_IMG_NAME_PREFIX)) == 0)
	{
		rc = seclib_set_oemkey(g_mteekey, MTEE_IMG_VFY_PUBK_SZ);
		snprintf(msg, DBG_MSG_SIZE, "TEE image, change to TEE key, rc = %d\n", rc);
		fastboot_info(msg);

		img_verified_status = seclib_image_check_buf((u8 *)data, (u32)sz);
	}
	else
	{
		if (setOEMkeyfromPGFS())
		{
			dprintf(INFO, "seclib_set_oemkey failed\n");
			img_verified_status = -1;
		} else {
			img_verified_status = seclib_image_check_buf((u8 *)data, (u32)sz);
		}
	}

	if (img_verified_status== 0)
	{
		fastboot_info("Image verification passed, start flashing...\r\n");
	}
	else
	{
		fastboot_info("Image verification failed...\r\n");
		if (setting_security()) // if S-ON
		{
			fastboot_fail_wrapper("Err! Stop flashing due to S-ON...\r\n");
			return;
		}
		else
		{
			fastboot_info("Ignore image verification fail due to S-OFF, start flashing...\r\n");

		}
	}

#if TAMPER_DETECT
	if (!strncmp(arg, "boot", strlen("boot")) || !strncmp(arg, "recovery", strlen("recovery")) ||
		!strncmp(arg, "system", strlen("system")) || !strncmp(arg, "hosd", strlen("hosd"))) {
			tamper_rec_update_image(arg);
			tamper_chk_update_image();
	}
#endif //!TAMPER_DETECT

#endif

   fastboot_info("erase partition...\r\n");
#ifdef EMMC_TYPE
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    erase_ret = emmc_erase(part_id, ptn, size);
#else
    erase_ret = emmc_erase(ptn, size);
#endif
#endif

#ifdef NAND_TYPE
    erase_ret = nand_erase(ptn,size);
#endif

    if(erase_ret != MMC_ERR_NONE)
    {
	fastboot_info("erase partition error...\r\n");
	fastboot_fail_wrapper("Erase error.");
	return;
    }
#endif // # if _MAKE_HTC_LK
// HTC_CSP_END
    fastboot_info("erase partition ok, start image flashing\r\n");
#ifdef EMMC_TYPE
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        dprintf (ALWAYS, "partid %d, addr 0x%llx, size 0x%x\n", part_id, ptn  , sz);
        if (emmc_write(part_id, ptn , (unsigned int *)data, sz) != sz)
#else
            if (emmc_write(ptn , (unsigned int *)data, sz) != sz)
#endif
#endif

#ifdef NAND_TYPE
#if defined(MTK_MLC_NAND_SUPPORT)
                if (nand_write_img((u64)ptn, (char*)data, sz,(u64)size,get_nand_image_type(arg))) 
#else
                    if (nand_write_img((u32)ptn, (char*)data, sz,(u32)size,get_nand_image_type(arg))) 
#endif
#endif
                    {
                        fastboot_fail_wrapper("flash write failure");
                        return;
                    }
    }
    fastboot_info("Bingo, image flash successfully, congratulations!!\r\n");
    fastboot_okay("");
    return;
}

void cmd_flash_mmc_sparse_img(const char *arg, void *data, unsigned sz)
{
    unsigned int chunk;
    unsigned int chunk_data_sz;
    sparse_header_t *sparse_header;
    chunk_header_t *chunk_header;
    uint32_t total_blocks = 0;
    unsigned long long ptn = 0;
    unsigned long long size = 0;
    int index = INVALID_PTN;
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    unsigned int part_id;
#endif

    index = partition_get_index(arg);
    ptn = partition_get_offset(index);
    if(ptn == (unsigned long long)(-1)) {
        fastboot_fail("partition table doesn't exist");
        return;
    }
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    part_id = partition_get_region(index);
#endif

    size = partition_get_size(index);

    /* Read and skip over sparse image header */
    sparse_header = (sparse_header_t *) data;
    dprintf(ALWAYS, "Image size span 0x%llx, partition size 0x%llx\n", (unsigned long long)sparse_header->total_blks*sparse_header->blk_sz, size);
    if((unsigned long long)sparse_header->total_blks*sparse_header->blk_sz > size)
    {
        fastboot_fail("sparse image size span overflow.");
        return;
    }

    data += sparse_header->file_hdr_sz;
    if(sparse_header->file_hdr_sz > sizeof(sparse_header_t))
    {
        /* Skip the remaining bytes in a header that is longer than
         * we expected.
         */
        data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
    }

    dprintf (ALWAYS, "=== Sparse Image Header ===\n");
    dprintf (ALWAYS, "magic: 0x%x\n", sparse_header->magic);
    dprintf (ALWAYS, "major_version: 0x%x\n", sparse_header->major_version);
    dprintf (ALWAYS, "minor_version: 0x%x\n", sparse_header->minor_version);
    dprintf (ALWAYS, "file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
    dprintf (ALWAYS, "chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
    dprintf (ALWAYS, "blk_sz: %d\n", sparse_header->blk_sz);
    dprintf (ALWAYS, "total_blks: %d\n", sparse_header->total_blks);
    dprintf (ALWAYS, "total_chunks: %d\n", sparse_header->total_chunks);

    display_info("\nWriting Flash ... ");
    /* Start processing chunks */
    for (chunk=0; chunk<sparse_header->total_chunks; chunk++)
    {
        /* Read and skip over chunk header */
        chunk_header = (chunk_header_t *) data;
        data += sizeof(chunk_header_t);

        //dprintf (ALWAYS, "=== Chunk Header ===\n");
        //dprintf (ALWAYS, "chunk_type: 0x%x\n", chunk_header->chunk_type);
        //dprintf (ALWAYS, "chunk_data_sz: 0x%x\n", chunk_header->chunk_sz);
        //dprintf (ALWAYS, "total_size: 0x%x\n", chunk_header->total_sz);

        if(sparse_header->chunk_hdr_sz > sizeof(chunk_header_t))
        {
            /* Skip the remaining bytes in a header that is longer than
             * we expected.
             */
            data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
        }

        chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;
        switch (chunk_header->chunk_type)
        {
            case CHUNK_TYPE_RAW:
                if(chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
                            chunk_data_sz))
                {
                    fastboot_fail("Bogus chunk size for chunk type Raw");
                    return;
                }

                dprintf (ALWAYS, "Raw: start block addr: 0x%x\n", total_blocks);

#ifdef EMMC_TYPE
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
                //dprintf (ALWAYS, "partid %d, addr 0x%llx, partsz 0x%x\n", part_id, ptn + ((unsigned long long)total_blocks*sparse_header->blk_sz) , chunk_data_sz);
                if (emmc_write(part_id, ptn + ((unsigned long long)total_blocks*sparse_header->blk_sz) , data, chunk_data_sz) != chunk_data_sz)
#else
                    if (emmc_write(ptn + ((unsigned long long)total_blocks*sparse_header->blk_sz) , data, chunk_data_sz) != chunk_data_sz)
#endif
#endif

#ifdef NAND_TYPE
#if defined(MTK_MLC_NAND_SUPPORT)
                        if (nand_write_img((u64)(ptn + ((unsigned long long)total_blocks*sparse_header->blk_sz)), (char*)data, chunk_data_sz,(u64)size,get_nand_image_type(arg))) 
#else
                            if (nand_write_img((u32)(ptn + ((unsigned long long)total_blocks*sparse_header->blk_sz)), (char*)data, chunk_data_sz,(u32)size,get_nand_image_type(arg))) 
#endif
#endif
                            {
                                fastboot_fail_wrapper("flash write failure");
                                return;
                            }

                total_blocks += chunk_header->chunk_sz;
                data += chunk_data_sz;
                break;

            case CHUNK_TYPE_DONT_CARE:
                dprintf (ALWAYS, "!!Blank: start: 0x%x  offset: 0x%x\n", total_blocks, chunk_header->chunk_sz);
                total_blocks += chunk_header->chunk_sz;
                break;

            case CHUNK_TYPE_CRC:
                if(chunk_header->total_sz != sparse_header->chunk_hdr_sz)
                {
                    fastboot_fail_wrapper("Bogus chunk size for chunk type Dont Care");
                    return;
                }
                total_blocks += chunk_header->chunk_sz;
                data += chunk_data_sz;
                break;

            default:
                fastboot_fail_wrapper("Unknown chunk type");
                return;
        }
        display_progress("\rWrite Data", total_blocks*sparse_header->blk_sz, sparse_header->total_blks*sparse_header->blk_sz);
    }

    dprintf(ALWAYS, "Wrote %d blocks, expected to write %d blocks\n",
            total_blocks, sparse_header->total_blks);

    if(total_blocks != sparse_header->total_blks)
    {
        fastboot_fail_wrapper("sparse image write failure");
    }
    else
    {
        display_info("\n\nOK");
        fastboot_okay("");
    }  

    return;
}

void cmd_flash_mmc(const char *arg, void *data, unsigned sz)
{
    sparse_header_t *sparse_header;
#ifdef MTK_SECURITY_SW_SUPPORT
    unsigned int permitted = 0;
    char msg[64];
#endif

    if(sz  == 0)
    {
        fastboot_okay("");
        return;
    }

    // security check here.
    // ret = decrypt_scm((uint32 **) &data, &sz);
#ifdef MTK_SECURITY_SW_SUPPORT
#if _MAKE_HTC_LK
    if (htc_sec_dl_permission_chk(arg, &permitted))
#else
    if (sec_dl_permission_chk(arg, &permitted))
#endif
    {
        sprintf(msg, "failed to get download permission for partition '%s'\n", arg);
        fastboot_fail(msg);
        return;
    }

    if (0 == permitted)
    {
        sprintf(msg, "download for partition '%s' is not allowed\n", arg);
        fastboot_fail(msg);
        return;
    }
#endif
    TIME_START;
#if _MAKE_HTC_LK
	setting_invalidate_checksum();
#endif
    sparse_header = (sparse_header_t *) data;
    if (sparse_header->magic != SPARSE_HEADER_MAGIC)
        cmd_flash_mmc_img(arg, data, sz);
    else
        cmd_flash_mmc_sparse_img(arg, data, sz);
    return;
}

void cmd_erase_mmc(const char *arg, void *data, unsigned sz)
{
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    unsigned int part_id;
#endif
    unsigned long long ptn = 0;
    unsigned long long size = 0;
    int index = INVALID_PTN;
    int erase_ret = MMC_ERR_NONE;
#ifdef MTK_SECURITY_SW_SUPPORT
    unsigned int permitted = 0;
    char msg[64];
#endif

    dprintf (ALWAYS, "cmd_erase_mmc\n");
    index = partition_get_index(arg);
    if(index == -1) {
        fastboot_fail_wrapper("Partition table doesn't exist");
        return;
    }
#if _MAKE_HTC_LK
	setting_invalidate_checksum();
#endif
#ifdef MTK_SECURITY_SW_SUPPORT
#if _MAKE_HTC_LK
    if (htc_sec_format_permission_chk(arg, &permitted))
#else
    if (sec_format_permission_chk(arg, &permitted))
#endif
    {
        sprintf(msg, "failed to get format permission for partition '%s'\n", arg);
        fastboot_fail(msg);
        return;
    }

    if (0 == permitted)
    {
        sprintf(msg, "format for partition '%s' is not allowed\n", arg);
        fastboot_fail(msg);
        return;
    }
#endif

#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    part_id = partition_get_region(index);
#endif
    ptn = partition_get_offset(index);

    size = partition_get_size(index);

    TIME_START;
#ifdef EMMC_TYPE
#ifdef MTK_NEW_COMBO_EMMC_SUPPORT
    erase_ret = emmc_erase(part_id, ptn, size);                 
#else
    erase_ret = emmc_erase(ptn, size);         
#endif
#endif

#ifdef NAND_TYPE
    erase_ret = nand_erase(ptn,size);
#endif

    if(erase_ret  == MMC_ERR_NONE)
    {
        fastboot_ok_wrapper("OK", size);
    }
    else
    {
        fastboot_fail_wrapper("Erase error.");
    }

    return;
}


/*LXO: END!Download related command*/
