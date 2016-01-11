#include "AudioALSACodecDeviceOutSpeakerNXP.h"

#include "AudioLock.h"
#include "audio_custom_exp.h"
//HTC_AUD_START
#ifndef NXP_SMARTPA_SUPPORT_HTC
#include <mtk_tfa98xx_interface.h>
#endif
// htc_audio ++
#include "htc_acoustic.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSAHardwareResourceManager.h"
// htc_audio --
//HTC_AUD_END

#if defined(MTK_SPEAKER_MONITOR_SUPPORT) // test only
#include "AudioALSASpeakerMonitor.h"
#endif
#define APLL_ON //Low jitter Mode Set

#define LOG_TAG "AudioALSACodecDeviceOutSpeakerNXP"

namespace android
{

AudioALSACodecDeviceOutSpeakerNXP *AudioALSACodecDeviceOutSpeakerNXP::mAudioALSACodecDeviceOutSpeakerNXP = NULL;
AudioALSACodecDeviceOutSpeakerNXP *AudioALSACodecDeviceOutSpeakerNXP::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutSpeakerNXP == NULL)
    {
        mAudioALSACodecDeviceOutSpeakerNXP = new AudioALSACodecDeviceOutSpeakerNXP();
//HTC_AUD_START
        mAudioALSACodecDeviceOutSpeakerNXP->mTFA_MODE = TFA_PLAYBACK;
        mAudioALSACodecDeviceOutSpeakerNXP->mTFA_DEVICE = AUDIO_DEVICE_OUT_SPEAKER;
        mAudioALSACodecDeviceOutSpeakerNXP->mTFA_STATUS = 0;
    }
    ASSERT(mAudioALSACodecDeviceOutSpeakerNXP != NULL);
    ALOGD("%s (), mAudioALSACodecDeviceOutSpeakerNXP: %p", __FUNCTION__, mAudioALSACodecDeviceOutSpeakerNXP);
//HTC_AUD_END
    return mAudioALSACodecDeviceOutSpeakerNXP;
}


AudioALSACodecDeviceOutSpeakerNXP::AudioALSACodecDeviceOutSpeakerNXP()
{
    ALOGD("%s()", __FUNCTION__);
    // use default samplerate to load setting.
    open();
    close();
}


AudioALSACodecDeviceOutSpeakerNXP::~AudioALSACodecDeviceOutSpeakerNXP()
{
    ALOGD("%s()", __FUNCTION__);
//HTC_AUD_START
#ifndef NXP_SMARTPA_SUPPORT_HTC
    ALOGD("MTK_Tfa98xx_Deinit");
    MTK_Tfa98xx_Deinit();
#endif
//HTC_AUD_END
}


status_t AudioALSACodecDeviceOutSpeakerNXP::open()
{
//HTC_AUD_START
#ifndef NXP_SMARTPA_SUPPORT_HTC
#ifndef EXTCODEC_ECHO_REFERENCE_SUPPORT
    MTK_Tfa98xx_SetBypassDspIncall(1);
#endif
#endif
    ALOGD("+%s(), mClientCount %d", __FUNCTION__, mClientCount);
    mClientCount++;
    // HTC_AUD_START, prepare I2S clock by using alsa widget
    ALOGD("%s(), Audio_i2s0_hd_Switch on", __FUNCTION__);
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "On"))
    {
        ALOGE("Error: Audio_i2s0_hd_Switch invalid value");
    }
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On48000"))
    {
        ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
    }

    {
    AudioAutoTimeoutLock _l(mTFA_CONFIG_LOCK);
    mTFA_DEVICE = AUDIO_DEVICE_OUT_SPEAKER;
    set_amp_mode(AUDIO_MODE_NORMAL,
                mTFA_DEVICE,
                mTFA_MODE,
                0,
                false);// int halmode, int device, int mode, int devflag, boot reload
    }
    ALOGD("%s(), Audio_Speaker_Switch on", __FUNCTION__);
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_Switch"), "On"))
    {
        ALOGE("Error: Audio_Speaker_Switch invalid value");
    }
    // HTC_AUD_END
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}
//HTC_AUD_END
//HTC_AUD_START
status_t AudioALSACodecDeviceOutSpeakerNXP::open(const uint32_t SampleRate, audio_devices_t device)
{
    ALOGD("+%s(), mClientCount = %d, SampleRate = %d, this: %p", __FUNCTION__, mClientCount, SampleRate, this);
//HTC_AUD_END

    if (mClientCount == 0)
    {
#ifdef APLL_ON
        ALOGD("+%s(), Audio_i2s0_hd_Switch on", __FUNCTION__);
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "On"))
        {
            ALOGE("Error: Audio_i2s0_hd_Switch invalid value");
        }
#endif
        if (SampleRate == 48000)
        {
//HTC_AUD_START  For VOIP ECHO issue
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
            if (AudioALSAStreamManager::getInstance()->getMode() == AUDIO_MODE_IN_COMMUNICATION && device == AUDIO_DEVICE_OUT_SPEAKER) {
                ALOGD("%s(), Audio_ExtCodec_EchoRef_Switch on", __FUNCTION__);
                if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "On"))
                {
                    ALOGE("Error: Audio_ExtCodec_EchoRef_Switch invalid value");
                }
            }
