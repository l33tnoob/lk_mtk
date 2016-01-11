#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <debug.h>
#include <string.h>
#include <dev/keys.h>
#include <dev/fbcon.h>
#include <platform.h>
#include <htc_board_info_and_setting.h>
#include <htc_version_info.h>
#include <platform/boot_mode.h>
#include <video_fb.h>
#include <htc_ramdump.h>
#include <platform/msdc_utils.h>
#include <kernel/thread.h>
#include <htc_reboot_info.h>
#include <platform/mt_rtc.h>

/* Installed font sets */
#include "font/asc24bpp.h"
#include "font/asc36bpp.h"
#include "font/asc48bpp.h"

#include "htc_security_util.h"
#include <htc_ddr_test.h>

// ===== from hboot include/functionkey.h ======
#define FUNKEY_BAR_UP					11	//For  Menu Selection
#define FUNKEY_BAR_DOWN					12
#define FUNKEY_BAR_SELECT				13

// ===== from hboot include/display.h =====
typedef struct {
	const char* display_string;
	void (*change_bootmode)(void);
	uint32_t display_color;
} bootmode_option_t;

struct _bootmode_option_list_t;
typedef struct _bootmode_option_list_t bootmode_option_list_t;
struct _bootmode_option_list_t {
	bootmode_option_t node;
	bootmode_option_list_t* next;
	bootmode_option_list_t* prev;
};

typedef enum {
	BOOTMODE_OPTION_FIRST = -1,

	BOOTMODE_OPTION_FASTBOOT = 0,
	BOOTMODE_OPTION_RECOVERY,
	BOOTMODE_OPTION_DOWNLOAD,
	BOOTMODE_OPTION_REBOOT,
	BOOTMODE_OPTION_REBOOT_BOOTLOADER,
	BOOTMODE_OPTION_POWER_DOWN,

	BOOTMODE_OPTION_MENU_TEST1,
	BOOTMODE_OPTION_MENU_TEST2,
	BOOTMODE_OPTION_MENU_TEST3,
/* For ramdump */
	BOOTMODE_OPTION_RAMDUMP_USB,
	BOOTMODE_OPTION_RAMDUMP_EMMC,
/* For ddrtest */
	BOOTMODE_OPTION_DDRTEST,
	BOOTMODE_OPTION_PRELOADER_DDRTEST,
	BOOTMODE_OPTION_LK_DDRTEST,
	BOOTMODE_OPTION_LAST,
} bootmode_option_enum;

/*
#define BLACK_C         0x0000        // black color for font
#define BLUE_C          0x001F        // blue color for font
#define GREEN_C         0x0400        // green color for font
#define LIMEGREEN_C     0x3666        // lime green color for font
#define YELLOWGREEN_C   0xA647        // yellow green color for font
#define RED_C           0xF800        // red color for font
#define YELLOW_C        0xFFE0        // yellow color for font
#define WHITE_C         0xFFFF        // white color for font
#define LIGHTGRAY_C     0xC618        // Light gray color for font
#define DARKGRAY_C      0x4208        // Dark gray color for font
#define MAGENTA_C       0xF81F        // Magenta color for font
#define GOLD_C          0xFEA0        // Gold color for font
#define DARKORANGE_C    0xFC60        // Dark Orange color for font
#define ORANGERED_C     0xFA20        // Orange Red color for font
#define PURPLE_C        0x8010        // Purple color for font
#define PINK_C          0xFC1F        // Pink color for font
*/


//== porting workaround ==
//According to the function draw_the_pixel and fbcon_drawglyph in file dev/fbcon/fbcon.c
//On a5ul, the display-driver use 3 bytes (0xBBGGRR) to represent the RGB color value of one pixel.
//We might need a more flexible way to dynamically choose the color-code according to the expected color-format
#define BLACK_C       0x0
#define BLUE_C        0xFF0000
#define GREEN_C       0x00FF00
#define LIMEGREEN_C   0x32CD32
#define YELLOWGREEN_C 0x32CD9A
#define RED_C         0x0000FF
#define YELLOW_C      0x00FFFF
#define WHITE_C       0xFFFFFF
#define LIGHTGRAY_C   0xAAAAAA
#define DARKGRAY_C    0x555555
#define MAGENTA_C     0xFF00FF
#define GOLD_C        0x00D7FF
#define DARKORANGE_C  0x008CFF
#define ORANGERED_C   0x0045FF
#define PURPLE_C      0x800080
#define PINK_C        0xCBC0FF



