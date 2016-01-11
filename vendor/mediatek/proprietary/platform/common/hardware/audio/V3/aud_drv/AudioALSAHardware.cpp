#include "AudioALSAHardware.h"

#include <media/AudioSystem.h>
#include <binder/IServiceManager.h>
#include <media/IAudioPolicyService.h>

#include <CFG_AUDIO_File.h>
#include <AudioCustParam.h>


#include "AudioALSAStreamManager.h"

#include "AudioFtm.h" // TODO(Harvey): remove it
#include "LoopbackManager.h" // TODO(Harvey): remove it

#include "AudioALSAStreamOut.h"
//HTC_AUD_START
// For MFG 4 in 1 test ++
#include "AudioALSAStreamIn.h"
// For MFG 4 in 1 test --
//HTC_AUD_END
#include "AudioMTKHardwareCommand.h"
//#include "AudioALSAVolumeController.h"
#include "AudioVolumeFactory.h"


#include "AudioALSAFMController.h"

#include "AudioALSAParamTuner.h"
#include "SpeechANCController.h"

#include "SpeechDriverFactory.h"

#include "SpeechEnhancementController.h"
#include "AudioALSADeviceParser.h"

#include "AudioALSAVoiceWakeUpController.h"

#include "WCNChipController.h"
#include "AudioBTCVSDControl.h"

//HTC_AUD_START
// For MFG test L/R Speaker API ++
#ifdef NXP_SMARTPA_SUPPORT
#include "AudioALSACodecDeviceOutSpeakerNXP.h"
#endif
// For MFG test L/R Speaker API --
// For effect icon ++
#include "htc_acoustic.h"
// For effect icon --
#include "SpeechVMRecorder.h"
//HTC_AUD_END

#include "AudioVUnlockDL.h"
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
#include "AudioALSASpeakerMonitor.h"
#endif

#define LOG_TAG "AudioALSAHardware"

namespace android
{

/*==============================================================================
 *                     setParameters keys // TODO(Harvey): Move to a specific class for setParameters
 *============================================================================*/
// Phone Call Related
static String8 keySetVTSpeechCall       = String8("SetVTSpeechCall");
static String8 keyBtHeadsetNrec         = String8("bt_headset_nrec");
static String8 keySetFlightMode         = String8("SetFlightMode");

// Set BGS Mute
static String8 keySet_BGS_DL_Mute = String8("Set_BGS_DL_Mute");
static String8 keySet_BGS_UL_Mute    = String8("Set_BGS_UL_Mute");
// Set Phone Call Mute
static String8 keySet_SpeechCall_DL_Mute = String8("Set_SpeechCall_DL_Mute");
static String8 keySet_SpeechCall_UL_Mute = String8("Set_SpeechCall_UL_Mute");

// BT WB
static String8 keySetBTMode     = String8("bt_wbs");

// FM Rx Related
static String8 keySetFmEnable           = String8("AudioSetFmDigitalEnable");
//HTC_AUD_START    For HTC FM Function
static String8 keyHTCSetFmEnable        = String8("AudioHTCSetFmDigitalEnable");  //HTC_AUD
//HTC_AUD_END
static String8 keyGetFmEnable           = String8("GetFmEnable");
static String8 keySetFmVolume           = String8("SetFmVolume");

// FM Tx Related
static String8 keyGetFmTxEnable         = String8("GetFmTxEnable");
static String8 keySetFmTxEnable         = String8("SetFmTxEnable");

static String8 keyFMRXForceDisableFMTX  = String8("FMRXForceDisableFMTX");

// TDM record Related
static String8 keySetTDMRecEnable       = String8("SetTDMRecEnable");


//record left/right channel switch
//only support on dual MIC for switch LR input channel for video record when the device rotate
static String8 keyLR_ChannelSwitch = String8("LRChannelSwitch");

// BesRecord Related
static String8 keyHDREC_SET_VOICE_MODE = String8("HDREC_SET_VOICE_MODE");
static String8 keyHDREC_SET_VIDEO_MODE = String8("HDREC_SET_VIDEO_MODE");

static String8 keyGET_AUDIO_VOLUME_VER = String8("GET_AUDIO_VOLUME_VERSION");

// TTY
static String8 keySetTtyMode     = String8("tty_mode");

//force use Min MIC or Ref MIC data
//only support on dual MIC for only get main Mic or Ref Mic data
// 0: default dual mic , 1: main mic, 2: sec mic
static String8 keyForceUseSpecificMicData = String8("ForceUseSpecificMic");

//HTC_AUD_START camera feature
static String8 keyCamcorderMode = String8("CAMCORDER_MODE");
static String8 keyDolbySrsEq = String8("dolby_srs_eq");
//HTC_AUD_END

// Audio Tool related
//<---for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration) and HQA
static String8 keySpeechParams_Update = String8("UpdateSpeechParameter");
static String8 keySpeechVolume_Update = String8("UpdateSphVolumeParameter");
#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
static String8 keyDualMicParams_Update = String8("UpdateDualMicParameters");
static String8 keyDualMicRecPly = String8("DUAL_MIC_REC_PLAY");
static String8 keyDUALMIC_IN_FILE_NAME = String8("DUAL_MIC_IN_FILE_NAME");
static String8 keyDUALMIC_OUT_FILE_NAME = String8("DUAL_MIC_OUT_FILE_NAME");
static String8 keyDUALMIC_GET_GAIN = String8("DUAL_MIC_GET_GAIN");
static String8 keyDUALMIC_SET_UL_GAIN = String8("DUAL_MIC_SET_UL_GAIN");
static String8 keyDUALMIC_SET_DL_GAIN = String8("DUAL_MIC_SET_DL_GAIN");
static String8 keyDUALMIC_SET_HSDL_GAIN = String8("DUAL_MIC_SET_HSDL_GAIN");
static String8 keyDUALMIC_SET_UL_GAIN_HF = String8("DUAL_MIC_SET_UL_GAIN_HF");
#endif
static String8 keyMagiConParams_Update = String8("UpdateMagiConParameters");
static String8 keyHACParams_Update = String8("UpdateHACParameters");
static String8 keyMusicPlusSet      = String8("SetMusicPlusStatus");
static String8 keyMusicPlusGet      = String8("GetMusicPlusStatus");
static String8 keyHiFiDACSet      = String8("SetHiFiDACStatus");
static String8 keyHiFiDACGet      = String8("GetHiFiDACStatus");
static String8 keyHDRecTunningEnable    = String8("HDRecTunningEnable");
static String8 keyHDRecVMFileName   = String8("HDRecVMFileName");
static String8 keyACFHCF_Update = String8("UpdateACFHCFParameters");
static String8 keyBesLoudnessSet      = String8("SetBesLoudnessStatus");
static String8 keyBesLoudnessGet      = String8("GetBesLoudnessStatus");
//--->
//TDM record
static    android_audio_legacy::AudioHardwareInterface *gAudioHardware = NULL;
static    android_audio_legacy::AudioStreamIn *gAudioStreamIn = NULL;
static uint32 gTDMsetSampleRate = 44100;
static bool gTDMsetExit = false;



// Dual Mic Noise Reduction, DMNR for Receiver
static String8 keyEnable_Dual_Mic_Setting = String8("Enable_Dual_Mic_Setting");
static String8 keyGet_Dual_Mic_Setting    = String8("Get_Dual_Mic_Setting");

// Dual Mic Noise Reduction, DMNR for Loud Speaker
static String8 keySET_LSPK_DMNR_ENABLE = String8("SET_LSPK_DMNR_ENABLE");
static String8 keyGET_LSPK_DMNR_ENABLE = String8("GET_LSPK_DMNR_ENABLE");

// Voice Clarity Engine, VCE
static String8 keySET_VCE_ENABLE = String8("SET_VCE_ENABLE");
static String8 keyGET_VCE_ENABLE = String8("GET_VCE_ENABLE");
static String8 keyGET_VCE_STATUS = String8("GET_VCE_STATUS"); // old name, rename to GET_VCE_ENABLE, but still reserve it

// Magic Conference Call
static String8 keyGET_MAGI_CONFERENCE_SUPPORT = String8("GET_MAGI_CONFERENCE_SUPPORT");
static String8 keySET_MAGI_CONFERENCE_ENABLE = String8("SET_MAGI_CONFERENCE_ENABLE");
static String8 keyGET_MAGI_CONFERENCE_ENABLE = String8("GET_MAGI_CONFERENCE_ENABLE");

// HAC
static String8 keyGET_HAC_SUPPORT = String8("GET_HAC_SUPPORT");
static String8 keySET_HAC_ENABLE = String8("HACSetting");
static String8 keyGET_HAC_ENABLE = String8("GET_HAC_ENABLE");

// Magic Conference Direction of Arrow
static String8 keySET_MAGI_CONFERENCE_DOA_ENABLE = String8("SET_MAGI_CONFERENCE_DOA_ENABLE");
static String8 keyGET_MAGI_CONFERENCE_DOA_ENABLE = String8("GET_MAGI_CONFERENCE_DOA_ENABLE");

//VoIP
//VoIP Dual Mic Noise Reduction, DMNR for Receiver
static String8 keySET_VOIP_RECEIVER_DMNR_ENABLE = String8("SET_VOIP_RECEIVER_DMNR_ENABLE");
static String8 keyGET_VOIP_RECEIVER_DMNR_ENABLE    = String8("GET_VOIP_RECEIVER_DMNR_ENABLE");

//VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
static String8 keySET_VOIP_LSPK_DMNR_ENABLE = String8("SET_VOIP_LSPK_DMNR_ENABLE");
static String8 keyGET_VOIP_LSPK_DMNR_ENABLE = String8("GET_VOIP_LSPK_DMNR_ENABLE");

//Vibration Speaker usage
static String8 keySET_VIBSPK_ENABLE = String8("SET_VIBSPK_ENABLE");
static String8 keySET_VIBSPK_RAMPDOWN = String8("SET_VIBSPK_RAMPDOWN");

// Voice Wake up
static String8 keyMTK_VOW_ENABLE = String8("MTK_VOW_ENABLE");
static String8 keyACCESS_VOW_PARAM = String8("VOWParameters");
static String8 keyACCESS_VOW_KR_PARAM = String8("VOWKRParameters");
static String8 keyACCESS_VOW_KRSR_PARAM = String8("VOWKRSRParameters");

// Voice UnLock
static String8 keyGetCaptureDropTime = String8("GetCaptureDropTime");

// low latency
static String8 keySCREEN_STATE = String8("screen_state");

// Loopbacks
static String8 keySET_LOOPBACK_USE_LOUD_SPEAKER = String8("SET_LOOPBACK_USE_LOUD_SPEAKER");
static String8 keySET_LOOPBACK_TYPE = String8("SET_LOOPBACK_TYPE");
static String8 keySET_LOOPBACK_MODEM_DELAY_FRAMES = String8("SET_LOOPBACK_MODEM_DELAY_FRAMES");

static String8 keyMTK_AUDENH_SUPPORT = String8("MTK_AUDENH_SUPPORT");
static String8 keyMTK_TTY_SUPPORT = String8("MTK_TTY_SUPPORT");
static String8 keyMTK_WB_SPEECH_SUPPORT = String8("MTK_WB_SPEECH_SUPPORT");
static String8 keyMTK_DUAL_MIC_SUPPORT = String8("MTK_DUAL_MIC_SUPPORT");
static String8 keyMTK_AUDIO_HD_REC_SUPPORT = String8("MTK_AUDIO_HD_REC_SUPPORT");
static String8 keyMTK_BESLOUDNESS_SUPPORT = String8("MTK_BESLOUDNESS_SUPPORT");
static String8 keyMTK_BESSURROUND_SUPPORT = String8("MTK_BESSURROUND_SUPPORT");
static String8 keyMTK_HDMI_MULTI_CHANNEL_SUPPORT = String8("MTK_HDMI_MULTI_CHANNEL_SUPPORT");
static String8 keyMTK_VOW_SUPPORT = String8("MTK_VOW_SUPPORT");

//HTC_AUD_START
// For MFG test L/R Speaker API ++
static String8 keyTEST_LR_SPEAKER = String8("TestLRSpeaker");
// For MFG test L/R Speaker API --
//HTC_AUD_END

#ifdef MTK_AUDIO_GAIN_TABLE
static String8 keyVolumeStreamType    = String8("volumeStreamType");;
static String8 keyVolumeDevice        = String8("volumeDevice");
static String8 keyVolumeIndex         = String8("volumeIndex");
static String8 keySpeechBand          = String8("getSpeechBand");

static String8 keySET_AT_ACS = String8("SET_AT_ACS");  //for TC1 AT%ACS
static String8 keySET_AT_ACSVolume = String8("SET_AT_ACSVolume");  //for ACS volume

static String8 keyGainTable_Update = String8("UpdateHALCustGainTable");
static String8 keyMicGain_Update = String8("UpdateMicGain");

static String8 keyBtSupportVolume = String8("bt_headset_vgs");
#endif
//HTC_AUD_START
// For MFG 4 in 1 test ++
static String8 keySET_MFG_AUDIO_ROUTE = String8("mfg_audio_route");
static String8 keySET_MFG_AUDIO_ROUTE_REC_ID = String8("mfg_audio_route_rec_id");
// For MFG 4 in 1 test --
// For effect icon ++
const String8 keyEffectIcon = String8("effect_icon");
// For effect icon --
// global effect and HWAEC
const String8 keyGlobalEffect = String8("global_effect");
const String8 keyHTCHWAEC = String8("HTCHWAEC");
//HTC_AUD_END

enum
{
    Normal_Coef_Index,
    Headset_Coef_Index,
    Handfree_Coef_Index,
    VOIPBT_Coef_Index,
    VOIPNormal_Coef_Index,
    VOIPHandfree_Coef_Index,
    AUX1_Coef_Index,
    AuX2_Coef_Index
};

/*==============================================================================
 *                     setParameters() keys for common
 *============================================================================*/

AudioALSAHardware::AudioALSAHardware() :
    mStreamManager(AudioALSAStreamManager::getInstance()),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mAudioSpeechEnhanceInfoInstance(AudioSpeechEnhanceInfo::getInstance()),
    mAudioALSAParamTunerInstance(AudioALSAParamTuner::getInstance()),
    mSpeechANCControllerInstance(SpeechANCController::getInstance()),
    mSpeechPhoneCallController(AudioALSASpeechPhoneCallController::getInstance()),
    mAudioAlsaDeviceInstance(AudioALSADeviceParser::getInstance()),
    mFmTxEnable(false)
{
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    AudioALSASpeakerMonitor::getInstance()->EnableSpeakerMonitorThread(true);
#endif

    // TODO(Harvey)
    mNextUniqueId = 1;
    mUseTuningVolume = false;

    // Use Audio Patch For Fm
    char property_value[PROPERTY_VALUE_MAX];
    property_get("persist.af.audio_patch_fm", property_value, "0");
    mUseAudioPatchForFm = (bool)atoi(property_value);
//HTC_AUD_START
    ALOGD("%s(): init_acoustic_parameters", __FUNCTION__);
    init_acoustic_parameters();
//HTC_AUD_END
}

AudioALSAHardware::~AudioALSAHardware()
{
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    AudioALSASpeakerMonitor::getInstance()->EnableSpeakerMonitorThread(false);
#endif
    if (mStreamManager != NULL) { delete mStreamManager; }
}

status_t AudioALSAHardware::initCheck()
{
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAHardware::setVoiceVolume(float volume)
{
    ALOGD("%s(), volume = %f, mUseTuningVolume = %d", __FUNCTION__, volume, mUseTuningVolume);

    if (volume < 0.0 || volume > 1.0)
    {
        ALOGE("-%s(), strange volume level %f, something wrong!!", __FUNCTION__, volume);
        return BAD_VALUE;
    }

#ifndef MTK_AUDIO_GAIN_TABLE
    if (mUseTuningVolume == false)
    {
        int MapVolume = AudioALSAVolumeController::logToLinear(volume);
        int degradeDB = ((256 - MapVolume) / VOICE_ONEDB_STEP) * 2;
        if (degradeDB > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1)
        {
            degradeDB = VOICE_VOLUME_MAX / VOICE_ONEDB_STEP;
        }
        MapVolume = 256 - VOICE_ONEDB_STEP * degradeDB;
        volume = AudioALSAVolumeController::linearToLog(MapVolume);
    }
#endif

    return mStreamManager->setVoiceVolume(volume);
}

status_t AudioALSAHardware::setMasterVolume(float volume)
{
    return mStreamManager->setMasterVolume(volume);
}

// default implementation is unsupported
status_t AudioALSAHardware::getMasterVolume(float *volume)
{
    return INVALID_OPERATION;
}

status_t AudioALSAHardware::setMode(int mode)
{
    status_t status;
#if defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)
    bool sph_stream_support = mStreamManager->IsSphStrmSupport();
    status_t status_change;
    if (sph_stream_support)
    {
        status_change = mStreamManager->DisableSphStrm(static_cast<audio_mode_t>(mode));
    }
#endif
    status = mStreamManager->setMode(static_cast<audio_mode_t>(mode));
#if defined(MTK_SPEAKER_MONITOR_SPEECH_SUPPORT)
    if (sph_stream_support == true && status_change == NO_ERROR)
    {
        mStreamManager->EnableSphStrm(static_cast<audio_mode_t>(mode));
    }
#endif
    return status;
}

status_t AudioALSAHardware::setMicMute(bool state)
{
    return mStreamManager->setMicMute(state);
}

status_t AudioALSAHardware::getMicMute(bool *state)
{
    if (state != NULL) { *state = mStreamManager->getMicMute(); }
    return NO_ERROR;
}

status_t AudioALSAHardware::setParameters(const String8 &keyValuePairs)
{
    ALOGD("+%s(): %s", __FUNCTION__, keyValuePairs.string());
    AudioParameter param = AudioParameter(keyValuePairs);

    /// parse key value pairs
    status_t status = NO_ERROR;
    int value = 0;
    float value_float = 0.0;
    String8 value_str;
//HTC_AUD_START
// For MFG 4 in 1 test ++
    char values[32];
    int err = 0;
// For MFG 4 in 1 test --
// For Acoustic Shock Version 2 ++
#ifdef ENABLE_ACOUSTIC_SHOCK2
    if (param.get(String8("mute_upper_speaker"), value_str) == NO_ERROR) {
        param.remove(String8("mute_upper_speaker"));
        if (value_str == "enable") {
            AudioALSAHardwareResourceManager::getInstance()->setMuteUpperSpk(true);
        }
    }
    if (param.get(String8("unmute_upper_speaker"), value_str) == NO_ERROR) {
        param.remove(String8("unmute_upper_speaker"));
        if (value_str == "disable") {
            AudioALSAHardwareResourceManager::getInstance()->setMuteUpperSpk(false);
        }
    }
#endif
// For Acoustic Shock Version 2 --
#ifdef INCALL_RECORDING_ENABLED
    if (param.get(String8("INCALL_RECORDING_MODE"), value_str) == NO_ERROR) {
        param.remove(String8("INCALL_RECORDING_MODE"));
        if (value_str == "ON") {
            mStreamManager->setIncallRecording(true);
        } else {
            mStreamManager->setIncallRecording(false);
        }
    }
#endif
//HTC_AUD_ADD++    For Camera landscape/portrait mode recording
    if (param.get(keyCamcorderMode, value_str) == NO_ERROR) {
        param.remove(keyCamcorderMode);
        if (value_str == "ON") {
            AudioALSAHardwareResourceManager::getInstance()->setCamcoderMode(true);
            ALOGD("camcorder mode enabled.");
        } else {
            AudioALSAHardwareResourceManager::getInstance()->setCamcoderMode(false);
            AudioALSAHardwareResourceManager::getInstance()->setAudioCamcoderType(CAM_UNKNOWN);
            ALOGD("camcorder mode disabled.");
        }
    }
    if (param.get(keyDolbySrsEq, value_str) == NO_ERROR) {
        param.remove(keyDolbySrsEq);
        if (value_str == "Landscape_Right") {
            AudioALSAHardwareResourceManager::getInstance()->setAudioCamcoderType(CAM_RIGHT);
            ALOGD("Set camcorder Type CAM_RIGHT.");
        } else if (value_str == "Landscape_Left") {
            AudioALSAHardwareResourceManager::getInstance()->setAudioCamcoderType(CAM_LEFT);
            ALOGD("Set camcorder Type CAM_LEFT.");
        } else if (value_str == "Portrait") {
            AudioALSAHardwareResourceManager::getInstance()->setAudioCamcoderType(CAM_PORT);
            ALOGD("Set camcorder Type CAM_PORT.");
        } else if (value_str == "Mono") {
            AudioALSAHardwareResourceManager::getInstance()->setAudioCamcoderType(CAM_MONO);
            ALOGD("Set camcorder Type CAM_MONO.");
        } else {
            AudioALSAHardwareResourceManager::getInstance()->setAudioCamcoderType(CAM_UNKNOWN);
            ALOGE("Unknown camcoder type set to unknown type.");
        }
    }
//HTC_AUD_ADD--
    if (param.get(String8("DQ_dump"), value_str) == NO_ERROR) {
        param.remove(String8("DQ_dump"));
        ALOGI("DQ_dump = %s", value_str.string());
        SpeechVMRecorder::GetInstance()->DumpRAM();
    }
    if (param.get(String8("DQ_enable"), value_str) == NO_ERROR) {
        param.remove(String8("DQ_enable"));
        ALOGI("DQ_enable = %s", value_str.string());
        if (value_str == "true") {
            mStreamManager->SetSpeechVmEnable(1);
            SpeechVMRecorder::GetInstance()->OpenDq(1);
        } else if (value_str == "false") {
            mStreamManager->SetSpeechVmEnable(0);
            SpeechVMRecorder::GetInstance()->OpenDq(0);
        }
    }

#ifdef NXP_SMARTPA_SUPPORT
    if (param.getInt(String8("reset_tfa98xx"), value) == NO_ERROR) {
        param.remove(String8("reset_tfa98xx"));
        if (value) {
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_reset_tfa98xx();
        }
    }
#endif
//HTC_AUD_END
    //Tina temp for 6595 eccci fake
    if (param.getInt(String8("ECCCI_Test"), value) == NO_ERROR)
    {
        param.remove(String8("ECCCI_Test"));
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEnh1DownlinkGain(value);
    }

    if (param.getInt(String8("TDM_Record"), value) == NO_ERROR)
    {
        param.remove(String8("TDM_Record"));
        switch (value)
        {
            case 0:
            {
                setTDMRecord(false);
                break;
            }
            case 1:
            {
                setTDMRecord(44100);
                break;
            }
            default:
            {
                setTDMRecord(value);
                break;
            }
        }
    }


    // TODO(Harvey): test code, remove it later
    if (param.getInt(String8("SET_MODE"), value) == NO_ERROR)
    {
        param.remove(String8("SET_MODE"));

        switch (value)
        {
            case 0:
            {
                setMode(AUDIO_MODE_NORMAL);
                mStreamManager->getStreamOut(0)->setParameters(String8("routing=2"));
                break;
            }
            case 2:
            {
                mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
                setMode(AUDIO_MODE_IN_CALL);
                break;
            }
            case 5:
            {
                mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
                setMode(AUDIO_MODE_IN_CALL_EXTERNAL);
                break;
            }
            default:
            {
                break;
            }
        }
    }


    // TODO(Harvey): test code, remove it later
    if (param.getInt(String8("JT"), value) == NO_ERROR)
    {
        param.remove(String8("JT"));

        switch (value)
        {
            case 0:
            {
                setMode(AUDIO_MODE_NORMAL);
                break;
            }
            case 1:
            {
                mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
                setMode(AUDIO_MODE_IN_CALL);
                break;
            }
            case 2:
            {
                mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
                setMode(AUDIO_MODE_IN_CALL_EXTERNAL);
                break;
            }
            default:
            {
                break;
            }
        }
    }


    // TODO(Harvey): test code, remove it later
    if (param.getInt(String8("HAHA"), value) == NO_ERROR)
    {
        param.remove(String8("HAHA"));

        AudioFtm *mAudioFtm = AudioFtm::getInstance();
        static LoopbackManager *pLoopbackManager = LoopbackManager::GetInstance();
        loopback_output_device_t loopback_output_device = LOOPBACK_OUTPUT_RECEIVER; // default use receiver here

        switch (value)
        {
            case 0:
            {
                setMode(AUDIO_MODE_NORMAL);
                break;
            }
            case 1:
            {
                mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
                setMode(AUDIO_MODE_IN_CALL);
                break;
            }
            case 2:
            {
                mAudioFtm->RecieverTest(false);
                break;
            }
            case 3:
            {
                mAudioFtm->RecieverTest(true);
                break;
            }
            case 4:
            {
                mAudioFtm->LouderSPKTest(false, false);
                break;
            }
            case 5:
            {
                mAudioFtm->LouderSPKTest(true, true);
                break;
            }
            case 6:
            {
                mAudioFtm->EarphoneTest(false);
                break;
            }
            case 7:
            {
                mAudioFtm->EarphoneTest(true);
                break;
            }
            case 8:
            {
                mAudioFtm->PhoneMic_Receiver_Loopback(MIC1_OFF);
                break;
            }
            case 9:
            {
                mAudioFtm->PhoneMic_Receiver_Loopback(MIC1_ON);
                break;
            }
            case 10:
            {
                mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
                break;
            }
            case 11:
            {
                mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC1_ON);
                break;
            }
            case 12:
            {
                mAudioFtm->HeadsetMic_EarphoneLR_Loopback(false, false);
                break;
            }
            case 13:
            {
                mAudioFtm->HeadsetMic_EarphoneLR_Loopback(true, true);
                break;
            }
            case 14:
            {
                mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
                break;
            }
            case 15:
            {
                mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC2_ON);
                break;
            }
            case 16:
            {
                mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC3_OFF);
                break;
            }
            case 17:
            {
                mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC3_ON);
                break;
            }
            case 18:
            {
                mAudioFtm->PhoneMic_Receiver_Loopback(MIC2_OFF);
                break;
            }
            case 19:
            {
                mAudioFtm->PhoneMic_Receiver_Loopback(MIC2_ON);
                break;
            }
            case 20:
            {
                mAudioFtm->PhoneMic_Receiver_Loopback(MIC3_OFF);
                break;
            }
            case 21:
            {
                mAudioFtm->PhoneMic_Receiver_Loopback(MIC3_ON);
                break;
            }
            case 22:
                // close single mic acoustic loopback
                pLoopbackManager->SetLoopbackOff();
                break;
            case 23:
                // open dual mic acoustic loopback (w/o DMNR)
                pLoopbackManager->SetLoopbackOn(MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR, loopback_output_device);
                break;
            case 24:
                // close dual mic acoustic loopback
                pLoopbackManager->SetLoopbackOff();
                break;
            case 25:
                // open dual mic acoustic loopback (w/ DMNR)
                pLoopbackManager->SetLoopbackOn(MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR, loopback_output_device);
                break;

