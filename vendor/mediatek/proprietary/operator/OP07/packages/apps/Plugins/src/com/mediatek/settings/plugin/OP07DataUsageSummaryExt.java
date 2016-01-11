package com.mediatek.settings.plugin;



import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.view.View;
import android.widget.Switch;

import com.mediatek.common.PluginImpl;
import com.mediatek.op07.plugin.R;
import com.mediatek.settings.ext.DefaultDataUsageSummaryExt;
import com.mediatek.xlog.Xlog;

@PluginImpl(interfaceName="com.mediatek.settings.ext.IDataUsageSummaryExt")
public class OP07DataUsageSummaryExt extends DefaultDataUsageSummaryExt {

    private static final String TAG = "OP07DataUsageSummaryExt";
    private Context mContext;
    private Activity mActivity;
    private Switch mDataEnabled;
    private DialogInterface mDialog;
    private View.OnClickListener mDialogListener;
    private TelephonyManager mTelephonyManager;
    private View mView;

    public OP07DataUsageSummaryExt(Context context) {
        super(context);
        mContext = context;
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
        boolean isEnable = mTelephonyManager.getDataEnabled();
      if (isNeedtoShowRoamingMsg() && !isEnable) {
        Xlog.d(TAG, "create new dialog");
        int message = R.string.data_conn_under_roaming_hint;
        AlertDialog.Builder dialogBuild = new AlertDialog.Builder(mActivity);
        dialogBuild.setMessage(mContext.getText(message))
        .setTitle(android.R.string.dialog_alert_title)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setNegativeButton(android.R.string.cancel, null)
        .create();
        dialogBuild.setPositiveButton(mContext.getString(R.string.data_roaming_settings), new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                mDialogListener.onClick(mView);
                Intent i = new Intent(Settings.ACTION_DATA_ROAMING_SETTINGS);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mActivity.startActivity(i);
                }
            });
        dialogBuild.show();
        }
      else {
        mDialogListener.onClick(v);
        }
       }
    };

    public boolean setDataEnableClickListener(Activity activity, View dataEnabledView,
            Switch dataEnabled, View.OnClickListener dataEnabledDialogListerner) {
        Xlog.d(TAG, "setDataEnableClickListener");
        mActivity = activity;
        mDataEnabled = dataEnabled;
        mView = dataEnabledView;
        mDialogListener = dataEnabledDialogListerner;
        dataEnabledView.setOnClickListener(mClickListener);
        return true;
    }

    private boolean isNeedtoShowRoamingMsg() {

        boolean isInRoaming = mTelephonyManager.isNetworkRoaming();
        boolean isRoamingEnabled = getDataRoaming();
        Xlog.d(TAG, "isInRoaming=" + isInRoaming + " isRoamingEnabled=" + isRoamingEnabled);

        return (isInRoaming && !isRoamingEnabled);
    }

    private boolean getDataRoaming() {
        final ContentResolver resolver = mContext.getContentResolver();
        return Settings.Global.getInt(resolver, Settings.Global.DATA_ROAMING, 0) != 0;
    }
}
