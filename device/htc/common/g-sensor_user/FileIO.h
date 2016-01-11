#ifndef INC_FILEIO_H
#define INC_FILEIO_H

#include "stdio.h"
#include "sensor_type.h"

//========================= Constant definition ===============================//
#define SETTING_APP_NAME	"AK8973"
#define SETTING_FILE_NAME	"/data/misc/AK8973Prms.txt"
#define KEYNAME_SIZE		32

#define SECTION_START "["
#define SECTION_END "]"
#define DEMILITER "="
#define TMP_EXT ".tmp"
typedef const char * LPCSTR;


//========================= Prototype of Function =============================//
//Load
void loadInt16(LPCSTR lpAppName, LPCSTR lpKeyName, int16* val, const int16 nDefault, LPCSTR lpFileName);
void loadInt16vec(LPCSTR lpAppName, LPCSTR lpKeyName, int16vec* vec, const int16 nDefault, LPCSTR lpFileName);

//Save
void saveInt16(LPCSTR lpAppName, LPCSTR lpKeyName, const int16 val, LPCSTR lpFileName);
void saveInt16vec(LPCSTR lpAppName, LPCSTR lpKeyName, const int16vec* vec, LPCSTR lpFileName);

int16 getPrivateProfileInt(const char *sec, const char *key, const int16 def, const char *file);
int16 writePrivateProfileString(const char *sec, const char *key, const char *str, const char *file);

#endif

