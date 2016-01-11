/* BEGIN PN:DTS2013053103858 , Added by d00238048, 2013.05.31*/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
    #include <platform/disp_drv_platform.h>

#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    //#include <linux/delay.h>
    #include <mach/mt_gpio.h>
#endif
#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03


const static unsigned char LCD_MODULE_ID = 0x02;
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH										(720)
#define FRAME_HEIGHT										(1280)


#define REGFLAG_DELAY										0xFC
#define REGFLAG_END_OF_TABLE								0xFD   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};


//update initial param for IC nt35521 0.01
static struct LCM_setting_table lcm_initialization_setting[] = {

#if 0
	// Set EXTC
	{0xB9,  3 ,{0xFF, 0x83, 0x94}},

	// Set MIPI
	{0xBA,  11 ,{0x73, 0x43,0xa0,0x65,0xb2,0x09,0x09,0x40,0x10,0x00,0x00}},

	// Set Power
	{0xB1,  15 ,{0x6C, 0x11, 0x11, 0x24,
					0x04, 0x11, 0xF1, 0x80,
					0xEA, 0x96, 0x23, 0x80,
					0xC0, 0xD2, 0x58}},

	// Set Display
	{0xB2,  11 ,{0x00, 0x64, 0x10, 0x07,
					0x32, 0x1C, 0x08, 0x08,
					0x1C, 0x4D, 0x00}},

	{0xBC,  1 ,{0x07}},

	{0xBF,  3 ,{0x41,0x0E,0x01}},

	// Set CYC
	{0xB4,  12 ,{0x00, 0xFF, 0x40, 0x50,
					0x40, 0x50, 0x40, 0x50,
					0x01, 0x6A, 0x01, 0x6A}},

	// Set D3
	{0xD3,  30 ,{0x00, 0x06, 0x00, 0x40,
					0x07, 0x00, 0x00, 0x32,
					0x10, 0x08, 0x00, 0x08,
					0x52, 0x15, 0x0F, 0x05,
					0x0F, 0x32, 0x10, 0x00,
					0x00, 0x00, 0x47, 0x44,
					0x0C, 0x0C, 0x47, 0x0C,
					0x0C, 0x47}},

	// Set GIP
	{0xD5,  44 ,{0x20, 0x21, 0x00, 0x01,0x02,0x03,
					0x04, 0x05, 0x06, 0x07,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x19, 0x19,0x18, 0x18,
					0x24, 0x25}},

	// Set D6
	{0xD6,  44 ,{0x24, 0x25, 0x07, 0x06,0x05,0x04,
					0x03, 0x02, 0x01, 0x00,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x58, 0x58, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x19, 0x19,
					0x20, 0x21}},

	// Set E0
	{0xE0,  42 ,{0x00, 0x01, 0x05, 0x31,0x38,0x3F,
					0x15, 0x3D, 0x06, 0x09,0x0B, 0x16,
					0x0E, 0x12, 0x15, 0x13,0x14, 0x07,
					0x11, 0x13, 0x18, 0x00,0x01, 0x05,
					0x31, 0x37, 0x3F, 0x14,0x3E, 0x06,
					0x09, 0x0B, 0x17, 0x0F,0x12, 0x15,
					0x13, 0x14, 0x07, 0x11,0x12, 0x16}},

	// Set BD
	{0xBD,  1 ,{0x00}},


    // Set c1
	{0xC1,  43 ,{0x01, 0x00, 0x08, 0x10,0x17,0x1F,
					0x26, 0x2E, 0x35, 0x3D,0x44, 0x4B,
					0x53, 0x5B, 0x62, 0x6A,0x72, 0x79,
					0x81, 0x89, 0x91, 0x99,0xA1, 0xA9,
					0xB1, 0xB9, 0xC1, 0xCA,0xD2, 0xDA,
					0xE3, 0xEC, 0xF5, 0xFF,0x16, 0x27,
					0xFB, 0x29, 0xD5, 0x45,0x22, 0xFF,
					0xC0}},

	// Set BD
	{0xBD,  1 ,{0x01}},

