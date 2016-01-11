#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "CheckSum_Generate.h"
#include "ChecksumBase.h"
#include "ByteChecksum.h"
#include "MD5Checksum.h"
#include "brom.h"

#ifdef WIN32
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

using std::cout;
using std::endl;
using std::string;

/*
 * global
 */
string      g_scatterfile_path;
string      g_ini_path;


int     FindScatterFile(const string &path, string &out);
void    DisplayResult(const RomMemChecksumInfoResult &checksumInfo);
int     GenChecksumIni(CChecksumBase *pChecksumGen);

/*
 * main
 */
int main(int argc, char* argv[], char* envp[])
{
    cout << endl;
    cout << "+++++++++++++++++++++++++" << endl;
    cout << "Tool version: " << TOOL_VERSION << endl;
    cout << "+++++++++++++++++++++++++" << endl;
    cout << endl;
    cout << endl;

    int nRetCode = 0;

    g_scatterfile_path  = ".";
    g_ini_path          = ".";

    bool do_md5_cksm    = false;
    bool debug_on       = false;
    /*
     * parse cmd line
     */
    for(int i = 1; i < argc; i++)
    {
        if( !strcmp(argv[i], "-s") || !strcmp(argv[i], "-S") )
        {
            i ++;
            g_scatterfile_path = argv[i];
        }
        else if( !strcmp(argv[i], "-d") || !strcmp(argv[i], "-D") )
        {
            i ++;
            g_ini_path = argv[i];
        }
        else if( !strcmp(argv[i], "-md5") || !strcmp(argv[i], "-MD5") )
        {
            do_md5_cksm = true;
        }
        else if( !strcmp(argv[i], "-dbg") || !strcmp(argv[i], "-DBG") )
        {
            debug_on = true;
        }
        else if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "-H") )
        {
            cout << "CheckSum_Gen.exe [-s] [-d] [-md5]" << endl;
            cout << "   [-s]     Scatterfile path" << endl;
            cout << "   [-d]     Checksum.ini file path" << endl;
            cout << "   [-md5]   Use md5 algorithm" << endl;
            cout << "   [-dbg]   Turn on debug log" << endl;
            
            return 0;
        }
    }

    /*
     * init global file path
     */
    if( g_scatterfile_path.empty() || (string::npos == g_scatterfile_path.find("scatter")) )
    {
        nRetCode = FindScatterFile(g_scatterfile_path, g_scatterfile_path);
        if( nRetCode )
        {
            cout << "Cannot find the scatterfile!" << endl;
            goto Ret;
        }
    }
    cout << "Scatterfile: " << g_scatterfile_path << endl << endl;

    if( g_ini_path.empty() || (string::npos == g_ini_path.find(".ini")))
    {
        g_ini_path = g_ini_path + PATH_PADDING + CHECKSUM_FILE;
    }


    /*
     * gen checksum ini
     */

    // 1. always do byte cksm for DA use
    GenChecksumIni(new CByteChecksum(debug_on));    

    // 2. other
    if( do_md5_cksm )
    {
        cout << endl << endl;
        cout << "MD5 is on going, maybe take few minutes...";
        cout << endl << endl;

        GenChecksumIni(new CMD5Checksum(debug_on));
    }

Ret:
    system("pause");
    return 0;
}





/*
 *
 */
int FindScatterFile(const string &path, string &out)
{
    string search_path, file_path;
    search_path = path + PATH_PADDING + "*scatter*.txt";

#ifdef WIN32 /* windows */
    WIN32_FIND_DATA find_data;
    HANDLE hFind;

    hFind = FindFirstFile((const char*)search_path.c_str(), &find_data);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
		return -1;
    }
    do
    {
        if( !((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) )
        {
            out = path + PATH_PADDING + find_data.cFileName;
            FindClose(hFind);
            return 0;
        }
    } while( FindNextFile(hFind, &find_data) );
    FindClose(hFind);

#else /* linux */
    struct dirent *ent = NULL;
    DIR *pDir = NULL;
    char dir[512];
    struct stat statbuf;

    if( (pDir = opendir((const char*) path.c_str())) == NULL)
    {
        return -1;
    }
    while( (ent = readdir(pDir)) != NULL)
    {
        file_path = path + PATH_PADDING + ent->d_name;
        lstat(file_path.c_str(), &statbuf);

        if( !S_ISDIR(statbuf.st_mode) && (string::npos != file_path.find("scatter")) )
        {
            out = file_path;
            closedir(pDir);
            return 0;
        }
    }
    closedir(pDir);

#endif

    return -1;
}

void DisplayResult(const RomMemChecksumInfoResult &checksumInfo)
{
    cout << endl << endl;
    cout << "Total roms: " << checksumInfo.roms_mem_chksum_info_count << endl << endl;

    RomMemChecksumInfo *p_roms = checksumInfo.p_roms_mem_chksum_info;

    for( int i = 0; i < checksumInfo.roms_mem_chksum_info_count; i++ )
    {
        cout << p_roms[i].name << "\t\t= " << p_roms[i].chksum_str << endl;
    }    
}

int GenChecksumIni(CChecksumBase *pChecksumGen)
{
    /*
     * do checksum
     */
    int nRetCode = -1;
    RomMemChecksumInfoResult result;

    if( !pChecksumGen )
    {
        cout << "pChecksumGen is NULL" << endl;
        goto Ret;
    }

    nRetCode = pChecksumGen->Init(g_scatterfile_path);
    if( nRetCode )
    {
        cout << "Checksum Init fail(" << nRetCode << ")!" << endl;
        goto Ret;
    }
    nRetCode = pChecksumGen->DoChecksum();
    if( nRetCode )
    {
        if( nRetCode > 0 )      // Brom error
            cout << "Checksum DoChecksum fail: " << StatusToString(nRetCode) << "(" << nRetCode << ")!" << endl;
        else                    // Tool error
            cout << "Checksum DoChecksum fail(" << nRetCode << ")!" << endl;
        goto Ret;
    }
    pChecksumGen->GetChecksumInfo(result);

    DisplayResult(result);

    /*
     * write ini
     */
    cout << "Write ini to: " << g_ini_path << endl;
    pChecksumGen->WriteChecksumIni(g_ini_path);

Ret:
    if( pChecksumGen )
    {
        pChecksumGen->DeInit();
    }

    return nRetCode;
}
