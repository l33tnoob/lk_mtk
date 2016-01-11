#include <stdlib.h>
#include <cutils/properties.h>
#include "camera_custom_types.h"
#include <aaa_log.h>
#include "libnvram.h"
#include "Custom_NvRam_LID.h"
#include "htc_camera_mfg1.h"
#include "camera_custom_nvram.h"

#undef LOG_TAG
#define LOG_TAG "htc_camcal"

#define INVALID_HANDLE_VALUE (-1)

static int main2_i4BVOffset = 0;
static int main2_awb_version = 0;
static int awb_main2_status = 0;

#define CAL_VALIT_BIT 0X01
#define CAL_CHECKSUM  0x02
#define CAL_VERSION   0x04
#define CAL_FUSEID    0x08

#define LSC_CONFIDENCE_LEVEL  (CAL_VALIT_BIT + CAL_CHECKSUM)
#define AWB_CONFIDENCE_LEVEL  (CAL_VALIT_BIT + CAL_CHECKSUM)


uint32_t Crc32CheckSumByte(uint8_t* pData, uint32_t uiLen, uint32_t preValue)
{
	const uint32_t crc32table[256] = {
		/* 0x00 */ 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
		/* 0x04 */ 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		/* 0x08 */ 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
		/* 0x0C */ 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		/* 0x10 */ 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
		/* 0x14 */ 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		/* 0x18 */ 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
		/* 0x1C */ 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		/* 0x20 */ 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
		/* 0x24 */ 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		/* 0x28 */ 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
		/* 0x2C */ 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		/* 0x30 */ 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
		/* 0x34 */ 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		/* 0x38 */ 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
		/* 0x3C */ 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		/* 0x40 */ 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
		/* 0x44 */ 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		/* 0x48 */ 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
		/* 0x4C */ 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		/* 0x50 */ 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
		/* 0x54 */ 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		/* 0x58 */ 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
		/* 0x5C */ 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		/* 0x60 */ 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
		/* 0x64 */ 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		/* 0x68 */ 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
		/* 0x6C */ 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		/* 0x70 */ 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
		/* 0x74 */ 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		/* 0x78 */ 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
		/* 0x7C */ 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		/* 0x80 */ 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
		/* 0x84 */ 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		/* 0x88 */ 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
		/* 0x8C */ 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		/* 0x90 */ 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
		/* 0x94 */ 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		/* 0x98 */ 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
		/* 0x9C */ 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		/* 0xA0 */ 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
		/* 0xA4 */ 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		/* 0xA8 */ 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
		/* 0xAC */ 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		/* 0xB0 */ 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
		/* 0xB4 */ 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		/* 0xB8 */ 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
		/* 0xBC */ 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		/* 0xC0 */ 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
		/* 0xC4 */ 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		/* 0xC8 */ 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
		/* 0xCC */ 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		/* 0xD0 */ 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
		/* 0xD4 */ 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		/* 0xD8 */ 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
		/* 0xDC */ 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		/* 0xE0 */ 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
		/* 0xE4 */ 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		/* 0xE8 */ 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
		/* 0xEC */ 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		/* 0xF0 */ 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
		/* 0xF4 */ 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		/* 0xF8 */ 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
		/* 0xFC */ 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,};
	uint32_t i,CheckSum,cvalue;
	CheckSum = preValue;
	for (i = 0; i < uiLen; i++){
		cvalue = *pData;
		CheckSum = (CheckSum>>8) ^ crc32table[(CheckSum & 0xFF) ^ (cvalue & 0xFF)];
		pData ++;
	}
	return CheckSum;

}

int check_mfg1_Data(void* buf, int id, int dev)
{
	uint32_t checksum_lsc =0;
	uint32_t checksum_awb =0;
    int lsc_status = 0 ,awb_status = 0;
	HTC_CAMERA_MFG1_STRUCT *mfg1_data = (HTC_CAMERA_MFG1_STRUCT *)buf;

	if (id !=AP_CFG_RDCL_HTC_CAMERA_MFG1_LID){
		MY_LOG("skip id %d != %d ",id, AP_CFG_RDCL_HTC_CAMERA_MFG1_LID);
	}

	if (mfg1_data->LSC_DATA.tag == 0x3099)
		lsc_status = lsc_status | 0x01;

	checksum_lsc = Crc32CheckSumByte((uint8_t*)&mfg1_data->LSC_DATA, (sizeof(mfg1_data->LSC_DATA)- 4), 0);

	if (mfg1_data->LSC_DATA.checksum == checksum_lsc)
		lsc_status = lsc_status | 0x02;


	if (mfg1_data->AWB_DATA.tag == 0x3099)
		awb_status = awb_status | 0x01;

	checksum_awb = Crc32CheckSumByte((uint8_t*)&mfg1_data->AWB_DATA, (sizeof(mfg1_data->AWB_DATA)- 4), 0);

	if (mfg1_data->AWB_DATA.checksum == checksum_awb)
		awb_status = awb_status | 0x02;


    if(dev == 4){
        awb_main2_status  = awb_status ;
    }

    if(lsc_status != 3){
        ALOGE("dev %d lsc status check failed ",dev);
        return -1;
    }

    if(awb_status != 3){
        ALOGE("dev %d awb status check failed ",dev);
        return -1;
    }

	if (lsc_status&CAL_VALIT_BIT){
		if (dev == 1)
			property_set("debug.camcal.lsc.main", "ok");
		else if (dev == 2)
			property_set("debug.camcal.lsc.front", "ok");
		else if (dev == 4)
			property_set("debug.camcal.lsc.main2", "ok");
	}

	if (awb_status&CAL_VALIT_BIT){
		if (dev == 1)
			property_set("debug.camcal.awb.main", "ok");
		else if (dev == 2)
			property_set("debug.camcal.awb.front", "ok");
		else if (dev == 4)
			property_set("debug.camcal.awb.main2", "ok");
	}

	MY_LOG("dev %d lsc_status %x %x %x %x ",dev, lsc_status, mfg1_data->LSC_DATA.tag, mfg1_data->LSC_DATA.checksum, checksum_lsc);
	MY_LOG("dev %d awb_status %x %x %x %x ",dev, awb_status, mfg1_data->AWB_DATA.tag, mfg1_data->AWB_DATA.checksum, checksum_awb);

	if ((dev==4) &&(awb_status&CAL_VALIT_BIT)&&(mfg1_data->AWB_DATA.version>=1)){
		main2_i4BVOffset = mfg1_data->AWB_DATA.i4BVOffset;
		main2_awb_version = mfg1_data->AWB_DATA.version;
	}
	return 0;
}

