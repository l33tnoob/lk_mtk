/* BEGIN PN:DTS2013053103858 , Added by d00238048, 2013.05.31*/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#include <platform/mt_i2c.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    //#include <linux/delay.h>
#include <mach/mt_gpio.h>
#include <linux/i2c.h>
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
#define FRAME_WIDTH										(1080)
#define FRAME_HEIGHT										(1920)


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

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/jiffies.h>
#include <linux/uaccess.h>
//#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
/*****************************************************************************
 * Define
 *****************************************************************************/
#define TPS_I2C_BUSNUM  1  //I2C_I2C_LCD_BIAS_CHANNEL//for I2C channel 0
#define I2C_ID_NAME "tps65132"
#define TPS_ADDR 0x3E
/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
static struct i2c_board_info __initdata tps65132_board_info = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
static struct i2c_client *tps65132_i2c_client = NULL;


/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);
/*****************************************************************************
 * Data Structure
 *****************************************************************************/

 struct tps65132_dev	{
	struct i2c_client	*client;

};

static const struct i2c_device_id tps65132_id[] = {
	{ I2C_ID_NAME, 0 },
	{ }
};

//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
//static struct i2c_client_address_data addr_data = { .forces = forces,};
//#endif
static struct i2c_driver tps65132_iic_driver = {
	.id_table	= tps65132_id,
	.probe		= tps65132_probe,
	.remove		= tps65132_remove,
	//.detect		= mt6605_detect,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "tps65132",
	},

};
/*****************************************************************************
 * Extern Area
 *****************************************************************************/

/*****************************************************************************
 * Function
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk( "*********hx8394d tps65132_iic_probe\n");
	printk("*********hx8394d TPS: info==>name=%s addr=0x%x\n",client->name,client->addr);
	tps65132_i2c_client  = client;
	return 0;
}


static int tps65132_remove(struct i2c_client *client)
{
  printk( "*********hx8394d tps65132_remove\n");
  tps65132_i2c_client = NULL;
   i2c_unregister_device(client);
  return 0;
}


 static int tps65132_write_bytes(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = tps65132_i2c_client;
	char write_data[2]={0};
	write_data[0]= addr;
	write_data[1] = value;
    ret=i2c_master_send(client, write_data, 2);
	if(ret<0)
	printk("*********hx8394d tps65132 write data fail !!\n");
	return ret ;
}



/*
 * module load/unload record keeping
 */

static int __init tps65132_iic_init(void)
{

   printk( "*********hx8394d tps65132_iic_init\n");
   i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
   printk( "*********hx8394d tps65132_iic_init2\n");
   i2c_add_driver(&tps65132_iic_driver);
   printk( "*********hx8394d tps65132_iic_init success\n");
   return 0;
}

static void __exit tps65132_iic_exit(void)
{
  printk( "*********hx8394d tps65132_iic_exit\n");
  i2c_del_driver(&tps65132_iic_driver);
}


module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
MODULE_LICENSE("GPL");
#endif


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

    params->dsi.mode   = BURST_VDO_MODE;//SYNC_PULSE_VDO_MODE;

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
    params->dsi.vertical_backporch				= 4;
    params->dsi.vertical_frontporch				= 4;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active				= 20;
    params->dsi.horizontal_backporch				= 40;
    params->dsi.horizontal_frontporch				= 100;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 450; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;

    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;

    params->dsi.lcm_esd_check_table[0].cmd			= 0x0A;
    params->dsi.lcm_esd_check_table[0].count		= 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

    params->dsi.lcm_esd_check_table[1].cmd          = 0xAB;
    params->dsi.lcm_esd_check_table[1].count        = 2;
    params->dsi.lcm_esd_check_table[1].para_list[0] = 0x00;
    params->dsi.lcm_esd_check_table[1].para_list[1] = 0x00;

    params->pwm_min = 6;
    params->pwm_default = 81;
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


#ifdef BUILD_LK
static struct mt_i2c_t tps65132_i2c;
#define TPS65132_SLAVE_ADDR 0x3e
/**********************************************************
  *
  *   [I2C Function For Read/Write hx8394d]
  *
  *********************************************************/
