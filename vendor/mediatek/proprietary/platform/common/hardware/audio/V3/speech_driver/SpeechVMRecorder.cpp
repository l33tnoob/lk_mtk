#include "SpeechVMRecorder.h"

#include <linux/rtpm_prio.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <cutils/properties.h>

#include <utils/threads.h>

#include <ftw.h>

#include <hardware_legacy/power.h>

#include "SpeechType.h"

#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"

#include "SpeechDriverFactory.h"
#include "SpeechDriverInterface.h"

#include "SpeechDriverFactory.h"

//#define FORCE_ENABLE_VM
//#define VM_FILENAME_ONLY_USE_VM0_TO_VM7

#define LOG_TAG "SpeechVMRecorder"

namespace android
{

/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char PROPERTY_KEY_VM_INDEX[PROPERTY_KEY_MAX]      = "persist.af.vm_index";
const char PROPERTY_KEY_VM_RECYCLE_ON[PROPERTY_KEY_MAX] = "persist.af.vm_recycle_on";
//HTC_AUD_START
const char PROPERTY_KEY_DQ_ENABLE[PROPERTY_KEY_MAX]     = "persist.audio.dq";
//HTC_AUD_END
/*==============================================================================
 *                     Constant
 *============================================================================*/
static const char VM_RECORD_WAKELOCK_NAME[] = "VM_RECORD_WAKELOCK";

static const uint32_t kCondWaitTimeoutMsec = 100; // 100 ms (modem local buf: 10k, and EPL has 2304 byte for each frame (20 ms))

static const uint32_t kReadBufferSize = 0x4000;   // 16 k


/*==============================================================================
 *                     VM File Recycle
 *============================================================================*/
static const uint32_t kMaxNumOfVMFiles = 4096;

static const uint32_t kMinKeepNumOfVMFiles = 16;        // keep at least 16 files which will not be removed
static const uint32_t kMaxSizeOfAllVMFiles = 209715200; // Total > 200 M

static const char     kFolderOfVMFile[]     = "/sdcard/mtklog/audio_dump/";
static const char     kPrefixOfVMFileName[] = "/sdcard/mtklog/audio_dump/VMLog";
static const uint32_t kSizeOfPrefixOfVMFileName = sizeof(kPrefixOfVMFileName) - 1;
static const uint32_t kMaxSizeOfVMFileName = 128;

typedef struct
{
    char     path[kMaxSizeOfVMFileName];
    uint32_t size;
} vm_file_info_t;

static vm_file_info_t gVMFileList[kMaxNumOfVMFiles];
static uint32_t       gNumOfVMFiles = 0;
static uint32_t       gTotalSizeOfVMFiles; // Total size of VM files in SD card

static int GetVMFileList(const char *path, const struct stat *sb, int typeflag)
{
    if (strncmp(path, kPrefixOfVMFileName, kSizeOfPrefixOfVMFileName) != 0)
    {
        return 0;
    }

    if (gNumOfVMFiles >= kMaxNumOfVMFiles)
    {
        return 0;
    }

    // path
    strcpy(gVMFileList[gNumOfVMFiles].path, path);

    // size
    gVMFileList[gNumOfVMFiles].size = sb->st_size;
    gTotalSizeOfVMFiles += sb->st_size;

    // increase index
    gNumOfVMFiles++;

    return 0; // To tell ftw() to continue
}

static int CompareVMFileName(const void *a, const void *b)
{
    return strcmp(((vm_file_info_t *)a)->path,
                  ((vm_file_info_t *)b)->path);
}

//HTC_AUD_START - DQ
#define DEBUG_INTERMAL_RAM_VERBOSE 1

#define TAG_DATETIME false  "[_DATETIMESTR_XXXX]"/* with "YYYYMMDD_HHMMSS_XXX" */
#define TAG_DATETIME_DQ     "[_DATETIMESTR_]"    /* with "YYYYMMDD_HHMMSS" */
#define TAG_DATETIME_LEN    (19)
#define TAG_DATETIME_LEN_DQ (15)
#define DATA_DIR "/data/"
#define LOG_FOLDER_NAME "htclog/"
#define LOG_DIR DATA_DIR LOG_FOLDER_NAME

#define RAM_SIZE_30MB   (30*1024*1024)  //buffer size 30MB
#define RAM_SIZE_10MB   (10*1024*1024)  //buffer size 10MB
#define RAM_SIZE_1MB    (1024*1024)     //buffer size 1MB
pthread_t thread_dq_dump = (pthread_t) -1;

static char *RAM = NULL;
//RAM_SIZE will be changed by dq_allocate_mem() which depends on the device's memory size
static int RAM_SIZE = RAM_SIZE_10MB;
static int ringbuffer_offset = 0;
//spilit our RAM into ping-pong buffers,
static int pp_size = RAM_SIZE_1MB; // 1MB
static int pp_numbers = 0;
static int pp_index = 0;
static int pp_prev = 0;

struct pingpong_offset_info_type
{
    int used;
    int offset;
};
static struct pingpong_offset_info_type *pp_info;

static int log_dumping = 0;

static void replace_datetime_tag_dq(char *str)
{
    const char *tag_datetime = TAG_DATETIME_DQ;
    char buf [TAG_DATETIME_LEN_DQ + 1], *ptr;
    struct tm *ptm;
    time_t t;

    t = time(NULL);
    ptm = localtime(&t);

    for (; str;)
    {
        /* replace date time string */
        ptr = strstr(str, tag_datetime);

        if (!ptr)
            break;

        if (!ptm)
        {
            snprintf(buf, sizeof(buf), "RAND_%10d", rand());
        }
        else
        {
            snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d",
                    ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
                    ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        }
        memcpy(ptr, buf, strlen(tag_datetime));
    }
}

static void dump_ram_to_file()
{
    char log_path[PATH_MAX];
    char *buffer = NULL;
    FILE *fdw = NULL;
    int count = 4096;
    int i = 0;
    int j = 0;
    int k = 0;
    int oldest_index = 0;
    int found = 0;
    int temp_offset = 0;
    int temp_length = 0;
    int err = 0;

    //find the oldest ringbuffer ping-pong index
    int pp_counts = 0;
    int total_payload_sz = 0; //a rough value;
    int shown_steps = 0;

    // this shown_percent_inteval means system prints logs at every (100/interval) percent.
    // e.g: 100/inteval(20) = 5, means every 5% payloads written to sdcard, system prints a log.
    int shown_percent_inteval = 20;
    int shown_total_payload = 0;

    // check vm_file_path is valid
    int ret = AudiocheckAndCreateDirectory(LOG_DIR);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, LOG_DIR);
        goto end;
    }

    snprintf(log_path, sizeof(log_path), "%s%s", LOG_DIR, "qxdm_ram_" TAG_DATETIME_DQ ".vm");
    log_path [sizeof(log_path) - 1] = 0;

    ALOGD("LOG path before: %s\n", log_path);
    replace_datetime_tag_dq(log_path);
    ALOGD("LOG path after: %s\n", log_path);

    // open VM file
    fdw = fopen(log_path, "wb");
    if (fdw == NULL)
    {
        ALOGE("%s(), fopen(%s) fail!!", __FUNCTION__, log_path);
        goto end;
    }
    if (RAM == NULL)
    {
        ALOGE("%s(), Do not reserve RAM buffer successfully before!!", __FUNCTION__);
        goto end;
    }
    ALOGD("dump dqlog to [%s] ...\n", log_path);