    // Set c1
	{0xC1,  42 ,{0x00, 0x08, 0x10,0x17,0x1F,0x26,
					0x2E, 0x35, 0x3D,0x44, 0x4B,0x53,
					0x5B, 0x62, 0x6A,0x72, 0x79,0x81,
					0x89, 0x91, 0x99,0xA1, 0xA9,0xB1,
					0xB9, 0xC1, 0xCA,0xD2, 0xDA,0xE3,
					0xEC, 0xF5, 0xFF,0x16, 0x27,0xFB,
					0x29, 0xD5, 0x45, 0x22,0xFF, 0xC0}},

    // Set BD
	{0xBD,  1 ,{0x02}},

    // Set c1
	{0xC1,  42 ,{0x00, 0x08, 0x10,0x17,0x1F,0x26,
					0x2E, 0x35, 0x3D,0x44, 0x4B,0x53,
					0x5B, 0x62, 0x6A,0x72, 0x79,0x81,
					0x89, 0x91, 0x99,0xA1, 0xA9,0xB1,
					0xB9, 0xC1, 0xCA,0xD2, 0xDA,0xE3,
					0xEC, 0xF5, 0xFF,0x16, 0x27,0xFB,
					0x29, 0xD5, 0x45, 0x22,0xFF, 0xC0}},


	{0xB6,  2 ,{0x07,0x07}},

	// Set Panel
	{0xCC,  1 ,{0x09}},
    // Set Panel
	{0xC0,  2 ,{0x30,0x14}},


	// Set TCON Option
	{0xC7,  4 ,{0x00, 0xC0, 0x40, 0xC0}},



	// Sleep Out
	{0x11,0,{}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29,0,{}},
	{REGFLAG_DELAY, 10,{}},


    {REGFLAG_END_OF_TABLE, 0x00, {}}
#else

   // Set EXTC
	{0xB9,  3 ,{0xFF, 0x83, 0x94}},

	// Set MIPI
	//{0xBA,  11 ,{0x73, 0x83}},
	{0xBA,  11 ,{0x73, 0x83,0xa0,0x65,0xb2,0x09,0x09,0x40,0x10,0x00,0x00}},
	//{0xBA,  11 ,{0x73, 0x43}},

	// Set Power
	{0xB1,  15 ,{0x6C, 0x12, 0x12, 0x24,
					0x04, 0x11, 0xF1, 0x80,
					0xEA, 0x96, 0x23, 0x80,
					0xC0, 0xD2, 0x58}},

	// Set Display
	{0xB2,  11 ,{0x00, 0x64, 0x10, 0x07,
					0x32, 0x1C, 0x08, 0x08,
					0x1C, 0x4D, 0x00}},

	{0xBC,  1 ,{0x07}},

	//{0xBF,  3 ,{0x41,0x0E,0x01}},

	// Set CYC
	{0xB4,  12 ,{0x00, 0xFF, 0x01, 0x50,
					0x01, 0x50, 0x01, 0x50,
					0x07, 0x6A, 0x11, 0x6A}},

	// Set D3
	{0xD3,  30 ,{0x00, 0x00, 0x00, 0x40,
					0x07, 0x00, 0x00, 0x32,
					0x10, 0x09, 0x00, 0x09,
					0x52, 0x15, 0x11, 0x05,
					0x11, 0x32, 0x10, 0x00,
					0x00, 0x00, 0x37, 0x33,
					0x0D, 0x0D, 0x37, 0x0C,
					0x0C, 0x47}},

	// Set GIP
	{0xD5,  44 ,{0x20, 0x21, 0x00, 0x01,0x02,0x03,
					0x04, 0x05, 0x06, 0x07,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x19, 0x19,0x18, 0x18,
					0x24, 0x25}},

	// Set D6
	{0xD6,  44 ,{0x24, 0x25, 0x07, 0x06,0x05,0x04,
					0x03, 0x02, 0x01, 0x00,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x58, 0x58, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x18, 0x18,
					0x18, 0x18, 0x18, 0x18,0x19, 0x19,
					0x20, 0x21}},

	// Set E0
	{0xE0,  42 ,{0x00, 0x01, 0x05, 0x31,0x38,0x3F,
					0x1B, 0x3E, 0x07, 0x0A,0x0C, 0x17,
					0x10, 0x14, 0x17, 0x15,0x16, 0x09,
					0x13, 0x15, 0x1A, 0x00,0x01, 0x05,
					0x31, 0x37, 0x3F, 0x1A,0x3F, 0x07,
					0x0A, 0x0C, 0x18, 0x11,0x14, 0x17,
					0x15, 0x16, 0x09, 0x13,0x14, 0x18}},

