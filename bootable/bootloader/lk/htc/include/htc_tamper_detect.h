#ifndef __HTC_TAMPER_DETECT_H
#define __HTC_TAMPER_DETECT_H

#define TAMLPER_ENCRYPT_UINT            256   /* if change this, remember to adjust it in setting.h also */

#define TAMPER_INFO_MAGIC               0xC6322568

/* Tamper reason */
#define TAMPER_TYPE_VALUE_INVALID       0x001
#define TAMPER_TYPE_SIGN_FAIL_BOOT      0x002
#define TAMPER_TYPE_SIGN_FAIL_RECOVERY  0x004
#define TAMPER_TYPE_UNLOCK_UPDATE_SYS   0x008
#define TAMPER_TYPE_SU_FOUND            0x010
#define TAMPER_TYPE_RUN_ROOT_ADBD       0x020
#define TAMPER_TYPE_RUN_ROOT_SH         0x040
#define TAMPER_TYPE_MNT_RW_SYS          0x080
#define TAMPER_TYPE_MNT_RW_RTFS         0x100
#define TAMPER_TYPE_PROP_NG_SECURE      0x200
#define TAMPER_TYPE_PROP_NG_DBG         0x400
#define TAMPER_TYPE_SIGN_FAIL_HOSD      0x800
#define TAMPER_TYPE_NUMBER              12	  /* should be adjusted if u add new tamper type */
#define TAMPER_TYPE_VALID_MASK          0xFFE /* should be adjusted if u add new tamper type */
#define TAMPER_TYPE_MASK                0xFFF /* should be adjusted if u add new tamper type */

int tamper_set_flag(unsigned tamper_flag);
int tamper_reset_flag(void);
int tamper_get_flag(unsigned *tamper_flag);
int tamper_get_status(void);
int tamper_sync_from_misc(void);
void tamper_rec_update_image(const char *image_name);
void tamper_chk_update_image(void);

#endif
