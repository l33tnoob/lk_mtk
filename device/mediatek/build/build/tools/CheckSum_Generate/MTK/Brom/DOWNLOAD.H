/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2001
*
*******************************************************************************/

/*******************************************************************************
 * Filename:
 * ---------
 *	   download.h
 *
 * Project:
 * --------
 *   MTK6208 DOWNLOAD AGENT
 *
 * Description:
 * ------------
 *   This file is intends for download agent specific definition
 *
 * Author:
 * -------
 *	Jensen Hu
 
 *******************************************************************************/
 
#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#include "mtk_status.h"

/* FPGA Support Begin, 2012/09/12 {*/
#define FPGA_SUPPORT 0
/* FPGA Support End, 2012/09/12 }*/

/* DA Version */
#define DA_MAJOR_VER			0x03
#define DA_MINOR_VER			0x02

#define Compat_Old_Load         1

/* Max Number of Load Sections */

#if defined(DA_INIT) || defined(BB_MT6573) || defined(BB_MT6575_E1) || defined(BB_MT6575) || defined(BB_MT6577) || defined(BB_MT6572) || defined(BB_MT6589) || defined(BB_MT6582) || defined(BB_MT8135) || defined(BB_MT6592) || defined(BB_MT6571) || defined(BB_MT6595)	|| defined(BB_MT8127) || defined(BB_MT2601) || defined(BB_MT8173) || defined(BB_MT6799)
#define MAX_LOAD_SECTIONS		40
#else
#define MAX_LOAD_SECTIONS		128
#endif
//DRAM packet length is 1M ,SRAM packet length is 8K
#define  DRAM_PACKET_SIZE 1048576
#define  SRAM_PACKET_SIZE 8192

// Max Die Number of MCP 
#define MAX_DIE_IN_MCP			2

/* IMEI/PID/SWV info max lenth */
#define IMEI_MAX_LENTH			(16)
#define PID_MAX_LENTH			(64)
#define SWV_MAX_LENTH			(128)

/* RETURN VALUE */
#define SOC_OK					0xC1
#define SOC_FAIL				0xCF
#define SYNC_CHAR				0xC0
#define CONT_CHAR				0x69
#define STOP_CHAR				0x96
#define ACK						0x5A
#define NACK					0xA5
#define UNKNOWN_CMD				0xBB

/* COMMANDS */
#define DA_DOWNLOAD_BLOADER_CMD		0x51
#define DA_NAND_BMT_REMARK_CMD		0x52

#define DA_SDMMC_SWITCH_PART_CMD    0x60
#define DA_SDMMC_WRITE_IMAGE_CMD    0x61
#define DA_SDMMC_WRITE_DATA_CMD     0x62
#define DA_SDMMC_GET_CARD_TYPE      0x63

#define DA_UFS_SWITCH_PART_CMD    0x80
#define DA_UFS_WRITE_IMAGE_CMD    0x81
#define DA_UFS_WRITE_DATA_CMD     0x82
#define DA_UFS_READ_GPT_CMD		  0x85
#define DA_UFS_WRITE_GPT_CMD	  0x89



#define DA_USB_SETUP_PORT     0x70
#define DA_USB_LOOPBACK       0x71
#define DA_USB_CHECK_STATUS   0x72
#define DA_USB_SETUP_PORT_EX  0x73

/*For EFFUSE CMD*/
#define DA_READ_REG32_CMD				0x7A
#define DA_WRITE_REG32_CMD			0x7B
#define DA_PWR_READ16_CMD				0x7C
#define DA_PWR_WRITE16_CMD			0x7D
#define DA_PWR_READ8_CMD         0x7E
#define DA_PWR_WRITE8_CMD        0x7F


#define DA_EMMC_OTP_CHECKDEVICE_CMD		0x99
#define DA_EMMC_OTP_GETSIZE_CMD			0x9A
#define DA_EMMC_OTP_READ_CMD				0x9B
#define DA_EMMC_OTP_PROGRAM_CMD			0x9C
#define DA_EMMC_OTP_LOCK_CMD				0x9D
#define DA_EMMC_OTP_LOCK_CHECKSTATUS_CMD	0x9E