            default:
            {
                break;
            }
        }
    }

    // VT call (true) / Voice call (false)
    if (param.getInt(keySetVTSpeechCall, value) == NO_ERROR)
    {
        param.remove(keySetVTSpeechCall);
        mStreamManager->setVtNeedOn((bool)value);
    }

    // Set BGS DL Mute (true) / Unmute(false)
    if (param.getInt(keySet_BGS_DL_Mute, value) == NO_ERROR)
    {
        param.remove(keySet_BGS_DL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        mStreamManager->setBGSDlMute((bool)value);
    }

    // Set BGS UL Mute (true) / Unmute(false)
    if (param.getInt(keySet_BGS_UL_Mute, value) == NO_ERROR)
    {
        param.remove(keySet_BGS_UL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        mStreamManager->setBGSUlMute((bool)value);
    }

    // Set SpeechCall DL Mute (true) / Unmute(false)
    if (param.getInt(keySet_SpeechCall_DL_Mute, value) == NO_ERROR)
    {
        param.remove(keySet_SpeechCall_DL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        mSpeechPhoneCallController->setDlMute((bool)value);
    }

    // Set SpeechCall UL Mute (true) / Unmute(false)
    if (param.getInt(keySet_SpeechCall_UL_Mute, value) == NO_ERROR)
    {
        param.remove(keySet_SpeechCall_UL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        mSpeechPhoneCallController->setUlMute((bool)value);
    }

    // FM enable
    if (param.getInt(keySetFmEnable, value) == NO_ERROR)
    {
        param.remove(keySetFmEnable);
        if (mUseAudioPatchForFm == false)
        {
            mStreamManager->setFmEnable((bool)value);
        }
    }

//HTC_AUD_START    For HTC FM Function
    // FM enable
    if (param.getInt(keyHTCSetFmEnable, value) == NO_ERROR)
    {
        param.remove(keyHTCSetFmEnable);
        if (mUseAudioPatchForFm == false)
        {
            mStreamManager->setFmEnable((bool)value);
        }
    }
//HTC_AUD_END

    // Set FM volume
    if (param.getFloat(keySetFmVolume, value_float) == NO_ERROR)
    {
        param.remove(keySetFmVolume);
        if (mUseAudioPatchForFm == false)
        {
            mStreamManager->setFmVolume(value_float);
        }
    }

    // Set FM Tx enable
    if (param.getInt(keySetFmTxEnable, value) == NO_ERROR)
    {
        param.remove(keySetFmTxEnable);
        mFmTxEnable = (bool)value;
    }

    // Force dusable FM Tx due to FM Rx is ready to play
    if (param.getInt(keyFMRXForceDisableFMTX, value) == NO_ERROR)
    {
        param.remove(keyFMRXForceDisableFMTX);
        if (value == true)
        {
            mFmTxEnable = false;
        }
    }


    if (param.getInt(keyMusicPlusSet, value) == NO_ERROR)
    {
        param.remove(keyMusicPlusSet);
        mStreamManager->SetMusicPlusStatus(value ? true : false);
    }

    // set the LR channel switch
    if (param.getInt(keyLR_ChannelSwitch, value) == NO_ERROR)
    {
        ALOGD("keyLR_ChannelSwitch=%d", value);
        bool bIsLRSwitch = value;
//HTC_AUD_START remove this feature, camera AP should use the camcorder parameter to do this
        //AudioALSAHardwareResourceManager::getInstance()->setMicInverse(bIsLRSwitch);
        ALOGD("Skip the setMicInverse due to HTC camera should use CAMCORDER_MODE parameter to control mic");
//HTC_AUD_END
        param.remove(keyLR_ChannelSwitch);
    }

    // BesRecord Mode setting
    if (param.getInt(keyHDREC_SET_VOICE_MODE, value) == NO_ERROR)
    {
        ALOGD("HDREC_SET_VOICE_MODE=%d", value); // Normal, Indoor, Outdoor,
        param.remove(keyHDREC_SET_VOICE_MODE);
        //Get and Check Voice/Video Mode Offset
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (value < hdRecordSceneTable.num_voice_rec_scenes)
        {
            int32_t BesRecScene = value + 1;//1:cts verifier offset
            mAudioSpeechEnhanceInfoInstance->SetBesRecScene(BesRecScene);
        }
        else
        {
            ALOGE("HDREC_SET_VOICE_MODE=%d exceed max value(%d)\n", value, hdRecordSceneTable.num_voice_rec_scenes);
        }
    }

    if (param.getInt(keyHDREC_SET_VIDEO_MODE, value) == NO_ERROR)
    {
        ALOGD("HDREC_SET_VIDEO_MODE=%d", value); // Normal, Indoor, Outdoor,
        param.remove(keyHDREC_SET_VIDEO_MODE);
        //Get and Check Voice/Video Mode Offset
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (value < hdRecordSceneTable.num_video_rec_scenes)
        {
            uint32_t offset = hdRecordSceneTable.num_voice_rec_scenes + 1;//1:cts verifier offset
            int32_t BesRecScene = value + offset;
            mAudioSpeechEnhanceInfoInstance->SetBesRecScene(BesRecScene);
        }
        else
        {
            ALOGE("HDREC_SET_VIDEO_MODE=%d exceed max value(%d)\n", value, hdRecordSceneTable.num_video_rec_scenes);
        }
    }

#ifdef BTNREC_DECIDED_BY_DEVICE
    // BT NREC on/off
    if (param.get(keyBtHeadsetNrec, value_str) == NO_ERROR)
    {
        param.remove(keyBtHeadsetNrec);
        if (value_str == "on")
        {
            mStreamManager->SetBtHeadsetNrec(true);
        }
        else if (value_str == "off")
        {
            mStreamManager->SetBtHeadsetNrec(false);
        }
    }
#endif
    if (param.get(keySetBTMode, value_str) == NO_ERROR)
    {
        param.remove(keySetBTMode);

        ALOGD("%s(), setBTMode = %d", __FUNCTION__, value);
        if (value_str == "on")
        {
            WCNChipController::GetInstance()->SetBTCurrentSamplingRateNumber(16000);
            AudioBTCVSDControl::getInstance()->BT_SCO_SetMode(true);
            mSpeechPhoneCallController->setBTMode(true);
        }
        else if (value_str == "off")
        {
            WCNChipController::GetInstance()->SetBTCurrentSamplingRateNumber(8000);
            AudioBTCVSDControl::getInstance()->BT_SCO_SetMode(false);
            mSpeechPhoneCallController->setBTMode(false);
        }
    }

    //<---for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)
    // calibrate speech parameters
    if (param.getInt(keySpeechParams_Update, value) == NO_ERROR)
    {
        ALOGD("setParameters Update Speech Parames");

        mStreamManager->UpdateSpeechParams(value);
        param.remove(keySpeechParams_Update);
    }
#if defined(MTK_DUAL_MIC_SUPPORT)
    if (param.getInt(keyDualMicParams_Update, value) == NO_ERROR)
    {
        param.remove(keyDualMicParams_Update);
        mStreamManager->UpdateDualMicParams();
    }
#endif
//HTC_AUD_START
#if defined(MTK_DUAL_MIC_SUPPORT)
    if (param.getInt(keyForceUseSpecificMicData, value) == NO_ERROR)
    {
        ALOGD("setParameters keyForceUseSpecificMicData %d", value);
        param.remove(keyForceUseSpecificMicData);
        AudioALSAHardwareResourceManager::getInstance()->setBuiltInMicSpecificType((builtin_mic_specific_type)value);
    }
#endif
//HTC_AUD_END

#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
    if (param.getInt(keyMagiConParams_Update, value) == NO_ERROR)
    {
        param.remove(keyMagiConParams_Update);
        mStreamManager->UpdateMagiConParams();
    }
#endif

#if defined(MTK_HAC_SUPPORT)
    if (param.getInt(keyHACParams_Update, value) == NO_ERROR)
    {
        param.remove(keyHACParams_Update);
        mStreamManager->UpdateHACParams();
    }
#endif

    // calibrate speech volume
    if (param.getInt(keySpeechVolume_Update, value) == NO_ERROR)
    {
        ALOGD("setParameters Update Speech volume");
        mStreamManager->UpdateSpeechVolume();
        param.remove(keySpeechVolume_Update);
    }

    if (param.getInt(keyBesLoudnessSet, value) == NO_ERROR)
    {
        param.remove(keyBesLoudnessSet);
        mStreamManager->SetBesLoudnessStatus(value ? true : false);
    }

    // ACF/HCF parameters calibration
    if (param.getInt(keyACFHCF_Update, value) == NO_ERROR)
    {
        param.remove(keyACFHCF_Update);
        mStreamManager->UpdateACFHCF(value);
    }
    // HD recording and DMNR calibration
#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (param.getInt(keyDualMicRecPly, value) == NO_ERROR)
    {
        unsigned short cmdType = value & 0x000F;
        bool bWB = (value >> 4) & 0x000F;
        status_t ret = NO_ERROR;
        switch (cmdType)
        {
                //dmnr tunning at ap side
            case DUAL_MIC_REC_PLAY_STOP:
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(false, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_MODE);
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(false);
                break;
            case DUAL_MIC_REC:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECONLY_MODE);
                break;
            case DUAL_MIC_REC_PLAY:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_MODE);
                break;
            case DUAL_MIC_REC_PLAY_HS:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_HEADSET, RECPLAY_MODE);
                break;
            case DUAL_MIC_REC_HF:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECONLY_HF_MODE);
                break;
            case DUAL_MIC_REC_PLAY_HF:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_HF_MODE);
                break;
            case DUAL_MIC_REC_PLAY_HF_HS:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_HEADSET, RECPLAY_HF_MODE);
                break;
            default:
                ret = BAD_VALUE;
                break;
        }
        if (ret == NO_ERROR)
        {
            param.remove(keyDualMicRecPly);
        }
    }

    if (param.get(keyDUALMIC_IN_FILE_NAME, value_str) == NO_ERROR)
    {
        if (mAudioALSAParamTunerInstance->setPlaybackFileName(value_str.string()) == NO_ERROR)
        {
            param.remove(keyDUALMIC_IN_FILE_NAME);
        }
    }

    if (param.get(keyDUALMIC_OUT_FILE_NAME, value_str) == NO_ERROR)
    {
        if (mAudioALSAParamTunerInstance->setRecordFileName(value_str.string()) == NO_ERROR)
        {
#ifndef DMNR_TUNNING_AT_MODEMSIDE
            if (mAudioSpeechEnhanceInfoInstance->SetBesRecVMFileName(value_str.string()) == NO_ERROR)
#endif
                param.remove(keyDUALMIC_OUT_FILE_NAME);
        }
    }

    if (param.getInt(keyDUALMIC_SET_UL_GAIN, value) == NO_ERROR)
    {
        if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_MIC_GAIN, value) == NO_ERROR)
        {
            param.remove(keyDUALMIC_SET_UL_GAIN);
        }
    }

    if (param.getInt(keyDUALMIC_SET_DL_GAIN, value) == NO_ERROR)
    {
        if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_RECEIVER_GAIN, value) == NO_ERROR)
        {
            param.remove(keyDUALMIC_SET_DL_GAIN);
        }
    }

    if (param.getInt(keyDUALMIC_SET_HSDL_GAIN, value) == NO_ERROR)
    {
        if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_HS_GAIN, value) == NO_ERROR)
        {
            param.remove(keyDUALMIC_SET_HSDL_GAIN);
        }
    }

    if (param.getInt(keyDUALMIC_SET_UL_GAIN_HF, value) == NO_ERROR)
    {
        if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_MIC_GAIN_HF, value) == NO_ERROR)
        {
            param.remove(keyDUALMIC_SET_UL_GAIN_HF);
        }
    }
