#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <reg.h>
#include <target.h>
#include <platform/mt_reg_base.h>
#include <platform/mmc_core.h>
#include <platform/mmc_wrapper.h>
#include <arch/arm.h>

#include "../app/mt_boot/fastboot.h"
#include <dev/udc.h>

typedef enum {
	DUMP_UNKNOWN = 0,
	DUMP_RAM,
	DUMP_EMMC,
} dump_type;

extern int fastboot_write(const unsigned char *data, int size);

unsigned hex2unsigned(const char *x)
{
    unsigned n = 0;

    while(*x) {
        switch(*x) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = (n << 4) | (*x - '0');
            break;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            n = (n << 4) | (*x - 'a' + 10);
            break;
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            n = (n << 4) | (*x - 'A' + 10);
            break;
        default:
            return n;
        }
        x++;
    }

    return n;
}

/*
 * example : fastboot dump ram ram.bin 0x43F00000 0x100000
 * arg = :00001000,43F00000,000100000,ram
 * example : fastboot dump emmc emmc.bin 200000 100
 * arg = :00001000,00030d40,00000064,emmc
 */
void cmd_dump(const char *arg, void *data, unsigned sz)
{
        uint64_t start_addr;
        uint32_t block_length, data_length;
        uint32_t read_length, block_size;
        char temp_buf[64]__attribute__((aligned(CACHE_LINE)));
	unsigned int dump_src = DUMP_UNKNOWN;
        uint8_t *cbuf = NULL;
 
	dprintf(ALWAYS, "cmd_dump()\n");
        /* read all argument */
        /* [dump command format] dump:{block size},{start},{len},{name} */
        memset(temp_buf, 0, 32);
        memcpy(temp_buf, arg + 1, 8);
        block_length = hex2unsigned((const char*)temp_buf);

        memcpy(temp_buf, arg + 10, 8);
        start_addr = (uint64_t)hex2unsigned((const char*)temp_buf);

        memcpy(temp_buf, arg + 19, 8);
        data_length  = hex2unsigned((const char*)temp_buf);

	strncpy(temp_buf, arg + 28, 10);
	if (!strcmp(temp_buf, "ram")) {
		dump_src = DUMP_RAM;
	} else if (!strcmp(temp_buf, "emmc")) {
		dump_src = DUMP_EMMC;
	}

	if (dump_src == DUMP_UNKNOWN) {
		dprintf(CRITICAL, "ERROR: Type not define '%s'\n", temp_buf);
		fastboot_fail("");
		return;
	} else if (dump_src == DUMP_EMMC) {
		/* get emmc block size, always is 512 */
		block_size = htc_mmc_get_device_blocksize();
		start_addr *= block_size;
		data_length *= block_size;
	}

	cbuf = (uint8_t*)memalign(CACHE_LINE, ROUNDUP(block_length, CACHE_LINE));
	if(cbuf == NULL){
		dprintf (CRITICAL, "[fastboot] Error in alloc %d byte memory.\n", block_length);
		fastboot_fail("Error in alloc");
		return;
	}

        /* write protocal string at first */
        strcpy(temp_buf, "DATA");
        snprintf(temp_buf + 4, 8 + 1, "%08x", block_length);
        fastboot_write((const unsigned char*)temp_buf, 0);

        do {
		/* Set read length from ram,  the unit of read_length is byte */
		if (block_length < data_length)
			read_length = block_length;
		else
			read_length = data_length;

		if (dump_src == DUMP_RAM) {
			/* For normal region, just transfer them. */
			//cbuf = (uint8_t *)start_addr;
			memcpy(cbuf, start_addr, read_length);
		} else if (dump_src == DUMP_EMMC) {
			/* read data from emmc, the unit of read_length is still byte */
			if (htc_mmc_read(start_addr, (unsigned int *)cbuf, read_length)) {
				dprintf(CRITICAL, "ERROR: Cannot read emmc\n");
				fastboot_fail("");
				return;
			}
		}

		/* write data from device to pc */
		fastboot_write(cbuf, read_length);

		/* shift address and decrease the rest read length, both of unit size is KB */
		data_length -= read_length;
		start_addr += read_length;
        }while(data_length);
        fastboot_okay("");
}

void htc_dump_command_register()
{
	fastboot_register("dump", cmd_dump, 0);
}

