#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>
#include <cutils/log.h>
#include <linux/ioctl.h>
#include <utils/Log.h>
#include <private/android_filesystem_config.h>
#include "downloadtp.h"

#define TOUCH_FW_UPDATE_STATUS   "/sys/android_touch_fwu/fw_update_status"
#define TOUCH_VENDOR_PATH   "/sys/android_touch_fwu/vendor"
#define TOUCH_CONTROL_DEV_PATH   "/dev/touch_fwu"


#define TOUCH_FWU_IOCTL_CODE			(0x81)
#define FW_UPDATE_PROCCESS			_IO(TOUCH_FWU_IOCTL_CODE, 1)
#define FW_FILE_SIZE				_IOW(TOUCH_FWU_IOCTL_CODE, 2, uint32_t)
#define FW_FILE_REQUEST				_IO(TOUCH_FWU_IOCTL_CODE, 3)
#define FW_LOAD_DONE				_IO(TOUCH_FWU_IOCTL_CODE, 4)
#define FW_UPDATE_BYPASS			_IO(TOUCH_FWU_IOCTL_CODE, 5)

#define BUF_SIZE	(1024*8)

int main(int argc, char **argv)
{
	int ret, i, tagLen;
	uint32_t fsize;
	FILE *fp_file;
	int fd_vendor_path_attr = -1;
	int fd_update_status_attr = -1;
	int fd_control_dev = -1;
	char file_path[PATH_MAX] = {0};
	char str_vendor[PATH_MAX] = {0};
	char *buf;
	char tag[40];
	int read_count = 0, write_size = 0;
	unsigned long size_count = 0;

	ALOGI("%s begin\n", __func__);
	LOGF("[TP] Update Start\n");

	// Check argc, argv
	for (i = 0; i < argc; i++) {
		ALOGI("  argv[%d] = {%s}\n", i, argv[i]);
	}
	if (argc < 2) {
		ALOGE("%s: Invalid argument !!!\n", __func__);
		LOGF("[TP] Invalid argument !!!\n");
		ret = -1;
		goto exit;
	}

	sprintf(file_path, "%s", argv[1]);

	// open vendor attr
	fd_vendor_path_attr = open(TOUCH_VENDOR_PATH, O_RDONLY);
	if (fd_vendor_path_attr < 0) {
		ALOGE("%s[%d] open %s fail", __func__, __LINE__, TOUCH_VENDOR_PATH);
		LOGF("[TP] open %s fail\n", TOUCH_VENDOR_PATH);
		ret = -1;
		goto exit;
	}

	read(fd_vendor_path_attr, str_vendor, PATH_MAX);
	ALOGI("read %s: %s", TOUCH_VENDOR_PATH, str_vendor);
	LOGF("[TP] read %s: %s\n", TOUCH_VENDOR_PATH, str_vendor);

	close(fd_vendor_path_attr);

	//Open source file
	fp_file = fopen(file_path, "rb");
	if (fp_file == NULL) {
		ALOGE("%s:%d, failed to open %s, error = %s", __func__, __LINE__, file_path, strerror(errno));
		LOGF("[TP] failed to open %s, error = %s\n", file_path, strerror(errno));
		ret = -1;
		goto exit;
	}
	fseek(fp_file, 0, SEEK_END);
	fsize = ftell(fp_file);
	ALOGI("fsize: %d", fsize);
	fseek(fp_file, 0, SEEK_SET);

	buf = (char*) malloc (sizeof(char)*fsize);
	read_count = fread(buf, 1, fsize, fp_file);
	ALOGI("read file, read_count=%d", read_count);
	LOGF("[TP] read file, read_count=%d\n", read_count);
	fclose(fp_file);

	ALOGI("open fd_control_dev");
	fd_control_dev = open(TOUCH_CONTROL_DEV_PATH, O_RDWR);
	if (fd_control_dev < 0) {
		ALOGE("%s[%d] open %s fail", __func__, __LINE__, TOUCH_CONTROL_DEV_PATH);
		LOGF("[TP] open %s fail\n", TOUCH_CONTROL_DEV_PATH);
		ret = -1;
		free(buf);
		goto exit;
	}

	if (buf[0] == 'T' && buf[1] == 'P') {
		i = 0;
		while ((tag[i] = buf[i]) != '\n')
			i++;
		tag[i] = '\0';
		tagLen = i+1;
		ALOGI("tag=%s", tag);
		ALOGI("str_vendor=%s", str_vendor);
		if (strstr(tag, str_vendor) == NULL) {
			ALOGI("This firmware is not for this chip");
			LOGF("[TP] This firmware is not for this chip\n");
			ret = 1; /* bypass */
			ioctl(fd_control_dev, FW_UPDATE_BYPASS, NULL);
			goto close_file;
		}
	}

	ret = ioctl(fd_control_dev, FW_FILE_SIZE, fsize);
	if (ret < 0) {
		ALOGE("%s: ioctl:FW_FILE_SIZE fail:%s", __func__, strerror(errno));
		LOGF("[TP] FW_FILE_SIZE fail\n");
		goto close_file;
	}

	ret = ioctl(fd_control_dev, FW_FILE_REQUEST, NULL);
	if (ret < 0) {
		ALOGE("%s:ioctl:FW_FILE_REQUEST fail:%s", __func__, strerror(errno));
		LOGF("[TP] FW_FILE_REQUEST fail\n");
		goto close_file;
	}

	ALOGI("Begin to transfer FW");
	for (i=0; i<read_count; i+=BUF_SIZE) {
		if (read_count - size_count >= BUF_SIZE)
			write_size = BUF_SIZE;
		else
			write_size = read_count - size_count;
		ret = write(fd_control_dev, &buf[i], write_size);
		ALOGI("write: %d", i);
		if (ret < 0) {
			ALOGE("%s: fw write error: %d", __func__, i);
			LOGF("[TP] fw write error: %d\n", i);
			goto close_file;
		}
		size_count += write_size;
	}

	if (size_count != fsize) {
		ALOGE("%s: fw size not equal error: %ld", __func__, size_count);
		LOGF("[TP] fw size not equal error: %ld\n", size_count);
		goto close_file;
	}

	ret = ioctl(fd_control_dev, FW_LOAD_DONE, NULL);
	if (ret < 0) {
		ALOGE("%s[%d] ioctl:FW_LOAD_DONE fail", __func__, __LINE__);
		LOGF("[TP] FW_LOAD_DONE fail\n");
		goto close_file;
	}

	ALOGI("Load FW Done");
	ALOGI("Begin TP FW Update");
	ret = ioctl(fd_control_dev, FW_UPDATE_PROCCESS, NULL);
	if (ret < 0) {
		ALOGE("%s[%d] ioctl:FW_UPDATE_PROCCESS fail", __func__, __LINE__);
		LOGF("[TP] FW_UPDATE_PROCCESS fail\n");
		goto close_file;
	}

close_file:
	free(buf);
	close(fd_control_dev);

exit:
	if (ret == 1) {
		ALOGI("downloadtp end, ret:%d\n", ret);
		LOGF("[TP] Update Bypass\n");
		ret = 0;
	} else if (ret == 0){
		ALOGI("downloadtp end, ret:OK\n");
		LOGF("[TP] Update Success\n");
	} else {
		ALOGI("downloadtp end, ret:Failed\n");
		LOGF("[TP] Update Failed!!!\n");
	}

	return ret;
}
