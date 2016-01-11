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
	else
	printk("*********hx8394d tps65132 write data sucess !!\n");
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
    params->dsi.vertical_backporch				= 14;
    params->dsi.vertical_frontporch				= 15;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active				= 36;
    params->dsi.horizontal_backporch				= 90;
    params->dsi.horizontal_frontporch				= 90;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 190; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;

    params->pwm_min = 7;
    params->pwm_default = 61;
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
		MDELAY(10);
		mt_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
		#ifdef BUILD_LK
		TPS65132_write_byte(0x00, 0x0E);
		TPS65132_write_byte(0x01, 0x0E);
		#else
		tps65132_write_bytes(0x00, 0x0E);
		tps65132_write_bytes(0x01, 0x0E);
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
	printf("%s,uboot:HTC hx8394d_lcm_init enter.\n", __func__);
#else
	printk("%s,kernel:HTC hx8394d_lcm_init enter.\n", __func__);
#endif
	 //enable VSP & VSN  ,here to set Offset voltage
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    //mdelay(50);

	TPS65132_enable(1);

	MDELAY(1);
	//reset high to low to high
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(2);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(5);
	lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(120);

	//lcm_id_pin_handle();
	// when phone initial , config output high, enable backlight drv chip
    // push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

	data_array[0] = 0x00043902;
	data_array[1] = 0x9483ffb9;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x00033902;
	data_array[1] = 0x008373ba;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x00103902;
	data_array[1] = 0x15556cb1;
	data_array[2] = 0xf1110411;
	data_array[3] = 0x2395e880;
	data_array[4] = 0x58d2c080;
	dsi_set_cmdq(data_array, 5, 1);
	//MDELAY(2);

	data_array[0] = 0x000c3902;
	data_array[1] = 0x106400b2;
	data_array[2] = 0x081c3207;
	data_array[3] = 0x004d1c08;
	dsi_set_cmdq(data_array, 4, 1);
	//MDELAY(2);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000007bc;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x000d3902;
	data_array[1] = 0x03ff00b4;
	data_array[2] = 0x035c035c;
	data_array[3] = 0x0170015c;
	data_array[4] = 0x00000070;
	dsi_set_cmdq(data_array, 5, 1);
	//MDELAY(2);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000055d2;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x001f3902;
	data_array[1] = 0x000600d3;
	data_array[2] = 0x00081a01;
	data_array[3] = 0x00071032;
	data_array[4] = 0x0f155407;
	data_array[5] = 0x12020405;
	data_array[6] = 0x33070510;
	data_array[7] = 0x370b0b33;
	data_array[8] = 0x00070710;
	dsi_set_cmdq(data_array, 9, 1);
	//MDELAY(2);

	data_array[0] = 0x002d3902;
	data_array[1] = 0x181919d5;
	data_array[2] = 0x1a1b1b18;
	data_array[3] = 0x0605041a;
	data_array[4] = 0x02010007;
	data_array[5] = 0x18212003;
	data_array[6] = 0x18232218;
	data_array[7] = 0x18181818;
	data_array[8] = 0x18181818;
	data_array[9] = 0x18181818;
	data_array[10] = 0x18181818;
	data_array[11] = 0x18181818;
	data_array[12] = 0x00000018;
	dsi_set_cmdq(data_array, 13, 1);
	//MDELAY(2);

	data_array[0] = 0x002d3902;
	data_array[1] = 0x191818d6;
	data_array[2] = 0x1a1b1b19;
	data_array[3] = 0x0102031a;
	data_array[4] = 0x05060700;
	data_array[5] = 0x18222304;
	data_array[6] = 0x18202118;
	data_array[7] = 0x18181818;
	data_array[8] = 0x18181818;
	data_array[9] = 0x18181818;
	data_array[10] = 0x18181818;
	data_array[11] = 0x18181818;
	data_array[12] = 0x00000018;
	dsi_set_cmdq(data_array, 13, 1);
	//MDELAY(2);

	data_array[0] = 0x002b3902;
	data_array[1] = 0x090300e0;
	data_array[2] = 0x1c3f2723;
	data_array[3] = 0x0b0a063a;
	data_array[4] = 0x12100d17;
	data_array[5] = 0x11071311;
	data_array[6] = 0x03001814;
	data_array[7] = 0x3f292309;
	data_array[8] = 0x0a063a1c;
	data_array[9] = 0x110e170b;
	data_array[10] = 0x07131112;
	data_array[11] = 0x00181312;
	dsi_set_cmdq(data_array, 12, 1);
	//MDELAY(2);

	data_array[0] = 0x00033902;
	data_array[1] = 0x001430c0;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x00053902;
	data_array[1] = 0x00c000c7;
	data_array[2] = 0x000000c0;
	dsi_set_cmdq(data_array, 3, 1);
	//MDELAY(2);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000009cc;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x00023902;
	data_array[1] = 0x000087df;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000ff51;
	dsi_set_cmdq(data_array, 2, 1);
	//MDELAY(2);

	data_array[0] = 0x00083902;
	data_array[1] = 0x14001fc9;
	data_array[2] = 0x001e811e;
	dsi_set_cmdq(data_array, 3, 1);
	//MDELAY(2);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00000055;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(5);

	data_array[0] = 0x00023902;
	data_array[1] = 0x0000005e;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x000a3902;
	data_array[1] = 0x2f3040ca;
	data_array[2] = 0x23262d2e;
	data_array[3] = 0x00002021;
	dsi_set_cmdq(data_array, 4, 1);

	data_array[0] = 0x00233902;
	data_array[1] = 0x000000ce;
	data_array[2] = 0x10101010;
	data_array[3] = 0x20202020;
	data_array[4] = 0x30202020;
	data_array[5] = 0x30303030;
	data_array[6] = 0x30303030;
	data_array[7] = 0x30303030;
	data_array[8] = 0x30303030;
	data_array[9] = 0x00003030;
	dsi_set_cmdq(data_array, 10, 1);

	data_array[0] = 0x00023902;
	data_array[1] = 0x00002453;
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00110500;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);

	data_array[0] = 0x00013902;
	data_array[1] = 0x00000029;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);

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
    lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	TPS65132_enable(0);
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
    lcm_init();
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


LCM_DRIVER hx8394d_hd720_dsi_vdo_dijing_drv =
{
    .name           = "hx8394d_hd720_dsi_vdo_dijing",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,/*shunyu init fun.*/
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
    .set_backlight  = lcm_setbacklight,

};

