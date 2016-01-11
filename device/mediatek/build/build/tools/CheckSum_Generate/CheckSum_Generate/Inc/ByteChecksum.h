#ifndef _BYTE_CHECKSUM_H_
#define _BYTE_CHECKSUM_H_

#include "ChecksumBase.h"

class CByteChecksum: public CChecksumBase
{
public:
    CByteChecksum();
    CByteChecksum(bool debug_on);
    ~CByteChecksum();

    int Init(const std::string &scatterfile);
    int DoChecksum();
    int DeInit();
    int WriteChecksumIni(const std::string &ini_path);
};

#endif //_BYTE_CHECKSUM_H_