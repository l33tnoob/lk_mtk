/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Copyright (c) 2008 High Tech Computer Corporation
Module Name:
    bma150_usr_calibration  - G-sensor calibration tool
Abstract:
Auther:
    viral wang 	Dec, 2008
Notes:
------------------------------------------------------------------*/
#include "FileIO.h"
#include "IOMessage.h"
#include "g_sensor_USER_log.h"
#include <utils/Log.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#ifndef LOGD
#define LOGE ALOGE
#define LOGD ALOGD
#define LOGV ALOGV
#endif

/*< Separate the name of parameter and value in setting file.*/
#define DELIMITER "="

//#define DEBUG
#define REAL_WRITE

/* mode settings */
#define BMA_MODE_NORMAL		0
#define BMA_MODE_SLEEP		1

#define GSR_DEVICE_NAME	"/dev/bma150"
#define ECS_DEVICE_NAME	"/dev/akm8973_daemon"
#define RESULT_FILE	"/data/misc/bma_result.txt"
//#define PROJECT_NAME	"Calibration USER 3.0"
//#define PROJECT_NAME	"Calibration USER 4.0" // Add support for BMA2X2 G-Sensor calibration
//#define PROJECT_NAME	"Calibration USER 4.1" // Do not let G-Sensor go to suspend in the end
//#define PROJECT_NAME	"Calibration USER 4.2" // Use snprintf for safety
//#define PROJECT_NAME    "Calibration USER 4.3" // Add support for Sensor HUB G-sensor calibration
//#define PROJECT_NAME    "Calibration USER 5.0" // Add support for Kionix KXTJ2 G-sensor calibration
//#define PROJECT_NAME "Calibration USER 5.1" // Support bma2x2 for LC USER
#define PROJECT_NAME "Calibration USER 5.2" // Support bma253


#define RETRY_TIMES              (3)
#define TEST_TIMES              (20)
#define ERROR_TOLERANCE         (15)
#define ERROR_TOLERANCE_BMA2X2  (60)
#define ERROR_TOLERANCE_LSM330  (60)
#define ERROR_TOLERANCE_LC_BMA2X2  (60)
#define CALIBRATE_THRESHOLD     (41)
#define CALIBRATE_THRESHOLD_Z   (49)
#define GRANULARITY              (1)

#define BMA250_BOSCH_ENABLE "/sys/class/htc_g_sensor/g_sensor/enable"
#define BMA250_BOSCH_GET_DATA "/sys/class/htc_g_sensor/g_sensor/get_raw_data"
#define BMA250_BOSCH_CHIP_LAYOUT "/sys/class/htc_g_sensor/g_sensor/chip_layout"
#define BMA250_BOSCH_SET_K_VALUE "/sys/class/htc_g_sensor/g_sensor/set_k_value"

#define BMA2X2_BOSCH_MODE "/sys/class/sensor_acc/bma/input/mode"
#define BMA2X2_BOSCH_GET_DATA "/sys/class/sensor_acc/bma/input/get_raw_data"
#define BMA2X2_BOSCH_CHIP_LAYOUT "/sys/class/sensor_acc/bma/input/chip_layout"
#define BMA2X2_BOSCH_SET_K_VALUE "/sys/class/sensor_acc/bma/input/set_k_value"

#define LSM330_ENABLE "/sys/class/ST_acc/acc/enable_device"
#define LSM330_GET_DATA "/sys/class/ST_acc/acc/get_raw_data"
#define LSM330_CHIP_LAYOUT "/sys/class/ST_acc/acc/chip_layout"
#define LSM330_SET_K_VALUE "/sys/class/ST_acc/acc/set_k_value"

#define HTC_SENSOR_HUB_ENABLE "/sys/class/htc_sensorhub/sensor_hub/enable"
#define HTC_SENSOR_HUB_CALIBRATION "/sys/class/htc_sensorhub/sensor_hub/calibrator_en"
#define HTC_SENSOR_HUB_GET_DATA "/sys/class/htc_sensorhub/sensor_hub/calibrator_data_acc"
#define HTC_SENSOR_HUB_SET_K_VALUE "/sys/class/htc_sensorhub/sensor_hub/calibrator_data_acc"

#define LC_BMA2X2_ENABLE "/sys/class/htc_g_sensor/bma2x2/enable"
#define LC_BMA2X2_GET_DATA "/sys/class/htc_g_sensor/bma2x2/value"
#define LC_BMA2X2_SET_K_VALUE "/sys/class/htc_g_sensor/bma2x2/set_k_value"
#define LC_BMA2X2_SET_K_VALUE_MSB "/sys/class/htc_g_sensor/bma2x2/set_k_value_msb"


#define KXTJ2_ENABLE "sys/class/htc_g_sensor/kxtj2/iio/enable"
const char *kxtj2_get_data[3] = {"/sys/class/htc_g_sensor/kxtj2/iio/in_accel_x_raw"
				 , "/sys/class/htc_g_sensor/kxtj2/iio/in_accel_y_raw"
				 , "/sys/class/htc_g_sensor/kxtj2/iio/in_accel_z_raw"
				};
const char *kxtj2_k_value[3] = {"/sys/class/htc_g_sensor/kxtj2/iio/in_accel_x_calibbias"
				, "/sys/class/htc_g_sensor/kxtj2/iio/in_accel_y_calibbias"
				, "/sys/class/htc_g_sensor/kxtj2/iio/in_accel_z_calibbias"
			       };
#define KXTJ2_CONVERT_TO_ADC    (256.0/9806550.0)

