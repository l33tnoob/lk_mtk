/* rsa.c
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

#include <htc_crypto.h>
#include <htc_rsa.h>
#include <htc_sha.h>

/* a[] -= mod */
static void subM(RSAPublicKey *key, uint32_t *a) {
    int64_t A = 0;
    int i;
    for (i = 0; i < key->len; ++i) {
        A += (uint64_t)a[i] - key->n[i];
        a[i] = (uint32_t)A;
        A >>= 32;
    }
}

/* return a[] >= mod */
static int geM(RSAPublicKey *key, const uint32_t *a) {
    int i;
    for (i = key->len; i;) {
        --i;
        if (a[i] < key->n[i]) return 0;
        if (a[i] > key->n[i]) return 1;
    }
    return 1;  /* equal */
}

/* montgomery c[] += a * b[] / R % mod */
static void montMulAdd(RSAPublicKey *key,
                       uint32_t* c,
                       const uint32_t a,
                       const uint32_t* b) {
    uint64_t A = (uint64_t)a * b[0] + c[0];
    uint32_t d0 = (uint32_t)A * key->n0inv;
    uint64_t B = (uint64_t)d0 * key->n[0] + (uint32_t)A;
    int i;

    for (i = 1; i < key->len; ++i) {
        A = (A >> 32) + (uint64_t)a * b[i] + c[i];
        B = (B >> 32) + (uint64_t)d0 * key->n[i] + (uint32_t)A;
        c[i - 1] = (uint32_t)B;
    }

    A = (A >> 32) + (B >> 32);

    c[i - 1] = (uint32_t)A;

    if (A >> 32) {
        subM(key, c);
    }
}

/* montgomery c[] = a[] * b[] / R % mod */
static void montMul(RSAPublicKey *key,
                    uint32_t* c,
                    const uint32_t* a,
                    const uint32_t* b) {
    int i;
    for (i = 0; i < key->len; ++i) {
        c[i] = 0;
    }
    for (i = 0; i < key->len; ++i) {
        montMulAdd(key, c, a[i], b);
    }
}

/* In-place public exponentiation.
** Input and output big-endian byte array in inout.
*/
void modpow3(RSAPublicKey *key, uint8_t* inout) {
    uint32_t a[RSANUMWORDS];
    uint32_t aR[RSANUMWORDS];
    uint32_t aaR[RSANUMWORDS];
    uint32_t *aaa = aR;  /* Re-use location. */
    int i;

    /* Convert from big endian byte array to little endian word array. */
    for (i = 0; i < key->len; ++i) {
        uint32_t tmp =
            (inout[((key->len - 1 - i) * 4) + 0] << 24) |
            (inout[((key->len - 1 - i) * 4) + 1] << 16) |
            (inout[((key->len - 1 - i) * 4) + 2] << 8) |
            (inout[((key->len - 1 - i) * 4) + 3] << 0);
        a[i] = tmp;
    }

    montMul(key, aR, a, key->rr);  /* aR = a * RR / R mod M   */
    montMul(key, aaR, aR, aR);     /* aaR = aR * aR / R mod M */
    montMul(key, aaa, aaR, a);     /* aaa = aaR * a / R mod M */

    /* Make sure aaa < mod; aaa is at most 1x mod too large. */
    if (geM(key, aaa)) {
        subM(key, aaa);
    }

    /* Convert to bigendian byte array */
    for (i = key->len - 1; i >= 0; --i) {
        uint32_t tmp = aaa[i];
        *inout++ = tmp >> 24;
        *inout++ = tmp >> 16;
        *inout++ = tmp >> 8;
        *inout++ = tmp >> 0;
    }
}

void modpow65537(RSAPublicKey *key, uint8_t* inout) {
    uint32_t a[RSANUMWORDS];
    uint32_t R1[RSANUMWORDS];
    uint32_t R2[RSANUMWORDS];
    uint32_t *final = R2;  /* Re-use location. */
    int i;

    /* Convert from big endian byte array to little endian word array. */
    for (i = 0; i < key->len; ++i) {
        uint32_t tmp =
            (inout[((key->len - 1 - i) * 4) + 0] << 24) |
            (inout[((key->len - 1 - i) * 4) + 1] << 16) |
            (inout[((key->len - 1 - i) * 4) + 2] << 8) |
            (inout[((key->len - 1 - i) * 4) + 3] << 0);
        a[i] = tmp;
    }

    montMul(key, R1, a, key->rr);
    for (i=0;i<8;i++){
       montMul(key, R2, R1, R1);  // 2,  8, 32, 128,  512, 2048,  8192, 32768
       montMul(key, R1, R2, R2);  // 4, 16, 64, 256, 1024, 4096, 16384, 65536
    }
    montMul(key, final, R1, a);

    /* Make sure 'final' < mod; 'final' is at most 1x mod too large. */
    if (geM(key, final)) {
        subM(key, final);
    }

    /* Convert to bigendian byte array */
    for (i = key->len - 1; i >= 0; --i) {
        uint32_t tmp = final[i];
        *inout++ = tmp >> 24;
        *inout++ = tmp >> 16;
        *inout++ = tmp >> 8;
        *inout++ = tmp >> 0;
    }
}

/* Expected PKCS1.5 signature padding bytes, for a keytool RSA signature.
** Has the 0-length optional parameter encoded in the ASN1 (as opposed to the
** other flavor which omits the optional parameter entirely). This code does not
** accept signatures without the optional parameter.
*/
static const uint8_t padding[RSANUMBYTES - SHA_DIGEST_SIZE] = {
    0x00,0x01,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
    0x30,0x21,0x30,0x09,0x06,0x05,0x2b,0x0e,0x03,0x02,0x1a,0x05,0x00,
    0x04,0x14
};

static const uint8_t sha256_padding[RSANUMBYTES - SHA256_DIGEST_SIZE] = {
    0x00,0x01,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x30,0x31,0x30,
    0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,
    0x00,0x04,0x20
};

/* Verify a 2048 bit RSA PKCS1.5 signature against an expected SHA-1/SHA-256 hash.
** Returns 0 on failure, 1 on success.
*/
int RSA_verify_ext(RSAPublicKey *key,
               const uint8_t *signature,
               const int len,
               const uint8_t *sha,
               const int halg) {
    uint8_t buf[RSANUMBYTES];
    int i;

    if (key->len != RSANUMWORDS) {
        return 0;  /* Wrong key passed in. */
    }

    if (len != sizeof(buf)) {
        return 0;  /* Wrong input length. */
    }

    for (i = 0; i < len; ++i) {
        buf[i] = signature[i];
    }
    if (key->exponent == RSA_EXPO_3)
        modpow3(key, buf);
    else if (key->exponent == RSA_EXPO_65537)
        modpow65537(key, buf);
    else
        return 0;

    /* Check pkcs1.5 padding bytes. */
	if (halg == HASH_SHA1){
		for (i = 0; (unsigned int)i < sizeof(padding); ++i) {
			if (buf[i] != padding[i]) {
				return 0;
			}
		}
	} else if (halg == HASH_SHA256) {
        for (i = 0; (unsigned int)i < sizeof(sha256_padding); ++i) {
            if (buf[i] != sha256_padding[i]) {
                return 0;
            }
        }
	}

    /* Check sha digest matches. */
    for (; i < len; ++i) {
        if (buf[i] != *sha++) {
            return 0;
        }
    }

    return 1;
}