#define COLOR_BLACK         0x00000000
#define COLOR_WHITE         0x00ffffff
#define COLOR_BLUE          0x000000ff
#define COLOR_GREEN         0x0000ff00
#define COLOR_RED           0x00ff0000
#define COLOR_GRAY          0x00808080
#define COLOR_LIGHTGRAY     0x00c0c0c0
#define COLOR_DARKGRAY      0x00404040
#define COLOR_YELLOW        0x00FFFF00
#define COLOR_MAGENTA       0x00FF00FF
#define COLOR_GOLD          0x00FFD700
#define COLOR_DARKORANGE    0x00FF8C00
#define COLOR_ORANGERED     0x00FF4500
#define COLOR_PURPLE        0x00800080
#define COLOR_RUU_BAR       0x009bcd37
#define COLOR_RUU_FRAME     0x00373737
#define COLOR_RUU_EMPTY     0x000b0b0b
#define COLOR_RUU_FAIL      0x00ed2d2d
#define COLOR_FOTA_BAR      0x004eaa77

#define FRAME_BG_BLACK      0x00        // Fill whole frame buffer with 0x00
#define FRAME_BG_WHITE      0xFF        // Fill whole frame buffer with 0xFF

#if defined(BLACK_BACKGND)
#define BACKGND_COLOR		COLOR_BLACK
#define FRAME_BG_COLOR		FRAME_BG_BLACK
#else
#define BACKGND_COLOR		COLOR_WHITE
#define FRAME_BG_COLOR		FRAME_BG_WHITE
#endif

// ===== end of display.h =====

// ===== dummy functions ============
#define FASTBOOT_MODE           0x77665500
extern BOOTMODE g_boot_mode;
void ddrtest_menu_init(void);
void display_ddrtest_info(char*);

/* security parameter */
extern char g_unlock_string[36];
#if TAMPER_DETECT
extern char g_tamper_string[36];
#endif
extern const char *SEC_BOOT_WARNING;

static void board_soft_reset(uint32_t the_mode)
{
	switch(the_mode) {
		case 0:
			dprintf(CRITICAL, "%s: reboot\r\n", __FUNCTION__);
			break;
		case FASTBOOT_MODE:
			dprintf(CRITICAL, "%s: reboot to fastboot\r\n", __FUNCTION__);
			break;
	}
}

static void radio_powerdown()
{
	dprintf(CRITICAL, "%s: power off\r\n", __FUNCTION__);
	mt6575_power_off();

}
// ===== end of dummy functions =====

static struct fbcon_config* fbcon_display_config = NULL;
static GraphicDevice* gGfxDev = NULL;
static int display_width()
{
	if(gGfxDev == NULL) {
		dprintf(CRITICAL, "%s: pointer is null\r\n", __FUNCTION__);
		return 0;
	} else
		return gGfxDev->winSizeX;
}

static int display_height()
{
	if(gGfxDev == NULL) {
		dprintf(CRITICAL, "%s: pointer is null\r\n", __FUNCTION__);
		return 0;
	} else
		return gGfxDev->winSizeY;
}

static int display_bpp()
{
	if(gGfxDev == NULL) {
		dprintf(CRITICAL, "%s: pointer is null\r\n", __FUNCTION__);
		return 0;
	} else
		return gGfxDev->gdfBytesPP*8;
}

static void *frame_buffer = NULL;
//void *RUU_frame_buffer = NULL;
static void *current_frame_buffer = NULL;
static int frame_buffer_size;

void display_allocate_buffers(unsigned int fb_start)
{
#if 1
	/*[FIXME] No VA Definition*/
	if (fb_start == 0xffffffff)
		frame_buffer = memalign(4096, frame_buffer_size);
	else
		frame_buffer = (void*) fb_start;

	if (frame_buffer == NULL) {
		dprintf(CRITICAL, "Allocte heap for frame buffer error! (halted)\r\n");
		while (1);
	}
	memset(frame_buffer, 0, frame_buffer_size);
#endif
}

/*
 * display_system_init - display system entry point
 */
static void init_font(void);
static void display_system_init()
{
	/* [FIXME] fbcon.c will cuase BB */
	gGfxDev = video_hw_init();
	if(gGfxDev == NULL) {
		dprintf(CRITICAL, "%s: gGfxDev is NULL, show current menu item using debug message\r\n", __FUNCTION__);
		return;
	}

	dprintf(SPEW, "init frame buffer\r\n");
	frame_buffer_size = display_height()*display_width()*(display_bpp() / 8);

	init_font();

	display_allocate_buffers(gGfxDev->frameAdrs);
	current_frame_buffer = frame_buffer;

	draw_background_black_or_white(BACKGND_COLOR);
}

