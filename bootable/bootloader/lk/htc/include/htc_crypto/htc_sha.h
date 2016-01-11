/* sha.h
**
** Copyright 2008, Google Inc.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of Google Inc. nor the names of its contributors may
**       be used to endorse or promote products derived from this software
**       without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY Google Inc. ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
** EVENT SHALL Google Inc. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
** OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
** ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _EMBEDDED_SHA_H_
#define _EMBEDDED_SHA_H_

#include "htc_inttypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_UNDEFINED_HASH_CODE 0
#define HASH_SHA1 1
#define HASH_SHA256 2
#define HASH_UNSUPPORTED_HASH_CODE 3
#define HASH_HASH_RESERVED 0x7fffffff /* force to 32 bits */

typedef struct SHA_CTX {
    uint64_t count;
    uint8_t buf[64];
    uint32_t state[5];
} SHA_CTX;

typedef struct SHA256Context{
	uint64_t byteCount;
	uint32_t H1, H2, H3, H4, H5, H6, H7, H8;
	int xOff;
	uint32_t X[64];
	int xBufOff;
	uint8_t xBuf[4];
} SHA256Context;

void htc_SHA_init(SHA_CTX *ctx);
void htc_SHA_update(SHA_CTX *ctx, const void* data, unsigned long long len);
const uint8_t* htc_SHA_final(SHA_CTX *ctx);

/* Convenience method. Returns digest parameter value. */
const uint8_t* SHA(const void *data, int len, uint8_t *digest);

//void SHA256Init(SHA256Context *sc);
//void SHA256Update(SHA256Context *sc, uint8_t inbuf[], int inOff, int len);
//void SHA256Final(SHA256Context *sc, uint8_t outbuf[], int outOff);

/* Convenience method. Returns digest parameter value. */
//const uint8_t* SHA256(const void *data, int len, uint8_t *digest);

#define SHA_DIGEST_SIZE 20
#define SHA256_DIGEST_SIZE 32


#ifdef __cplusplus
}
#endif

#endif