#endif

    if (param.getInt(keyHDRecTunningEnable, value) == NO_ERROR)
    {
        ALOGD("keyHDRecTunningEnable=%d", value);
        bool bEnable = value;
        mAudioSpeechEnhanceInfoInstance->SetBesRecTuningEnable(bEnable);
        param.remove(keyHDRecTunningEnable);
    }

    if (param.get(keyHDRecVMFileName, value_str) == NO_ERROR)
    {
        ALOGD("keyHDRecVMFileName=%s", value_str.string());
        if (mAudioSpeechEnhanceInfoInstance->SetBesRecVMFileName(value_str.string()) == NO_ERROR)
        {
            param.remove(keyHDRecVMFileName);
        }
    }
    // --->for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)

#if defined(MTK_DUAL_MIC_SUPPORT)
    // Dual Mic Noise Reduction, DMNR for Receiver
    if (param.getInt(keyEnable_Dual_Mic_Setting, value) == NO_ERROR)
    {
        param.remove(keyEnable_Dual_Mic_Setting);
        mStreamManager->Enable_DualMicSettng(SPH_ENH_DYNAMIC_MASK_DMNR, (bool) value);
    }

    // Dual Mic Noise Reduction, DMNR for Loud Speaker
    if (param.getInt(keySET_LSPK_DMNR_ENABLE, value) == NO_ERROR)
    {
        param.remove(keySET_LSPK_DMNR_ENABLE);
        mStreamManager->Set_LSPK_DlMNR_Enable(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, (bool) value);

    }

    // VoIP Dual Mic Noise Reduction, DMNR for Receiver
    if (param.getInt(keySET_VOIP_RECEIVER_DMNR_ENABLE, value) == NO_ERROR)
    {
        param.remove(keySET_VOIP_RECEIVER_DMNR_ENABLE);
        mAudioSpeechEnhanceInfoInstance->SetDynamicVoIPSpeechEnhancementMask(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR, (bool)value);
    }

    // VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
    if (param.getInt(keySET_VOIP_LSPK_DMNR_ENABLE, value) == NO_ERROR)
    {
        param.remove(keySET_VOIP_LSPK_DMNR_ENABLE);
        mAudioSpeechEnhanceInfoInstance->SetDynamicVoIPSpeechEnhancementMask(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, (bool)value);
    }

#endif

    // Voice Clarity Engine, VCE
    if (param.getInt(keySET_VCE_ENABLE, value) == NO_ERROR)
    {
        param.remove(keySET_VCE_ENABLE);
        mStreamManager->SetVCEEnable((bool) value);
    }

#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
    // Magic Conference Call
    if (param.getInt(keySET_MAGI_CONFERENCE_ENABLE, value) == NO_ERROR)
    {
        param.remove(keySET_MAGI_CONFERENCE_ENABLE);
        mStreamManager->SetMagiConCallEnable((bool) value);
    }
#endif

#if defined(MTK_HAC_SUPPORT)
    // HAC
    if (param.get(keySET_HAC_ENABLE, value_str) == NO_ERROR)
    {
        param.remove(keySET_HAC_ENABLE);

        if (value_str == "ON")
        {
            mStreamManager->SetHACEnable(true);
        }
        else if (value_str == "OFF")
        {
            mStreamManager->SetHACEnable(false);
        }
    }

#endif

    if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
    {
        if (param.getInt(keySET_VIBSPK_ENABLE, value) == NO_ERROR)
        {
            param.remove(keySET_VIBSPK_ENABLE);
            AudioVIBSPKControl::getInstance()->setVibSpkEnable((bool)value);
            ALOGD("setParameters VibSPK!!, %x", value);
        }
        if (param.getInt(keySET_VIBSPK_RAMPDOWN, value) == NO_ERROR)
        {
            AudioFtmBase *mAudioFtm = AudioFtmBase::createAudioFtmInstance();
            param.remove(keySET_VIBSPK_RAMPDOWN);
            mAudioFtm->SetVibSpkRampControl(value);
            ALOGD("setParameters VibSPK_Rampdown!!, %x", value);
        }
    }

#ifdef MTK_VOW_SUPPORT
    // Voice Wake Up
    if (param.getInt(keyMTK_VOW_ENABLE, value) == NO_ERROR)
    {
        param.remove(keyMTK_VOW_ENABLE);
        mStreamManager->setVoiceWakeUpNeedOn((bool)value);
    }

    if (param.get(keyACCESS_VOW_PARAM, value_str) == NO_ERROR)
    {
        param.remove(keyACCESS_VOW_PARAM);
        //parse format
        int par_type = -1;
        int par_value = -1;
        sscanf(value_str.string(), "%d,%d", &par_type, &par_value);
        if ((par_type < 1) || (par_type > 10))
        {
            ALOGE("%s, wrong index, par_type = %d, par_value = %d", keyACCESS_VOW_PARAM.string(), par_type, par_value);
        }
        else
        {
            ALOGD("%s, par_type = %d, par_value = %d", keyACCESS_VOW_PARAM.string(), par_type, par_value);
            AudioALSAVoiceWakeUpController::getInstance()->SetVOWCustParam(par_type - 1, par_value);
        }
    }

    if (param.get(keyACCESS_VOW_KR_PARAM, value_str) == NO_ERROR)
    {
        param.remove(keyACCESS_VOW_KR_PARAM);
        //parse format
        int par_type = -1;
        int par_value = -1;
        sscanf(value_str.string(), "%d,%d", &par_type, &par_value);
        if ((par_type < 1) || (par_type > 4))
        {
            ALOGE("%s, wrong index, par_type = %d, par_value = %d", keyACCESS_VOW_KR_PARAM.string(), par_type, par_value);
        }
        else
        {
            ALOGD("%s, par_type = %d, par_value = %d", keyACCESS_VOW_KR_PARAM.string(), par_type, par_value);
            AudioALSAVoiceWakeUpController::getInstance()->SetVOWCustParam(par_type + 9, par_value);
        }
    }

    if (param.get(keyACCESS_VOW_KRSR_PARAM, value_str) == NO_ERROR)
    {
        param.remove(keyACCESS_VOW_KRSR_PARAM);
        //parse format
        int par_type = -1;
        int par_value = -1;
        sscanf(value_str.string(), "%d,%d", &par_type, &par_value);
        if ((par_type < 1) || (par_type > 4))
        {
            ALOGE("%s, wrong index, par_type = %d, par_value = %d", keyACCESS_VOW_KRSR_PARAM.string(), par_type, par_value);
        }
        else
        {
            ALOGD("%s, par_type = %d, par_value = %d", keyACCESS_VOW_KRSR_PARAM.string(), par_type, par_value);
            //[Todo]Update value in nvram and in VOWcontroller
            AudioALSAVoiceWakeUpController::getInstance()->SetVOWCustParam(par_type + 17, par_value);
        }
    }
#endif

#ifdef  MTK_TTY_SUPPORT
    // Set TTY mode
    if (param.get(keySetTtyMode, value_str) == NO_ERROR)
    {
        param.remove(keySetTtyMode);
        tty_mode_t tty_mode;

        if (value_str == "tty_full")
        {
            tty_mode = AUD_TTY_FULL;
        }
        else if (value_str == "tty_vco")
        {
            tty_mode = AUD_TTY_VCO;
        }
        else if (value_str == "tty_hco")
        {
            tty_mode = AUD_TTY_HCO;
        }
        else if (value_str == "tty_off")
        {
            tty_mode = AUD_TTY_OFF;
        }
        else
        {
            ALOGD("setParameters tty_mode error !!");
            tty_mode = AUD_TTY_ERR;
        }
        mSpeechPhoneCallController->setTtyMode(tty_mode);
    }
#endif

    // Low latency mode
    if (param.get(keySCREEN_STATE, value_str) == NO_ERROR)
    {
        param.remove(keySCREEN_STATE);
        //Diable low latency mode for 32bits audio task
#ifndef MTK_DYNAMIC_CHANGE_HAL_BUFFER_SIZE
        setLowLatencyMode(value_str == "on");
#endif
    }

    // Loopback use speaker or not
    static bool bForceUseLoudSpeakerInsteadOfReceiver = false;
    if (param.getInt(keySET_LOOPBACK_USE_LOUD_SPEAKER, value) == NO_ERROR)
    {
        param.remove(keySET_LOOPBACK_USE_LOUD_SPEAKER);
        bForceUseLoudSpeakerInsteadOfReceiver = value & 0x1;
    }

    // Assign delay frame for modem loopback // 1 frame = 20ms
    if (param.getInt(keySET_LOOPBACK_MODEM_DELAY_FRAMES, value) == NO_ERROR)
    {
        param.remove(keySET_LOOPBACK_MODEM_DELAY_FRAMES);
        SpeechDriverInterface *pSpeechDriver = NULL;
        for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++)
        {
            pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex((modem_index_t)modem_index);
            if (pSpeechDriver != NULL) // Might be single talk and some speech driver is NULL
            {
                pSpeechDriver->SetAcousticLoopbackDelayFrames((int32_t)value);
            }
        }
    }

    // Loopback
    if (param.get(keySET_LOOPBACK_TYPE, value_str) == NO_ERROR)
    {
        param.remove(keySET_LOOPBACK_TYPE);

        // parse format like "SET_LOOPBACK_TYPE=1" / "SET_LOOPBACK_TYPE=1+0"
        int type_value = NO_LOOPBACK;
        int device_value = -1;
        sscanf(value_str.string(), "%d,%d", &type_value, &device_value);
        ALOGV("type_value = %d, device_value = %d", type_value, device_value);

        const loopback_t loopback_type = (loopback_t)type_value;
        loopback_output_device_t loopback_output_device;

        if (loopback_type == NO_LOOPBACK) // close loopback
        {
            LoopbackManager::GetInstance()->SetLoopbackOff();
        }
        else // open loopback
        {
            if (device_value == LOOPBACK_OUTPUT_RECEIVER ||
                device_value == LOOPBACK_OUTPUT_EARPHONE ||
                device_value == LOOPBACK_OUTPUT_SPEAKER) // assign output device
            {
                loopback_output_device = (loopback_output_device_t)device_value;
            }
            else // not assign output device
            {
                if (AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADSET,   "") == android_audio_legacy::AudioSystem::DEVICE_STATE_AVAILABLE ||
                    AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADPHONE, "") == android_audio_legacy::AudioSystem::DEVICE_STATE_AVAILABLE)
                {
                    loopback_output_device = LOOPBACK_OUTPUT_EARPHONE;
                }
                else if (bForceUseLoudSpeakerInsteadOfReceiver == true)
                {
                    loopback_output_device = LOOPBACK_OUTPUT_SPEAKER;
                }
                else
                {
                    loopback_output_device = LOOPBACK_OUTPUT_RECEIVER;
                }
            }
            LoopbackManager::GetInstance()->SetLoopbackOn(loopback_type, loopback_output_device);
        }
    }

//HTC_AUD_START
#ifdef NXP_SMARTPA_SUPPORT
    //Remember to porting all of them.
    {//HTC_AUD_START - TFA config
    bool checkTFAsetting = false;
    if (param.get(String8("VOICE_PLAYING_MODE"), value_str) == NO_ERROR) {
        param.remove(String8("VOICE_PLAYING_MODE"));
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->set_tfa_status(TFA_STATUS_RECORDER, (value_str == "ON")?true:false);
    }
    if (param.get(String8("VIDEO_PLAYING"), value_str) == NO_ERROR) {
        param.remove(String8("VIDEO_PLAYING"));
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->set_tfa_status(TFA_STATUS_VIDEO, (value_str == "ON")?true:false);
    }
    if (param.get(String8("NOTES_PLAYING_MODE"), value_str) == NO_ERROR) {
        param.remove(String8("NOTES_PLAYING_MODE"));
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->set_tfa_status(TFA_STATUS_NOTE, (value_str == "ON")?true:false);
    }
    }//HTC_AUD_END - TFA config
#endif

#ifdef ENABLE_HTC_SUBWOOFER
    if (param.get(String8("htc_boom_base"), value_str) == NO_ERROR) {
        param.remove(String8("htc_boom_base"));
        if (value_str == "ON") {
            ALOGD("[Subwoofer] device connect");
            property_set("htc.audio.subwoofer", "1");
            property_set("htc.audio.subwoofer.state", "0");
        } else {
            ALOGD("[Subwoofer] disconnect or non-subwoofer connect");
            property_set("htc.audio.subwoofer", "0");
            mStreamManager->setSubWooferEnable(false);
        }
#ifdef NXP_SMARTPA_SUPPORT
        AudioALSACodecDeviceOutSpeakerNXP::getInstance()->set_tfa_status(TFA_STATUS_WOOFER, (value_str == "ON")?true:false);
#endif
    }
    char subwoofer_state[PROPERTY_VALUE_MAX] = {0};
    property_get("htc.audio.subwoofer", subwoofer_state, "0");
    if (atoi(subwoofer_state) == 1){
        if (param.get(String8("audio_start"), value_str) == NO_ERROR) {
            if (value_str == "ON") {
                ALOGD("[Subwoofer] audio start");
                mStreamManager->setSubWooferEnable(true);
            } else {
               ALOGD("[Subwoofer] audio stop");
               mStreamManager->setSubWooferEnable(false);
            }
       }
   } else {
   //    ALOGI("[Subwoofer didn't connect]");
   }
#endif

#ifdef NXP_SMARTPA_SUPPORT
// htc_audio ++
// For MFG test L/R Speaker API ++
    if (param.getInt(keyTEST_LR_SPEAKER, value) == NO_ERROR)
    {
        param.remove(keyTEST_LR_SPEAKER);
        if (value == 0) // test R speaker
        {
            // unmute R speaker
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false, false);
            // mute L speaker
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(true, true);
        } else if (value == 1) // test L speaker
        {
            // unmute L speaker
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false, true);
            // mute R speaker
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(true, false);
        } else if (value == 2) // test both L and R speaker
        {
            // unmute L speaker
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false, true);
            // unmute R speaker
            AudioALSACodecDeviceOutSpeakerNXP::getInstance()->NXP_tfa_volume_mute(false, false);
        }
    }
// For MFG test L/R Speaker API --
#endif
//HTC_AUD_END

#ifdef ENABLE_HTC_SUBWOOFER
    if (param.get(String8("htc_boom_base"), value_str) == NO_ERROR) {
        param.remove(String8("htc_boom_base"));
        if (value_str == "ON") {
            ALOGD("[Subwoofer] device connect");
            property_set("htc.audio.subwoofer", "1");
            property_set("htc.audio.subwoofer.state", "0");
        } else {
            ALOGD("[Subwoofer] disconnect or non-subwoofer connect");
            property_set("htc.audio.subwoofer", "0");
        }
    }
#endif

