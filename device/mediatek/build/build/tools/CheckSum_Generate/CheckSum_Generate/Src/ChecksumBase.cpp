#include "ChecksumBase.h"
#include "CheckSum_Generate.h"
#include <iostream>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using std::cout;
using std::endl;

CChecksumBase::CChecksumBase()
{
    m_scatterfile   = "";
    m_dl_handle     = NULL;
    memset(&m_checksumInfo, 0, sizeof(m_checksumInfo));
}
CChecksumBase::~CChecksumBase()
{
}

/*
 * virtual
 */
int CChecksumBase::GetChecksumInfo(RomMemChecksumInfoResult &checksumInfo)
{
    memcpy(&checksumInfo, &m_checksumInfo, sizeof(RomMemChecksumInfoResult));
    return 0;
}
int CChecksumBase::WriteChecksumIni(const std::string &ini_path)
{
    char str[256];
    m_ini.SetIniPath(ini_path.c_str());
    //m_ini.Clear();
    
    m_ini.Write(VERSION_SECTION, VERSION_KEYWORD, "V1");
    m_ini.Write(VERSION_SECTION, TOOL_VER_KEYWORD, TOOL_VERSION);

    sprintf(str, "%u", (unsigned int)time(NULL));
    m_ini.Write(VERSION_SECTION, CHANGE_ID, str);
    m_ini.Write(ENABLE_SECTION, ENABLE_ITEM, "1");

    RomMemChecksumInfo *p_roms = m_checksumInfo.p_roms_mem_chksum_info;
    for( int i = 0; i < m_checksumInfo.roms_mem_chksum_info_count; i++ )
    {
        m_ini.Write(CHECKSUM_SECTION, p_roms[i].name, p_roms[i].chksum_str);
    }
    return 0;
}

/*
 * protected
 */
int __stdcall CChecksumBase::CBInit(void *usr_arg, const char* image_name)
{
    if( image_name )
        cout << endl << "ROM: " << image_name << endl;
    return 0;
}
int __stdcall CChecksumBase::CBProgress(unsigned char finished_percentage, unsigned int finished_bytes, unsigned int total_bytes, void *usr_arg)
{
    cout << "   finished:" << static_cast<unsigned short>(finished_percentage) << "%"
         << "    " << finished_bytes << "/" << total_bytes << endl;
    return 0;
}
