#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "htcsecure.h"
#include <utils/Log.h>
#include <fcntl.h>
#include <unistd.h>

#define LOG_TAG	"HtcMFGSecureLib"

#define CONTROL_PARTITION_BLOCK	"/dev/block/bootdevice/by-name/control"
#define HASH_DATA_LEN	    128
#define HASH_INDEX_LEN	    16

static int prHashForStorageLibrary(char *code, char *result)
{
    int index[HASH_INDEX_LEN] = {21, 23, 2, 29, 4, 10, 3, 25, 20, 5, 11, 13, 17, 27, 19, 7};
    int num, pos, i;
    char data;

    pos = 0;

    for (i = 1; i <= HASH_INDEX_LEN; i ++) {
	data = code[index[i - 1] - 1];
	num = data * i % 3;

	if (num == 0) /* 0 ~ 8 */
	    result[pos++] = (char)((data * index[i - 1] % 9) + 48);
	else if (num == 1) /* A ~ Z */
	    result[pos++] = (char)((data * index[i - 1] % 26) + 65);
	else /* a ~ z */
	    result[pos++] = (char)((data * index[i - 1] % 26) + 97);
    }

    return 0;
}

int find_mount_emmc(const char *findme, char *emmcindex)
{
    int fd;
    int res;
    int size;
#if 0
    char *token = NULL;
    const char delims[] = "\n";
#endif
    char buf[4096];
    char *end, *start;
    char emmcname[16], emmcsize[16], emmcerasesize[16];

    fd = open("/proc/emmc", O_RDONLY);
    if (fd < 0) {
	SLOGE("Failed to open /proc/emmc");
	return -errno;
    }

    memset(buf, 0, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';
    size = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (size <= 0) {
	SLOGE("There's no partition information in /proc/emmc");
	return -1;
    }

    start = buf;
    while ((end = strchr(start, '\n')) != NULL) {
	sscanf(start, "%[^:]: %[^ ] %[^ ] %s\n",
		emmcindex, emmcsize, emmcerasesize, emmcname);
	start = end + 1;

	if (!strcmp(emmcname, findme))
	    return 1;
    }

#if 0
    token = strtok(buf, delims);

    while (token) {
	char emmcname[16], emmcsize[16], emmcerasesize[16];

	res = sscanf(token, "%[^:]: %[^ ] %[^ ] %15s",
		    emmcindex, emmcsize, emmcerasesize, emmcname);

	if (res == 4 && !strcmp(emmcname, findme))
	    return 1;

	token = strtok(NULL, delims);
    }
#endif
    return -1;
}

int get_random(char *buf, size_t len)
{
    int i;
    unsigned char randbuf;
    int rc = 0;

    srand(time(NULL));

    for (i = 0; i < len; i ++) {
	randbuf = (unsigned char)rand();
	buf[i] = (randbuf % (127 - 32)) + 32;
	rc ++;
    }

    return rc != len ? -1 : 0;
}

/*
 column 1: value of Htc_MFG_Secure_Rule_Enabled (true/false), 1 byte
 column 2: value of Htc_MFG_Secure_Rule_Enabled_Verify_Code, 16 bytes
 column 3: value of Htc_Encrypted_Code, 32 bytes
 column 4: value of Htc_Encoded_Code_By_Encrypted_Code, 16 bytes 
 */
enum {
    SECURE_RULE_EN = 1,
    SECURE_RULE_VERIFY_CODE,
    ENCRYPTED_CODE,
    ENCODED_CODE,
    TOTAL_COLUMN,
};

#define SECURE_RULE_ENABLE_LEN  1
#define SECURE_RULE_VERIFY_CODE_LEN 16
#define ENCRYPTEDCODE_LEN   32
#define ENCODEDCODE_LEN 16

static int secure_str[TOTAL_COLUMN] = {0, 0, SECURE_RULE_ENABLE_LEN, SECURE_RULE_ENABLE_LEN + SECURE_RULE_VERIFY_CODE_LEN,
				SECURE_RULE_ENABLE_LEN + SECURE_RULE_VERIFY_CODE_LEN + ENCRYPTEDCODE_LEN};

static int read_from_emmc(int column, char *rd_string, int len)
{
    int fd, ret = 0;
    char *src;
    int pos;
    char emmc_index[16];
    char str_buf[64];

    pos = secure_str[column];

    memset(str_buf, 0, sizeof(str_buf));
    if (!access(CONTROL_PARTITION_BLOCK, F_OK))
	snprintf(str_buf, sizeof(str_buf), "%s", CONTROL_PARTITION_BLOCK);
    else {
	if (find_mount_emmc("\"control\"", emmc_index) < 1) {
	    SLOGE("Error finding control partition\n");
	    return -1;
	}

	snprintf(str_buf, sizeof(str_buf), "/dev/block/%s", emmc_index);
    }

    fd = open(str_buf, O_RDWR);
    if (fd < 0) {
	SLOGE("Fail to open %s", str_buf);
	return -1;
    }

    if (lseek64(fd, pos + 8, SEEK_SET) < 0) {
	SLOGE("Seek %s failed %s\n", str_buf, strerror(errno));
	ret = -1;
	goto out;
    }

    if (read(fd, rd_string, len) != len) {
	SLOGE("read from \"/proc/write_to_control\" failed.");
	close(fd);
	ret = -1;
	goto out;
    }

out:
    close(fd);
    return ret;
}

static int write_to_emmc(int column, char *wr_string, int len)
{
    int fd, ret = 0;
    char *dest;
    int pos;
    char emmc_index[16];
    char str_buf[64];

    pos = secure_str[column];

    memset(str_buf, 0, sizeof(str_buf));

    if (!access(CONTROL_PARTITION_BLOCK, F_OK))
	snprintf(str_buf, sizeof(str_buf), "%s", CONTROL_PARTITION_BLOCK);
    else {
	if (find_mount_emmc("\"control\"", emmc_index) < 1) {
	    SLOGE("Error finding control partition\n");
	    return -1;
	}

	snprintf(str_buf, sizeof(str_buf), "/dev/block/%s", emmc_index);
    }

    fd = open(str_buf, O_RDWR);
    if (fd < 0) {
	SLOGE("Fail to open %s", str_buf);
	return -1;
    }

    if (lseek64(fd, pos + 8, SEEK_SET) < 0) {
	SLOGE("Seek %s failed %s\n", str_buf, strerror(errno));
	ret = -1;
	goto out;
    }

    if (write(fd, wr_string, len) != len) {
	SLOGE("write to %s failed.", str_buf);
	ret = -1;
    }

out:
    close(fd);
    return ret;
}

static char Htc_MFG_Secure_Rule_Enabled_Verify_Code[SECURE_RULE_VERIFY_CODE_LEN + 1];
void setHtcSecureRule(boolean bHtc_MFG_Secure_Rule_Enabled)
{
    get_random(Htc_MFG_Secure_Rule_Enabled_Verify_Code, SECURE_RULE_VERIFY_CODE_LEN);
    Htc_MFG_Secure_Rule_Enabled_Verify_Code[SECURE_RULE_VERIFY_CODE_LEN] = '\0';

    if (bHtc_MFG_Secure_Rule_Enabled) {
	write_to_emmc(SECURE_RULE_EN, "T", SECURE_RULE_ENABLE_LEN);
	write_to_emmc(SECURE_RULE_VERIFY_CODE, Htc_MFG_Secure_Rule_Enabled_Verify_Code, SECURE_RULE_VERIFY_CODE_LEN);
    } else
	write_to_emmc(SECURE_RULE_EN, "F", SECURE_RULE_ENABLE_LEN);

    memset(Htc_MFG_Secure_Rule_Enabled_Verify_Code, 0, sizeof(Htc_MFG_Secure_Rule_Enabled_Verify_Code));

    return;
}

boolean getHtcSecureRule(void)
{
    char Htc_MFG_Secure_Rule_Enabled[SECURE_RULE_ENABLE_LEN];
    int ret;

    ret = read_from_emmc(SECURE_RULE_EN, Htc_MFG_Secure_Rule_Enabled, SECURE_RULE_ENABLE_LEN);

    if (!memcmp(Htc_MFG_Secure_Rule_Enabled, "T", SECURE_RULE_ENABLE_LEN))
	return true;
    else
	return false;
}

char* getHtcSecureRuleVerifyCode(void)
{
    int ret;

    ret = read_from_emmc(SECURE_RULE_VERIFY_CODE, Htc_MFG_Secure_Rule_Enabled_Verify_Code, SECURE_RULE_VERIFY_CODE_LEN);
    Htc_MFG_Secure_Rule_Enabled_Verify_Code[SECURE_RULE_VERIFY_CODE_LEN] = '\0';

    if (ret)
	return NULL;
    else
	return Htc_MFG_Secure_Rule_Enabled_Verify_Code;
}

boolean setHtcEncryptedCode(char* strHtc_Encrypted_Code)
{
    int ret;

    if (!strHtc_Encrypted_Code) {
	SLOGE("strHtc_Encrypted_Code does not exist.");
	return false;
    }

    ret = write_to_emmc(ENCRYPTED_CODE, strHtc_Encrypted_Code, ENCRYPTEDCODE_LEN);

    if (ret)
	return false;
    else
	return true;
}

boolean setEncodedCodeByEncryptedCode(char* strHtc_Encoded_Code_By_Encrypted_Code)
{
    int ret;

    if (!strHtc_Encoded_Code_By_Encrypted_Code) {
	SLOGE("strHtc_Encoded_Code_By_Encrypted_Code does not exist.");
	return false;
    }

    ret = write_to_emmc(ENCODED_CODE, strHtc_Encoded_Code_By_Encrypted_Code, ENCODEDCODE_LEN);

    if (ret)
	return false;
    else
	return true;
}

boolean isCameraKeepOff(void)
{
    int ret;
    char Htc_Encrypted_Code[ENCRYPTEDCODE_LEN] = "";
    char Htc_Encoded_Code_By_Encrypted_Code[ENCODEDCODE_LEN] = "";
    char aHtc_Encoded_Code_By_Encrypted_Code[ENCODEDCODE_LEN] = "";

    if (!getHtcSecureRule()) {
	SLOGW("Htc secure rule is not set!");
	return false;
    }
    ret = read_from_emmc(ENCRYPTED_CODE, Htc_Encrypted_Code, ENCRYPTEDCODE_LEN);
    if (ret)
	return false;

    ret = read_from_emmc(ENCODED_CODE, Htc_Encoded_Code_By_Encrypted_Code, ENCODEDCODE_LEN);
    if (ret)
	return false;

    prHashForStorageLibrary(Htc_Encrypted_Code, aHtc_Encoded_Code_By_Encrypted_Code);

    if (memcmp(Htc_Encoded_Code_By_Encrypted_Code, aHtc_Encoded_Code_By_Encrypted_Code, ENCODEDCODE_LEN)) {
	SLOGW("Encoded Code is not matched after calculation.");
	return true;
    } else
	return false;
}
