#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_sqlite.h"

static FTM_RET FTDM_EP_DATA_infoInternal
(
	FTM_EP_ID		xEPID,
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

	pEP = (FTDM_EP_PTR)FTM_MEM_malloc(sizeof(FTDM_EP));
	if (pEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	memset(pEP, 0, sizeof(FTDM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	FTM_EP	xTmpInfo;

	xRet = FTDM_DBIF_EP_get(pEP->xInfo.xEPID, &xTmpInfo);
	if (xRet != FTM_RET_OK)
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

		xRet = FTDM_EP_DATA_infoInternal(pInfo->xEPID, &ulFirstTime, &ulLastTime, &ulCount);
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

	xRet = FTDM_DBIF_EP_del((*ppEP)->xInfo.xEPID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("The EP[%08x] removed from database failed.\n", (*ppEP)->xInfo.xEPID);
	}

	FTM_MEM_free(*ppEP);

	return	xRet;
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

			xRet = FTDM_DBIF_EP_DATA_del(pEP->xInfo.xEPID, pEP->ulCount - ulCount, ulCount);
			if (xRet == FTM_RET_OK)
			{
				pEP->ulCount -= ulCount;

				FTM_ULONG	ulBeginTime, ulEndTime;
				xRet = FTDM_DBIF_EP_DATA_info(pEP->xInfo.xEPID,&ulBeginTime, &ulEndTime);
				if (xRet == FTM_RET_OK)
				{
					pEP->ulFirstTime = ulBeginTime;
					pEP->ulLastTime = ulEndTime;
				}
			}

		}
	}

	xRet = FTDM_DBIF_EP_DATA_append(pEP->xInfo.xEPID, pData);
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
	FTM_EP_ID		xEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET		xRet;

	if ((pulBeginTime == NULL) || (pulEndTime == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTDM_DBIF_EP_DATA_info(xEPID, pulBeginTime, pulEndTime);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTDM_DBIF_EP_DATA_count(xEPID, pulCount);
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
				pEP->xInfo.xEPID, 
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
				pEP->xInfo.xEPID, 
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

	xRet = FTDM_DBIF_EP_DATA_del( pEP->xInfo.xEPID, nIndex, nCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulFirstTime, ulLastTime;
		FTM_ULONG	ulCount;

		xRet = FTDM_EP_DATA_infoInternal(pEP->xInfo.xEPID, &ulFirstTime, &ulLastTime, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%08x] information update failed.\n", pEP->xInfo.xEPID);	
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

	xRet = FTDM_DBIF_EP_DATA_delWithTime( pEP->xInfo.xEPID, nBeginTime, nEndTime);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulFirstTime, ulLastTime;
		FTM_ULONG	ulCount;

		xRet = FTDM_EP_DATA_infoInternal(pEP->xInfo.xEPID, &ulFirstTime, &ulLastTime, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%08x] information update failed.\n", pEP->xInfo.xEPID);	
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

	return	FTDM_DBIF_EP_DATA_countWithTime( pEP->xInfo.xEPID, nBeginTime, nEndTime, pulCount);
}

FTM_INT	FTDM_EPSeeker(const void *pElement, const void *pKey)
{
	FTDM_EP_PTR		pEP= (FTDM_EP_PTR)pElement;
	FTM_EP_ID_PTR	pEPID = (FTM_EP_ID_PTR)pKey;

	if (pEP->xInfo.xEPID == *pEPID)
	{
		return	1;	
	}

	return	0;
}