#define DA_WRITE_USB_DOWNLOAD_CONTROL_BIT_CMD   0xA0
#define DA_WRITE_PARTITION_TBL_CMD      0xA1
#define DA_READ_PARTITION_TBL_CMD       0xA2
#define DA_READ_BMT                     0xA3
#define DA_SDMMC_WRITE_PMT_CMD 0xA4
#define DA_SDMMC_READ_PMT_CMD 0xA5
#define DA_READ_IMEI_PID_SWV_CMD 0xA6
#define DA_READ_DOWNLOAD_INFO 0xA7
#define DA_WRITE_DOWNLOAD_INFO 0xA8
#define DA_SDMMC_WRITE_GPT_CMD 0xA9

#define DA_NOR_BLOCK_INDEX_TO_ADDRESS   0xB0	// deprecated
#define DA_NOR_ADDRESS_TO_BLOCK_INDEX   0xB1	// deprecated
#define DA_NOR_WRITE_DATA               0xB2	// deprecated
#define DA_NAND_WRITE_DATA              0xB3
#define DA_SECURE_USB_RECHECK_CMD		0xB4
#define DA_SECURE_USB_DECRYPT_CMD		0xB5
#define DA_NFB_BL_FEATURE_CHECK_CMD		0xB6	// deprecated
#define DA_NOR_BL_FEATURE_CHECK_CMD		0xB7	// deprecated

#define DA_SF_WRITE_IMAGE_CMD           0xB8	// deprecated


/* Android S-USBDL { */
#define DA_SECURE_USB_IMG_INFO_CHECK_CMD  0xB9
#define DA_SECURE_USB_WRITE               0xBA
#define DA_SECURE_USB_ROM_INFO_UPDATE_CMD 0xBB
#define DA_SECURE_USB_GET_CUST_NAME_CMD   0xBC
#define DA_SECURE_USB_CHECK_BYPASS_CMD    0xBE
#define DA_SECURE_USB_GET_BL_SEC_VER_CMD  0xBF
/* Android S-USBDL } */

#define DA_VERIFY_IMG_CHKSUM_CMD    0xBD

#define DA_GET_BATTERY_VOLTAGE_CMD  0xD0
#define DA_POST_PROCESS				0xD1
#define DA_SPEED_CMD				0xD2
#define DA_MEM_CMD					0xD3
#define DA_FORMAT_CMD				0xD4
#define DA_WRITE_CMD				0xD5
#define DA_READ_CMD					0xD6
#define DA_WRITE_REG16_CMD			0xD7
#define DA_READ_REG16_CMD			0xD8
#define DA_FINISH_CMD				0xD9
#define DA_GET_DSP_VER_CMD			0xDA
#define DA_ENABLE_WATCHDOG_CMD		0xDB
#define DA_NFB_WRITE_BLOADER_CMD	0xDC	// deprecated
#define DA_NAND_IMAGE_LIST_CMD		0xDD
#define DA_NFB_WRITE_IMAGE_CMD		0xDE
#define DA_NAND_READPAGE_CMD		0xDF
#define DA_CHK_PC_SEC_INFO_CMD		0xE0
#define DA_UPDATE_FLASHTOOL_CFG_CMD	0xE1
#define DA_CUST_PARA_GET_INFO_CMD	0xE2	// deprecated
#define DA_CUST_PARA_READ_CMD		0xE3	// deprecated
#define DA_CUST_PARA_WRITE_CMD		0xE4	// deprecated
#define DA_SEC_RO_GET_INFO_CMD		0xE5	// deprecated
#define DA_SEC_RO_READ_CMD			0xE6	// deprecated
#define DA_SEC_RO_WRITE_CMD			0xE7	// deprecated
#define DA_ENABLE_DRAM				0xE8
#define DA_OTP_CHECKDEVICE_CMD		0xE9
#define DA_OTP_GETSIZE_CMD			0xEA
#define DA_OTP_READ_CMD				0xEB
#define DA_OTP_PROGRAM_CMD			0xEC
#define DA_OTP_LOCK_CMD				0xED
#define DA_OTP_LOCK_CHECKSTATUS_CMD	0xEE
#define DA_GET_PROJECT_ID_CMD		0xEF
#define DA_GET_FAT_INFO_CMD			0xF0	// deprecated
#define DA_FDM_MOUNTDEVICE_CMD				0xF1
#define DA_FDM_SHUTDOWN_CMD					0xF2
#define DA_FDM_READSECTORS_CMD				0xF3
#define DA_FDM_WRITESECTORS_CMD				0xF4
#define DA_FDM_MEDIACHANGED_CMD				0xF5
#define DA_FDM_DISCARDSECTORS_CMD			0xF6
#define DA_FDM_GETDISKGEOMETRY_CMD			0xF7
#define DA_FDM_LOWLEVELFORMAT_CMD			0xF8
#define DA_FDM_NONBLOCKWRITESECTORS_CMD		0xF9
#define DA_FDM_RECOVERABLEWRITESECTORS_CMD	0xFA
#define DA_FDM_RESUMESECTORSTATES			0xFB
#define DA_NAND_EXTRACT_NFB_CMD				0xFC	// deprecated
#define DA_NAND_INJECT_NFB_CMD				0xFD	// deprecated