static kal_uint32 TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    tps65132_i2c.id = 1;
    /* Since i2c will left shift 1 bit, we need to set hx8394d I2C address to >>1 */
    tps65132_i2c.addr = (TPS65132_SLAVE_ADDR);
    tps65132_i2c.mode = ST_MODE;
    tps65132_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&tps65132_i2c, write_data, len);
    printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}
#endif


static void TPS65132_enable(char en)
{

	mt_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_LCD_BIAS_ENP_PIN_M_GPIO);
	mt_set_gpio_mode(GPIO_LCD_BIAS_ENN_PIN, GPIO_LCD_BIAS_ENN_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENN_PIN, GPIO_DIR_OUT);

	if (en)
	{

		mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
		MDELAY(5);
		mt_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
		MDELAY(20);
		#ifdef BUILD_LK
		TPS65132_write_byte(0x00, 0x0E);
		MDELAY(5);
		TPS65132_write_byte(0x01, 0x0E);
		MDELAY(5);
		#else
		tps65132_write_bytes(0x00, 0x0E);
		MDELAY(5);
		tps65132_write_bytes(0x01, 0x0E);
		MDELAY(5);
		#endif
	}
	else
	{

		mt_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
		MDELAY(10);
		mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
		MDELAY(10);
	}

}