#define BMA253_BOSCH_ENABLE "/sys/class/htc_g_sensor/bma253/enable"
#define BMA253_BOSCH_GET_DATA "/sys/class/htc_g_sensor/bma253/get_raw_data"
#define BMA253_BOSCH_CHIP_LAYOUT "/sys/class/htc_g_sensor/bma253/chip_layout"
#define BMA253_BOSCH_SET_K_VALUE "/sys/class/htc_g_sensor/bma253/set_k_value"

int save_offset_mcu(int *str){
	FILE *fp_file;
	int i = 0;
	int data[3]={0};
	if (access("/data/misc/AccOffset.txt", R_OK|W_OK) != 0) {
		fp_file = fopen("/data/misc/AccOffset.txt", "w+");
		if (fp_file == NULL) {
			ALOGE("Unable to open %s (%s)", "/data/misc/AccOffset.txt", strerror(errno));
			return -1;
		}
		rewind(fp_file);
	}else{
		fp_file = fopen("/data/misc/AccOffset.txt", "w+");//setting file
		if(!fp_file) {
			ALOGE("open file '%s' failed: %s\n", "/data/system/cyweeLOG.ini", strerror(errno));
			return -errno;
		}
	}

	for(i = 0;i<3;i++){
		data[i] = str[i];
	}
	fprintf(fp_file, "%d %d %d\n", data[0], data[1], data[2]);

	fflush(fp_file);
	fclose(fp_file);
	return 0;
}

