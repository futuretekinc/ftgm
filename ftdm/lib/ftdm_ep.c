#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_dbif.h"

#undef	__MODULE__
#define	__MODULE__ FTDM_TRACE_MODULE_EP

static FTM_RET FTDM_EP_getDataInfoInternal
(
	FTDM_EP_PTR		pEP,
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
	FTDM_DBIF_PTR	pDBIF,
	FTM_EP_PTR		pInfo,
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
	pEP->pDBIF = pDBIF;

	xRet = FTDM_DBIF_isEPExist(pDBIF, pInfo->pEPID, &bExist);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		return	xRet;	
	}

	if (!bExist)
	{
		xRet = FTDM_DBIF_appendEP(pDBIF, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "EP append failed.\n");
			FTM_MEM_free(pEP);
			return	xRet;
		}
	}
	else
	{
		FTM_ULONG	ulFirstTime, ulLastTime, ulCount = 0;

		xRet = FTDM_EP_getDataInfoInternal(pEP, &ulFirstTime, &ulLastTime, &ulCount);
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
	FTDM_DBIF_PTR	pDBIF = (*ppEP)->pDBIF;

	xRet = FTDM_DBIF_removeEP(pDBIF, (*ppEP)->xInfo.pEPID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The EP[%s] removed from database failed.\n", (*ppEP)->xInfo.pEPID);
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

FTM_RET	FTDM_EP_getInfo
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

FTM_RET	FTDM_EP_setInfo
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
		ERROR2(xRet, "EP is invalid!\n");
		return	xRet;	
	}

	if (strcmp(pEP->xInfo.pEPID, pInfo->pEPID) != 0)
	{
		ERROR2(xRet, "EPID mismatch!\n");
		return	FTM_RET_INVALID_ID;	
	}

	xRet = FTM_RET_OK;	

	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	xRet =FTDM_DBIF_setEP(pEP->pDBIF, pEP->xInfo.pEPID, &pEP->xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set EP.\n");
	}

	return	xRet;
}

FTM_RET	FTDM_EP_setFields
(
	FTDM_EP_PTR		pEP,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEP != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTM_EP	xInfo;

	memcpy(&xInfo, &pEP->xInfo, sizeof(FTM_EP));

	FTM_EP_setFields(&xInfo, xFields, pInfo);

	xRet = FTM_EP_isValid(&xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP is invalid!\n");
		return	xRet;	
	}

	if (strcmp(pEP->xInfo.pEPID, xInfo.pEPID) != 0)
	{
		ERROR2(xRet, "EPID mismatch!\n");
		return	FTM_RET_INVALID_ID;	
	}

	xRet = FTM_RET_OK;	

	memcpy(&pEP->xInfo, &xInfo, sizeof(FTM_EP));

	xRet =FTDM_DBIF_setEP(pEP->pDBIF, pEP->xInfo.pEPID, &pEP->xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set EP.\n");
	}

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

			xRet = FTDM_DBIF_delEPData(pEP->pDBIF, pEP->xInfo.pEPID, pEP->ulCount - ulCount, ulCount);
			if (xRet == FTM_RET_OK)
			{
				pEP->ulCount -= ulCount;

				FTM_ULONG	ulBeginTime, ulEndTime;
				xRet = FTDM_DBIF_getEPDataInfo(pEP->pDBIF, pEP->xInfo.pEPID,&ulBeginTime, &ulEndTime);
				if (xRet == FTM_RET_OK)
				{
					pEP->ulFirstTime = ulBeginTime;
					pEP->ulLastTime = ulEndTime;
				}
			}

		}
	}

	xRet = FTDM_DBIF_appendEPData(pEP->pDBIF, pEP->xInfo.pEPID, pData);
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

FTM_RET FTDM_EP_getDataInfoInternal
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

	FTM_RET		xRet;

	xRet = FTDM_DBIF_getEPDataInfo(pEP->pDBIF, pEP->xInfo.pEPID, pulBeginTime, pulEndTime);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTDM_DBIF_getEPDataCount(pEP->pDBIF, pEP->xInfo.pEPID, pulCount);
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

