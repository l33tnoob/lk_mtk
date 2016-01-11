#include "MD5Checksum.h"
#include "CheckSum_Generate.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "brom.h"

/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}

/*
 * constructor
 */
CMD5Checksum::CMD5Checksum()
{
    memset(PADDING, 0x0, sizeof(PADDING));
    PADDING[0] = 0x80;

    MD5_Reset();
}
CMD5Checksum::CMD5Checksum(bool debug_on)
{
    m_debug_on = debug_on;

    memset(PADDING, 0x0, sizeof(PADDING));
    PADDING[0] = 0x80;

    MD5_Reset();
}
CMD5Checksum::~CMD5Checksum()
{
    DeInit();
}


/*
 * public
 */
int CMD5Checksum::Init(const std::string &scatterfile)
{
    if( m_debug_on )
    {
        Brom_DebugClear();
        Brom_DebugOn();
    }

    int ret = 0;
    m_scatterfile = scatterfile;
    
    if( m_scatterfile.empty() )
        return -1;

    m_checksumInfo.p_roms_mem_chksum_info = (RomMemChecksumInfo*)malloc(sizeof(RomMemChecksumInfo) * MAX_LOAD_SECTIONS);
    if( !m_checksumInfo.p_roms_mem_chksum_info )
    {
        return -2;
    }
    memset(m_checksumInfo.p_roms_mem_chksum_info, 0, sizeof(RomMemChecksumInfo) * MAX_LOAD_SECTIONS);

    ret = DL_Create(&m_dl_handle);

    return ret;
    
}
int CMD5Checksum::DeInit()
{
    if( m_dl_handle )
    {
        DL_Destroy(&m_dl_handle);
        m_dl_handle = NULL;
    }
    if( m_checksumInfo.p_roms_mem_chksum_info )
    {
        free(m_checksumInfo.p_roms_mem_chksum_info);
    }
    if( m_debug_on )
    {
        Brom_DebugOff();
    }
    return 0;
}
int CMD5Checksum::DoChecksum()
{
    int ret = 0;
    if( !m_dl_handle )
        return -1;
/*
    RomMemChecksumInfoArg arg;
    int stopFlag    = 0;

    memset(&arg, 0, sizeof(RomMemChecksumInfoArg));
    arg.calc_type   = CUST;
    arg.p_stopflag  = &stopFlag;
    arg.m_cb_rom_mem_checksum_init          = CBInit;
    arg.m_cb_rom_mem_checksum               = CBProgress;
    arg.m_cb_rom_mem_checksum_init_arg      = NULL;
    arg.m_cb_rom_mem_checksum_arg           = NULL;
    arg.cb_rom_mem_checksum_cust_arg        = this;
    arg.cb_rom_mem_checksum_cust_update     = CBMD5Update;
    arg.cb_rom_mem_checksum_cust_finalize   = CBMD5Final;
    
    arg.scatter_filepath    = m_scatterfile.c_str();

    ret = DL_CalculateROMsMemBuf(m_dl_handle, &arg, &m_checksumInfo);
*/
    if( (ret = DL_LoadScatter(m_dl_handle, m_scatterfile.c_str(), NULL)) != S_DONE )
    {
        return ret;
    }

    if( (ret = DL_AutoLoadRomImages(m_dl_handle, m_scatterfile.c_str())) != S_DONE )
    {
        return ret;
    }

    unsigned short rom_count;
    if( (ret = DL_GetCount(m_dl_handle, &rom_count)) != S_DONE )
    {
        return ret;
    }
    
    if( (ret = DL_Rom_GetInfoAll(m_dl_handle, m_rom, MAX_LOAD_SECTIONS)) != S_DONE )
    {
        return ret;
    }

    int i, j;
    m_checksumInfo.roms_mem_chksum_info_count = 0;
    for(i = 0, j = 0; i < rom_count; i ++)
    {
        if( m_rom[i].enable == false )
        {
            continue;
        }
        m_checksumInfo.p_roms_mem_chksum_info[j].name = m_rom[i].name;
        m_checksumInfo.p_roms_mem_chksum_info[j].filepath = m_rom[i].filepath;

        CBInit(NULL, m_rom[i].name);
        //ret = MD5_File(m_rom[i].filepath, m_checksumInfo.p_roms_mem_chksum_info[j].chksum_str);
        ret = MD5_File_Ex(m_rom[i].filepath, m_checksumInfo.p_roms_mem_chksum_info[j].chksum_str, m_rom[i].filesize);
        if( ret )
        {
            return ret;
        }

        m_checksumInfo.roms_mem_chksum_info_count ++;
        j ++;
    }

    return ret;
}
int CMD5Checksum::WriteChecksumIni(const std::string &ini_path)
{
    char str[256];
    m_ini.SetIniPath(ini_path.c_str());
    //m_ini.Clear();
    
    m_ini.Write(VERSION_SECTION, VERSION_KEYWORD, "V2"); //MD5 use V2
    m_ini.Write(VERSION_SECTION, TOOL_VER_KEYWORD, TOOL_VERSION);

    sprintf(str, "%u", (unsigned int)time(NULL));
    m_ini.Write(VERSION_SECTION, CHANGE_ID, str);
    m_ini.Write(ENABLE_SECTION, ENABLE_ITEM, "1");

    RomMemChecksumInfo *p_roms = m_checksumInfo.p_roms_mem_chksum_info;
    for( int i = 0; i < m_checksumInfo.roms_mem_chksum_info_count; i++ )
    {
        m_ini.Write(CHECKSUM_SECTION_V2, p_roms[i].name, p_roms[i].chksum_str);
    }
    return 0;
}