int Read_calibrator(int *str){
	FILE *fp;
	int readBytes = 0;
	int data[3]={0};
	int i = 0;
	/*wait firmware calibration ready*/
	usleep(3000000);
	fp = fopen(HTC_SENSOR_HUB_GET_DATA, "r");
	if(!fp) {
			ALOGE("open File failed: %s\n", strerror(errno));
			return 0;
		}
	rewind(fp);

	readBytes = fscanf(fp, "%d %d %d\n", &data[0], &data[1], &data[2]);
	for(i = 0;i<3;i++){
		str[i] = data[i];
        }

	fclose(fp);
	if (readBytes <= 0) {
		ALOGE("%s\n", strerror(errno));
		return readBytes;
	}

        return  0;
}
int save_offset_bosch_bma250(int16vec *Aoff)
{
	FILE *fp;

	fp = fopen("/data/misc/AccOffset.txt", "w");
	if (fp == NULL) {
		LOGE("AccOffset.txt, open file fail\n ");
		return 0;
	}

	fprintf(fp, "%s.x %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.x);
	fprintf(fp, "%s.y %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.y);
	fprintf(fp, "%s.z %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.z);

	fclose(fp);
	return 1;
}

int save_offset_bosch_bma253(int16vec *Aoff)
{
	FILE *fp;

	fp = fopen("/data/misc/AccOffset.txt", "w");
	if (fp == NULL) {
		LOGE("AccOffset.txt, open file fail\n ");
		return 0;
	}

	fprintf(fp, "%s.x %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.x);
	fprintf(fp, "%s.y %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.y);
	fprintf(fp, "%s.z %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.z);

	fclose(fp);
	return 1;
}


int saveAO_ACC_AK8975(int16vec *Aoff)
{
	FILE *fp;

	fp = fopen("/data/misc/AccPrmsF.ini", "w");
	if (fp == NULL) {
		LOGE("saveAO_ACC_AK8975, open file fail\n ");
		return 0;
	}

	fprintf(fp, "%s.x %s %f\n", "AS", DELIMITER, (float)256);
	fprintf(fp, "%s.y %s %f\n", "AS", DELIMITER, (float)256);
	fprintf(fp, "%s.z %s %f\n", "AS", DELIMITER, (float)256);
	fprintf(fp, "%s.x %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.x);
	fprintf(fp, "%s.y %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.y);
	fprintf(fp, "%s.z %s %f\n", "AO", DELIMITER, (float)(*Aoff).u.z);

	fclose(fp);
	return 1;
}

int enableDevice(unsigned char value)
{
	int ret_val = -1;
	int fd;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);
	fd = open(BMA250_BOSCH_ENABLE, O_RDWR);
	ret_val = write(fd, buf, sizeof(buf));
	close(fd);
	return ret_val;
}

int enableBMA253Device(unsigned char value)
{
	int ret_val = -1;
	int fd;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);
	fd = open(BMA253_BOSCH_ENABLE, O_RDWR);
	ret_val = write(fd, buf, sizeof(buf));
	close(fd);
	return ret_val;
}


int enableBma2x2Device(unsigned char value)
{
	int ret_val = -1;
	int fd;
	char buf[128] = "";
	unsigned char mode = 2;

	if (value == 1)
		mode = 0;
	else
		mode = 2;

	snprintf(buf, 128, "%d", mode);
	fd = open(BMA2X2_BOSCH_MODE, O_RDWR);
	ret_val = write(fd, buf, sizeof(buf));
	close(fd);
	return ret_val;
}

int enableLSM330Device(unsigned char value)
{
	int ret_val = -1;
	int fd;
	char buf[128];

	snprintf(buf, 128, "%d", value);
	fd = open(LSM330_ENABLE, O_RDWR);
	ret_val = write(fd, buf, sizeof(buf));
	close(fd);
	return ret_val;
}

void enableKXTJ2Device(unsigned char value)
{
	int ret_val = -1;
	int fd;
	char buf[8];

	ret_val = snprintf(buf, sizeof(buf), "%d", !!value);
	if (ret_val <= 0) {
		ALOGE("%s: snprintf fail, ret_val = %d\n", __func__, ret_val);
		return;
	}

	fd = open(KXTJ2_ENABLE, O_RDWR);
	if (fd >= 0) {
		ret_val = write(fd, buf, sizeof(buf));
		if (ret_val < 0) {
			ALOGE("%s: write fail, ret_val = %d, strerr = %s\n",
			      __func__, ret_val, strerror(errno));
		}
		close(fd);
	} else {
		ALOGE("%s: open fail, fd = %d, strerr = %s\n", __func__, fd, strerror(errno));
	}
}

int enableMCUDevice(unsigned char value)
{
        int ret_val = -1;
        int fd;
        char buf[128];

        snprintf(buf, 128, "%d", value);
        fd = open(HTC_SENSOR_HUB_ENABLE, O_RDWR);
        ret_val = write(fd, buf, sizeof(buf));
        close(fd);
        return ret_val;
}

int DoMCUCalibration(unsigned char value)
{
        int ret_val = -1;
        int fd;
        char buf[128];

        snprintf(buf, 128, "%d", value);
        fd = open(HTC_SENSOR_HUB_CALIBRATION, O_RDWR);
        ret_val = write(fd, buf, sizeof(buf));
        close(fd);
        return ret_val;
}

int get_chip_layout(void)
{
	unsigned char par;
	int ret_val = -1;
	int fd;
	char buf[128] = "";
	int len = 0;

	fd = open(BMA250_BOSCH_CHIP_LAYOUT, O_RDONLY);
	ret_val = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGD("chip_layout buf = %s\n", buf);
	len = strlen(buf);
	LOGD("buf[%d] = %c\n", (len - 2), buf[len-2]);

	if (buf[len-2] == '1')
		ret_val = 0;
	else
		ret_val = 1;

	return ret_val;
}

int get_bma253_chip_layout(void)
{
	unsigned char par;
	int ret_val = -1;
	int fd;
	char buf[128] = "";
	int len = 0;

	fd = open(BMA253_BOSCH_CHIP_LAYOUT, O_RDONLY);
	ret_val = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGD("chip_layout buf = %s\n", buf);
	len = strlen(buf);
	LOGD("buf[%d] = %c\n", (len - 2), buf[len-2]);

	if (buf[len-2] == '1')
		ret_val = 0;
	else
		ret_val = 1;

	return ret_val;
}

int get_bma2x2_chip_layout(void)
{
	unsigned char par;
	int ret_val = -1;
	int fd;
	char buf[128] = "";
	int len = 0;

	fd = open(BMA2X2_BOSCH_CHIP_LAYOUT, O_RDONLY);
	ret_val = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGD("chip_layout buf = %s\n", buf);
	len = strlen(buf);
	LOGD("buf[%d] = %c\n", (len - 2), buf[len-2]);

	if (buf[len-2] == '1')
		ret_val = 1;
	else
		ret_val = 0;

	return ret_val;
}

int get_lsm330_chip_layout(void)
{
	unsigned char par;
	int ret_val = -1;
	int fd;
	char buf[128] = "";
	int len = 0;

	fd = open(LSM330_CHIP_LAYOUT, O_RDONLY);
	ret_val = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGD("chip_layout buf = %s\n", buf);
	len = strlen(buf);
	LOGD("buf[%d] = %c\n", (len - 2), buf[len-2]);

	if (buf[len-2] == '1')
		ret_val = 1;
	else
		ret_val = 0;

	return ret_val;
}

int get_bma250_bosch_data(int16 data[3])
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";
	int int_data[3] = {0};

	fd = open(BMA250_BOSCH_GET_DATA, O_RDONLY);
	ret = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGV("%s: buf = \"%s\"\n", __PRETTY_FUNCTION__, buf);
	sscanf(buf, "x = %d, y = %d, z = %d", &int_data[0], &int_data[1], &int_data[2]);

	data[0] = int_data[0];
	data[1] = int_data[1];
	data[2] = int_data[2];

	return ret;
}

int get_bma253_bosch_data(int16 data[3])
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";
	int int_data[3] = {0};

	fd = open(BMA253_BOSCH_GET_DATA, O_RDONLY);
	ret = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGV("%s: buf = \"%s\"\n", __PRETTY_FUNCTION__, buf);
	sscanf(buf, "x = %d, y = %d, z = %d", &int_data[0], &int_data[1], &int_data[2]);

	data[0] = (int_data[0]/4);
	data[1] = (int_data[1]/4);
	data[2] = (int_data[2]/4);

	return ret;
}

int get_bma2x2_bosch_data(int16 data[3])
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";
	int int_data[3] = {0};

	fd = open(BMA2X2_BOSCH_GET_DATA, O_RDONLY);
	ret = read(fd, buf, (sizeof(buf) - 1));
	close(fd);

	LOGV("%s: buf = \"%s\"\n", __PRETTY_FUNCTION__, buf);
	sscanf(buf, "x = %d, y = %d, z = %d", &int_data[0], &int_data[1], &int_data[2]);

	data[0] = int_data[0];
	data[1] = int_data[1];
	data[2] = int_data[2];

	return ret;
}
int get_lsm330_data(int16 data[3])
{
        int ret = -1;
        int fd = 0;
        char buf[128] = "";
        int int_data[3] = {0};

        fd = open(LSM330_GET_DATA, O_RDONLY);
        ret = read(fd, buf, (sizeof(buf) - 1));
        close(fd);

        LOGV("%s: buf = \"%s\"\n", __PRETTY_FUNCTION__, buf);
        sscanf(buf, "x = %d, y = %d, z = %d", &int_data[0], &int_data[1], &int_data[2]);

        data[0] = int_data[0]/1000;//unit convert to mili G
        data[1] = int_data[1]/1000;
        data[2] = int_data[2]/1000;

        return ret;
}

int get_lc_bma2x2_data(int16 data[3])
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";
	int int_data[3] = {0};

	fd = open(LC_BMA2X2_GET_DATA, O_RDONLY);
	ret = read(fd, buf, (sizeof(buf) - 1)) ;
	close(fd);

	sscanf(buf, "x = %d, y = %d, z = %d\n", &int_data[0], &int_data[1], &int_data[2]);
	data[0] = int_data[0];
	data[1] = int_data[1];
	data[2] = int_data[2];

	return ret;
}

int myIntRound(double dInput)
{
	if(dInput >= 0.0f)
		return ((int)(dInput + 0.5f));

	return ((int)(dInput - 0.5f));
}

void get_kxtj2_data(int16 data[3])
{
	int ret, i;
	int fd;
	char buf[32];
	double dou_data;

	for (i = 0; i < 3; i++) {
		fd = open(kxtj2_get_data[i], O_RDONLY);
		if (fd >= 0) {
			ret = read(fd, buf, (sizeof(buf) - 1));
			if (ret < 0) {
				ALOGE("%s: read fail, ret = %d, strerr = %s\n",
				      __func__, ret, strerror(errno));
				close(fd);
				return;
			} else {
				buf[ret-1] = '\0';
				dou_data = atoi(buf) * KXTJ2_CONVERT_TO_ADC;
				data[i] = myIntRound(dou_data);
				//ALOGV("%s: [%d]: d_ADC = %lf, i_ADC = %d\n", __PRETTY_FUNCTION__,
				//                  i, dou_data, data[i]);
			}
		} else {
			ALOGE("%s: open fail, fd = %d, strerr = %s\n",
			      __func__, fd, strerror(errno));
		}
	}

	return;
}

int set_bma250_bosch_k_value(int value)
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);

	LOGD("Write to set_k_value: 0x%x\n", value);

	fd = open(BMA250_BOSCH_SET_K_VALUE, O_RDWR);
	ret = write(fd, buf, (sizeof(buf) - 1));
	close(fd);

	return ret;
}

