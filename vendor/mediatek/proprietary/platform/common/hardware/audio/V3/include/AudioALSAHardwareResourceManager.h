#ifndef ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
#define ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H

#include <tinyalsa/asoundlib.h>

#include "AudioType.h"
#include "AudioLock.h"
#include "AudioUtility.h"

namespace android
{

typedef enum
{
    AUDIO_SPEAKER_MODE_D = 0,
    AUDIO_SPEAKER_MODE_AB = 1,

} AUDIO_SPEAKER_MODE;


enum builtin_mic_specific_type
{
    BUILTIN_MIC_DEFAULT,
    BUILTIN_MIC_MIC1_ONLY,
    BUILTIN_MIC_MIC2_ONLY,
    BUILTIN_MIC_MIC3_ONLY,
};

//HTC_AUD_START
// For Camera landscape/portrait mode recording ++
enum audio_camcorder_type
{
    CAM_RIGHT,
    CAM_LEFT,
    CAM_PORT,
    CAM_MONO,
    CAM_UNKNOWN,
};
// For Camera landscape/portrait mode recording --
// For Listen key for volume control when suspend ++
typedef enum
{
    VOL_WAKE_KEY_PHONE = 1 << 0,
    VOL_WAKE_KEY_FM = 1 << 1,
    VOL_WAKE_KEY_LPA = 1 << 2

} VOL_WAKE_KEYY;
// For Listen key for volume control when suspend --
//HTC_AUD_END

class AudioALSADeviceConfigManager;
class AudioALSAHardwareResourceManager
{
    public:
        virtual ~AudioALSAHardwareResourceManager();
        static AudioALSAHardwareResourceManager *getInstance();


        /**
         * output devices
         */
        virtual status_t setOutputDevice(const audio_devices_t new_devices, const uint32_t sample_rate);
        virtual status_t startOutputDevice(const audio_devices_t new_devices, const uint32_t SampleRate);
        virtual status_t stopOutputDevice();
        virtual status_t changeOutputDevice(const audio_devices_t new_devices);
        virtual audio_devices_t getOutputDevice();


        /**
         * input devices
         */
        virtual status_t setInputDevice(const audio_devices_t new_device);
        virtual status_t startInputDevice(const audio_devices_t new_device);
        virtual status_t stopInputDevice(const audio_devices_t stop_device);
        virtual status_t changeInputDevice(const audio_devices_t new_device);
        virtual audio_devices_t getInputDevice();


        /**
         * HW Gain2
         */
        virtual status_t setHWGain2DigitalGain(const uint32_t gain);


        /**
         * Interrupt Rate
         */
        virtual status_t setInterruptRate(const uint32_t rate);
        virtual status_t setInterruptRate2(const uint32_t rate);


        /**
         * sgen
         */
        virtual status_t setSgenMode(const sgen_mode_t sgen_mode);
        virtual status_t setSgenSampleRate(const sgen_mode_samplerate_t sample_rate);
        virtual status_t openAddaOutput(const uint32_t sample_rate);
        virtual status_t closeAddaOutput();

        /**
        * sidetone
        */
        virtual status_t EnableSideToneFilter(const bool enable);
        /**
        *   Current Sensing
        */
        virtual status_t setSPKCurrentSensor(bool bSwitch);

        virtual status_t setSPKCurrentSensorPeakDetectorReset(bool bSwitch);

        /**
         * MIC inverse
         */
        virtual status_t setMicInverse(bool bMicInverse);
        virtual bool getMicInverse(void);

        virtual void setMIC1Mode(bool isphonemic);
        virtual void setMIC2Mode(bool isphonemic);

        /**
         * build in mic specific type
         */
        virtual void     setBuiltInMicSpecificType(const builtin_mic_specific_type type) { mBuiltInMicSpecificType = type; }

        /**
        * build in mic specific type
        */
        virtual void     EnableAudBufClk(bool bEanble);


        /**
         * Headphone Change information
         */
        virtual void     setHeadPhoneChange(bool bchange) { mHeadchange = bchange; }
        virtual bool     getHeadPhoneChange(void) { return mHeadchange ; }