	// Set BD
	{0xBD,  1 ,{0x00}},


    // Set c1
	{0xC1,  43 ,{0x01, 0x00, 0x08, 0x10,0x17,0x1F,
					0x27, 0x2F, 0x36, 0x3D,0x47, 0x4F,
					0x57, 0x5F, 0x67, 0x6F,0x77, 0x7F,
					0x86, 0x8F, 0x96, 0x9E,0xA6, 0xAE,
					0xB7, 0xBE, 0xC5, 0xCF,0xD7, 0xDF,
					0xE7, 0xEF, 0xF7, 0xFF,0x12, 0x61,
					0xC4, 0x15, 0x22, 0xB8,0x35, 0x09,
					0xC0}},

	// Set BD
	{0xBD,  1 ,{0x01}},

    // Set c1
	{0xC1,  42 ,{0x00, 0x08, 0x10,0x17,0x1F,0x27,
					0x2F, 0x36, 0x3E,0x47, 0x4F,0x57,
					0x5F, 0x67, 0x6F,0x77, 0x7F,0x87,
					0x8F, 0x97, 0x9F,0xA7, 0xAF,0xB7,
					0xBE, 0xC5, 0xCF,0xD7, 0xDF,0xE7,
					0xEF, 0xF7, 0xFF,0x12, 0x61,0x18,
					0x7F, 0x44, 0x2B, 0x9B,0xEE, 0xC0}},

    // Set BD
	{0xBD,  1 ,{0x02}},

    // Set c1
	{0xC1,  42 ,{0x00, 0x09, 0x11,0x19,0x21,0x29,
					0x31, 0x39, 0x41,0x4A, 0x52,0x5A,
					0x62, 0x6B, 0x73,0x7B, 0x82,0x8A,
					0x93, 0x9B, 0xA3,0xAB, 0xB4,0xBB,
					0xC2, 0xCA,0xD3, 0xDB,0xE3,0xEB,
					0xF2, 0xF8,0xFF,0x15, 0xE5,0x99,
					0xDA, 0xAF, 0xE8,0x98,0x0F, 0xC0}},


	{0xB6,  2 ,{0x6E,0x6E}},

	// Set Panel
	{0xCC,  1 ,{0x09}},
    // Set Panel
	{0xC0,  2 ,{0x30,0x14}},


	// Set TCON Option
	{0xC7,  4 ,{0x00, 0xC0, 0x40, 0xC0}},


    {0x51,  1 ,{0xFF}},

	{0xC9,	7 ,{0x1F, 0xC0, 0x14, 0x1E,0x81,0x1E,0x00}},
    {0x55,  1 ,{0x00}},
    {0x5E,  1 ,{0x00}},
    {0xCA,0x09,{0x40,0x30,0x2F,0x2E,0x2D,0x26,0x23,0x21,0x20}},
    {0xCE,0x22,{0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00}},

	// Sleep Out
	{0x11,0,{}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29,0,{}},
	{REGFLAG_DELAY, 10,{}},

    {0x53,0x01,{0x24}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}

#endif
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                if(table[i].count <= 10)
                    MDELAY(table[i].count);
                else
                    MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    params->type   = LCM_TYPE_DSI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    params->physical_width = 68;
    params->physical_height = 121;

    params->dsi.mode   = SYNC_PULSE_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

   // Highly depends on LCD driver capability.
   //video mode timing

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 2;
    params->dsi.vertical_backporch				= 14;
    params->dsi.vertical_frontporch				= 15;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active				= 36;
    params->dsi.horizontal_backporch				= 90;
    params->dsi.horizontal_frontporch				= 90;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 230; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd			= 0x0A;
	params->dsi.lcm_esd_check_table[0].count		= 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x1C;

    params->pwm_min = 7;
    params->pwm_default = 94;
    params->pwm_max = 255;
    params->camera_blk = 194;
    params->camera_dua_blk = 194;
    params->camera_rec_blk = 194;
}
#if 0
/*to prevent electric leakage*/
static void lcm_id_pin_handle(void)
{
    mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
    mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
}
#endif