void draw_the_pixel(uint32_t pixel_offset, uint32_t paint)
{
	unsigned byte_per_pixel = display_bpp() / 8;
	uint8_t *pixels = current_frame_buffer + (pixel_offset * (byte_per_pixel));

	if (byte_per_pixel == 2){
		*pixels = (paint >> 8) & 0xFF;
		*(pixels + 1) = paint & 0xFF;
	} else if(byte_per_pixel == 3) {
		*pixels = (paint >> 16) & 0xFF;
		*(pixels + 1) = (paint >> 8) & 0xFF;
		*(pixels + 2) = paint & 0xFF;
	} else if(byte_per_pixel == 4) {
		*pixels = (paint >> 16) & 0xFF;
		*(pixels + 1) = (paint >> 8) & 0xFF;
		*(pixels + 2) = paint & 0xFF;
		*(pixels + 3) = 0xFF;
	} else {
		dprintf(CRITICAL, "byte per pixel should be either 2 or 3, not [%u]\n", byte_per_pixel);
	}
}

//paint all panel with black or white, 0xffffff is white, other is black
void draw_background_black_or_white(uint32_t paint)
{
	char byte_value=0;
	if(paint == COLOR_WHITE)
		byte_value = 0xff;

	memset(gGfxDev->frameAdrs, byte_value, display_width() * display_height() * (display_bpp() / 8));
}

void display_menu(void);

void display_change_to_download(void)
{
	g_boot_mode = HTC_DOWNLOAD;
	boot_linux_from_storage();
}


void display_change_to_recovery(void)
{
	g_boot_mode = RECOVERY_BOOT;
	boot_linux_from_storage();
	return;
}

void display_change_to_reboot(void)
{
	dprintf(CRITICAL, "rebooting the device\n");
	mtk_arch_reset(1); //bypass pwr key when reboot
}

void display_change_to_reboot_bootloader(void)
{
	dprintf(CRITICAL, "rebooting the device to bootloader\n");
	Set_Clr_RTC_PDN1_bit13(true); //Set RTC fastboot bit
	mtk_arch_reset(1); //bypass pwr key when reboot
}

void display_change_to_power_down(void)
{
	while(keys_get_state(KEY_POWER))
	{;}

	radio_powerdown();
	return;
}

void display_change_to_ramdump_usb(void)
{
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	htc_ramdump2USB();
	return;
}

void display_change_to_ramdump_emmc(void)
{
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	htc_ramdump2eMMC();
	return;
}

void display_change_to_ddrtest(void)
{
#if _DDR_TEST
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	htc_setting_set_bootmode(BOOTMODE_DDRTEST);
	ddrtest_menu_init();
#endif
	return;
}

void display_change_to_preloader_ddrtest(void)
{
#if _DDR_TEST
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	htc_set_reboot_reason(HTC_REBOOT_INFO_DDRTEST, NULL);
	mtk_arch_reset(1); //bypass pwr key when reboot
#endif
	return;
}


void display_change_to_lk_ddrtest(void)
{
#if _DDR_TEST
	extern void htc_ddrtest_init();
	extern int ddr_test(void);

	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	/* initialize ddrtest_dev */
	htc_ddrtest_init();

	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	thread_t *thr = NULL;
	thr = thread_create("ddrtest", ddr_test, 0, DEFAULT_PRIORITY, 4096);
	thread_resume(thr);
#endif
	return;
}

void display_change_to_menu_test1(void)
{
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	return;
}
void display_change_to_menu_test2(void)
{
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	return;
}
void display_change_to_menu_test3(void)
{
	dprintf(CRITICAL, "%s\r\n", __FUNCTION__);
	return;
}



