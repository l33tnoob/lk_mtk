#ifndef _C_INI_H_
#define _C_INI_H_

#include <string>

class CIni
{
public:
	CIni();
    CIni(const char *path);
	~CIni();

    void    SetIniPath(const char *path);
    void    Clear();

    int     Write(const char* lpSection, const char* lpKeyName, const char* lpString);
	int     Read(const char* section, const char* lpKeyName, const char* lpDefault,
		            char* lpReturnedString, unsigned short nSize);

private:
    std::string m_ini_path;

    int     WriteProfileString (const char* path, const char* section, 
                            const char* lpKeyName, const char* lpString);
    int     ReadProfileString (const char* path, const char* section,
                            const char* lpKeyName, const char* lpDefault,
		                    char* lpReturnedString, unsigned short nSize);
};

#endif //_C_INI_H_