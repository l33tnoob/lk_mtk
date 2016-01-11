/*************************************************************************
 *
 *  $Workfile: FileIO.c $
 *
 *  $Author: G-kihara $
 *  $Date: 08/05/19 17:39 $
 *  $Revision: 9 $
 *
 *  Developped by AsahiKASEI EMD Corporation
 *  Provided by AsahiKASEI Microsystems
 *  Copyright (C) 2007 AsahiKASEI EMD Corporation, All rights reserved
 *
 *************************************************************************/
#include "FileIO.h"
#include <utils/Log.h>
//==========================================================================
//
//  FUNCTION: loadInt16
//
//  PURPOSE:  Load Int16 data
//
//  DESCRIPTION:
//
//==========================================================================
void loadInt16(LPCSTR lpAppName, LPCSTR lpKeyName, int16* val, const int16 nDefault, LPCSTR lpFileName)
{
	uint ret;

	ret = getPrivateProfileInt(lpAppName, lpKeyName, (int)nDefault, lpFileName);
	*val = (int16)(ret);
}

//==========================================================================
//
//  FUNCTION:  loadInt16vec
//
//  PURPOSE:  Load Int16vec data
//
//  DESCRIPTION:
//
//==========================================================================
void loadInt16vec(LPCSTR lpAppName, LPCSTR lpKeyName, int16vec* vec, const int16 nDefault, LPCSTR lpFileName)
{
	char keyName[KEYNAME_SIZE];

	sprintf(keyName, "%s.x", lpKeyName);
	loadInt16(lpAppName, keyName, &(vec->u.x), nDefault, lpFileName);
	sprintf(keyName, "%s.y", lpKeyName);
	loadInt16(lpAppName, keyName, &(vec->u.y), nDefault, lpFileName);
	sprintf(keyName, "%s.z", lpKeyName);
	loadInt16(lpAppName, keyName, &(vec->u.z), nDefault, lpFileName);
}

//==========================================================================
//
//  FUNCTION: saveInt16
//
//  PURPOSE:  Save Int16 data
//
//  DESCRIPTION:
//
//==========================================================================
void saveInt16(LPCSTR lpAppName, LPCSTR lpKeyName, const int16 val, LPCSTR lpFileName)
{
	char keyData[KEYNAME_SIZE];

	sprintf(keyData, "%d", (int16)val);
	writePrivateProfileString(lpAppName, lpKeyName, keyData, lpFileName);
}

//==========================================================================
//
//  FUNCTION: saveInt16vec
//
//  PURPOSE:  Save Int16vec data
//
//  DESCRIPTION:
//
//==========================================================================
void saveInt16vec(LPCSTR lpAppName, LPCSTR lpKeyName, const int16vec* vec, LPCSTR lpFileName)
{
	char keyName[KEYNAME_SIZE];

	sprintf(keyName, "%s.x", lpKeyName);
	saveInt16(lpAppName, keyName, vec->u.x, lpFileName);
	sprintf(keyName, "%s.y", lpKeyName);
	saveInt16(lpAppName, keyName, vec->u.y, lpFileName);
	sprintf(keyName, "%s.z", lpKeyName);
	saveInt16(lpAppName, keyName, vec->u.z, lpFileName);
}

