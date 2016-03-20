#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_sqlite.h"

static FTM_RET	FTDM_EP_createInternal(FTM_EP_PTR 	pInfo, FTM_BOOL	bNew);

FTM_RET	FTDM_EP_init
(
	FTM_VOID
)
{
	return	FTM_EP_init();
}

FTM_RET FTDM_EP_final
(
	FTM_VOID
)
{
	return	FTM_EP_final();
}

FTM_RET	FTDM_EP_loadFromFile
(
	FTM_CHAR_PTR	pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "ep", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xEPItemList;

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "eps", &xEPItemList);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG		ulItemCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xEPItemList, &ulItemCount);	
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG		i;
				FTM_CONFIG_ITEM	xItem;

				for(i = 0 ; i < ulItemCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xEPItemList, i, &xItem);
					if (xRet == FTM_RET_OK)
					{
						FTM_EP	xInfo;

						xRet = FTM_CONFIG_ITEM_getEP(&xItem, &xInfo);
						if (xRet == FTM_RET_OK)
						{
							xRet = FTDM_EP_createInternal(&xInfo, FTM_TRUE);
							if (xRet != FTM_RET_OK)
							{
								ERROR("Can't not append EP[%08x]\n", xInfo.xEPID);
							}
						}
						else
						{
							ERROR("EP configuratoin load failed.\n");	
						}
					}
					else
					{
						ERROR("EP configuratoin load failed.\n");	
					}
				}
			}
		}
	}

	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}


FTM_RET	FTDM_EP_loadFromDB
(
	FTM_VOID
)
{
	FTM_ULONG	nMaxEPCount = 0;

	if ((FTDM_DBIF_EP_count(&nMaxEPCount) == FTM_RET_OK) &&
		(nMaxEPCount > 0))
	{

		FTM_EP_PTR	pInfos;
		FTM_ULONG	nEPCount = 0;
		
		pInfos = (FTM_EP_PTR)calloc(nMaxEPCount, sizeof(FTM_EP));
		if (pInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_EP_getList(pInfos, nMaxEPCount, &nEPCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nEPCount ; i++)
			{
				FTDM_EP_createInternal(&pInfos[i], FTM_FALSE);
			}
		}

		free(pInfos);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_loadConfig
(
	FTDM_CFG_EP_PTR	pConfig
)
{
	FTM_ULONG	nMaxEPCount = 0;

	if (FTDM_CFG_EP_count(pConfig, &nMaxEPCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < nMaxEPCount ; i++)
		{
			FTM_EP	xInfo;

			if (FTDM_CFG_EP_getAt(pConfig, i, &xInfo) == FTM_RET_OK)
			{
				FTDM_EP_createInternal(&xInfo, FTM_TRUE);	
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_saveToDB
(
	FTM_VOID
)
{
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;
	FTM_EP_PTR	pEP;

	xRet = FTM_EP_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_EP_getAt(i, &pEP);
		if (xRet == FTM_RET_OK)
		{
			FTM_EP	xInfo;

			xRet = FTDM_DBIF_EP_get(pEP->xEPID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_EP_append(pEP);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Failed to save the new EP.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR("Failed to get EP information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_createInternal
(
	FTM_EP_PTR 	pInfo,
	FTM_BOOL	bNew
)
{
	ASSERT(pInfo != NULL);

	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTM_EP_get(pInfo->xEPID, (FTM_EP_PTR _PTR_)&pEP);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;	
	}
	
	pEP = (FTDM_EP_PTR)FTM_MEM_malloc(sizeof(FTDM_EP));
	if (pEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	memset(pEP, 0, sizeof(FTDM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	xRet = FTM_EP_append((FTM_EP_PTR)pEP);
	if (xRet == FTM_RET_OK)
	{
		if (bNew)
		{
			xRet = FTDM_DBIF_EP_append(pInfo);
			if (xRet != FTM_RET_OK)
			{
				FTM_EP_remove((FTM_EP_PTR)pEP);
				FTM_MEM_free(pEP);
			}
		}
		else
		{
			FTM_ULONG	ulCount = 0;

			xRet = FTDM_DBIF_EP_DATA_count(pInfo->xEPID, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				pEP->ulCount = ulCount;
			}
		}
	}
	else
	{
		FTM_MEM_free(pEP);
	}

	return	xRet;
}

FTM_RET	FTDM_EP_create
(
	FTM_EP_PTR 	pInfo
)
{
	return	FTDM_EP_createInternal(pInfo, FTM_TRUE);
}

FTM_RET	FTDM_EP_destroy
(
	FTM_EP_ID 			xEPID
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP = NULL;

	xRet = FTM_EP_get(xEPID, (FTM_EP_PTR _PTR_)&pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] not found.\n", xEPID);
		return	xRet;
	}

	xRet = FTM_EP_remove((FTM_EP_PTR)pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_EP_del(xEPID);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The EP[%08x] removed from database failed.\n", xEPID);
		}

		FTM_MEM_free(pEP);
	}
	else
	{
		ERROR("EP[%08x] destroy failed.\n", xEPID);
	
	}

	return	xRet;
}

FTM_RET	FTDM_EP_count
(
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;
	FTM_ULONG	i, ulTotalCount = 0, ulCount = 0;

	xRet = FTM_EP_count(&ulTotalCount);
	if (xType == 0)
	{
		*pulCount = ulTotalCount;
		return	xRet;
	}

	for(i = 0 ; i < ulTotalCount ; i++)
	{
		FTM_EP_getAt(i, (FTM_EP_PTR _PTR_)&pEP);
		if (xType == (pEP->xInfo.xEPID & FTM_EP_TYPE_MASK))
		{
			ulCount++;			
		}
	}

	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_get
(
	FTM_EP_ID			xEPID,
	FTDM_EP_PTR	_PTR_ 	ppEP
)
{
	ASSERT(ppEP != NULL);

	return	FTM_EP_get(xEPID, (FTM_EP_PTR _PTR_)ppEP);
}

FTM_RET	FTDM_EP_getAt
(
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR	_PTR_ 	ppEP
)
{
	ASSERT(ppEP != NULL);

	return	FTM_EP_getAt(ulIndex, (FTM_EP_PTR _PTR_)ppEP);
}


FTM_RET FTDM_EP_showList(FTM_VOID)
{
	FTM_ULONG	i, ulCount;

	MESSAGE("\n# EP Information\n");
	MESSAGE("\t%8s %16s %16s %8s %8s %8s %8s %16s %8s %16s %8s\n",
			"EPID", "TYPE", "NAME", "UNIT", "STATE", "INTERVAL", "TIMEOUT", "DID", "DEPID", "PID", "PEPID");
	if (FTM_EP_count(&ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTDM_EP_PTR	pEP;

			FTM_EP_getAt(i, (FTM_EP_PTR _PTR_)&pEP);
			MESSAGE("\t%08lx %16s %16s %8s ",
				pEP->xInfo.xEPID,
				FTM_EP_typeString(pEP->xInfo.xType),
				pEP->xInfo.pName,
				pEP->xInfo.pUnit);

			if (pEP->xInfo.bEnable)
			{
				MESSAGE("%8s ", "ENABLE");
			}
			else
			{
				MESSAGE("%8s ", "DISABLE");
			}

			MESSAGE("%8lu %8lu %16s %08lx %16s %08lx\n",
				pEP->xInfo.ulInterval,
				pEP->xInfo.ulTimeout,
				pEP->xInfo.pDID,
				pEP->xInfo.xDEPID,
				pEP->xInfo.pPID,
				pEP->xInfo.xPEPID);
		}
	}

	return	FTM_RET_OK;
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

