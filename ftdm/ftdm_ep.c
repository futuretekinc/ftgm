#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_sqlite.h"

static FTM_RET FTDM_EP_DATA_infoInternal
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_EP_init
(
	FTDM_EP_PTR	pEP,
	FTM_EP_PTR	pInfo
)
{
	ASSERT(pEP != NULL);
	ASSERT(pInfo != NULL);

	memset(&pEP, 0, sizeof(FTM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	return	FTM_RET_OK;
}

FTM_RET FTDM_EP_final
(
	FTDM_EP_PTR	pEP
)
{
	return	FTM_RET_OK;
}


FTM_RET	FTDM_EP_create
(
	FTM_EP_PTR	pInfo,
	FTDM_EP_PTR	_PTR_ ppEP
)
{
	ASSERT(pInfo != NULL);

	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;
	FTM_BOOL	bExist = FTM_TRUE;

	pEP = (FTDM_EP_PTR)FTM_MEM_malloc(sizeof(FTDM_EP));
	if (pEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	memset(pEP, 0, sizeof(FTDM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	xRet = FTDM_DBIF_EP_isExist(pInfo->pEPID, &bExist);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		return	xRet;	
	}

	if (!bExist)
	{
		xRet = FTDM_DBIF_EP_append(pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP append failed[%08x].\n", xRet);
			FTM_MEM_free(pEP);
			return	xRet;
		}
	}
	else
	{
		FTM_ULONG	ulFirstTime, ulLastTime, ulCount = 0;

		xRet = FTDM_EP_DATA_infoInternal(pInfo->pEPID, &ulFirstTime, &ulLastTime, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			pEP->ulFirstTime = ulFirstTime;
			pEP->ulLastTime = ulLastTime;
			pEP->ulCount = ulCount;
		}
	}

	*ppEP = pEP;
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_destroy
(
	FTDM_EP_PTR	_PTR_ ppEP
)
{
	ASSERT(ppEP != NULL);

	FTM_RET		xRet;

	xRet = FTM_EP_isStatic(&(*ppEP)->xInfo);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_EP_del((*ppEP)->xInfo.pEPID);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The EP[%s] removed from database failed.\n", (*ppEP)->xInfo.pEPID);
		}
	}
	else
	{
		xRet = FTM_RET_OK;	
	}

	FTM_MEM_free(*ppEP);

	*ppEP = NULL;

	return	xRet;
}

FTM_RET	FTDM_EP_destroy2
(
	FTDM_EP_PTR	_PTR_ ppEP
)
{
	ASSERT(ppEP != NULL);

	FTM_MEM_free(*ppEP);

	*ppEP = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_get
(
	FTDM_EP_PTR		pEP,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEP != NULL);
	ASSERT(pInfo != NULL);

	memcpy(pInfo, &pEP->xInfo, sizeof(FTM_EP));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_set
(
	FTDM_EP_PTR		pEP,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEP != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;

	xRet = FTM_EP_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (strcmp(pEP->xInfo.pEPID, pInfo->pEPID) != 0)
	{
		return	FTM_RET_INVALID_ID;	
	}

	if (FTM_EP_isStatic(&pEP->xInfo) == FTM_RET_OK)
	{
		memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

		if (FTM_EP_isStatic(pInfo) != FTM_RET_OK)
		{
			xRet =FTDM_DBIF_EP_del(pEP->xInfo.pEPID);
		}
	}
	else
	{
		memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

		if (FTM_EP_isStatic(pInfo) == FTM_RET_OK)
		{
			xRet =FTDM_DBIF_EP_append(&pEP->xInfo);
		}
	}

	return	FTM_RET_OK;
}


FTM_RET	FTDM_EP_DATA_add
(
	FTDM_EP_PTR		pEP,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	
	if (pEP->xInfo.xLimit.xType == FTM_EP_LIMIT_TYPE_COUNT)
	{
	
		if (pEP->ulCount >= pEP->xInfo.xLimit.xParams.ulCount)
		{
			FTM_ULONG	ulCount = pEP->ulCount - pEP->xInfo.xLimit.xParams.ulCount + 1;

			xRet = FTDM_DBIF_EP_DATA_del(pEP->xInfo.pEPID, pEP->ulCount - ulCount, ulCount);
			if (xRet == FTM_RET_OK)
			{
				pEP->ulCount -= ulCount;

				FTM_ULONG	ulBeginTime, ulEndTime;
				xRet = FTDM_DBIF_EP_DATA_info(pEP->xInfo.pEPID,&ulBeginTime, &ulEndTime);
				if (xRet == FTM_RET_OK)
				{
					pEP->ulFirstTime = ulBeginTime;
					pEP->ulLastTime = ulEndTime;
				}
			}

		}
	}

	xRet = FTDM_DBIF_EP_DATA_append(pEP->xInfo.pEPID, pData);
	if (xRet == FTM_RET_OK)
	{
		pEP->ulCount++;	
		if ((pEP->ulLastTime == 0) || (pEP->ulLastTime < pData->ulTime))
		{
			pEP->ulLastTime = pData->ulTime;
		}

		if((pEP->ulFirstTime == 0) || (pEP->ulFirstTime > pData->ulTime))
		{
			pEP->ulFirstTime = pData->ulTime;
		}
	}

	return	xRet;
}

FTM_RET FTDM_EP_DATA_infoInternal
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulBeginTime != NULL);
	ASSERT(pulEndTime != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;

	xRet = FTDM_DBIF_EP_DATA_info(pEPID, pulBeginTime, pulEndTime);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTDM_DBIF_EP_DATA_count(pEPID, pulCount);
}

FTM_RET FTDM_EP_DATA_info
	
(	
	FTDM_EP_PTR		pEP,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulBeginTime != NULL);
	ASSERT(pulEndTime != NULL);
	ASSERT(pulCount != NULL);

	*pulBeginTime = pEP->ulFirstTime;
	*pulEndTime	= pEP->ulLastTime;
	*pulCount = pEP->ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_DATA_get
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	ASSERT(pEP != NULL);
	ASSERT(pEPData != NULL);
	ASSERT(pCount != NULL);

	return	FTDM_DBIF_EP_DATA_get(
				pEP->xInfo.pEPID, 
				nStartIndex,
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_EP_DATA_getWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	ASSERT(pEP != NULL);
	ASSERT(pCount != NULL);

	return	FTDM_DBIF_EP_DATA_getWithTime(
				pEP->xInfo.pEPID, 
				nBeginTime, 
				nEndTime, 
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_EP_DATA_del
(
	FTDM_EP_PTR			pEP,
	FTM_INT 			nIndex, 
	FTM_ULONG			nCount
) 
{
	FTM_RET		xRet;

	xRet = FTDM_DBIF_EP_DATA_del( pEP->xInfo.pEPID, nIndex, nCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulFirstTime, ulLastTime;
		FTM_ULONG	ulCount;

		xRet = FTDM_EP_DATA_infoInternal(pEP->xInfo.pEPID, &ulFirstTime, &ulLastTime, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%s] information update failed.\n", pEP->xInfo.pEPID);	
		}
		else
		{
			pEP->ulFirstTime = ulFirstTime;
			pEP->ulLastTime = ulLastTime;
			pEP->ulCount = ulCount;
		}
	
	}

	return	xRet;
}

FTM_RET	FTDM_EP_DATA_delWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime
) 
{
	FTM_RET		xRet;

	xRet = FTDM_DBIF_EP_DATA_delWithTime( pEP->xInfo.pEPID, nBeginTime, nEndTime);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulFirstTime, ulLastTime;
		FTM_ULONG	ulCount;

		xRet = FTDM_EP_DATA_infoInternal(pEP->xInfo.pEPID, &ulFirstTime, &ulLastTime, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%s] information update failed.\n", pEP->xInfo.pEPID);	
		}
		else
		{
			pEP->ulFirstTime = ulFirstTime;
			pEP->ulLastTime = ulLastTime;
			pEP->ulCount = ulCount;
		}
	
	}

	return	xRet;
}

FTM_RET	FTDM_EP_DATA_count
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG_PTR		pulCount
) 
{
	ASSERT(pulCount != NULL);

	*pulCount = pEP->ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_DATA_countWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pulCount
) 
{
	ASSERT(pulCount != NULL);

	return	FTDM_DBIF_EP_DATA_countWithTime( pEP->xInfo.pEPID, nBeginTime, nEndTime, pulCount);
}

FTM_INT	FTDM_EPSeeker
(
	const FTM_VOID_PTR	pElement, 
	const FTM_VOID_PTR	pKey
)
{
	FTDM_EP_PTR		pEP= (FTDM_EP_PTR)pElement;
	FTM_CHAR_PTR	pEPID = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pEP->xInfo.pEPID, pEPID) == 0);
}

