#ifndef BMA_GSR_H
#define BMA_GSR_H


#include <linux/ioctl.h>

#define BMAIO				0xA1

/* BMA150 register address */
#define CHIP_ID_REG			0x00
#define VERSION_REG			0x01
#define X_AXIS_LSB_REG		0x02
#define X_AXIS_MSB_REG		0x03
#define Y_AXIS_LSB_REG		0x04
#define Y_AXIS_MSB_REG		0x05
#define Z_AXIS_LSB_REG		0x06
#define Z_AXIS_MSB_REG		0x07
#define TEMP_RD_REG			0x08
#define SMB150_STATUS_REG	0x09
#define SMB150_CTRL_REG		0x0a
#define SMB150_CONF1_REG	0x0b
#define LG_THRESHOLD_REG	0x0c
#define LG_DURATION_REG		0x0d
#define HG_THRESHOLD_REG	0x0e
#define HG_DURATION_REG		0x0f
#define MOTION_THRS_REG		0x10
#define HYSTERESIS_REG		0x11
#define CUSTOMER1_REG		0x12
#define CUSTOMER2_REG		0x13
#define RANGE_BWIDTH_REG	0x14
#define SMB150_CONF2_REG	0x15

#define OFFS_GAIN_X_REG		0x16
#define OFFS_GAIN_Y_REG		0x17
#define OFFS_GAIN_Z_REG		0x18
#define OFFS_GAIN_T_REG		0x19
#define OFFSET_X_REG		0x1a
#define OFFSET_Y_REG		0x1b
#define OFFSET_Z_REG		0x1c
#define OFFSET_T_REG		0x1d


#define BMA_IOCTL_INIT                  _IO(BMAIO, 0x31)
#define BMA_IOCTL_WRITE                 _IOW(BMAIO, 0x32, char[5])
#define BMA_IOCTL_READ                  _IOWR(BMAIO, 0x33, char[5])
#define BMA_IOCTL_READ_ACCELERATION	_IOWR(BMAIO, 0x34, short[7])
#define BMA_IOCTL_SET_MODE		_IOW(BMAIO, 0x35, short)
#define BMA_IOCTL_GET_CHIP_LAYOUT	_IOR(BMAIO, 0x37, short)
#define BMA_IOCTL_GET_CALI_MODE		_IOR(BMAIO, 0x38, short)
#define BMA_IOCTL_SET_CALI_MODE		_IOW(BMAIO, 0x39, short)
#define BMA_IOCTL_READ_CALI_VALUE       _IOR(BMAIO, 0x3a, char[3])
#define BMA_IOCTL_SET_UPDATE_USER_CALI_DATA	_IOW(BMAIO, 0x3d, short)

/* mode settings */
#define BMA_MODE_NORMAL   	0
#define BMA_MODE_SLEEP       	1
#define BMA_MODE_WAKE_UP     	2

#endif
