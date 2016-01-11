#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_NXP_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_NXP_H

#include "AudioType.h"
//HTC_AUD_START
#include "AudioLock.h"
//HTC_AUD_END
#include "AudioALSACodecDeviceBase.h"

//HTC_AUD_START
//TFA table loading
#define TFA_STATUS_RECORDER (1<<0)
#define TFA_STATUS_VIDEO    (1<<1)
#define TFA_STATUS_NOTE     (1<<2)
#define TFA_STATUS_WOOFER   (1<<3)
#define TFA_STATUS_FM       (1<<4)
//HTC_AUD_END

namespace android
{

class AudioALSACodecDeviceOutSpeakerNXP : public AudioALSACodecDeviceBase
{
    public:
        virtual ~AudioALSACodecDeviceOutSpeakerNXP();

        static AudioALSACodecDeviceOutSpeakerNXP *getInstance();


        /**
         * open/close codec driver
         */
//HTC_AUD_START
        status_t open(const uint32_t SampleRate, audio_devices_t device);
//HTC_AUD_END
        status_t close();

//HTC_AUD_START
// For MFG test L/R Speaker API ++
        void NXP_tfa_volume_mute(bool mute, bool isleft);
// For MFG test L/R Speaker API --
        int NXP_tfa_mix(bool mix);
        void NXP_tfa_set_flags(int flags, bool enable);

        void set_tfa_device(audio_devices_t device) { mTFA_DEVICE = device; };
        void set_tfa_status(int status, bool enable);
        //void set_tfa_mode(int mode) { mTFA_MODE = mode; };
        void NXP_reset_tfa98xx();
//HTC_AUD_END

    protected:
        AudioALSACodecDeviceOutSpeakerNXP();



    private:
        /**
         * singleton pattern
         */
        static AudioALSACodecDeviceOutSpeakerNXP *mAudioALSACodecDeviceOutSpeakerNXP;
        status_t open();
//HTC_AUD_START
        AudioLock mTFA_CONFIG_LOCK;
        int mTFA_STATUS, mTFA_MODE;
        audio_devices_t mTFA_DEVICE;
//HTC_AUD_END
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_NXP_H
