package com.mediatek.op.wifi;

import android.content.Context;
import android.provider.Settings;

import com.mediatek.common.PluginImpl;
import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.xlog.Xlog;

@PluginImpl(interfaceName="com.mediatek.common.wifi.IWifiFwkExt")
public class WifiFwkExtOP03 extends DefaultWifiFwkExt {
    private static final String TAG = "WifiFwkExtOP03";

    public WifiFwkExtOP03(Context context) {
        super(context);
    }

    public void init() {
         super.init();
    }

    public boolean needRandomSsid() {
        Xlog.d(TAG, "needRandomSsid =yes");
        return true;
    }

    public void setCustomizedWifiSleepPolicy(Context context) {
        int sleepPolicy = Settings.Global.getInt(context.getContentResolver(), Settings.Global.WIFI_SLEEP_POLICY, Settings.Global.WIFI_SLEEP_POLICY_NEVER);
        Xlog.d(TAG, "Before--> setCustomizedWifiSleepPolicy:" + sleepPolicy);
        if (sleepPolicy == Settings.Global.WIFI_SLEEP_POLICY_NEVER)
        sleepPolicy = Settings.Global.WIFI_SLEEP_POLICY_NEVER_WHILE_PLUGGED;

        Settings.Global.putInt(context.getContentResolver(), Settings.Global.WIFI_SLEEP_POLICY, sleepPolicy);
     Xlog.d(TAG, "After--> setCustomizedWifiSleepPolicy is:" + sleepPolicy);
    }

    public int hasNetworkSelection() {
        return IWifiFwkExt.OP_03;
    }

}