static void lcm_resume_init(void)
{
  unsigned int data_array[35];
#ifdef BUILD_LK
	printf("%s,uboot:hx8394d_lcm_init enter.\n", __func__);
#else
	printk("%s,kernel:hx8394d_lcm_init enter.\n", __func__);
#endif
	 //enable VSP & VSN  ,here to set Offset voltage
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    //mdelay(50);
	//reset high to low to high
    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(2);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(5);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(120);

    //lcm_id_pin_handle();
	// when phone initial , config output high, enable backlight drv chip
    // push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

	data_array[0]=0x00043902;
	data_array[1]=0x9483FFB9;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00033902;
	data_array[1]=0x008373BA;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00103902;
	data_array[1]=0x12126CB1;
	data_array[2]=0xF1110424;
	data_array[3]=0x2396EA80;
	data_array[4]=0x58D2C080;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0]=0x000C3902;
	data_array[1]=0x106400B2;
	data_array[2]=0x081C3207;
	data_array[3]=0x004D1C08;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000007BC;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x000D3902;
	data_array[1]=0x01FF00B4;
	data_array[2]=0x01500150;
	data_array[3]=0x116A0750;
	data_array[4]=0x0000006A;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0]=0x001F3902;
	data_array[1]=0x000000D3;
	data_array[2]=0x00000740;
	data_array[3]=0x00091032;
	data_array[4]=0x11155209;
	data_array[5]=0x10321105;
	data_array[6]=0x37000000;
	data_array[7]=0x370D0D33;
	data_array[8]=0x00470C0C;
	dsi_set_cmdq(data_array, 9, 1);

	data_array[0]=0x002D3902;
	data_array[1]=0x002120D5;
	data_array[2]=0x04030201;
	data_array[3]=0x18070605;
	data_array[4]=0x18181818;
	data_array[5]=0x18181818;
	data_array[6]=0x18181818;
	data_array[7]=0x18181818;
	data_array[8]=0x18181818;
	data_array[9]=0x18181818;
	data_array[10]=0x19181818;
	data_array[11]=0x24181819;
	data_array[12]=0x00000025;
	dsi_set_cmdq(data_array, 13, 1);

	data_array[0]=0x002D3902;
	data_array[1]=0x072524D6;
	data_array[2]=0x03040506;
	data_array[3]=0x18000102;
	data_array[4]=0x18181818;
	data_array[5]=0x58181818;
	data_array[6]=0x18181858;
	data_array[7]=0x18181818;
	data_array[8]=0x18181818;
	data_array[9]=0x18181818;
	data_array[10]=0x18181818;
	data_array[11]=0x20191918;
	data_array[12]=0x00000021;
	dsi_set_cmdq(data_array, 13, 1);

	data_array[0]=0x002B3902;
	data_array[1]=0x050100E0;
	data_array[2]=0x1B3F3831;
	data_array[3]=0x0C0A073E;
	data_array[4]=0x17141017;
	data_array[5]=0x13091615;
	data_array[6]=0x01001A15;
	data_array[7]=0x3F373105;
	data_array[8]=0x0A073F1A;
	data_array[9]=0x1411180C;
	data_array[10]=0x09161517;
	data_array[11]=0x00181413;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000000BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x002C3902;
	data_array[1]=0x080001C1;
	data_array[2]=0x271F1710;
	data_array[3]=0x473D362F;
	data_array[4]=0x675F574F;
	data_array[5]=0x867F776F;
	data_array[6]=0xA69E968F;
	data_array[7]=0xC5BEB7AE;
	data_array[8]=0xE7DFD7CF;
	data_array[9]=0x12FFF7EF;
	data_array[10]=0x2215C461;
	data_array[11]=0xC00935B8;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000001BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x002B3902;
	data_array[1]=0x100800C1;
	data_array[2]=0x2F271F17;
	data_array[3]=0x4F473E36;
	data_array[4]=0x6F675F57;
	data_array[5]=0x8F877F77;
	data_array[6]=0xAFA79F97;
	data_array[7]=0xCFC6BEB7;
	data_array[8]=0xEFE7DFD7;
	data_array[9]=0x6112FFF7;
	data_array[10]=0x2B447F18;
	data_array[11]=0x00C0EE9B;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000002BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x002B3902;
	data_array[1]=0x110900C1;
	data_array[2]=0x31292119;
	data_array[3]=0x524A4139;
	data_array[4]=0x736B625A;
	data_array[5]=0x938A827B;
	data_array[6]=0xB4ABA39B;
	data_array[7]=0xD3CAC2BB;
	data_array[8]=0xF2EBE3DB;
	data_array[9]=0xE515FFF8;
	data_array[10]=0xE8AFDA99;
	data_array[11]=0x00C00F98;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000009CC;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00033902;
	data_array[1]=0x001430C0;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00053902;
	data_array[1]=0x40C000C7;
	data_array[2]=0x000000C0;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]=0x00083902;
	data_array[1]=0x14001FC9;
	data_array[2]=0x001E811E;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x00000055;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(5);

	data_array[0]=0x00023902;
	data_array[1]=0x0000005E;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x000A3902;
	data_array[1]=0x2F3040CA;
	data_array[2]=0x23262D2E;
	data_array[3]=0x00002021;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0]=0x00233902;
	data_array[1]=0x000000CE;
	data_array[2]=0x10101010;
	data_array[3]=0x20202020;
	data_array[4]=0x30202020;
	data_array[5]=0x30303030;
	data_array[6]=0x30303030;
	data_array[7]=0x30303030;
	data_array[8]=0x30303030;
	data_array[9]=0x00003030;
	dsi_set_cmdq(data_array, 10, 1);

	data_array[0]= 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0]= 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	data_array[0]=0x00023902;
	data_array[1]=0x00000051;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x00002453;
	dsi_set_cmdq(data_array, 2, 1);

    //lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);

