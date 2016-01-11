#ifndef HEADER_AES_RIJNDAEL_H
#define HEADER_AES_RIJNDAEL_H

#define AES_ENCRYPT	1
#define AES_DECRYPT	0
#define AES_BLOCK_SIZE 16
#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  ((keybits)/8+28)
#define NROUNDS(keybits)   ((keybits)/32+6)

#ifdef  __cplusplus
extern "C" {
#endif

int htc_rijndaelSetupEncrypt(unsigned long *rk, const unsigned char *key, int keybits);
int htc_rijndaelSetupDecrypt(unsigned long *rk, const unsigned char *key, int keybits);
void htc_rijndaelEncrypt(const unsigned long *rk, int nrounds,
                     const unsigned char plaintext[AES_BLOCK_SIZE], unsigned char ciphertext[AES_BLOCK_SIZE]);
void htc_rijndaelDecrypt(const unsigned long *rk, int nrounds,
                     const unsigned char ciphertext[AES_BLOCK_SIZE], unsigned char plaintext[AES_BLOCK_SIZE]);
void htc_AES_cbc_crypto(const unsigned char *in, unsigned char *out, unsigned long length,
                    const unsigned char *szKey, const unsigned int key_size,
                    unsigned char *ivec, const int crypto);
void htc_AES_ctr_crypto(const unsigned char *in, unsigned char *out, unsigned long length,
                    const unsigned char *szKey, const unsigned int key_size,
                    unsigned char *ivec);
void htc_AES_ctr_inc(unsigned char *counter);

#ifdef  __cplusplus
}
#endif

#endif

