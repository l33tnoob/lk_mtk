/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_fm.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   FM meta implement.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 03 12 2012 vend_am00076
 * [ALPS00251394] [Patch Request]
 * .
 *
 * 03 02 2012 vend_am00076
 * NULL
 * .
 *
 * 01 26 2011 hongcheng.xia
 * [ALPS00030208] [Need Patch] [Volunteer Patch][MT6620 FM]enable FM Meta mode
 * .
 *
 * 11 18 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 16 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 15 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 11 15 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for FM meta enable

 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/reboot.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>

#include "MetaPub.h"
#include "meta_cryptfs_para.h"

#define LOGD ALOGD
#define LOGE ALOGE

#undef  LOG_TAG
#define LOG_TAG  "CRYPTFS_META"


/********************************************************************************
//FUNCTION:
//		META_CRYPTFS_init
//DESCRIPTION:
//		CRYPTFS Init for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
bool META_CRYPTFS_init()
{
	LOGD("META_CRYPTFS_init ...\n");
	return 1;
}

/********************************************************************************
//FUNCTION:
//		META_CRYPTFS_deinit
//DESCRIPTION:
//		CRYPTFS deinit for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//     
********************************************************************************/
void META_CRYPTFS_deinit()
{
	LOGD("META_CRYPTFS_deinit ...\n");
	return;   
}

#define DEFAULT_ENCRYPTION_RETRIES 100

void wait_for_vold_decrypt(char* expected_status){
    char vold_decrypt[PROPERTY_VALUE_MAX];
    int i = 0;

    for(i = 0; i < DEFAULT_ENCRYPTION_RETRIES; i++ ) {
       property_get("vold.decrypt", vold_decrypt, "");
       if(!strcmp(vold_decrypt, expected_status)) {
          LOGD("vold_decryp wait well. It is already (%s)\n", expected_status);
          return;
       }
       sleep(1);
    }

    LOGD("vold_decryp wait fail. expected(%s), but still (%s) \n", expected_status, vold_decrypt);
}

void wait_for_encryption_type(){
    char encryption_type[PROPERTY_VALUE_MAX];
    int i = 0;

    for(i = 0; i < DEFAULT_ENCRYPTION_RETRIES; i++ ) {
       property_get("vold.encryption.type", encryption_type, "");
       if(strcmp(encryption_type, "")) {
          LOGD("encryption_type wait well. It is (%s)\n", encryption_type);
          return;
       }
       sleep(1);
    }

    LOGD("encryption_type wait fail. it is still empty \n");
}

int get_encrypt_phone_status()
{
    char crypto_state[PROPERTY_VALUE_MAX];
    char vold_decrypt[PROPERTY_VALUE_MAX];
    char encryption_type[PROPERTY_VALUE_MAX];
    int i = 0;
    
    property_get("ro.crypto.state", crypto_state, "");
    property_get("vold.decrypt", vold_decrypt, "");


    LOGD("crypto_state=(%s), vold_decrypt=(%s)\n", crypto_state, vold_decrypt);
    if (!strcmp(crypto_state, "")){
       /* for first boot, and support 'defaut encryption ', we need to wait until encrypted and decryted */
       wait_for_vold_decrypt("trigger_restart_framework");
    }
    else if (!strcmp(crypto_state, "encrypted")){
        wait_for_encryption_type();
        property_get("vold.encryption.type", encryption_type, "");

        LOGD("vold.encryption.type(%s)\n", encryption_type);
        if(!strcmp(encryption_type, "default")) {
            /* for encryption with default type, we need to wait until decryted */
            wait_for_vold_decrypt("trigger_restart_framework");
        }
        else {
            property_get("vold.decrypt", vold_decrypt, "");
            if (!strcmp(vold_decrypt, "trigger_restart_framework")) {
                /* it already decrypt */
            }
            else {
                /* for encryption with 'pass, piin or pattern' type, just return */
                wait_for_vold_decrypt("trigger_restart_min_framework");
                /* ask meta pc side to popup dialog by returning 1 */
               return 1; 
            }
        }      
    }
    else {
       /* for the device without encryption, just return status directly */    
    }
 
    /* it means the meta doens't to */
    return 0; 
}