static bootmode_option_t bootmode_options[] =
{
	[BOOTMODE_OPTION_DOWNLOAD] = { "BOOT TO DOWNLOAD MODE", display_change_to_download, YELLOWGREEN_C },
	[BOOTMODE_OPTION_RECOVERY] = { "BOOT TO RECOVERY MODE", display_change_to_recovery, LIMEGREEN_C },
	[BOOTMODE_OPTION_REBOOT] = { "REBOOT", display_change_to_reboot, GOLD_C },
	[BOOTMODE_OPTION_REBOOT_BOOTLOADER] = { "REBOOT TO BOOT-LOADER", display_change_to_reboot_bootloader, PURPLE_C },
	[BOOTMODE_OPTION_POWER_DOWN] = { "POWER DOWN", display_change_to_power_down, ORANGERED_C },
	[BOOTMODE_OPTION_RAMDUMP_USB] = { "RAMDUMP TO USB", display_change_to_ramdump_usb, PURPLE_C },
	[BOOTMODE_OPTION_RAMDUMP_EMMC] = { "RAMDUMP TO EMMC", display_change_to_ramdump_emmc, ORANGERED_C },
	[BOOTMODE_OPTION_DDRTEST] = { "DDR TEST", display_change_to_ddrtest, MAGENTA_C },
	[BOOTMODE_OPTION_PRELOADER_DDRTEST] = { "PRELOADER DDR TEST", display_change_to_preloader_ddrtest, YELLOWGREEN_C },
	[BOOTMODE_OPTION_LK_DDRTEST] = { "LK DDR TEST", display_change_to_lk_ddrtest, LIMEGREEN_C },

	[BOOTMODE_OPTION_MENU_TEST1] = { "Menu Test 1", display_change_to_menu_test1, GOLD_C },
	[BOOTMODE_OPTION_MENU_TEST2] = { "Menu Test 2", display_change_to_menu_test2, PURPLE_C },
	[BOOTMODE_OPTION_MENU_TEST3] = { "Menu Test 3", display_change_to_menu_test3, ORANGERED_C },
};

static const bootmode_option_list_t* fastboot_support_bootmodes = NULL;
static const bootmode_option_list_t* bootloader_support_bootmodes = NULL;
static const bootmode_option_list_t* ramdump_support_bootmodes = NULL;
static const bootmode_option_list_t* ddrtest_support_bootmodes = NULL;


/* Add new node to boot mode option list */
void set_supported_bootmode_option(const bootmode_option_list_t** bootmode, bootmode_option_enum option)
{
	bootmode_option_list_t* head;
	bootmode_option_list_t* new_node;

	if (option <= BOOTMODE_OPTION_FIRST && option >= BOOTMODE_OPTION_LAST) {
		dprintf(CRITICAL, "%s: option out of range\r\n", __FUNCTION__);
		goto fail_set_supported_bootmode_option;
	}

	new_node = (bootmode_option_list_t*) malloc(sizeof(bootmode_option_list_t));

	if (new_node == NULL) {
		dprintf(CRITICAL, "%s: failed to allocate memory\r\n", __FUNCTION__);
		goto fail_set_supported_bootmode_option;
	}

	if (*bootmode == NULL) {
		new_node->next = new_node;
		new_node->prev = new_node;
		*bootmode = new_node;
	} else {
		head = (*bootmode)->next->prev;
		new_node->next = head;
		new_node->prev = head->prev;
		head->prev->next = new_node;
		head->prev = new_node;
	}

	new_node->node = bootmode_options[option];

fail_set_supported_bootmode_option:
	return;
}

void set_fastboot_support_bootmode(bootmode_option_enum option)
{
	set_supported_bootmode_option(&fastboot_support_bootmodes, option);
}

void set_htcbl_support_bootmode(bootmode_option_enum option)
{
	set_supported_bootmode_option(&bootloader_support_bootmodes, option);
}

void set_ramdump_support_bootmode(bootmode_option_enum option)
{
        set_supported_bootmode_option(&ramdump_support_bootmodes, option);
}

void set_ddrtest_support_bootmode(bootmode_option_enum option)
{
        set_supported_bootmode_option(&ddrtest_support_bootmodes, option);
}

static const bootmode_option_list_t* bootmode_focus_row = NULL;
const bootmode_option_list_t* display_get_current_support_bootmodes(void)
{
	static const bootmode_option_list_t* support_bootmodes = NULL;

	switch (htc_setting_get_bootmode())
	{
	/*
		case BOOTMODE_HTCBL:
			if (support_bootmodes != bootloader_support_bootmodes) {
				support_bootmodes = bootloader_support_bootmodes;
				bootmode_focus_row = support_bootmodes;
			}
			break;
	*/
		default:
		case BOOTMODE_FASTBOOT:
			if (support_bootmodes != fastboot_support_bootmodes) {
				support_bootmodes = fastboot_support_bootmodes;
				bootmode_focus_row = support_bootmodes;
			}
			break;
		 case BOOTMODE_RAMDUMP:
                        if (support_bootmodes != ramdump_support_bootmodes) {
                                support_bootmodes = ramdump_support_bootmodes;
                                bootmode_focus_row = support_bootmodes;
                        }
                        break;
		 case BOOTMODE_DDRTEST:
                        if (support_bootmodes != ddrtest_support_bootmodes) {
                                support_bootmodes = ddrtest_support_bootmodes;
                                bootmode_focus_row = support_bootmodes;
                        }
                        break;
	}

	return support_bootmodes;
}

