#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep.h"
#include "ftdm_ep_management.h"
#include "ftdm_sqlite.h"
#include "ftdm_log.h"

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
		FTM_LIST_setSeeker(pEPM->pList, FTDM_EP_seeker);
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
	FTDM_CONTEXT_PTR	pFTDM,
	FTDM_EPM_PTR _PTR_ ppEPM
)
{
	ASSERT(ppEPM != NULL);
	FTM_RET			xRet;
	FTDM_EPM_PTR	pEPM;

	pEPM = (FTDM_EPM_PTR)FTM_MEM_malloc(sizeof(FTDM_EPM));
	if (pEPM == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pEPM->pFTDM = pFTDM;

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

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
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

							xRet = FTDM_createEP(pEPM->pFTDM, &xInfo, &pEP);
							if (xRet != FTM_RET_OK)
							{
								ERROR2(xRet, "Can't not append EP[%s]\n", xInfo.pEPID);
							}
							else
							{
								xRet = FTDM_EPM_append(pEPM, pEP);
							}
							//FTDM_LOG_createEP(xInfo.pEPID, xRet);
						}
						else
						{
							ERROR2(xRet, "EP configuratoin load failed.\n");	
						}
					}
					else
					{
						ERROR2(xRet, "EP configuratoin load failed.\n");	
					}
				}
			}
		}
	}

	FTM_CONFIG_destroy(&pConfig);

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

				xRet = FTDM_createEP(pEPM->pFTDM, &xInfo, &pEP);	
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
#if 0
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;
	FTDM_EP_PTR	pEP;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF(pEPM->pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

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
		
			xRet = FTDM_DBIF_getEP(pDBIF, pEP->xInfo.pEPID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_createEP(pDBIF, &pEP->xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to save the new EP.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR2(xRet, "Failed to get EP information[%08x]\n", xRet);
		}
	}
#endif
	return	FTM_RET_OK;
}