#if 0
    i = 0;
    j = 0;
    for (i = 0; i < RAM_SIZE; i += count)
    {
        if (fwrite(RAM+i, sizeof(char), count, fdw) < 0)
        {
            ALOGD("write failed, total %d bytes, [%s]: %s\n", i, log_path, strerror(errno));
            break;
        }
        if (i > (int)(RAM_SIZE/100)* j)
        {
            j++;
            ALOGD("dump dqlog %d percents ...\n", i/(RAM_SIZE/100));
        }
    }
#else
    for (i = 0; i < pp_numbers; i++)
    {
        if (pp_info[i].used)
        {
            pp_counts++;
            shown_total_payload += pp_info[i].offset;
        }
        ALOGD("pp_info[%d].used = %d, pp_counts = %d, shown_total_payload %d", i, pp_info[i].used, pp_counts, shown_total_payload);
    }

    if (pp_counts == pp_numbers)
    {
        oldest_index = (pp_index + 1) % pp_numbers;
        ALOGD("[1].all [%d] ping-pong buffers are used. oldest_index = %d, pp_index = %d", pp_counts, oldest_index, pp_index);
    }
    else if ((0 < pp_counts) && (pp_counts < pp_numbers))
    {
        oldest_index = 0;
        ALOGD("[2].[%d] ping-pong buffers are used. oldest_index = %d, pp_index = %d", pp_counts, oldest_index, pp_index);
    }
    else if (pp_counts == 0)
    {
        oldest_index = 0;
        ALOGD("[3].[%d] ping-pong buffers are used. oldest_index = %d, pp_index = %d", pp_counts, oldest_index, pp_index);
    }

    if (shown_total_payload <= 0)
    {
        shown_total_payload = RAM_SIZE;
    }

    i = oldest_index;
    for (j = 0; j < pp_numbers; j++)
    {
        //start write pp[oldest -> index] to file
        if (pp_info[i].used)
        {
            //write this ping-pong[i]
            temp_offset = 0;
            temp_length = 0;
            for (temp_offset = 0; temp_offset < pp_info[i].offset; )
            {
                if (temp_offset + 4096 > pp_info[i].offset)
                {
                    temp_length = pp_info[i].offset - temp_offset;

                    if (DEBUG_INTERMAL_RAM_VERBOSE)
                    {
                        ALOGE("write pp[%d] move next length (ramain packet sz) = %d", i, temp_length);
                    }
                }
                else
                {
                    temp_length = 4096;
                }
                if (fwrite(RAM + (pp_size * i) + temp_offset, sizeof(char), temp_length, fdw) < 0)
                {
                    ALOGE("write [%s]: %s\n", log_path, strerror (errno));
                    goto end;
                }

                temp_offset = temp_offset + temp_length;
                total_payload_sz = total_payload_sz + temp_length;

                if (DEBUG_INTERMAL_RAM_VERBOSE)
                {
                    ALOGE("write pp[%d] %d bytes --> temp_offset %d, total_payload_sz %d", i, temp_length, temp_offset, total_payload_sz);
                }

                if (total_payload_sz > (int)(shown_total_payload / shown_percent_inteval) * shown_steps)
                {
                    shown_steps++;
                    ALOGD("dump dqlog %d percents ...\n", total_payload_sz / (shown_total_payload / 100));
                }
            }
        }
        else
        {
            if (DEBUG_INTERMAL_RAM_VERBOSE)
            {
                ALOGE("pp[%d] is never used, skip to write to file.\n", i);
            }
        }
        i = (i + 1) % pp_numbers;
    }

    //error check if there is no any payload written to sdcard.
    if (total_payload_sz <= 0)
    {
        ALOGE("no any mem buffers dumps to storage ?");
        if (pp_counts <= 0)
        {
            //there is no any buffers written to memory.
            //we dump empty information to dqlog
            char empty_info[16] = "empty          ";
            if (fwrite(empty_info, sizeof(char), sizeof(empty_info), fdw) < 0)
            {
                ALOGE("write [%s]: %s\n", log_path, strerror (errno));
                goto end;
            }
            ALOGE("DUMP EMPTY DATA TO DQLOG.");
        }
    }