void toHex(char *passwd, char *hex){
    int i = 0;
    int len = strlen(passwd);
    for(i = 0; i < len; i++) {
        hex += sprintf(hex, "%x", passwd[i]);
    }
}

int decrypt_data(char *passwd)
{
    int rtn=0;
    char cmd[255] = {'\0'};
    char hex[255] = {'\0'};

    toHex(passwd, hex);
    snprintf(cmd, sizeof(cmd), "cryptfs checkpw %s", hex);

    rtn = do_cmd_for_cryptfs(cmd);
    if (rtn) {
      LOGE("Fail: cryptfs checkpw, err=%d\n", rtn);
      return  rtn;
    }

    rtn = do_cmd_for_cryptfs("cryptfs restart");
     if (rtn) {
       LOGE("Fail: cryptfs restart, err=%d\n", rtn);
       return  rtn;
     }

    return 0;
}

int do_cmd_for_cryptfs(char* cmd) {
   
    int i;
    int ret;
    int sock;
    char final_cmd[255] = "0 "; /* 0 is a (now required) sequence number */
	ret = strlcat(final_cmd, cmd, sizeof(final_cmd));
	if (ret >= sizeof(final_cmd)) {
		LOGE("Fail: the cmd is too long (%s)", final_cmd);
		return (-1);
    }

    if ((sock = socket_local_client("vold",
                                     ANDROID_SOCKET_NAMESPACE_RESERVED,
                                     SOCK_STREAM)) < 0) {
        LOGE("Error connecting (%s)\n", strerror(errno));
        exit(4);
    }
	LOGD("do_cmd_for_cryptfs: %s\n", final_cmd);

    if (write(sock, final_cmd, strlen(final_cmd) + 1) < 0) {
        LOGE("Fail: write socket");
        return errno;
    }

    ret = do_monitor_for_cryptfs(sock, 1);
	close(sock);

    return ret;
}

int do_monitor_for_cryptfs(int sock, int stop_after_cmd) {
    char *buffer = malloc(4096);

    if (!stop_after_cmd)
        LOGD("[Connected to Vold]\n");

    while(1) {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 10;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        if ((rc = select(sock +1, &read_fds, NULL, NULL, &to)) < 0) {
            LOGE("Error in select (%s)\n", strerror(errno));
            free(buffer);
            return errno;
        } else if (!rc) {
            continue;
            LOGE("[TIMEOUT]\n");
            return ETIMEDOUT;
        } else if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, 4096);
            if ((rc = read(sock, buffer, 4096)) <= 0) {
                if (rc == 0)
                    LOGE("Lost connection to Vold - did it crash?\n");
                else
                    LOGE("Error reading data (%s)\n", strerror(errno));
                free(buffer);
                if (rc == 0)
                    return ECONNRESET;
                return errno;
            }
            
            int offset = 0;
            int i = 0;

            for (i = 0; i < rc; i++) {
                if (buffer[i] == '\0') {
                    int code;
					int rtn_code= -1;
                    char tmp[4];
                    char* token;

                    strncpy(tmp, buffer + offset, 3);
                    tmp[3] = '\0';
                    code = atoi(tmp);

                    LOGD("'%s'\n", buffer + offset);
                    if (stop_after_cmd) {
                        if (code >= 200 && code < 600) {
                            if (code == 200) {
                                if (strlen(buffer+offset) > 4) {
                                   token = strtok(buffer+offset+4, " ");								  
								   token = strtok(NULL, " ");
                                   rtn_code = atoi(token);
                                }
                                LOGD("cryptfs cmd, rtn_code=%d\n", rtn_code);
                                free(buffer);
                                return rtn_code;                                
                            }
                            else {
                                LOGE("invalid cryptfs cmd \n");
                                free(buffer);
                                return -1;
                            }
                        }
                    }
                    offset = i + 1;
                }
            }
        }
    }
    free(buffer);
    return 0;
}

#define FORCE_ENCRYPT_CONFIG "forceencrypt_config"
#define FORCE_ENCRYPT_CONFIG_MAX_LEN 2
struct env_ioctl
{
    char *name;
    int name_len;
    char *value;
    int value_len;  
};
#define ENV_MAGIC    'e'
#define ENV_READ        _IOW(ENV_MAGIC, 1, int)
#define ENV_WRITE       _IOW(ENV_MAGIC, 2, int)

