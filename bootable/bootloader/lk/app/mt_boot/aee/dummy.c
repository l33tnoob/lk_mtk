#include <dev/mrdump.h>

int mrdump_detection(void)
{
  return 0;
}

int mrdump_run2(void)
{
  return 0;
}

#if _MAKE_HTC_LK
int aee_mrdump_set_defaul_value(void)
{
  return 0;
}

unsigned int aee_mrdump_get_lbaooo(void)
{
  return 0;
}

unsigned char *aee_get_reboot_string()
{
  return 0;
}

void htc_ramdump2USB()
{
  return;
}

void htc_ramdump2eMMC()
{
  return;
}
#endif
