#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>   
#include <platform/mt_pmic.h>
#include <printf.h>
#include "bq24158.h"

#if !defined(CONFIG_POWER_EXT)
#include <platform/upmu_common.h>
#endif


/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define bq24158_SLAVE_ADDR_WRITE   0xD4
#define bq24158_SLAVE_ADDR_Read    0xD5

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
kal_uint8 bq24158_reg[bq24158_REG_NUM] = {0};

/**********************************************************
  *
  *   [I2C Function For Read/Write bq24158] 
  *
  *********************************************************/
static struct mt_i2c_t bq24158_i2c;

int bq24158_read_byte(kal_uint8 cmd, kal_uint8 *returnData)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint16 len;
    *returnData = cmd;

    bq24158_i2c.id = I2C1;
    /* Since i2c will left shift 1 bit, we need to set BQ24261 I2C address to >>1 */
    bq24158_i2c.addr = (bq24158_SLAVE_ADDR_WRITE >> 1);
    bq24158_i2c.mode = HS_MODE;
    bq24158_i2c.speed = 3400;
    len = 1;

    ret_code = i2c_write_read(&bq24158_i2c, returnData, len, len);

    if(I2C_OK != ret_code)
        dprintf(CRITICAL, "%s: i2c_read: ret_code: %d\n", __func__, ret_code);

    return 0;
}

int bq24158_write_byte(kal_uint8 cmd, kal_uint8 writeData)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= cmd;
    write_data[1] = writeData;

    bq24158_i2c.id = I2C1;
    /* Since i2c will left shift 1 bit, we need to set BQ24261 I2C address to >>1 */
    bq24158_i2c.addr = (bq24158_SLAVE_ADDR_WRITE >> 1);
    bq24158_i2c.mode = HS_MODE;
    bq24158_i2c.speed = 3400;
    len = 2;
	//dprintf(CRITICAL, "%s: i2c_write: [0x%x] write 0x%x\n", __func__, write_data[0], write_data[1]);
    ret_code = i2c_write(&bq24158_i2c, write_data, len);

    if(I2C_OK != ret_code)
        dprintf(CRITICAL, "%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return 1;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 bq24158_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 bq24158_reg = 0;
    int ret = 0;

   dprintf(INFO,"--------------------------------------------------\n");

    ret = bq24158_read_byte(RegNum, &bq24158_reg);

	dprintf(INFO,"[bq24158_read_interface] Reg[%x]=0x%x\n", RegNum, bq24158_reg);
	
    bq24158_reg &= (MASK << SHIFT);
    *val = (bq24158_reg >> SHIFT);
	
	dprintf(INFO,"[bq24158_read_interface] val=0x%x\n", *val);
	
    return ret;
}

kal_uint32 bq24158_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 bq24158_reg = 0;
    int ret = 0;

    dprintf(INFO,"--------------------------------------------------\n");

    ret = bq24158_read_byte(RegNum, &bq24158_reg);
    dprintf(INFO,"[bq24158_config_interface] Reg[%x]=0x%x\n", RegNum, bq24158_reg);
    
    bq24158_reg &= ~(MASK << SHIFT);
    bq24158_reg |= (val << SHIFT);

	if(RegNum == bq24158_CON4 && val == 1 && MASK ==CON4_RESET_MASK && SHIFT == CON4_RESET_SHIFT)
	{
		// RESET bit
	}
	else if(RegNum == bq24158_CON4)
	{
		bq24158_reg &= ~0x80;	//RESET bit read returs 1, so clear it
	}
	 

    ret = bq24158_write_byte(RegNum, bq24158_reg);
    dprintf(INFO,"[bq24158_config_interface] write Reg[%x]=0x%x\n", RegNum, bq24158_reg);

    // Check
    //bq24158_read_byte(RegNum, &bq24158_reg);
    //printk("[bq24158_config_interface] Check Reg[%x]=0x%x\n", RegNum, bq24158_reg);

    return ret;
}

//write one register directly
kal_uint32 bq24158_reg_config_interface (kal_uint8 RegNum, kal_uint8 val)
{   
    int ret = 0;
    ret = bq24158_write_byte(RegNum, val);

    return ret;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//CON0----------------------------------------------------

void bq24158_set_tmr_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_TMR_RST_MASK),
                                    (kal_uint8)(CON0_TMR_RST_SHIFT)
                                    );
}