/* 1 means to set cfg successfuly
    0 means that failed to set cfg */
int set_encrypt_cfg_status(char cfg) {
    char name[] = FORCE_ENCRYPT_CONFIG;
    char value[FORCE_ENCRYPT_CONFIG_MAX_LEN];
    unsigned int value_max_len = FORCE_ENCRYPT_CONFIG_MAX_LEN;

    if(cfg != 0 && cfg != 1) {
       SLOGE("%s, invalid cfg(%d) \n", __FUNCTION__, cfg);
       return 0; 
    }

    value[0] = cfg+'0';
    value[1] = '\0';

    struct env_ioctl env_ioctl_obj;
    int env_fd = 0;
    int data_fd = 0;
    int ret = 0;
    
    unsigned int name_len_set=strlen(name)+1;
    unsigned int value_len_set=strlen(value)+1;


    if ((env_fd = open("/proc/lk_env", O_RDWR)) < 0) {
        SLOGE("Open env fail for read %s.\n",name);
        goto FAIL_RUTURN;
    }
    if (!(env_ioctl_obj.name = malloc(name_len_set))) {
        SLOGE("Allocate Memory for env name fail.\n");
        goto FREE_FD;
    } else {
        memset(env_ioctl_obj.name,0x0,name_len_set);
    }
    if (!(env_ioctl_obj.value = malloc(value_max_len))){
        SLOGE("Allocate Memory for env value fail.\n");
        goto FREE_ALLOCATE_NAME;
    } else {
        memset(env_ioctl_obj.value,0x0,value_max_len);
    }
    env_ioctl_obj.name_len = name_len_set;
    env_ioctl_obj.value_len = value_max_len;
    memcpy(env_ioctl_obj.name, name, name_len_set);
    memcpy(env_ioctl_obj.value, value, value_len_set);
    if ((ret = ioctl(env_fd, ENV_WRITE, &env_ioctl_obj))) {
        SLOGE("Set env for %s check fail ret = %d, errno = %d.\n", name,ret, errno);
        goto FREE_ALLOCATE_VALUE;
    }
    free(env_ioctl_obj.name);
    free(env_ioctl_obj.value);
    close(env_fd);
    return 1;
FREE_ALLOCATE_VALUE:
    free(env_ioctl_obj.value);
FREE_ALLOCATE_NAME:
    free(env_ioctl_obj.name);
FREE_FD:
    close(env_fd);
FAIL_RUTURN:
    return 0;
}

/*
  0 means to disable 'default encryption'  
  1 means to enable 'default encryption' 
  others(ex, 'F') means Failure that no such cfg or something wrong 
*/
int get_encrypt_cfg_status()
{
    char name[] = FORCE_ENCRYPT_CONFIG;
    char cryptfs_env_value[FORCE_ENCRYPT_CONFIG_MAX_LEN];
    unsigned int value_max_len = FORCE_ENCRYPT_CONFIG_MAX_LEN;
    
    struct env_ioctl env_ioctl_obj;
    int env_fd;
    int ret=0;
    unsigned int name_len=strlen(name)+1;
    if((env_fd = open("/proc/lk_env", O_RDWR)) < 0) {
        SLOGE("Open env fail for read %s.\n",name);
        goto FAIL_RUTURN;
    }
    if(!(env_ioctl_obj.name = malloc(name_len))) {
        SLOGE("Allocate Memory for env name fail.\n");
        goto FREE_FD;
    }else{
        memset(env_ioctl_obj.name,0x0,name_len);
    }
    if(!(env_ioctl_obj.value = malloc(value_max_len))){
        SLOGE("Allocate Memory for env value fail.\n");
        goto FREE_ALLOCATE_NAME;
    }else{
        memset(env_ioctl_obj.value,0x0,value_max_len);
    }
    env_ioctl_obj.name_len = name_len;
    env_ioctl_obj.value_len = value_max_len;
    memcpy(env_ioctl_obj.name, name, name_len);
    if((ret = ioctl(env_fd, ENV_READ, &env_ioctl_obj))) {
        SLOGE("Get env for %s check fail ret = %d, errno = %d.\n", name,ret, errno);
        goto FREE_ALLOCATE_VALUE;
    }
    if(env_ioctl_obj.value) {
        memcpy(cryptfs_env_value,env_ioctl_obj.value,env_ioctl_obj.value_len);
        SLOGI("%s  = %s \n", env_ioctl_obj.name,env_ioctl_obj.value);
    } else {
        SLOGE("%s is not be set.\n",name);
        goto FREE_ALLOCATE_VALUE;
    }

    free(env_ioctl_obj.name);
    free(env_ioctl_obj.value);
    close(env_fd);
    return (cryptfs_env_value[0]-'0');
FREE_ALLOCATE_VALUE:
    free(env_ioctl_obj.value);
FREE_ALLOCATE_NAME:
    free(env_ioctl_obj.name);
FREE_FD:
    close(env_fd);
FAIL_RUTURN:
    return 'F';  // means failure
}

