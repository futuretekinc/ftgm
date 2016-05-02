#include <string.h>
#include "ftm_util.h"

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

	gettimeofday(&xTime, NULL);
	pValue[0] = (FTM_ULONG)xTime.tv_sec;
	pValue[1] = (FTM_ULONG)(xTime.tv_usec << 16) + (FTM_ULONG)xTime.tv_usec;
	pValue[2] = (FTM_ULONG)pValue[0] ^ 0xFFFFFFFF;
	pValue[3] = (FTM_ULONG)pValue[1] ^ 0xFFFFFFFF;

	sprintf(pNewID, "%08lx%08lx%08lx%08lx", pValue[0], pValue[1], pValue[2], pValue[3]);

	strncpy(pID, pNewID, ulLen);

	return	FTM_RET_OK;
}
