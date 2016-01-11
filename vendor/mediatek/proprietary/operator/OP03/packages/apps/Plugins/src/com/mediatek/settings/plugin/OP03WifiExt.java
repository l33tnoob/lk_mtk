package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.ListPreference;

import com.mediatek.common.PluginImpl;
import com.mediatek.settings.ext.DefaultWifiExt;
import com.mediatek.xlog.Xlog;

@PluginImpl(interfaceName="com.mediatek.settings.ext.IWifiExt")
public class OP03WifiExt extends DefaultWifiExt {

    private static final String TAG = "WifiExt";

    public OP03WifiExt(Context context) {
        super(context);
    }

    public void setSleepPolicyPreference(ListPreference sleepPolicyPref, String[] entriesArray, String[] valuesArray) {
        String[] newEntries = {entriesArray[1], entriesArray[2]};
        String[] newValues = {valuesArray[1], valuesArray[2]};

        sleepPolicyPref.setEntries(newEntries);
        sleepPolicyPref.setEntryValues(newValues);
        Xlog.d(TAG, "setSleepPolicyPreference");
    }
}

