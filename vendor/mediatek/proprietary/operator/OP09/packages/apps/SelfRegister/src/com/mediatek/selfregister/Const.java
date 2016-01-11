/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.selfregister;

/**
 * Constants definition.
 */
public class Const {

    public static final String TAG = "SelfRegister/";

    public static final String ACTION_BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";
    public static final String ACTION_PRE_BOOT_COMPLETED =
            "android.intent.action.PRE_BOOT_COMPLETED";
    public static final String ACTION_RETRY = "com.mediatek.selfregister.RETRY";
    public static final String ACTION_SUBINFO_RECORD_UPDATED =
            "android.intent.action.ACTION_SUBINFO_RECORD_UPDATED";

    public static final String SERVICE_DMAGENT = "DmAgent";

    public static final int MANUFACTURE_MAX_LENGTH = 3;
    public static final int MODEL_MAX_LENGTH = 20;
    public static final int SOFTWARE_VERSION_MAX_LENGTH = 60;

    public static final String ICCID_DEFAULT_VALUE = "00000000000000000000";

    public static final int ONE_HOUR = 60 * 60 * 1000;
    public static final int ONE_SECOND = 1000;

    public static final int RETRY_TIMES_MAX = 10;
    public static final String PRE_KEY_RETRY_TIMES = "pref_key_retry_times";
    public static final String PRE_KEY_SI = "pref_key_si";
    public static final String PRE_KEY_NI = "pref_key_ni";
    public static final String PRE_KEY_BASEID = "pref_key_basestation_id";
    public static final String PRE_KEY_RECEIVED_SUBINFO_BROADCAST =
            "pref_key_received_subinfo_broadcast";

    public static final int MEID_LENGTH = 14;
    public static final int IMEI_LENGTH = 15;

    public static final int[] SINGLE_SIM_SLOT = {0};
    public static final int[] DUAL_SIM_SLOT_LIST = {0, 1};

    public static final String SERVER_URL = "http://zzhc.vnet.cn";
    public static final String CONTENT_TYPE = "application/encrypted-json";

    public static final String JSON_RESULT_CODE = "resultCode";
    public static final String JSON_RESULT_DESC = "resultDesc";

    public static final String SOFTWARE_VERSION_DEFAULT = "L1.P1";
    public static final String MANUFACTURER_DEFAULT = "MTK";
    public static final String MODEL_DEFAULT = "Model";
    public static final String OPERATING_SYSTEM = "android";
}