#endif
    ALOGD("dump dqlog end...\n");
end:
    if (fdw != NULL)
        fclose(fdw);
}

static void *thread_dump_to_file(void *arg)
{
    acquire_wake_lock(PARTIAL_WAKE_LOCK, VM_RECORD_WAKELOCK_NAME);
    prctl (PR_SET_NAME, (unsigned long) "dq_dump_file", 0, 0, 0);
    pthread_detach (pthread_self());
    dump_ram_to_file();
    log_dumping = 0;
    release_wake_lock(VM_RECORD_WAKELOCK_NAME);
    return NULL;
}

status_t SpeechVMRecorder::DumpRAM()
{
    log_dumping = 1;

    if (pthread_create(&thread_dq_dump, NULL, thread_dump_to_file, NULL) < 0)
        ALOGE("thread_dump_to_file pthread_create failed: %s\n", strerror(errno));
    else
        ALOGD("thread_dump_to_file pthread_create successfully\n");

    return NO_ERROR;
}

static unsigned long copyToRAM(unsigned char *ptr, int len)
{
    int temp_count = 0;
    int temp_index = 0;
    int temp_offset = 0;

    if (RAM)
    {
        temp_index = 0;
        temp_offset = 0;

        if ((len + pp_info[pp_index].offset) >= pp_size)
        {
            temp_index = pp_index;

            //packet size is over this ping-pong buffer
            //try to write this packet to the next ping-pong buffer
            pp_prev = pp_index;
            pp_index = (pp_index+1)%pp_numbers;
            pp_info[pp_index].used = 1;
            pp_info[pp_index].offset = 0;

            temp_offset = pp_info[pp_index].offset;

            ringbuffer_offset = pp_index*pp_size;
            memcpy(RAM+ringbuffer_offset, ptr, len);
            pp_info[pp_index].offset = pp_info[pp_index].offset + len;

            if (DEBUG_INTERMAL_RAM_VERBOSE)
            {
                ALOGE("[1]: len %d > pp_info[%d].offset = %d --> move to pp_info[%d].offset = %d, ringbuffer_offset %d, pp_numbers %d\n",
                    len, pp_prev, pp_info[pp_prev].offset,
                    pp_index, temp_offset, ringbuffer_offset, pp_numbers);
            }
        }
        else
        {
            temp_index = pp_index;
            temp_offset = pp_info[pp_index].offset;

            pp_info[pp_index].used = 1;

            ringbuffer_offset = pp_index*pp_size + pp_info[pp_index].offset;
            memcpy(RAM+ringbuffer_offset, ptr, len);
            pp_info[pp_index].offset = pp_info[pp_index].offset + len;

            if (DEBUG_INTERMAL_RAM_VERBOSE)
            {
                ALOGE("[2]: len %d < pp_info[%d].offset = %d --> pp_info[%d].offset = %d, ringbuffer_offset %d, pp_numbers %d\n",
                    len, temp_index, temp_offset,
                    pp_index, pp_info[pp_index].offset, ringbuffer_offset, pp_numbers);
            }
        }
    }
    else
    {
        ALOGE("No available RAM to write DQ log packets, skip\n");
    }

    return len;
}

static unsigned long get_mem_info()
{
    char buffer[1024];

    int fd = open("/proc/meminfo", O_RDONLY);

    if (fd < 0)
    {
        ALOGD("Unable to open /proc/meminfo: %s\n", strerror(errno));
        return 0;
    }

    const int len = read(fd, buffer, sizeof(buffer)-1);
    close(fd);

    if (len < 0)
    {
        ALOGD("Empty /proc/meminfo");
        return 0;
    }
    buffer[len] = 0;

    static const char* const tags = "MemTotal:";
    static const int tagsLen = 9;
    unsigned long mem = 0;

    char *p = buffer;
    if (*p)
    {
        if (strncmp(p, tags, tagsLen) == 0)
        {
            p += tagsLen;
            while (*p == ' ') p++;
            char* num = p;
            while (*p >= '0' && *p <= '9') p++;
            if (*p != 0)
            {
                *p = 0;
                p++;
            }
            mem = atol(num);
        }
    }
    ALOGD("read mem from meminfo = %lu\n", mem);
    return mem;
}