#define DA_MEMORY_TEST_CMD					0xFE
#define DA_ENTER_RELAY_MODE_CMD				0xFF

/* SPEED_PARA */
typedef enum {
	UART_BAUD_921600	= 0x01,
	UART_BAUD_460800	= 0x02,
	UART_BAUD_230400	= 0x03,
	UART_BAUD_115200	= 0x04,
	UART_BAUD_57600		= 0x05,
	UART_BAUD_38400		= 0x06,
	UART_BAUD_19200		= 0x07,
	UART_BAUD_9600		= 0x08,
	UART_BAUD_4800		= 0x09,
	UART_BAUD_2400		= 0x0a,
	UART_BAUD_1200		= 0x0b,
	UART_BAUD_300		= 0x0c,
	UART_BAUD_110		= 0x0d
}UART_BAUDRATE;

typedef enum {
	 CS_0 = 0
	,CS_1
	,CS_2
	,CS_3
	,CS_4
	,CS_5
	,CS_6
	,CS_7
	,CS_WITH_DECODER
	,MAX_CS = CS_WITH_DECODER
	,HW_CHIP_SELECT_END
} HW_ChipSelect_E;

typedef enum {
	 HW_STORAGE_NOR = 0
	,HW_STORAGE_NAND
	,HW_STORAGE_EMMC
	,HW_STORAGE_SDMMC
	,HW_STORAGE_UFS
	,HW_STORAGE_NONE
	,HW_STORAGE_TYPE_END
} HW_StorageType_E;

typedef enum {
	 HW_RAM_UNKNOWN = 0
	,HW_RAM_SRAM
	,HW_RAM_DRAM
	,HW_RAM_TYPE_END
} HW_RAMType_E;

typedef enum {
	 HW_MEM_NOR = 0			// NOR Flash
    ,HW_MEM_NAND			// NAND Flash
	,HW_MEM_EXT_SRAM		// External SRAM
	,HW_MEM_EXT_DRAM		// External DRAM
	,HW_MEM_EMMC            // EMMC Flash
	,HW_MEM_SDMMC           // SDMMC Flash
	,HW_MEM_TYPE_END
} HW_MemoryType_E;

typedef enum {
	 HW_MEM_IO_8BIT = 0		//  8-Bits Memory I/O
	,HW_MEM_IO_16BIT		// 16-Bits Memory I/O
	,HW_MEM_IO_32BIT		// 32-Bits Memory I/O
	,HW_MEM_IO_TYPE_END
} HW_MemoryIO_E;

typedef enum {
	 HW_MEM_DUMP = 0		// Memory Dump (Warning: it's not a test scenario!)
	,HW_MEM_PATTERN_TEST	// Pattern Test Scenario
	,HW_MEM_INC_DEC_TEST	// Increment/Decrement Test Scenario
	,HW_MEM_ADDR_BUS_TEST	// EMI Address Bus Test Scenario
	,HW_MEM_DATA_BUS_TEST	// EMI Data Bus Test Scenario
	,HW_MEM_IO_BUS_TEST		// NFI I/O Bus Test Scenario 
	,HW_MEM_TEST_TYPE_END
} HW_MemoryTestMethod_E;

