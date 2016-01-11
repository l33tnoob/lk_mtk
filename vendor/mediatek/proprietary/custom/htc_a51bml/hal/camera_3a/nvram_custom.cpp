#include "camera_custom_nvram.h"
#include "string.h"

int nvram_needCombineDef(int id)
{
    if(id==CAMERA_NVRAM_DATA_3A || id==CAMERA_NVRAM_DATA_STROBE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void nvram_combineStrobeNvram(NVRAM_CAMERA_STROBE_STRUCT* def, NVRAM_CAMERA_STROBE_STRUCT* nvram)
{
    memcpy(def->engTab.yTab, nvram->engTab.yTab, 40*40*sizeof(short));
}

void nvram_combine3ANvram(NVRAM_CAMERA_3A_STRUCT* def, NVRAM_CAMERA_3A_STRUCT* nvram)
{
    memcpy(&def->rFlashAWBNVRAM.rCalibrationData, &nvram->rFlashAWBNVRAM.rCalibrationData, sizeof(FLASH_AWB_CALIBRATION_DATA_STRUCT));
}