/* update message in message area */
struct _sd_message {
	int start_row;
	int col;
	int row;
};

static struct _sd_message sd_message = {
	.start_row = 9,
	.col = 3,
	.row = 9,
};

/* update message in information area */
static struct _sd_message sd_info = {
	.start_row = 10,
	.col = 1,
	.row = 10,
};

static struct _sd_message sd_debug = {
        .start_row = 15,
        .col = 1,
        .row = 10,
};

static uint32_t chW, chH;
static uint8_t* font;
static unsigned int max_imgname_length;

/*
return	- false, in frame max. range
	- true, out of max. frame buffer range
*/
static bool ChkFrameBufOffset(uint32_t offset)
{
	if(offset >= display_width() * display_height())
		return false;

	return true;
}

static void init_font(void)
{
	if (display_height() / 48 > 30) {
		chW = char48_width;
		chH = char48_height;
		font = (uint8_t*) asc48bpp;
	} else if (display_height() / 36 > 30) {
		chW = char36_width;
		chH = char36_height;
		font = (uint8_t*) asc36bpp;
	} else {
		chW = char24_width;
		chH = char24_height;
		font = (uint8_t*) asc24bpp;
	}

	max_imgname_length = display_width() / chW - 15;
	if (max_imgname_length > 25)
		max_imgname_length = 25;
}

//
// Row pixel position = wRow
// Clumn pixel position = wColumn
// Propose: Show one character in screen
// color: set the color of font, ex: red=0xF800, green=0x07E0, blue=0x001F.
static void ShowCharacterColor(uint32_t row, uint32_t col, char c, bool invert,
	uint32_t color, bool is_ruu_mode)
{
	uint32_t pixel_offset;
	void* frame_buf;
	const uint8_t* the_font;
	bool pixel;

	frame_buf = current_frame_buffer;

	if (frame_buf == NULL) {
		// dprintf(CRITICAL, "frame_bufer was not initialized!\r\n");
		return;
	}

	the_font = (uint8_t*) font + c * chH * (chW + 7) / 8;

	pixel_offset = col + row * display_width(); /* compute pixel offset */

	if (!ChkFrameBufOffset(pixel_offset + chW))
		return;

	for (uint32_t w1 = 0; w1 < chH; ++w1) {
		unsigned shift = 0;
		for (uint32_t w2 = 0; w2 < chW; ++w2) {
			shift = w2 % 8;
			pixel = (((*the_font << shift) & 0x80) != 0);

			if ((pixel ^ invert) & 1) {
				/* draw the pixel */
				//*(frame_buf + pixel_offset + w2) = (uint16_t) color;
				draw_the_pixel(pixel_offset + w2, color);
			}

			if (shift == 7) {
				++the_font;
			}
		}

		if (shift < 7) {
			++the_font;
		}
		pixel_offset += display_width();

		if (!ChkFrameBufOffset(pixel_offset + chW))
			return;
	}
}

//
//  Show String in color screen
//
// color: set the color of font, ex: red=0xF800, green=0x07E0, blue=0x001F.
static void ShowStringColor(uint32_t wPixRow, uint32_t wPixColumn, const char* lpBuf, bool bInvert,
	uint32_t color, bool is_ruu_mode)
{
	int i = 0;

	while (*lpBuf > 0) {
		ShowCharacterColor(wPixRow, (wPixColumn + i * chW), *lpBuf,
				   bInvert, color, is_ruu_mode);
		lpBuf++;
		i++;
	}

}

//  Clear String in screen
//
// color: set the color of font, ex: red=0xF800, green=0x07E0, blue=0x001F.
static void ClearStringInsScreen(uint32_t wRow, int StartCol, int EndCol, uint32_t wColor, int is_ruu_mode)
{
	const char ClearChar = 0x0;
	int i;
	wRow *= chH;
	if (wRow > (display_height() - chH))
		wRow = display_height() - chH;

	for (i = StartCol; i < EndCol; i++) {
		ShowCharacterColor(wRow, (i * chW), ClearChar, true, wColor, is_ruu_mode != 0);
	}
}

static void ClearColumnStringInsScreen(uint32_t wRow, uint32_t wColor, int is_ruu_mode)
{
	ClearStringInsScreen(wRow, 0, display_width() / chW, wColor, is_ruu_mode);
}