FTM_RET FTDM_EP_DATA_setLimit
(	
	FTDM_EP_PTR			pEP,
	FTM_EP_LIMIT_PTR	pLimit
)
{
	ASSERT(pEP != NULL);
	ASSERT(pLimit != NULL);
	FTM_RET		xRet;


	if (pEP->xInfo.xLimit.xType == pLimit->xType)
	{
		switch(pEP->xInfo.xLimit.xType)
		{
		case	FTM_EP_LIMIT_TYPE_COUNT:
			{
				if (pEP->xInfo.xLimit.xParams.ulCount == pLimit->xParams.ulCount )
				{
					return	FTM_RET_OK;
				}
			}
			break;

		case	FTM_EP_LIMIT_TYPE_TIME:
			{
				if ((pEP->xInfo.xLimit.xParams.xTime.ulStart == pLimit->xParams.xTime.ulStart)
					&& (pEP->xInfo.xLimit.xParams.xTime.ulEnd == pLimit->xParams.xTime.ulEnd))
				{
					return	FTM_RET_OK;
				}
			}
			break;

		case	FTM_EP_LIMIT_TYPE_HOURS:
			{
				if (pEP->xInfo.xLimit.xParams.ulHours == pLimit->xParams.ulHours)
				{
					return	FTM_RET_OK;
				}
			}
			break;

		case	FTM_EP_LIMIT_TYPE_DAYS:
			{
				if (pEP->xInfo.xLimit.xParams.ulDays == pLimit->xParams.ulDays)
				{
					return	FTM_RET_OK;
				}
			}
			break;

		case	FTM_EP_LIMIT_TYPE_MONTHS:
			{
				if (pEP->xInfo.xLimit.xParams.ulMonths == pLimit->xParams.ulMonths)
				{
					return	FTM_RET_OK;
				}
			}
			break;

		}
	}

	memcpy(&pEP->xInfo.xLimit, pLimit, sizeof(FTM_EP_LIMIT));
	switch(pEP->xInfo.xLimit.xType)
	{
	case	FTM_EP_LIMIT_TYPE_COUNT:
		{
			FTM_ULONG	ulCount = 0;

			xRet = FTDM_DBIF_getEPDataCount(pEP->pDBIF, pEP->xInfo.pEPID, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to get EP[%s] data count!\n", pEP->xInfo.pEPID);	
				break;
			}

			if (pLimit->xParams.ulCount < ulCount)
			{
				xRet = FTDM_DBIF_delEPData(pEP->pDBIF, pEP->xInfo.pEPID, pLimit->xParams.ulCount, ulCount - pLimit->xParams.ulCount);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to remove EP[%s] data[%lu:%lu]!\n", pEP->xInfo.pEPID,
							pLimit->xParams.ulCount, ulCount - pLimit->xParams.ulCount);
				}
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_TIME:
		{
			xRet = FTDM_DBIF_delEPDataWithTime(pEP->pDBIF, pEP->xInfo.pEPID, pLimit->xParams.xTime.ulStart, pLimit->xParams.xTime.ulEnd);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to remove EP[%s] data[%lu:%lu]!\n", pEP->xInfo.pEPID,
						pLimit->xParams.xTime.ulStart, pLimit->xParams.xTime.ulEnd);
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_HOURS:
		{
			FTM_TIME	xTime;
			FTM_TIME	xStartTime;
			FTM_TIME	xEndTime;
			FTM_ULONG	ulStartTime;
			FTM_ULONG	ulEndTime;

			xRet = FTDM_DBIF_getEPDataInfo(pEP->pDBIF, pEP->xInfo.pEPID, &ulStartTime, &ulEndTime);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to EP[%s] data times!\n", pEP->xInfo.pEPID);	
				break;
			}

			FTM_TIME_setSeconds(&xStartTime, ulStartTime);
			FTM_TIME_setSeconds(&xEndTime, ulEndTime);

			xRet = FTM_TIME_getCurrent(&xTime);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to get current time!\n");
				break;	
			}

			xRet = FTM_TIME_align(&xTime, FTM_TIME_ALIGN_1H, &xTime);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to align time!\n");	
				break;
			}

			xRet = FTM_TIME_subHours(&xTime, pLimit->xParams.ulHours, &xTime);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to calculate time!\n");
				break;
			}

			FTM_TIME_toSecs(&xEndTime, &ulStartTime);
			FTM_TIME_toSecs(&xTime, &ulEndTime);

			xRet = FTDM_DBIF_delEPDataWithTime(pEP->pDBIF, pEP->xInfo.pEPID, ulStartTime, ulEndTime);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to remove EP[%s] data.\n", pEP->xInfo.pEPID);			
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_DAYS:
		{
			if (pEP->xInfo.xLimit.xParams.ulDays == pLimit->xParams.ulDays)
			{
				return	FTM_RET_OK;
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_MONTHS:
		{
			if (pEP->xInfo.xLimit.xParams.ulMonths == pLimit->xParams.ulMonths)
			{
				return	FTM_RET_OK;
			}
		}
		break;

	}


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

	return	FTDM_DBIF_getEPDataList(
				pEP->pDBIF,
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
	FTM_BOOL			bAscending,
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	ASSERT(pEP != NULL);
	ASSERT(pCount != NULL);

	return	FTDM_DBIF_getEPDataListWithTime(
				pEP->pDBIF,
				pEP->xInfo.pEPID, 
				nBeginTime, 
				nEndTime, 
				bAscending,
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_EP_DATA_del
(
	FTDM_EP_PTR			pEP,
	FTM_INT 			nIndex, 
	FTM_ULONG			nCount,
	FTM_ULONG_PTR		pulCount
) 
{
	FTM_RET		xRet;
	FTM_ULONG	ulFirstTime, ulLastTime;
	FTM_ULONG	ulCount1 = 0, ulCount2= 0;

	xRet = FTDM_EP_getDataInfoInternal(pEP, &ulFirstTime, &ulLastTime, &ulCount1);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTDM_DBIF_delEPData(pEP->pDBIF, pEP->xInfo.pEPID, nIndex, nCount);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_getDataInfoInternal(pEP, &ulFirstTime, &ulLastTime, &ulCount2);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "EP[%s] information update failed.\n", pEP->xInfo.pEPID);	
		}
		else
		{
			pEP->ulFirstTime = ulFirstTime;
			pEP->ulLastTime = ulLastTime;
			pEP->ulCount = ulCount2;

			if (ulCount1 > ulCount2)
			{
				*pulCount = ulCount1 - ulCount2;
			}
			else
			{
				*pulCount = 0;
			}
		}
	
	}

	return	xRet;
}

