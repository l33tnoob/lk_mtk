package com.mediatek.op.documentsui;

import android.app.DownloadManager;
import android.app.DownloadManager.Query;
import android.content.Context;
import android.database.Cursor;

import libcore.io.IoUtils;

import com.mediatek.common.PluginImpl;
import com.mediatek.common.documentsui.IDocumentsUIExtension;
import com.mediatek.xlog.Xlog;

/**
 * This is the OP09 implementation of documentsui plugin.
 */

@PluginImpl(interfaceName = "com.mediatek.common.documentsui.IDocumentsUIExtension")
public class DocumentsUIExtensionOP09 extends DefaultDocumentsUIExtension {

    private static final String TAG = "DocumentsUIExtensionOP09";
    private Context mContext;
    private DownloadManager mDownloadManager;

    public DocumentsUIExtensionOP09(Context context) {
        super(context);
        mContext = context;
        mDownloadManager = (DownloadManager) mContext.getSystemService(Context.DOWNLOAD_SERVICE);
        mDownloadManager.setAccessAllDownloads(true);
    }

    /**
     * @return Return current download item whether is downloading
     */
    public boolean checkIsDownloadingItem(String docId) {
        boolean isDownloadingItem = false;
        Cursor cursor = null;
        try {
            cursor = mDownloadManager.query(new Query().setFilterById(Long.parseLong(docId)));
            if (cursor != null && cursor.moveToFirst()) {
                final int status = cursor.getInt(
                        cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_STATUS));
                if (status == DownloadManager.STATUS_RUNNING) {
                    isDownloadingItem = true;
                }
            }
        } finally {
            IoUtils.closeQuietly(cursor);
        }
        Xlog.d(TAG, "checkIsDownloadingItem: docId = " + docId
                + ", downloading = " + isDownloadingItem);
        return isDownloadingItem;
    }
}