typedef enum {
	 NUTL_READ_PAGE_SPARE = 0
	,NUTL_READ_PAGE_ONLY
	,NUTL_READ_SPARE_ONLY
	,NUTL_READ_PAGE_WITH_ECC
	,NUTL_READ_PAGE_SPARE_WITH_ECCDECODE
	,NUTL_VERIFY_AFTER_PROGRAM
	,NUTL_READ_PAGE_SPARE_NORANDOM
	,NUTL_READ_FLAG_END
} NUTL_ReadFlag_E;

typedef enum {
	 NUTL_ERASE = 0
	,NUTL_FORCE_ERASE
	,NUTL_MARK_BAD_BLOCK
	,NUTL_ERASE_FLAG_END
} NUTL_EraseFlag_E;

typedef enum {
	 NUTL_PROGRAM_PAGE_SPARE = 0
	,NUTL_PROGRAM_PAGE_ONLY
	,NUTL_PROGRAM_SPARE_ONLY
	,NUTL_PROGRAM_PAGE_FDM
    ,NUTL_PROGRAM_PAGE_SPARE_WITH_ECCENCODE
	,NUTL_PROGRAM_FLAG_END
} NUTL_ProgramFlag_E;

typedef enum {
	 NUTL_ADDR_LOGICAL= 0
	,NUTL_ADDR_PHYSICAL
	,NUTL_ADDR_FLAG_END
} NUTL_AddrTypeFlag_E;

typedef enum {
     EMMC_PART_UNKNOWN = 0
    ,EMMC_PART_BOOT1
    ,EMMC_PART_BOOT2
    ,EMMC_PART_RPMB
    ,EMMC_PART_GP1
    ,EMMC_PART_GP2
    ,EMMC_PART_GP3
    ,EMMC_PART_GP4
    ,EMMC_PART_USER
    ,EMMC_PART_END
} EMMC_Part_E;

typedef enum {
     UFS_PART_UNKNOWN = 0
    ,UFS_PART_LU0		//boot1
    ,UFS_PART_LU1		//boot2
    ,UFS_PART_LU2		//WP1
    ,UFS_PART_LU3		//WP2
    ,UFS_PART_LU4		//USE
    ,UFS_PART_END
} UFS_Part_E;


//----------------------------------------

// NOR Flash Callback Function group id
typedef enum {
	 ID_DUMMY_CMD = 0
	,ID_AMD_CMD_CB_UNLOCK_BYPASS_PGM
	,ID_AMD_CMD_CB_MIRRORBIT_BUF_PGM
	,ID_AMD_CMD_CB_MIRRORBIT_V2_BUF_PGM
	,ID_AMD_CMD_CB_S29WS_MIRRORBIT_BUF_PGM
	,ID_AMD_CMD_CB_S29N_MIRRORBIT_BUF_PGM
	,ID_AMD_CMD_CB_S29GL_MIRRORBIT_BUF_PGM
	,ID_AMD_CMD_CB_WORD_PGM
	,ID_AMD_AM29PDL128G_CMD_CB_UNLOCK_BYPASS_PGM
	,ID_TOSHIBA_CMD_CB_BUF_PGM
	,ID_INTEL_CMD_CB_WORD_PGM
	,ID_INTEL_CMD_CB_32WORD_BUF_PGM
	,ID_INTEL_CMD_CB_SIBLEY_BUF_PGM
	,ID_RENESAS_CMD_CB_WORD_PGM_64Mb
	,ID_RENESAS_CMD_CB_128WORD_PAGE_PGM_64Mb
	,ID_RENESAS_CMD_CB_WORD_PGM
	,ID_RENESAS_CMD_CB_128WORD_PAGE_PGM
	,ID_SHARP_CMD_CB_WORD_PGM
	,ID_SHARP_CMD_CB_16WORD_BUF_PGM
	,ID_SHARP_CMD_CB_WORD_PGM_NO_UNLOCK
	,ID_SST_CMD_CB_WORD_PGM
	,ID_SAMSUNG_CMD_CB_UNLOCK_BYPASS_PGM
	,ID_TOSHIBA_CMD_CB_PAGE_8WORDS_PGM
//FIXME: Serial Flash
	,ID_SST_CMD_CB_BUF_PGM
	,ID_MXIC_CMD_CB_BUF_PGM
	,ID_COMMON_NOR_CMD_CB_PGM
	
	,ID_NOR_CALLBACK_UNKNOWN = 0xFFFF
} NOR_CMD_Callback_ID_E;