static int dq_allocate_mem()
{
#if 0
    unsigned long memory_size_long = 0;
    memory_size_long = get_mem_info();

    if (memory_size_long <= (1024*1024))
    {
        RAM_SIZE = RAM_SIZE_10MB; //use the default value
    }
    else
    {
        RAM_SIZE = RAM_SIZE_30MB;
    }
    ALOGD("change RAM_SIZE to (%d) MB, total mem %lu MB\n", (RAM_SIZE/(1024*1024)), memory_size_long/1024);
#endif

    if ((RAM = new char[RAM_SIZE]) == NULL)
    {
        ALOGE ("alloc RAM buffer failed: %s (%d)\n", strerror (errno), errno);
        return -1;
    }
    memset(RAM, 0x0, RAM_SIZE);

    //calculate all ping pong buffers index
    pp_numbers = RAM_SIZE/pp_size;
    if (pp_numbers <= 0)
    {
        ALOGD("pp_numbers is less than 0.\n");
        if (RAM)
        {
            delete RAM;
            RAM = NULL;
        }
        return -1;
    }

    pp_index = 0;
    pp_prev = pp_index;
    pp_info = new struct pingpong_offset_info_type[pp_numbers];

    if (pp_info == NULL)
    {
        ALOGD("alloc mem for pp_info failed: %s(%d)\n", strerror(errno), errno);
        if (RAM)
        {
            delete RAM;
            RAM = NULL;
        }
        return -1;
    }

    memset(pp_info, 0x0, sizeof(struct pingpong_offset_info_type)*pp_numbers);

    ALOGD("RAM_SIZE %d, pp_size=%d, pp_numbers=%d, pp_index=%d, pp_prev=%d, pp_info=%p[sz=%d, index %d]",
        RAM_SIZE, pp_size, pp_numbers, pp_index, pp_prev, pp_info, sizeof(struct pingpong_offset_info_type)*pp_numbers, pp_numbers);

    return 0;
}

static int dq_free_mem()
{
    if (RAM != NULL)
    {
        ALOGD("free RAM %p\n", RAM);
        delete RAM;
        RAM = NULL;
    }

    if (pp_info != NULL)
    {
        ALOGD("free pp_info %p\n", pp_info);
        delete pp_info;
        pp_info = NULL;
    }

    return 0;
}
//HTC_AUD_END - DQ

/*==============================================================================
 *                     Implementation
 *============================================================================*/

SpeechVMRecorder *SpeechVMRecorder::mSpeechVMRecorder = NULL;
SpeechVMRecorder *SpeechVMRecorder::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mSpeechVMRecorder == NULL)
    {
        mSpeechVMRecorder = new SpeechVMRecorder();
    }
    ASSERT(mSpeechVMRecorder != NULL);
    return mSpeechVMRecorder;
}

SpeechVMRecorder::SpeechVMRecorder()
{
    mStarting = false;
    mEnable = false;
//HTC_AUD_START
    mEnableDq = false;
//HTC_AUD_END
    mDumpFile = NULL;
    memset((void *)&mRingBuf, 0, sizeof(RingBuf));

    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    GetNBSpeechParamFromNVRam(&eSphParamNB);
    mAutoVM = eSphParamNB.uAutoVM;

    m_CtmDebug_Started = false;
    pCtmDumpFileUlIn = NULL;
    pCtmDumpFileDlIn = NULL;
    pCtmDumpFileUlOut = NULL;
    pCtmDumpFileDlOut = NULL;
}

SpeechVMRecorder::~SpeechVMRecorder()
{
    Close();
}

status_t SpeechVMRecorder::Open()
{
    mMutex.lock();

    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mEnable == false);

    int ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, VM_RECORD_WAKELOCK_NAME);
    ALOGD("%s(), acquire_wake_lock: %s, return %d.", __FUNCTION__, VM_RECORD_WAKELOCK_NAME, ret);

    // create another thread to avoid fwrite() block CCCI read thread
    pthread_create(&mRecordThread, NULL, DumpVMRecordDataThread, (void *)this);

    mMutex.unlock();
    mEnable = true;

    ALOGD("-%s(), mEnable=%d ", __FUNCTION__, mEnable );
    return NO_ERROR;
}

status_t SpeechVMRecorder::OpenFile()
{
    char vm_file_path[kMaxSizeOfVMFileName];
    memset((void *)vm_file_path, 0, kMaxSizeOfVMFileName);

#ifdef VM_FILENAME_ONLY_USE_VM0_TO_VM7
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VM_INDEX, property_value, "0");

    uint8_t vm_file_number = atoi(property_value);
    sprintf(vm_file_path, "%s_%u.vm", kPrefixOfVMFileName, vm_file_number++);

    sprintf(property_value, "%u", vm_file_number & 0x7);
    property_set(PROPERTY_KEY_VM_INDEX, property_value);