void display_write(uint32_t wColumn, uint32_t wRow, const char* lpBuf, bool bInvert, uint32_t color)
{
	ShowStringColor(wRow * chH, wColumn * chW, lpBuf, bInvert, color, 0);
	//ShowStringColor(wColumn * chH, wRow * chW, lpBuf, bInvert, color, 0);
}

void display_sd_info(const char * data, uint32_t chr_color)
{
	display_write(sd_info.col, sd_info.row, data, 0, chr_color);
	sd_info.row++;
}

void display_sd_info_clr_all(void)
{
	for ( ; ; ) {
		if (sd_info.row > sd_info.start_row) {
			sd_info.row--;
			ClearColumnStringInsScreen(sd_info.row, BACKGND_COLOR, 0);
		}else
			break;
	}
}

void display_sd_info_init()
{
	sd_info.start_row = sd_message.row;
	sd_info.row = sd_info.start_row;
}

/* ramdump++ */
void display_sd_debug_info(const char * data, uint32_t chr_color)
{
        display_write(sd_debug.col, sd_debug.row, data, 0, chr_color);
        sd_debug.row++;
	display_update_screen();
}

void display_sd_debug_clr_all(void)
{
        for ( ; ; ) {
                if (sd_debug.row > sd_debug.start_row) {
                        sd_debug.row--;
                        ClearColumnStringInsScreen(sd_debug.row, BACKGND_COLOR, 0);
                }else
                        break;
        }
}

void display_sd_debug_init()
{
	sd_debug.start_row = sd_info.row +1;
	sd_debug.row = sd_debug.start_row;
}
/* ramdump -- */

struct keyinfo_string {
	char up_str[32];	//For  Menu Selection
	char down_str[32];
	char select_str[32];
};
static struct keyinfo_string keyinfo;

void display_set_funkey_str(uint32_t keyfunction_id)
{
	switch(keyfunction_id) {
		case FUNKEY_BAR_DOWN:	 //For  Menu Selection
			sprintf(keyinfo.down_str, "%s to next item", "VOL Down");
			break;
		case FUNKEY_BAR_UP:
			sprintf(keyinfo.up_str, "%s to previous item", "VOL Up");
			break;
		case FUNKEY_BAR_SELECT:
			sprintf(keyinfo.select_str, "%s to select item", "Power Key");
			break;
		default:
			break;
	}
}

const char *display_get_funkey_str(unsigned keyfunction_id)
{
	switch(keyfunction_id) {
		case FUNKEY_BAR_DOWN:	//For  Menu Selection
			return keyinfo.down_str;
			break;
		case FUNKEY_BAR_UP:
			return keyinfo.up_str;
			break;
		case FUNKEY_BAR_SELECT:
			return keyinfo.select_str;
			break;
		default:
			break;
	}
	return "";
}

void display_menu_option(void)
{
	const bootmode_option_list_t* head = display_get_current_support_bootmodes();
	const bootmode_option_list_t* iter = head;

	if(gGfxDev == NULL)
		dprintf(CRITICAL, "\n\n\n[MENU]\n");

	while (iter != NULL) {
		if (unlikely(iter == bootmode_focus_row)) {
			if(gGfxDev != NULL) {
				display_write(sd_info.col, sd_info.row, iter->node.display_string, 1, BLUE_C);
				sd_info.row++;
			} else {
				//no panel, show only menu items using debug message
				dprintf(CRITICAL, "[MENU] * %s\n", iter->node.display_string);
			}
		} else {
			if(gGfxDev != NULL)
				display_sd_info(iter->node.display_string, iter->node.display_color);
			else //no panel, show only menu items using debug message
				dprintf(CRITICAL, "[MENU]   %s\n", iter->node.display_string);
		}

		if (iter->next == head)
			break;

		iter = iter->next;
	}
}


void display_reboot_reason()
{
	/* HTC: show reset reason */
	int x_limit = (display_width()/chW/3)*2-5;
	int x = display_width()/chW/3;
	int y = display_height()/chH-8;

        if(strlen(aee_get_reboot_string())>0)
	{
                char *ptr = aee_get_reboot_string();
		char str[128]= {0};
                if (strlen(ptr) > x_limit)
                {
                        strncpy(str, aee_get_reboot_string(), x_limit);
			display_write(x, y++, str, 0, BLUE_C);
			ptr = aee_get_reboot_string()+x_limit;
			display_write(x, y++, ptr, 0, BLUE_C);
                }else
                        display_write(x, y++, ptr, 0, BLUE_C);


		snprintf(str, sizeof(str), "IMEI: 0x%8x\n", htc_get_reboot_reason());
		display_write(x, y++, str, 0, RED_C);

        }
}

