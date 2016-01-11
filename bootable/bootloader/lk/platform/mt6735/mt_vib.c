/******************************************************************************
 * mt_vib.c
 *
 * DESCRIPTION:
 *
 ******************************************************************************/

#include <platform/mt_gpio.h>
#include <platform/upmu_hw.h>


void vib_turn_on(int delay_ms)
{
	if(delay_ms == 0) {
		pmic_set_register_value(PMIC_RG_VIBR_EN,0);
	}
	else if(delay_ms > 0) {
		pmic_set_register_value(PMIC_RG_VIBR_EN,1);
		dprintf(CRITICAL, "vib_turn_on, delay_ms = %d\n", delay_ms);
		mdelay(delay_ms);
		vib_turn_off();
	}
}

void vib_turn_off(void)
{
	dprintf(CRITICAL, "vib_turn_off\n");
	pmic_set_register_value(PMIC_RG_VIBR_EN,0);
}