kal_uint32 bq24158_get_otg_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_OTG_MASK),
                                    (kal_uint8)(CON0_OTG_SHIFT)
                                    );
    return val;
}

void bq24158_set_en_stat(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_EN_STAT_MASK),
                                    (kal_uint8)(CON0_EN_STAT_SHIFT)
                                    );
}

kal_uint32 bq24158_get_chip_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_STAT_MASK),
                                    (kal_uint8)(CON0_STAT_SHIFT)
                                    );
    return val;
}

kal_uint32 bq24158_get_boost_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_BOOST_MASK),
                                    (kal_uint8)(CON0_BOOST_SHIFT)
                                    );
    return val;
}

kal_uint32 bq24158_get_fault_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_FAULT_MASK),
                                    (kal_uint8)(CON0_FAULT_SHIFT)
                                    );
    return val;
}

//CON1----------------------------------------------------

void bq24158_set_input_charging_current(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LIN_LIMIT_MASK),
                                    (kal_uint8)(CON1_LIN_LIMIT_SHIFT)
                                    );
}

void bq24158_set_v_low(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LOW_V_MASK),
                                    (kal_uint8)(CON1_LOW_V_SHIFT)
                                    );
}

void bq24158_set_te(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_TE_MASK),
                                    (kal_uint8)(CON1_TE_SHIFT)
                                    );
}

void bq24158_set_ce(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_CE_MASK),
                                    (kal_uint8)(CON1_CE_SHIFT)
                                    );
}

void bq24158_set_hz_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_HZ_MODE_MASK),
                                    (kal_uint8)(CON1_HZ_MODE_SHIFT)
                                    );
}

void bq24158_set_opa_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_OPA_MODE_MASK),
                                    (kal_uint8)(CON1_OPA_MODE_SHIFT)
                                    );
}

//CON2----------------------------------------------------

void bq24158_set_oreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OREG_MASK),
                                    (kal_uint8)(CON2_OREG_SHIFT)
                                    );
}

void bq24158_set_otg_pl(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_PL_MASK),
                                    (kal_uint8)(CON2_OTG_PL_SHIFT)
                                    );
}

void bq24158_set_otg_en(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_EN_MASK),
                                    (kal_uint8)(CON2_OTG_EN_SHIFT)
                                    );
}

//CON3----------------------------------------------------

kal_uint32 bq24158_get_vender_code(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_VENDER_CODE_MASK),
                                    (kal_uint8)(CON3_VENDER_CODE_SHIFT)
                                    );
    return val;
}

kal_uint32 bq24158_get_pn(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_PIN_MASK),
                                    (kal_uint8)(CON3_PIN_SHIFT)
                                    );
    return val;
}

kal_uint32 bq24158_get_revision(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_REVISION_MASK),
                                    (kal_uint8)(CON3_REVISION_SHIFT)
                                    );
    return val;
}

//CON4----------------------------------------------------

void bq24158_set_reset(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_RESET_MASK),
                                    (kal_uint8)(CON4_RESET_SHIFT)
                                    );
}

void bq24158_set_iocharge(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_CHR_MASK),
                                    (kal_uint8)(CON4_I_CHR_SHIFT)
                                    );
}

void bq24158_set_iterm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_TERM_MASK),
                                    (kal_uint8)(CON4_I_TERM_SHIFT)
                                    );
}

//CON5----------------------------------------------------

void bq24158_set_dis_vreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_DIS_VREG_MASK),
                                    (kal_uint8)(CON5_DIS_VREG_SHIFT)
                                    );
}

void bq24158_set_io_level(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_IO_LEVEL_MASK),
                                    (kal_uint8)(CON5_IO_LEVEL_SHIFT)
                                    );
}

kal_uint32 bq24158_get_sp_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_SP_STATUS_MASK),
                                    (kal_uint8)(CON5_SP_STATUS_SHIFT)
                                    );
    return val;
}

kal_uint32 bq24158_get_en_level(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_EN_LEVEL_MASK),
                                    (kal_uint8)(CON5_EN_LEVEL_SHIFT)
                                    );
    return val;
}

void bq24158_set_vsp(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_VSP_MASK),
                                    (kal_uint8)(CON5_VSP_SHIFT)
                                    );
}

//CON6----------------------------------------------------

void bq24158_set_i_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_ISAFE_MASK),
                                    (kal_uint8)(CON6_ISAFE_SHIFT)
                                    );
}

