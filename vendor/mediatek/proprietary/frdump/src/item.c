/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/ 
  

#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"
 
#include "item.h"

#define TAG        "[ITEM] "
  

item_t ftm_auto_test_items[] = 
{
#ifdef FEATURE_FTM_TOUCH
	item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif

#ifdef FEATURE_FTM_LCM
    item(ITEM_LCM,     uistr_lcm),
#endif

#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
    item(ITEM_SIGNALTEST, uistr_sig_test),
#endif

#ifdef FEATURE_FTM_BATTERY
    item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif

#ifdef FEATURE_FTM_EXT_BUCK
    item(ITEM_EXT_BUCK, uistr_info_title_ext_buck_item),
#endif

#ifdef FEATURE_FTM_EXT_VBAT_BOOST
    item(ITEM_EXT_VBAT_BOOST, uistr_info_title_ext_vbat_boost_item),
#endif

#ifdef FEATURE_FTM_FLASH
   item(ITEM_FLASH,   uistr_nand_flash),
#endif

#ifdef FEATURE_FTM_RTC
    item(ITEM_RTC,     uistr_rtc),
#endif

#ifdef FEATURE_FTM_LCD
//    item(ITEM_LCD,     uistr_lcm_test),
    item(ITEM_BACKLIGHT,     uistr_backlight_level),
#endif

#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
    item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    item(ITEM_BT, uistr_bluetooth),
#endif
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
    item(ITEM_WIFI, uistr_wifi), //no uistr for wifi
#endif
#endif

#ifdef FEATURE_FTM_EMMC
    item(ITEM_EMMC,   uistr_emmc),
#endif

#ifdef FEATURE_FTM_MEMCARD
    item(ITEM_MEMCARD, uistr_memory_card),
#endif

#ifdef FEATURE_FTM_SIM
    item(ITEM_SIM, uistr_sim_detect),
#endif

#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
	item(ITEM_GPS,	   uistr_gps),
#endif
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
        item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif

#ifdef FEATURE_FTM_MAIN2_CAMERA
        item(ITEM_MAIN2_CAMERA,  uistr_main2_sensor),
#endif

#ifdef FEATURE_FTM_SUB_CAMERA
        item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif



#ifdef FEATURE_FTM_AUDIO
    item(ITEM_LOOPBACK_PHONEMICSPK,uistr_info_audio_loopback_phone_mic_speaker),
#endif
#ifdef RECEIVER_HEADSET_AUTOTEST
#ifdef FEATURE_FTM_AUDIO
    item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
#endif

#ifdef FEATURE_FTM_MATV
    //item(ITEM_MATV_NORMAL,  "MATV HW Test"),
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
#ifdef FEATURE_FTM_RF
    item(ITEM_RF_TEST,  uistr_rf_test),
#endif
    item(ITEM_MAX_IDS, NULL),
#ifdef FEATURE_FTM_HDMI
    item(ITEM_HDMI, "HDMI"),
#endif

};
item_t pc_control_items[] = 
{
    item(ITEM_FM,      "AT+FM"),
    item(ITEM_MEMCARD,      "AT+MEMCARD"),
    item(ITEM_SIM,      "AT+SIM"),
    item(ITEM_GPS,      "AT+GPS"),
    item(ITEM_EMMC,      "AT+EMMC"),
    item(ITEM_WIFI,	"AT+WIFI"),
    item(ITEM_LOOPBACK_PHONEMICSPK,      "AT+RINGTONE"),
    item(ITEM_SIGNALTEST,      "AT+SIGNALTEST"),
    item(ITEM_RTC,      "AT+RTC"),
    item(ITEM_CHARGER,      "AT+CHARGER"),
    item(ITEM_BT,      "AT+BT"),
    item(ITEM_MAIN_CAMERA, "AT+MAINCAMERA"),
    item(ITEM_SUB_CAMERA, "AT+SUBCAMERA"),
    item(ITEM_KEYS, "AT+KEY"),
    item(ITEM_MATV_AUTOSCAN, "AT+MATV"), 
   	item(ITEM_TOUCH, "AT+MTOUCH"),
    item(ITEM_TOUCH_AUTO, "AT+TOUCH"),
    #ifdef FEATURE_FTM_FLASH
    item(ITEM_CLRFLASH, "AT+FLASH"),
	#endif
	#ifdef FEATURE_FTM_EMMC
	item(ITEM_CLREMMC,"AT+FLASH"),
	#endif
    item(ITEM_VIBRATOR, "AT+VIBRATOR"),
    item(ITEM_LED, "AT+LED"),
#ifdef FEATURE_FTM_RECEIVER
    item(ITEM_RECEIVER, "AT+RECEIVER"),
#endif
    item(ITEM_HEADSET, "AT+HEADSET"),
    item(ITEM_CMMB, "AT+CMMB"),
    item(ITEM_GSENSOR, "AT+GSENSOR"),
    item(ITEM_MSENSOR, "AT+MSENSOR"),
    item(ITEM_ALSPS, "AT+ALSPS"),
    item(ITEM_GYROSCOPE, "AT+GYROSCOPE"),
    item(ITEM_IDLE, "AT+IDLE"),
    #ifdef FEATURE_FTM_LCM
    item(ITEM_LCM, "AT+LCM"),
    #endif
	  //item(ITEM_VIBRATOR_PHONE, "AT+PVIBRATOR"),
    //item(ITEM_RECEIVER_PHONE, "AT+PRECEIVER"),
    //item(ITEM_HEADSET_PHONE, "AT+PHEADSET"),
    //item(ITEM_LOOPBACK_PHONEMICSPK_PHONE, "AT+PLOOPBACK"),
    item(ITEM_MICBIAS, "AT+MICBIAS"),
#if 0
    item(ITEM_RECEIVER_FREQ_RESPONSE, "AT+RECRESPONSE"),
    item(ITEM_SPEAKER_FREQ_RESPONSE, "AT+SPKRESPONSE"),
    item(ITEM_RECEIVER_THD, "AT+RECTHD"),
    item(ITEM_SPEAKER_THD, "AT+SPKTHD"),
    item(ITEM_HEADSET_THD, "AT+HDSTHD"),
#endif
#ifdef FEATURE_FTM_HDMI
    item(ITEM_HDMI, "AT+HDMI"),
#endif
	item(ITEM_MAX_IDS, NULL),
	#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    item(ITEM_SPEAKER_MONITOR_SET_TMP,      "AT+SPKSETTMP"),
    item(ITEM_SPEAKER_MONITOR,      "AT+SPKMNTR"),
#endif
};
item_t ftm_debug_test_items[] = 
{
#ifdef FEATURE_FTM_AUDIO
		item(ITEM_RECEIVER_DEBUG, uistr_info_audio_receiver_debug),
#endif
	item(ITEM_MAX_IDS, NULL),
};

