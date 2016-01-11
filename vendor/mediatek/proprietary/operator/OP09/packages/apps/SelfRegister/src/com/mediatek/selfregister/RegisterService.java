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

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.CellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants.CardType;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.common.dm.DmAgent;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

/**
 * Service which process the main logic of registration.
 */
public class RegisterService extends Service {
    private static final String SUB_TAG = Const.TAG + "RegisterService";
    private static final String KEY_MISC_CONFIG = Settings.Global.TELEPHONY_MISC_FEATURE_CONFIG;
    /// M: Index of misc feature config switch in engineer mode.
    private static final int BIT_MISC_CONFIG = 2;

    /// M: Some data from callback methods saved here.
    public Map<String, Object> mPhoneValues = new HashMap<String, Object>();

    /// M: Key for mPhoneValues.
    public static final String KEY_SI = "key_system_id";
    public static final String KEY_NI = "key_network_id";
    public static final String KEY_BASEID = "key_basestation_id";
    public static final String KEY_ROAMING = "key_roaming";
    public static final String KEY_NETWORK_TYPE = "key_network_type";
    public static final String KEY_CDMA_CARD_TYPE = "key_cdma_card_type";

    /// M: Message for handler.
    private static final int MSG_HANDLE_REGISTER = 0;
    private static final int MSG_HANDLE_RESULT = 1;
    private static final int MSG_HANDLE_NETWORK = 2;

    private DmAgent mAgent;
    private TelephonyManager mTelephonyManager;
    private ConnectivityManager mConnectivityManager;
    private PhoneStateListener mPhoneStateListener;
    private Context mContext;
    private int[] mSlotList;

    @Override
    public void onCreate() {
        mContext = this;

        if (mTelephonyManager == null) {
            mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        }

        if (mAgent == null) {
            Log.i(SUB_TAG, "onCreate(), Get DmAgent.");
            IBinder binder = ServiceManager.getService(Const.SERVICE_DMAGENT);
            if (binder == null) {
                Log.i(SUB_TAG, "Get DmAgent fail, binder is null!");
                return;
            }
            mAgent = DmAgent.Stub.asInterface(binder);
        }

        /// M: Register CDMA card type receiver.
        registerCardTypeReceiver();
    }