FTM_RET	FTDM_EP_DATA_delWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pulCount
) 
{
	FTM_RET		xRet;
	FTM_ULONG	ulFirstTime, ulLastTime;
	FTM_ULONG	ulCount1 = 0, ulCount2 = 0;

	xRet = FTDM_EP_getDataInfoInternal(pEP, &ulFirstTime, &ulLastTime, &ulCount1);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTDM_DBIF_delEPDataWithTime(pEP->pDBIF, pEP->xInfo.pEPID, nBeginTime, nEndTime);
	if (xRet == FTM_RET_OK)
	{

		xRet = FTDM_EP_getDataInfoInternal(pEP, &ulFirstTime, &ulLastTime, &ulCount2);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "EP[%s] information update failed.\n", pEP->xInfo.pEPID);	
		}
		else
		{
			pEP->ulFirstTime = ulFirstTime;
			pEP->ulLastTime = ulLastTime;
			pEP->ulCount = ulCount2;

			if (ulCount1 > ulCount2)
			{
				*pulCount = ulCount1 - ulCount2;
			}
			else
			{
				*pulCount = 0;
			}
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

	return	FTDM_DBIF_getEPDataCountWithTime(pEP->pDBIF, pEP->xInfo.pEPID, nBeginTime, nEndTime, pulCount);
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

FTM_RET	FTDM_EP_print
(
	FTDM_EP_PTR			pEP
)
{
	ASSERT(pEP != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulCount;

	xRet = FTM_EP_print(&pEP->xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTDM_EP_DATA_count(pEP, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%-16s : %lu\n", "data count", ulCount);
	}
	else
	{
		MESSAGE("%-16s : %s\n", "data count", "unknown");
	}

	return	FTM_RET_OK;
}
