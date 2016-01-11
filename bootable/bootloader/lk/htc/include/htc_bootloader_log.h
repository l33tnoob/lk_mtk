#ifndef __HTC_BOOTLOADER_LOG_H
#define __HTC_BOOTLOADER_LOG_H

#if ENABLE_BOOTLOADER_LOG
#if (BL_LOG_PHYS && BL_LOG_SIZE)
	#define BOOT_DEBUG_LOG_BASE_PA	BL_LOG_PHYS
	#define BOOT_DEBUG_LOG_SIZE	BL_LOG_SIZE
#else
	#error "ERROR! `BOOT_DEBUG_LOG_BASE_PA' is not set."
	#error "ERROR! `BOOT_DEBUG_LOG_SIZE' is not set."
#endif
#define BOOT_DEBUG_LOG_BASE_VA	(addr_t)(BOOT_DEBUG_LOG_BASE_PA)


#if (LAST_BL_LOG_PHYS)
	#define BOOT_DEBUG_LAST_LOG_BASE_PA	(LAST_BL_LOG_PHYS)
#else
	#error "`LAST_BL_LOG_PHYS' is not set."
#endif
#define BOOT_DEBUG_LAST_LOG_BASE_VA	(addr_t)(BOOT_DEBUG_LAST_LOG_BASE_PA)

void bldr_log_write(const char *s);
void bldr_log_init(void*, size_t);
#else
static inline void bldr_log_write(const char *s) { };
static inline void bldr_log_init(void*, size_t) { };
#endif /* ENABLE_BOOTLOADER_LOG */
#endif /* __HTC_BOOTLOADER_LOG_H */