/*
 * protected
 */
int __stdcall CMD5Checksum::CBMD5Update(const unsigned char* buf, unsigned int buf_len, unsigned short index, void *usr_arg)
{
    ((CMD5Checksum*)usr_arg)->MD5_Update(buf, buf_len);
    return 0;
}
int __stdcall CMD5Checksum::CBMD5Final(unsigned short index, char *p_str, void *usr_arg)
{
    ((CMD5Checksum*)usr_arg)->MD5_Final(p_str);
    return 0;
}

void CMD5Checksum::MD5_Reset()
{
    /* Magic */
    state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;

    /* count */
    count[0] = count[1] = 0;

    /* clean temp buffer*/
    memset(temp_buf, 0x0, sizeof(temp_buf));
    buf_size = 0;
}
void CMD5Checksum::MD5_Core(const unsigned char block[64])
{
    unsigned int a = state[0], b = state[1], c = state[2], d = state[3], *x;
    x = (unsigned int*)block;

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1  */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2  */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3  */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4  */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5  */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6  */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7  */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8  */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9  */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}
void CMD5Checksum::MD5_GetResult(char *out_buffer)
{
    if( !out_buffer ) return;
    sprintf(out_buffer, "%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X", 
        *(unsigned char*)(state), 
        *((unsigned char*)(state) + 1), 
        *((unsigned char*)(state) + 2), 
        *((unsigned char*)(state) + 3), 
        *((unsigned char*)(state) + 4), 
        *((unsigned char*)(state) + 5), 
        *((unsigned char*)(state) + 6), 
        *((unsigned char*)(state) + 7),
        *((unsigned char*)(state) + 8),
        *((unsigned char*)(state) + 9),
        *((unsigned char*)(state) + 10),
        *((unsigned char*)(state) + 11),
        *((unsigned char*)(state) + 12),
        *((unsigned char*)(state) + 13),
        *((unsigned char*)(state) + 14),
        *((unsigned char*)(state) + 15)
        );
}
int CMD5Checksum::MD5_Update(const unsigned char *in_buffer, const unsigned int size)
{
    if( !in_buffer ) return -1;

    count[0] += size << 3;
    count[1] += size >> 29;
    if( count[0] < (size << 3) ) count[1]++;

    if(size + buf_size < 64)
    {
        memcpy(&temp_buf[buf_size], in_buffer, size);
        buf_size += size;
        return 0;
    }
    else
    {
        memcpy(&temp_buf[buf_size], in_buffer, 64 - buf_size);
    }
    MD5_Core(temp_buf);

    unsigned int i;
    for(i = 64 - buf_size; i + 64 <= size; i += 64)
    {
        MD5_Core(&in_buffer[i]);
    }

    buf_size = size - i;
    memcpy(temp_buf, &in_buffer[i], buf_size);

    return 0;
}
int CMD5Checksum::MD5_Final(char *out_result)
{
    memcpy(&temp_buf[buf_size], PADDING, 64 - buf_size);
    if( buf_size >= 56 )
    {
        MD5_Core(temp_buf);
        memset(temp_buf, 0, sizeof(temp_buf));
    }

    *(unsigned int*)&temp_buf[56] = count[0];
    *(unsigned int*)&temp_buf[60] = count[1];
    MD5_Core(temp_buf);

    if( out_result )
        MD5_GetResult(out_result);
    MD5_Reset();

    return 0;
}
int CMD5Checksum::MD5_File(const char *file_path, char *out_result)
{
    if( !file_path ) return -1;

    FILE *file = fopen(file_path, "rb");
    if( !file ) return -2;

    const int BUFFER_SIZE = 1024 * 1024;
    unsigned char *buffer = (unsigned char*)malloc(BUFFER_SIZE);
    if( !buffer ) return -3;

    unsigned int len = 0;
    while( len = fread(buffer, 1, BUFFER_SIZE, file) )
    {
        MD5_Update(buffer, len);
    }
    MD5_Final(out_result);

    free(buffer);
    fclose(file);
    return 0;
}
int CMD5Checksum::MD5_File_Ex(const char *file_path, char *out_result, const unsigned int file_size)
{
    if( !file_path ) return -1;

    FILE *file = fopen(file_path, "rb");
    if( !file ) return -2;

    const int BUFFER_SIZE = 4 * 1024 * 1024;
    unsigned char *buffer = (unsigned char*)malloc(BUFFER_SIZE);
    if( !buffer ) return -3;

    unsigned int len = 0;
    unsigned int total_read = 0;
    while( len = fread(buffer, 1, BUFFER_SIZE, file) )
    {
        MD5_Update(buffer, len);
        total_read += len;

        if( file_size )
            CBProgress((double)total_read / file_size * 100, total_read, file_size, NULL);
    }
    MD5_Final(out_result);

    free(buffer);
    fclose(file);
    return 0;
}