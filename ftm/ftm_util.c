#include <string.h>
#include "ftm_util.h"
#include "KISA_SHA256.h"

FTM_RET	FTM_makeID
(
	FTM_CHAR_PTR	pID,
	FTM_ULONG		ulLen
)
{
	ASSERT(pID != NULL);
	struct timeval	xTime;
	FTM_CHAR		pNewID[FTM_ID_LEN+1];
	FTM_ULONG		pValue[4];
	FTM_ULONG		pDigest[8];

	gettimeofday(&xTime, NULL);
	pValue[0] = (FTM_ULONG)xTime.tv_sec;
	pValue[1] = (FTM_ULONG)(xTime.tv_usec << 16) + (FTM_ULONG)xTime.tv_usec;
	pValue[2] = (FTM_ULONG)pValue[0] ^ 0xFFFFFFFF;
	pValue[3] = (FTM_ULONG)pValue[1] ^ 0xFFFFFFFF;

	SHA256_Encrpyt( (FTM_UINT8_PTR)pValue, sizeof(pValue), (FTM_UINT8_PTR)pDigest);

	pValue[2] = pDigest[0] ^ pDigest[2] ^ pDigest[4] ^ pDigest[6];
	pValue[3] = pDigest[1] ^ pDigest[3] ^ pDigest[5] ^ pDigest[7];

	sprintf(pNewID, "%08lx%08lx%08lx%08lx", pValue[0], pValue[1], pValue[2], pValue[3]);

	strncpy(pID, pNewID, ulLen);

	return	FTM_RET_OK;
}

