#ifndef __CRYPTO_H__
#define __CRYPTO_H__

// sha
#define HASH_UNDEFINED_HASH_CODE 0
#define HASH_SHA1 1
#define HASH_SHA256 2
#define HASH_UNSUPPORTED_HASH_CODE 3
#define HASH_HASH_RESERVED 0x7fffffff /* force to 32 bits */

// rsa
#define RSA_EXPO_3 3
#define RSA_EXPO_65537 65537

// aes
#define AES_ENCRYPT	1
#define AES_DECRYPT	0

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define HBOOT_MAGIC "Hbo@tSig"
#define HBOOT_MAGIC_SIZE 8
#define DIGEST_SIZE 20
#define DIGEST256_SIZE 32
#define SIGNATURE_SIZE 256

void compute_digest_sha256(void *data, unsigned dlen, void *digest_out);
int is_signature_okay_ext(void *digest, const int halg, void *signature, void *pubkey);

#endif
