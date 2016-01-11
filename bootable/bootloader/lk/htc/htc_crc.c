#include "mt_typedefs.h"

#define uint32_t u32
#define bool BOOL
#define true TRUE
#define false FALSE


static unsigned dwCrcTable[256];


unsigned Reflect(unsigned ref, char ch)
{

	unsigned value = 0;
	int i;

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for (i = 1; i < (ch + 1); i++) {
		if (ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}


// Call this function only once to initialize the CRC table.
void Init_CRC32_Table(void)
{
	// This is the official polynomial used by CRC-32
	// in PKZip, WinZip and Ethernet.
	unsigned ulPolynomial = 0x04c11db7;
	int i, j;

	// 256 values representing ASCII character codes.
	for (i = 0; i <= 0xFF; i++) {
		dwCrcTable[i] = Reflect(i, 8) << 24;
		for (j = 0; j < 8; j++)
			dwCrcTable[i] =
			    (dwCrcTable[i] << 1) ^ (dwCrcTable[i] & (1 << 31) ?
						    ulPolynomial : 0);
		dwCrcTable[i] = Reflect(dwCrcTable[i], 32);
	}
}

uint32_t Crc32CheckSum(uint32_t dwStartAddr, uint32_t dwTotalLen, uint32_t crc)
{
	static bool bCRC32TableInited = false;
	uint32_t dwCheckSum, dwCvalue, dwCount;
	uint32_t* pulStartAddr;
	uint32_t dwTotalLenForDWORD;

	/*
	 * not print, that effects emapi_router
	 *
	HLOGD("dwStartAddr = 0x%X, dwTotalLen = 0x%X, crc = 0x%X\r\n",
		dwStartAddr, dwTotalLen, crc);
	*/

	if (!bCRC32TableInited) {
		Init_CRC32_Table();
		bCRC32TableInited = true;
	}

	dwCheckSum = crc;
	pulStartAddr = (uint32_t*) dwStartAddr;
	dwTotalLenForDWORD = (dwTotalLen / 4) * 4;

	for (dwCount = 0; dwCount < dwTotalLenForDWORD; dwCount += 4) {
		dwCvalue = *pulStartAddr;
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   (dwCvalue & 0xFF)];
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   ((dwCvalue >> 8) & 0xFF)];
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   ((dwCvalue >> 16) & 0xFF)];
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   ((dwCvalue >> 24) & 0xFF)];
		pulStartAddr++;
	}

	// the buffer length is not divided by 4, get last trial bytes and fill the other bytes with zero
	if (dwTotalLen > dwTotalLenForDWORD) {
		dwCvalue = 0;
		memcpy(&dwCvalue, pulStartAddr,
		       dwTotalLen - dwTotalLenForDWORD);

		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   (dwCvalue & 0xFF)];
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   ((dwCvalue >> 8) & 0xFF)];
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   ((dwCvalue >> 16) & 0xFF)];
		dwCheckSum =
		    (dwCheckSum >> 8) ^ dwCrcTable[(dwCheckSum & 0xFF) ^
						   ((dwCvalue >> 24) & 0xFF)];
	}

	return dwCheckSum;
}