// NAND Flash Callback Function group id
typedef enum {
	 ID_COMMON_CB_FUNC_SET = 0
	,ID_COMMON_CB_FUNC_SET_WITHOUT_COPYBACK
	,ID_ST_CB_FUNC_SET
	,ID_ST_CB_FUNC_SET_WITHOUT_COPYBACK
	,ID_superAND_CB_FUNC_SET
	
	,ID_NAND_CALLBACK_UNKNOWN = 0xFFFF
} NAND_CMD_Callback_ID_E;

//----------------------------------------

// NOR flash device id 
typedef enum {
           NOR_Spansion_S25FL004A = 0
           ,NOR_Spansion_S25FL008A
           ,NOR_Spansion_S25FL016A1
           ,NOR_Spansion_S25FL032A
           ,NOR_Spansion_S25FL064A
           ,NOR_Spansion_S25FL128P
           ,NOR_Spansion_S25FL256S
           ,NOR_Spansion_S25FL016A
           ,NOR_MXIC_25L400
           ,NOR_MXIC_25L80
           ,NOR_MXIC_25L160
           ,NOR_MXIC_25L1635D
           ,NOR_MXIC_25L320
           ,NOR_MXIC_25L640
           ,NOR_MXIC_25L1280
           ,NOR_MXIC_25L3235D
           ,NOR_MXIC_25L256
           ,NOR_MXIC_25L512
           ,NOR_MXIC_25U256
           ,NOR_ST_M25P80
           ,NOR_ST_M25P16
           ,NOR_ST_M25P32
           ,NOR_ST_M25P641
           ,NOR_ST_M25P128
           ,NOR_ST_M25PX32
           ,NOR_ST_M25PX64
           ,NOR_ST_M25P64
           ,NOR_SST_25VF016B
           ,NOR_WINBOND_W25X40
           ,NOR_WINBOND_W25X80
           ,NOR_WINBOND_W25X16
           ,NOR_WINBOND_W25X32
           ,NOR_WINBOND_W25X64
           ,NOR_WINBOND_W25Q256FV
           ,NOR_WINBOND_W25Q16CV
           ,NOR_WINBOND_W25Q32BV
           ,NOR_WINBOND_W25Q64BV
           ,NOR_WINBOND_W25Q128BV
           ,NOR_GD_GD25Q16BSIG
           ,NOR_GD_GD25Q32BSIG
           ,NOR_GD_GD25Q64BSIG
           ,NOR_GD_GD25Q128BSIG
           ,NOR_SST_SST25VF020
           ,NOR_SST_SST25VF040B
           ,NOR_SST_SST25VF080B
           ,NOR_SST_SST25VF016B
           ,NOR_SST_SST25VF032B
           ,NOR_SST_SST25VF064C
           ,NOR_ATMEL_AT25DF321
           ,NOR_ATMEL_AT25DF641
           ,NOR_ATMEL_AT26DF081A
           ,NOR_EON_EN25B40
           ,NOR_EON_EN25F80
           ,NOR_EON_EN25P16
           ,NOR_EON_EN25P32
           ,NOR_EON_EN25P64
           ,NOR_EON_EN25Q64
           ,NOR_EON_EN25P80
           ,NOR_AMIC_A25L40P
           ,NOR_AMIC_A25L040
           ,NOR_AMIC_A25L032
           
           ,NOR_LAST
           ,NOR_UNKNOWN = 0xFFFF		// Unknown Device 
} NOR_DeviceID_E;

