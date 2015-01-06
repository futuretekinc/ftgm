#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_ep.h"
#include "sqlite_if.h"
#include "ftm_debug.h"
#include "simclist.h"

static FTM_RET	FTDM_insertEP
(
	FTM_EP_INFO_PTR	pEPInfo
);

static FTM_RET	FTDM_removeEP
(
 	FTM_EP_INFO_PTR	pEPInfo
);

static FTM_INT	FTDM_EPSeeker
(
	const void *pElement, 
	const void *pKey)
;

static list_t	xEPList;

FTM_RET	FTDM_initEP
(
	FTM_VOID
)
{
	FTM_ULONG	nMaxEPCount = 0;

	if (list_init(&xEPList) < 0)
	{
		return	FTM_RET_INTERNAL_ERROR;	
	}

	list_attributes_seeker(&xEPList, FTDM_EPSeeker);

	if ((FTDM_DBIF_getEPCount(&nMaxEPCount) == FTM_RET_OK) &&
		(nMaxEPCount > 0))
	{

		FTM_EP_INFO_PTR	pEPInfos;
		FTM_ULONG			nEPCount = 0;
		
		pEPInfos = (FTM_EP_INFO_PTR)calloc(nMaxEPCount, sizeof(FTM_EP_INFO));
		if (pEPInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_getEPList(pEPInfos, nMaxEPCount, &nEPCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nEPCount ; i++)
			{
				FTM_EP_INFO_PTR	pEPInfo;
				pEPInfo = (FTM_EP_INFO_PTR)malloc(sizeof(FTM_EP_INFO));
				if (pEPInfo == NULL)
				{
					return	FTM_RET_NOT_ENOUGH_MEMORY;	
				}
	
				memcpy(pEPInfo, &pEPInfos[i], sizeof(FTM_EP_INFO));

				FTDM_insertEP(pEPInfo);
			}
		}

		free(pEPInfos);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_insertEP
(
	FTM_EP_INFO_PTR	pEPInfo
)
{
	if (pEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	list_append(&xEPList, pEPInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_removeEP
(
 	FTM_EP_INFO_PTR	pEPInfo
)
{
	if (pEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (list_delete(&xEPList, pEPInfo) == 0)
	{
		return	FTM_RET_OK;
	}
	else
	{
		return	FTM_RET_INTERNAL_ERROR;		
	}
}

FTM_RET	FTDM_getEP
(
 	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR _PTR_	ppEPInfo
)
{
	FTM_EP_INFO_PTR	pEPInfo = NULL;

	if (ppEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	pEPInfo = (FTM_EP_INFO_PTR)list_seek(&xEPList, &xEPID);
	if (pEPInfo != NULL)
	{
		*ppEPInfo = pEPInfo;	

		return	FTM_RET_OK;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTDM_createEP
(
	FTM_EP_INFO_PTR 	pEPInfo
)
{
	FTM_EP_INFO_PTR	pTempInfo;
	FTM_EP_INFO_PTR	pNewInfo;
	FTM_RET	nRet;

	if (pEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_getEP(pEPInfo->xEPID, &pTempInfo);
	if ((nRet == FTM_RET_OK) && (pTempInfo != NULL))
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;	
	}

	pNewInfo = (FTM_EP_INFO_PTR)malloc(sizeof(FTM_EP_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pEPInfo, sizeof(FTM_EP_INFO));

	nRet = FTDM_insertEP(pNewInfo);
	
	if (nRet != FTM_RET_OK)
	{
		free(pNewInfo);
		return	nRet;
	}

	FTDM_DBIF_insertEPInfo(pEPInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_destroyEP
(
	FTM_EPID 			xEPID
)
{
	FTM_RET	nRet;
	FTM_EP_INFO_PTR	pEPInfo = NULL;

	nRet = FTDM_getEP(xEPID, &pEPInfo);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	nRet = FTDM_DBIF_removeEPInfo(xEPID);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTDM_removeEP(pEPInfo);
	free(pEPInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getEPCount
(
	FTM_ULONG_PTR	pnCount
)
{
	*pnCount = list_size(&xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getEPInfo
(
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR	_PTR_ 	ppEPInfo
)
{
	FTM_RET	nRet;
	
	if (ppEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_getEP(xEPID, ppEPInfo);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getEPInfoByIndex
(
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR	_PTR_ 	ppEPInfo
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	
	if (ppEPInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pEPInfo = (FTM_EP_INFO_PTR)list_get_at(&xEPList, nIndex);
	if (pEPInfo != NULL)
	{
		*ppEPInfo = pEPInfo;

		return	FTM_RET_OK;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTDM_appendEPData
(
	FTM_EPID		xEPID,
	FTM_EP_DATA_PTR	pData
)
{
	if (pData == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTDM_DBIF_appendEPData(xEPID, pData);
}

FTM_RET	FTDM_getEPData
(
	FTM_EPID			xEPID, 
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	return	FTDM_DBIF_getEPData(
				xEPID, 
				nStartIndex,
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_getEPDataWithTime
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_EP_DATA_PTR 	pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
)
{
	return	FTDM_DBIF_getEPDataWithTime(
				xEPID, 
				nBeginTime, 
				nEndTime, 
				pEPData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_removeEPData
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nIndex, 
	FTM_ULONG			nCount
) 
{
	return	FTDM_DBIF_removeEPData(
				xEPID, 
				nIndex, 
				nCount);
}

FTM_RET	FTDM_removeEPDataWithTime
(
	FTM_EPID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime
) 
{
	return	FTDM_DBIF_removeEPDataWithTime(
				xEPID, 
				nBeginTime, 
				nEndTime);
}

FTM_INT	FTDM_EPSeeker(const void *pElement, const void *pKey)
{
	FTM_EP_INFO_PTR	pEPInfo = (FTM_EP_INFO_PTR)pElement;
	FTM_EPID_PTR	pEPID = (FTM_EPID_PTR)pKey;

	if (pEPInfo->xEPID == *pEPID)
	{
		return	1;	
	}

	return	0;
}
