#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
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
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/mt_disp_drv.h>
#include "fastboot.h"
#include "sys_commands.h"
extern boot_img_hdr *g_boot_hdr;
extern int g_is_64bit_kernel;
extern struct fastboot_var *varlist;
extern struct fastboot_cmd *cmdlist;
extern int mboot_android_check_img_info(char *part_name, part_hdr_t *part_hdr);
extern int boot_linux_from_storage(void);
extern const char* mt_disp_get_lcm_id(void);
extern void boot_linux(void *kernel, unsigned *tags,char *cmdline, unsigned machtype,void *ramdisk, unsigned ramdisk_size);
extern unsigned int memory_size();
extern void custom_port_in_kernel(BOOTMODE boot_mode, char *command);
#ifdef MTK_OFF_MODE_CHARGE_SUPPORT
extern void set_off_mode_charge_status(int enable);
extern bool get_off_mode_charge_status(void);
#endif

#define MODULE_NAME "FASTBOOT"
#define MAX_RSP_SIZE 64


extern BOOTMODE g_boot_mode;
extern char g_CMDLINE [300];
extern timer_t wdt_timer;
extern unsigned int _heap_end;

int has_set_p2u = 0; //0-unset; 1-on; 2-off
#ifdef MTK_TC7_COMMON_DEVICE_INTERFACE
static unsigned char *head_of_adb_cmdline;
static unsigned char adb_cmdline_appended;
#endif

void cmd_overwirte_cmdline(const char *arg, void *data, unsigned sz)
{
  const char *new_kernel_cmd_line = arg;
  int cmd_len;

  cmd_len = strlen(new_kernel_cmd_line);

  if(cmd_len > 300)
  {
    printf("[FASTBOOT] Input cmdline length is too long!");
    fastboot_fail("cmdline length is too long");
  }

  strcpy(g_CMDLINE, new_kernel_cmd_line);

  printf("[FASTBOOT] New command line is %s\n", g_CMDLINE);
  fastboot_okay("");
}



void cmd_oem_append_cmdline(const char *arg, void *data, unsigned sz)
{
  int cmd_len;
  char new_kernel_cmd_line[300];
  printf("APPEND KERNEL CMDLINE\n");
  fastboot_info("APPEND KERNEL CMDLINE\n");
  sprintf(new_kernel_cmd_line, "%s%s", g_CMDLINE,arg);
  cmd_len = strlen(new_kernel_cmd_line);
  if(cmd_len > 300)
  {
    printf("[FASTBOOT] Input cmdline length is too long!");
    fastboot_fail("cmdline length is too long");
    return;
  }
  strcpy(g_CMDLINE, new_kernel_cmd_line);
  printf("[FASTBOOT] New command line:%s\n", g_CMDLINE);
  fastboot_okay("");
  return;
}

#ifdef MTK_TC7_COMMON_DEVICE_INTERFACE
void cmd_oem_ADB_Auto_Enable(const char *arg, void *data, unsigned sz)
{
  char *str_enable=" enable";
  char *str_disable=" disable";
  char *str_help=" help";
  if(adb_cmdline_appended)
  {
    if(memcmp(arg,str_enable,strlen(str_enable))==0)
    {
      *head_of_adb_cmdline='1';
      printf("[FASTBOOT] New command line:%s\n", g_CMDLINE);
      fastboot_okay("");
      return;
    }
    else if(memcmp(arg,str_disable,strlen(str_disable))==0)
    {
      *head_of_adb_cmdline='0';
      printf("[FASTBOOT] New command line:%s\n", g_CMDLINE);
      fastboot_okay("");
      return;
    }
    else if(memcmp(arg,str_help,strlen(str_help))==0)
    {
     fastboot_info("fastboot oem auto-ADB enable    Enable Auto ADB");
     fastboot_info("fastboot oem auto-ADB disable   Disable Auto ADB");    
     fastboot_okay("");
     return;
    }
    else
    {
      fastboot_fail("Please enter correct cmd");
    }
  }
  else 
  {
    if(memcmp(arg,str_enable,strlen(str_enable))==0)
    {
      cmd_oem_append_cmdline(" AdbAutoEnable=1",data,sz);
      head_of_adb_cmdline=strchr(g_CMDLINE,'\0')-0x1;
      printf("Head of auto-adb cmd %p,%s\n",head_of_adb_cmdline,head_of_adb_cmdline);
      adb_cmdline_appended=1;
      return;
    }
    else if(memcmp(arg,str_disable,strlen(str_disable))==0)  
    {
      cmd_oem_append_cmdline(" AdbAutoEnable=0",data,sz);
      head_of_adb_cmdline=strchr(g_CMDLINE,'\0')-0x1;
      printf("Head of auto-adb cmd %p,%s\n",head_of_adb_cmdline,head_of_adb_cmdline);
      adb_cmdline_appended=1;
      return;
    }
    else if(memcmp(arg,str_help,strlen(str_help))==0)
    {
     fastboot_info("fastboot oem auto-ADB enable:  enable Auto ADB");
     fastboot_info("fastboot oem auto-ADB disable: disable Auto ADB");
     fastboot_okay("");
     return;
    }
    else
    {
      fastboot_fail("Please enter correct cmd");
    }
  }
  return;
}
#endif

