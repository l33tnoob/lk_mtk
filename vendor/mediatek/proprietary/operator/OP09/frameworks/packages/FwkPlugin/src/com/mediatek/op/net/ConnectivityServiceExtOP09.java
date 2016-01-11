package com.mediatek.op.net;

import android.content.Context;
import android.net.NetworkCapabilities;
import android.util.Log;

import com.mediatek.common.PluginImpl;
import com.mediatek.common.telephony.ILteDataOnlyController;
import com.mediatek.op.telephony.OP09LteDataOnlyController;

/**
 * OP09 plugin implementation of IConnectivityServiceExt.
 */
@PluginImpl(interfaceName = "com.mediatek.common.net.IConnectivityServiceExt")
public class ConnectivityServiceExtOP09 extends DefaultConnectivityServiceExt {
    private static final String TAG = "OP09/IConnectivityServiceExt";

    private ILteDataOnlyController mLteDataOnlyControllerExt;

    @Override
    public void init(Context context) {
        super.init(context);
        mLteDataOnlyControllerExt = new OP09LteDataOnlyController(mContext);
    }

    @Override
    public boolean ignoreRequest(Object networkCapabilities) {
        log("ignoreRequest: enter");
        NetworkCapabilities nc = (NetworkCapabilities) networkCapabilities;
        if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_MMS)
                && nc.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR)) {
            if (mLteDataOnlyControllerExt != null) {
                return (!mLteDataOnlyControllerExt.checkPermission());
            }
        }
        log("ignoreRequest: return false");
        return  false;
    }

    private void log(String s) {
        Log.d(TAG, s);
    }
}