/********************************************************************************
//FUNCTION:
//		META_CRYPTFS_OP
//DESCRIPTION:
//		META CRYPTFS test main process function.
//
//PARAMETERS:
//
//RETURN VALUE:
//		void
//      
********************************************************************************/
void META_CRYPTFS_OP(FT_CRYPTFS_REQ *req) 
{
	LOGD("req->op:%d\n", req->op);
	int ret = 0;
	FT_CRYPTFS_CNF cryptfs_cnf;
	memcpy(&cryptfs_cnf, req, sizeof(FT_H) + sizeof(CRYPTFS_OP));
	cryptfs_cnf.header.id ++; 
	switch (req->op) {
	      case CRYPTFS_OP_QUERY_STATUS:              
              {
                  bool encrypted_status = 0;
				  cryptfs_cnf.m_status = META_SUCCESS;
				  encrypted_status = get_encrypt_phone_status();
				  LOGD("encrypted_status:%d \n", encrypted_status);
		    	  cryptfs_cnf.result.query_status_cnf.status = encrypted_status;
				  WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
              }
			break;

	      case CRYPTFS_OP_VERIFY:
              {
                  char* pw = req->cmd.verify_req.pwd;
				  int pw_len = req->cmd.verify_req.length;

				  cryptfs_cnf.m_status = META_SUCCESS;             
	              LOGD("pw = %s, pw_len = %d \n", pw, pw_len);
                  if (pw_len < 4  || pw_len > 16) {
					  cryptfs_cnf.result.verify_cnf.decrypt_result = 0;   
					  LOGE("Invalid passwd length =%d \n", pw_len);
					  WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
                      break;
                  }
                   
				  if(!decrypt_data(pw)) {
				     cryptfs_cnf.result.verify_cnf.decrypt_result = 1;			  
	              }
	              else {
	                cryptfs_cnf.result.verify_cnf.decrypt_result = 0;			   
	              }

				  LOGD("verify result:%d \n", cryptfs_cnf.result.verify_cnf.decrypt_result);
				  WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
              }
			break;

          case CRYPTFS_OP_SET_CFG:           
                {
                    bool set_cfg_result = 0;
                    char cfg = req->cmd.set_cfg_req.cfg;

                    cryptfs_cnf.m_status = META_SUCCESS;
                    set_cfg_result = set_encrypt_cfg_status(cfg);
                    LOGD("set_cfg_result:%d \n", set_cfg_result);
                    cryptfs_cnf.result.set_cfg_cnf.set_cfg_result = set_cfg_result;
                    WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
                }
                break;

          case CRYPTFS_OP_GET_CFG:           
                {
                    char get_cfg_status = 'F';
                    cryptfs_cnf.m_status = META_SUCCESS;
                    get_cfg_status = get_encrypt_cfg_status();
                    LOGD("get_cfg_status:%d \n", get_cfg_status);
                    cryptfs_cnf.result.get_cfg_cnf.get_cfg_result = get_cfg_status;
                    WriteDataToPC(&cryptfs_cnf, sizeof(FT_CRYPTFS_CNF), NULL, 0);
                }
                break;

	      default:	
            LOGE("Error: unsupport op code = %d\n", req->op);	  	
			break;
	}		
}

