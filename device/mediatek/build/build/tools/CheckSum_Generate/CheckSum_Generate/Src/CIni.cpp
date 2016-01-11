#include "CIni.h"

//#undef WIN32

#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define LINE_LEN 1024
#endif

using namespace std;

/*
 * constructor
 */
CIni::CIni()
{
    m_ini_path = "";
}
CIni::CIni(const char *path)
{
    if( path )
        m_ini_path = path;
    else
        m_ini_path = "";
}
CIni::~CIni()
{
    m_ini_path = "";
}

/*
 * public
 */
void CIni::SetIniPath(const char *path)
{
    if( path )
        m_ini_path = path;
    else
        m_ini_path = "";
}

void CIni::Clear()
{
    FILE *file = fopen(m_ini_path.c_str(), "w");
    if( file )
    {
        fclose(file);
    }
}

int CIni::Write (const char* section,const char* lpKeyName, const char* lpString)
{
#ifdef WIN32
    return ::WritePrivateProfileString (section, lpKeyName, lpString, (const char*)m_ini_path.c_str());
#else
    return WriteProfileString ((const char*)m_ini_path.c_str(), section, lpKeyName, lpString);
#endif
}

int CIni::Read (const char* section, const char* lpKeyName, const char* lpDefault,
		                        char* lpReturnedString, unsigned short nSize)
{
#ifdef WIN32
	return ::GetPrivateProfileString (section, lpKeyName, NULL, lpReturnedString, nSize, (const char*)m_ini_path.c_str());
#else
    return ReadProfileString((const char*)m_ini_path.c_str(), section, lpKeyName, NULL, lpReturnedString, nSize);
#endif
}


/*
 * private
 */

#ifndef WIN32

int CIni::WriteProfileString (const char* path, const char* section,
                                    const char* lpKeyName, const char* lpString)
{
    int retcode = 0;
    char line[LINE_LEN];
    char temp_section[64];
    bool find_section = false,
         find_key = false;
    bool next_section = false;
    char *writeBuf, *readBuf;
    unsigned int file_size = 0;
    //unsigned int write_size = 0;

    FILE *file = fopen(path, "r");
    if(file == NULL)
    {
        // create it
        char temp_buf[LINE_LEN];
        file = fopen(path, "w");
        if(file == NULL) return -1;

        sprintf(temp_section, "[%s]\n", section);
        sprintf(line, "%s = %s\n", lpKeyName, lpString);
        sprintf(temp_buf, "%s%s", temp_section, line);
        fwrite(temp_buf, 1, strlen(temp_buf), file);
        fclose(file);

        return 0;
    }

    fseek(file, 0, SEEK_END);
	file_size = ftell(file);
    fseek(file, 0, 0);

    file_size += 1024 * sizeof(char);   // 1k maybe enough for add new items


    // Profile always be small, maybe we needn't use temp file
    writeBuf = (char *)malloc(file_size);
    readBuf = (char *)malloc(1024 * sizeof(char));
    if(writeBuf == NULL || readBuf == NULL)
    {
        retcode = -2;
        goto errorret;
    }

    //memset(writeBuf, 0, 1024 * 1024 * sizeof(char));
    memset(writeBuf, 0, file_size);
    memset(readBuf, 0, 1024 * sizeof(char));

    // 1. Find section
    fseek(file, 0, 0);
    sprintf(temp_section, "[%s]", section);
    while( fgets(line, LINE_LEN, file) )
    {
        sprintf(writeBuf, "%s%s", writeBuf, line);
        if(line[0] == ';')
        {
            continue;
        }

        if(strstr(line, temp_section) != NULL)
        {
            find_section = true;
            break;
        }
    }
    if(find_section == false)
    {
        sprintf(writeBuf, "%s%s\n", writeBuf, temp_section);
    }

    // 2. Find keyword
    line[0] = '\0';
    while( fgets(line, LINE_LEN, file) )
    {
        if(line[0] == ';')
        {
            sprintf(writeBuf, "%s%s", writeBuf, line);
            continue;
        }

        // Can't find in this section, add it before the next section.
        // But be careful, the keyname must not have "["
        //if(strstr(line, "[") != NULL)
        if(line[0] == '[')
        {
            next_section = true;
            break;
        }

        //if(strstr(line, lpKeyName) != NULL)
        if( !strncmp(line, lpKeyName, strlen(lpKeyName)) )
        {
            int len = strlen(lpKeyName);
            if(line[len] == ' '
                || line[len] == '\t'
                || line[len] == '\n'
                || line[len] == '=') // check if the whole word matched
            {
                find_key = true;
                break;
            }
        }
        sprintf(writeBuf, "%s%s", writeBuf, line);
    }

    sprintf(writeBuf, "%s%s=%s\n", writeBuf, lpKeyName, lpString);
    if(next_section == true)
    {
        sprintf(writeBuf, "%s%s", writeBuf, line);
    }

    // 3.Read other data
    memset(readBuf, 0, 1024 * sizeof(char));
    while( fread(readBuf, 1, 1024 - 1, file) )
    {
        sprintf(writeBuf, "%s%s", writeBuf, readBuf);
        memset(readBuf, 0, 1024 * sizeof(char));
        //write_size += strlen(readBuf);
    }
    fclose(file);

    // Destroy the old and create new
    file = fopen(path, "w");
    if(file == NULL)
    {
        retcode = -5;
        goto errorret;
    }
    fwrite(writeBuf, 1, strlen(writeBuf), file);


errorret:
    if(readBuf) free(readBuf);
    if(writeBuf) free(writeBuf);
    if(file) fclose(file);

    return retcode;
}


int CIni::ReadProfileString (const char* path, const char* section,
                                    const char* lpKeyName, const char* lpDefault,
		                                char* lpReturnedString, unsigned short nSize)
{
    int retcode = 0;
    char line[LINE_LEN];
    char temp_section[50];
    bool find_section = false,
         find_key = false;

    FILE *file = fopen(path, "r");
    if(file == NULL) return -1;

    sprintf(temp_section, "[%s]", section);
    while( fgets(line, LINE_LEN, file) )
    {
        if(line[0] == ';')
        {
            continue;
        }

        if(strstr(line, temp_section) != NULL)
        {
            find_section = true;
            break;
        }
    }
    if(find_section == false)
    {
        retcode = -2;
        goto errorret;
    }

    while( fgets(line, LINE_LEN, file) )
    {
        if(line[0] == ';')
        {
            continue;
        }

        if(line[0] == '[')
        {
            break;
        }

        if( !strncmp(line, lpKeyName, strlen(lpKeyName)) )
        {
            int len = strlen(lpKeyName);
            if(line[len] == ' '
                || line[len] == '\t'
                || line[len] == '\n'
                || line[len] == '=' ) // check if the whole word matched
            {
                find_key = true;
                break;
            }
        }
    }

    if(find_key == false)
    {
        if(lpDefault != NULL)
            strncpy(lpReturnedString, lpDefault, nSize);
        else
        {
            retcode = -3;
            goto errorret;
        }
    }
    else
    {
        strncpy(lpReturnedString, strstr(line, "=") + 1, nSize);
    }

errorret:
    fclose(file);

    return retcode;
}
#endif