#ifdef MTK_AUDIO_GAIN_TABLE
    if (param.getInt(keyVolumeStreamType, value) == NO_ERROR)
    {
        int device = 0;
        int index = 0;
        if (param.getInt(keyVolumeDevice, device) == NO_ERROR)
        {
            if (param.getInt(keyVolumeIndex, index) == NO_ERROR)
            {
                //mSetModeLock.lock();
                mStreamManager->setAnalogVolume(value, device, index, 0);
                // mSetModeLock.unlock();
            }
        }
        param.remove(keyVolumeStreamType);
        param.remove(keyVolumeDevice);
        param.remove(keyVolumeIndex);
    }

    // Set ACS volume
    if (param.get(keySET_AT_ACSVolume, value_str) == NO_ERROR)
    {
        param.remove(keySET_AT_ACSVolume);

        sscanf(value_str.string(), "%d", &value);
        mAudioALSAVolumeController->setAnalogVolume(0, AudioALSAHardwareResourceManager::getInstance()->getOutputDevice(), value, (audio_mode_t)AUDIO_MODE_IN_CALL);
    }

    // TC1 AT%ACS
    if (param.get(keySET_AT_ACS, value_str) == NO_ERROR)
    {
        param.remove(keySET_AT_ACS);

        int acs_value = -1;
        sscanf(value_str.string(), "%d", &acs_value);
        uint32_t current_device;

        switch (acs_value)
        {
            case 0:  // turn off loopback
                LoopbackManager::GetInstance()->SetLoopbackOff();
                break;
            case 1:   // turn on loopback by current device status
                current_device = AudioALSAHardwareResourceManager::getInstance()->getOutputDevice();
                if (current_device == AUDIO_DEVICE_OUT_WIRED_HEADSET)
                {
                    LoopbackManager::GetInstance()->SetLoopbackOn(MD_HEADSET_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
                }
                else if (current_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
                {
                    LoopbackManager::GetInstance()->SetLoopbackOn(MD_MAIN_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
                }
                else
                {
                    LoopbackManager::GetInstance()->SetLoopbackOn(MD_MAIN_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
                }
                break;
            case 2:
                LoopbackManager::GetInstance()->SetLoopbackOn(MD_HEADSET_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
                break;
            case 3:
                LoopbackManager::GetInstance()->SetLoopbackOn(MD_MAIN_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
                break;
            case 5:
                AudioALSAHardwareResourceManager::getInstance()->changeOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
                break;
            case 6:
                AudioALSAHardwareResourceManager::getInstance()->changeOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
                break;
            case 7:
                AudioALSAHardwareResourceManager::getInstance()->changeOutputDevice(AUDIO_DEVICE_OUT_SPEAKER);
                break;
        }
    }

    if (param.getInt(keyGainTable_Update, value) == NO_ERROR)
    {
        ALOGD("setParameters Update Gain Table");
        mAudioALSAVolumeController->initVolumeController();
        param.remove(keyGainTable_Update);
    }
    if (param.getInt(keyMicGain_Update, value) == NO_ERROR)
    {
        ALOGD("setParameters Update Mic Gain");
        //mAudioResourceManager->SetInputDeviceGain();
        mStreamManager->SetCaptureGain();
        param.remove(keyMicGain_Update);
    }

    //BT VGS feature +
    if (param.get(keyBtSupportVolume, value_str) == NO_ERROR)
    {
        param.remove(keyBtSupportVolume);
        if (value_str == "on")
        {
            mAudioALSAVolumeController->setBtVolumeCapability(true);
        }
        else if (value_str == "off")
        {
            mAudioALSAVolumeController->setBtVolumeCapability(false);
        }
    }
#endif
//HTC_AUD_START
// For MFG 4 in 1 test ++
    if (param.get(keySET_MFG_AUDIO_ROUTE, value_str) == NO_ERROR) {
        if (!strcmp(value_str, "headset")) {
            ALOGD("%s MFG, HEADSET", __func__);
            // route to AUDIO_DEVICE_OUT_WIRED_HEADSET
            mStreamManager->getStreamOut(0)->setSuspend(true);
            mStreamManager->getStreamOut(0)->setParameters(String8("routing=4"));
            mStreamManager->getStreamOut(0)->setSuspend(false);
        } else if (!strcmp(value_str, "handset")) {
            ALOGD("%s MFG, HANDSET", __func__);
            // route to AUDIO_DEVICE_OUT_EARPIECE
            mStreamManager->getStreamOut(0)->setSuspend(true);
            mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
            mStreamManager->getStreamOut(0)->setSuspend(false);
        } else if (!strcmp(value_str, "speaker")) {
            ALOGD("%s MFG, SPEAKER", __func__);
            // route to AUDIO_DEVICE_OUT_SPEAKER
            mStreamManager->getStreamOut(0)->setSuspend(true);
            mStreamManager->getStreamOut(0)->setParameters(String8("routing=2"));
            mStreamManager->getStreamOut(0)->setSuspend(false);
        } else if (!strcmp(value_str, "main_mic")) {
            ALOGD("%s MFG, MAIN_MIC", __func__);
            // route to AUDIO_DEVICE_IN_BUILTIN_MIC (main mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483652"));
                AudioALSAHardwareResourceManager::getInstance()->setBuiltInMicSpecificType(BUILTIN_MIC_MIC1_ONLY);
                mStreamManager->getStreamIn(0)->setSuspend(false);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else if (!strcmp(value_str, "back_mic")) {
            ALOGD("%s MFG, BACK_MIC", __func__);
            // route to AUDIO_DEVICE_IN_BACK_MIC (2nd mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483776"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else {
            ALOGD("%s MFG, NOT SUPPORTED", __func__);
        }
        param.remove(keySET_MFG_AUDIO_ROUTE);
    }

    if(param.get(keySET_MFG_AUDIO_ROUTE_REC_ID, value_str) == NO_ERROR) {
        if (!strcmp(value_str, "0")) {
            ALOGD("%s MFG, rec_id = 0", __func__);
            // route to AUDIO_DEVICE_IN_BUILTIN_MIC (main mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483652"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else if (!strcmp(value_str, "1")) {
            ALOGD("%s MFG, rec_id = 1", __func__);
            // route to AUDIO_DEVICE_IN_BUILTIN_MIC (main mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483652"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else if (!strcmp(value_str, "2")) {
            ALOGD("%s MFG, rec_id = 2", __func__);
            // route to AUDIO_DEVICE_IN_WIRED_HEADSET (hs mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483664"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else if (!strcmp(value_str, "13")) {
            ALOGD("%s MFG, rec_id = 13", __func__);
            // route to AUDIO_DEVICE_IN_BACK_MIC (2nd mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483776"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else if (!strcmp(value_str, "21")) {
            ALOGD("%s MFG, rec_id = 21", __func__);
            // route to AUDIO_DEVICE_IN_BUILTIN_MIC (main mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483652"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
                // Start new input device if StreamIn is not ready
                if (mStreamManager->getStreamIn(0)->CheckStandby()) {
                    char pbuffer[4];
                    mStreamManager->getStreamIn(0)->read(pbuffer, sizeof(pbuffer));
                }
                // Apply mini-box setting
                mAudioALSAVolumeController->ApplyMicGain(Idle_Video_Record_Handset, AUDIO_MODE_NORMAL);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else if (!strcmp(value_str, "22")) {
            ALOGD("%s MFG, rec_id = 22", __func__);
            // route to AUDIO_DEVICE_IN_BACK_MIC (2nd mic)
            if (mStreamManager->getStreamInVectorSize() > 0) {
                mStreamManager->getStreamIn(0)->setSuspend(true);
                mStreamManager->getStreamIn(0)->setParameters(String8("routing=2147483776"));
                mStreamManager->getStreamIn(0)->setSuspend(false);
                // Start new input device if StreamIn is not ready
                if (mStreamManager->getStreamIn(0)->CheckStandby()) {
                    char pbuffer[4];
                    mStreamManager->getStreamIn(0)->read(pbuffer, sizeof(pbuffer));
                }
                // Apply mini-box setting
                mAudioALSAVolumeController->ApplyMicGain(Voice_Rec_Mic_Handset, AUDIO_MODE_NORMAL);
            } else
                ALOGE("%s, no valid input for switch mfg path", __func__);
        } else {
            ALOGD("%s MFG, Not Support", __func__);
        }
        param.remove(keySET_MFG_AUDIO_ROUTE_REC_ID);
    }
// For MFG 4 in 1 test --

// For effect icon ++
    if (param.get(keyEffectIcon, value_str) == NO_ERROR)
    {
        param.remove(keyEffectIcon);
        if (value_str == "on") {
            update_effect_icon(1);
            ALOGI("Enable Effect Icon");
        } else {
            update_effect_icon(0);
            ALOGI("Disable Effect Icon");
        }
    }
// For effect icon --
//HTC_AUD_END

//HTC_AUD_START - global effect and HWAEC
    if (param.get(keyGlobalEffect, value_str) == NO_ERROR)
    {
        param.remove(keyGlobalEffect);
        int flag = AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag();
        if (value_str == "SRS") {
            flag |= SRS_EFFECT;
            AudioALSAHardwareResourceManager::getInstance()->setAudioUsecaseFlag(flag);
            ALOGI("%s: SRS ON, flag 0x%x", __FUNCTION__, AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag());
        } else {
            flag &= (~SRS_EFFECT);
            AudioALSAHardwareResourceManager::getInstance()->setAudioUsecaseFlag(flag);
            ALOGI("%s: SRS OFF, flag 0x%x", __FUNCTION__, AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag());
        }

        if (has_rt550x()) {
            set_rt55xx_amp(AudioALSAStreamManager::getInstance()->getMode(),
                           AudioALSAHardwareResourceManager::getInstance()->getOutputDevice(),
                           AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag());
        } // TODO: if there is no rt550x
    }

    if (param.get(keyHTCHWAEC, value_str) == NO_ERROR)
    {
        param.remove(keyHTCHWAEC);
        int flag = AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag();
        if (value_str == "ON") {
            flag |= HW_AEC_FLAG;
            AudioALSAHardwareResourceManager::getInstance()->setAudioUsecaseFlag(flag);
            ALOGI("%s: HTCHWAEC ON, flag 0x%x", __FUNCTION__, AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag());
        } else {
            flag &= (~HW_AEC_FLAG);
            AudioALSAHardwareResourceManager::getInstance()->setAudioUsecaseFlag(flag);
            ALOGI("%s: HTCHWAEC OFF, flag 0x%x", __FUNCTION__, AudioALSAHardwareResourceManager::getInstance()->getAudioUsecaseFlag());
        }
    }
//HTC_AUD_END

    if (param.getInt(keySetFlightMode, value) == NO_ERROR)
    {
        param.remove(keySetFlightMode);
        if (value == 1)
        {
           ALOGD("flignt mode=1");
        }
        else if (value == 0)
        {
           ALOGD("flight mode=0");
        }
    }

    if (param.size())
    {
        ALOGW("%s(), still have param.size() = %d, remain param = \"%s\"",
              __FUNCTION__, param.size(), param.toString().string());
        status = BAD_VALUE;
    }

    ALOGD("-%s(): %s ", __FUNCTION__, keyValuePairs.string());
    return status;
}

String8 AudioALSAHardware::getParameters(const String8 &keys)
{
    ALOGD("+%s(), key = %s", __FUNCTION__, keys.string());

    String8 value;
    int cmdType = 0;
    AudioParameter param = AudioParameter(keys);
    AudioParameter returnParam = AudioParameter();

    if (param.get(keyMTK_AUDENH_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_AUDENH_SUPPORT);
#ifdef MTK_AUDENH_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_AUDENH_SUPPORT, value);
    }
    if (param.get(keyMTK_TTY_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_TTY_SUPPORT);
#ifdef MTK_TTY_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_TTY_SUPPORT, value);
    }
    if (param.get(keyMTK_WB_SPEECH_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_WB_SPEECH_SUPPORT);
#ifdef MTK_WB_SPEECH_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_WB_SPEECH_SUPPORT, value);
    }
    if (param.get(keyMTK_DUAL_MIC_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_DUAL_MIC_SUPPORT);
#ifdef MTK_DUAL_MIC_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_DUAL_MIC_SUPPORT, value);
    }
    if (param.get(keyMTK_AUDIO_HD_REC_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_AUDIO_HD_REC_SUPPORT);
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_AUDIO_HD_REC_SUPPORT, value);
    }
    if (param.get(keyMTK_BESLOUDNESS_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_BESLOUDNESS_SUPPORT);
#ifdef MTK_BESLOUDNESS_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_BESLOUDNESS_SUPPORT, value);
    }
    if (param.get(keyMTK_BESSURROUND_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_BESSURROUND_SUPPORT);
#ifdef MTK_BESSURROUND_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_BESSURROUND_SUPPORT, value);
    }
    if (param.get(keyMTK_HDMI_MULTI_CHANNEL_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_HDMI_MULTI_CHANNEL_SUPPORT);
#ifdef MTK_HDMI_MULTI_CHANNEL_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_HDMI_MULTI_CHANNEL_SUPPORT, value);
    }

    if (param.get(keyMTK_VOW_SUPPORT, value) == NO_ERROR)
    {
        param.remove(keyMTK_VOW_SUPPORT);
#ifdef MTK_VOW_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_VOW_SUPPORT, value);
    }


    if (param.get(keyGetFmEnable, value) == NO_ERROR)
    {
        param.remove(keyGetFmEnable);

        value = (mStreamManager->getFmEnable() == true) ? "true" : "false";
        returnParam.add(keyGetFmEnable, value);
    }

#if defined(MTK_DUAL_MIC_SUPPORT)
    if (param.getInt(keyDUALMIC_GET_GAIN, cmdType) == NO_ERROR)
    {
        unsigned short gain = 0;
        char buf[32];

        if (mAudioALSAParamTunerInstance->getDMNRGain((unsigned short)cmdType, &gain) == NO_ERROR)
        {
            sprintf(buf, "%d", gain);
            returnParam.add(keyDUALMIC_GET_GAIN, String8(buf));
            param.remove(keyDUALMIC_GET_GAIN);
        }
    }
#endif

    if (param.get(keyGetFmTxEnable, value) == NO_ERROR)
    {
        param.remove(keyGetFmTxEnable);

        value = (mFmTxEnable == true) ? "true" : "false";
        returnParam.add(keyGetFmTxEnable, value);
    }

    if (param.get(keyMusicPlusGet, value) == NO_ERROR)
    {

        bool musicplus_status = mStreamManager->GetMusicPlusStatus();
        value = (musicplus_status) ? "1" : "0";
        param.remove(keyMusicPlusGet);
        returnParam.add(keyMusicPlusGet, value);
    }

    if (param.get(keyBesLoudnessGet, value) == NO_ERROR)
    {

        bool besloudness_status = mStreamManager->GetBesLoudnessStatus();
        value = (besloudness_status) ? "1" : "0";
        param.remove(keyBesLoudnessGet);
        returnParam.add(keyBesLoudnessGet, value);
    }

    // Audio Volume version
    if (param.get(keyGET_AUDIO_VOLUME_VER, value) == NO_ERROR)
    {
        param.remove(keyGET_AUDIO_VOLUME_VER);
        value = "1";
        returnParam.add(keyGET_AUDIO_VOLUME_VER, value);
    }

    // check if the LR channel switched
    if (param.get(keyLR_ChannelSwitch, value) == NO_ERROR)
    {
        bool bIsLRSwitch = AudioALSAHardwareResourceManager::getInstance()->getMicInverse();
        value = (bIsLRSwitch == true) ? "1" : "0";
        param.remove(keyLR_ChannelSwitch);
        returnParam.add(keyLR_ChannelSwitch, value);
    }

    // get the Capture drop time for voice unlock
    if (param.get(keyGetCaptureDropTime, value) == NO_ERROR)
    {
        char buf[32];
        sprintf(buf, "%d", CAPTURE_DROP_MS);
        param.remove(keyGetCaptureDropTime);
        returnParam.add(keyGetCaptureDropTime, String8(buf));
    }

    if (param.get(keyACCESS_VOW_PARAM, value) == NO_ERROR)
    {
        //parse format
        int par_type = -1;
        int par_value = -1;
        sscanf(value.string(), "%d", &par_type);
        param.remove(keyACCESS_VOW_PARAM);
        //get value from nvram
        if ((par_type < 1) || (par_type > 10))
        {
            par_value = 2;
            ALOGE("%s, wrong index = %d", keyACCESS_VOW_PARAM.string(), par_type);
        }
        else
        {
            par_value = GetVOWCustParamFromNV(par_type - 1);
        }
        //return value
        char buf[32];
        sprintf(buf, "%d", par_value);
        returnParam.add(keyACCESS_VOW_PARAM, String8(buf));
        ALOGD("%s, par_type = %d, par_value = %d", keyACCESS_VOW_KR_PARAM.string(), par_type, par_value);
    }

    if (param.get(keyACCESS_VOW_KR_PARAM, value) == NO_ERROR)
    {
        //parse format
        int par_type = -1;
        int par_value = -1;
        sscanf(value.string(), "%d", &par_type);
        param.remove(keyACCESS_VOW_KR_PARAM);
        //get value from nvram
        if ((par_type < 1) || (par_type > 4))
        {
            par_value = 2;
            ALOGE("%s, wrong index = %d", keyACCESS_VOW_KR_PARAM.string(), par_type);
        }
        else
        {
            par_value = GetVOWCustParamFromNV(par_type + 9);
        }
        //return value
        char buf[32];
        sprintf(buf, "%d", par_value);
        returnParam.add(keyACCESS_VOW_KR_PARAM, String8(buf));
        ALOGD("%s, par_type = %d, par_value = %d", keyACCESS_VOW_KR_PARAM.string(), par_type, par_value);
    }

    if (param.get(keyACCESS_VOW_KRSR_PARAM, value) == NO_ERROR)
    {
        //parse format
        int par_type = -1;
        int par_value = -1;
        sscanf(value.string(), "%d", &par_type);
        param.remove(keyACCESS_VOW_KRSR_PARAM);
        //get value from nvram
        if ((par_type < 1) || (par_type > 4))
        {
            par_value = 2;
            ALOGE("%s, wrong index = %d", keyACCESS_VOW_KRSR_PARAM.string(), par_type);
        }
        else
        {
            par_value = GetVOWCustParamFromNV(par_type + 17);
        }
        //return value
        char buf[32];
        sprintf(buf, "%d", par_value);
        returnParam.add(keyACCESS_VOW_KRSR_PARAM, String8(buf));
        ALOGD("%s, par_type = %d, par_value = %d", keyACCESS_VOW_KRSR_PARAM.string(), par_type, par_value);
    }

    if (param.get(keyGET_MAGI_CONFERENCE_SUPPORT, value) == NO_ERROR)
    {
#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
        value = "1";
#else
        value = "0";
#endif
        param.remove(keyGET_MAGI_CONFERENCE_SUPPORT);
        returnParam.add(keyGET_MAGI_CONFERENCE_SUPPORT, value);
    }

#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
    if (param.get(keyGET_MAGI_CONFERENCE_ENABLE, value) == NO_ERROR)
    {
        bool magiccon_status = mStreamManager->GetMagiConCallEnable();
        value = (magiccon_status) ? "1" : "0";
        param.remove(keyGET_MAGI_CONFERENCE_ENABLE);
        returnParam.add(keyGET_MAGI_CONFERENCE_ENABLE, value);
    }
#endif

    if (param.get(keyGET_HAC_SUPPORT, value) == NO_ERROR)
    {
#if defined(MTK_HAC_SUPPORT)
        value = "1";
#else
        value = "0";
#endif
        param.remove(keyGET_HAC_SUPPORT);
        returnParam.add(keyGET_HAC_SUPPORT, value);
    }

#if defined(MTK_HAC_SUPPORT)
    if (param.get(keyGET_HAC_ENABLE, value) == NO_ERROR)
    {
        bool hac_status = mStreamManager->GetHACEnable();
        value = (hac_status) ? "1" : "0";
        param.remove(keyGET_HAC_ENABLE);
        returnParam.add(keyGET_HAC_ENABLE, value);
    }
#endif

#ifdef MTK_AUDIO_GAIN_TABLE
    if (param.get(keySpeechBand, value) == NO_ERROR)
    {
        param.remove(keySpeechBand);
        bool nb = mAudioALSAVolumeController->isNbSpeechBand();
        value = nb ? "1" : "0";
        returnParam.add(keySpeechBand, value);
    }
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)
    if (param.getInt(keyDUALMIC_GET_GAIN, cmdType) == NO_ERROR)
    {
        unsigned short gain = 0;
        char buf[32];

        if (mAudioALSAParamTunerInstance->getDMNRGain((unsigned short)cmdType, &gain) == NO_ERROR)
        {
            sprintf(buf, "%d", gain);
            returnParam.add(keyDUALMIC_GET_GAIN, String8(buf));
            param.remove(keyDUALMIC_GET_GAIN);
        }
    }
#endif
    // Dual Mic Noise Reduction, DMNR for Receiver
    if (param.get(keyGet_Dual_Mic_Setting, value) == NO_ERROR) // new name
    {
        param.remove(keyGet_Dual_Mic_Setting);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_DMNR) > 0) ? "1" : "0";
        returnParam.add(keyGet_Dual_Mic_Setting, value);
    }

    // Dual Mic Noise Reduction, DMNR for Loud Speaker: feature face out and be replaced by MagiConference
    if (param.get(keyGET_LSPK_DMNR_ENABLE, value) == NO_ERROR)
    {
        param.remove(keyGET_LSPK_DMNR_ENABLE);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) > 0) ? "1" : "0";
        returnParam.add(keyGET_LSPK_DMNR_ENABLE, value);
    }
    // VoIP Dual Mic Noise Reduction, DMNR for Receiver
    if (param.get(keyGET_VOIP_RECEIVER_DMNR_ENABLE, value) == NO_ERROR)
    {
        param.remove(keyGET_VOIP_RECEIVER_DMNR_ENABLE);
//        value = (mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) > 0) ? "1" : "0";
        value = "0";
        returnParam.add(keyGET_VOIP_RECEIVER_DMNR_ENABLE, value);
    }

    // VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
    if (param.get(keyGET_VOIP_LSPK_DMNR_ENABLE, value) == NO_ERROR)
    {
        param.remove(keyGET_VOIP_LSPK_DMNR_ENABLE);
//        value = (mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) > 0) ? "1" : "0";
        value = "0";
        returnParam.add(keyGET_VOIP_LSPK_DMNR_ENABLE, value);
    }

    // Voice Clarity Engine, VCE
    if (param.get(keyGET_VCE_ENABLE, value) == NO_ERROR) // new name
    {
        param.remove(keyGET_VCE_ENABLE);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_VCE) > 0) ? "1" : "0";
        returnParam.add(keyGET_VCE_ENABLE, value);
    }

    if (param.get(keyGET_VCE_STATUS, value) == NO_ERROR) // old name
    {
        param.remove(keyGET_VCE_STATUS);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_VCE) > 0) ? "1" : "0";
        returnParam.add(keyGET_VCE_STATUS, value);
    }

    const String8 keyValuePairs = returnParam.toString();
    ALOGD("-%s(), return \"%s\"", __FUNCTION__, keyValuePairs.string());
    return keyValuePairs;
}