#endif
//HTC_AUD_END

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On48000"))
            {
                ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
            }
        }
        else if (SampleRate == 44100)
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On44100"))
            {
                ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
            }
        }
        else if (SampleRate == 32000)
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On32000"))
            {
                ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
            }
        }
        else if (SampleRate == 16000)
        {
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
            ALOGD("%s(), Audio_ExtCodec_EchoRef_Switch on", __FUNCTION__);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "On"))
            {
                ALOGE("Error: Audio_ExtCodec_EchoRef_Switch invalid value");
            }
#endif
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On16000"))
            {
                ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
            }
        }
        else if (SampleRate == 8000)
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "On8000"))
            {
                ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
            }
        }
//HTC_AUD_START
#ifndef NXP_SMARTPA_SUPPORT_HTC
        MTK_Tfa98xx_SetSampleRate(SampleRate);
        MTK_Tfa98xx_SpeakerOn();
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
        //Echo Reference configure will be set on all sample rate
        MTK_Tfa98xx_EchoReferenceConfigure(1);
#endif
#endif
//HTC_AUD_END

#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
        //AudioALSASpeakerMonitor::getInstance()->Activate();
        //AudioALSASpeakerMonitor::getInstance()->EnableSpeakerMonitorThread(true);
#endif
//HTC_AUD_START
        set_sample_rate(SampleRate);
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_Switch"), "On")) //Power up the TFA before the set_amp_mode because the TFA is on DSP mode
        {
            ALOGE("Error: Audio_Speaker_Switch invalid value");
        }

        {
        AudioAutoTimeoutLock _l(mTFA_CONFIG_LOCK);
        mTFA_DEVICE = device;
        set_amp_mode(AudioALSAStreamManager::getInstance()->getMode(),
                    mTFA_DEVICE,
                    mTFA_MODE,
                    0,
                    false);// int halmode, int device, int mode, int devflag, boot reload
        }
//HTC_AUD_END
    }
    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutSpeakerNXP::close()
{
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0)
    {
//HTC_AUD_START
#ifndef NXP_SMARTPA_SUPPORT_HTC
        MTK_Tfa98xx_SpeakerOff();
#endif
//HTC_AUD_END
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_SideGen_Switch"), "Off"))
        {
            ALOGE("Error: Audio_i2s0_SideGen_Switch invalid value");
        }
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
        ALOGD("%s(), Audio_ExtCodec_EchoRef_Switch off", __FUNCTION__);
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ExtCodec_EchoRef_Switch"), "Off"))
        {
            ALOGE("Error: Audio_ExtCodec_EchoRef_Switch invalid value");
        }
#endif
#ifdef APLL_ON
        ALOGD("+%s(), Audio_i2s0_hd_Switch off", __FUNCTION__);
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_i2s0_hd_Switch"), "Off"))
        {
            ALOGE("Error: Audio_i2s0_hd_Switch invalid value");
        }
#endif

#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
        //AudioALSASpeakerMonitor::getInstance()->EnableSpeakerMonitorThread(false);
        //AudioALSASpeakerMonitor::getInstance()->Deactivate();
#endif
//HTC_AUD_START
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_Switch"), "Off"))
        {
            ALOGE("Error: Audio_Speaker_Switch invalid value");
        }
//HTC_AUD_END
    }

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}

//HTC_AUD_START
// For MFG test L/R speaker API ++
void AudioALSACodecDeviceOutSpeakerNXP::NXP_tfa_volume_mute(bool mute, bool isleft)
{
    tfa_volume_mute(mute, isleft);
}
// For MFG test L/R speaker API --

int AudioALSACodecDeviceOutSpeakerNXP::NXP_tfa_mix(bool mix)
{
    return tfa_mix(mix);
}

void AudioALSACodecDeviceOutSpeakerNXP::NXP_tfa_set_flags(int flags, bool enable)
{
    tfa_set_flags(flags, enable);
}

void AudioALSACodecDeviceOutSpeakerNXP::set_tfa_status(int status, bool enable)
{
    AudioAutoTimeoutLock _l(mTFA_CONFIG_LOCK);

    if (enable) mTFA_STATUS |= status;
    else mTFA_STATUS &= ~status;

    ALOGD("%s: mTFA_STATUS=0x%x", __func__, mTFA_STATUS);
    if (mTFA_STATUS & TFA_STATUS_FM) {
        //For MTK it will NOT handle by current device in set_amp_mode.
        mTFA_MODE = TFA_FM;
    } else if (mTFA_STATUS & TFA_STATUS_WOOFER) {
        //TFA_PLAYBACK_WOOFER priority have Highest prioirty
        mTFA_MODE = TFA_PLAYBACK_WOOFER;
    } else if (mTFA_STATUS & (TFA_STATUS_VIDEO|TFA_STATUS_NOTE|TFA_STATUS_RECORDER)) {
        //TFA_VIDEO, NOTE, RECORDER priority > TFA_PLAYBACK
        mTFA_MODE = TFA_RECORDER;
    } else {
        mTFA_MODE = TFA_PLAYBACK;
    }

    set_amp_mode(AudioALSAStreamManager::getInstance()->getMode(),
                 mTFA_DEVICE,
                 mTFA_MODE,
                 0,
                 false);// int halmode, int device, int mode, int devflag, boot reload
}

void AudioALSACodecDeviceOutSpeakerNXP::NXP_reset_tfa98xx()
{
    ALOGD("%s () +++", __FUNCTION__);
    // use default samplerate to load setting.
    reset_tfa98xx();
    open();
    close();
    ALOGD("%s () ---", __FUNCTION__);
}
//HTC_AUD_END

} // end of namespace android
