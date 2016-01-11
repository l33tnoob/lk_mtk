#ifndef _CHECKSUM_GEN_H_
#define _CHECKSUM_GEN_H_

#ifdef WIN32    /* windows */
#define PATH_PADDING    "\\"
#else           /* linux*/ 
#define PATH_PADDING    "/"
#endif


#define TOOL_VERSION        "v3.1512.00"

#define VERSION_SECTION     "VERSION"
#define VERSION_KEYWORD     "VERSION"
#define TOOL_VER_KEYWORD    "TOOL"
#define CHANGE_ID           "Change_ID"

#define CHECKSUM_FILE       "Checksum.ini"
#define CHECKSUM_SECTION    "CheckSum"
#define CHECKSUM_SECTION_V2 "CheckSum_V2"

#define ENABLE_SECTION      "IsEnableChecksum"
#define ENABLE_ITEM         "CHECKSUM_SWITCH"


#endif //_CHECKSUM_GEN_H_