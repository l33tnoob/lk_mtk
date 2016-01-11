package com.mediatek.op.audioprofile;

import android.content.Context;
import com.mediatek.common.PluginImpl;

/**
 * Implementation of plugin for IAudioProfileExtension.
 */
@PluginImpl(interfaceName = "com.mediatek.common.audioprofile.IAudioProfileExtension")
public class AudioProfileExtensionOP01 extends DefaultAudioProfileExtension {

    /**
     * Dummy constructor which is needed by plugin mechansim.
     *
     * @param context the context passed by the caller
     */
    public AudioProfileExtensionOP01(Context context) {
    }

    @Override
    public boolean shouldCheckDefaultProfiles() {
        return IS_SUPPORT_OUTDOOR_EDITABLE;
    }

    @Override
    public boolean shouldSyncGeneralRingtoneToOutdoor() {
        return false;
    }
}