void htc_voprintf(const char *msg, ...)
{
       va_list ap;
       char msgbuf[128], *p;

       va_start(ap, msg);
       p = msgbuf;
       if (msg[0] == '\r') {
               *p++ = msg[0];
               msg++;
       }
       vsnprintf(p, sizeof(msgbuf) - (p - msgbuf), msg, ap);
       display_ddrtest_info(msgbuf);
       dprintf(CRITICAL,"%s", msgbuf);
       va_end(ap);
}

void display_ddrtest_info(char* info)
{
	/* HTC: show ddrtest info */
	int x_limit = (display_width()/chW)-5;
	int y_limit = (display_height()/chH)-5;
	int x = 1;
	int clear_y = 0;
	static int y = 0;
	char str[128]= {0};
	char *ptr;

	if (y==0)
		y = (display_height()/chH/3)+5;
	if (y==y_limit){
		y = (display_height()/chH/3)+5;
		clear_y = (display_height()/chH/3)+5;
		for ( ; ; ) {
			if (clear_y < y_limit) {
				ClearColumnStringInsScreen(clear_y, BACKGND_COLOR, 0);
				clear_y++;
			}else
				break;
		}
	}

	if(strlen(info)>0)
	{
		ptr = info;
                if (strlen(info) > x_limit)
                {
			strncpy(str, info, x_limit);
			display_write(x, y++, str, 0, BLUE_C);
			ptr = info+x_limit;
			display_write(x, y++, ptr, 0, BLUE_C);
                }else
                        display_write(x, y++, ptr, 0, BLUE_C);
        }
	display_update_screen();
}

void display_lk_info() {
	char str[64]={0};
	char* secure_flag_str[] = {"S-OFF", "S-ENG", "S-DBG", "S-ON", "S-?"};
	char* p_secure_flag_str = NULL;
	int secure_flag = read_security_level();

	if (secure_flag < SECLEVEL_MFG || secure_flag > SECLEVEL_USER)
	{
		p_secure_flag_str = secure_flag_str[SECLEVEL_MAX];
	}
	else
	{
		p_secure_flag_str = secure_flag_str[secure_flag];
	}

#if TAMPER_DETECT
		display_write(sd_info.col, 0, g_tamper_string, 1, DARKGRAY_C);
#endif

	display_write(sd_info.col, 1, g_unlock_string, 0, MAGENTA_C);

	snprintf(str, sizeof(str), "%s %d %s %s", PROJECT, htc_setting_pcbid(), HTC_LK_BUILD_MODE, p_secure_flag_str);
	display_write(sd_info.col, 2, str, 0, LIMEGREEN_C);

	if (boot_get_Security_Check_Fail_flag())
		display_write(sd_info.col, 3, SEC_BOOT_WARNING, 1, MAGENTA_C);

	snprintf(str, sizeof(str), "CID-%s", htc_setting_cid());
	display_write(sd_info.col, 4, str, 0, LIMEGREEN_C);

	snprintf(str, sizeof(str), "LK-%s %s", HTC_LK_VERSION, HTC_LK_COMMIT_ID);
	display_write(sd_info.col, 5, str, 0, LIMEGREEN_C);

	snprintf(str, sizeof(str), "OS-%s", htc_setting_get_firmware_main_version());
	display_write(sd_info.col, 6, str, 0, LIMEGREEN_C);
}

void display_menu(void)
{
	if(gGfxDev == NULL) {
		//no panel, show only menu items using debug message
		display_menu_option();
		return;
	}

	display_sd_info_clr_all();
	display_sd_info_init();

	display_lk_info();


	if (strlen(display_get_funkey_str(FUNKEY_BAR_UP)))
		display_sd_info(display_get_funkey_str(FUNKEY_BAR_UP), DARKORANGE_C);
	if (strlen(display_get_funkey_str(FUNKEY_BAR_DOWN)))
		display_sd_info(display_get_funkey_str(FUNKEY_BAR_DOWN), DARKORANGE_C);
	if (strlen(display_get_funkey_str(FUNKEY_BAR_SELECT)))
		display_sd_info(display_get_funkey_str(FUNKEY_BAR_SELECT), DARKORANGE_C);
	display_sd_info("", WHITE_C);

	display_menu_option();

	/* HTC: show reset reason */
	if(htc_setting_get_bootmode() == BOOTMODE_RAMDUMP)
		display_reboot_reason();

	mt_disp_update(0, 0, display_width(), display_height());

	return;
}

