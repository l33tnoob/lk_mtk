#ifndef __TCA6418_H__
#define __TCA6418_H__

#define CAM_I2C_ADDR	0
#define CAM2_ID		1
#define MAIN_CAM_ID	2
#define FRONT_CAM_ID	3
#define LCD_ID1		4
#define LCD_ID0		5
#define FLASH_EN	6
#define TORCH_FLASH	7
#define CIR_3V3_EN	8
#define CIR_LS_EN	9
#define CIR_RST		10
#define CAM2_A2V9_EN	11
#define CAM2_D1V2_EN	12
#define CAM2_RST	13

enum {
	IOEXP_INPUT,
	IOEXP_OUTPUT,
};

enum {
	IOEXP_PULLDOWN,
	IOEXP_NOPULL,
};

/* TCA6418 set GPIO funcion */
int ioexp_gpio_set_value(uint8_t gpio, uint8_t value);
int ioexp_gpio_get_value(uint8_t gpio);
int ioexp_i2c_probe(void);
//---------------------//

#endif //__TCA6418_H__