// NAND flash device id 
typedef enum {
	 NAND_K9F5608Q0C = 0
	,NAND_K9F5608X0C
	,NAND_K9F5616Q0C
	,NAND_K9F5616X0C
	,NAND_K9K1208X0C
	,NAND_K9K1G08X0A
	,NAND_K9F1G08Q0M
	,NAND_K9F1G08X0M
	,NAND_K9F1G16Q0M
	,NAND_K9F1G16X0M
	,NAND_K9F2G08X0M
	,NAND_K9F2G16X0M
	,NAND_K9F2G08R0A
	,NAND_K9K4G08Q0M
	,NAND_K9K4G08U0M
	,NAND_K9K4G16Q0M
	,NAND_K9K4G16U0M
	,NAND_KTN0403CS_TCR1
	,NAND_KSLCGBL2GA2H2A
	,NAND_S34ML04G200TFI00
	,NAND_KSLCBBL1FB4G3A
	,NAND_TC58DVM82A1FT
	,NAND_TC58DVM92A1FT
	,NAND_TC58DVG02A1FT
	,NAND_TC58NVG0S3AFT
	,NAND_TH58NVG1S3AFT
	,NAND_HY27XS08561M
	,NAND_HY27XS08121M
	,NAND_HY27XA081G1M
	,NAND_HY27XA161G1M
	,NAND_ST128W3A
	,NAND_ST128W4A
	,NAND_ST256W3A
	,NAND_ST256W4A
	,NAND_ST512W3A
	,NAND_ST512W4A
	,NAND_ST01GW3A
	,NAND_ST01GW4A
	,NAND_ST512R3B
	,NAND_ST512W3B
	,NAND_ST512R4B
	,NAND_ST512W4B
	,NAND_ST01GR3B
	,NAND_ST01GW3B
	,NAND_ST01GR4B
	,NAND_ST01GW4B
	,NAND_ST02GR3B
	,NAND_ST02GW3B
	,NAND_ST02GR4B
	,NAND_ST02GW4B
	,NAND_ST04GR3B
	,NAND_ST04GW3B
	,NAND_ST04GR4B
	,NAND_ST04GW4B
	,NAND_ST08GR3B
	,NAND_ST08GW3B
	,NAND_ST08GR4B
	,NAND_ST08GW4B
	,NAND_K9F2808U0C
	,NAND_K9K8G08U0M
	,NAND_TH58NVG2D4BFT
	,NAND_HY27UF081G2M
	,NAND_TH58NVG1S8BFT
	,NAND_MT29F2G08AAC
	,NAND_MT29F2G08ABD
	,NAND_MT29F4G08BAB
	,NAND_MT29F4G16ABD
	,NAND_MT29F64G08CBABB
	,NAND_MT29F32G08CBADB
	,NAND_MT29F128G08CFAAA
	,NAND_SDTNRGAME_008G
	,NAND_SDTNRGAMA_008G
	,NAND_TC58TEG5DCJTA00
	,NAND_SDTNRGAMA_008GK
	,NAND_SDTNRFAMA_004GK
	,NAND_SDTNQGAMA_008G
	,NAND_TC58TEG5DCKTA00
	,NAND_TC58TEG6DDKTA00
	,NAND_H27UCG8T2ATR
	,NAND_SUPERAND51208
	,NAND_SUPERAND51216
	,NAND_HYF33DS51280
	,NAND_HYF33DS1G80
	,NAND_HYF33DS1G16
	,NAND_NCSPN4N2A
	,NAND_TC58NWM9S3B
	,NAND_HY27UF082G2M
	,NAND_M6MGA157F2LCWG08
	,NAND_M6MGA157F2LCWG16
	,NAND_TC58NWM9S8C
	,NAND_TC58NWG0S8C
	,NAND_HY27US16561M
	,NAND_HY27UF162G5A
	,NAND_HY27UT088G2A
	,NAND_TC58NVG4D1DTG
	,NAND_TY58NYG3S5F
	,NAND_K5E1257ACM
	,NAND_K5E1G12ACF
	,NAND_MT29F2G16ABD
	,NAND_HYD0SFG0MF1P
	,NAND_HYD0SQG0MF1P
	,NAND_H9DA4VH4JJAMCR
	,NAND_H9DA4GH4JJAMCR
	,NAND_H27UBG8T2CTR
	,NAND_H27UCG8T2ETR
	,NAND_H8BCS0CG0MBR
	,NAND_H8BCS0PG0MBP
	,NAND_TC58NYG0S8C
    ,NAND_TC58NYG1S8C
	,NAND_ST512R3A
	,NAND_ST512R4A
	,NAND_MT29F1G08ABB
	,NAND_MT29F1G16ABB
    ,NAND_ASU1GA30GT_G30CA
    ,NAND_K522H1GACE
    ,NAND_H8BCS0SI0MBR
    ,NAND_MT29F4G08ABC
    ,NAND_MT29C4G48MAZAPAKD
    ,NAND_H8BCS0UN0MCR
    ,NAND_TYBC0A111236KC10
	,NAND_TOSHIBA_mLBA
    ,NAND_H9DA4GH4JJAMCR_4EM
    ,NAND_H9DA4VH2GJAMCR_4EM
    ,NAND_KA100O015B_BJTT
    ,NAND_KA100O015E_BJTT
    ,NAND_KF94G16Q4V
    ,NAND_KF94G16Q4X
    ,NAND_MT29C4G96MAZAPCJA_5IT
    ,NAND_MT29C4G48MAAHBAAKS_5WT
    ,NAND_MT29C8G96MAZBADKD_5WT
    ,NAND_KF98G16Q4X
    ,NAND_TY58NYG2S8E
   ,NAND_K9K8G16U0K
   ,NAND_TYBC0A111557KC
   ,NAND_TH58NVG7DDJTA20
   ,NAND_H27U8G8T2B
   ,NAND_H27UAG8T2BTR
   ,NAND_ST08GW3C
   ,NAND_TC58NVG6D2GTA00
   ,NAND_H27UBG8T2B
   ,NAND_K9GBG08U0A
   ,NAND_H27UAG8T2A
   ,NAND_MT29F8G08ABA
   ,NAND_H27U4G8F2DTR
   ,NAND_MT29F1G08ABA
   ,NAND_H27US08561A
   ,NAND_H9TA4GH2GDMCPR
   ,NAND_H9DA4VH2GJMMCR_4EM
   ,NAND_A5R4GA41ATS
   ,NAND_KEM51200_NADGSBXM2C32
    ,NAND_F592G81A//bayi
   //------ SPI-NAND begin ------
   ,SNAND_GD5F4GQ4UAYIG
   //------ SPI-NAND end ------

    ,NAND_LAST
    ,SNAND_LAST = NAND_LAST
	,NAND_UNKNOWN = 0xFFFF		    // Unknown Device
	,SNAND_UNKNOWN = NAND_UNKNOWN
} NAND_DeviceID_E;
/* Add for update DL image status for eMMC, 2012.02.20 */
typedef enum {
    kDLMediateImage = 0,
    kDLFirstImage = 1,
    kDLLastImage = 2,
    kDLOnlyImage = 3,
    kDLUnknownImage = 4,
} DLImageStatus;