    private void registerPhoneStateListener(final int subId) {
        if (mTelephonyManager == null) {
            Log.e(SUB_TAG, "registerPhoneStateListener(), mTelephonyManager is null!");
            return;
        }

        mPhoneStateListener = new PhoneStateListener(subId) {

            @Override
            public void onCellLocationChanged(CellLocation location) {
                super.onCellLocationChanged(location);

                if (location instanceof CdmaCellLocation) {
                    int systemId = ((CdmaCellLocation) location).getSystemId();
                    int networkId = ((CdmaCellLocation) location).getNetworkId();
                    int baseId = ((CdmaCellLocation) location).getBaseStationId();

                    Log.d(SUB_TAG, "onCellLocationChanged(), SI: " + systemId
                            + " NI: " + networkId + " Base ID: " + baseId);
                    mPhoneValues.put(KEY_SI, systemId);
                    mPhoneValues.put(KEY_NI, networkId);
                    mPhoneValues.put(KEY_BASEID, baseId);

                    /// M: Save the SI and NI in SharedPreference, which may be used in retry.
                    SharedPreferences.Editor editor = PreferenceManager
                            .getDefaultSharedPreferences(mContext).edit();
                    editor.putInt(Const.PRE_KEY_SI, systemId);
                    editor.putInt(Const.PRE_KEY_NI, networkId);
                    editor.putInt(Const.PRE_KEY_BASEID, baseId);
                    editor.commit();

                    /// M: When SI/NI and network both ready, send the MSG.
                    ///    When SI/NI is ready but network is not ready, the MSG is sent in
                    ///    Network onAvailable().
                    if (mHandler.hasMessages(MSG_HANDLE_REGISTER)
                            && mPhoneValues.containsKey(KEY_NETWORK_TYPE)) {
                        mHandler.removeMessages(MSG_HANDLE_REGISTER);
                        mHandler.sendEmptyMessage(MSG_HANDLE_REGISTER);
                        Log.v(SUB_TAG, "Send MSG_HANDLE_REGISTER from onCellLocationChanged().");
                    }
                } else {
                    Log.v(SUB_TAG, "onCellLocationChanged(), not CDMA location.");
                }
            }

            @Override
            public void onServiceStateChanged(ServiceState serviceState) {
                super.onServiceStateChanged(serviceState);

                if (serviceState.getState() != ServiceState.STATE_IN_SERVICE) {
                    Log.w(SUB_TAG, "onServiceStateChanged(), not in service!");
                    return;
                }

                boolean roaming = serviceState.getRoaming();
                Log.d(SUB_TAG, "onServiceStateChanged(), roaming: " + roaming);

                /// M: If it is the first time to get the service state after the RegisterService
                ///    started, handle the register.
                if (!mPhoneValues.containsKey(KEY_ROAMING) && isSwitchOpen()) {
                    CellLocation location = mTelephonyManager.getCellLocation();
                    Log.d(SUB_TAG, "CellLocation is "
                            + (location == null ? null : location.toString()));

                    mPhoneValues.put(KEY_ROAMING, roaming);
                    /// M: Both network and SI/NI are ready.
                    if (mPhoneValues.containsKey(KEY_NETWORK_TYPE) && location != null) {
                        mHandler.sendEmptyMessage(MSG_HANDLE_REGISTER);
                        Log.v(SUB_TAG, "Send MSG_HANDLE_REGISTER from onServiceStateChanged().");
                    } else {
                        /// M: Network or SI/NI not ready.
                        Log.w(SUB_TAG, "CellLocation or Network is not ready, wait 30s...");
                        mHandler.sendEmptyMessageDelayed(MSG_HANDLE_REGISTER,
                                Const.ONE_SECOND * 30);
                    }
                } else {
                    mPhoneValues.put(KEY_ROAMING, roaming);
                }
            }
        };

        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_SERVICE_STATE | PhoneStateListener.LISTEN_CELL_LOCATION);
    }

    private void registerCardTypeReceiver() {
        Intent intent = registerReceiver(null,
                new IntentFilter(TelephonyIntents.ACTION_CDMA_CARD_TYPE));
        if (intent == null) {
            Log.e(SUB_TAG, "registerCardTypeReceiver(), intent is null!");
            return;
        }

        CardType cardType = (CardType) intent
              .getSerializableExtra(TelephonyIntents.INTENT_KEY_CDMA_CARD_TYPE);
        mPhoneValues.put(KEY_CDMA_CARD_TYPE, cardType);
        Log.d(SUB_TAG, "registerCardTypeReceiver(), CDMA card type: " + cardType);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    @Override
    public void onDestroy() {
        Log.d(SUB_TAG, "onDestroy()");
        mPhoneValues.clear();

        if (mConnectivityManager != null) {
            mConnectivityManager.unregisterNetworkCallback(mNetworkCallback);
        }

        if (mTelephonyManager != null) {
            mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        }

        /// M: To avoid JE.
        ///    MSG_HANDLE_REGISTER may be handled after this service has been destroyed.
        if (mHandler.hasMessages(MSG_HANDLE_REGISTER)) {
            mHandler.removeMessages(MSG_HANDLE_REGISTER);
        }

        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(SUB_TAG, "onStartCommand(), startId: " + startId);
        if (intent == null) {
            Log.e(SUB_TAG, "Intent is null!");
            stopSelf();
            return START_STICKY;
        }

        String action = intent.getAction();
        Log.d(SUB_TAG, "Action: " + action);

        if (action.equalsIgnoreCase(Const.ACTION_BOOT_COMPLETED) ||
                action.equalsIgnoreCase(Const.ACTION_RETRY)) {
            if (!isSwitchOpen()) {
                Log.w(SUB_TAG, "Switch in MiscConfig is not open, stop.");
                stopSelf();
                return START_NOT_STICKY;
            }

            if (!isIccCardExist()) {
                Log.d(SUB_TAG, "onStartCommand(), No SIM card.");
                stopSelf();
                return START_NOT_STICKY;
            }

            if (action.equalsIgnoreCase(Const.ACTION_RETRY) || needRegisterPhoneState()) {
                int[] subId = SubscriptionManager.getSubId(0);
                if (subId != null && subId[0] > 0) {
                    Log.d(SUB_TAG, "registerPhoneStateListener, subId: " + subId[0]);
                    registerPhoneStateListener(subId[0]);
                } else {
                    Log.e(SUB_TAG, "registerPhoneStateListener, subId invalid:"
                            + (subId == null ? "null" : subId[0]));
                    registerPhoneStateListener(SubscriptionManager.getDefaultSubId());
                }
            }

            /// M: Register callback to monitor network.
            /// the network maybe unstable after boot up, wait a while.
            mHandler.sendEmptyMessageDelayed(MSG_HANDLE_NETWORK, Const.ONE_SECOND * 10);
        } else if (action.equalsIgnoreCase(Const.ACTION_PRE_BOOT_COMPLETED)) {
            setRegisterFlag(false);
        } else if (action.equalsIgnoreCase(Const.ACTION_SUBINFO_RECORD_UPDATED)) {
            int[] subId = SubscriptionManager.getSubId(0);
            if (subId != null && subId[0] > 0) {
                Log.d(SUB_TAG, "registerPhoneStateListener from broadcast, subId: " + subId[0]);
                registerPhoneStateListener(subId[0]);
            } else {
                Log.e(SUB_TAG, "registerPhoneStateListener from broadcast, subId invalid:"
                        + (subId == null ? "null" : subId[0]));
                registerPhoneStateListener(SubscriptionManager.getDefaultSubId());
            }
        } else {
            Log.e(SUB_TAG, "Action is invalid!");
        }

        return START_STICKY;
    }

    /// M: Check if SUBINFO_RECORD_UPDATED is broadcasted before the BOOT_COMPLETED
    private boolean needRegisterPhoneState() {
        Intent intent = registerReceiver(null,
                new IntentFilter(Const.ACTION_SUBINFO_RECORD_UPDATED));
        Log.d(SUB_TAG, "needRegisterPhoneState(), intent: " + intent);

        if (intent != null) {
            /// M: Don't need the SUBINFO_RECORD_UPDATED any more as we registered the
            ///    PhoneStateListener already.
            SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(mContext);
            preference.edit().putBoolean(Const.PRE_KEY_RECEIVED_SUBINFO_BROADCAST, true).commit();
            return true;
        }
        return false;
    }

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_HANDLE_REGISTER:
                if (isSingleLoad()) {
                    mSlotList = Const.SINGLE_SIM_SLOT;
                } else {
                    mSlotList = Const.DUAL_SIM_SLOT_LIST;
                }
                handleRegister();
                break;
            case MSG_HANDLE_RESULT:
                boolean success = checkRegisterResult((JSONObject) msg.obj);
                if (!success) {
                    Log.e(SUB_TAG, "Register fail!");
                    setRetryAlarm();
                    stopSelf();
                    return;
                }

                /// M: Save ICCIDs to nvram and set register flag.
                saveICCID();
                setRegisterFlag(true);
                stopSelf();
                break;
            case MSG_HANDLE_NETWORK:
                if (mConnectivityManager == null) {
                    mConnectivityManager =
                            (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
                    Log.d(SUB_TAG, "register network callback.");
                    NetworkRequest networkRequest = new NetworkRequest.Builder()
                        .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).build();
                    mConnectivityManager.registerNetworkCallback(networkRequest, mNetworkCallback);
                }
                break;
            default:
                super.handleMessage(msg);
            }
        }
    };

    private void handleRegister() {
        if (!needRegister()) {
            Log.d(SUB_TAG, "handleRegister(), Do not need to register, stop.");
            stopSelf();
            return;
        }

        Log.d(SUB_TAG, "Need register.");

        /// M: Check the network.
        if (!mPhoneValues.containsKey(KEY_NETWORK_TYPE)) {
            Log.w(SUB_TAG, "handleRegister(), Network is not ready.");
            setRetryAlarm();
            stopSelf();
            return;
        }

        /// M: Send message and get response.
        new Thread(mNetworkTask).start();
    }

    private void saveICCID() {
        String[] iccIDs = getIccIDFromCard();

        if (mAgent == null) {
            Log.d(SUB_TAG, "Save ICCID failed, DmAgent is null!");
            return;
        }

        try {
            boolean ret = mAgent.writeIccID1(iccIDs[0].getBytes(), iccIDs[0].length());
            if (!ret) {
                Log.e(SUB_TAG, "Write ICCID1 fail!");
            }

            if (iccIDs.length < 2) {
                return;
            }
            ret = mAgent.writeIccID2(iccIDs[1].getBytes(), iccIDs[1].length());
            if (!ret) {
                Log.e(SUB_TAG, "Write ICCID2 fail!");
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private boolean checkRegisterResult(JSONObject response) {
        if (response == null) {
            Log.e(SUB_TAG, "checkRegisterResult(), response is null!");
            return false;
        }

        int resultCode = -1;
        String resultDesc = null;

        try {
            resultCode = response.getInt(Const.JSON_RESULT_CODE);
            resultDesc = response.getString(Const.JSON_RESULT_DESC);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        Log.d(SUB_TAG, "analyseResponse(), resultCode:" + resultCode + " resultDesc:" + resultDesc);

        if (resultCode == 0) {
            return true;
        }

        return false;
    }

    private boolean isSwitchOpen() {
        int config = Settings.Global.getInt(getContentResolver(), KEY_MISC_CONFIG, 0);
        return ((config & (1 << BIT_MISC_CONFIG)) != 0);
    }

    private boolean needRegister() {
        Log.v(SUB_TAG, "Enter needRegister()...");
        if (mAgent == null) {
            Log.d(SUB_TAG, "DmAgent is null! Don't register.");
            return false;
        }

        if (isSingleLoad() && !isIccCardForCT(0)) {
            Log.d(SUB_TAG, "needRegister(), Not CT card.");
            return false;
        }

        if (isInternationalRoaming()) {
            Log.d(SUB_TAG, "needRegister(), Device is in roaming.");
            return false;
        }

        if (!isRegistered()) {
            Log.d(SUB_TAG, "needRegister(), Never registered.");
            return true;
        }

        if (isIccIDIdentical()) {
            Log.d(SUB_TAG, "needRegister(), ICCID is identical.");
            return false;
        }

        return true;
    }

    private boolean isIccIDIdentical() {
        String[] iccIDFromCard = getIccIDFromCard();
        String[] iccIDFromDevice = getSavedIccID();

        if (iccIDFromCard.length != iccIDFromDevice.length) {
            Log.d(SUB_TAG, "ICCID is not the same: array length is not equal.");
            return false;
        }

        for (int i = 0; i < iccIDFromCard.length; i++) {
            if (!iccIDFromCard[i].equals(iccIDFromDevice[i])) {
                Log.d(SUB_TAG, "Can't find the same ICCID from device:" + iccIDFromCard[i]);
                return false;
            }
        }

        return true;
    }

    /// M: Get ICCID through DmAgent.
    private String[] getSavedIccID() {
        if (mAgent == null) {
            Log.d(SUB_TAG, "Get ICCID failed, DmAgent is null!");
            return null;
        }

        String[] iccIDArr = new String[mSlotList.length];
        try {
            byte[] iccID1 = mAgent.readIccID1();
            if (iccID1 != null) {
                iccIDArr[0] = new String(iccID1);
            }

            if (SubscriptionManager.isValidSlotId(1)) {
                /// M: Multi-SIM
                byte[] iccID2 = mAgent.readIccID2();
                if (iccID2 != null) {
                    iccIDArr[1] = new String(iccID2);
                }
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "Remote exception when get ICCID!" + re);
        }

        return iccIDArr;
    }

    /**
     * Get ICCID from SIM card.
     * @return Array of ICCIDs of two SIM cards. If only support single SIM, the length of
     *         this array is 1. If support dual SIM, but the second slot is empty, it returns
     *         default value(all zero).
     */
    public String[] getIccIDFromCard() {
        if (mSlotList == null) {
            Log.w(SUB_TAG, "getIccIDFromCard() may be error, mSlotList is null!");
            if (isSingleLoad()) {
                mSlotList = Const.SINGLE_SIM_SLOT;
            } else {
                mSlotList = Const.DUAL_SIM_SLOT_LIST;
            }
        }

        String[] iccIDArr = new String[mSlotList.length];

        for (int i = 0; i < mSlotList.length; i++) {
            int[] subId = SubscriptionManager.getSubId(i);

            if (subId == null || subId[0] < 0) {
                Log.d(SUB_TAG, "getIccIDFromCard(), Slot " + i
                        + " is empty, set default value.");
                iccIDArr[i] = Const.ICCID_DEFAULT_VALUE;
            } else {
                iccIDArr[i] = mTelephonyManager.getSimSerialNumber(subId[0]);
                Log.d(SUB_TAG, "getIccIDFromCard(), Slot " + i + ":" + iccIDArr[i]);
            }

            /// M: To avoid JE.
            if (iccIDArr[i] == null) {
                Log.e(SUB_TAG, "getIccIDFromCard(), iccIDArr[" + i + "] is null!");
                iccIDArr[i] = Const.ICCID_DEFAULT_VALUE;
            }
        }

        return iccIDArr;
    }

    private boolean isIccCardForCT(int slot) {
        boolean isCTCard = false;

        if (slot == 0 && mPhoneValues.containsKey(KEY_CDMA_CARD_TYPE)) {
            switch ((CardType) (mPhoneValues.get(KEY_CDMA_CARD_TYPE))) {
            case CT_3G_UIM_CARD:
            case CT_UIM_SIM_CARD:
            case CT_4G_UICC_CARD:
                isCTCard = true;
                break;
            default:
                break;
            }
        }

        return isCTCard;
    }

    private boolean isInternationalRoaming() {
        boolean isRoaming = (Boolean) mPhoneValues.get(KEY_ROAMING);
        return isRoaming;
    }

    private boolean isIccCardExist() {
        boolean hasCard = false;

        if (mTelephonyManager != null) {
            if (isSingleLoad()) {
                hasCard = mTelephonyManager.hasIccCard();
            } else {
                for (int i = 0; i < 2; i++) {
                    hasCard = mTelephonyManager.hasIccCard(i);
                    if (hasCard) {
                        break;
                    }
                }
            }
        } else {
            Log.e(SUB_TAG, "mTelephonyManager is null!");
        }

        Log.d(SUB_TAG, "isIccCardExist(): " + hasCard);
        return hasCard;
    }

    ///M: Read register flag from nvram by DmAgent.
    private boolean isRegistered() {
        int registerFlag = 0;

        try {
            byte[] readData = mAgent.readSelfRegisterFlag();
            if (readData != null && readData.length > 0) {
                try {
                    registerFlag = Integer.parseInt(new String(readData));
                } catch (NumberFormatException nfe) {
                    Log.w(SUB_TAG, "Register flag parse int failed!");
                }
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "Remote exception when read register flag!" + re);
        }

        return registerFlag == 1;
    }

    private void setRegisterFlag(boolean flag) {
        if (mAgent == null) {
            Log.d(SUB_TAG, "set registerFlag failed, DmAgent is null!");
            return;
        }

        Log.d(SUB_TAG, "Set registerFlag:" + flag);
        String registerFlag = flag ? "1" : "0";

        try {
            boolean result = mAgent.setSelfRegisterFlag(registerFlag.getBytes(),
                    registerFlag.length());
            if (!result) {
                Log.e(SUB_TAG, "setRegisterFlag(), Set register flag fail!");
            }
        } catch (RemoteException re) {
            Log.e(SUB_TAG, "Remote exception when setRegisterFlag!" + re);
        }
    }

    private void setRetryAlarm() {
        SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(this);
        int retryTimes = preference.getInt(Const.PRE_KEY_RETRY_TIMES, 0);
        if (retryTimes >= Const.RETRY_TIMES_MAX) {
            Log.e(SUB_TAG, "Already retried " + retryTimes + " times, register failed!");
            return;
        }

        retryTimes++;
        Log.d(SUB_TAG, "setRetryAlarm(), Retry one hour later...(" + retryTimes + ")");
        preference.edit().putInt(Const.PRE_KEY_RETRY_TIMES, retryTimes).commit();

        AlarmManager alarm = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        Intent intent = new Intent(Const.ACTION_RETRY, null, this, RegisterService.class);
        PendingIntent operation = PendingIntent.getService(this, 0, intent,
                PendingIntent.FLAG_ONE_SHOT);
        alarm.setExact(AlarmManager.RTC_WAKEUP,
                System.currentTimeMillis() + Const.ONE_HOUR, operation);
    }

    /**
     * Check if the device support multi-SIM or not.
     * @return true if support only one SIM, false if support multi-SIM.
     */
    public boolean isSingleLoad() {
        if (mTelephonyManager == null) {
            Log.e(SUB_TAG, "isSingleLoad(), mTelephonyManager is null! return false as default.");
            return false;
        }

        return (mTelephonyManager.getSimCount() == 1);
    }

    /**
     * Get instance of TelephonyManager in RegisterService.
     * @return The reference of mTelephonyManager.
     */
    public TelephonyManager getTelephonyManager() {
        return mTelephonyManager;
    }

    /**
     * Get instance of DmAgent in RegisterService.
     * @return The reference of mAgent.
     */
    public DmAgent getDmAgent() {
        return mAgent;
    }

    private ConnectivityManager.NetworkCallback mNetworkCallback =
            new ConnectivityManager.NetworkCallback() {

        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            if (mConnectivityManager == null) {
                Log.e(SUB_TAG, "Network onAvailable: mConnectivityManager is null!");
                return;
            }

            Log.d(SUB_TAG, "NetworkCallbackListener.onAvailable: network=" + network);

            NetworkInfo networkInfo = mConnectivityManager.getNetworkInfo(network);
            if (networkInfo == null) {
                Log.e(SUB_TAG, "NetworkInfo is null!");
                return;
            }

            int networkType = networkInfo.getType();
            int subId = SubscriptionManager.getDefaultDataSubId();
            int slotId = SubscriptionManager.getSlotId(subId);

            Log.i(SUB_TAG, new StringBuilder("NetworkType == ")
                    .append(networkInfo.getTypeName())
                    .append("(").append(networkType).append(")").append("\n")
                    .append("subId == ").append(subId).append("\n")
                    .append("slotId == ").append(slotId)
                    .toString());

            if (networkType == ConnectivityManager.TYPE_MOBILE && slotId == 1) {
                Log.d(SUB_TAG, "Network is from SIM 2, ignored.");
                return;
            }

            mPhoneValues.put(KEY_NETWORK_TYPE, networkType);

            /// M: If there is MSG_HANDLE_REGISTER pending in mHander, means network is been
            ///    waiting. Remove the delay message and send the message to handle register
            ///    immediately.
            if (mHandler.hasMessages(MSG_HANDLE_REGISTER) && mPhoneValues.containsKey(KEY_SI)) {
                mHandler.removeMessages(MSG_HANDLE_REGISTER);
                mHandler.sendEmptyMessage(MSG_HANDLE_REGISTER);
                Log.v(SUB_TAG, "Send MSG_HANDLE_REGISTER from NetworkCallback onAvailable().");
            }
        }

        @Override
        public void onLost(Network network) {
            super.onLost(network);
            if (mConnectivityManager == null) {
                Log.e(SUB_TAG, "Network onLost: mConnectivityManager is null!");
                return;
            }
            Log.d(SUB_TAG, "NetworkCallbackListener.onLost: network=" + network);

            NetworkInfo networkInfo = mConnectivityManager.getNetworkInfo(network);
            if (networkInfo == null) {
                Log.e(SUB_TAG, "NetworkInfo is null!");
                return;
            }

            int networkType = networkInfo.getType();

            Log.i(SUB_TAG, new StringBuilder("NetworkType == ")
                    .append(networkInfo.getTypeName())
                    .append("(").append(networkType).append(")").append("\n")
                    .toString());

            mPhoneValues.remove(KEY_NETWORK_TYPE);
        }
    };

    Runnable mNetworkTask = new Runnable() {
        @Override
        public void run() {
            String registerMessage = new RegisterMessage(
                    (RegisterService) mContext).getRegisterMessage();
            JSONObject response = HttpUtils.httpSend(Const.SERVER_URL, registerMessage);

            Message msg = mHandler.obtainMessage(MSG_HANDLE_RESULT, response);
            mHandler.sendMessage(msg);
        }
    };

}