size_t AudioALSAHardware::getInputBufferSize(uint32_t sampleRate, int format, int channelCount)
{
    // TODO(Harvey): check it
    return 1024; // mStreamManager->getInputBufferSize(sampleRate, static_cast<audio_format_t>(format), channelCount);
}

android_audio_legacy::AudioStreamOut *AudioALSAHardware::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status)
{
    return mStreamManager->openOutputStream(devices, format, channels, sampleRate, status);
}

void AudioALSAHardware::closeOutputStream(android_audio_legacy::AudioStreamOut *out)
{
    mStreamManager->closeOutputStream(out);
}

android_audio_legacy::AudioStreamIn *AudioALSAHardware::openInputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    return mStreamManager->openInputStream(devices, format, channels, sampleRate, status, acoustics);
}

void AudioALSAHardware::closeInputStream(android_audio_legacy::AudioStreamIn *in)
{
    mStreamManager->closeInputStream(in);
}


status_t AudioALSAHardware::dumpState(int fd, const Vector<String16> &args)
{
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAHardware::dump(int fd, const Vector<String16> &args)
{
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}


#if 1

static status_t TDMrecordInit(uint32_t sampleRate)
{
    ALOGD("+%s(), sampleRate = %d", __FUNCTION__, sampleRate);

    if (gAudioHardware == NULL)
    {
        gAudioHardware = android_audio_legacy::createAudioHardware();
    }

    if (gAudioStreamIn == NULL)
    {
        android::AudioParameter paramVoiceMode = android::AudioParameter();
        paramVoiceMode.addInt(android::String8("HDREC_SET_VOICE_MODE"), 0);
        gAudioHardware->setParameters(paramVoiceMode.toString());

        uint32_t device = AUDIO_DEVICE_IN_TDM;
        //        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;

        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        status_t status = 0;
        android::AudioParameter param = android::AudioParameter();

        gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
        if (gAudioStreamIn == NULL)
        {
            ALOGD("Reopen openInputStream with format=%d, channel=%d, sampleRate=%d \n", format, channel, sampleRate);
            gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
        }

        //        param.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_TDM);
        ALOGD("recordInit samplerate=%d\n", sampleRate);

        if (gAudioStreamIn != NULL)
        {
            gAudioStreamIn->setParameters(param.toString());
        }
    }
    android::AudioParameter param = android::AudioParameter();
    param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_TDM);
    //    param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_BUILTIN_MIC);

    if (gAudioStreamIn != NULL)
    {
        gAudioStreamIn->setParameters(param.toString());
    }

    return NO_ERROR;
}


status_t AudioALSAHardware::setTDMRecord(int samplerate)
{
    ALOGD("+%s(), samplerate = %d", __FUNCTION__, samplerate);

    if (samplerate > 0)
    {
        SetTDMrecordEnable(samplerate);
    }
    else
    {
        SetTDMrecordDisable();
    }
    return NO_ERROR;
}

static void *TDM_Record_Thread(void *mPtr)
{

    short pbuffer[8192] = {0};
    char filenameL[] = "/data/record_tdm.pcm";
    //   FILE *fpL = fopen(filenameL, "wb+");
    int readSize = 0;

    TDMrecordInit(gTDMsetSampleRate);
    while (1)
    {

        memset(pbuffer, 0, sizeof(pbuffer));

        readSize = gAudioStreamIn->read(pbuffer, 8192 * 2);

#if 0
        if (fpL != NULL)
        {
            fwrite(pbuffer, readSize, 1, fpL);
        }
#endif
        if (gTDMsetExit)
        {
            break;
        }
    }
    //    fclose(fpL);

    ALOGD("%s: Stop", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;

}

status_t AudioALSAHardware::SetTDMrecordEnable(uint32_t sampleRate)
{
    pthread_t TDMRecordThread;

    pthread_create(&TDMRecordThread, NULL, TDM_Record_Thread, NULL);

    gTDMsetSampleRate = sampleRate;

    gTDMsetExit = false;

    return NO_ERROR;

}

status_t AudioALSAHardware::SetTDMrecordDisable(void)
{
    ALOGD("+%s()", __FUNCTION__);

    gTDMsetExit = true;


    if (gAudioStreamIn != NULL)
    {
        gAudioStreamIn->standby();
        gAudioHardware->closeInputStream(gAudioStreamIn);
        gAudioStreamIn = NULL;
    }

    return NO_ERROR;
}

status_t AudioALSAHardware::SetEMParameter(void *ptr, int len)
{
    ALOGD("%s() len [%d] sizeof [%d]", __FUNCTION__, len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

    if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB = (AUDIO_CUSTOM_PARAM_STRUCT *)ptr;
        mStreamManager->SetEMParameter(pSphParamNB);
        return NO_ERROR;
    }
    else
    {
        ALOGE("len [%d] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%d] ", len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return UNKNOWN_ERROR;
    }
}

status_t AudioALSAHardware::GetEMParameter(void *ptr , int len)
{
    ALOGD("%s() len [%d] sizeof [%d]", __FUNCTION__, len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

    if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT))
    {
        GetNBSpeechParamFromNVRam((AUDIO_CUSTOM_PARAM_STRUCT *)ptr);
        return NO_ERROR;
    }
    else
    {
        ALOGE("len [%d] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%d] ", len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return UNKNOWN_ERROR;
    }
}

status_t AudioALSAHardware::SetAudioCommand(int par1, int par2)
{
    ALOGD("%s(), par1 = 0x%x, par2 = %d", __FUNCTION__, par1, par2);
    switch (par1)
    {
        case SETOUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Normal_Coef_Index, par2);
            break;
        }
        case SETNORMALOUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Normal_Coef_Index, par2);
            break;
        }
        case SETHEADSETOUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Headset_Coef_Index, par2);
            break;
        }
        case SETSPEAKEROUTPUTFIRINDEX:
        {
            UpdateOutputFIR(Handfree_Coef_Index, par2);
            break;
        }
        case SET_LOAD_VOLUME_SETTING:
        {
            mAudioALSAVolumeController->initVolumeController();
            setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
#ifndef LPDK_AUDIO_USE
            //            const sp<IAudioPolicyService> &aps = AudioSystem::get_audio_policy_service();
            //            aps->SetPolicyManagerParameters(POLICY_LOAD_VOLUME, 0, 0, 0);
#endif
            break;
        }
        case SET_SPEECH_VM_ENABLE:
        {
            ALOGD("+SET_SPEECH_VM_ENABLE(%d)", par2);

            mStreamManager->SetSpeechVmEnable(par2);
            ALOGD("-SET_SPEECH_VM_ENABLE(%d)", par2);
            break;
        }
        case SET_DUMP_SPEECH_DEBUG_INFO:
        {
            ALOGD(" SET_DUMP_SPEECH_DEBUG_INFO(%d)", par2);
            //mSpeechDriverFactory->GetSpeechDriver()->ModemDumpSpeechParam();
            break;
        }
        case SET_DUMP_AUDIO_DEBUG_INFO:
        {
            ALOGD(" SET_DUMP_AUDIO_DEBUG_INFO(%d)", par2);
            AudioALSAHardwareResourceManager::getInstance()->setAudioDebug(true);
            break;
        }
        case SET_DUMP_AUDIO_AEE_CHECK:
        {
            ALOGD(" SET_DUMP_AUDIO_AEE_CHECK(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamout.aee.dump", "0");
            }
            else
            {
                property_set("streamout.aee.dump", "1");
            }
            break;
        }
        case SET_DUMP_AUDIO_STREAM_OUT:
        {
            ALOGD(" SET_DUMP_AUDIO_STREAM_OUT(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamout.pcm.dump", "0");
                //::ioctl(mFd, AUDDRV_AEE_IOCTL, 0);
            }
            else
            {
                property_set("streamout.pcm.dump", "1");
                //::ioctl(mFd, AUDDRV_AEE_IOCTL, 1);
            }
            break;
        }
        case SET_DUMP_AUDIO_MIXER_BUF:
        {
            ALOGD(" SET_DUMP_AUDIO_MIXER_BUF(%d)", par2);
            if (par2 == 0)
            {
                property_set("af.mixer.pcm", "0");
            }
            else
            {
                property_set("af.mixer.pcm", "1");
            }
            break;
        }
        case SET_DUMP_AUDIO_TRACK_BUF:
        {
            ALOGD(" SET_DUMP_AUDIO_TRACK_BUF(%d)", par2);
            if (par2 == 0)
            {
                property_set("af.track.pcm", "0");
            }
            else
            {
                property_set("af.track.pcm", "1");
            }
            break;
        }
        case SET_DUMP_A2DP_STREAM_OUT:
        {
            ALOGD(" SET_DUMP_A2DP_STREAM_OUT(%d)", par2);
            if (par2 == 0)
            {
                property_set("a2dp.streamout.pcm", "0");
            }
            else
            {
                property_set("a2dp.streamout.pcm", "1");
            }
            break;
        }
        case SET_DUMP_AUDIO_STREAM_IN:
        {
            ALOGD(" SET_DUMP_AUDIO_STREAM_IN(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamin.pcm.dump", "0");
            }
            else
            {
                property_set("streamin.pcm.dump", "1");
            }
            break;
        }
        case SET_DUMP_IDLE_VM_RECORD:
        {
            ALOGD(" SET_DUMP_IDLE_VM_RECORD(%d)", par2);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
            if (par2 == 0)
            {
                property_set("streamin.vm.dump", "0");
            }
            else
            {
                property_set("streamin.vm.dump", "1");
            }
#endif
            break;
        }
        case SET_DUMP_AP_SPEECH_EPL:
        {
            ALOGD(" SET_DUMP_AP_SPEECH_EPL(%d)", par2);
            if (par2 == 0)
            {
                property_set("streamin.epl.dump", "0");
            }
            else
            {
                property_set("streamin.epl.dump", "1");
            }
            break;
        }
        case SET_MagiASR_TEST_ENABLE:
        {
            ALOGD(" SET_MagiASR_TEST_ENABLE(%d)", par2);
            if (par2 == 0)
            {
                //disable MagiASR verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceMagiASR(false);
            }
            else
            {
                //enable MagiASR verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceMagiASR(true);
            }
            break;
        }
        case SET_AECREC_TEST_ENABLE:
        {
            ALOGD(" SET_AECREC_TEST_ENABLE(%d)", par2);
            if (par2 == 0)
            {
                //disable AECRec verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceAECRec(false);
            }
            else
            {
                //enable AECRec verify mode
                mAudioSpeechEnhanceInfoInstance->SetForceAECRec(true);
            }
            break;
        }
#ifdef MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT
        //ANC Control
        case SET_SPEECH_ANC_STATUS:
        {
            ALOGD(" SET_SPEECH_ANC_STATUS(%d)", par2);
            mSpeechANCControllerInstance->SetApplyANC((bool)par2);
            mSpeechANCControllerInstance->EanbleANC((bool)par2);
            break;
        }
        case SET_SPEECH_ANC_LOG_ENABLE:
        {
            ALOGD(" SET_SPEECH_ANC_LOG_ENABLE");
            mSpeechANCControllerInstance->SetEanbleANCLog(true, (bool)par2);
            break;
        }
        case SET_SPEECH_ANC_LOG_DISABLE:
        {
            ALOGD(" SET_SPEECH_ANC_LOG_DISABLE");
            mSpeechANCControllerInstance->SetEanbleANCLog(false, (bool)par2);
            break;
        }
        case START_SPEECH_ANC_CALIBRATION:
        {
            ALOGD(" START_SPEECH_ANC_CALIBRATION");
            break;
        }