void set_p2u(int mode)
{
  if(mode == 1)
  {
    if(strlen(g_CMDLINE) + strlen(" printk.disable_uart=1") + 1 > sizeof(g_CMDLINE)) 
    {
      printf("command line is too long, will not set printk_on\n");
      fastboot_info("command line is too long, will not set printk_on");
    }
    else
    {
      sprintf(g_CMDLINE, "%s %s", g_CMDLINE, "printk.disable_uart=0");
      fastboot_info("printk to uart is on!");
    }
  }
  else if(mode == 2)
  {
    if(strlen(g_CMDLINE) + strlen(" printk.disable_uart=1") + 1 > sizeof(g_CMDLINE))
    {
      printf("command line is too long, will not set printk_off\n");
      fastboot_info("command line is too long, will not set printk_off");
    } 
    else
    {
      sprintf(g_CMDLINE, "%s %s", g_CMDLINE, "printk.disable_uart=1");
      fastboot_info("printk to uart is off!");
    }
  }
}

void cmd_continue(const char *arg, void *data, unsigned sz)
{
  g_boot_mode = NORMAL_BOOT;
  if(has_set_p2u) 
  {
    set_p2u(has_set_p2u);
    fastboot_info("phone will continue boot up after 5s...");
    fastboot_okay("");
    mdelay(5000);
  }
  else
  {
    fastboot_okay("");
  }
  udc_stop();
  //mtk_wdt_init(); //re-open wdt
  timer_cancel(&wdt_timer);
  mtk_wdt_restart();

  /*Will not return*/
  boot_linux_from_storage();
}

void cmd_oem_p2u(const char *arg, void *data, unsigned sz)
{
  if(!strncmp(arg, " on", strlen(" on")))
  {
    has_set_p2u = 1;
  }
  else if(!strncmp(arg, " off", strlen(" off")))
  {
    has_set_p2u = 2;
  }
  else
  {
    has_set_p2u = 0;
    fastboot_info("unknown argument");
  }
  fastboot_okay("");
}

void cmd_oem_reboot2recovery(const char *arg, void *data, unsigned sz)
{
   extern void Set_RTC_Recovery_Mode(bool flag)__attribute__((weak));

   if(Set_RTC_Recovery_Mode)
   {
       Set_RTC_Recovery_Mode(1);
       fastboot_okay("");
       mtk_arch_reset(1); //bypass pwr key when reboot
   }
   else
   {
       fastboot_fail("Not support this function (need RTC porting)");
   }
}
#ifdef MTK_OFF_MODE_CHARGE_SUPPORT
void set_off_mode_charge(int mode)
{
    struct fastboot_var *var;

    for (var = varlist; var; var = var->next){
        if (!strcmp(var->name, "off-mode-charge")) {
            var->value = (mode == 1) ? "1" : "0";
        }
    }
    set_off_mode_charge_status(mode);
}
void cmd_oem_off_mode_charge(const char *arg, void *data, unsigned sz)
{
    if(!strncmp(arg, " 1", strlen(" 1")))
    {//CHARGE MODE
        set_off_mode_charge(1);
       }
       else if(!strncmp(arg, " 0", strlen(" 0")))
       {//NORMAL MODE
        set_off_mode_charge(0);
       }
       else
       {
             fastboot_info("unknown argument");
       }
    fastboot_okay("");
}
#endif
#ifdef MTK_JTAG_SWITCH_SUPPORT
extern unsigned int set_ap_jtag(unsigned int);
extern unsigned int get_ap_jtag(void);
void cmd_oem_ap_jtag(const char *arg, void *data, unsigned sz)
{
    char response[MAX_RSP_SIZE];

    if(!strncmp(arg, " 1", strlen(" 1")))
    {//turn ap jtag on
             fastboot_info("Enable AP JTAG");
	     set_ap_jtag(1);
    }
    else if(!strncmp(arg, " 0", strlen(" 0")))
    {//turn ap jtag off
             fastboot_info("Disable AP JTAG");
	     set_ap_jtag(0);
    }
    else
    {
	snprintf(response, MAX_RSP_SIZE,"\tCurrent AP JTAG setting:%s", get_ap_jtag()?"Enable":"Disable");
	fastboot_info(response);
    }
    fastboot_okay("");
}
#endif
void cmd_getvar(const char *arg, void *data, unsigned sz)
{
	struct fastboot_var *var;
	char response[MAX_RSP_SIZE];

	if(!strcmp(arg, "all")){
		for (var = varlist; var; var = var->next){
			snprintf(response, MAX_RSP_SIZE,"\t%s: %s", var->name, var->value);
			fastboot_info(response);
		}
		fastboot_okay("Done!!");
		return;
	}
	for (var = varlist; var; var = var->next) {
		if (!strcmp(var->name, arg)) {
			fastboot_okay(var->value);
			return;
		}
	}
	fastboot_okay("");
}

