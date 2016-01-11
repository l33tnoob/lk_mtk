#ifndef __COMMANDS_H
#define __COMMANDS_H

void cmd_getvar(const char *arg, void *data, unsigned sz);
void cmd_boot(const char *arg, void *data, unsigned sz);
void cmd_reboot(const char *arg, void *data, unsigned sz);
void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz);
void cmd_download(const char *arg, void *data, unsigned sz);
void cmd_overwirte_cmdline(const char *arg, void *data, unsigned sz);
void cmd_continue(const char *arg, void *data, unsigned sz);
void cmd_oem_p2u(const char *arg, void *data, unsigned sz);
void cmd_oem_reboot2recovery(const char *arg, void *data, unsigned sz);
void cmd_oem_append_cmdline(const char *arg, void *data, unsigned sz);
#ifdef MTK_JTAG_SWITCH_SUPPORT
void cmd_oem_ap_jtag(const char *arg, void *data, unsigned sz);
#endif
#ifdef MTK_OFF_MODE_CHARGE_SUPPORT
void cmd_oem_off_mode_charge(const char *arg, void *data, unsigned sz);
#endif
#ifdef MTK_TC7_COMMON_DEVICE_INTERFACE
void cmd_oem_ADB_Auto_Enable(const char *arg, void *data, unsigned sz);
#endif
#endif