#endif
        case SET_CUREENT_SENSOR_ENABLE:
        {

            ALOGD("SET_CUREENT_SENSOR_ENABLE");
            AudioALSAHardwareResourceManager::getInstance()->setSPKCurrentSensor((bool)par2);
            break;
        }
        case SET_CURRENT_SENSOR_RESET:
        {
            ALOGD("SET_CURRENT_SENSOR_RESET");
            AudioALSAHardwareResourceManager::getInstance()->setSPKCurrentSensorPeakDetectorReset((bool)par2);
            break;
        }
        case SET_SPEAKER_MONITOR_TEMP_UPPER_BOUND:
        {
            ALOGD("SET_SPEAKER_MONITOR_TEMP_UPPER_BOUND %d", par2);
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
            AudioALSASpeakerMonitor::getInstance()->SetTempUpperBound(par2);
#endif
            break;
        }
        case SET_SPEAKER_MONITOR_TEMP_LOWER_BOUND:
        {
            ALOGD("SET_SPEAKER_MONITOR_TEMP_LOWER_BOUND %d", par2);
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
            AudioALSASpeakerMonitor::getInstance()->SetTempLowerBound(par2);
#endif
            break;
        }
        case SET_TDM_LOOPBACK_I0I1_ENABLE:
        {
            // HDMI record enable

        }
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioALSAHardware::GetAudioCommand(int par1)
{
    ALOGD("%s  par1 = %d ", __FUNCTION__, par1);
    int result = 0 ;
    char value[PROPERTY_VALUE_MAX];

    switch (par1)
    {
        case GETOUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
            break;
        }
        case GETAUDIOCUSTOMDATASIZE:
        {
            int AudioCustomDataSize = sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
            ALOGD("GETAUDIOCUSTOMDATASIZE  AudioCustomDataSize = %d", AudioCustomDataSize);
            return AudioCustomDataSize;
        }
        case GETNORMALOUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
            break;
        }
        case GETHEADSETOUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Headset_Coef_Index];
            break;
        }
        case GETSPEAKEROUTPUTFIRINDEX:
        {
            AUDIO_PARAM_MED_STRUCT pMedPara;
            GetMedParamFromNV(&pMedPara);
            result = pMedPara.select_FIR_output_index[Handfree_Coef_Index];
            break;
        }
        case GET_DUMP_AUDIO_AEE_CHECK:
        {
            property_get("streamout.aee.dump", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_STREAM_OUT=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_STREAM_OUT:
        {
            property_get("streamout.pcm.dump", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_STREAM_OUT=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_MIXER_BUF:
        {
            property_get("af.mixer.pcm", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_MIXER_BUF=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_TRACK_BUF:
        {
            property_get("af.track.pcm", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_TRACK_BUF=%d", result);
            break;
        }
        case GET_DUMP_A2DP_STREAM_OUT:
        {
            property_get("a2dp.streamout.pcm", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_A2DP_STREAM_OUT=%d", result);
            break;
        }
        case GET_DUMP_AUDIO_STREAM_IN:
        {
            property_get("streamin.pcm.dump", value, "0");
            result = atoi(value);
            ALOGD(" GET_DUMP_AUDIO_STREAM_IN=%d", result);
            break;
        }
        case GET_DUMP_IDLE_VM_RECORD:
        {
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
            property_get("streamin.vm.dump", value, "0");
            result = atoi(value);
#else
            result = 0;
#endif
            ALOGD(" GET_DUMP_IDLE_VM_RECORD=%d", result);
            break;
        }
        case GET_DUMP_AP_SPEECH_EPL:
        {
            property_get("streamin.epl.dump", value, "0");
            result = atoi(value);

            int result1 = 0 ;
            char value1[PROPERTY_VALUE_MAX];
            property_get("streamin.epl.dump", value1, "0");
            result1 = atoi(value1);

            if (result1 == 1)
            {
                result = 1;
            }
            ALOGD(" GET_DUMP_AP_SPEECH_EPL=%d", result);
            break;
        }
        case GET_MagiASR_TEST_ENABLE:
        {
            //get the MagiASR verify mode status
            result = mAudioSpeechEnhanceInfoInstance->GetForceMagiASRState();
            ALOGD(" GET_MagiASR_TEST_ENABLE=%d", result);
            break;
        }
        case GET_AECREC_TEST_ENABLE:
        {
            //get the AECRec verify mode status
            result = 0;
            if (mAudioSpeechEnhanceInfoInstance->GetForceAECRecState())
            {
                result = 1;
            }

            ALOGD(" GET_AECREC_TEST_ENABLE=%d", result);
            break;
        }
        case GET_SPEECH_ANC_SUPPORT:
        {
            result = mSpeechANCControllerInstance->GetANCSupport();
            ALOGD("GetAudioCommand par1 = %d, result = %d ", par1, result);
            break;
        }

#ifdef MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT
        case GET_SPEECH_ANC_STATUS:
        {
            result = mSpeechANCControllerInstance->GetApplyANC();
            ALOGD("GetAudioCommand par1 = %d, result = %d ", par1, result);
            break;
        }
        case GET_SPEECH_ANC_LOG_STATUS:
        {
            result = mSpeechANCControllerInstance->GetEanbleANCLog();
            ALOGD("GetAudioCommand par1 = %d, result = %d ", par1, result);
            break;
        }

        case GET_SPEECH_ANC_CALIBRATION_STATUS:
        {
            result = false;
            ALOGD("GetAudioCommand par1 = %d, result = %d ", par1, result);
            break;
        }
#endif

        default:
        {
            ALOGD(" GetAudioCommonCommand: Unknown command\n");
            break;
        }
    }

    // call fucntion want to get status adn return it.
    return result;
}

status_t AudioALSAHardware::SetAudioCommonData(int par1, size_t len, void *ptr)
{
    ALOGD("%s(), par1 = 0x%x, len = %d", __FUNCTION__, par1, len);
    switch (par1)
    {
        case SETMEDDATA:
        {
            ASSERT(len == sizeof(AUDIO_PARAM_MED_STRUCT));
            SetMedParamToNV((AUDIO_PARAM_MED_STRUCT *)ptr);
            break;
        }
        case SETAUDIOCUSTOMDATA:
        {
            ASSERT(len == sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
            SetAudioCustomParamToNV((AUDIO_VOLUME_CUSTOM_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
            setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
            break;
        }
#if defined(MTK_DUAL_MIC_SUPPORT)
        case SET_DUAL_MIC_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            SetDualMicSpeechParamToNVRam((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
            SpeechEnhancementController::GetInstance()->SetDualMicSpeechParametersToAllModem((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            break;
        }
#endif
#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
        case SET_SPEECH_MAGICON_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
            SetMagiConSpeechParamToNVRam((AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
            SpeechEnhancementController::GetInstance()->SetMagiConSpeechParametersToAllModem((AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *)ptr);
            break;
        }
#endif
#if defined(MTK_HAC_SUPPORT)
        case SET_SPEECH_HAC_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
            SetHACSpeechParamToNVRam((AUDIO_CUSTOM_HAC_PARAM_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
            SpeechEnhancementController::GetInstance()->SetHACSpeechParametersToAllModem((AUDIO_CUSTOM_HAC_PARAM_STRUCT *)ptr);
            break;
        }
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
        case SET_WB_SPEECH_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
            SetWBSpeechParamToNVRam((AUDIO_CUSTOM_WB_PARAM_STRUCT *)ptr);
            SpeechEnhancementController::GetInstance()->SetWBSpeechParametersToAllModem((AUDIO_CUSTOM_WB_PARAM_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController(); // for DRC2.0 need volume to get speech mode
            break;
        }
#endif
        case SET_AUDIO_VER1_DATA:
        {
            ASSERT(len == sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));

            SetVolumeVer1ParamToNV((AUDIO_VER1_CUSTOM_VOLUME_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
#ifndef LPDK_AUDIO_USE
            //Do it at AudioSystem, avoid deadlock, Hochi
            //            const sp<IAudioPolicyService> &aps = AudioSystem::get_audio_policy_service();
            //            aps->SetPolicyManagerParameters(POLICY_LOAD_VOLUME, 0, 0, 0);
#endif
            setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
            break;
        }
        // for Audio Taste Tuning
        case AUD_TASTE_TUNING:
        {
            status_t ret = NO_ERROR;
            AudioTasteTuningStruct audioTasteTuningParam;
            memcpy((void *)&audioTasteTuningParam, ptr, sizeof(AudioTasteTuningStruct));

            switch (audioTasteTuningParam.cmd_type)
            {
                case AUD_TASTE_STOP:
                {
                    mAudioALSAParamTunerInstance->enableModemPlaybackVIASPHPROC(false);
                    audioTasteTuningParam.wb_mode = mAudioALSAParamTunerInstance->m_bWBMode;
                    mAudioALSAParamTunerInstance->updataOutputFIRCoffes(&audioTasteTuningParam);

                    break;
                }
                case AUD_TASTE_START:
                {

                    mAudioALSAParamTunerInstance->setMode(audioTasteTuningParam.phone_mode);
                    ret = mAudioALSAParamTunerInstance->setPlaybackFileName(audioTasteTuningParam.input_file);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }
                    ret = mAudioALSAParamTunerInstance->setDLPGA((uint32) audioTasteTuningParam.dlPGA);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }
                    mAudioALSAParamTunerInstance->updataOutputFIRCoffes(&audioTasteTuningParam);
                    ret = mAudioALSAParamTunerInstance->enableModemPlaybackVIASPHPROC(true, audioTasteTuningParam.wb_mode);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }

                    break;
                }
                case AUD_TASTE_DLDG_SETTING:
                case AUD_TASTE_INDEX_SETTING:
                {
                    mAudioALSAParamTunerInstance->updataOutputFIRCoffes(&audioTasteTuningParam);
                    break;
                }
                case AUD_TASTE_DLPGA_SETTING:
                {
                    mAudioALSAParamTunerInstance->setMode(audioTasteTuningParam.phone_mode);
                    ret = mAudioALSAParamTunerInstance->setDLPGA((uint32) audioTasteTuningParam.dlPGA);
                    if (ret != NO_ERROR)
                    {
                        return ret;
                    }

                    break;
                }
                default:
                    break;
            }

            break;
        }
        case HOOK_FM_DEVICE_CALLBACK:
        {
            AudioALSAFMController::getInstance()->setFmDeviceCallback((AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *)ptr);
            break;
        }
        case UNHOOK_FM_DEVICE_CALLBACK:
        {
            AudioALSAFMController::getInstance()->setFmDeviceCallback(NULL);
            break;
        }
        case HOOK_BESLOUDNESS_CONTROL_CALLBACK:
        {
            mStreamManager->SetBesLoudnessControlCallback((BESLOUDNESS_CONTROL_CALLBACK_STRUCT *)ptr);
            break;
        }
        case UNHOOK_BESLOUDNESS_CONTROL_CALLBACK:
        {
            mStreamManager->SetBesLoudnessControlCallback(NULL);
            break;
        }
        default:
            ALOGD(" GetAudioCommand: Unknown command\n");
            break;
    }
    return NO_ERROR;
}

status_t AudioALSAHardware::GetAudioCommonData(int par1, size_t len, void *ptr)
{
    ALOGD("%s par1=%d, len=%d", __FUNCTION__, par1, len);
    switch (par1)
    {
        case GETMEDDATA:
        {
            ASSERT(len == sizeof(AUDIO_PARAM_MED_STRUCT));
            GetMedParamFromNV((AUDIO_PARAM_MED_STRUCT *)ptr);
            break;
        }
        case GETAUDIOCUSTOMDATA:
        {
            ASSERT(len == sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
            GetAudioCustomParamFromNV((AUDIO_VOLUME_CUSTOM_STRUCT *)ptr);
            break;
        }
#if defined(MTK_DUAL_MIC_SUPPORT)
        case GET_DUAL_MIC_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            GetDualMicSpeechParamFromNVRam((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            break;
        }
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
        case GET_WB_SPEECH_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
            GetWBSpeechParamFromNVRam((AUDIO_CUSTOM_WB_PARAM_STRUCT *) ptr);
            break;
        }
#endif
#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
        case GET_SPEECH_MAGICON_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
            GetMagiConSpeechParamFromNVRam((AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *)ptr);
            break;
        }
#endif
#if defined(MTK_HAC_SUPPORT)
        case GET_SPEECH_HAC_PARAMETER:
        {
            ASSERT(len == sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
            GetHACSpeechParamFromNVRam((AUDIO_CUSTOM_HAC_PARAM_STRUCT *)ptr);
            break;
        }
#endif
        case GET_AUDIO_VER1_DATA:
        {
            GetVolumeVer1ParamFromNV((AUDIO_VER1_CUSTOM_VOLUME_STRUCT *) ptr);
            break;
        }
        case GET_AUDIO_POLICY_VOL_FROM_VER1_DATA:
        {
            AUDIO_CUSTOM_VOLUME_STRUCT *pTarget = (AUDIO_CUSTOM_VOLUME_STRUCT *) ptr;

            if ((pTarget->bRev == CUSTOM_VOLUME_REV_1) && (len == sizeof(AUDIO_CUSTOM_VOLUME_STRUCT)))
            {
                AUDIO_VER1_CUSTOM_VOLUME_STRUCT Source;
                GetVolumeVer1ParamFromNV(&Source);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_VOICE_CALL], (void *)Source.audiovolume_sph, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_SIP], (void *)Source.audiovolume_sip, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

                memcpy((void *)Source.audiovolume_ring[VOLUME_NORMAL_MODE], (void *)Source.audiovolume_ring[VOLUME_HEADSET_SPEAKER_MODE], sizeof(unsigned char)*AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)Source.audiovolume_ring[VOLUME_HEADSET_MODE], (void *)Source.audiovolume_ring[VOLUME_HEADSET_SPEAKER_MODE], sizeof(unsigned char)*AUDIO_MAX_VOLUME_STEP);

                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_RING], (void *)Source.audiovolume_ring, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_ALARM], (void *)Source.audiovolume_ring, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_NOTIFICATION], (void *)Source.audiovolume_ring, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_MUSIC], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_ENFORCED_AUDIBLE], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_TTS], (void *)audiovolume_tts_nonspeaker, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_TTS][CUSTOM_VOLUME_SPEAKER_MODE], (void *)Source.audiovolume_media[VOLUME_SPEAKER_MODE], sizeof(unsigned char)* AUDIO_MAX_VOLUME_STEP);
                
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_BLUETOOTH_SCO], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_BOOT], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_VIBSPK], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_SYSTEM], (void *)audiovolume_system, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_DTMF], (void *)audiovolume_dtmf, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_ACCESSIBILITY], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_REROUTING], (void *)audiovolume_rerouting, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_PATCH], (void *)audiovolume_patch, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_VOICE_CALL] = Source.audiovolume_level[VER1_VOL_TYPE_SPH];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_SYSTEM] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_RING] = Source.audiovolume_level[VER1_VOL_TYPE_RING];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_MUSIC] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_ALARM] = Source.audiovolume_level[VER1_VOL_TYPE_RING];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_NOTIFICATION] = Source.audiovolume_level[VER1_VOL_TYPE_RING];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_BLUETOOTH_SCO] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_ENFORCED_AUDIBLE] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_DTMF] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_TTS] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_BOOT] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_VIBSPK] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_SIP] = Source.audiovolume_level[VER1_VOL_TYPE_SIP];
                pTarget->audiovolume_level[CUSTOM_VOL_TYPE_ACCESSIBILITY] = pTarget->audiovolume_level[CUSTOM_VOL_TYPE_REROUTING] = pTarget->audiovolume_level[CUSTOM_VOL_TYPE_PATCH] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
                pTarget->bReady = 1;
                mUseTuningVolume = true;
                memcpy((void*)&VolCache,(void*)pTarget,sizeof(AUDIO_CUSTOM_VOLUME_STRUCT));
                ALOGD("Get PolicyCustomVolume");
            }
            break;
        }
        case GET_VOICE_CUST_PARAM:
        {
            GetVoiceRecogCustParamFromNV((VOICE_RECOGNITION_PARAM_STRUCT *)ptr);
            break;
        }
        case GET_VOICE_FIR_COEF:
        {
            AUDIO_HD_RECORD_PARAM_STRUCT custHDRECParam;
            GetHdRecordParamFromNV(&custHDRECParam);
            ASSERT(len == sizeof(custHDRECParam.hd_rec_fir));
            memcpy(ptr, (void *)custHDRECParam.hd_rec_fir, len);
            break;
        }
        case GET_VOICE_GAIN:
        {
            AUDIO_VER1_CUSTOM_VOLUME_STRUCT custGainParam;
            GetVolumeVer1ParamFromNV(&custGainParam);
            uint16_t *pGain = (uint16_t *)ptr;
#if 0
            *pGain = 0;
            *(pGain + 1) = 0;
#else
            *pGain = mAudioALSAVolumeController->MappingToDigitalGain(custGainParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
            *(pGain + 1) = mAudioALSAVolumeController->MappingToDigitalGain(custGainParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
#endif
            break;
        }
#ifdef MTK_AUDIO_GAIN_TABLE
        case GET_TC1_DISP:
        {
            ASSERT(len == sizeof(PCDispTotolStru));
            if (NULL != mAudioALSAParamTunerInstance)
            {
                mAudioALSAParamTunerInstance->getGainInfoForDisp(ptr);
            }
            break;
        }
#if defined(MTK_AUDIO_GAIN_TABLE_BT)
		case GET_BT_NREC_DISP:
		{
			ASSERT(len == sizeof(PCDispItem));
			if (NULL != mAudioALSAParamTunerInstance)
			{
				mAudioALSAParamTunerInstance->getBtNrecInfoForDisp(ptr);
			}
			break;
		}
#endif
#endif
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioALSAHardware::SetAudioData(int par1, size_t len, void *ptr)
{
    ALOGD("%s(), par1 = 0x%x, len = %d", __FUNCTION__, par1, len);
    return SetAudioCommonData(par1, len, ptr);
}

status_t AudioALSAHardware::GetAudioData(int par1, size_t len, void *ptr)
{
    ALOGD("%s(), par1 = 0x%x, len = %d", __FUNCTION__, par1, len);
    return GetAudioCommonData(par1, len, ptr);
}

// ACF Preview parameter
status_t AudioALSAHardware::SetACFPreviewParameter(void *ptr , int len)
{
    ALOGD("%s()", __FUNCTION__);
    mStreamManager->SetACFPreviewParameter(ptr, len);
    return NO_ERROR;
}

status_t AudioALSAHardware::SetHCFPreviewParameter(void *ptr , int len)
{
    ALOGD("%s()", __FUNCTION__);
    mStreamManager->SetHCFPreviewParameter(ptr, len);
    return NO_ERROR;
}

//for PCMxWay Interface API
int AudioALSAHardware::xWayPlay_Start(int sample_rate)
{
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->Start();
}

int AudioALSAHardware::xWayPlay_Stop(void)
{
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->Stop();
}

int AudioALSAHardware::xWayPlay_Write(void *buffer, int size_bytes)
{
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->Write(buffer, size_bytes);
}

int AudioALSAHardware::xWayPlay_GetFreeBufferCount(void)
{
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->GetFreeBufferCount();
}

int AudioALSAHardware::xWayRec_Start(int sample_rate)
{
    ALOGD("%s()", __FUNCTION__);
    return Record2Way::GetInstance()->Start();
}

int AudioALSAHardware::xWayRec_Stop(void)
{
    ALOGD("%s()", __FUNCTION__);
    return Record2Way::GetInstance()->Stop();
}

int AudioALSAHardware::xWayRec_Read(void *buffer, int size_bytes)
{
    ALOGD("%s()", __FUNCTION__);
    return Record2Way::GetInstance()->Read(buffer, size_bytes);
}


int AudioALSAHardware::ReadRefFromRing(void *buf, uint32_t datasz, void *DLtime)
{
    ALOGD("%s()", __FUNCTION__);

    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->ReadRefFromRing(buf, datasz, DLtime);
}

int AudioALSAHardware::GetVoiceUnlockULTime(void *DLtime)
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->GetVoiceUnlockULTime(DLtime);
}

int AudioALSAHardware::SetVoiceUnlockSRC(uint outSR, uint outChannel)
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->SetSRC(outSR, outChannel);
}

bool AudioALSAHardware::startVoiceUnlockDL()
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->startInput();
}

bool AudioALSAHardware::stopVoiceUnlockDL()
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->stopInput();
}

void AudioALSAHardware::freeVoiceUnlockDLInstance()
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL::freeInstance();
    return;
}

bool AudioALSAHardware::getVoiceUnlockDLInstance()
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    if (VInstance != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int AudioALSAHardware::GetVoiceUnlockDLLatency()
{
    ALOGD("%s()", __FUNCTION__);
    AudioVUnlockDL *VInstance = AudioVUnlockDL::getInstance();
    return VInstance->GetLatency();
}

void AudioALSAHardware::setLowLatencyMode(bool mode)
{
    mStreamManager->setLowLatencyMode(mode);
}

#endif

bool AudioALSAHardware::UpdateOutputFIR(int mode , int index)
{
    ALOGD("%s(),  mode = %d, index = %d", __FUNCTION__, mode, index);

    // save index to MED with different mode.
    AUDIO_PARAM_MED_STRUCT eMedPara;
    GetMedParamFromNV(&eMedPara);
    eMedPara.select_FIR_output_index[mode] = index;

    // copy med data into audio_custom param
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    GetNBSpeechParamFromNVRam(&eSphParamNB);

    for (int i = 0; i < NB_FIR_NUM;  i++)
    {
        ALOGD("eSphParamNB.sph_out_fir[%d][%d] = %d <= eMedPara.speech_output_FIR_coeffs[%d][%d][%d] = %d",
              mode, i, eSphParamNB.sph_out_fir[mode][i],
              mode, index, i, eMedPara.speech_output_FIR_coeffs[mode][index][i]);
    }

    memcpy((void *)eSphParamNB.sph_out_fir[mode],
           (void *)eMedPara.speech_output_FIR_coeffs[mode][index],
           sizeof(eSphParamNB.sph_out_fir[mode]));

    // set to nvram
    SetNBSpeechParamToNVRam(&eSphParamNB);
    SetMedParamToNV(&eMedPara);

    // set to modem side
    SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);

    return true;
}

