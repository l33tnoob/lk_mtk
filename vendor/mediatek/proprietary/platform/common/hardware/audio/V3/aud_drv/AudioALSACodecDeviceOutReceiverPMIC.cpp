#include "AudioALSACodecDeviceOutReceiverPMIC.h"

//HTC_AUD_START
#include "AudioALSACodecDeviceOutSpeakerNXP.h"
#include "htc_acoustic.h"
//HTC_AUD_END

#include "AudioLock.h"

#include "AudioUtility.h"

#define LOG_TAG "AudioALSACodecDeviceOutReceiverPMIC"

namespace android
{

AudioALSACodecDeviceOutReceiverPMIC *AudioALSACodecDeviceOutReceiverPMIC::mAudioALSACodecDeviceOutReceiverPMIC = NULL;
AudioALSACodecDeviceOutReceiverPMIC *AudioALSACodecDeviceOutReceiverPMIC::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutReceiverPMIC == NULL)
    {
        mAudioALSACodecDeviceOutReceiverPMIC = new AudioALSACodecDeviceOutReceiverPMIC();
    }
    ASSERT(mAudioALSACodecDeviceOutReceiverPMIC != NULL);
    return mAudioALSACodecDeviceOutReceiverPMIC;
}


AudioALSACodecDeviceOutReceiverPMIC::AudioALSACodecDeviceOutReceiverPMIC()
{
    ALOGD("%s()", __FUNCTION__);
}


AudioALSACodecDeviceOutReceiverPMIC::~AudioALSACodecDeviceOutReceiverPMIC()
{
    ALOGD("%s()", __FUNCTION__);
}


//HTC_AUD_START
status_t AudioALSACodecDeviceOutReceiverPMIC::open(const uint32_t SampleRate, audio_devices_t device)
{
     ALOGD("+%s(), mClientCount = %d, SampleRate: %d, device: 0x%x", __FUNCTION__, mClientCount, SampleRate, device);

    if (mClientCount == 0)
    {
#ifdef HTC_NXP_RECEIVER_SUPPORT
// path: 6795 --> nxp --> receiver
        // AMP mode should config correctly when calling SpeakerNXP::open()
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->open(SampleRate, device);

        // mute L and R
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(true/*mute*/, true/*isLeft*/);
        ALOGD("%s: mute R speaker", __FUNCTION__);
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(true/*mute*/, false/*isLeft*/);

        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Receiver_Switch"), "On"))
        {
            ALOGE("%s: Error: Audio_Receiver_Switch invalid value", __FUNCTION__);
        }

        ALOGD("%s: mix L and R", __FUNCTION__);
        int ret = AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_mix(true);
        if (ret)
            ALOGW("%s: mix R to L failed, err %d", __FUNCTION__, ret);

        // un-mute L
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false/*mute*/, true/*isLeft*/);

        // enable Receiver flag
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_set_flags(RCV_MODE_FLAG, true/*enable*/);

#else
#ifdef MTK_TC7_FEATURE
// path: 6795 --> 6331(line_out) --> receiver
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_class_Switch"), "RECEIVER"))
        {
            ALOGE("Error: Audio_Speaker_class_Switch invalid value");
        }
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speaker_Amp_Switch"), "On"))
        {
            ALOGE("Error: Speaker_Amp_Switch invalid value");
        }
#else
// path: 6795 --> 6331(AU_HS) --> receiver
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_class_Switch"), "RECEIVER"))
            {
                ALOGE("Error: Audio_Speaker_class_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speaker_Amp_Switch"), "On"))
            {
                ALOGE("Error: Speaker_Amp_Switch invalid value");
            }
        }
        else
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Voice_Amp_Switch"), "On"))
            {
                ALOGE("Error: Voice_Amp_Switch invalid value");
            }
        }
#endif
#endif
    }

    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}
//HTC_AUD_END

status_t AudioALSACodecDeviceOutReceiverPMIC::close()
{
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0)
    {
// HTC_AUD_START
#ifdef HTC_NXP_RECEIVER_SUPPORT
// path: 6795 --> nxp --> receiver

        // disable Receiver flag
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_set_flags(RCV_MODE_FLAG, false/*enable*/);

        // mute L
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(true/*mute*/, true/*isLeft*/);

        ALOGD("%s: un-mix L and R", __FUNCTION__);
        int ret = AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_mix(false);
        if (ret)
            ALOGW("%s: un-mix R to L failed, err %d", __FUNCTION__, ret);

        // un-mute L and R
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false/*mute*/, true/*isLeft*/);
        ALOGD("%s: un-mute R speaker", __FUNCTION__);
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false/*mute*/, false/*isLeft*/);

        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->close();


        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Receiver_Switch"), "Off"))
        {
            ALOGE("%s: Error: Audio_Receiver_Switch invalid value", __func__);
        }
#else
#ifdef MTK_TC7_FEATURE
// path: 6795 --> 6331(line_out) --> receiver
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speaker_Amp_Switch"), "Off"))
        {
            ALOGE("Error: Speaker_Amp_Switch invalid value");
        }
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_class_Switch"), "CLASSAB"))
        {
            ALOGE("Error: Audio_Speaker_class_Switch invalid value");
        }
#else
// path: 6795 --> 6331(AU_HS) --> receiver
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER))
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speaker_Amp_Switch"), "Off"))
            {
                ALOGE("Error: Speaker_Amp_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_class_Switch"), "CLASSAB"))
            {
                ALOGE("Error: Audio_Speaker_class_Switch invalid value");
            }
        }
        else
        {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Voice_Amp_Switch"), "Off"))
            {
                ALOGE("Error: Voice_Amp_Switch invalid value");
            }
        }
#endif
#endif
// HTC_AUD_END
    }
    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


} // end of namespace android