int readNvramReal(void* buf, int id, int dev)
{
	F_ID rNvramFileID;
	int i4RecSize;
	int i4RecNum;


	rNvramFileID = NVM_GetFileDesc(id, &i4RecSize, &i4RecNum, ISREAD);
	if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
		MY_LOG("readNvramData(): create NVRAM file fail\n");
		return -1; //NVRAM_CAMERA_FILE_ERROR;
	}

	if (dev == 4 /*DUAL_CAMERA_MAIN_SECOND_SENSOR*/) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
	}

	if (dev == 2 /*DUAL_CAMERA_SUB_SENSOR*/) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
	}

	read(rNvramFileID.iFileDesc, buf, i4RecSize);

	NVM_CloseFileDesc(rNvramFileID);

	MY_LOG("readNvramReal %p id %d dev %d i4RecSize %d", buf, id, dev, i4RecSize);
    return check_mfg1_Data(buf,id,dev);
}

MINT32 import_HTC_MFG1_LscData(void *pDst, MUINT32 u4SensorDev)
{
	int ret;
	HTC_CAMERA_MFG1_STRUCT mfg1_data;
	CAM_CAL_DATA_STRUCT* pCalData = (CAM_CAL_DATA_STRUCT*)pDst;

	ret = readNvramReal(&mfg1_data,  AP_CFG_RDCL_HTC_CAMERA_MFG1_LID, u4SensorDev);

	if(ret <0){
		MY_LOG("no lsc calibration data dev %d", u4SensorDev);
		return -1;
	}


	pCalData->Command = CAMERA_CAM_CAL_DATA_SHADING_TABLE;
	pCalData->DataVer = CAM_CAL_SINGLE_EEPROM_DATA;
	::memcpy(&pCalData->SingleLsc.LscTable.MtkLcsData, &mfg1_data.LSC_DATA.MtkLcsData,  sizeof(mfg1_data.LSC_DATA.MtkLcsData));
	MY_LOG("addr %p dev %d size %d", &mfg1_data.LSC_DATA.MtkLcsData, u4SensorDev, sizeof(mfg1_data.LSC_DATA.MtkLcsData));

	return 0;
}


int import_HTC_MFG1_AwbData(AWB_GAIN_T *rUnitGain, MUINT32 u4SensorDev)
{
	int ret;
	HTC_CAMERA_MFG1_STRUCT mfg1_data;

	ret = readNvramReal(&mfg1_data,  AP_CFG_RDCL_HTC_CAMERA_MFG1_LID, u4SensorDev);
	if ((ret <0)){
		MY_LOG("no awb calibration data dev %d ", u4SensorDev);
		return 0;
	}

	rUnitGain->i4R = mfg1_data.AWB_DATA.Gain_R;
	rUnitGain->i4G = mfg1_data.AWB_DATA.Gain_G;
	rUnitGain->i4B = mfg1_data.AWB_DATA.Gain_B;
	MY_LOG("R: %x G: %x B: %x", mfg1_data.AWB_DATA.Gain_R, mfg1_data.AWB_DATA.Gain_G, mfg1_data.AWB_DATA.Gain_B);
	return 0;
}

int import_HTC_MFG1_AEC_Main2BV_diff(MUINT32 u4SensorDev, INT32 *i4BVOffset)
{
	int ret;
	char value[PROPERTY_VALUE_MAX] = {'\0'};
	char property_value[PROPERTY_VALUE_MAX];
	int skip_calibration_data = 0;
	HTC_CAMERA_MFG1_STRUCT mfg1_data;

	property_get("debug.camcal.aec.disable", value, "0");
	skip_calibration_data = atoi(value);

	if (skip_calibration_data){ //this is to support aec calibration
		//MY_LOG("i4BVOffset %d dev %d", *i4BVOffset, u4SensorDev);
		sprintf(property_value, "%d", *i4BVOffset);
		property_set("debug.camcal.aec.bvoffset", property_value);
		return 0;
	}

	if (u4SensorDev != 4){
		return 0;
	}

	if (((awb_main2_status&CAL_VALIT_BIT) ==0)||(main2_awb_version < 1)){
		MY_LOG("dev %d BV %d no calibration awb_status %d version %d",u4SensorDev, *i4BVOffset, awb_main2_status, main2_awb_version);
		return 0;
	}

	if (main2_i4BVOffset!= *i4BVOffset) {
		MY_LOG("i4BVOffset old %d  new %d dev %d", *i4BVOffset,main2_i4BVOffset,  u4SensorDev);
		*i4BVOffset = main2_i4BVOffset;
	}

	property_get("debug.camcal.aec.forced", value, "0");
	ret = atoi(value);
	if (ret){
		*i4BVOffset = ret;
		MY_LOG("forced main2 BV golden %d ", *i4BVOffset);
	}
	return 0;

	return ret;
}