status_t AudioALSAHardware::setMasterMute(bool muted)
{
    return INVALID_OPERATION;
}

#define FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES (AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
const char *strAudioPatchRole[] = {"AUDIO_PORT_ROLE_NONE", "AUDIO_PORT_ROLE_SOURCE", "AUDIO_PORT_ROLE_SINK"};
const char *strAudioPatchType[] = {"AUDIO_PORT_TYPE_NONE", "AUDIO_PORT_TYPE_DEVICE", "AUDIO_PORT_TYPE_MIX", "AUDIO_PORT_TYPE_SESSION"};

int AudioALSAHardware::createAudioPatch(unsigned int num_sources,
                                        const struct audio_port_config *sources,
                                        unsigned int num_sinks,
                                        const struct audio_port_config *sinks,
                                        audio_patch_handle_t *handle)
{
    int status = NO_ERROR;
    ALOGD("+%s num_sources [%d] , num_sinks [%d]", __FUNCTION__, num_sources, num_sinks);
#if 1 //Debug
    if (handle == NULL || sources == NULL || sinks == NULL)
    {
        ALOGW("Ptr is null");
        return BAD_VALUE;
    }
    ALOGD("handlecheck %s handle [0x%x] current size %d",__FUNCTION__, *handle,mAudioHalPatchVector.size());
    int i = 0, j = 0;

    for (i = 0; i < num_sources ; i++)
    {
        ALOGD("== source [%d]/[%d] ==", i, num_sources);
        ALOGD("id 0x%x", sources[i].id);
        ALOGD("role 0x%x %s", sources[i].role, strAudioPatchRole[sources[i].role]);
        ALOGD("type 0x%x %s", sources[i].type, strAudioPatchType[sources[i].type]);
        ALOGD("config_mask 0x%x", sources[i].config_mask);
        ALOGD("sample_rate 0x%x", sources[i].sample_rate);
        ALOGD("channel_mask 0x%x", sources[i].channel_mask);
        ALOGD("gain.index 0x%x", sources[i].gain.index);
        ALOGD("gain.mode 0x%x", sources[i].gain.mode);
        ALOGD("gain.channel_mask 0x%x", sources[i].gain.channel_mask);
        ALOGD("gain.ramp_duration_ms 0x%x", sources[i].gain.ramp_duration_ms);
#if 0 //When gain check , enable        
        for (j = 0; j < sizeof(audio_channel_mask_t) * 8; j++)
        {
            ALOGD("gain.values[%d] 0x%x", j, sources[i].gain.values[j]);
        }
#endif
        if (sources[i].type == AUDIO_PORT_TYPE_DEVICE)
        {
            ALOGD("device.hw_module %x", sources[i].ext.device.hw_module);
            ALOGD("device.type %x", sources[i].ext.device.type);
            ALOGD("device.address %s", sources[i].ext.device.address);
        }
        else if (sources[i].type == AUDIO_PORT_TYPE_MIX)
        {
            ALOGD("mix.hw_module %x", sources[i].ext.mix.hw_module);
            ALOGD("mix.handle %x", sources[i].ext.mix.handle);
            ALOGD("mix.usecase.stream %x", sources[i].ext.mix.usecase.stream);
            ALOGD("mix.usecase.source %x", sources[i].ext.mix.usecase.source);
        }
        else if (sources[i].type == AUDIO_PORT_TYPE_SESSION)
        {

        }

    }

    for (i = 0; i < num_sinks ; i++)
    {
        ALOGD("== sinks [%d]/[%d] ==", i, num_sinks);
        ALOGD("id 0x%x", sinks[i].id);
        ALOGD("role 0x%x %s", sinks[i].role, strAudioPatchRole[sinks[i].role]);
        ALOGD("type 0x%x %s", sinks[i].type, strAudioPatchType[sinks[i].type]);
        ALOGD("config_mask 0x%x", sinks[i].config_mask);
        ALOGD("sample_rate 0x%x", sinks[i].sample_rate);
        ALOGD("channel_mask 0x%x", sinks[i].channel_mask);
        ALOGD("gain.index 0x%x", sinks[i].gain.index);
        ALOGD("gain.mode 0x%x", sinks[i].gain.mode);
        ALOGD("gain.channel_mask 0x%x", sinks[i].gain.channel_mask);
        ALOGD("gain.ramp_duration_ms 0x%x", sinks[i].gain.ramp_duration_ms);
#if 0 //When gain check , enable       
        for (j = 0; j < sizeof(audio_channel_mask_t) * 8; j++)
        {
            ALOGD("gain.values[%d] 0x%x", j, sinks[i].gain.values[j]);
        }
#endif
        if (sinks[i].type == AUDIO_PORT_TYPE_DEVICE)
        {
            ALOGD("device.hw_module %x", sinks[i].ext.device.hw_module);
            ALOGD("device.type %x", sinks[i].ext.device.type);
            ALOGD("device.address %s", sinks[i].ext.device.address);
        }
        else if (sinks[i].type == AUDIO_PORT_TYPE_MIX)
        {
            ALOGD("mix.hw_module %x", sinks[i].ext.mix.hw_module);
            ALOGD("mix.handle %x", sinks[i].ext.mix.handle);
            ALOGD("mix.usecase.stream %x", sinks[i].ext.mix.usecase.stream);
            ALOGD("mix.usecase.source %x", sinks[i].ext.mix.usecase.source);
        }
        else if (sinks[i].type == AUDIO_PORT_TYPE_SESSION)
        {

        }
    }


#endif
#if 1
    //    ALOGD("+%s num_sources [%d] , num_sinks [%d]", __FUNCTION__, num_sources, num_sinks);
    audio_devices_t eOutDeviceList = AUDIO_DEVICE_NONE;
    audio_devices_t eInputDeviceList = AUDIO_DEVICE_NONE;
    audio_source_t eInputSource = AUDIO_SOURCE_DEFAULT;
    do
    {
        if (handle == NULL || sources == NULL || sinks == NULL)
        {
            ALOGW("Ptr is null");
            status = BAD_VALUE;
            break;
        }
        // We can support legacy routing with setting single source or single sink
        if ((!num_sources && !num_sinks) || (num_sources > 1) || (num_sinks > AUDIO_PATCH_PORTS_MAX))
        {
            ALOGW("num is invalid");
            status = BAD_VALUE;
            break;
        }

        if (sources[0].type == AUDIO_PORT_TYPE_MIX)
        {

            if (sinks[0].type != AUDIO_PORT_TYPE_DEVICE)
            {
                ALOGW("sinks[0].type != AUDIO_PORT_TYPE_DEVICE");
                status = BAD_VALUE;
                break;
            }

            int dDeviceIndex;

            for (dDeviceIndex = 0; dDeviceIndex < num_sinks; dDeviceIndex++)
            {
                eOutDeviceList |= sinks[dDeviceIndex].ext.device.type;
            }
            if (eOutDeviceList == AUDIO_DEVICE_NONE)
            {
                ALOGW("Mixer->Device Routing to AUDIO_DEVICE_NONE");
                status = BAD_VALUE;
                break;
            }
            ALOGD("+routing createAudioPatch Mixer->%x", eOutDeviceList);
            AudioParameter param;
            param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
            status = mStreamManager->setParameters(param.toString(), sources[0].ext.mix.handle);
            //Gain Apply to do
            if (status == NO_ERROR)
            {
                ssize_t index;
                ssize_t total = mAudioHalPatchVector.size();
                for (index = total-1;index >= 0; index--)
                {

                    if (mAudioHalPatchVector[index]->sources[0].type == AUDIO_PORT_TYPE_MIX &&
                        mAudioHalPatchVector[index]->sinks[0].type == AUDIO_PORT_TYPE_DEVICE &&
                        sources[0].ext.mix.handle == mAudioHalPatchVector[index]->sources[0].ext.mix.handle)
                    {
                        AudioHalPatch *patch;
                        patch = mAudioHalPatchVector[index];
                        ALOGD("handlecheck createAudioPatch() removing patch handle %d index %u DL", mAudioHalPatchVector[index]->mHalHandle,index);
                        mAudioHalPatchVector.removeAt(index);
                        delete(patch);
                        break;
                    }
                }
            }
            else
            {
                ALOGE("Err %s %d",__FUNCTION__,__LINE__);
            }

        }
        else if (sources[0].type == AUDIO_PORT_TYPE_DEVICE)
        {

            if (sinks[0].type == AUDIO_PORT_TYPE_MIX)
            {

                eInputDeviceList = sources[0].ext.device.type;
                eInputSource = sinks[0].ext.mix.usecase.source;
                ALOGD("+routing createAudioPatch %x->Mixer Src %x", eInputDeviceList, eInputSource);
                AudioParameter param;
                param.addInt(String8(AudioParameter::keyRouting), (int)eInputDeviceList);
                param.addInt(String8(AudioParameter::keyInputSource),
                             (int)eInputSource);

                status = mStreamManager->setParameters(param.toString(), sinks[0].ext.mix.handle);
#if 0
                if (status != NO_ERROR)
                {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    break;
                }
                else if (eInputDeviceList == AUDIO_DEVICE_IN_FM_TUNER)
                {
                    if (mUseAudioPatchForFm == true)
                    {
                        // FM enable (indirect mode)
                        mStreamManager->setFmVolume(1.0); // For be sure mFmVolume doesn't equal to -1.0
                        status = mStreamManager->setFmEnable(true, true, false);
                    }
                }

                //Gain Apply to do
#else
                if (status == NO_ERROR)
                {
                    ssize_t index;
                    ssize_t total = mAudioHalPatchVector.size();
                    for (index = total-1;index >= 0; index--)
                    {

                        if (mAudioHalPatchVector[index]->sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
                            mAudioHalPatchVector[index]->sinks[0].type == AUDIO_PORT_TYPE_MIX &&
                            sinks[0].ext.mix.handle == mAudioHalPatchVector[index]->sinks[0].ext.mix.handle)
                        {
                            AudioHalPatch *patch;
                            patch = mAudioHalPatchVector[index];
                            ALOGD("handlecheck createAudioPatch() removing patch handle %d index %u UL", mAudioHalPatchVector[index]->mHalHandle,index);
                            mAudioHalPatchVector.removeAt(index);
                            delete(patch);
                            break;
                        }
                    }

                    if (eInputDeviceList == AUDIO_DEVICE_IN_FM_TUNER)
                    {
                        if (mUseAudioPatchForFm == true)
                        {
                            status = mStreamManager->setFmEnable(true, true, false);
                        }
                    }
                }
                else
                {
                    ALOGE("Err %s %d",__FUNCTION__,__LINE__);
                }
#endif
            }
            else if (sinks[0].type == AUDIO_PORT_TYPE_DEVICE)
            {
                ALOGW("sinks[0].type == AUDIO_PORT_TYPE_DEVICE");
                // DO Device to Device
                eInputDeviceList = sources[0].ext.device.type;
                int dDeviceIndex;

                for (dDeviceIndex = 0; dDeviceIndex < num_sinks; dDeviceIndex++)
                {
                    eOutDeviceList |= sinks[dDeviceIndex].ext.device.type; //should be only one device , limited by frameworks
                }

                if (eInputDeviceList != AUDIO_DEVICE_IN_FM_TUNER || !(eOutDeviceList & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES))
                {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;
                    break;
                }
                else if (eInputDeviceList == AUDIO_DEVICE_IN_FM_TUNER)
                {

                    if (mUseAudioPatchForFm == false)
                    {
                        ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                        status = INVALID_OPERATION;
                        break;
                    }
#if 1
                    AudioParameter param;
                    param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
                    //The sources[0].ext.mix.handle doesn't record IOport
                    status = mStreamManager->setParametersToStreamOut(param.toString());

                    if (status != NO_ERROR)
                    {
                        ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                        break;
                    }

                    ALOGD("+routing createAudioPatch %x->%x",eInputDeviceList,eOutDeviceList);
                    mStreamManager->setFmVolume(0);//initial value is -1 , should change it first
                    // FM enable
                    status = mStreamManager->setFmEnable(true,true,true);
#else
                    /*
                                        else if (eOutDeviceList != AUDIO_DEVICE_OUT_WIRED_HEADSET && eOutDeviceList != AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
                                        {
                                            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                                            status = INVALID_OPERATION;
                                            break;
                                        }
                    */
                    else // FM earphone
                    {
                        int dGainDB = 256;
                        unsigned int ramp_duration_ms = 0;

                        if (sinks[0].config_mask & AUDIO_PORT_CONFIG_GAIN)
                        {
                            if (sinks[0].gain.mode & AUDIO_GAIN_MODE_JOINT | AUDIO_GAIN_MODE_CHANNELS) //Hw support joint only
                            {
                                dGainDB = sinks[0].gain.values[0] / 100;
                                if (dGainDB >= 0)//Support degrade only
                                {
                                    dGainDB = 0;
                                }
                                else
                                {
                                    dGainDB = (-1) * dGainDB;
                                }

                                dGainDB = dGainDB << 2;
                                if (dGainDB > 256)
                                {
                                    dGainDB = 256;
                                }
                                dGainDB = 256 - dGainDB;
                            }

                            if (sinks[0].gain.mode & AUDIO_GAIN_MODE_RAMP)
                            {
                                ramp_duration_ms = sinks[0].gain.ramp_duration_ms;
                            }
                        }

                        // routing
                        for (dDeviceIndex = 0; dDeviceIndex < num_sinks; dDeviceIndex++)
                        {
                            eOutDeviceList |= sinks[dDeviceIndex].ext.device.type;
                        }

                        AudioParameter param;
                        param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
                        //status = mStreamManager->setParameters(param.toString(), sources[0].ext.mix.handle);

                        status = mStreamManager->setParametersToStreamOut(param.toString());//The sources[0].ext.mix.handle doesn't record IOport

                        if (status != NO_ERROR)
                        {
                            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                            break;
                        }
                        else
                        {
#ifndef MTK_AUDIO_GAIN_TABLE
                            float fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
                            ALOGD("fFMVolume %f", fFMVolume);

                            if (fFMVolume < 0)
                            {
                                fFMVolume = 0;
                            }
                            else if (fFMVolume > 1.0)
                            {
                                fFMVolume = 1.0;
                            }
                            ALOGD("+routing createAudioPatch %x->%x", eInputDeviceList, eOutDeviceList);
                            mStreamManager->setFmVolume(fFMVolume); // initial value is -1 , should change it first
#endif
                            // FM enable
                            status = mStreamManager->setFmEnable(true, true, true);
                            //FMTODO : Gain setting

                        }
                    }
#endif
                    break;
                }
            }

        }



    }
    while (0);

    if (status == NO_ERROR)
    {
#if 0
        for (size_t index = 0; *handle != AUDIO_PATCH_HANDLE_NONE && index < mAudioHalPatchVector.size(); index++)
        {
            if (*handle == mAudioHalPatchVector[index]->mHalHandle)
            {
                ALOGV("createAudioPatch() removing patch handle %d", *handle);
                mAudioHalPatchVector.removeAt(index);
//HTC_AUD_START
                delete(removePatch);//HTC_AUD
//HTC_AUD_END
                break;
            }
        }
#endif
        *handle = android_atomic_inc(&mNextUniqueId);
        AudioHalPatch *newPatch = new AudioHalPatch(*handle);
        newPatch->num_sources = num_sources;
        newPatch->num_sinks = num_sinks;
        for (unsigned int index = 0; index < num_sources ; index++)
        {
            memcpy((void *)&newPatch->sources[index], (void *)&sources[index], sizeof(struct audio_port_config));
        }
        for (unsigned int index = 0; index < num_sinks ; index++)
        {
            memcpy((void *)&newPatch->sinks[index], (void *)&sinks[index], sizeof(struct audio_port_config));
        }
        mAudioHalPatchVector.add(newPatch);

        ALOGD("handlecheck %s sucess new *handle 0x%x", __FUNCTION__, (int)(*handle));
    }
    else
    {
        ALOGD("Fail status %d", (int)(status));
    }
    ALOGD("-%s num_sources [%d] , num_sinks [%d]", __FUNCTION__, num_sources, num_sinks);
#endif
    return status;
}

