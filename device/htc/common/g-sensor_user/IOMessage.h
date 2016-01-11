/*************************************************************************
 *
 *  $Workfile: AKDBMessage.h $
 *
 *  $Author: Weirx $
 *  $Date: 07/12/06 10:26 $
 *	$Version: 3 $
 *
 *  Revision: 10 based
 *
 *  Developped by AsahiKASEI EMD Corporation
 *  Provided by AsahiKASEI Microsystems
 *  Copyright (C) 2005 AsahiKASEI EMD Corporation, All rights reserved
 *
 *************************************************************************/
#pragma once
#include "GSR.h"
#include "sensor_type.h"

/*========================= Constant definition =============================*/
#define AKDBMES_WRITE_REGISTER		0x01
#define AKDBMES_READ_REGISTER		0x02
#define AKDBMES_RESET			0x03
#define AKDBMES_VERSION			0x04
#define AKDBMES_SET_MODE		0x05
#define AKDBMES_GETDATA			0x06
#define AKDBMES_SET_VECTOR		0x07
#define AKDBMES_GET_VECTOR		0x08
#define AKDBMES_SET_YPR			0x09
#define AKDBMES_GET_OPEN_STATUS		0x0A
#define AKDBMES_GET_CLOSE_STATUS	0x0B
#define AKDBMES_GET_DELAY		0x0C
#define AKDBMES_GET_MATRIX		0x0E

/* For G-sensor */
#define BMA_INT_STATUS			0x11
#define BMA_WRITE_REGISTER		0x12
#define BMA_READ_REGISTER		0x13
#define BMA_READ_ACCELERATION	0x14
#define BMA_SET_MODE			0x15
#define BMA_GET_CHIP_LAYOUT		0x16
#define BMA_SET_CALI_MODE		0x17
#define BMA_READ_CALI_VALUE		0x18
#define BMA_SET_UPDATE_USER_CALI_DATA	0x19

/*For manual calibrati */
#define AKDBMES_SET_MCAL_SIGNAL		0x21
#define AKDBMES_GET_MCAL_SIGNAL		0x22
#define	AKDBMES_GET_MCAL_STATUS		0x23
#define AKDBMES_MCAL_DONE		0x24

/*========================= Prototype of Function ===========================*/
/* Send message to DeviceDriver and receive ACK from it. */
int16 SendIOMessage(int hDevice, const int16 message, const int16 hParam,
	int16 *lParam);
