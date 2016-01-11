#ifndef _HTC_VERSION_INFO_H_
#define _HTC_VERSION_INFO_H_

typedef struct {
	unsigned char version[16];
	unsigned char build_mode[16];
	unsigned char commit_id[16];
} htc_version_info_struct;

extern unsigned int HTC_LK_HEADER;

#define HTC_LK_VERSION		(((htc_version_info_struct*)&HTC_LK_HEADER)->version)
#define HTC_LK_BUILD_MODE	(((htc_version_info_struct*)&HTC_LK_HEADER)->build_mode)
#define HTC_LK_COMMIT_ID	(((htc_version_info_struct*)&HTC_LK_HEADER)->commit_id)


#endif //_HTC_VERSION_INFO_H_