int AudioALSAHardware::releaseAudioPatch(audio_patch_handle_t handle)
{
    //TODO
    int status = NO_ERROR;
    ssize_t index;
    bool bReturnFlag = false;
    AudioHalPatch *patch;
    ALOGD("-%s handle [0x%x]", __FUNCTION__, handle);
    do
    {

        if (handle == AUDIO_PATCH_HANDLE_NONE)
        {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = BAD_VALUE;
            return status;
        }

        for (index = 0; index < mAudioHalPatchVector.size(); index++)
        {
            if (handle == mAudioHalPatchVector[index]->mHalHandle)
            {
                break;
            }
        }
        if (index == mAudioHalPatchVector.size())
        {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = INVALID_OPERATION;
            return status;
        }

        patch = mAudioHalPatchVector[index];
        mAudioHalPatchVector.removeAt(index);

        if (patch->sources[0].type == AUDIO_PORT_TYPE_MIX)
        {

            if (patch->sinks[0].type != AUDIO_PORT_TYPE_DEVICE)
            {
                ALOGW("sinks[0].type != AUDIO_PORT_TYPE_DEVICE");
                status = BAD_VALUE;
                break;
            }
            ALOGD("+routing releaseAudioPatch Mixer->%x", patch->sinks[0].ext.device.type);
#if 1
            for (index = mAudioHalPatchVector.size()-1; index >= 0; index--)
            {
                for (int sink_i=0;sink_i<mAudioHalPatchVector[index]->num_sinks;sink_i++)
                {
                    if ((mAudioHalPatchVector[index]->sinks[sink_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                        (mAudioHalPatchVector[index]->sinks[sink_i].ext.device.type != AUDIO_DEVICE_NONE))
                    {
                            ALOGD("Still have AudioPatches routing to outputDevice, Don't routing null Size %d",mAudioHalPatchVector.size());
                            status = NO_ERROR;
                            bReturnFlag = true;
                            break;
                    }
                }
            }
            if (bReturnFlag)
                break;
#else
            for (index = 0; index < mAudioHalPatchVector.size(); index++)
            {
                for (int sink_i = 0; sink_i < mAudioHalPatchVector[index]->num_sinks; sink_i++)
                {
                    if (mAudioHalPatchVector[index]->sinks[sink_i].type == AUDIO_PORT_TYPE_DEVICE && mAudioHalPatchVector[index]->sinks[sink_i].ext.device.type != AUDIO_DEVICE_NONE)
                    {
                        ALOGD("Still have AudioPatches routing to outputDevice, Don't routing null Size %d", mAudioHalPatchVector.size());
                        status = NO_ERROR;
                        bReturnFlag = true;
                        break;
                    }
                }
            }
            if (bReturnFlag)
            {
                break;
            }
#endif
#if 0   //Policy doesn't change to none , it will non-sync
            AudioParameter param;
            param.addInt(String8(AudioParameter::keyRouting), (int)AUDIO_DEVICE_NONE);
            status = mStreamManager->setParameters(param.toString(), patch->sources[0].ext.mix.handle);
#endif
        }
        else if (patch->sources[0].type == AUDIO_PORT_TYPE_DEVICE)
        {

            if (patch->sinks[0].type == AUDIO_PORT_TYPE_MIX)
            {
                // close FM if need (indirect)
                ALOGD("+routing releaseAudioPatch %x->Mixer", patch->sources[0].ext.device.type);
                if (mUseAudioPatchForFm == true)
                {
                    if (patch->sources[0].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER)
                    {
                        for (index = mAudioHalPatchVector.size()-1; index >= 0; index--)
                        {
                            for (int source_i = 0; source_i < mAudioHalPatchVector[index]->num_sources; source_i++)
                            {
                                if ((mAudioHalPatchVector[index]->sources[source_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                                    (mAudioHalPatchVector[index]->sources[source_i].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER))
                                {
                                        ALOGD("Still have AudioPatches need  AUDIO_DEVICE_IN_FM_TUNER, Don't Disable FM [%d]",
                                            mAudioHalPatchVector.size());
                                    status = NO_ERROR;
                                    bReturnFlag = true;
                                    break;
                                }
                            }
                        }

                        if (!bReturnFlag)
                        {
                            status = mStreamManager->setFmEnable(false);
                        }
                    }
                }

                audio_devices_t eInDeviceList = AUDIO_DEVICE_NONE;
                for (index = mAudioHalPatchVector.size()-1; index >= 0; index--)
                {
                    for (int source_i = 0; source_i < mAudioHalPatchVector[index]->num_sources; source_i++)
                    {
                        if ((mAudioHalPatchVector[index]->sources[source_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                            (mAudioHalPatchVector[index]->sources[source_i].ext.device.type != AUDIO_DEVICE_NONE))
                        {
                            eInDeviceList = mAudioHalPatchVector[index]->sources[source_i].ext.device.type;
                                ALOGD("Still have AudioPatches need  routing input device, Don't change routing [%d]"
                                    ,mAudioHalPatchVector.size());
                            status = NO_ERROR;
                            break;
                        }
                    }
                }

                AudioParameter param;
                param.addInt(String8(AudioParameter::keyRouting), (int)eInDeviceList);
                status = mStreamManager->setParameters(param.toString(), patch->sinks[0].ext.mix.handle);
            }
            else if (patch->sinks[0].type == AUDIO_PORT_TYPE_DEVICE)
            {
                ALOGD("+routing releaseAudioPatch %x->%x", patch->sources[0].ext.device.type, patch->sinks[0].ext.device.type);
                ALOGW("sinks[0].type == AUDIO_PORT_TYPE_DEVICE");
                if ((patch->sources[0].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER) && (patch->sinks[0].ext.device.type & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES))
                {
                    // close FM if need (direct)
                    if (mUseAudioPatchForFm == false)
                    {
                        ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                        status = INVALID_OPERATION;
                    }
                    else
                    {
                        for (index = mAudioHalPatchVector.size()-1; index >= 0; index--)
                        {
                            for (int source_i = 0; source_i < mAudioHalPatchVector[index]->num_sources; source_i++)
                            {
                                if ((mAudioHalPatchVector[index]->sources[source_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                                    (mAudioHalPatchVector[index]->sources[source_i].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER))
                                {
                                        ALOGD("Still have AudioPatches need  AUDIO_DEVICE_IN_FM_TUNER, Don't Disable FM [%d]",
                                            mAudioHalPatchVector.size());
                                    status = NO_ERROR;
                                    bReturnFlag = true;
                                    break;
                                }
                            }
                        }
                        if (!bReturnFlag)
                        {
                            status = mStreamManager->setFmEnable(false);
                            mStreamManager->setFmVolume(0);
                        }

                        audio_devices_t eOutDeviceList = AUDIO_DEVICE_NONE;
                        //Restore previous output device setting
                        for (index = mAudioHalPatchVector.size()-1; index >= 0; index--)
                        {
                            for (int sink_i = 0; sink_i < mAudioHalPatchVector[index]->num_sinks; sink_i++)
                            {
                                if ((mAudioHalPatchVector[index]->sinks[sink_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                                    (mAudioHalPatchVector[index]->sinks[sink_i].ext.device.type != AUDIO_DEVICE_NONE) &&
                                    (mAudioHalPatchVector[index]->sources[0].type == AUDIO_PORT_TYPE_MIX))
                                {
                                        eOutDeviceList = eOutDeviceList|mAudioHalPatchVector[index]->sinks[sink_i].ext.device.type;
                                        ALOGD("Still have AudioPatches routing to outputDevice, Don't routing null sink_i/Size %d/%d , device 0x%x handle %x",
                                            sink_i,mAudioHalPatchVector.size(),eOutDeviceList,mAudioHalPatchVector[index]->mHalHandle);
                                    status = NO_ERROR;
                                }
                            }
                            if (eOutDeviceList)
                                    break;
                        }
                        AudioParameter param;
                        param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
                        status = mStreamManager->setParametersToStreamOut(param.toString());

                    }
                    break;
                }
                else
                {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;//TODO
                    break;
                }

            }
        }
    }
    while (0);

    if (status == NO_ERROR)
    {
        ALOGD("handlecheck %s remove handle [%x] OK",__FUNCTION__,handle);
        delete(patch);
    }
    else
    {
        ALOGD("handlecheck %s remove handle [%x] NG",__FUNCTION__,handle);
        mAudioHalPatchVector.add(patch);
    }
    ALOGD("-%s handle [0x%x] status [%d]", __FUNCTION__, handle, status);
    return status;
}

int AudioALSAHardware::getAudioPort(struct audio_port *port)
{
    //TODO , I think the implementation is designed in aps.
    ALOGW("-%s Unsupport", __FUNCTION__);
    return INVALID_OPERATION;
}

// We limit valid for existing Audio port of AudioPatch
int AudioALSAHardware::setAudioPortConfig(const struct audio_port_config *config)
{
    int status = NO_ERROR;

    do
    {

        if (config == NULL)
        {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = BAD_VALUE;
            break;
        }

        if ((config->config_mask & AUDIO_PORT_CONFIG_GAIN) == 0)
        {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = INVALID_OPERATION;
            break;
        }

        ALOGD("%s", __FUNCTION__);
        ALOGD("config->type [0x%x]", config->type);
        ALOGD("config->role [0x%x]", config->role);
        ALOGD("config->gain.mode [0x%x]", config->gain.mode);
        ALOGD("config->gain.values[0] [0x%x]", config->gain.values[0]);
        ALOGD("config->gain.ramp_duration_ms [0x%x]", config->gain.ramp_duration_ms);

        if (config->type == AUDIO_PORT_TYPE_MIX)
        {
            if (config->role == AUDIO_PORT_ROLE_SOURCE)
            {
                //Apply Gain to MEMIF , don't support it so far
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = INVALID_OPERATION;
                break;
            }
            else if (config->role == AUDIO_PORT_ROLE_SINK)
            {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = INVALID_OPERATION;
                break;
            }
            else
            {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = BAD_VALUE;
                break;
            }
        }
        else if (config->type == AUDIO_PORT_TYPE_DEVICE)
        {
            if (mUseAudioPatchForFm == false)
            {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = INVALID_OPERATION;
                break;
            }
            if (config->role == AUDIO_PORT_ROLE_SINK || config->role == AUDIO_PORT_ROLE_SOURCE)
            {
                //Support specific device eg. headphone/speaker
                size_t indexOfPatch;
                size_t indexOfSink;
                audio_port_config *pstCurConfig = NULL;
                bool bhit = false;
                for (indexOfPatch = 0; indexOfPatch < mAudioHalPatchVector.size() && !bhit; indexOfPatch++)
                {
                    for (indexOfSink = 0; indexOfSink < mAudioHalPatchVector[indexOfPatch]->num_sinks; indexOfSink++)
                    {
                        if ((config->ext.device.type == mAudioHalPatchVector[indexOfPatch]->sinks[indexOfSink].ext.device.type)
                            && (mAudioHalPatchVector[indexOfPatch]->sources[indexOfSink].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER)
                            && (mAudioHalPatchVector[indexOfPatch]->sinks[indexOfSink].ext.device.type & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES))
                        {
                            bhit = true;
                            pstCurConfig = &(mAudioHalPatchVector[indexOfPatch]->sinks[indexOfSink]);
                            break;
                        }
                    }
                }

                if (!bhit || pstCurConfig == NULL)
                {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;
                    break;
                }

                if (!config->gain.mode)
                {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;
                    break;
                }
#if 0
                int dGainDB = 0;
                unsigned int ramp_duration_ms = 0;
                if (config->gain.mode & AUDIO_GAIN_MODE_JOINT | AUDIO_GAIN_MODE_CHANNELS) //Hw support joint only
                {
                    dGainDB = config->gain.values[0] / 100;
                    if (dGainDB >= 0)//Support degrade only
                    {
                        dGainDB = 0;
                    }
                    else
                    {
                        dGainDB = (-1) * dGainDB;
                    }

                    dGainDB = dGainDB << 2;
                    if (dGainDB > 256)
                    {
                        dGainDB = 256;
                    }
                    dGainDB = 256 - dGainDB;
                }

                if (config->gain.mode & AUDIO_GAIN_MODE_RAMP)
                {
                    ramp_duration_ms = config->gain.ramp_duration_ms;
                }

#ifndef MTK_AUDIO_GAIN_TABLE
                //FMTODO : Gain setting
                float fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
                ALOGD("fFMVolume %f", fFMVolume);
                if (fFMVolume >= 0 && fFMVolume <= 1.0)
                {
                    mStreamManager->setFmVolume(fFMVolume);
                }
                else
                {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = BAD_VALUE;
                    break;
                }
#endif
#else
                int dGainDB = 0;
                unsigned int ramp_duration_ms = 0;
                float fFMVolume;
                if (config->gain.mode & AUDIO_GAIN_MODE_JOINT | AUDIO_GAIN_MODE_CHANNELS) //Hw support joint only
                {
                    fFMVolume = MappingFMVolofOutputDev(config->gain.values[0],pstCurConfig->ext.device.type);
                }
                else
                {
#ifndef MTK_AUDIO_GAIN_TABLE
                    fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
#else
                    fFMVolume = AudioMTKGainController::linearToLog(dGainDB);
#endif
                }

                if (config->gain.mode & AUDIO_GAIN_MODE_RAMP)
                {
                    ramp_duration_ms = config->gain.ramp_duration_ms;
            }

                ALOGD("fFMVolume %f",fFMVolume);
                if (fFMVolume >=0 && fFMVolume <=1.0)
                {
                    mStreamManager->setFmVolume(fFMVolume);
                }
                else
            {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = BAD_VALUE;
                break;
                }
#endif

            }
            else
            {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = BAD_VALUE;
                break;
            }
        }


    }
    while (0);

    //TODO
    return status;
}

float AudioALSAHardware::MappingFMVolofOutputDev(int Gain, audio_devices_t eOutput)
{
    float fFMVolume;
    if ((eOutput & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES) == 0)
    {
        ALOGE("Error FM createAudioPatch direct mode fail device [0x%x]",eOutput);
        return 1.0;
    }

    if (mUseTuningVolume == false)
    {
        int dGainDB = 0;
        dGainDB = Gain / 100;
        if (dGainDB >= 0)//Support degrade only
            dGainDB = 0;
        else
            dGainDB = (-1)*dGainDB;
        dGainDB = dGainDB<<2;
        if (dGainDB > 256)
            dGainDB = 256;
        dGainDB = 256- dGainDB;

#ifndef MTK_AUDIO_GAIN_TABLE
        fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
#else
        fFMVolume = AudioMTKGainController::linearToLog(dGainDB);
#endif

        ALOGD("default f fFMVolume %f",fFMVolume);
        if (fFMVolume < 0)
                fFMVolume = 0;
        else if (fFMVolume > 1.0)
                fFMVolume = 1.0;
    }
    else
    {
        const float fCUSTOM_VOLUME_MAPPING_STEP = 256.0f;
        unsigned char* array;

        if (eOutput & AUDIO_DEVICE_OUT_SPEAKER)
            array = VolCache.audiovolume_steamtype[CUSTOM_VOL_TYPE_MUSIC][CUSTOM_VOLUME_SPEAKER_MODE];
        else
            array = VolCache.audiovolume_steamtype[CUSTOM_VOL_TYPE_MUSIC][CUSTOM_VOLUME_HEADSET_MODE];

        int dIndex = 15- (((-1)*Gain)/300);
        int dMaxIndex = VolCache.audiovolume_level[CUSTOM_VOL_TYPE_MUSIC];
        ALOGD("FM index %d",dIndex);
        if (dIndex > 15)
            dIndex = 15;
        else if (dIndex < 0)
            dIndex = 0;
        float vol = (fCUSTOM_VOLUME_MAPPING_STEP * dIndex) / dMaxIndex;
        float volume =0.0;
        if (vol == 0) {
            volume = vol;
        } else {    // map volume value to custom volume
            float unitstep = fCUSTOM_VOLUME_MAPPING_STEP/dMaxIndex;
            if (vol < fCUSTOM_VOLUME_MAPPING_STEP/dMaxIndex) {
                volume = array[0];
            } else {
                int Index = (vol+0.5)/unitstep;
                vol -= (Index*unitstep);
                float Remind = (1.0 - (float)vol/unitstep);
                if (Index != 0) {
                    volume = ((array[Index]  - (array[Index] - array[Index-1]) * Remind)+0.5);
                } else {
                    volume = 0;
                }
            }
            // -----clamp for volume
            if ( volume > 253.0) {
                volume = fCUSTOM_VOLUME_MAPPING_STEP;
            } else if ( volume <= array[0]) {
                volume = array[0];
            }
        }

#ifndef MTK_AUDIO_GAIN_TABLE
        fFMVolume = AudioALSAVolumeController::linearToLog(volume);
#else
        fFMVolume = AudioMTKGainController::linearToLog(volume);
#endif

    }
    ALOGD("Final fFMVolume %f",fFMVolume);
    return fFMVolume;
}

android_audio_legacy::AudioStreamOut *AudioALSAHardware::openOutputStreamWithFlags(uint32_t devices,
                                                                                   audio_output_flags_t flags,
                                                                                   int *format,
                                                                                   uint32_t *channels,
                                                                                   uint32_t *sampleRate,
                                                                                   status_t *status)
{
    return mStreamManager->openOutputStream(devices, format, channels, sampleRate, status);
}


};

namespace android_audio_legacy
{

AudioStreamOut::~AudioStreamOut() {}

AudioStreamIn::~AudioStreamIn() {}

// default implementation is unsupported
status_t AudioStreamOut::getNextWriteTimestamp(int64_t *timestamp)
{
    return INVALID_OPERATION;
}
#if 0
status_t AudioStreamOut::setCallBack(stream_callback_t callback, void *cookie)
{
    return INVALID_OPERATION;
}
#endif


status_t AudioStreamOut::getPresentationPosition(uint64_t *frames, struct timespec *timestamp)
{
    return INVALID_OPERATION;
}


AudioMTKStreamOutInterface::~AudioMTKStreamOutInterface() {}
#if 0
AudioStreamOut *AudioHardwareInterface::openOutputStreamWithFlags(uint32_t devices,
                                                                  audio_output_flags_t flags,
                                                                  int *format,
                                                                  uint32_t *channels,
                                                                  uint32_t *sampleRate,
                                                                  status_t *status)
{
    return openOutputStream(devices, format, channels, sampleRate, status);
}
#endif
AudioMTKHardwareInterface *AudioMTKHardwareInterface::create()
{
    /*
     * FIXME: This code needs to instantiate the correct audio device
     * interface. For now - we use compile-time switches.
     */
    android_audio_legacy::AudioMTKHardwareInterface *hw = 0;
    char value[PROPERTY_VALUE_MAX];

    ALOGV("Creating MTK AudioHardware");
    hw = new android::AudioALSAHardware();

    return hw;

}

AudioHardwareInterface *AudioHardwareInterface::create()
{
    /*
     * FIXME: This code needs to instantiate the correct audio device
     * interface. For now - we use compile-time switches.
     */
    AudioHardwareInterface *hw = 0;
    char value[PROPERTY_VALUE_MAX];

    ALOGV("Creating MTK AudioALSAHardware");
    hw = new android::AudioALSAHardware();

    return hw;

}

extern "C" AudioMTKHardwareInterface *createMTKAudioHardware()
{
    /*
     * FIXME: This code needs to instantiate the correct audio device
     * interface. For now - we use compile-time switches.
     */
    return AudioMTKHardwareInterface::create();

}


extern "C" AudioHardwareInterface *createAudioHardware()
{
    return AudioHardwareInterface::create();
}

};