#ifdef BUILD_LK
	printf("%s,uboot:hx8394d_lcm_init exit.\n", __func__);
#else
	printk("%s,kernel:hx8394d_lcm_init exit.\n", __func__);
#endif
}


static void lcm_init(void)
{
  unsigned int data_array[35];
#ifdef BUILD_LK
	printf("%s,uboot:hx8394d_lcm_init enter.\n", __func__);
#else
	printk("%s,kernel:hx8394d_lcm_init enter.\n", __func__);
#endif
	 //enable VSP & VSN  ,here to set Offset voltage
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    //mdelay(50);
	//reset high to low to high
    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(2);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(5);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(120);

    //lcm_id_pin_handle();
	// when phone initial , config output high, enable backlight drv chip
    // push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

	data_array[0]=0x00043902;
	data_array[1]=0x9483FFB9;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00033902;
	data_array[1]=0x008373BA;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00103902;
	data_array[1]=0x12126CB1;
	data_array[2]=0xF1110424;
	data_array[3]=0x2396EA80;
	data_array[4]=0x58D2C080;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0]=0x000C3902;
	data_array[1]=0x106400B2;
	data_array[2]=0x081C3207;
	data_array[3]=0x004D1C08;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000007BC;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x000D3902;
	data_array[1]=0x01FF00B4;
	data_array[2]=0x01500150;
	data_array[3]=0x116A0750;
	data_array[4]=0x0000006A;
	dsi_set_cmdq(data_array, 5, 1);

	data_array[0]=0x001F3902;
	data_array[1]=0x000000D3;
	data_array[2]=0x00000740;
	data_array[3]=0x00091032;
	data_array[4]=0x11155209;
	data_array[5]=0x10321105;
	data_array[6]=0x37000000;
	data_array[7]=0x370D0D33;
	data_array[8]=0x00470C0C;
	dsi_set_cmdq(data_array, 9, 1);

	data_array[0]=0x002D3902;
	data_array[1]=0x002120D5;
	data_array[2]=0x04030201;
	data_array[3]=0x18070605;
	data_array[4]=0x18181818;
	data_array[5]=0x18181818;
	data_array[6]=0x18181818;
	data_array[7]=0x18181818;
	data_array[8]=0x18181818;
	data_array[9]=0x18181818;
	data_array[10]=0x19181818;
	data_array[11]=0x24181819;
	data_array[12]=0x00000025;
	dsi_set_cmdq(data_array, 13, 1);

	data_array[0]=0x002D3902;
	data_array[1]=0x072524D6;
	data_array[2]=0x03040506;
	data_array[3]=0x18000102;
	data_array[4]=0x18181818;
	data_array[5]=0x58181818;
	data_array[6]=0x18181858;
	data_array[7]=0x18181818;
	data_array[8]=0x18181818;
	data_array[9]=0x18181818;
	data_array[10]=0x18181818;
	data_array[11]=0x20191918;
	data_array[12]=0x00000021;
	dsi_set_cmdq(data_array, 13, 1);

	data_array[0]=0x002B3902;
	data_array[1]=0x050100E0;
	data_array[2]=0x1B3F3831;
	data_array[3]=0x0C0A073E;
	data_array[4]=0x17141017;
	data_array[5]=0x13091615;
	data_array[6]=0x01001A15;
	data_array[7]=0x3F373105;
	data_array[8]=0x0A073F1A;
	data_array[9]=0x1411180C;
	data_array[10]=0x09161517;
	data_array[11]=0x00181413;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000000BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x002C3902;
	data_array[1]=0x080001C1;
	data_array[2]=0x271F1710;
	data_array[3]=0x473D362F;
	data_array[4]=0x675F574F;
	data_array[5]=0x867F776F;
	data_array[6]=0xA69E968F;
	data_array[7]=0xC5BEB7AE;
	data_array[8]=0xE7DFD7CF;
	data_array[9]=0x12FFF7EF;
	data_array[10]=0x2215C461;
	data_array[11]=0xC00935B8;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000001BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x002B3902;
	data_array[1]=0x100800C1;
	data_array[2]=0x2F271F17;
	data_array[3]=0x4F473E36;
	data_array[4]=0x6F675F57;
	data_array[5]=0x8F877F77;
	data_array[6]=0xAFA79F97;
	data_array[7]=0xCFC6BEB7;
	data_array[8]=0xEFE7DFD7;
	data_array[9]=0x6112FFF7;
	data_array[10]=0x2B447F18;
	data_array[11]=0x00C0EE9B;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000002BD;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x002B3902;
	data_array[1]=0x110900C1;
	data_array[2]=0x31292119;
	data_array[3]=0x524A4139;
	data_array[4]=0x736B625A;
	data_array[5]=0x938A827B;
	data_array[6]=0xB4ABA39B;
	data_array[7]=0xD3CAC2BB;
	data_array[8]=0xF2EBE3DB;
	data_array[9]=0xE515FFF8;
	data_array[10]=0xE8AFDA99;
	data_array[11]=0x00C00F98;
	dsi_set_cmdq(data_array, 12, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x000009CC;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00033902;
	data_array[1]=0x001430C0;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00053902;
	data_array[1]=0x40C000C7;
	data_array[2]=0x000000C0;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]=0x00083902;
	data_array[1]=0x14001FC9;
	data_array[2]=0x001E811E;
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x00000055;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(5);

	data_array[0]=0x00023902;
	data_array[1]=0x0000005E;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x000A3902;
	data_array[1]=0x2F3040CA;
	data_array[2]=0x23262D2E;
	data_array[3]=0x00002021;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0]=0x00233902;
	data_array[1]=0x000000CE;
	data_array[2]=0x10101010;
	data_array[3]=0x20202020;
	data_array[4]=0x30202020;
	data_array[5]=0x30303030;
	data_array[6]=0x30303030;
	data_array[7]=0x30303030;
	data_array[8]=0x30303030;
	data_array[9]=0x00003030;
	dsi_set_cmdq(data_array, 10, 1);

	data_array[0]= 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0]= 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	data_array[0]=0x00023902;
	data_array[1]=0x0000FF51;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0]=0x00023902;
	data_array[1]=0x00002453;
	dsi_set_cmdq(data_array, 2, 1);

    //lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);

