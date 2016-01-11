#include "AudioALSAPlaybackHandlerBTCVSD.h"

#include <time.h>

#include "AudioUtility.h"

#include "WCNChipController.h"
#include "AudioBTCVSDControl.h"


#define LOG_TAG "AudioALSAPlaybackHandlerBTCVSD"

namespace android
{

static bool mBTMode_Open;

AudioALSAPlaybackHandlerBTCVSD::AudioALSAPlaybackHandlerBTCVSD(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mWCNChipController(WCNChipController::GetInstance()),
    mAudioBTCVSDControl(AudioBTCVSDControl::getInstance()),
    mFd2(mAudioBTCVSDControl->getFd())
{
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_BT_CVSD;
}


AudioALSAPlaybackHandlerBTCVSD::~AudioALSAPlaybackHandlerBTCVSD()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerBTCVSD::open()
{
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

    // HW attribute config // TODO(Harvey): query this
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = android_audio_legacy::AudioSystem::popCount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mWCNChipController->GetBTCurrentSamplingRateNumber();

    mBTMode_Open = mAudioBTCVSDControl->BT_SCO_isWideBand();

    // open mAudioBTCVSDControl
    mAudioBTCVSDControl->BTCVSD_Init(mFd2, mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels);


    // bit conversion
    initBitConverter();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBTCVSD::close()
{
    ALOGD("+%s()", __FUNCTION__);

#if 1 // clean bt buffer before closing
        const uint32_t mute_buf_len = 8192;
        char mute_buf[mute_buf_len];
        memset(mute_buf, 0, mute_buf_len);

        this->write(mute_buf, mute_buf_len);
        this->write(mute_buf, mute_buf_len);
        this->write(mute_buf, mute_buf_len);
        this->write(mute_buf, mute_buf_len);
#endif

    // close mAudioBTCVSDControl
    mAudioBTCVSDControl->BTCVSD_StandbyProcess(mFd2);

    // bit conversion
    deinitBitConverter();

    // debug pcm dump
    ClosePCMDump();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerBTCVSD::routing(const audio_devices_t output_devices)
{
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBTCVSD::pause()
{
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBTCVSD::resume()
{
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBTCVSD::flush()
{
    return INVALID_OPERATION;
}

status_t AudioALSAPlaybackHandlerBTCVSD::setVolume(uint32_t vol)
{
    return INVALID_OPERATION;
}


int AudioALSAPlaybackHandlerBTCVSD::drain(audio_drain_type_t type)
{
    return 0;
}


ssize_t AudioALSAPlaybackHandlerBTCVSD::write(const void *buffer, size_t bytes)
{
    ALOGV("%s(), buffer = %p, bytes = %d", __FUNCTION__, buffer, bytes);

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);


    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBuffer, bytes, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


    // write data to bt cvsd driver
    uint8_t *outbuffer, *inbuf, *workbuf;
    uint32_t insize, outsize, workbufsize, total_outsize, src_fs_s;

    inbuf = (uint8_t *)pBufferAfterBitConvertion;
    do
    {
        outbuffer = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf();
        outsize = SCO_TX_ENCODE_SIZE;
        insize = bytesAfterBitConvertion;
        workbuf = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDWorkBuf();
        workbufsize = SCO_TX_PCM64K_BUF_SIZE;
        total_outsize = 0;
        do
        {
            if(mBTMode_Open != mAudioBTCVSDControl->BT_SCO_isWideBand())
            {            
                ALOGD("BTSCO change mode after TX_Begin!!!");
                mAudioBTCVSDControl->BT_SCO_TX_End(mFd2);
                mAudioBTCVSDControl->BT_SCO_TX_Begin(mFd2, mStreamAttributeSource->sample_rate, mStreamAttributeSource->num_channels); 
                mBTMode_Open = mAudioBTCVSDControl->BT_SCO_isWideBand();
                return bytes;
            }
            
            if (mAudioBTCVSDControl->BT_SCO_isWideBand())
            {
                mAudioBTCVSDControl->btsco_process_TX_MSBC(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize, mStreamAttributeSource->sample_rate); // return insize is consumed size
                ALOGV("WriteDataToBTSCOHW, do mSBC encode outsize=%d, consumed size=%d, bytesAfterBitConvertion=%d", outsize, insize, bytesAfterBitConvertion);
            }
            else
            {
                mAudioBTCVSDControl->btsco_process_TX_CVSD(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize, mStreamAttributeSource->sample_rate); // return insize is consumed size
                ALOGV("WriteDataToBTSCOHW, do CVSD encode outsize=%d, consumed size=%d, bytesAfterBitConvertion=%d", outsize, insize, bytesAfterBitConvertion);
            }
            outbuffer += outsize;
            inbuf += insize;
            bytesAfterBitConvertion -= insize;
            ASSERT(bytesAfterBitConvertion >= 0);

            insize = bytesAfterBitConvertion;
            total_outsize += outsize;
        }
        while (total_outsize < BTSCO_CVSD_TX_OUTBUF_SIZE && outsize != 0);

        ALOGV("WriteDataToBTSCOHW write to kernel(+) total_outsize = %d", total_outsize);
        if (total_outsize > 0)
        {
            WritePcmDumpData((void *)(mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf()), total_outsize);
            ssize_t WrittenBytes = ::write(mFd2, mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf(), total_outsize);
        }
        ALOGV("WriteDataToBTSCOHW write to kernel(-) remaining bytes = %d", bytesAfterBitConvertion);
    }
    while (bytesAfterBitConvertion > 0);

    return bytes;
}


} // end of namespace android
