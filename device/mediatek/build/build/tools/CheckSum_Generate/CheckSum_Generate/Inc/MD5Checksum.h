#ifndef _MD5_CHECKSUM_H_
#define _MD5_CHECKSUM_H_

#include "ChecksumBase.h"

class CMD5Checksum: public CChecksumBase
{
public:
    CMD5Checksum();
    CMD5Checksum(bool debug_on);
    ~CMD5Checksum();

    int Init(const std::string &scatterfile);
    int DoChecksum();
    int DeInit();
    int WriteChecksumIni(const std::string &ini_path);

protected:
    static int __stdcall CBMD5Final(unsigned short index, char *p_str, void *usr_arg);
    static int __stdcall CBMD5Update(const unsigned char* buf, unsigned int buf_len, unsigned short index, void *usr_arg);

protected:
    unsigned int	state[4];
    unsigned int    count[2];
    unsigned char   PADDING[64];
    unsigned char   temp_buf[64];
    unsigned int    buf_size;

//public:
    void    MD5_Reset();
    int     MD5_Update(const unsigned char *in_buffer, const unsigned int size);
    int     MD5_Final(char *out_result);
    void    MD5_Core(const unsigned char block[64]);
    void    MD5_GetResult(char *out_buffer);
    int     MD5_File(const char *file_path, char *out_result);
    int     MD5_File_Ex(const char *file_path, char *out_result, const unsigned int file_size);

private:
    ROM_INFO m_rom[MAX_LOAD_SECTIONS];
};

#endif //_MD5_CHECKSUM_H_