static void lcm_init(void)
{
  unsigned int data_array[35];
#ifdef BUILD_LK
	printf("%s,uboot:nt35596_lcm_init enter.\n", __func__);
#else
	printk("%s,kernel:nt35596_lcm_init enter.\n", __func__);
#endif
	 //enable VSP & VSN  ,here to set Offset voltage
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    //mdelay(50);


	TPS65132_enable(1);
	MDELAY(1);

	//reset high to low to high
    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(20);


	//initial code
	data_array[0] = 0xeeff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x40181500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	data_array[0] = 0x00181500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0xeeff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x317c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01ff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01001500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x55011500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x40021500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x50051500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x1E061500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x28071500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0c081500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x870b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x870c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb00e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb30f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x4a141500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x12151500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x12161500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00181500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x77191500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x551a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x131b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x82581500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02591500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x025a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x025b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x825c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x825d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x025e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x025f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x31721500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x05ff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01001500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0b011500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0c021500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x09031500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0a041500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00051500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0f061500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x10071500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x1a081500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00091500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x000a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x000b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x000c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x130d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x150e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x170f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01101500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0b111500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0c121500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x09131500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0a141500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00151500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0f161500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x10171500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x1a181500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00191500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x001c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x131d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x151e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x171f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00201500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01211500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00221500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x40231500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x40241500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6d251500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd8291500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2a2a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x002b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x89b61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x14b71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x05b81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x044b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x114c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x104d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x014e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x014f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x10501500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00511500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00521500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x08531500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01541500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6d551500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x445b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x005c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x745f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x75601500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xff631500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00641500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x04671500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x04681500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x006c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x807a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x917b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd87c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x607d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x157f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00801500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00831500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x08931500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0a941500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x008a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0f9b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01ff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00751500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00761500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00771500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x54781500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00791500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7e7a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x007b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x9a7c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x007d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb07e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x007f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xc1801500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00811500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd1821500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00831500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdf841500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00851500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xec861500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01871500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x17881500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01891500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x398a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x018b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6e8c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x018d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x998e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x018f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdd901500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02911500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x14921500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02931500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x15941500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02951500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x48961500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02971500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7f981500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02991500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa29a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x029b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd29c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x029d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xf39e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x039f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x20a01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2fa31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x3fa51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x52a71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x68aa1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03ab1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x82ac1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03ad1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa5ae1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03af1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd9b01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03b11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xffb21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x54b61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7eb81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x9aba1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00bb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb0bc1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00bd1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xc1be1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00bf1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd1c01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00c11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdfc21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00c31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xecc41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x17c61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x39c81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6eca1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01cb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x99cc1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01cd1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xddce1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02cf1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x14d01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x15d21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x48d41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7fd61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa2d81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd2da1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02db1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xf3dc1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03dd1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x20de1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03df1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2fe01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x3fe21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x52e41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x68e61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x82e81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa5ea1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03eb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd9ec1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03ed1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xffee1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00ef1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00f01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00f11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x54f21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00f31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7ef41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00f51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x9af61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00f71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb0f81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00f91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xc1fa1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02ff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00001500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd1011500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00021500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdf031500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00041500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xec051500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01061500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x17071500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01081500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x39091500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x010a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6e0b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x010c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x990d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x010e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdd0f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02101500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x14111500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02121500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x15131500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02141500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x48151500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02161500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7f171500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02181500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa2191500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x021a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd21b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x021c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xf31d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x031e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x201f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03201500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2f211500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03221500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x3f231500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03241500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x52251500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03261500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x68271500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03281500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x82291500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x032a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa52b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x032d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd92f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03301500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xff311500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00321500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00331500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00341500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x54351500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00361500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7e371500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00381500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x9a391500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x003a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb03b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x003d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xc13f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00401500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd1411500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00421500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdf431500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00441500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xec451500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01461500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x17471500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01481500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x39491500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x014a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6e4b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x014c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x994d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x014e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdd4f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02501500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x14511500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02521500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x15531500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02541500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x48551500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02561500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7f581500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02591500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa25a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x025b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd25c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x025d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xf35e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x035f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x20601500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03611500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2f621500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03631500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x3f641500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03651500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x52661500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03671500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x68681500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03691500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x826a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x036b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa56c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x036d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd96e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x036f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xff701500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00711500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00721500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00731500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x54741500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00751500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7e761500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00771500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x9a781500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00791500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb07a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x007b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xc17c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x007d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd17e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x007f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdf801500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00811500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xec821500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01831500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x17841500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01851500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x39861500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01871500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6e881500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01891500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x998a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x018b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdd8c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x028d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x148e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x028f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x15901500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02911500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x48921500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02931500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7f941500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02951500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa2961500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02971500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd2981500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02991500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xf39a1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x039b1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x209c1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x039d1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2f9e1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x039f1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x3fa01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x52a31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x68a51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x82a71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03a91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa5aa1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03ab1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd9ac1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03ad1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xffae1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00af1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x54b21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7eb41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x9ab61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xb0b81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00b91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xc1ba1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00bb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd1bc1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00bd1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xdfbe1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00bf1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xecc01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x17c21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x39c41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x6ec61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x99c81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01c91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xddca1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02cb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x14cc1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02cd1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x15ce1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02cf1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x48d01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x7fd21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa2d41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd2d61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x02d71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xf3d81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03d91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x20da1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03db1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x2fdc1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03dd1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x3fde1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03df1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x52e01500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e11500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x68e21500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x82e41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e51500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xa5e61500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xd9e81500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x03e91500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xffea1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x05ff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x80e71500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x01fb1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00ec1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00ff1500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x06d31500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x04d41500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00351500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(140);

	data_array[0] = 0x00290500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(15);

	data_array[0] = 0x24531500;
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0xff511500;
	dsi_set_cmdq(data_array, 1, 1);

    //lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);

#ifdef BUILD_LK
	printf("%s,uboot:nt35596_lcm_init exit.\n", __func__);
#else
	printk("%s,kernel:nt35596_lcm_init exit.\n", __func__);
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
    TPS65132_enable(0);

    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);

    //lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(20);
#if 0
    //disable VSP & VSN
	lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
	lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
    mdelay(5);
#endif
#ifdef BUILD_LK
	printf("%s,uboot:nt35596_lcm_suspend exit.\n", __func__);
#else
	printk("%s,kernel:nt35596_lcm_suspend exit.\n", __func__);
#endif

}
static void lcm_resume(void)
{
    lcm_init();
#ifdef BUILD_LK
	printf("%s,uboot:nt35596_lcm_resume exit.\n", __func__);
#else
	printk("%s,kernel:nt35596_lcm_resume exit.\n", __func__);
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

    if(lcd_id0 == 2)
		return 1;
	else
	 	return 0;
}

static void lcm_setbacklight(unsigned int level)
{
#ifdef BUILD_LK
	printf("%s, nt35596 backlight: level = %d\n", __func__, level);
#else
	printk("%s, nt35596 backlight: level = %d\n", __func__, level);
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


LCM_DRIVER nt35596_fhd_dsi_vdo_truly_tps65132_drv =
{
    .name           = "nt35596_fhd_dsi_vdo_truly_tps65132",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,/*shunyu init fun.*/
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
    .set_backlight  = lcm_setbacklight,
	.set_lcm_cmd    = lcm_set_lcm_cmd,
};