#else
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strcpy(vm_file_path, kPrefixOfVMFileName);
    strftime(vm_file_path + kSizeOfPrefixOfVMFileName, kMaxSizeOfVMFileName - kSizeOfPrefixOfVMFileName - 1, "_%Y_%m_%d_%H%M%S.vm", timeinfo);
#endif

    ALOGD("%s(), vm_file_path: \"%s\"", __FUNCTION__, vm_file_path);

    // check vm_file_path is valid
    int ret = AudiocheckAndCreateDirectory(vm_file_path);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, vm_file_path);
        return UNKNOWN_ERROR;
    }

    // open VM file
    mDumpFile = fopen(vm_file_path, "wb");
    if (mDumpFile == NULL)
    {
        ALOGE("%s(), fopen(%s) fail!!", __FUNCTION__, vm_file_path);
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

uint16_t SpeechVMRecorder::CopyBufferToVM(RingBuf ul_ring_buf)
{
    mMutex.lock();

    if (mStarting == false)
    {
        ALOGD("%s(), mStarting == false, return.", __FUNCTION__);
        mExitCond.signal(); // wake up thread to exit
        mMutex.unlock();
        return 0;
    }

    // get free space of internal input buffer
    uint16_t free_space = RingBuf_getFreeSpace(&mRingBuf);
    SLOGV("%s(), mRingBuf remain data count: %u, free_space: %u", __FUNCTION__, RingBuf_getDataCount(&mRingBuf), free_space);

    // get data count in share buffer
    uint16_t ul_data_count = RingBuf_getDataCount(&ul_ring_buf);
    SLOGV("%s(), ul_ring_buf data count: %u", __FUNCTION__, ul_data_count);

    // check free space for internal input buffer
    uint16_t copy_data_count = 0;
    if (ul_data_count <= free_space)
    {
        copy_data_count = ul_data_count;
    }
    else
    {
        ALOGE("%s(), ul_data_count(%u) > free_space(%u)", __FUNCTION__, ul_data_count, free_space);
        copy_data_count = free_space;
    }

    // copy data from modem share buffer to internal input buffer
    if (copy_data_count > 0)
    {
        SLOGV("%s(), copy_data_count: %u", __FUNCTION__, copy_data_count);
        RingBuf_copyFromRingBuf(&mRingBuf, &ul_ring_buf, copy_data_count);
    }

    // signal
    mExitCond.signal(); // wake up thread to fwrite data.
    mMutex.unlock();

    return copy_data_count;
}

void *SpeechVMRecorder::DumpVMRecordDataThread(void *arg)
{
    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    SpeechVMRecorder *pSpeechVMRecorder = (SpeechVMRecorder *)arg;
    RingBuf &ring_buf = pSpeechVMRecorder->mRingBuf;

//HTC_AUD_START
    // open file
    if ((pSpeechVMRecorder->mAutoVM & VM_RECORD_VM_MASK) && !pSpeechVMRecorder->mDumpFile
            && pSpeechVMRecorder->OpenFile() != NO_ERROR)
//HTC_AUD_END
    {
        pSpeechVMRecorder->mEnable = false;
        pthread_exit(NULL);
        return 0;
    }

    // open modem record function
    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    status_t retval = pSpeechDriver->VoiceMemoRecordOn();
    if (retval != NO_ERROR)
    {
        ALOGE("%s(), VoiceMemoRecordOn() fail!! Return.", __FUNCTION__);
        pSpeechDriver->VoiceMemoRecordOff();        
        pSpeechVMRecorder->mEnable = false;
        pthread_exit(NULL);
        return 0;
    }

    // Internal Input Buffer Initialization
    pSpeechVMRecorder->mRingBuf.pBufBase = new char[kReadBufferSize];
    pSpeechVMRecorder->mRingBuf.bufLen   = kReadBufferSize;
    pSpeechVMRecorder->mRingBuf.pRead    = pSpeechVMRecorder->mRingBuf.pBufBase;
    pSpeechVMRecorder->mRingBuf.pWrite   = pSpeechVMRecorder->mRingBuf.pBufBase;

    ASSERT(pSpeechVMRecorder->mRingBuf.pBufBase != NULL);
    memset(pSpeechVMRecorder->mRingBuf.pBufBase, 0, pSpeechVMRecorder->mRingBuf.bufLen);

//HTC_AUD_START
    if (pSpeechVMRecorder->mEnableDq && RAM)
    {
        memset(RAM, 0x0, RAM_SIZE);
        memset(pp_info, 0x0, sizeof(struct pingpong_offset_info_type)*RAM_SIZE/pp_size);
    }
//HTC_AUD_END

    pSpeechVMRecorder->mStarting = true;

    while (1)
    {
        // lock & wait data
        pSpeechVMRecorder->mMutex.lock();
        if (pSpeechVMRecorder->mExitCond.waitRelative(pSpeechVMRecorder->mMutex, milliseconds(kCondWaitTimeoutMsec)) != NO_ERROR)
        {
            ALOGW("%s(), waitRelative fail", __FUNCTION__);
        }

//HTC_AUD_START
        // To open legacy file when enabling VM at recording.
        if ((pSpeechVMRecorder->mAutoVM & VM_RECORD_VM_MASK) && !pSpeechVMRecorder->mDumpFile)
        {
            // open file
            if (pSpeechVMRecorder->OpenFile() != NO_ERROR)
            {
                pSpeechVMRecorder->mEnable = false;
                pthread_exit(NULL);
                return 0;
            }
        }
        // To allocate DQ RAM when enabling DQ at recording.
        if (pSpeechVMRecorder->mEnableDq && !RAM)
            dq_allocate_mem();
//HTC_AUD_END

        // make sure VM is still recording after conditional wait
        if (pSpeechVMRecorder->mEnable == false)
        {

            // close file
            if (pSpeechVMRecorder->mDumpFile != NULL)
            {
                fflush(pSpeechVMRecorder->mDumpFile);
                fclose(pSpeechVMRecorder->mDumpFile);
                pSpeechVMRecorder->mDumpFile = NULL;
            }

            // release local ring buffer
            if (pSpeechVMRecorder->mRingBuf.pBufBase != NULL)
            {
                delete []pSpeechVMRecorder->mRingBuf.pBufBase;
                pSpeechVMRecorder->mRingBuf.pBufBase = NULL;
                pSpeechVMRecorder->mRingBuf.pRead    = NULL;
                pSpeechVMRecorder->mRingBuf.pWrite   = NULL;
                pSpeechVMRecorder->mRingBuf.bufLen   = 0;
            }

            ALOGD("%s(), pid: %d, tid: %d, mEnable == false, break.", __FUNCTION__, getpid(), gettid());
            pSpeechVMRecorder->mMutex.unlock();
            break;
        }

        // write data to sd card
        const uint16_t data_count = RingBuf_getDataCount(&ring_buf);
        uint16_t write_bytes = 0;
//HTC_AUD_START
        // write data to RAM (for DQ)
        uint16_t write_bytes_ram = 0;
//HTC_AUD_END

        if (data_count > 0)
        {
            const char *end = ring_buf.pBufBase + ring_buf.bufLen;
            if (ring_buf.pRead <= ring_buf.pWrite)
            {
//HTC_AUD_START
                if(pSpeechVMRecorder->mAutoVM & VM_RECORD_VM_MASK)
//HTC_AUD_END
                    write_bytes += fwrite((void *)ring_buf.pRead, sizeof(char), data_count, pSpeechVMRecorder->mDumpFile);
//HTC_AUD_START
                if (pSpeechVMRecorder->mEnableDq && !log_dumping)
                    write_bytes_ram += copyToRAM((unsigned char *)ring_buf.pRead, data_count);
//HTC_AUD_END
            }
            else
            {
                int r2e = end - ring_buf.pRead;
//HTC_AUD_START
                if((pSpeechVMRecorder->mAutoVM & VM_RECORD_VM_MASK))
                {
//HTC_AUD_END
                    write_bytes += fwrite((void *)ring_buf.pRead, sizeof(char), r2e, pSpeechVMRecorder->mDumpFile);
                    write_bytes += fwrite((void *)ring_buf.pBufBase, sizeof(char), data_count - r2e, pSpeechVMRecorder->mDumpFile);
//HTC_AUD_START
                }
                if (pSpeechVMRecorder->mEnableDq && !log_dumping)
                {
                    write_bytes_ram += copyToRAM((unsigned char *)ring_buf.pRead, r2e);
                    write_bytes_ram += copyToRAM((unsigned char *)ring_buf.pBufBase, data_count - r2e);
                }
//HTC_AUD_END
            }
//HTC_AUD_START
            if(write_bytes < write_bytes_ram)
                write_bytes = write_bytes_ram;
//HTC_AUD_END
            ring_buf.pRead += write_bytes;
            if (ring_buf.pRead >= end) { ring_buf.pRead -= ring_buf.bufLen; }

            SLOGV("data_count: %u, write_bytes: %u", data_count, write_bytes);
        }

        if (write_bytes != data_count)
        {
            ALOGE("%s(), write_bytes(%d) != data_count(%d), SD Card might be full!!", __FUNCTION__, write_bytes, data_count);
        }

        // unlock
        pSpeechVMRecorder->mMutex.unlock();
    }

//HTC_AUD_START
    if (RAM && !pSpeechVMRecorder->mEnableDq) //we don't need DQ anymore
        dq_free_mem();
//HTC_AUD_END

    pthread_exit(NULL);
    return 0;
}


status_t SpeechVMRecorder::Close()
{
    mMutex.lock();

    ALOGD("+%s()", __FUNCTION__);

    if (mEnable == false)
    {
        ALOGW("-%s(), mEnable == false, return!!", __FUNCTION__);
        mMutex.unlock();
        return INVALID_OPERATION;
    }

    mStarting = false;

    int ret = 0;


    // VM log recycle mechanism
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_VM_RECYCLE_ON, property_value, "1"); //"1": default on
    const bool vm_recycle_on = (property_value[0] == '0') ? false : true;

    if (vm_recycle_on == true)
    {
        // Get gVMFileList, gNumOfVMFiles, gTotalSizeOfVMFiles
        memset(gVMFileList, 0, sizeof(gVMFileList));
        gNumOfVMFiles = 0;
        gTotalSizeOfVMFiles = 0;

        ret = ftw(kFolderOfVMFile, GetVMFileList, FTW_D);
        ASSERT(ret == 0);

        // Sort file name
        qsort(gVMFileList, gNumOfVMFiles, sizeof(vm_file_info_t), CompareVMFileName);
        //for(int i = 0; i < gNumOfVMFiles; i++)
        //    ALOGD("%s(), %s, %u", __FUNCTION__, gVMFileList[i].path, gVMFileList[i].size);

        // Remove VM files
        uint32_t index_vm_file_list = 0;
        while (gNumOfVMFiles > kMinKeepNumOfVMFiles && gTotalSizeOfVMFiles > kMaxSizeOfAllVMFiles)
        {
            ALOGD("%s(), gNumOfVMFiles = %lu, gTotalSizeOfVMFiles = %lu", __FUNCTION__, gNumOfVMFiles, gTotalSizeOfVMFiles);

            ALOGD("%s(), remove(%s), size = %lu", __FUNCTION__, gVMFileList[index_vm_file_list].path, gVMFileList[index_vm_file_list].size);
            ret = remove(gVMFileList[index_vm_file_list].path);
            ASSERT(ret == 0);

            gNumOfVMFiles--;
            gTotalSizeOfVMFiles -= gVMFileList[index_vm_file_list].size;

            index_vm_file_list++;
        }
    }


    // release wake lock
    ret = release_wake_lock(VM_RECORD_WAKELOCK_NAME);
    ALOGD("%s(), release_wake_lock:%s return %d.", __FUNCTION__, VM_RECORD_WAKELOCK_NAME, ret);

    // turn off record function
    SpeechDriverFactory::GetInstance()->GetSpeechDriver()->VoiceMemoRecordOff();
    
    mEnable = false;
    mMutex.unlock();
    mExitCond.signal(); // wake up thread to exit
    
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void SpeechVMRecorder::SetVMRecordCapability(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB)
{
    ALOGD("%s(), uAutoVM = 0x%x, debug_info[0] = %u, speech_common_para[0] = %u", __FUNCTION__,
          pSphParamNB->uAutoVM, pSphParamNB->debug_info[0], pSphParamNB->speech_common_para[0]);

    mAutoVM = pSphParamNB->uAutoVM;

    SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    const bool speech_on = pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK);
    const bool rec_on    = pSpeechDriver->GetApSideModemStatus(RECORD_STATUS_MASK);


    if (GetVMRecordCapability() == true && GetVMRecordStatus() == false && speech_on == true)
    {
        // turn off normal phone record
        if (rec_on == true)
        {
            ALOGW("%s(), Turn off normal phone recording!!", __FUNCTION__);
            ALOGW("%s(), The following record file will be silence until VM/EPL is closed.", __FUNCTION__);
        }

        ALOGD("%s(), Open VM/EPL record", __FUNCTION__);
        Open();
    }
    else if (GetVMRecordCapability() == false && GetVMRecordStatus() == true)
    {
        ALOGD("%s(), Close VM/EPL record", __FUNCTION__);
        ALOGD("%s(), Able to continue to do phone record.", __FUNCTION__);
        Close();
    }

}

