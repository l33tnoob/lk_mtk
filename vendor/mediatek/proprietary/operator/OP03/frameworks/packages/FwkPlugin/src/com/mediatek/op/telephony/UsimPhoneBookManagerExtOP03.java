package com.mediatek.op.telephony;

import com.mediatek.common.PluginImpl;

@PluginImpl(interfaceName="com.mediatek.common.telephony.IUsimPhoneBookManagerExt")
public class UsimPhoneBookManagerExtOP03 extends UsimPhoneBookManagerExt {
    public boolean isSupportAas() {
        return true;
    }
}