/*Support to check format/download status, 2013/01/19 {*/
#define DL_MAGIC "DOWNLOAD INFORMATION!!"
#define DL_INFO_VER_V1  "V1.0"
#define DL_INFO_VER_V2  "V2.0"

#define DL_MAGIC_NUM_COUNT 32
#define DL_MAGIC_OFFSET 24
#define DL_IMG_NAME_LENGTH 16
#define DL_CUSTOM_INFO_SIZE (128)

/*download status v1 and old version for emmc*/
#define FORMAT_START    "FORMAT_START"
#define FORMAT_DONE      "FORMAT_DONE"
#define BL_START             "BL_START"
#define BL_DONE              "BL_DONE"
#define DL_START            "DL_START"
#define DL_DONE             "DL_DONE"
#define DL_ERROR           "DL_ERROR"
#define DL_CK_DONE       "DL_CK_DONE"
#define DL_CK_ERROR      "DL_CK_ERROR"

/*v1 and old version for emmc*/
#define CHECKSUM_PASS "PASS"
#define CHECKSUM_FAIL "FAIL"

/*old version for nand*/
#define NAND_DL_PASS_V0 "PASS"
#define NAND_DL_FAIL_V0  "FAIL"

typedef enum {
	DL_INFO_VERSION_V0 = 0,
	DL_INFO_VERSION_V1 = 1,
	DL_INFO_VERSION_UNKOWN = 0xFF,
} DLInfoVersion;