void bq24158_set_v_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_VSAFE_MASK),
                                    (kal_uint8)(CON6_VSAFE_SHIFT)
                                    );
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
void bq24158_dump_register(void)
{
    int i=0;
    dprintf(INFO, "[bq24158] ");
    for (i=0;i<bq24158_REG_NUM;i++)
    {
        bq24158_read_byte(i, &bq24158_reg[i]);
        dprintf(INFO,"[0x%x]=0x%x ", i, bq24158_reg[i]);        
    }
    dprintf(INFO, "\n");
}

int bq24158_dump_register_htc(char *buf, int size)
{
    int i=0;
	int len = 0;

    for (i=0;i<bq24158_REG_NUM;i++)
    {
        bq24158_read_byte(i, &bq24158_reg[i]);
        len += scnprintf(buf + len, size - len,
			"bq24158_REG[0x%x]: 0x%X;\n", i, bq24158_reg[i]);
    }

	return len;
}

void bq24158_charging(void)
{
	//if (upmu_is_chr_det() == KAL_FALSE)
	//	return;

	bq24158_reg_config_interface(0x06,0x77);
	bq24158_reg_config_interface(0x00,0xD0);
	bq24158_reg_config_interface(0x01,0xF8);
	bq24158_reg_config_interface(0x05,0x13);

	bq24158_reg_config_interface(0x04,0x08); //set Iocharger to 500mA
	bq24158_reg_config_interface(0x02,0x52);

	bq24158_set_ce(0);      //charger enable 
	bq24158_set_hz_mode(0);
	bq24158_set_opa_mode(0);
	bq24158_set_oreg(0x2B);  //4.36 V for Charger Vout Voreg float voltage

	bq24158_set_tmr_rst(0x1); //kick watchdog 

	bq24158_dump_register();  //dump register
}

#if 1
extern int g_enable_high_vbat_spec;
extern int g_pmic_cid;

void bq24158_hw_init(void)
{    
    if(g_enable_high_vbat_spec == 1)
    {
        if(g_pmic_cid == 0x1020)
        {
            printk("[bq24158_hw_init] (0x06,0x77) because 0x1020\n");
            bq24158_reg_config_interface(0x06,0x77); // set ISAFE
        }
        else
        {
            printk("[bq24158_hw_init] (0x06,0x77)\n");
            bq24158_reg_config_interface(0x06,0x77); // set ISAFE and HW CV point (4.34)
        }
    }
    else
    {
        printk("[bq24158_hw_init] (0x06,0x77) \n");
        bq24158_reg_config_interface(0x06,0x77); // set ISAFE
    }
}
#endif

/**********************************************************
  *
  *   [platform_driver API] 
  *
  *********************************************************/
kal_uint8 g_reg_value_bq24158=0;
static ssize_t show_bq24158_access(struct device *dev,struct device_attribute *attr, char *buf)
{
    dprintf(INFO,"[show_bq24158_access] 0x%x\n", g_reg_value_bq24158);
    return sprintf(buf, "%u\n", g_reg_value_bq24158);
}
static ssize_t store_bq24158_access(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int ret=0;
    char *pvalue = NULL;
    unsigned int reg_value = 0;
    unsigned int reg_address = 0;
    
    dprintf(INFO,"[store_bq24158_access] \n");
    
    if(buf != NULL && size != 0)
    {
        dprintf(INFO,"[store_bq24158_access] buf is %s and size is %d \n",buf,size);
        reg_address = simple_strtoul(buf,&pvalue,16);
        
        if(size > 3)
        {        
            reg_value = simple_strtoul((pvalue+1),NULL,16);        
            dprintf(INFO,"[store_bq24158_access] write bq24158 reg 0x%x with value 0x%x !\n",reg_address,reg_value);
            ret=bq24158_config_interface(reg_address, reg_value, 0xFF, 0x0);
        }
        else
        {    
            ret=bq24158_read_interface(reg_address, &g_reg_value_bq24158, 0xFF, 0x0);
            dprintf(INFO,"[store_bq24158_access] read bq24158 reg 0x%x with value 0x%x !\n",reg_address,g_reg_value_bq24158);
            dprintf(INFO,"[store_bq24158_access] Please use \"cat bq24158_access\" to get value\r\n");
        }        
    }    
    return size;
}

//zijian add for build error
 kal_uint32 bq24158_config_interface_liao (kal_uint8 RegNum, kal_uint8 val)
 {	 
	 int ret = 0;
	 
	 ret = bq24158_write_byte(RegNum, val);
 
	 return ret;
 }