//HTC_AUD_START
int SetDqParam(int value)
{
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", value);
    return property_set(PROPERTY_KEY_DQ_ENABLE, property_value);
}

char *GetDqParam()
{
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_DQ_ENABLE, property_value, "0"); //default off
    return property_value;
}

status_t SpeechVMRecorder::OpenDq(int iEnalbe)
{
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    ALOGD("+%s(%d)", __FUNCTION__,iEnalbe);

    mMutex.lock();

    if (iEnalbe && !RAM)
        dq_allocate_mem();
    else if (!iEnalbe && !mStarting && RAM)
        dq_free_mem();

    if (SetDqParam(iEnalbe) != NO_ERROR)
    {
        mMutex.unlock();
        ALOGE("-%s(%d) failed!", __FUNCTION__,iEnalbe);
        return UNKNOWN_ERROR;
    }
    GetNBSpeechParamFromNVRam(&eSphParamNB);
    SetVMRecordCapability(&eSphParamNB);

    mMutex.unlock();
    ALOGD("-%s(%d)", __FUNCTION__,iEnalbe);
    return NO_ERROR;
}
//HTC_AUD_END

bool SpeechVMRecorder::GetVMRecordCapability() const
{
#if defined(FORCE_ENABLE_VM)
    return true;
#else
//HTC_AUD_START
#if 1
    if (atoi(GetDqParam()) == 0)
        mEnableDq = false;
    else
        mEnableDq = true;

    return (mEnableDq || (mAutoVM & VM_RECORD_VM_MASK) > 0);
#else
//HTC_AUD_END
    return ((mAutoVM & VM_RECORD_VM_MASK) > 0);
//HTC_AUD_START
#endif
//HTC_AUD_END
#endif
}