#ifdef BUILD_LK
	printf("%s,uboot:hx8394d_lcm_init exit.\n", __func__);
#else
	printk("%s,kernel:hx8394d_lcm_init exit.\n", __func__);
#endif
}


static void lcm_suspend(void)
{
    //push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	unsigned int data_array[16];
	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(40);

    //reset low
    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    //lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(5);
#if 0
    //disable VSP & VSN
	lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
	lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
    mdelay(5);
#endif
#ifdef BUILD_LK
	printf("%s,uboot:hx8394d_lcm_suspend exit.\n", __func__);
#else
	printk("%s,kernel:hx8394d_lcm_suspend exit.\n", __func__);
#endif

}
static void lcm_resume(void)
{
    lcm_resume_init();
#ifdef BUILD_LK
	printf("%s,uboot:hx8394d_lcm_resume exit.\n", __func__);
#else
	printk("%s,kernel:hx8394d_lcm_resume exit.\n", __func__);
#endif

}

static unsigned int lcm_compare_id(void)
{
	unsigned char low_read0 = 0;
    unsigned char  high_read0 = 0;
	unsigned int ret = 0;
	unsigned char  lcd_id0 = 0;

    ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_mode fail\n");
	#else
	    printk("ID0 mt_set_gpio_mode fail\n");
	#endif
    }

    ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_dir fail\n");
	#else
		printk("ID0 mt_set_gpio_dir fail\n");
	#endif
    }

    ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_pull_enable fail\n");
	#else
	    printk("ID0 mt_set_gpio_pull_enable fail\n");
	#endif
    }

	//pull down ID0 PIN
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_pull_select->Down fail\n");
	#else
		printk("ID0 mt_set_gpio_pull_select->Down fail\n");
	#endif
    }

	MDELAY(100);
    //get ID0  status
    low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);

    //pull up ID0 PIN
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
    if(0 != ret)
    {
	#ifdef BUILD_LK
        printf("ID0 mt_set_gpio_pull_select->Up fail\n");
	#else
		printk("ID0 mt_set_gpio_pull_select->Up fail\n");
	#endif
    }

	//delay 100ms , for charging capacitance
    MDELAY(100);
    //get ID0 ID1 status
    high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