int set_bma253_bosch_k_value(int value)
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);

	LOGD("Write to set_k_value: 0x%x\n", value);

	fd = open(BMA253_BOSCH_SET_K_VALUE, O_RDWR);
	ret = write(fd, buf, (sizeof(buf) - 1));
	close(fd);

	return ret;
}


int set_bma2x2_bosch_k_value(int value)
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);

	LOGD("Write to set_k_value: 0x%x\n", value);

	fd = open(BMA2X2_BOSCH_SET_K_VALUE, O_RDWR);
	ret = write(fd, buf, (sizeof(buf) - 1));
	close(fd);

	return ret;
}

int set_lsm330_k_value(int value)
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);

	LOGD("Write to set_k_value: 0x%x\n", value);

	fd = open(LSM330_SET_K_VALUE, O_RDWR);
	ret = write(fd, buf, (sizeof(buf) - 1));
	close(fd);

	return ret;
}

int set_lc_bma2x2_k_value(int value)
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);

	LOGD("Write to set_k_value: 0x%x\n", value);

	fd = open(LC_BMA2X2_SET_K_VALUE, O_RDWR);
	ret = write(fd, buf, (sizeof(buf) - 1));
	close(fd);

	return ret;
}

int set_lc_bma2x2_k_value_msb(int value)
{
	int ret = -1;
	int fd = 0;
	char buf[128] = "";

	snprintf(buf, 128, "%d", value);

	LOGD("Write to set_k_value_msb: 0x%x\n", value);

	fd = open(LC_BMA2X2_SET_K_VALUE_MSB, O_RDWR);
	ret = write(fd, buf, (sizeof(buf) - 1));
	close(fd);

	return ret;
}


void set_kxtj2_k_value(int value)
{
	int ret, i;
	int fd;
	int8_t *offset = (int8_t *)&value;
	int bias;
	char buf[32];

	ALOGD("%s: value = 0x%x\n", __func__, value);

	for (i = 0; i < 3; i++) {
		//ALOGD("%s: offset[%d] = %d\n", __func__, (2-i), offset[(2-i)]);
		bias = -(offset[(2-i)] / KXTJ2_CONVERT_TO_ADC);

		ret = snprintf(buf, sizeof(buf), "%d", bias);
		if (ret <= 0) {
			ALOGE("%s: snprintf fail, ret = %d, strerr = %s\n",
			      __func__, ret, strerror(errno));
			return;
		}

		//ALOGD("Write k_value [%d] to [%s], ret = %d\n", bias, kxtj2_k_value[i], ret);
		fd = open(kxtj2_k_value[i], O_RDWR);
		if (fd >= 0) {
			ret = write(fd, buf, ret);
			if (ret < 0) {
				ALOGE("%s: write fail, ret = %d, i = %d, strerr = %s\n",
				      __func__, ret, i, strerror(errno));
			}
			close(fd);
		} else {
			ALOGE("%s: open fail, fd = %d, i = %d, strerr = %s\n",
			      __func__, fd, i, strerror(errno));
		}
	}
}

