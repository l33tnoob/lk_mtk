#include "ByteChecksum.h"
#include "CheckSum_Generate.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "brom.h"

/*
 * constructor
 */
CByteChecksum::CByteChecksum()
{
    m_dl_handle = NULL;
}
CByteChecksum::CByteChecksum(bool debug_on)
{
    m_debug_on  = debug_on;
    m_dl_handle = NULL;
}
CByteChecksum::~CByteChecksum()
{
    DeInit();
}

/*
 * public
 */
int CByteChecksum::Init(const std::string &scatterfile)
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

    ret = DL_Create(&m_dl_handle);

    return ret;
}
int CByteChecksum::DeInit()
{
    if( m_dl_handle )
    {
        DL_Destroy(&m_dl_handle);
        m_dl_handle = NULL;
    }
    if( m_debug_on )
    {
        Brom_DebugOff();
    }
    return 0;
}
int CByteChecksum::DoChecksum()
{
    int ret = 0;
    if( !m_dl_handle )
        return -1;

    RomMemChecksumInfoArg arg;
    int stopFlag    = 0;

    memset(&arg, 0, sizeof(RomMemChecksumInfoArg));
    arg.calc_type   = ByteSum;
    arg.p_stopflag  = &stopFlag;
    arg.m_cb_rom_mem_checksum_init  = CBInit;
    arg.m_cb_rom_mem_checksum       = CBProgress;
    arg.m_cb_rom_mem_checksum_init_arg = NULL;
    arg.m_cb_rom_mem_checksum_arg   = NULL;
    
    arg.scatter_filepath    = m_scatterfile.c_str();

    ret = DL_CalculateROMsMemBuf(m_dl_handle, &arg, &m_checksumInfo);
    return ret;
}
int CByteChecksum::WriteChecksumIni(const std::string &ini_path)
{
    char str[256];
    m_ini.SetIniPath(ini_path.c_str());
    m_ini.Clear();
    
    m_ini.Write(VERSION_SECTION, VERSION_KEYWORD, "V1");
    m_ini.Write(VERSION_SECTION, TOOL_VER_KEYWORD, TOOL_VERSION);

    sprintf(str, "%u", (unsigned int)time(NULL));
    m_ini.Write(VERSION_SECTION, CHANGE_ID, str);
    m_ini.Write(ENABLE_SECTION, ENABLE_ITEM, "1");

    RomMemChecksumInfo *p_roms = m_checksumInfo.p_roms_mem_chksum_info;
    for( int i = 0; i < m_checksumInfo.roms_mem_chksum_info_count; i++ )
    {
        sprintf(str, "0x%04X", p_roms[i].chksum);
        m_ini.Write(CHECKSUM_SECTION, p_roms[i].name, str);
    }
    return 0;
}