#ifdef BUILD_LK
	printf("low_read0 =%d,high_read0=%d\n",low_read0,high_read0);
#else
    printk("low_read0 =%d,high_read0=%d\n",low_read0,high_read0);
#endif

	if( low_read0 != high_read0 )
    {
        /*float status , pull down ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
        #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Down fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Down fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
    }
    else if((LCD_HW_ID_STATUS_LOW == low_read0) && (LCD_HW_ID_STATUS_LOW == high_read0))
    {
        /*low status , pull down ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
            #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Down fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Down fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_LOW;
    }
    else if((LCD_HW_ID_STATUS_HIGH == low_read0) && (LCD_HW_ID_STATUS_HIGH == high_read0))
    {
        /*high status , pull up ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
        if(0 != ret)
        {
        #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Up fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Up fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_HIGH;
    }
    else
    {
     #ifdef BUILD_LK
        printf(" Read LCD_id0 error\n");
	 #else
		printk(" Read LCD_id0 error\n");
	 #endif
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DISABLE);
        if(0 != ret)
        {
        #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Disbale fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Disbale fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_ERROR;
    }
#ifdef BUILD_LK
	printf("which_lcd_module_triple,lcd_id0:%d\n",lcd_id0);
#else
    printk("which_lcd_module_triple,lcd_id0:%d\n",lcd_id0);
#endif

    if(lcd_id0 == 1)
		return 1;
	else
	 	return 0;
}

static void lcm_setbacklight(unsigned int level)
{
#ifdef BUILD_LK
	printf("%s, hx8394d backlight: level = %d\n", __func__, level);
#else
	printk("%s, hx8394d backlight: level = %d\n", __func__, level);
#endif
	// Refresh value of backlight level.

	unsigned int cmd = 0x51;
	unsigned int count = 1;
	unsigned int value = level;
	dsi_set_cmdq_V2(cmd,count,&value,1);
}

static void lcm_set_lcm_cmd(void* handle,unsigned int *lcm_cmd,unsigned char *lcm_count,unsigned char *lcm_value)
{
#ifdef BUILD_LK
	printf("%s, lcm\n", __func__);
#else
     printk("%s, lcm\n", __func__);
#endif

	unsigned int cmd = lcm_cmd[0];
	unsigned char count = lcm_count[0];
	unsigned char *ppara =  lcm_value;

	dsi_set_cmdq_V2(cmd, count, ppara, 1);
}


LCM_DRIVER hx8394d_hd720_dsi_vdo_truly_drv =
{
    .name           = "hx8394d_hd720_dsi_vdo_truly",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,/*shunyu init fun.*/
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
    .set_backlight  = lcm_setbacklight,
	.set_lcm_cmd    = lcm_set_lcm_cmd,
};

