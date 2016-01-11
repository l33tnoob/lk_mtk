#ifndef HTCSECURE_H
#define HTCSECURE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int boolean;

#define	true	1
#define false	0

void setHtcSecureRule(boolean bHtc_MFG_Secure_Rule_Enabled);

boolean getHtcSecureRule(void);

char* getHtcSecureRuleVerifyCode(void);

boolean setHtcEncryptedCode(char * strHtc_Encrypted_Code);

boolean setEncodedCodeByEncryptedCode(char* strHtc_Encoded_Code_By_Encrypted_Code);

boolean isCameraKeepOff(void);
#ifdef __cplusplus
}
#endif
#endif
