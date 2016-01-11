#ifndef _CFG_HTC_DATA_FILE_H
#define _CFG_HTC_DATA_FILE_H

/**
htc_board_info_struct size:  2048	partition size:   16K
htc_mfg_struct size:        75804	parittion size:  256K
htc_misc_struct size:       14352	parittion size: 1024K
**/

#define MTK_PRODUCT_INFO_SZE    (   1*1024)
#define HTC_BOARD_INFO_SIZE     (   2*1024)
#define HTC_MFG_SIZE            (  30*1024)
#define HTC_RESERVED_SIZE       (  14*1024)

#define MTK_PRODUCT_INFO_OFFSET (0)
#define HTC_BOARD_INFO_OFFSET   (1024*1024)
#define HTC_MFG_OFFSET          (1152*1024)
#define HTC_RESERVED_OFFSET     (2048*1024)

#define BOOTLOADER_CONFIG_MAX 512

/* board_info data */
#define CID_SIZE 8
#define IMEI_SIZE 15
#define MEID_SIZE 14
#define BOARD_INFO_MAGIC "HTC-BOARD-INFO!@"

struct sku_id{
	unsigned char num;
	unsigned char pcb_apn[3]; /* PCBA P/N (part number) */
	unsigned char pcb_reserved;
	unsigned char bom_id; 		/**< BOM version
					 * If current BOM is XA01, the value should be 1h (0001);
					 * If current BOM is XA02, the value should be 2h (0010).
					 */
	unsigned char pcb_id2;
	unsigned char pcb_id;
	unsigned char rf_id[32];
	unsigned char checksum[4];
	unsigned char engineer_id[4];
};

typedef struct{
	char magic[16];
	int pid;
	char cid[8];
	char non_used_0[IMEI_SIZE + 1];	//imei[IMEI_SIZE + 1];
	int non_used_1;					//keycardid;
	char device_id[32];
	/**
	char board_meid[MEID_SIZE + 2];
	union {
		unsigned int val;
		char buf[16];
	} mdm9k_serial;
	char board_imei2[IMEI_SIZE + 1];
	unsigned char hashimei[32];
	**/
} htc_board_info_struct;

typedef struct{
	char start[0];
	htc_board_info_struct data;
	unsigned char reserved[HTC_BOARD_INFO_SIZE - sizeof(htc_board_info_struct)];
	char end[0];
} HTC_BOARD_INFO;

typedef struct{
	//mfg partition
	char firmware_main_version[16];
	char mfg_name[16];
	char model_id[64];
	char mb_serial_no[16];
	char color_ID[16];
	struct sku_id skuid;
	int cfgdata[BOOTLOADER_CONFIG_MAX];
	char autodownload[16];
	char cProductDate[8];
} htc_mfg_struct;

typedef struct{
	htc_mfg_struct data;
	unsigned char reserved[HTC_MFG_SIZE - sizeof(htc_mfg_struct)];
} HTC_MFG;

typedef struct{
	unsigned char data[HTC_RESERVED_SIZE];
} HTC_RESERVED;

#define CFG_FILE_HTC_BOARD_INFO_SIZE    sizeof(HTC_BOARD_INFO)
#define CFG_FILE_HTC_BOARD_INFO_TOTAL   1

#define CFG_FILE_HTC_MFG_SIZE    sizeof(HTC_MFG)
#define CFG_FILE_HTC_MFG_TOTAL   1

#define CFG_FILE_HTC_RESERVED_SIZE    sizeof(HTC_RESERVED)
#define CFG_FILE_HTC_RESERVED_TOTAL   1


#ifndef NULL
#define NULL ((void *)0)
#endif

#define MTK_BARCODE_OFFSET          MTK_PRODUCT_INFO_OFFSET
#define MTK_BARCODE_SIZE            64

#define MTK_IMEI_SIZE               10
#define MTK_IMEI_OFFSET(index)      (MTK_BARCODE_OFFSET + MTK_BARCODE_SIZE + index*MTK_IMEI_SIZE)

#endif /* _CFG_HTC_DATA_FILE_H */
