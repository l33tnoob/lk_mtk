#ifndef __HTC_FASTBOOT_POLICY_H__
#define __HTC_FASTBOOT_POLICY_H__

#include <htc_security_util.h>

int htc_fastboot_cmd_policy_check(const char *prefix);
int htc_fastboot_getvar_policy_check(const char *name);
#endif //__HTC_FASTBOOT_POLICY_H__