bool SpeechVMRecorder::GetVMRecordCapabilityForCTM4Way() const
{
    bool retval = false;

    if ((mAutoVM & VM_RECORD_VM_MASK) > 0) // cannot support VM and CTM4way record at the same time
    {
        retval =  false;
    }
    else if ((mAutoVM & VM_RECORD_CTM4WAY_MASK) > 0)
    {
        retval = true;
    }

    return retval;
}

int SpeechVMRecorder::StartCtmDebug()
{
    ALOGD("%s()", __FUNCTION__);

    if (m_CtmDebug_Started) { return false; }

    const uint8_t kMaxPathLength = 80;
    char ctm_file_path_UlIn[kMaxPathLength];
    char ctm_file_path_DlIn[kMaxPathLength];
    char ctm_file_path_UlOut[kMaxPathLength];
    char ctm_file_path_DlOut[kMaxPathLength];
    memset((void *)ctm_file_path_UlIn, 0, kMaxPathLength);
    memset((void *)ctm_file_path_DlIn, 0, kMaxPathLength);
    memset((void *)ctm_file_path_UlOut, 0, kMaxPathLength);
    memset((void *)ctm_file_path_DlOut, 0, kMaxPathLength);
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(ctm_file_path_UlIn, kMaxPathLength, "/sdcard/mtklog/audio_dump/%Y_%m_%d_%H%M%S_CtmUlIn.pcm", timeinfo);
    strftime(ctm_file_path_DlIn, kMaxPathLength, "/sdcard/mtklog/audio_dump/%Y_%m_%d_%H%M%S_CtmDlIn.pcm", timeinfo);
    strftime(ctm_file_path_UlOut, kMaxPathLength, "/sdcard/mtklog/audio_dump/%Y_%m_%d_%H%M%S_CtmUlOut.pcm", timeinfo);
    strftime(ctm_file_path_DlOut, kMaxPathLength, "/sdcard/mtklog/audio_dump/%Y_%m_%d_%H%M%S_CtmDlOut.pcm", timeinfo);
    int ret;
    ret = AudiocheckAndCreateDirectory(ctm_file_path_UlIn);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_UlIn);
        return UNKNOWN_ERROR;
    }
    ret = AudiocheckAndCreateDirectory(ctm_file_path_DlIn);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_DlIn);
        return UNKNOWN_ERROR;
    }
    ret = AudiocheckAndCreateDirectory(ctm_file_path_UlOut);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_UlOut);
        return UNKNOWN_ERROR;
    }
    ret = AudiocheckAndCreateDirectory(ctm_file_path_DlOut);
    if (ret < 0)
    {
        ALOGE("%s(), AudiocheckAndCreateDirectory(%s) fail!!", __FUNCTION__, ctm_file_path_DlOut);
        return UNKNOWN_ERROR;
    }
    pCtmDumpFileUlIn = fopen(ctm_file_path_UlIn, "wb");
    pCtmDumpFileDlIn = fopen(ctm_file_path_DlIn, "wb");
    pCtmDumpFileUlOut = fopen(ctm_file_path_UlOut, "wb");
    pCtmDumpFileDlOut = fopen(ctm_file_path_DlOut, "wb");

    if (pCtmDumpFileUlIn == NULL) { ALOGW("Fail to Open pCtmDumpFileUlIn"); }
    if (pCtmDumpFileDlIn == NULL) { ALOGW("Fail to Open pCtmDumpFileDlIn"); }
    if (pCtmDumpFileUlOut == NULL) { ALOGW("Fail to Open pCtmDumpFileUlOut"); }
    if (pCtmDumpFileDlOut == NULL) { ALOGW("Fail to Open pCtmDumpFileDlOut"); }

    m_CtmDebug_Started = true;

    return true;
}

