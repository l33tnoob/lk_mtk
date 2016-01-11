#include <debug.h>
#include <sys/types.h>
#include <fastboot.h>
#include <htc_fastboot_policy.h>

#define POLICY_ALL_ALLOWED 0
#define POLICY_SHIP_DENIED 1
#define POLICY_SON_DENIED 2
#define POLICY_SON_NO_SMARTCARD_DENIED 4
#define POLICY_SON_NO_UNLOCKED_DENIED 8
#define POLICY_ALL_DENIED 0xFFFFFFFF

#ifdef SHIP_BUILD
#define HTC_FB_POLICY_LOGI( fmt, args...)   do {;}while(0)
#define HTC_FB_POLICY_LOGE( fmt, args...)   dprintf(CRITICAL,"[HTC FB POLICY/E] "fmt,##args)
#else
#define HTC_FB_POLICY_LOGI( fmt, args...)   dprintf(CRITICAL,"[HTC FB POLICY/I] "fmt,##args)
#define HTC_FB_POLICY_LOGE( fmt, args...)   dprintf(CRITICAL,"[HTC FB POLICY/E] "fmt,##args)
#endif


typedef struct {
	const char* name;
	int policy;
} fastboot_policy_struct;

fastboot_policy_struct fastboot_oem_cmd_policy_table[] = {

	//cfgdata
	{ "oem readconfig", POLICY_ALL_ALLOWED },
	{ "oem writeconfig", POLICY_SON_DENIED },

	//sku
	{ "oem readsku", POLICY_ALL_ALLOWED },
	{ "oem writesku", POLICY_SON_DENIED },

	//security
	{ "oem readsecureflag", POLICY_ALL_ALLOWED },
	{ "oem readsmartcardmagic", POLICY_ALL_ALLOWED },
	{ "oem get_tamper_flag", POLICY_ALL_ALLOWED },
	{ "oem get_force_sec_boot_reason", POLICY_ALL_ALLOWED },
	{ "oem writesecureflag", POLICY_ALL_ALLOWED },
	{ "oem writesmartcardmagic", POLICY_SON_DENIED },
	{ "oem unlock", POLICY_ALL_DENIED },
	{ "oem lock", POLICY_ALL_DENIED },
	{ "oem key", POLICY_ALL_DENIED },
	{ "oem lks", POLICY_ALL_DENIED },

	//oem reboot-XXXX
	{ "oem reboot-download", POLICY_ALL_ALLOWED },
	{ "oem reboot-recovery", POLICY_ALL_ALLOWED },
	{ "oem reboot-ftm", POLICY_ALL_ALLOWED },
	{ "oem reboot-meta", POLICY_ALL_ALLOWED },
	{ "oem rebootRUU", POLICY_ALL_ALLOWED },

	//eMMC func
	{ "oem listpartition", POLICY_SON_DENIED },
	{ "oem read_mmc", POLICY_SON_DENIED },
	{ "oem write_mmc", POLICY_SON_DENIED },
	{ "oem test_emmc", POLICY_SON_DENIED },
	{ "oem check_emmc_mid", POLICY_SON_DENIED },
	{ "oem get_ext_csd_emmc", POLICY_SON_DENIED },
	{ "oem get_wp_info_emmc", POLICY_SON_DENIED },

	//Others
	{ "oem ddrtest", POLICY_SON_DENIED },
	{ "oem p2u", POLICY_ALL_DENIED},
	{ "oem append-cmdline", POLICY_ALL_DENIED},
	{ "oem off-mode-charge", POLICY_ALL_DENIED},
	{ "oem auto-ADB", POLICY_ALL_DENIED},
	{ "oem setiwledcon3", POLICY_SON_DENIED },
	{ "oem set_batt_chg_current", POLICY_SON_DENIED },
	{ "oem ?", POLICY_SON_DENIED },
};

fastboot_policy_struct fastboot_cmd_policy_table[] = {
	//boot
	{ "reboot", POLICY_ALL_ALLOWED },
	{ "reboot-bootloader", POLICY_ALL_ALLOWED },
	{ "boot", POLICY_SON_NO_UNLOCKED_DENIED },
	{ "continue", POLICY_SON_NO_UNLOCKED_DENIED },

	//Security
	{ "signature", POLICY_ALL_DENIED},

	{ "flash", POLICY_ALL_ALLOWED },
	{ "download", POLICY_SON_NO_UNLOCKED_DENIED },
	{ "erase", POLICY_ALL_ALLOWED },



	{ "getvar", POLICY_ALL_ALLOWED },
	{ "dump", POLICY_SON_NO_SMARTCARD_DENIED },
};

