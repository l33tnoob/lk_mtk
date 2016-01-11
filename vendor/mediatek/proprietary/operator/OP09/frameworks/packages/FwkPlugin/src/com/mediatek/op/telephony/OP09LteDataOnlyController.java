/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.op.telephony;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.telephony.SubscriptionManager;

import com.mediatek.common.PluginImpl;
import com.mediatek.internal.telephony.cdma.CdmaFeatureOptionUtils;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;
/**
 * For check Lte data only mode that whether show dialog prompt.
 */
@PluginImpl(interfaceName = "com.mediatek.common.telephony.ILteDataOnlyController")
public class OP09LteDataOnlyController extends DefaultLteDataOnlyController {
    private static final String TAG = "OP09LteDataOnlyController";

    private static final String CHECK_PERMISSION_SERVICE_PACKAGE =
            "com.mediatek.op09.plugin";
    private static final String ACTION_CHECK_PERMISSISON_SERVICE =
            "com.mediatek.OP09.LTE_DATA_ONLY_MANAGER";

    /**
     * Constructor method.
     * @param context For start service.
     */
    public OP09LteDataOnlyController(Context context) {
        super(context);
    }

    @Override
    public boolean checkPermission() {
        if (is4GDataOnly()) {
            startService();
            Xlog.d(TAG, "checkPermission result : false");
            return false;
        }
        Xlog.d(TAG, "checkPermission result : true");
        return true;
    }

    @Override
    public boolean checkPermission(int subId) {
        int slotId = SubscriptionManager.getSlotId(subId);
        Xlog.d(TAG, "checkPermission subId=" + subId + ", slotId=" + slotId);
        if (CdmaFeatureOptionUtils.getExternalModemSlot() == slotId) {
            return checkPermission();
        } else {
            return true;
        }
    }

    private void startService() {
        Intent serviceIntent = new Intent(ACTION_CHECK_PERMISSISON_SERVICE);
        serviceIntent.setPackage(CHECK_PERMISSION_SERVICE_PACKAGE);
        if (mContext != null) {
            mContext.startService(serviceIntent);
        }
    }

    private boolean is4GDataOnly() {
        if (mContext == null) {
            Xlog.e(TAG, "is4GDataOnly error,mContext == null");
            return false;
        }
         int patternLteDataOnly = Settings.Global.getInt(mContext.getContentResolver(),
                                    Settings.Global.LTE_ON_CDMA_RAT_MODE,
                                    TelephonyManagerEx.SVLTE_RAT_MODE_4G);
        return (patternLteDataOnly == TelephonyManagerEx.SVLTE_RAT_MODE_4G_DATA_ONLY);
     }
}
