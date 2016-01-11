#ifndef _G_SENSOR_MFG_LOG_H
#define _G_SENSOR_MFG_LOG_H

#include <utils/Log.h>

#undef LOG_TAG
#define LOG_TAG "G_SENSOR_CALIBRATION_USER"

//static int debug_flag; //0:concise, 1:verbose, 2:debug
//static int operation_mode; //0:normal, 1:simulator

#ifdef __cplusplus
extern "C" {
#endif

#define AKM_LOGD(msg, args...) if(get_debug_flag())LOGD(msg, ##args)
#define AKM_LOGE(msg, args...) if(get_debug_flag())LOGE(msg, ##args)
#define AKM_LOGI(msg, args...) if(get_debug_flag() == 2)LOGD(msg, ##args)

#ifdef __cplusplus
}
#endif

#endif
