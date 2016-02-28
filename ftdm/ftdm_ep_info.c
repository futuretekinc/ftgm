#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_ep_info.h"
#include "ftdm_sqlite.h"
#include "ftm_list.h"
#include "ftm_mem.h"

static FTM_INT	FTDM_EPSeeker
(
	const void *pElement, 
	const void *pKey)
;

static FTM_LIST	xEPList;

FTM_RET	FTDM_EP_init
(
	FTDM_CFG_EP_PTR	pConfig
)
{
	FTM_ULONG	nMaxEPCount = 0;

	if (FTM_LIST_init(&xEPList) < 0)
	{
		return	FTM_RET_INTERNAL_ERROR;	
	}

	FTM_LIST_setSeeker(&xEPList, FTDM_EPSeeker);

	if ((FTDM_DBIF_EP_count(&nMaxEPCount) == FTM_RET_OK) &&
		(nMaxEPCount > 0))
	{

		FTM_EP_PTR	pEPInfos;
		FTM_ULONG			nEPCount = 0;
		
		pEPInfos = (FTM_EP_PTR)calloc(nMaxEPCount, sizeof(FTM_EP));
		if (pEPInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_EP_getList(pEPInfos, nMaxEPCount, &nEPCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nEPCount ; i++)
			{
				FTM_EP_PTR	pEPInfo;
				pEPInfo = (FTM_EP_PTR)malloc(sizeof(FTM_EP));
				if (pEPInfo == NULL)
				{
					return	FTM_RET_NOT_ENOUGH_MEMORY;	
				}
	
				memcpy(pEPInfo, &pEPInfos[i], sizeof(FTM_EP));

				FTM_LIST_append(&xEPList, pEPInfo);
			}
		}

		free(pEPInfos);
	}

	if (FTDM_CFG_EP_count(pConfig, &nMaxEPCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < nMaxEPCount ; i++)
		{
			FTM_EP	xEPInfo;

			if (FTDM_CFG_EP_getAt(pConfig, i, &xEPInfo) == FTM_RET_OK)
			{
				if (FTM_LIST_seek(&xEPList, &xEPInfo.xEPID) != FTM_RET_OK)
				{
					FTDM_EP_add(&xEPInfo);	
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_EP_final
(
	FTM_VOID
)
{
	FTM_EP_PTR pEPInfo;

	FTM_LIST_iteratorStart(&xEPList);
	while(FTM_LIST_iteratorNext(&xEPList, (FTM_VOID_PTR _PTR_)&pEPInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pEPInfo);	
	}	

	FTM_LIST_final(&xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_add
(
	FTM_EP_PTR 	pEPInfo
)
{
	FTM_EP_PTR	pTempInfo;
	FTM_EP_PTR	pNewInfo;
	FTM_RET	nRet;

	if (pEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTM_LIST_get(&xEPList, &pEPInfo->xEPID, (FTM_VOID_PTR _PTR_)&pTempInfo);
	if ((nRet == FTM_RET_OK) && (pTempInfo != NULL))
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;	
	}

	pNewInfo = (FTM_EP_PTR)malloc(sizeof(FTM_EP));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pEPInfo, sizeof(FTM_EP));

	nRet = FTM_LIST_append(&xEPList, pNewInfo);
	if (nRet != FTM_RET_OK)
	{
		free(pNewInfo);
		return	nRet;
	}

	FTDM_DBIF_EP_append(pEPInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_del
(
	FTM_EP_ID 			xEPID
)
{
	FTM_RET	nRet;
	FTM_EP_PTR	pEPInfo = NULL;

	nRet = FTM_LIST_get(&xEPList, &xEPID, (FTM_VOID_PTR _PTR_)&pEPInfo);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	nRet = FTDM_EP_del(xEPID);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_remove(&xEPList, pEPInfo);
	free(pEPInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_count
(
	FTM_EP_CLASS	xClass,
	FTM_ULONG_PTR	pulCount
)
{
	if (xClass == 0)
	{
		FTM_LIST_count(&xEPList, pulCount);
	}
	else
	{
		FTM_EP_PTR	pEPInfo;
		FTM_ULONG		ulCount = 0;

		FTM_LIST_iteratorStart(&xEPList);
		while(FTM_LIST_iteratorNext(&xEPList, (FTM_VOID_PTR _PTR_)&pEPInfo) == FTM_RET_OK)
		{
			if (xClass == (pEPInfo->xEPID & FTM_EP_CLASS_MASK))
			{
				ulCount++;			
			}
		}

		*pulCount = ulCount;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_get
(
	FTM_EP_ID				xEPID,
	FTM_EP_PTR	_PTR_ 	ppEPInfo
)
{
	FTM_RET	nRet;
	
	if (ppEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTM_LIST_get(&xEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEPInfo);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_getAt
(
	FTM_ULONG				nIndex,
	FTM_EP_PTR	_PTR_ 	ppEPInfo
)
{
	FTM_EP_PTR	pEPInfo;

	
	if (ppEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (FTM_LIST_getAt(&xEPList, nIndex, (FTM_VOID_PTR _PTR_)&pEPInfo) == FTM_RET_OK)
	{
		*ppEPInfo = pEPInfo;

		return	FTM_RET_OK;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTDM_EP_DATA_add
(
	FTM_EP_ID		xEPID,
	FTM_EP_DATA_PTR	pData
)
{
	if (pData == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTDM_DBIF_EP_DATA_append(xEPID, pData);
}

FTM_RET FTDM_EP_DATA_info
(
	FTM_EP_ID		xEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET	nRet;

	if ((pulBeginTime == NULL) || (pulEndTime == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_DBIF_EP_DATA_info(xEPID, pulBeginTime, pulEndTime);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	FTDM_DBIF_EP_DATA_count(xEPID, pulCount);
}

FTM_RET	FTDM_EP_DATA_get
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	return	FTDM_DBIF_EP_DATA_get(
				xEPID, 
				nStartIndex,
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_EP_DATA_getWithTime
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	return	FTDM_DBIF_EP_DATA_getWithTime(
				xEPID, 
				nBeginTime, 
				nEndTime, 
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_EP_DATA_del
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nIndex, 
	FTM_ULONG			nCount
) 
{
	return	FTDM_DBIF_EP_DATA_del( xEPID, nIndex, nCount);
}

FTM_RET	FTDM_EP_DATA_delWithTime
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime
) 
{
	return	FTDM_DBIF_EP_DATA_delWithTime( xEPID, nBeginTime, nEndTime);
}

FTM_RET	FTDM_EP_DATA_count
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG_PTR		pCount
) 
{
	return	FTDM_DBIF_EP_DATA_count(xEPID, pCount);
}

FTM_RET	FTDM_EP_DATA_countWithTime
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pCount
) 
{
	return	FTDM_DBIF_EP_DATA_countWithTime( xEPID, nBeginTime, nEndTime, pCount);
}

FTM_INT	FTDM_EPSeeker(const void *pElement, const void *pKey)
{
	FTM_EP_PTR	pEPInfo = (FTM_EP_PTR)pElement;
	FTM_EP_ID_PTR	pEPID = (FTM_EP_ID_PTR)pKey;

	if (pEPInfo->xEPID == *pEPID)
	{
		return	1;	
	}

	return	0;
}

