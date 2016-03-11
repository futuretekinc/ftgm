#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_sqlite.h"

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
						FTM_EP	xEP;

						xRet = FTM_CONFIG_ITEM_getEP(&xItem, &xEP);
						if (xRet == FTM_RET_OK)
						{
							xRet = FTM_EP_createCopy(&xEP, NULL);
							if (xRet != FTM_RET_OK)
							{
								ERROR("Can't not append EP[%08x]\n", xEP.xEPID);
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

		FTM_EP_PTR	pEPs;
		FTM_ULONG	nEPCount = 0;
		
		pEPs = (FTM_EP_PTR)calloc(nMaxEPCount, sizeof(FTM_EP));
		if (pEPs == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_EP_getList(pEPs, nMaxEPCount, &nEPCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nEPCount ; i++)
			{
				FTM_EP_createCopy(&pEPs[i], NULL);
			}
		}

		free(pEPs);
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
			FTM_EP	xEP;

			if (FTDM_CFG_EP_getAt(pConfig, i, &xEP) == FTM_RET_OK)
			{
				FTM_EP_createCopy(&xEP, NULL);	
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_add
(
	FTM_EP_PTR 	pEP
)
{
	ASSERT(pEP != NULL);

	FTM_RET		xRet;
	FTM_EP_PTR	pNewEP;

	xRet = FTM_EP_createCopy(pEP, &pNewEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_EP_append(pNewEP);
		if (xRet != FTM_RET_OK)
		{
			FTM_EP_destroy(pNewEP);
		}
	}

	return	xRet;
}

FTM_RET	FTDM_EP_del
(
	FTM_EP_ID 			xEPID
)
{
	FTM_RET		xRet;
	FTM_EP_PTR	pEP = NULL;

	xRet = FTM_EP_get(xEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] not found.\n", xEPID);
		return	xRet;
	}

	xRet = FTM_EP_destroy(pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_EP_del(xEPID);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The EP[%08x] removed from database failed.\n", xEPID);
		
		}
	}
	else
	{
		ERROR("EP[%08x] destroy failed.\n", xEPID);
	
	}

	TRACE("FTDM_EP_del(%08x) = %d\n", xEPID, xRet);
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
	FTM_EP_PTR	pEP;
	FTM_ULONG	i, ulTotalCount = 0, ulCount = 0;

	xRet = FTM_EP_count(&ulTotalCount);
	if (xType == 0)
	{
		*pulCount = ulTotalCount;
		return	xRet;
	}

	for(i = 0 ; i < ulTotalCount ; i++)
	{
		FTM_EP_getAt(i, &pEP);
		if (xType == (pEP->xEPID & FTM_EP_TYPE_MASK))
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
	FTM_EP_PTR	_PTR_ 	ppEP
)
{
	ASSERT(ppEP != NULL);

	return	FTM_EP_get(xEPID, ppEP);
}

FTM_RET	FTDM_EP_getAt
(
	FTM_ULONG			ulIndex,
	FTM_EP_PTR	_PTR_ 	ppEP
)
{
	ASSERT(ppEP != NULL);

	return	FTM_EP_getAt(ulIndex, ppEP);
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
			FTM_EP_PTR	pEP;

			FTM_EP_getAt(i, &pEP);
			MESSAGE("\t%08lx %16s %16s %8s ",
				pEP->xEPID,
				FTM_EP_typeString(pEP->xType),
				pEP->pName,
				pEP->pUnit);

			if (pEP->bEnable)
			{
				MESSAGE("%8s ", "ENABLE");
			}
			else
			{
				MESSAGE("%8s ", "DISABLE");
			}

			MESSAGE("%8lu %8lu %16s %08lx %16s %08lx\n",
				pEP->ulInterval,
				pEP->ulTimeout,
				pEP->pDID,
				pEP->xDEPID,
				pEP->pPID,
				pEP->xPEPID);
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

