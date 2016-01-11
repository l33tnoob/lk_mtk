/*-
 * Copyright (c) 2001-2003 Allan Saddi <allan@saddi.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ALLAN SADDI AND HIS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL ALLAN SADDI OR HIS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: sha256.c 680 2003-07-25 21:57:49Z asaddi $
 */

#include <htc_sha.h>

#define Ch(x, y, z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sum0(x) ((((x) >> 2) | ((x) << 30)) ^ (((x) >> 13) | ((x) << 19)) ^ (((x) >> 22) | ((x) << 10)))
#define Sum1(x) ((((x) >> 6) | ((x) << 26)) ^ (((x) >> 11) | ((x) << 21)) ^ (((x) >> 25) | ((x) << 7)))
#define Theta0(x) ((((x) >> 7) | ((x) << 25)) ^ (((x) >> 18) | ((x) << 14)) ^ ((x) >> 3))
#define Theta1(x) ((((x) >> 17) | ((x) << 15)) ^ (((x) >> 19) | ((x) << 13)) ^ ((x) >> 10))

static const uint32_t K[64] = {
	0x428a2f98L, 0x71374491L, 0xb5c0fbcfL, 0xe9b5dba5L, 0x3956c25bL, 0x59f111f1L, 0x923f82a4L, 0xab1c5ed5L,
	0xd807aa98L, 0x12835b01L, 0x243185beL, 0x550c7dc3L, 0x72be5d74L, 0x80deb1feL, 0x9bdc06a7L, 0xc19bf174L,
	0xe49b69c1L, 0xefbe4786L, 0x0fc19dc6L, 0x240ca1ccL, 0x2de92c6fL, 0x4a7484aaL, 0x5cb0a9dcL, 0x76f988daL,
	0x983e5152L, 0xa831c66dL, 0xb00327c8L, 0xbf597fc7L, 0xc6e00bf3L, 0xd5a79147L, 0x06ca6351L, 0x14292967L,
	0x27b70a85L, 0x2e1b2138L, 0x4d2c6dfcL, 0x53380d13L, 0x650a7354L, 0x766a0abbL, 0x81c2c92eL, 0x92722c85L,
	0xa2bfe8a1L, 0xa81a664bL, 0xc24b8b70L, 0xc76c51a3L, 0xd192e819L, 0xd6990624L, 0xf40e3585L, 0x106aa070L,
	0x19a4c116L, 0x1e376c08L, 0x2748774cL, 0x34b0bcb5L, 0x391c0cb3L, 0x4ed8aa4aL, 0x5b9cca4fL, 0x682e6ff3L,
	0x748f82eeL, 0x78a5636fL, 0x84c87814L, 0x8cc70208L, 0x90befffaL, 0xa4506cebL, 0xbef9a3f7L, 0xc67178f2L
};

void processBlock(SHA256Context *sc){
	int t, i;
	uint32_t a = sc->H1, b = sc->H2, c = sc->H3, d = sc->H4;
	uint32_t e = sc->H5, f = sc->H6, g = sc->H7, h = sc->H8;

	for (t = 16; t <= 63; t++)
		sc->X[t] = Theta1(sc->X[t - 2]) + sc->X[t - 7] + Theta0(sc->X[t - 15]) + sc->X[t - 16];

	t = 0;
	for(i = 0; i < 8; i ++){
		h += Sum1(e) + Ch(e, f, g) + K[t] + sc->X[t], t++;
		d += h;
		h += Sum0(a) + Maj(a, b, c);
		g += Sum1(d) + Ch(d, e, f) + K[t] + sc->X[t], t++;
		c += g;
		g += Sum0(h) + Maj(h, a, b);
		f += Sum1(c) + Ch(c, d, e) + K[t] + sc->X[t], t++;
		b += f;
		f += Sum0(g) + Maj(g, h, a);
		e += Sum1(b) + Ch(b, c, d) + K[t] + sc->X[t], t++;
		a += e;
		e += Sum0(f) + Maj(f, g, h);
		d += Sum1(a) + Ch(a, b, c) + K[t] + sc->X[t], t++;
		h += d;
		d += Sum0(e) + Maj(e, f, g);
		c += Sum1(h) + Ch(h, a, b) + K[t] + sc->X[t], t++;
		g += c;
		c += Sum0(d) + Maj(d, e, f);
		b += Sum1(g) + Ch(g, h, a) + K[t] + sc->X[t], t++;
		f += b;
		b += Sum0(c) + Maj(c, d, e);
		a += Sum1(f) + Ch(f, g, h) + K[t] + sc->X[t], t++;
		e += a;
		a += Sum0(b) + Maj(b, c, d);
	}

	sc->H1 += a;
	sc->H2 += b;
	sc->H3 += c;
	sc->H4 += d;
	sc->H5 += e;
	sc->H6 += f;
	sc->H7 += g;
	sc->H8 += h;

	sc->xOff = 0;
	for (i = 0; i < 16; i++)
		sc->X[i] = 0;
}

