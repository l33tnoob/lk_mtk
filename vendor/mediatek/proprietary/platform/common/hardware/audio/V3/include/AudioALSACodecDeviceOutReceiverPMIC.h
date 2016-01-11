#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_RECEIVER_PMIC_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_RECEIVER_PMIC_H

#include "AudioType.h"

#include "AudioALSACodecDeviceBase.h"


namespace android
{

class AudioALSACodecDeviceOutReceiverPMIC : public AudioALSACodecDeviceBase
{
    public:
        virtual ~AudioALSACodecDeviceOutReceiverPMIC();

        static AudioALSACodecDeviceOutReceiverPMIC *getInstance();


        /**
         * open/close codec driver
         */
        status_t open(const uint32_t SampleRate, audio_devices_t device);
        status_t close();



    protected:
        AudioALSACodecDeviceOutReceiverPMIC();



    private:
        /**
         * singleton pattern
         */
        static AudioALSACodecDeviceOutReceiverPMIC *mAudioALSACodecDeviceOutReceiverPMIC;
        status_t open() { return NO_ERROR; };
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_RECEIVER_PMIC_H
