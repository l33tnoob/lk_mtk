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
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <unistd.h>

#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"
#include "item.h"


#define TAG "RAMDUMP"

static item_t ftm_menu_items[] = {
    item(ITEM_RAMDUMP,   uistr_ramdump),
    item(ITEM_REBOOT,    uistr_reboot),
    item(ITEM_MAX_IDS,   NULL),
};


static int ram_dump_mode()
{
    int chosen_item = 0;
    struct itemview itv;
    struct ftm_param param;
    text_t title;
    item_t *items;

    LOGD(TAG "%s\n", __FUNCTION__);

    items = get_item_list();

    ui_init_itemview(&itv);
    init_text(&title, uistr_item_test, COLOR_YELLOW);

    itv.set_title(&itv, &title);
    itv.set_items(&itv, items, 0);

    chosen_item = ITEM_RAMDUMP;
    param.name = get_item_name(items, chosen_item);
    param.test_type = get_item_test_type(items, param.name);
    LOGD(TAG "%s, param.name = %s\n", __FUNCTION__ , param.name);
    LOGD(TAG "%s, param.test_type = %d\n", __FUNCTION__ , param.test_type);
    ftm_entry(chosen_item, &param);

    return 0;
}


int main(int argc, char **argv)
{
    int exit = 0;
    struct ftm_param param;
    struct itemview riv;  /* ramdump item menu */
    item_t *items;
    text_t ftm_title;
    int bootMode;

    ui_init();

    show_slash_screen(uistr_ramdump_mode, 1000);

    bootMode = getBootMode();

    //if(FACTORY_BOOT == bootMode)
    {
        ftm_init();
        ui_init_itemview(&riv);
        init_text(&ftm_title, uistr_ramdump_mode, COLOR_YELLOW);

        items = ftm_menu_items;
        riv.set_title(&riv, &ftm_title);
        riv.set_items(&riv, items, 0);

        ram_dump_mode();
        exit = 1;//HTC: always exit ramdump mode after ram_dump_mode()

        while (!exit) 
        {
            int chosen_item = riv.run(&riv, NULL);
            switch (chosen_item) 
            {
            case ITEM_RAMDUMP:
                ram_dump_mode();
                break;
            case ITEM_REBOOT:
                exit = 1;
                riv.exit(&riv);
                break;
            default:
                param.name = get_item_name(items, chosen_item);
                ftm_entry(chosen_item, &param);
                break;
            }
        }//end while

        ui_printf("Rebooting...\n");
        //sync();
        property_set("sys.powerctl","reboot");

        return EXIT_SUCCESS;

    }
/*
    else
    {
        LOGD(TAG "Unsupported Factory mode\n");
    }
*/
    return EXIT_SUCCESS;
}