void processLength(SHA256Context *sc, uint64_t bitLength){
	if (sc->xOff > 14)
		processBlock(sc);

	sc->X[14] = (uint32_t)((bitLength >> 32) & 0xFFFFFFFF);
	sc->X[15] = (uint32_t)(bitLength & 0xFFFFFFFF);
}

void processWord(SHA256Context *sc, uint8_t inbuf[], int inOff){
	sc->X[sc->xOff++] = ((inbuf[inOff] & 0xff) << 24) | ((inbuf[inOff + 1] & 0xff) << 16)
				| ((inbuf[inOff + 2] & 0xff) << 8) | ((inbuf[inOff + 3] & 0xff));

	if (sc->xOff == 16)
		processBlock(sc);
}

void unpackWord(uint32_t word, uint8_t outbuf[], int outOff){
	outbuf[outOff]     = (uint8_t)((word >> 24) & 0xFF);
	outbuf[outOff + 1] = (uint8_t)((word >> 16) & 0xFF);
	outbuf[outOff + 2] = (uint8_t)((word >> 8) & 0xFF);
	outbuf[outOff + 3] = (uint8_t)(word & 0xFF);
}

void update(SHA256Context *sc, uint8_t in){
	sc->xBuf[sc->xBufOff++] = in;

	if (sc->xBufOff == sizeof(sc->xBuf)){
		processWord(sc, sc->xBuf, 0);
		sc->xBufOff = 0;
	}

	sc->byteCount++;
}

void finish(SHA256Context *sc){
	uint64_t bitLength = (sc->byteCount << 3);

	update(sc, (uint8_t)128);

	while (sc->xBufOff != 0)
		update(sc, (uint8_t)0);

	processLength(sc, bitLength);

	processBlock(sc);
}

void SHA256Init(SHA256Context *sc){
	int i;

	sc->byteCount = 0;

	sc->H1 = 0x6a09e667L;
	sc->H2 = 0xbb67ae85L;
	sc->H3 = 0x3c6ef372L;
	sc->H4 = 0xa54ff53aL;
	sc->H5 = 0x510e527fL;
	sc->H6 = 0x9b05688cL;
	sc->H7 = 0x1f83d9abL;
	sc->H8 = 0x5be0cd19L;

	sc->xOff = 0;
	for (i = 0; i != sizeof(sc->X)/4; i++)
		sc->X[i] = 0;

	sc->xBufOff = 0;
	for (i = 0; i != sizeof(sc->xBuf); i++)
		sc->xBuf[i] = 0;
}

void SHA256Update(SHA256Context *sc, uint8_t inbuf[], int inOff, int len){
	while ((sc->xBufOff != 0) && (len > 0)){
		update(sc, inbuf[inOff]);

		inOff++;
		len--;
	}

	while (len > sizeof(sc->xBuf)){
		processWord(sc, inbuf, inOff);

		inOff += sizeof(sc->xBuf);
		len -= sizeof(sc->xBuf);
		sc->byteCount += sizeof(sc->xBuf);
	}

	while (len > 0){
		update(sc, inbuf[inOff]);

		inOff++;
		len--;
	}
}

void SHA256Final(SHA256Context *sc, uint8_t outbuf[], int outOff){
	finish(sc);

	unpackWord(sc->H1, outbuf, outOff);
	unpackWord(sc->H2, outbuf, outOff + 4);
	unpackWord(sc->H3, outbuf, outOff + 8);
	unpackWord(sc->H4, outbuf, outOff + 12);
	unpackWord(sc->H5, outbuf, outOff + 16);
	unpackWord(sc->H6, outbuf, outOff + 20);
	unpackWord(sc->H7, outbuf, outOff + 24);
	unpackWord(sc->H8, outbuf, outOff + 28);
}


/* Convenience function */
const uint8_t* SHA256(const void *data, int len, uint8_t *digest) {
	SHA256Context ctx;
	SHA256Init(&ctx);
	SHA256Update(&ctx, data, 0, len);
	SHA256Final(&ctx, digest, 0);

    return digest;
}