/*DL status old version on EMMC {*/
typedef struct {
    unsigned int image_index;
    unsigned int pc_checksum;
    unsigned int da_checksum;
    char   checksum_status[8];
} CHECKSUM_INFO_EMMC;

typedef struct {
    char          magic_num[DL_MAGIC_NUM_COUNT];
    CHECKSUM_INFO_EMMC part_info[MAX_LOAD_SECTIONS];
    char          ram_checksum[16];
    char          download_status[16];
} DL_STATUS_EMMC;
/*DL status old version on EMMC }*/

/*Old version on NAND {*/
typedef struct {
    unsigned int image_index;
    unsigned int pc_checksum;
    unsigned int da_checksum;
    unsigned int pl_checksum;
} CHECKSUM_INFO;

typedef struct {
    char          magic_num[DL_MAGIC_NUM_COUNT];
    CHECKSUM_INFO part_info[20];
    char          isram_checksum[16];
    char          exmem_checksum[16];
    char          download_status[16];
} DL_STATUS;
/*Old version on NAND }*/

/*version v1.0 {*/
typedef struct {
    char image_name[DL_IMG_NAME_LENGTH];
} IMG_DL_INFO;

typedef struct {
    unsigned int image_index;
    unsigned int pc_checksum;
    unsigned int da_checksum;
    char   checksum_status[8];
} CHECKSUM_INFO_V1;
//Just compatible old load
#ifdef Compat_Old_Load
typedef struct {
    char          magic_num[DL_MAGIC_OFFSET];
    char		  version[DL_MAGIC_NUM_COUNT-DL_MAGIC_OFFSET];
    CHECKSUM_INFO_V1 part_info[40];
    char          ram_checksum[16];
    char          download_status[16];
    IMG_DL_INFO   img_dl_info[40];
} DL_STATUS_V_old;
#endif
typedef struct {
    char          magic_num[DL_MAGIC_OFFSET];
    char		  version[DL_MAGIC_NUM_COUNT-DL_MAGIC_OFFSET];
    CHECKSUM_INFO_V1 part_info[MAX_LOAD_SECTIONS];
    char          ram_checksum[16];
    char          download_status[16];
    IMG_DL_INFO   img_dl_info[MAX_LOAD_SECTIONS];
} DL_STATUS_V1;
/*version v1.0 }*/
/*Support to check format/download status, 2013/01/19 {*/

/* Boot channel, 2012/08/22 { */
typedef enum {
    MSDC_0 = 0,
    MSDC_1,
    MSDC_2,
    MSDC_3,
    MSDC_4,
    MSDC_END
} BootChannel_E;
/* Boot channel, 2012/08/22 } */

/* Add Partition Operation type, 2012/12/21 {*/
typedef enum {
    OPER_BOOTLOADERS = 0,
    OPER_UPDATE,
    OPER_INVISIBLE,
    OPER_PROTECTED,
    OPER_RESERVED,
    OPER_BINREGION,
    OPER_UNKNOWN,
    OPER_END
} Partition_Operation_E;
/* Add Partition Operation type, 2012/12/21 }*/
typedef enum {
	PMT = 0
    ,GPT =0x1			
    ,Only_P_GPT	= 0x2	
    ,Only_S_GPT	= 0x4	
    ,Both_P_S_GPT =	0x6	
    ,PT_UNKNOWN 	
} PTType;

typedef enum {
	PGPT = 0
    ,SGPT
    ,GTP_UNKNOWN
} GPTType;


typedef enum {
	MAC = 10
    ,WINDOWS
    ,LINUX
} HostOS;
#ifdef _WIN32
	#define HOST_OS WINDOWS
#else
	#define HOST_OS LINUX
#endif

#endif
