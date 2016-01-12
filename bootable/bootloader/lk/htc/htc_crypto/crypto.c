//#include "bootimg.h"
#include <htc_sha.h>
#include <htc_rsa.h>
#include <htc_aes_rijndael.h>
#include <htc_crypto.h>

extern const uint8_t* SHA256(const void *data, int len, uint8_t *digest);

void compute_digest(void *data, unsigned dlen, void *digest_out)
{
    uint8_t *digest = digest_out;
    SHA(data, dlen, digest);
}

void compute_digest_sha256(void *data, unsigned dlen, void *digest_out)
{
    uint8_t *digest = digest_out;
    SHA256(data, dlen, digest);
}

int is_signature_okay_ext(void *digest, const int halg, void *signature, void *pubkey)
{
	if (!digest || !signature || !pubkey)
		return 0;

    return RSA_verify_ext(pubkey, signature, SIGNATURE_SIZE, digest, halg);
}

void compute_rsa_cipher(void *key, void *inout, const int expo)
{
	if (expo==RSA_EXPO_3)
		modpow3(key, inout);
	else if (expo==RSA_EXPO_65537)
		modpow65537(key, inout);
}

void htc_compute_aes_cbc_cipher(const unsigned char *in, unsigned char *out, unsigned long length,
                            const unsigned char *szKey, const unsigned int key_size,
                            unsigned char *ivec, const int crypto){
	htc_AES_cbc_crypto(in, out, length, szKey, key_size, ivec, crypto);
}

void htc_compute_aes_ctr_cipher(const unsigned char *in, unsigned char *out, unsigned long length,
                            const unsigned char *szKey, const unsigned int key_size,
                            unsigned char *ivec){
	htc_AES_ctr_crypto(in, out, length, szKey, key_size, ivec);
}