fastboot_policy_struct fastboot_getvar_policy_table[] = {
	{ "all", POLICY_ALL_ALLOWED },
	{ "product", POLICY_ALL_DENIED },
	{ "kernel", POLICY_ALL_DENIED },
	{ "serialno", POLICY_ALL_ALLOWED },
	{ "max-download-size", POLICY_ALL_ALLOWED },
	{ "battery-status", POLICY_ALL_ALLOWED },
	{ "off-mode-charge", POLICY_ALL_DENIED },
	{ "charger-screen-enabled", POLICY_ALL_DENIED },
	{ "display-panel", POLICY_ALL_DENIED },
	{ "version", POLICY_ALL_ALLOWED },

	//security
	{ "warranty", POLICY_ALL_ALLOWED },
	{ "secure", POLICY_ALL_ALLOWED },
	{ "unlocked", POLICY_ALL_ALLOWED },

	//eMMC func
	{ "partition-", POLICY_ALL_DENIED },
};

static fastboot_policy_struct* get_fastboot_oem_cmd_policy(const char* prefix) {
	int i = 0;
	int size = sizeof(fastboot_oem_cmd_policy_table) / sizeof(fastboot_policy_struct);

	for ( i = 0; i < size; i++ ) {
		if (
			!strncmp("oem ", fastboot_oem_cmd_policy_table[i].name, 4) &&
			0 == strncmp(fastboot_oem_cmd_policy_table[i].name, prefix, strlen(fastboot_oem_cmd_policy_table[i].name)))
		{
			return &fastboot_oem_cmd_policy_table[i];
		}
	}

	return NULL;
}

static fastboot_policy_struct* get_fastboot_cmd_policy(const char* prefix) {
	int i = 0;
	int size = sizeof(fastboot_cmd_policy_table) / sizeof(fastboot_policy_struct);

	for ( i = 0; i < size; i++ ) {
		if (0 == strncmp(fastboot_cmd_policy_table[i].name, prefix, strlen(fastboot_cmd_policy_table[i].name))) {
			return &fastboot_cmd_policy_table[i];
		}
	}

	return NULL;
}

static fastboot_policy_struct* get_fastboot_getvar_policy(const char* name) {
	int i = 0;
	int size = sizeof(fastboot_getvar_policy_table) / sizeof(fastboot_policy_struct);

	for ( i = 0; i < size; i++ ) {
		if (0 == strncmp(fastboot_getvar_policy_table[i].name, name, strlen(fastboot_getvar_policy_table[i].name))) {
			return &fastboot_getvar_policy_table[i];
		}
	}

	return NULL;
}

static int htc_fastboot_policy_check(fastboot_policy_struct* fastboot_policy) {
	int ret = 0;

	if (fastboot_policy != NULL) {
		switch(fastboot_policy->policy) {
			case POLICY_ALL_DENIED:
				ret = 0;
				break;
			case POLICY_SHIP_DENIED:
				#ifdef SHIP_BUILD
				ret = 0;
				#else
				ret = 1;
				#endif
				break;
			case POLICY_SON_DENIED:
				if (!setting_security()) { //S-OFF
					ret = 1;
				}
				break;
			case POLICY_SON_NO_SMARTCARD_DENIED:
				if (setting_security()) { // S-ON
					if (setting_smart_card()) { //Have SMART-SD
						ret = 1;
					}
				} else { // S-OFF
					ret = 1;
				}
				break;
			case POLICY_SON_NO_UNLOCKED_DENIED:
				if (setting_security()) { // S-ON
					if (get_unlock_status()) { //UNLOCKED
						ret = 1;
					}
				} else { // S-OFF
					ret = 1;
				}
				break;

			default:
				ret = 1;
				break;
		}
	}
	return ret;
}

int htc_fastboot_cmd_policy_check(const char *prefix) {
	fastboot_policy_struct* fastboot_policy = NULL;

	if (prefix != NULL) {
		if (strncmp("oem ", prefix, 4) == 0 ) {
			fastboot_policy = get_fastboot_oem_cmd_policy(prefix);
			if (fastboot_policy == NULL) {
				HTC_FB_POLICY_LOGI("WARNING: \"%s\" is not found! allow by default!\n", prefix);
				return 1;
			} else {
				return htc_fastboot_policy_check(fastboot_policy);
			}
		} else {
			fastboot_policy = get_fastboot_cmd_policy(prefix);
			if (fastboot_policy == NULL) {
				HTC_FB_POLICY_LOGI("WARNING: \"%s\" is not found! allow by default!\n", prefix);
				return 1;
			} else {
				return htc_fastboot_policy_check(fastboot_policy);
			}
		}
	}

	return 0;
}

int htc_fastboot_getvar_policy_check(const char *name) {
	fastboot_policy_struct* fastboot_policy = NULL;

	if (name != NULL) {
		fastboot_policy = get_fastboot_getvar_policy(name);
		if (fastboot_policy == NULL) {
			HTC_FB_POLICY_LOGI("WARNING: \"getvar %s\" is not found! allow by default!\n", name);
			return 1;
		} else {
			return htc_fastboot_policy_check(fastboot_policy);
		}
	}

	return 0;
}