int main(int argc, char **argv)
{
	int16vec tmpA, tmpB, Aoff;
	int      m_aDevice, retry_times = 1, fd;
	int16    i2cData;
	char	  string[64];
	int      i;
	int   temp_data[3]={0};
	char   ver[64] = "";
	short  chip_layout = -1;
	struct stat statbuf;
	int 	rc = 0;
	signed char cali_values[3] = {0, 0, 0};
	int16  offset[3] = {0, 0, 0};
	FILE *f = NULL;
	int is_bosch_bma250_driver = 0;
	int is_bosch_bma2x2_driver = 0;
	int is_bosch_bma253_driver = 0;
	int is_lsm330_driver = 0;
	int is_htc_sensor_hub = 0;
	int k_value = 0;
	int is_lc_bma2x2_driver = 0;
	int is_kxtj2_driver = 0;

	strncat (ver, PROJECT_NAME, 63);
	LOGD("%s\n", ver);
	// Check if Bosch bam250 driver is probed
	f = fopen(BMA250_BOSCH_ENABLE, "r");
	if (f) {
		fclose(f);
		LOGD( "BOSCH bma250 driver\n");
		is_bosch_bma250_driver = 1;
	} else {
		LOGD( "Not BOSCH bma250 driver");
		is_bosch_bma250_driver = 0;
	}

	// Check if Bosch bam2X2 driver is probed
	f = fopen(BMA2X2_BOSCH_MODE, "r");
	if (f) {
		fclose(f);
		LOGD( "BOSCH bma2x2 driver\n");
		is_bosch_bma2x2_driver = 1;
	} else {
		LOGD( "Not BOSCH bma2x2 driver");
		is_bosch_bma2x2_driver = 0;
	}

	// Check if LSM330 driver is probed
	f = fopen(LSM330_ENABLE, "r");
	if (f) {
		fclose(f);
		LOGD( "LSM330 driver\n");
		is_lsm330_driver = 1;
	} else {
		LOGD( "Not LSM330 driver");
		is_lsm330_driver = 0;
	}
        // Check if LSM330 driver is probed
        f = fopen(HTC_SENSOR_HUB_ENABLE, "r");
        if (f) {
                fclose(f);
                LOGD( "MCU driver\n");
                is_htc_sensor_hub = 1;
        } else {
                LOGD( "Not MCU driver");
                is_htc_sensor_hub = 0;
        }

	// Check if LC bma2x2 driver is probed
	f = fopen(LC_BMA2X2_ENABLE, "r");
	if (f) {
		fclose(f);
		LOGD("LC bma2x2 driver\n");
		is_lc_bma2x2_driver = 1;
	} else {
		LOGD("Not LC bma2x2 driver\n");
		is_lc_bma2x2_driver = 0;
	}

	// Check if KXTJ2 driver is probed
	f = fopen(KXTJ2_ENABLE, "r");
	if (f) {
		fclose(f);
		LOGD( "KXTJ2 driver\n");
		is_kxtj2_driver = 1;
	} else {
		LOGD( "Not KXTJ2 driver");
		is_kxtj2_driver = 0;
	}

	// Check if Bosch bam253 driver is probed
	f = fopen(BMA253_BOSCH_ENABLE, "r");
	if (f) {
		fclose(f);
		LOGD( "BOSCH bma253 driver\n");
		is_bosch_bma253_driver = 1;
	} else {
		LOGD( "Not BOSCH bma253 driver");
		is_bosch_bma253_driver = 0;
	}

	if (is_bosch_bma250_driver) {
		// Enable G-Sensor and get chip layout
		enableDevice(1);
		chip_layout = get_chip_layout();
	} else if (is_bosch_bma2x2_driver) {
		// Enable G-Sensor and get chip layout
		enableBma2x2Device(1);
		chip_layout = get_bma2x2_chip_layout();
	} else if (is_lsm330_driver) {
		// Enable G-Sensor and get chip layout
		enableLSM330Device(1);
		chip_layout = get_lsm330_chip_layout();
	} else if (is_htc_sensor_hub){
		enableMCUDevice(1);
		chip_layout = 0;
	} else if (is_lc_bma2x2_driver) {
		chip_layout = 0;
	} else if (is_bosch_bma253_driver) {
		// Enable G-Sensor and get chip layout
		enableBMA253Device(1);
		chip_layout = get_bma253_chip_layout();
	} else if (is_kxtj2_driver){
		enableKXTJ2Device(1);
		chip_layout = 0;
	} else {
		// Open serial communication device.
		m_aDevice=open(GSR_DEVICE_NAME,0444);
		while(m_aDevice<0){
			LOGE("Open m_aDevice ERROR, m_aDevice= %d\n", m_aDevice);
			rc = 1;
			goto end;
		}

		/* Get G-Sensor chip layout */
		if (!SendIOMessage(m_aDevice,
				BMA_GET_CHIP_LAYOUT, NULL, &chip_layout))
			LOGE("layouts retrieve error!!");
	}
	LOGD("%s: chip_layout = %d\n",
		__PRETTY_FUNCTION__, chip_layout);

	// Create a file to save result
	fd = open (RESULT_FILE, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if (fd == -1) {
		LOGE("bmafct: create /data/misc/bma_result.txt fail \n");
		rc = 2;
		goto end;
	}

	/* Unexpected values */
	if (chip_layout != 0 && chip_layout != 1) {
		/* show result */
		snprintf(string, 64, "Unknown G-Sensor chip layout!!\n");
		LOGE("%s", string);
		write(fd, string, strlen(string));
		rc = 8;
		goto end;
	}

	if ((!is_bosch_bma250_driver)
	    && (!is_bosch_bma2x2_driver)
	    && (!is_lsm330_driver)
	    && (!is_htc_sensor_hub)
		&& (!is_lc_bma2x2_driver)
		&& (!is_bosch_bma253_driver)
	    && (!is_kxtj2_driver)) {
		/* Get the calibration data */
		if (!SendIOMessage(m_aDevice, BMA_READ_CALI_VALUE, NULL, cali_values)) {
			LOGE("Get calibration data ERROR!!\n");
			offset[0] = 0;
			offset[1] = 0;
			offset[2] = 0;
		} else {
			offset[0] = cali_values[0];
			offset[1] = cali_values[1];
			offset[2] = cali_values[2];
		}

		LOGV("offset[0], (offset_x) = %d\n", offset[0]);
		LOGV("offset[1], (offset_y) = %d\n", offset[1]);
		LOGV("offset[2], (offset_z) = %d\n", offset[2]);

		if (!SendIOMessage(m_aDevice, BMA_SET_CALI_MODE, 1, NULL))
			LOGE("set calibration mode to 1 fail!!\n");

		if (!SendIOMessage(m_aDevice, BMA_SET_MODE, BMA_MODE_NORMAL, NULL))
			LOGE("enable BMA150 fail!!\n");
		else
			LOGV("enable BMA150 successful!!\n");

		//set soft_reset
		if (!SendIOMessage(m_aDevice, BMA_READ_REGISTER, 0x0a, &i2cData)) {
			LOGE("set soft_reset: I2C read from bma150 reg 0x0a ERROR");
			rc = 20;
			goto end;
		}

		i2cData=(i2cData & 0xfd) | 0x02;
		if (!SendIOMessage(m_aDevice, BMA_WRITE_REGISTER, 0x0a, &i2cData)) {
			LOGE("set soft_reset: I2C write to bma150 reg 0x0a ERROR");
			rc = 21;
			goto end;
		}
		usleep(1000);

		//Configure the bma150: 2G @25Hz
		if (!SendIOMessage(m_aDevice, BMA_READ_REGISTER, 0x14, &i2cData)) {
			LOGE("I2C read from bma150 reg 0x14 ERROR");
			rc = 3;
			goto end;
		}

		i2cData=i2cData&0xe0;
		if (!SendIOMessage(m_aDevice, BMA_WRITE_REGISTER, 0x14, &i2cData)) {
			LOGE("I2C write to bma150 reg 0x14 ERROR");
			rc = 4;
			goto end;
		}
	}

	//Read the Acceleration data of axis x, y, z and compare with 0, 0 , -1g
	Aoff.u.x=Aoff.u.y=Aoff.u.z=0;
	while (retry_times <= TEST_TIMES)
	{
		/*kuochih 20100203: enlarge the time from 20ms to 50ms to meet the HW requirement.*/
		usleep(50000);
		if(is_htc_sensor_hub){
			DoMCUCalibration(1);
			LOGD("Do MCU calibration\n");
			break;
		}
		if (is_bosch_bma250_driver) {
			get_bma250_bosch_data(tmpA.v);
		} else if (is_bosch_bma2x2_driver) {
			get_bma2x2_bosch_data(tmpA.v);
                } else if (is_lsm330_driver) {
			get_lsm330_data(tmpA.v);
		} else if (is_lc_bma2x2_driver) {
			get_lc_bma2x2_data(tmpA.v);
		} else if (is_bosch_bma253_driver) {
			get_bma253_bosch_data(tmpA.v);
		} else if (is_kxtj2_driver) {
			get_kxtj2_data(tmpA.v);
		} else {
			if (!SendIOMessage(m_aDevice, BMA_READ_ACCELERATION, NULL,
					tmpA.v)) {
				LOGE("READ ACCELERATION ERROR\n");
				rc = 5;
				goto end;
			}
		}

		/*kuochih 20100203: enlarge the time from 20ms to 50ms to meet the HW requirement.*/
		usleep(50000);

		if (is_bosch_bma250_driver) {
			get_bma250_bosch_data(tmpB.v);
		} else if (is_bosch_bma2x2_driver) {
			get_bma2x2_bosch_data(tmpB.v);
		} else if (is_lsm330_driver) {
			get_lsm330_data(tmpB.v);
		} else if (is_lc_bma2x2_driver) {
			get_lc_bma2x2_data(tmpB.v);
		} else if (is_bosch_bma253_driver) {
			get_bma253_bosch_data(tmpB.v);
		} else if (is_kxtj2_driver) {
			get_kxtj2_data(tmpB.v);
		} else {
			if (!SendIOMessage(m_aDevice, BMA_READ_ACCELERATION, NULL,
					tmpB.v)) {
				LOGE("READ ACCELERATION ERROR\n");
				rc = 6;
				goto end;
			}
		}

		if ((!is_bosch_bma250_driver) && (!is_bosch_bma2x2_driver) && (!is_lsm330_driver) && (!is_lc_bma2x2_driver)) {
			tmpA.u.x = tmpA.u.x + offset[0];
			tmpA.u.y = tmpA.u.y + offset[1];
			tmpA.u.z = tmpA.u.z + offset[2];

			tmpB.u.x = tmpB.u.x + offset[0];
			tmpB.u.y = tmpB.u.y + offset[1];
			tmpB.u.z = tmpB.u.z + offset[2];
		}

		//only for debug
		LOGD("x=%d\ty=%d\tz=%d\n",tmpA.u.x,tmpA.u.y,tmpA.u.z);
		LOGD("x=%d\ty=%d\tz=%d\n",tmpB.u.x,tmpB.u.y,tmpB.u.z);

		if (is_bosch_bma2x2_driver) {
			if((tmpA.u.x-tmpB.u.x)>ERROR_TOLERANCE_BMA2X2||(tmpB.u.x-tmpA.u.x)>ERROR_TOLERANCE_BMA2X2||
				(tmpA.u.y-tmpB.u.y)>ERROR_TOLERANCE_BMA2X2||(tmpB.u.y-tmpA.u.y)>ERROR_TOLERANCE_BMA2X2||
				(tmpA.u.z-tmpB.u.z)>ERROR_TOLERANCE_BMA2X2||(tmpB.u.z-tmpA.u.z)>ERROR_TOLERANCE_BMA2X2)
			{
				LOGE("Out of range of ERROR TOLERANCE_BMA2X2\n");

				rc = 7;/* Please keep device stable! */
				goto end;
			}
		} else if (is_lsm330_driver) {
			if((tmpA.u.x-tmpB.u.x)>ERROR_TOLERANCE_LSM330||(tmpB.u.x-tmpA.u.x)>ERROR_TOLERANCE_LSM330||
				(tmpA.u.y-tmpB.u.y)>ERROR_TOLERANCE_LSM330||(tmpB.u.y-tmpA.u.y)>ERROR_TOLERANCE_LSM330||
				(tmpA.u.z-tmpB.u.z)>ERROR_TOLERANCE_LSM330||(tmpB.u.z-tmpA.u.z)>ERROR_TOLERANCE_LSM330)
			{
				LOGE("Out of range of ERROR TOLERANCE_BMA2X2\n");

				rc = 7;/* Please keep device stable! */
				goto end;
			}
		} else if (is_lc_bma2x2_driver) {
			if((tmpA.u.x-tmpB.u.x)>ERROR_TOLERANCE_LC_BMA2X2||(tmpB.u.x-tmpA.u.x)>ERROR_TOLERANCE_LC_BMA2X2||
				(tmpA.u.y-tmpB.u.y)>ERROR_TOLERANCE_LC_BMA2X2||(tmpB.u.y-tmpA.u.y)>ERROR_TOLERANCE_LC_BMA2X2||
				(tmpA.u.z-tmpB.u.z)>ERROR_TOLERANCE_LC_BMA2X2||(tmpB.u.z-tmpA.u.z)>ERROR_TOLERANCE_LC_BMA2X2)
			{
				LOGE("Out of range of ERROR TOLERANCE_BMA2X2\n");

				rc = 7;/* Please keep device stable! */
				goto end;
			}
		} else {
			if((tmpA.u.x-tmpB.u.x)>ERROR_TOLERANCE||(tmpB.u.x-tmpA.u.x)>ERROR_TOLERANCE||
				(tmpA.u.y-tmpB.u.y)>ERROR_TOLERANCE||(tmpB.u.y-tmpA.u.y)>ERROR_TOLERANCE||
				(tmpA.u.z-tmpB.u.z)>ERROR_TOLERANCE||(tmpB.u.z-tmpA.u.z)>ERROR_TOLERANCE)
			{
				LOGE("Out of range of ERROR TOLERANCE\n");

				rc = 7;/* Please keep device stable! */
				goto end;
			}
		}
		Aoff.u.x+=(tmpA.u.x+tmpB.u.x);
		Aoff.u.y+=(tmpA.u.y+tmpB.u.y);
		Aoff.u.z+=(tmpA.u.z+tmpB.u.z);
		usleep(10000);
		retry_times+=2;
	}

#ifdef DEBUG
  	LOGD("Aoff.u.x = %d\n", Aoff.u.x);
  	LOGD("Aoff.u.y = %d\n", Aoff.u.y);
  	LOGD("Aoff.u.z = %d\n", Aoff.u.z);

  	LOGD("(Aoff.u.x / TEST_TIMES) = %d\n", (Aoff.u.x / TEST_TIMES) );
  	LOGD("(Aoff.u.y / TEST_TIMES) = %d\n", (Aoff.u.y / TEST_TIMES) );
  	LOGD("(Aoff.u.z / TEST_TIMES) = %d\n", (Aoff.u.z / TEST_TIMES) );
#endif

	if( Aoff.u.x > 0 )
		Aoff.u.x = ( Aoff.u.x / TEST_TIMES + GRANULARITY / 2 ) / GRANULARITY;
	else
		Aoff.u.x = ( Aoff.u.x / TEST_TIMES - GRANULARITY / 2 ) / GRANULARITY;

	if( Aoff.u.y > 0 )
		Aoff.u.y = ( Aoff.u.y / TEST_TIMES + GRANULARITY / 2 ) / GRANULARITY;
	else
		Aoff.u.y = ( Aoff.u.y / TEST_TIMES - GRANULARITY / 2 ) / GRANULARITY;

	LOGV("Before calculate: Aoff.u.z = %d\n", Aoff.u.z);
	if (chip_layout == 1) {
		if (is_bosch_bma2x2_driver) {
			if( Aoff.u.z > 1024 * 20 ) {
				LOGV("11\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) - 1024 ) / GRANULARITY;
			} else {
				LOGV("22\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) - 1024 ) / GRANULARITY;
			}
		} else if (is_lsm330_driver) {
			if( Aoff.u.z > 1000 * 20 ) {
				LOGV("111\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) - 1000 ) / GRANULARITY;
			} else {
				LOGV("222\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) - 1000 ) / GRANULARITY;
			}
		} else if (is_lc_bma2x2_driver) {
		} else {
			if( Aoff.u.z > 256 * 20 ) {
				LOGV("1\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) - 256 ) / GRANULARITY;
			} else {
				LOGV("2\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) - 256 ) / GRANULARITY;
			}
		}
	} else {
		if (is_bosch_bma2x2_driver) {
			if( Aoff.u.z > (-1024) * 20 ) {
				LOGV("33\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) + 1024 ) / GRANULARITY;
			} else {
				LOGV("44\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) + 1024 ) / GRANULARITY;
			}
		} else if (is_lsm330_driver) {
			if( Aoff.u.z > (-1000) * 20 ) {
				LOGV("333\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) + 1000 ) / GRANULARITY;
			} else {
				LOGV("444\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) + 1000 ) / GRANULARITY;
			}
		} else if (is_lc_bma2x2_driver) {
			if( Aoff.u.z > 1024 * 20 ) {
				LOGV("3333\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) - 1024 ) / GRANULARITY;
			} else {
				LOGV("4444\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) - 1024 ) / GRANULARITY;
			}
		} else {
			if( Aoff.u.z > (-256) * 20 ) {
				LOGV("3\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES + GRANULARITY / 2 ) + 256 ) / GRANULARITY;
			} else {
				LOGV("4\n");
				Aoff.u.z = ( ( Aoff.u.z / TEST_TIMES - GRANULARITY / 2 ) + 256 ) / GRANULARITY;
			}
		}
	}
	LOGV("After calculate: Aoff.u.z = %d\n", Aoff.u.z);
	snprintf(string, 64, "Offset setting: x=%d\ty=%d\tz=%d\n",Aoff.u.x, Aoff.u.y, Aoff.u.z);

#ifdef REAL_WRITE
	if (is_bosch_bma250_driver) {
		if (!save_offset_bosch_bma250(&Aoff)) {
			rc == 22;
			goto end;
		}

		k_value = 0x67 << 24 | (-Aoff.u.x & 0xFF) << 16 |
			(-Aoff.u.y & 0xFF) << 8 | (-Aoff.u.z & 0xFF);
		set_bma250_bosch_k_value(k_value);
	} else if (is_bosch_bma2x2_driver) {
		if (!save_offset_bosch_bma250(&Aoff)) {
			rc == 22;
			goto end;
		}

		k_value = 0x67 << 24 | (-Aoff.u.x & 0xFF) << 16 |
			(-Aoff.u.y & 0xFF) << 8 | (-Aoff.u.z & 0xFF);
		set_bma2x2_bosch_k_value(k_value);
	} else if (is_lsm330_driver) {
		if (!save_offset_bosch_bma250(&Aoff)) {
			rc == 22;
			goto end;
		}

		k_value = 0x67 << 24 | (-Aoff.u.x & 0xFF) << 16 |
			(-Aoff.u.y & 0xFF) << 8 | (-Aoff.u.z & 0xFF);
		set_lsm330_k_value(k_value);
	} else if (is_lc_bma2x2_driver) {
		if (!save_offset_bosch_bma250(&Aoff)) {
			rc == 22;
			goto end;
		}
		if (-Aoff.u.x > 127 || -Aoff.u.x < -128 ||
			-Aoff.u.y > 127 || -Aoff.u.y < -128 ||
			-Aoff.u.z > 127 || -Aoff.u.z < -128) {
			k_value = 0x67 << 24 | ((-Aoff.u.x >> 8) & 0xFF) << 16 |
				((-Aoff.u.y >> 8) & 0xFF) | ((-Aoff.u.z >> 8) & 0xFF);
			set_lc_bma2x2_k_value_msb(k_value);
		} else {
			set_lc_bma2x2_k_value_msb(k_value);
		}
		k_value = 0x67 << 24 | (-Aoff.u.x & 0xFF) << 16 |
			(-Aoff.u.y & 0xFF) << 8 | (-Aoff.u.z & 0xFF);
		set_lc_bma2x2_k_value(k_value);
	} else if (is_kxtj2_driver) {
		if (!save_offset_bosch_bma250(&Aoff)) {
			rc == 22;
			goto end;
		}

		k_value = 0x67 << 24 | (-Aoff.u.x & 0xFF) << 16 |
			(-Aoff.u.y & 0xFF) << 8 | (-Aoff.u.z & 0xFF);
		set_kxtj2_k_value(k_value);
	} else if (is_bosch_bma253_driver) {
		if (!save_offset_bosch_bma253(&Aoff)) {
			rc == 22;
			goto end;
		}
		
		k_value = 0x67 << 24 | (-Aoff.u.x & 0xFF) << 16 |
			(-Aoff.u.y & 0xFF) << 8 | (-Aoff.u.z & 0xFF);
		set_bma253_bosch_k_value(k_value);
	} else {
		if (stat("/data/misc/AK8973Prms.txt", &statbuf) == -1) {
			if (!saveAO_ACC_AK8975(&Aoff)) {
				rc = 22;
				goto end;
			}
		} else
			saveInt16vec("AK8973", "AOFFSET", &Aoff, "/data/misc/AK8973Prms.txt");
	}
#endif
	if (is_htc_sensor_hub) {
		Read_calibrator(temp_data);
		Aoff.u.x = temp_data[0];
		Aoff.u.y = temp_data[1];
		Aoff.u.z = temp_data[2];
		save_offset_mcu(temp_data);
	}
	snprintf(string, 64, "Offset setting: x=%d\ty=%d\tz=%d\n",Aoff.u.x,Aoff.u.y,Aoff.u.z);
	LOGD("%s", string);
	write(fd, string, strlen(string));
//	LOGD("Calibrtaion Successed!\n");
//	LOGD("offset setting: x=%d\ty=%d\tz=%d\n",Aoff.u.x,Aoff.u.y,Aoff.u.z);
end:
	if ((!is_bosch_bma250_driver)
	    && (!is_bosch_bma2x2_driver)
	    && (!is_lsm330_driver)
	    && (!is_htc_sensor_hub)
		&& (!is_lc_bma2x2_driver)
		&& (!is_bosch_bma253_driver)
	    && (!is_kxtj2_driver)) {
		if (!SendIOMessage(m_aDevice, BMA_SET_CALI_MODE, 0, NULL))
			LOGE("set calibration mode to 0 fail!!\n");

		if (!SendIOMessage(m_aDevice, BMA_SET_UPDATE_USER_CALI_DATA, 1, NULL)) {
			LOGE("set UPDATE_USER_CALI_DATA to 1 fail!!\n");
			return -1;
		}
	}
#if 0 // Do not enter suspend, it will let G-Sensor not working if upper layer not restart G-Sensor
	else if (is_bosch_bma2x2_driver) {
		enableBma2x2Device(0);
	} else if (is_bosch_bma250_driver) {
		enableDevice(0);
	}
#endif
	return rc;
}

