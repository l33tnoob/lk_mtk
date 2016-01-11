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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <cutils/properties.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "utils.h"

#define TAG "[RAMDUMP] "
#define mod_to_ramdump(p) (struct ramdump_display*)((char*)(p) + sizeof(struct ftm_module))

int g_nr_lines;

struct ramdump_display {
    char info[1024];
    bool exit_thd;
    int result;

    /* for UI display */
    text_t title;
    text_t text;

    pthread_t update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    bool renew;
};

enum {
    AEE_RAMDUMP_RUNNING,
    AEE_RAMDUMP_DONE,
    AEE_RAMDUMP_FAIL,
    AEE_RAMDUMP_UNKNOWN,
};


int get_ramdump_status()
{
    char ramdump_status[128] = {0};

    property_get("debug.mtk.aee.ramdump", ramdump_status, NULL);
	LOGD(TAG "ramdump_status=%s\n", ramdump_status) ;
    if (strcmp(ramdump_status, "running") == 0) {
        return AEE_RAMDUMP_RUNNING;
    } else if (strcmp(ramdump_status, "done") == 0) {
        return AEE_RAMDUMP_DONE;
    } else if (strcmp(ramdump_status, "fail") == 0) {
        return AEE_RAMDUMP_FAIL;
    } else {
        return AEE_RAMDUMP_UNKNOWN;
    }
}


void update_ramdump_status(char* output_buf, int buf_len, int dot)
{
    char *ptr;
    int blankLinesBeforeText = 12;

    memset(output_buf, 0x0, buf_len);

    ptr = output_buf;

    while (blankLinesBeforeText-- > 0)
    {
        ptr += sprintf(ptr, "\n");
        g_nr_lines++;
    }

    switch (get_ramdump_status()) {
    case AEE_RAMDUMP_RUNNING:
        ptr += sprintf(ptr, "Processing ramdump ");
        while (dot-- > 0)
        {
            ptr += sprintf(ptr, ".");
        }
        break;

    case AEE_RAMDUMP_DONE:
        ptr += sprintf(ptr, "Processing ramdump - DONE");
        break;

    case AEE_RAMDUMP_FAIL:
        ptr += sprintf(ptr, "Processing ramdump - FAIL");
        break;

    default:
        ptr += sprintf(ptr, "Processing ramdump - UNKNOWN");
        break;
    }

    ptr += sprintf(ptr, "\n");
    g_nr_lines++;

    ptr += sprintf(ptr, "Long press UP to exit \n");
    g_nr_lines++;

    return;
}


char ** trans_dumpinfo(const char *str, int *line)
{
    char **pstrs = NULL;
    int len = 0;
    int row = 0;
    const char *start;
    const char *end;

    if((str == NULL) || (line == NULL))
    {
        return NULL;
    }

    len = strlen(str) + 1;
    start  = str;
    end    = str;
    pstrs = (char**)malloc(g_nr_lines * sizeof(char*));

    if (!pstrs)
    {
        LOGE("In ramdump mode: malloc failed\n");
        return NULL;
    }

    while (len--)
    {
        if ('\n' == *end)
        {
            pstrs[row] = (char*)malloc((end - start + 1) * sizeof(char));

            if (!pstrs[row])
            {
                LOGE("In ramdump mode: malloc failed\n");
                return NULL;
            }
                                                                                                             
            strncpy(pstrs[row], start, end - start);
            pstrs[row][end - start] = '\0';
            start = end + 1;
            row++;
        }
        end++;
    }

    *line = row;
    return pstrs;
}


void tear_down(char **pstr, int row)
{
    int i;

    if(pstr == NULL)
    {
        return;
    }
    for (i = 0; i < row; i++)
    {
        if (pstr[i])
        {
            free(pstr[i]);
            pstr[i] = NULL;
        }
    }

    if (pstr)
    {
        free(pstr);
        pstr = NULL;
    }
}


int ramdump_entry(struct ftm_param *param, void *priv)
{
    char *buf = NULL;
    struct textview *tv;
    text_t info;
    int nr_line;
    int avail_lines = 0;
    text_t rbtn;
    int count = 0;
    int chosen;
    bool exit = false;

    LOGD(TAG "%s\n", __FUNCTION__);

    struct ramdump_display *rdump_display = (struct ramdump_display *)priv;
    tv = &rdump_display->tv;

    buf = malloc(BUFSZ);
    LOGD(TAG "after creating buf");

    init_text(&rdump_display->title, param->name, COLOR_YELLOW);
    init_text(&info, buf, COLOR_YELLOW);
    init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
    ui_init_textview(tv, textview_key_handler, tv);
    tv->set_btn(tv, NULL, NULL, &rbtn);
    tv->set_title(tv, &rdump_display->title);
    tv->set_text(tv, &info);

    avail_lines = get_avail_textline();

    while(1){
        //if (ui_key_pressed(UI_KEY_BACK))
        if (ui_key_pressed(UI_KEY_BACK) || get_ramdump_status() == AEE_RAMDUMP_DONE || get_ramdump_status() == AEE_RAMDUMP_FAIL)//HTC: always exit ramdump mode after ramdump finished
        {
            if (buf)
            {
                LOGD(TAG "before free buf");
                free(buf);
                buf = NULL;
            }
			/*work around, when debug.mtk.aee.ramdump reply Done, dbg file is not really generated*/
			{
				int i = 60 ;
				while(i--)
				{
					usleep(500000) ;  //sleep 30 seconds
				}
			}
			break;
        }
		if(get_ramdump_status() == AEE_RAMDUMP_UNKNOWN)
		{
			static int try_times = 20 ;
			if(try_times)
			{
				usleep(500000) ;
				try_times-- ;
			}
			else
			{
				if(buf)
				{
					LOGD(TAG "unknown status for 10 seconds, reset") ;
					free(buf) ;
					buf = NULL ;
				}
				break ;
			}
		}
        update_ramdump_status(buf, BUFSZ, count);
        tv->m_pstr = trans_dumpinfo(info.string, &nr_line);
        tv->m_nr_lines = g_nr_lines;
        LOGD(TAG "g_nr_lines=%d, avail_lines=%d\n", g_nr_lines, avail_lines);
        tv->m_start = 0;
        tv->m_end = (nr_line < avail_lines ? nr_line : avail_lines);
        LOGD(TAG "vi.m_end is %d\n", tv->m_end);
        tv->redraw(tv);
        LOGD(TAG "Before tear_down\n");
        tear_down(tv->m_pstr, nr_line);
        usleep(500000);	//0.5 sec
        count = (count + 1) % 6;
    }

    return 0;
}


int ramdump_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct ramdump_display *rdump_display;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_RAMDUMP, sizeof(struct ramdump_display));
    if (!mod)
    {
        return -ENOMEM;
    }

    rdump_display  = mod_to_ramdump(mod);
    rdump_display->mod     = mod;

    ret = ftm_register(mod, ramdump_entry, (void*)rdump_display);

    return ret;
}
