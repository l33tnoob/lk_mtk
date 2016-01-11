/* 
 * Copyright (C) 2014 HTC Corporation.
 */
#ifndef __HTC_RAMDUMP_H
#define __HTC_RAMDUMP_H


#define MAX_FILENAME_LEN        64
#define RESET_MSG_LENGTH        512 //It must be sizeof(int) alignment.

typedef struct _ramdump_e{
        char name[MAX_FILENAME_LEN];
        int     type;
} ramdump_e;

enum {
	RAMDUMP_ND_RDMP, /* No_Delete.rdmp */
	/*
	RAMDUMP_LAST_K,
	RAMDUMP_LK_LOG,
	RAMDUMP_LK_LAST_LOG,
	*/
};

#define RESTART_REASON_OEM_BASE         0x6F656D00
#define RESTART_REASON_BOOT_BASE        0x77665500
#define RESTART_REASON_MASK             0xFFFFFF00
#define RESTART_REASON_POWEROFF         (RESTART_REASON_BOOT_BASE | 0xBB)

struct ramdump_file_list {
        char filename[MAX_FILENAME_LEN];
        int display_row;
};

void htc_ramdump2eMMC();
void htc_ramdump2USB();

#endif