void cmd_reboot(const char *arg, void *data, unsigned sz)
{
  dprintf(INFO, "rebooting the device\n");
  fastboot_okay("");
  mtk_arch_reset(1); //bypass pwr key when reboot
}

void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz)
{
  dprintf(INFO, "rebooting the device to bootloader\n");
  fastboot_okay("");
  Set_Clr_RTC_PDN1_bit13(true); //Set RTC fastboot bit
  mtk_arch_reset(1); //bypass pwr key when reboot
}

extern char g_CMDLINE [300];
extern unsigned int g_kmem_off;
extern unsigned int g_rmem_off;
extern unsigned int g_bimg_sz;
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
void cmd_boot(const char *arg, void *data, unsigned sz)
{
	unsigned kernel_actual;
	unsigned ramdisk_actual;
	struct boot_img_hdr *boot_hdr;
	char *ptr = ((char*) data);
	unsigned page_size = 0;
	unsigned page_mask = 0;
	boot_hdr = (boot_img_hdr*)malloc(sizeof(boot_img_hdr));

	if (!boot_hdr)
	{
		printf("cmd_boot,boot_hdr = NULL\n");
		return;
	}

	memcpy(boot_hdr, data, sizeof(boot_img_hdr));

	printf("\n============================================================\n");
	boot_hdr->magic[7] = '\0';
	printf("[%s] Android Boot IMG Hdr - Magic 	        : %s\n",MODULE_NAME,boot_hdr->magic);
	printf("[%s] Android Boot IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,boot_hdr->kernel_size);
	printf("[%s] Android Boot IMG Hdr - Kernel addr 	: 0x%x\n",MODULE_NAME,boot_hdr->kernel_addr);
	printf("[%s] Android Boot IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,boot_hdr->ramdisk_size);
	printf("[%s] Android Boot IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,boot_hdr->page_size);
	printf("============================================================\n");

	/* ensure commandline is terminated */
	boot_hdr->cmdline[BOOT_ARGS_SIZE-1] = 0;

	if(boot_hdr->page_size) {
		page_size = boot_hdr->page_size;
		page_mask = page_size - 1;
		//page_mask = 2*1024 ; /*FIXME*/
	}
	else
	{
		printf("[FASTBOOT] Please specify the storage page-size in the boot header!\n");
		fastboot_fail("Please specify the storage page-size in the boot header!\n");
		return;
	}

	kernel_actual = ROUND_TO_PAGE(boot_hdr->kernel_size, page_mask);
	ramdisk_actual = ROUND_TO_PAGE(boot_hdr->ramdisk_size, page_mask);

	/* sz should have atleast raw boot image */
	if (page_size + kernel_actual + ramdisk_actual > sz) {
		fastboot_fail("incomplete bootimage");
		return;
	}

	if((boot_hdr->kernel_addr <= ROUND_TO_PAGE((boot_hdr->ramdisk_addr + boot_hdr->ramdisk_size), page_mask)) && (ROUND_TO_PAGE((boot_hdr->kernel_addr + boot_hdr->kernel_size), page_mask) >= boot_hdr->ramdisk_addr)) {
		fastboot_fail("invalid kernel & ramdisk address: images overlap");
		return;
	}

	if((boot_hdr->kernel_addr < DRAM_PHY_ADDR) || (ROUND_TO_PAGE((boot_hdr->kernel_addr + boot_hdr->kernel_size), page_mask) > (DRAM_PHY_ADDR + memory_size()))) {
		fastboot_fail("invalid kernel address: not lie in memory");
		return;
	}

	if((boot_hdr->kernel_addr <= _heap_end) && (ROUND_TO_PAGE((boot_hdr->kernel_addr + boot_hdr->kernel_size), page_mask) >= MEMBASE)) {
		fastboot_fail("invalid kernel address: overlap with lk");
		return;
	}

	if((boot_hdr->kernel_addr <= ROUND_TO_PAGE((SCRATCH_ADDR + boot_hdr->kernel_size + boot_hdr->ramdisk_size), page_mask)) && (ROUND_TO_PAGE((boot_hdr->kernel_addr + boot_hdr->kernel_size), page_mask) >= SCRATCH_ADDR)) {
		fastboot_fail("invalid kernel address: overlap with the download image");
		return;
	}

	if((boot_hdr->ramdisk_addr < DRAM_PHY_ADDR) || (ROUND_TO_PAGE((boot_hdr->ramdisk_addr + boot_hdr->ramdisk_size), page_mask) > (DRAM_PHY_ADDR + memory_size()))) {
		fastboot_fail("invalid ramdisk address: not lie in memory");
		return;
	}

	if((boot_hdr->ramdisk_addr <= _heap_end) && (ROUND_TO_PAGE((boot_hdr->ramdisk_addr + boot_hdr->ramdisk_size), page_mask) >= MEMBASE)) {
		fastboot_fail("invalid ramdisk address: overlap with lk");
		return;
	}

	if((boot_hdr->ramdisk_addr <= ROUND_TO_PAGE((SCRATCH_ADDR + boot_hdr->kernel_size + boot_hdr->ramdisk_size), page_mask)) && (ROUND_TO_PAGE((boot_hdr->ramdisk_addr + boot_hdr->ramdisk_size), page_mask) >= SCRATCH_ADDR)) {
		fastboot_fail("invalid ramdisk address: overlap with the download image");
		return;
	}

	/*
	 * check mkimg header
	 */
	printf("check mkimg header\n");
	if (mboot_android_check_img_info(PART_KERNEL, (part_hdr_t *)(ptr + boot_hdr->page_size)) == -1) {
		printf("no mkimg header in kernel image\n");
		if(g_is_64bit_kernel){
			memmove((void*) SCRATCH_ADDR, (ptr + boot_hdr->page_size), boot_hdr->kernel_size);
		}else{
			memmove((void*) boot_hdr->kernel_addr, (ptr + boot_hdr->page_size), boot_hdr->kernel_size);
		}
	}
	else {
		printf("mkimg header exist in kernel image\n");
		if(g_is_64bit_kernel){
			memmove((void*) SCRATCH_ADDR, (ptr + boot_hdr->page_size + MKIMG_HEADER_SZ), boot_hdr->kernel_size);
		}else{
			memmove((void*) boot_hdr->kernel_addr, (ptr + boot_hdr->page_size + MKIMG_HEADER_SZ), boot_hdr->kernel_size);
		}
	}

	if ((mboot_android_check_img_info(PART_ROOTFS, (part_hdr_t *)(ptr + boot_hdr->page_size + kernel_actual)) == -1) 
			&& (mboot_android_check_img_info("RECOVERY", (part_hdr_t *)(ptr + boot_hdr->page_size + kernel_actual)) == -1)) {
		printf("no mkimg header in ramdisk image\n");
		memmove((void*) boot_hdr->ramdisk_addr, (ptr + boot_hdr->page_size + kernel_actual), boot_hdr->ramdisk_size);
	}
	else {
		printf("mkimg header exist in ramdisk image\n");
		memmove((void*) boot_hdr->ramdisk_addr, (ptr + boot_hdr->page_size + kernel_actual + MKIMG_HEADER_SZ), boot_hdr->ramdisk_size);
	}

#ifndef MACH_FPGA
	sprintf(g_CMDLINE, "%s %s", g_CMDLINE, boot_hdr->cmdline);
	sprintf(g_CMDLINE, "%s lcm=%1d-%s", g_CMDLINE, DISP_IsLcmFound(), mt_disp_get_lcm_id());
	sprintf(g_CMDLINE, "%s fps=%1d", g_CMDLINE, mt_disp_get_lcd_time());
	sprintf(g_CMDLINE, "%s vram=%1d", g_CMDLINE, DISP_GetVRamSize());
#endif

#ifdef SELINUX_STATUS
#if SELINUX_STATUS == 1
	sprintf(g_CMDLINE, "%s androidboot.selinux=disabled", g_CMDLINE);
#elif SELINUX_STATUS == 2
	sprintf(g_CMDLINE, "%s androidboot.selinux=permissive", g_CMDLINE);
#endif
#endif

	fastboot_okay("");
	udc_stop();
	//mtk_wdt_init(); //re-open wdt
	timer_cancel(&wdt_timer);
	mtk_wdt_restart();

	g_boot_hdr = boot_hdr;
	g_boot_mode = NORMAL_BOOT;
	custom_port_in_kernel(g_boot_mode, g_CMDLINE);

	printf("Kernel Address: 0x%8X\n", boot_hdr->kernel_addr);
	printf("Ramdisk Address: 0x%8X\n", boot_hdr->ramdisk_addr);
	printf("Atag Address: 0x%8X\n", boot_hdr->tags_addr);
	printf("Command: %s\n", boot_hdr->cmdline);

	boot_linux((void*) boot_hdr->kernel_addr, (void*) boot_hdr->tags_addr,
			(char*) g_CMDLINE, board_machtype(),
			(void*) boot_hdr->ramdisk_addr, boot_hdr->ramdisk_size);
#if 0
  unsigned kernel_actual;
  unsigned ramdisk_actual;
  struct boot_img_hdr boot_hdr;
  unsigned int k_pg_cnt = 0;
  unsigned int r_pg_cnt = 0;
  unsigned int b_pg_cnt = 0;
  unsigned int size_b = 0;
  unsigned int pg_sz = 2*1024 ;
  int strlen = 0;

  /*copy hdr data from download_base*/
  memcpy(&boot_hdr, data, sizeof(boot_hdr));

  /* ensure commandline is terminated */
  boot_hdr->cmdline[BOOT_ARGS_SIZE-1] = 0;

  printf("\n============================================================\n");
	boot_hdr->magic[7] = '\0';
  printf("[%s] Android Boot IMG Hdr - Magic 	        : %s\n",MODULE_NAME,boot_hdr->magic);
  printf("[%s] Android Boot IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,boot_hdr->kernel_size);
  printf("[%s] Android Boot IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,boot_hdr->ramdisk_size);
  printf("[%s] Android Boot IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,boot_hdr->page_size);
  printf("============================================================\n");

  //***************
  //* check partition magic
  //*
  if (strncmp(boot_hdr->magic,BOOT_MAGIC, sizeof(BOOT_MAGIC))!=0) {
    printf("[%s] boot image header magic error\n", MODULE_NAME);
    return -1;
  }

  g_kmem_off =  (unsigned int)target_get_scratch_address();
  g_kmem_off = g_kmem_off + MKIMG_HEADER_SZ + BIMG_HEADER_SZ;


  if(boot_hdr->kernel_size % pg_sz == 0)
  {
    k_pg_cnt = boot_hdr->kernel_size / pg_sz;
  }
  else
  {
    k_pg_cnt = (boot_hdr->kernel_size / pg_sz) + 1;
  }

  if(boot_hdr->ramdisk_size % pg_sz == 0)
  {
    r_pg_cnt = boot_hdr->ramdisk_size / pg_sz;
  }
  else
  {
    r_pg_cnt = (boot_hdr->ramdisk_size / pg_sz) + 1;
  }

  printf(" > page count of kernel image = %d\n",k_pg_cnt);
  g_rmem_off = g_kmem_off + k_pg_cnt * pg_sz;

  printf(" > kernel mem offset = 0x%x\n",g_kmem_off);
  printf(" > rootfs mem offset = 0x%x\n",g_rmem_off);

  //***************
  //* specify boot image size
  //*
  g_bimg_sz = (k_pg_cnt + r_pg_cnt + 2)* pg_sz;
  printf(" > boot image size = 0x%x\n",g_bimg_sz);

  memmove((void*)CFG_BOOTIMG_LOAD_ADDR , g_kmem_off, boot_hdr->kernel_size);
  memmove((void*)CFG_RAMDISK_LOAD_ADDR , g_rmem_off, boot_hdr->ramdisk_size);

  //custom_port_in_kernel(g_boot_mode, commanline);
  //strlen += sprintf(commanline, "%s lcm=%1d-%s", commanline, DISP_IsLcmFound(), mt_disp_get_lcm_id());
  //strlen += sprintf(commanline, "%s fps=%1d", commanline, mt_disp_get_lcd_time());

  fastboot_okay("");

  udc_stop();

  mtk_wdt_init();
  boot_linux((void *)CFG_BOOTIMG_LOAD_ADDR, (unsigned *)CFG_BOOTARGS_ADDR,
		   (const char*) boot_hdr->cmdline, board_machtype(),
		   (void *)CFG_RAMDISK_LOAD_ADDR, boot_hdr->ramdisk_size);
#endif
}