void display_show_confidential(void)
{
#if !defined (STOCKUI_ROM)
	int nr_row = display_height() / chH * 3 / 5;
	int centre_col = display_width() / chW;

	//PR_DEBUG("%s: nr_row: %d\n", __FUNCTION__, nr_row);
	//PR_DEBUG("%s: centre_col: %d\n", __FUNCTION__, centre_col);

	display_write((centre_col - 17) / 2 , nr_row, "This build is for", 0, RED_C);
	display_write((centre_col - 25) / 2, nr_row + 1, "development purposes only", 0, RED_C);
	display_write((centre_col - 32) / 2, nr_row + 2, "Do not distribute outside of HTC", 0, RED_C);
	display_write((centre_col - 33) / 2, nr_row + 3, "without HTC's written permission.", 0, RED_C);
	display_write((centre_col - 21) / 2, nr_row + 4, "Failure to comply may", 0, RED_C);
	display_write((centre_col - 21) / 2, nr_row + 5, "lead to legal action.", 0, RED_C);

	display_update_screen();
#endif
}

int display_update_screen(void)
{
	mt_disp_update(0, 0, display_width(), display_height());
}

static void display_increase_focus_row()
{

	bootmode_focus_row = bootmode_focus_row->next;
	return;
}

static void display_decrease_focus_row()
{
	bootmode_focus_row = bootmode_focus_row->prev;
	return;
}

int display_goto_next_bootmode(void)
{
	display_increase_focus_row();
	display_menu();

	/* no change */
	return 0;
}

int display_goto_previous_bootmode(void)
{
	display_decrease_focus_row();
	display_menu();

	/* no change */
	return 0;
}

int display_select_current_bootmode(void)
{
	bootmode_focus_row->node.change_bootmode();
	return htc_setting_get_bootmode();
}

void simple_menu_init(void)
{
	display_system_init();

	display_set_funkey_str(FUNKEY_BAR_UP);
	display_set_funkey_str(FUNKEY_BAR_DOWN);
	display_set_funkey_str(FUNKEY_BAR_SELECT);

	set_fastboot_support_bootmode(BOOTMODE_OPTION_REBOOT);
	set_fastboot_support_bootmode(BOOTMODE_OPTION_REBOOT_BOOTLOADER);
	set_fastboot_support_bootmode(BOOTMODE_OPTION_DOWNLOAD);
	set_fastboot_support_bootmode(BOOTMODE_OPTION_RECOVERY);
	set_fastboot_support_bootmode(BOOTMODE_OPTION_DDRTEST);
	set_fastboot_support_bootmode(BOOTMODE_OPTION_POWER_DOWN);

	display_menu();
}

/* HTC: ramdump ++ */
void ramdump_menu_init(void)
{
        display_system_init();

        display_set_funkey_str(FUNKEY_BAR_UP);
        display_set_funkey_str(FUNKEY_BAR_DOWN);
        display_set_funkey_str(FUNKEY_BAR_SELECT);

	set_ramdump_support_bootmode(BOOTMODE_OPTION_REBOOT);
	set_ramdump_support_bootmode(BOOTMODE_OPTION_REBOOT_BOOTLOADER);
	set_ramdump_support_bootmode(BOOTMODE_OPTION_RAMDUMP_USB);
	set_ramdump_support_bootmode(BOOTMODE_OPTION_RAMDUMP_EMMC);

	display_menu();
}

/* HTC: ddrtest ++ */
void ddrtest_menu_init(void)
{
	display_system_init();

	display_set_funkey_str(FUNKEY_BAR_UP);
	display_set_funkey_str(FUNKEY_BAR_DOWN);
	display_set_funkey_str(FUNKEY_BAR_SELECT);

	set_ddrtest_support_bootmode(BOOTMODE_OPTION_REBOOT);
	set_ddrtest_support_bootmode(BOOTMODE_OPTION_REBOOT_BOOTLOADER);
	set_ddrtest_support_bootmode(BOOTMODE_OPTION_PRELOADER_DDRTEST);
	set_ddrtest_support_bootmode(BOOTMODE_OPTION_LK_DDRTEST);

	display_menu();
}

void display_print_ramdump_files(struct ramdump_file_list *filelist, int file_num)
{
        int i = 0, count = 0;
        char fn[MAX_FILENAME_LEN + 16] = {0};

        //SD_UPDATE_DBG("%s: file_num = %d\r\n", __func__, file_num);

        for (i = 0; i < file_num; i++) {
                filelist[i].display_row = sd_info.row;
                sprintf(fn,"[%d] %s", ++count, filelist[i].filename);
                display_sd_info(fn, LIMEGREEN_C);
        }
}

/* HTC: ramdump -- */

