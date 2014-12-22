#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_ep.h"
#include "sqlite_if.h"
#include "ftdm_debug.h"

FTDM_RET	FTDM_insertEP
(
	FTDM_EP_PTR	pEP
);

static FTDM_EP	_header = 
{ 
	.pPrev = &_header, 
	.pNext = &_header,
	.xInfo = 
	{
		.xEPID	= 0x0,
		.xType 	= 0x0
	}
};

FTDM_RET	FTDM_initEP
(
	FTDM_VOID
)
{
	FTDM_ULONG	nMaxEPCount = 0;

	if (_header.pNext != &_header)
	{
		return	FTDM_RET_ALREADY_INITIALIZED;	
	}

	if ((FTDM_DBIF_getEPCount(&nMaxEPCount) == FTDM_RET_OK) &&
		(nMaxEPCount > 0))
	{

		FTDM_EP_INFO_PTR	pEPInfos;
		FTDM_ULONG			nEPCount = 0;
		
		pEPInfos = (FTDM_EP_INFO_PTR)calloc(nMaxEPCount, sizeof(FTDM_EP_INFO));
		if (pEPInfos == NULL)
		{
			return	FTDM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_getEPList(pEPInfos, nMaxEPCount, &nEPCount) == FTDM_RET_OK)
		{
			FTDM_INT	i;

			for(i = 0 ; i < nEPCount ; i++)
			{
				FTDM_EP_PTR pEP;
				
				pEP = (FTDM_EP_PTR)malloc(sizeof(FTDM_EP));
				if (pEP == NULL)
				{
					return	FTDM_RET_NOT_ENOUGH_MEMORY;	
				}
	
				memcpy(&pEP->xInfo, &pEPInfos[i], sizeof(FTDM_EP_INFO));

				FTDM_insertEP(pEP);
			}
		}

		free(pEPInfos);
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_insertEP
(
	FTDM_EP_PTR	pEP
)
{
	ASSERT(pEP != NULL);

	pEP->pNext = &_header;
	pEP->pPrev = _header.pPrev;

	_header.pPrev->pNext = pEP;
	_header.pPrev = pEP;

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_removeEP
(
 	FTDM_EP_PTR	pEP
)
{
	ASSERT(pEP != NULL);

	pEP->pNext->pPrev = pEP->pPrev;
	pEP->pPrev->pNext = pEP->pNext;

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getEP
(
 	FTDM_EP_ID			xEPID,
	FTDM_EP_PTR _PTR_	ppEP
)
{
	ASSERT(ppEP != NULL);

	FTDM_EP_PTR	pCurrent = _header.pNext;

	while(pCurrent != &_header)
	{
		if (xEPID == pCurrent->xInfo.xEPID)
		{
			*ppEP = pCurrent;
			return	FTDM_RET_OK;
		}

		pCurrent = pCurrent->pNext;
	}

	return	FTDM_RET_OBJECT_NOT_FOUND;
}

FTDM_RET	FTDM_createEP
(
	FTDM_EP_ID 			xEPID, 
	FTDM_EP_INFO_PTR 	pInfo
)
{
	FTDM_RET	nRet;
	FTDM_EP_PTR	pEP = NULL;

	if (pInfo == NULL)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_getEP(xEPID, &pEP);
	if ((nRet == FTDM_RET_OK) && (pEP != NULL))
	{
		return	FTDM_RET_ALREADY_EXIST_OBJECT;	
	}

	pEP = (FTDM_EP_PTR)malloc(sizeof(FTDM_EP));
	if (pEP == NULL)
	{
		return	FTDM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pEP->xInfo, pInfo, sizeof(FTDM_EP_INFO));
	pEP->xInfo.xEPID = xEPID;

	nRet = FTDM_insertEP(pEP);
	
	if (nRet != FTDM_RET_OK)
	{
		free(pEP);
		return	nRet;
	}

	FTDM_DBIF_insertEPInfo(pInfo);

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_destroyEP
(
	FTDM_EP_ID 			xEPID
)
{
	FTDM_RET	nRet;
	FTDM_EP_PTR	pEP = NULL;

	nRet = FTDM_getEP(xEPID, &pEP);
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_OBJECT_NOT_FOUND;	
	}

	nRet = FTDM_DBIF_removeEPInfo(xEPID);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;	
	}

	FTDM_removeEP(pEP);
	free(pEP);

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getEPCount
(
	FTDM_ULONG_PTR	pnCount
)
{
	FTDM_ULONG	nCount = 0;

	FTDM_EP_PTR	pCurrent = _header.pNext;
	
	while(pCurrent != &_header)
	{
		nCount++;
		pCurrent = pCurrent->pNext;
	}

	*pnCount = nCount;

	return	FTDM_RET_OK;
}
FTDM_RET	FTDM_getEPInfo
(
	FTDM_EP_ID			xEPID,
	FTDM_EP_INFO_PTR	pInfo
)
{
	FTDM_RET	nRet;
	FTDM_EP_PTR	pEP;
	
	if (pInfo == NULL)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_getEP(xEPID, &pEP);
	if (nRet != FTDM_RET_OK)
	{
		return	nRet;
	}

	memcpy(pInfo, &pEP->xInfo, sizeof(FTDM_EP_INFO));

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getEPInfoByIndex
(
	FTDM_ULONG			nIndex,
	FTDM_EP_INFO_PTR	pInfo
)
{
	FTDM_ULONG	nCount = 0;

	if (pInfo == NULL)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	FTDM_EP_PTR	pCurrent = _header.pNext;
	
	while(pCurrent != &_header)
	{
		if (nIndex == nCount++)
		{
			memcpy(pInfo, &pCurrent->xInfo, sizeof(FTDM_EP_INFO));
			return	FTDM_RET_OK;
		}

		pCurrent = pCurrent->pNext;
	}

	return	FTDM_RET_OBJECT_NOT_FOUND;
}

FTDM_RET	FTDM_appendEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG			nTime, 
	FTDM_ULONG 			nValue
)
{
	return	FTDM_DBIF_appendEPData(nTime, xEPID, nValue);
}

FTDM_RET	FTDM_getEPData
(
	FTDM_EP_ID_PTR		pEPID, 
	FTDM_ULONG			nEPIDCount,
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_EP_DATA_PTR 	pEPData,
	FTDM_ULONG			nMaxCount,
	FTDM_ULONG_PTR		pCount 
)
{
	return	FTDM_DBIF_getEPData(pEPID, nEPIDCount, nBeginTime, nEndTime, pEPData, pCount);
}

FTDM_RET	FTDM_removeEPData
(
	FTDM_EP_ID_PTR		pEPID, 
	FTDM_ULONG			nEPIDCount,
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime,
	FTDM_ULONG			nCount
) 
{
	return	FTDM_DBIF_removeEPData(pEPID, nEPIDCount, nBeginTime, nEndTime, nCount);
}

