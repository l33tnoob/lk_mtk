#ifndef _CHECKSUM_BASE_H_
#define _CHECKSUM_BASE_H_

#include <string>
#include "flashtool_handle.h"
#include "CIni.h"


class CChecksumBase
{
public:
    CChecksumBase();
    ~CChecksumBase();

    // pure
    virtual int Init(const std::string &scatterfile) = 0;
    virtual int DoChecksum() = 0;
    virtual int DeInit() = 0;
    //
    virtual int GetChecksumInfo(RomMemChecksumInfoResult &checksumInfo);
    virtual int WriteChecksumIni(const std::string &ini_path);

protected:
    std::string                 m_scatterfile;
    DL_HANDLE_T                 m_dl_handle;
    RomMemChecksumInfoResult    m_checksumInfo;
    CIni                        m_ini;
    bool                        m_debug_on;

protected:
    static int __stdcall CBInit(void *usr_arg, const char* image_name);
    static int __stdcall CBProgress(unsigned char finished_percentage, unsigned int finished_bytes, unsigned int total_bytes, void *usr_arg);
};

#endif //_CHECKSUM_BASE_H_