//==========================================================================
//
//  FUNCTION: getPrivateProfileInt
//
//  PURPOSE:
//
//  DESCRIPTION:
//
//==========================================================================
int16 getPrivateProfileInt(const char *sec, const char *key, const int16 def, const char *file)
{
	FILE *fp;
	char read_buf[32];
	char read_data[32];
	char section[32];
	char key_name[32];
	int16 len = 0;
	int16 len2 = 0;
	int16 len3 = 0;
	int16 ret = 0;
	int16 len_sec = 0;

	memset(read_buf,	0x0, sizeof(read_buf));
	memset(read_data,	0x0, sizeof(read_data));
	memset(section,		0x0, sizeof(section));
	memset(key_name,	0x0, sizeof(key_name));

	// open
	if((fp=fopen(file, "r")) == NULL){
		return def;
	}

	strcpy(section, SECTION_START);
	strcat(section, sec);
	strcat(section, SECTION_END);

	strcpy(key_name, key);
	strcat(key_name, DEMILITER);
	len=strlen(key_name);
	len_sec=strlen(section);

	while(fgets(read_buf, 32, fp)!=NULL){
		// search key
		if(strncmp(read_buf, key_name, len) != 0){
			memset(read_buf, 0x0, sizeof(read_buf));
			continue;
		}
		len2 = strlen(read_buf);
		len3 = (read_buf[len+len2-len-1] == '\n')? len2-len-1:len2-len;
		strncpy(read_data, &read_buf[len], len3);
		ret = 1;
		break;
	}

     // close
    fclose(fp);

    return (ret)?atoi(read_data):def;
}

//==========================================================================
//
//  FUNCTION: writePrivateProfileString
//
//  PURPOSE:
//
//  DESCRIPTION:
//
//==========================================================================
int16 writePrivateProfileString(const char *sec, const char *key, const char *str, const char *file)
{
	FILE *fp;
	FILE *nfp;
	char read_buf[32];
	char write_data[32];
	char read_data[32];
	char section[32];
	char tmpfile[32];
	char key_name[32];
	int16 len = 0;
	int16 ret = 0;
	int16 in_sec = 0;
	int16 len_sec = 0;
	int16 key_flg = 0;
	int16 ini_file_flg = 1;

	memset(read_buf,	0x0, sizeof(read_buf));
	memset(read_data,	0x0, sizeof(read_data));
	memset(write_data,	0x0, sizeof(write_data));
	memset(section,		0x0, sizeof(section));
	memset(tmpfile,		0x0, sizeof(tmpfile));
	memset(key_name,	0x0, sizeof(key_name));

	strcpy(tmpfile, file);
	strcat(tmpfile, TMP_EXT);

	sprintf(section, "%s%s%s\n", SECTION_START, sec, SECTION_END);

	strcpy(key_name, key);
	strcat(key_name, DEMILITER);
	len = strlen(key_name);
	len_sec = strlen(section);

	// open
	if((fp = fopen(file, "r")) == NULL){
		ini_file_flg = 0;
	}

	if((nfp = fopen(tmpfile, "w")) == NULL){
		ALOGE("open %s for write fail!!\n", tmpfile);
//		Disp_ErrorMessage(ERMES_TMPFILEOPEN_FAILED);
		return 0;
	}

	if(ini_file_flg == 1) {
		while(fgets(read_buf, 32, fp) != NULL){
			// search section start
			if(strncmp(read_buf, section, len_sec) != 0 && in_sec == 0){
				fputs(read_buf, nfp);
				memset(read_buf, 0x0, sizeof(read_buf));
				continue;
			}

			if(in_sec == 0){
				in_sec = 1;
				fputs(read_buf, nfp);
				memset(read_buf, 0x0, sizeof(read_buf));
				continue;
			}

			// search key
			if(strncmp(read_buf, key_name, len) != 0){
				fputs(read_buf, nfp);
				memset(read_buf, 0x0, sizeof(read_buf));
				continue;
			}

			// find key
			key_flg = 1;
			sprintf(write_data, "%s%s\n", key_name, str);

			fputs(write_data, nfp);

			memset(read_buf, 0x0, sizeof(read_buf));
			ret = 1;
		 }
	}

	// if section was not found, make section
	if(in_sec == 0){
		fputs(section, nfp);
	}
	// if key was not found, make key
	if(key_flg != 1){
		sprintf(write_data, "%s%s\n", key_name, str);
		fputs(write_data, nfp);
	}

	// close
	if(ini_file_flg == 1) {
		fclose(fp);
	}
	fclose(nfp);

	if(ini_file_flg == 1) {
		remove(file);
	}
	rename(tmpfile, file);

    return 1;
}
