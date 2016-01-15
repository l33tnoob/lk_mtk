#include <debug.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <kernel/timer.h>
#include <dev/keys.h>
#include <platform/mt_disp_drv.h>
#include <platform/mt_typedefs.h>

extern int target_volume_up();
extern uint32_t target_volume_down();
extern int target_power_key();

//periodic timer to poll key status
#define KEY_POLLING_INTERVAL_IN_MS 30
static timer_t key_polling_timer;
static event_t key_pressed;

static enum handler_return check_key_state(struct timer *the_timer, time_t now, void *arg)
{
	enum handler_return ret = INT_NO_RESCHEDULE;

	int vol_up, vol_down, power_key;
	//set default last_xx value to 1 to skip the initial long press (ex: press volumn-down to enter fastboot)
	static int last_vol_up = 1, last_vol_down = 1, last_power_key = 1;

	vol_up = target_volume_up();
	vol_down = target_volume_down();
	power_key = target_power_key();
	keys_post_event(KEY_VOLUMEUP, vol_up);
	keys_post_event(KEY_VOLUMEDOWN, vol_down);
	keys_post_event(KEY_POWER, power_key);

	//signal event only if the key is released last time and is pressed now
	if( ( vol_up && !last_vol_up ) ||
		( vol_down && !last_vol_down ) ||
		( power_key && !last_power_key ) )
	{
		ret = INT_RESCHEDULE;
		event_signal(&key_pressed, false);
	}

	last_vol_up    = vol_up;
	last_vol_down  = vol_down;
	last_power_key = power_key;

	return ret;
}

static int key_and_menu_handler(void *arg)
{
	simple_menu_init();

	for (;;) {
		event_wait(&key_pressed);

		if(keys_get_state(KEY_VOLUMEUP)) {
			//dprintf(INFO, "Key Pressed: up\n");
			display_goto_previous_bootmode();
		} else if(keys_get_state(KEY_VOLUMEDOWN)) {
			//dprintf(INFO, "Key Pressed: down\n");
			display_goto_next_bootmode();
		} else if(keys_get_state(KEY_POWER)) {
			//dprintf(INFO, "Key Pressed: power\n");
			display_select_current_bootmode();
		}

	}
	return 0;
}

void htc_key_and_menu_init(void)
{
	thread_t *thr = NULL;

	dprintf(CRITICAL, "zzytest, htc_key_and_menu_init begin\n");
	event_init(&key_pressed, 0, EVENT_FLAG_AUTOUNSIGNAL);
	//periodic timer to poll key status
	timer_initialize(&key_polling_timer);
	timer_set_periodic(&key_polling_timer, KEY_POLLING_INTERVAL_IN_MS, check_key_state, NULL);
	//timer_cancel(&key_polling_timer);
	thr = thread_create("key_and_menu", key_and_menu_handler, 0, DEFAULT_PRIORITY, 4096);
	if (!thr)
	{
		dprintf(CRITICAL, "%s: failed to create thread key_and_menu\r\n", __func__);
		return;
	}
	thread_resume(thr);
}

/* for ramdump: */
#ifdef MTK_BQ24296_SUPPORT
static int ramdump_chr_handler(void *arg)
{
    for(;;){
        bq24296_hw_init();
        bq24296_charging_enable(KAL_TRUE);
        msleep(1000 * 10);
    }
}
#endif
static int key_ramdump_handler(void *arg)
{
        ramdump_menu_init();

        for (;;) {
                event_wait(&key_pressed);

                if(keys_get_state(KEY_VOLUMEUP)) {
                        //dprintf(INFO, "Key Pressed: up\n");
                        display_goto_previous_bootmode();
                } else if(keys_get_state(KEY_VOLUMEDOWN)) {
                        //dprintf(INFO, "Key Pressed: down\n");
                        display_goto_next_bootmode();
                } else if(keys_get_state(KEY_POWER)) {
                        //dprintf(INFO, "Key Pressed: power\n");
                        display_select_current_bootmode();
                }

        }
        return 0;
}

void htc_ramdump_menu_init(void)
{
        thread_t *thr = NULL;

        event_init(&key_pressed, 0, EVENT_FLAG_AUTOUNSIGNAL);
        //periodic timer to poll key status
        timer_initialize(&key_polling_timer);
        timer_set_periodic(&key_polling_timer, KEY_POLLING_INTERVAL_IN_MS, check_key_state, NULL);
        //timer_cancel(&key_polling_timer);
#ifdef MTK_BQ24296_SUPPORT
        thr = thread_create("ramdump_chr", ramdump_chr_handler, 0, DEFAULT_PRIORITY, 4096);
#endif
        thr = thread_create("ramdump_menu", key_ramdump_handler, 0, DEFAULT_PRIORITY, 4096);
        if (!thr)
        {
                dprintf(CRITICAL, "%s: failed to create thread radump_menu\r\n", __func__);
                return;
        }
        thread_resume(thr);
}