item_t ftm_test_items[] = 
{
    item(ITEM_RAMDUMP, uistr_ramdump),
    item(ITEM_MAX_IDS, NULL),
};

item_t ftm_cust_items[ITEM_MAX_IDS];
item_t ftm_cust_auto_items[ITEM_MAX_IDS];
 

item_t *get_item_list(void)
{
	item_t *items;

	LOGD(TAG "get_item_list");

	items = ftm_test_items;

	return items;
}

item_t *get_debug_item_list(void)
{
	item_t *items;

	LOGD(TAG "get_debug_item_list");

	items = ftm_debug_test_items;

	return items;
}

item_t *get_manual_item_list(void)
{
	item_t *items;
	item_t *items_auto;
	int i = 0;
	int j =0;
	LOGD(TAG "get_manual_item_list");

	items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

	items_auto = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;

	while (items_auto[i].name != NULL)
	{
		for(j =0;items[j].name != NULL ;j++)
		{
			if(strcmp(items[j].name,items_auto[i].name)==0)
			{
				items[j].mode = FTM_AUTO_ITEM;
				LOGD(TAG "%s",items[j].name);
			}
		}
		i++;
	}

	return items;
}

item_t *get_auto_item_list(void)
{
	item_t *items;

	//items = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;
	items = ftm_cust_auto_items;

	return items;
}

const char *get_item_name(item_t *item, int id)
{
	int i = 0;

    if(item == NULL)
    {
        return NULL;
    }

	while (item->name != NULL) 
    {
		if (item->id == id)
			return item->name;
		item++;
	}
	return NULL;
}

int get_item_id(item_t *item, char *name)
{
	int i = 0;

    if((item == NULL) || (name == NULL))
    {
        return -1;
    }

	while (item->name != NULL)
	{
		if(strlen(item->name)==strlen(name))
		{
			if (!strncasecmp(item->name, name, strlen(item->name)))
				return item->id;
		}
		item++;
	}
	return -1;
}

int get_item_test_type(item_t *item, char *name)
{
    if((item == NULL) || (name == NULL))
    {
        return -1;
    }

    while (item->name != NULL)
	{
		if(strlen(item->name)==strlen(name))
		{
        	if (!strncasecmp(item->name, name, strlen(item->name)))
            {
                return item->mode;
            }
		}
        item++;
    }
    return -1;
}