int SpeechVMRecorder::StopCtmDebug()
{
    ALOGD("%s()", __FUNCTION__);

    if (!m_CtmDebug_Started) { return false; }

    m_CtmDebug_Started = false;

    fclose(pCtmDumpFileUlIn);
    fclose(pCtmDumpFileDlIn);
    fclose(pCtmDumpFileUlOut);
    fclose(pCtmDumpFileDlOut);
    return true;
}
void SpeechVMRecorder::GetCtmDebugDataFromModem(RingBuf ul_ring_buf, FILE *pFile)
{
    int InpBuf_freeSpace = 0;
    int ShareBuf_dataCnt = 0;

    if (m_CtmDebug_Started == false)
    {
        ALOGD("GetCtmDebugDataFromModem, m_CtmDebug_Started=false");
        return;
    }

    // get data count in share buffer
    ShareBuf_dataCnt = RingBuf_getDataCount(&ul_ring_buf);

    char linear_buffer[ShareBuf_dataCnt];
    char *pM2AShareBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
    if (ul_ring_buf.pRead + ShareBuf_dataCnt <= pM2AShareBufEnd)
    {
        memcpy(linear_buffer, ul_ring_buf.pRead, ShareBuf_dataCnt);
    }
    else
    {
        uint32_t r2e = pM2AShareBufEnd - ul_ring_buf.pRead;
        memcpy(linear_buffer, ul_ring_buf.pRead, r2e);
        memcpy((void *)(linear_buffer + r2e), ul_ring_buf.pBufBase, ShareBuf_dataCnt - r2e);
    }

    fwrite(linear_buffer, sizeof(char), ShareBuf_dataCnt, pFile);

}

};