        /**
         * debug dump register & DAC I2S Sgen
         */
        virtual void     setAudioDebug(const bool enable);

//HTC_AUD_START
        inline audio_devices_t getOutDevices(void) { return mOutputDevices; }
// For Acoustic Shock Version 2 ++
#ifdef ENABLE_ACOUSTIC_SHOCK2
        void             setMuteUpperSpk(bool enable);
#endif
// For Acoustic Shock Version 2 --
//HTC_AUD_ADD++    For Camera landscape/portrait mode recording
        void             setCamcoderMode(bool enable) { mCamcoderMode = enable; if (!enable) mMicInverse = false; }
        void             setAudioCamcoderType(const audio_camcorder_type type) { mAudioCamcorderType = type; if (type == CAM_LEFT) mMicInverse = true; else mMicInverse = false; }
//HTC_AUD_ADD--
        bool             getCamcoderMode() { return mCamcoderMode; }
        audio_camcorder_type getAudioCamcoderType() { return mAudioCamcorderType; }
// For Listen key for volume control when suspend ++
        void             volkeyAndBatteryWakeupNotify(VOL_WAKE_KEYY volkey, int enable);
// For Listen key for volume control when suspend --
        void             setAudioUsecaseFlag(const int flag) { mAudioUsecaseFlag = flag; }
        int              getAudioUsecaseFlag(void) { return mAudioUsecaseFlag; }
        virtual builtin_mic_specific_type getBuiltInMicSpecificType() { return mBuiltInMicSpecificType;}

#ifndef NXP_SMARTPA_SUPPORT
        status_t        setFmSpeaker(bool enable);
#endif
//HTC_AUD_END

    protected:
        AudioALSAHardwareResourceManager();



    private:
        /**
         * singleton pattern
         */
        static AudioALSAHardwareResourceManager *mAudioALSAHardwareResourceManager;

        /**
         *  low level for device open close
         */
         virtual status_t OpenReceiverPath(const uint32_t SampleRate);
         virtual status_t CloseReceiverPath();
         virtual status_t OpenHeadphonePath(const uint32_t SampleRate);
         virtual status_t CloseHeadphonePath();
//HTC_AUD_START
         virtual status_t OpenSpeakerPath(const uint32_t SampleRate, const audio_devices_t new_devices);
//HTC_AUD_END
         virtual status_t CloseSpeakerPath();
//HTC_AUD_START
         virtual status_t OpenHeadphoneSpeakerPath(const uint32_t SampleRate, const audio_devices_t new_devices);
//HTC_AUD_END
         virtual status_t CloseHeadphoneSpeakerPath();
         virtual status_t OpenBuiltInMicPath();
         virtual status_t CloseBuiltInMicPath();
         virtual status_t OpenBackMicPath();
         virtual status_t CloseBackMicPath();
         virtual status_t OpenWiredHeadsetMicPath();
         virtual status_t CloseWiredHeadsetMicPath();

        status_t setMicType(void);
        status_t SetExtDacGpioEnable(bool bEnable);
        bool GetExtDacPropertyEnable(void);

        AudioLock mLock;
        AudioLock mLockin;

        struct mixer *mMixer;
        struct pcm *mPcmDL;
        AudioALSADeviceConfigManager  *mDeviceConfigManager;

        audio_devices_t mOutputDevices;
        audio_devices_t mInputDevice;

        uint32_t        mOutputDeviceSampleRate;
        uint32_t        mInputDeviceSampleRate;

        bool mIsChangingOutputDevice;
        bool mIsChangingInputDevice;

        uint32_t mStartOutputDevicesCount;
        uint32_t mStartInputDeviceCount;

        bool mMicInverse;
        builtin_mic_specific_type mBuiltInMicSpecificType;

        bool mHeadchange;
        uint32_t mPhoneMicMode;
        uint32_t mHeadsetMicMode;
//HTC_AUD_START
        bool mCamcoderMode;
        audio_camcorder_type mAudioCamcorderType;
        audio_camcorder_type mCurAudioCamcorderType;
        int mAudioUsecaseFlag;
//HTC_AUD_END
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
