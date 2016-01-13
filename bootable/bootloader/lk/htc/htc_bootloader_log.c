#include <stdlib.h>
#include <platform.h>
#include <sys/types.h>
#include <htc_bootloader_log.h>
#include <libfdt.h>
#include <string.h>
#include <printf.h>

static void* bldr_log_base;
static void* last_bldr_log_base;

static size_t bldr_log_size = 0;

#define BOOT_DEBUG_MAGIC		0xAACCBBDD

struct bldr_log_header {
	uint32_t magic;
	uint32_t offset;
	uint32_t rotate_flag;
};

static int bldr_log_check_header(void)
{
	struct bldr_log_header *header = (struct bldr_log_header *)bldr_log_base;
	/* Check magic first */
	if (header->magic == BOOT_DEBUG_MAGIC) {
		/* Check offset range */
		if (header->offset >= sizeof(struct bldr_log_header) &&
		    header->offset <  sizeof(struct bldr_log_header) + bldr_log_size)
			return 1;
	}
	return 0;
}

static void bldr_log_reset(void)
{
	struct bldr_log_header *header = (struct bldr_log_header *)bldr_log_base;

	header->magic = BOOT_DEBUG_MAGIC;
	header->offset = sizeof(struct bldr_log_header);
	header->rotate_flag = false;
}

void bldr_log_write(const char *s)
{
	struct bldr_log_header *header = (struct bldr_log_header *)bldr_log_base;
	int32_t len = strlen(s);

	if (!bldr_log_base)
	{
		/* Ramlog not initialized! */
		return;
	}

	if (header->offset + len > bldr_log_size) {
		header->offset = sizeof(struct bldr_log_header);
		header->rotate_flag = true;
	}

	len = MIN(len, (int32_t)(bldr_log_size - header->offset));
	memcpy(bldr_log_base + header->offset, s, len);
	header->offset += len;
}

void bldr_log_init(void* bldr_log_base_pa, size_t size)
{
	// volatile uint32_t magic; // remove unused variable
	zzytest_printf("begin bldr_log_init\n");
	if (!bldr_log_base_pa)
	{
		dprintf(CRITICAL, "Ramlog address is NULL!\r\n\n");
		return;
	}

	bldr_log_base = (void*) (addr_t) bldr_log_base_pa;
	bldr_log_size = size;

	last_bldr_log_base = bldr_log_base + bldr_log_size;

	if (bldr_log_check_header()) {
		/* back up previous bootloader log buffer to last_bldr_log_base */
		/* TODO: rotate log buffer to avoid memcpy */
		memcpy(last_bldr_log_base, bldr_log_base, bldr_log_size);
	}

	bldr_log_reset();

	dprintf(INFO,"%s: bldr_log_base=%p, bldr_log_size=%zd\r\n", __func__, bldr_log_base, bldr_log_size);
}
