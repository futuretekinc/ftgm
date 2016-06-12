#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_ep_management.h"
#include "ftdm_sqlite.h"

static 
FTM_BOOL	FTDM_EPM_seekEP
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pKey
);

FTM_RET	FTDM_EPM_init
(
	FTDM_EPM_PTR	pEPM
)
{
	ASSERT(pEPM != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_create(&pEPM->pList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(pEPM->pList, FTDM_EPM_seekEP);
	}
	return	xRet;
}

FTM_RET FTDM_EPM_final
(
	FTDM_EPM_PTR	pEPM
)
{
	ASSERT(pEPM != NULL);
	FTDM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPM->pList);
	while(FTM_LIST_iteratorNext(pEPM->pList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTDM_EP_destroy2(&pEP);
	}

	FTM_LIST_destroy(pEPM->pList);
	pEPM->pList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EPM_create
(
	FTDM_EPM_PTR _PTR_ ppEPM
)
{
	ASSERT(ppEPM != NULL);
	FTM_RET			xRet;
	FTDM_EPM_PTR	pEPM;

	pEPM = (FTDM_EPM_PTR)FTM_MEM_malloc(sizeof(FTDM_EPM));
	if (pEPM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTDM_EPM_init(pEPM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEPM);
		return	xRet;
	}

	*ppEPM = pEPM;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EPM_destroy
(
	FTDM_EPM_PTR _PTR_ ppEPM
)
{
	ASSERT(ppEPM != NULL);

	FTDM_EPM_final(*ppEPM);

	FTM_MEM_free(*ppEPM);

	*ppEPM = NULL;

	return	FTM_RET_OK;
}


FTM_RET	FTDM_EPM_loadFromFile
(
	FTDM_EPM_PTR	pEPM,
	FTM_CHAR_PTR	pFileName
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_create(pFileName, &pConfig);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(pConfig, "ep", &xSection);
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
							FTDM_EP_PTR	pEP;

							xRet = FTDM_EP_create(&xInfo, &pEP);
							if (xRet != FTM_RET_OK)
							{
								ERROR("Can't not append EP[%s]\n", xInfo.pEPID);
							}
							else
							{
								FTDM_EPM_append(pEPM, pEP);
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

	FTM_CONFIG_destroy(&pConfig);

	return	FTM_RET_OK;
}


FTM_RET	FTDM_EPM_loadFromDB
(
	FTDM_EPM_PTR	pEPM
)
{
	FTM_RET		xRet;
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
				FTDM_EP_PTR	pEP;

				xRet = FTDM_EP_create(&pInfos[i], &pEP);
				if (xRet == FTM_RET_OK)
				{
					xRet = FTDM_EPM_append(pEPM, pEP);
					if (xRet != FTM_RET_OK)
					{
						WARN("EP append failed[%08x]\n", xRet);	
					}
				}
			}
		}

		free(pInfos);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EPM_loadConfig
(
	FTDM_EPM_PTR	pEPM,
	FTDM_CFG_EP_PTR	pConfig
)
{
	FTM_RET		xRet;
	FTM_ULONG	nMaxEPCount = 0;

	if (FTDM_CFG_EP_count(pConfig, &nMaxEPCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < nMaxEPCount ; i++)
		{
			FTM_EP	xInfo;

			if (FTDM_CFG_EP_getAt(pConfig, i, &xInfo) == FTM_RET_OK)
			{
				FTDM_EP_PTR	pEP;

				xRet = FTDM_EP_create(&xInfo, &pEP);	
				if (xRet == FTM_RET_OK)
				{
					FTDM_EPM_append(pEPM, pEP);
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EPM_saveToDB
(
	FTDM_EPM_PTR	pEPM
)
{
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_count(pEPM, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTDM_EPM_getAt(pEPM, i, &pEP);
		if (xRet == FTM_RET_OK)
		{
			FTM_EP	xInfo;
		
			xRet = FTDM_DBIF_EP_get(pEP->xInfo.pEPID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_EP_append(&pEP->xInfo);	
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

FTM_RET	FTDM_EPM_count
(
	FTDM_EPM_PTR	pEPM,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;
	FTM_ULONG	i, ulTotalCount = 0, ulCount = 0;

	xRet = FTM_LIST_count(pEPM->pList, &ulTotalCount);
	if (xType == 0)
	{
		*pulCount = ulTotalCount;
		return	xRet;
	}

	for(i = 0 ; i < ulTotalCount ; i++)
	{
		FTDM_EPM_getAt(pEPM, i, &pEP);
		if (xType == (pEP->xInfo.xType & FTM_EP_TYPE_MASK))
		{
			ulCount++;			
		}
	}

	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET FTDM_EPM_append
(
	FTDM_EPM_PTR		pEPM,
	FTDM_EP_PTR			pEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	return	FTM_LIST_append(pEPM->pList, pEP);
}

FTM_RET FTDM_EPM_remove
(
	FTDM_EPM_PTR		pEPM,
	FTDM_EP_PTR			pEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	return	FTM_LIST_remove(pEPM->pList, pEP);
}


FTM_RET	FTDM_EPM_get
(
	FTDM_EPM_PTR		pEPM,
	FTM_CHAR_PTR		pEPID,
	FTDM_EP_PTR	_PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_get(pEPM->pList, pEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTDM_EPM_getAt
(
	FTDM_EPM_PTR		pEPM,
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR	_PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_getAt(pEPM->pList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTDM_EPM_getEPIDList
(
	FTDM_EPM_PTR		pEPM,
	FTM_EPID_PTR		pEPIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEPIDs != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTDM_EPM_count(pEPM, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		FTDM_EP_PTR	pEP;

		xRet = FTDM_EPM_getAt(pEPM, ulIndex + i, &pEP);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		strcpy(pEPIDs[(*pulCount)++], pEP->xInfo.pEPID);
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_EPM_showList
(
	FTDM_EPM_PTR	pEPM
)
{
	FTM_ULONG	i, ulCount;

	MESSAGE("\n# EP Information\n");
	MESSAGE("\t%16s %16s %16s %8s %8s %8s %8s %8s %16s %8s %8s\n",
			"EPID", "TYPE", "NAME", "UNIT", "STATE", "UPDATE", 
			"REPORT", "TIMEOUT", "DID", "COUNT", "TIME");
	if (FTDM_EPM_count(pEPM, 0, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTDM_EP_PTR	pEP;

			FTDM_EPM_getAt(pEPM, i, &pEP);
			MESSAGE("\t%16s %16s %16s %8s ",
				pEP->xInfo.pEPID,
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

			MESSAGE("%8lu %8lu %8lu %16s %8d %10d %10d\n",
				pEP->xInfo.ulUpdateInterval,
				pEP->xInfo.ulReportInterval,
				pEP->xInfo.ulTimeout,
				pEP->xInfo.pDID,
				pEP->ulCount,
				pEP->ulFirstTime,
				pEP->ulLastTime);
		}
	}

	return	FTM_RET_OK;
}


FTM_BOOL	FTDM_EPM_seekEP
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pKey
)
{
	ASSERT(pElement != NULL);
	ASSERT(pKey != NULL);

	FTDM_EP_PTR		pEP = (FTDM_EP_PTR)pElement;
	FTM_CHAR_PTR	pEPID = (FTM_CHAR_PTR)pKey;

	return	strcmp(pEP->xInfo.pEPID, pEPID) == 0;

}
