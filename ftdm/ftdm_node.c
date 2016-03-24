#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_node.h"
#include "ftdm_sqlite.h"
#include "ftm_mem.h"

FTM_RET	FTDM_NODE_init
(
	FTM_VOID
)
{
	return	FTM_NODE_init();
}

FTM_RET	FTDM_NODE_final
(
	FTM_VOID
)
{
	return	FTM_NODE_final();
}

FTM_RET	FTDM_NODE_loadConfig
(
	FTDM_CFG_NODE_PTR	pConfig
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_loadFromDB
(
	FTM_VOID
)
{
	FTM_RET		xRet;
	FTM_ULONG	nMaxNodeCount = 0;

	if ((FTDM_DBIF_NODE_count(&nMaxNodeCount) == FTM_RET_OK) &&
		(nMaxNodeCount > 0))
	{

		FTM_NODE_PTR	pInfos;
		FTM_ULONG		nNodeCount = 0;
		
		pInfos = (FTM_NODE_PTR)FTM_MEM_malloc(nMaxNodeCount * sizeof(FTM_NODE));
		if (pInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_NODE_getList(pInfos, nMaxNodeCount, &nNodeCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nNodeCount ; i++)
			{
				FTDM_NODE_PTR	pNode;

				pNode = (FTDM_NODE_PTR)FTM_MEM_malloc(sizeof(FTDM_NODE));
				if (pNode == NULL)
				{
					ERROR("Not enough memory!\n");
					break;	
				}

				memcpy(&pNode->xInfo, &pInfos[i], sizeof(FTM_NODE));

				xRet = FTM_NODE_append((FTM_NODE_PTR)pNode);
				if (xRet != FTM_RET_OK)
				{
					FTM_MEM_free(pNode);	
				}
			}
		}

		FTM_MEM_free(pInfos);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xNodeSection;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "node", &xNodeSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xNodeItemList;
		xRet = FTM_CONFIG_ITEM_getChildItem(&xNodeSection, "nodes", &xNodeItemList);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG		ulItemCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xNodeItemList, &ulItemCount);	
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG		i;
				FTM_CONFIG_ITEM	xNodeItem;

				for(i = 0 ; i < ulItemCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xNodeItemList, i, &xNodeItem);
					if (xRet == FTM_RET_OK)
					{
						FTM_NODE	xInfo;
						xRet = FTM_CONFIG_ITEM_getNode(&xNodeItem, &xInfo);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
				
						xRet = FTDM_NODE_create(&xInfo);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Cant not append NODE[%s]\n", xInfo.pDID);
						}
					}
				}
			}
		}
	}
	else
	{
		ERROR("NODE section not found!\n");	
	}

	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_saveToDB
(
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTDM_NODE_PTR	pNode;
	
	xRet = FTM_NODE_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't get NODE count.\n");
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTDM_NODE_getAt(i, &pNode);
		if (xRet == FTM_RET_OK)
		{
			FTM_NODE	xInfo;
			
			xRet = FTDM_DBIF_NODE_get(pNode->xInfo.pDID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				TRACE("NODE[%s]	save to DB.\n", pNode->xInfo.pDID);
				xRet = FTDM_DBIF_NODE_create(&pNode->xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Failed to save the new node.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR("Failed to get node information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET    FTDM_NODE_create
(   
	FTM_NODE_PTR	pInfo
)   
{
	ASSERT(pInfo != NULL);

	FTM_RET    		xRet;
	FTDM_NODE_PTR	pNode;
	
	if (FTDM_NODE_get(pInfo->pDID, &pNode) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	xRet = FTDM_DBIF_NODE_create(pInfo);
	if (xRet == FTM_RET_OK)
	{
		pNode = (FTDM_NODE_PTR)FTM_MEM_malloc(sizeof(FTDM_NODE));
		if (pNode == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_NODE_destroy(pInfo->pDID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
		memset(pNode, 0, sizeof(FTDM_NODE));
		memcpy(&pNode->xInfo, pInfo, sizeof(FTM_NODE));
		xRet = FTM_NODE_append((FTM_NODE_PTR)pNode);
		if (xRet != FTM_RET_OK)
		{
			FTDM_DBIF_NODE_destroy(pInfo->pDID);
			FTM_MEM_free(pNode);
		}
	}

	return  xRet;
}	  

FTM_RET 	FTDM_NODE_destroy
(
	FTM_CHAR_PTR	pDID
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	if (pDID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTDM_NODE_get(pDID, &pNode);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_NODE_destroy(pDID);
	FTM_NODE_remove((FTM_NODE_PTR)pNode);

	FTM_MEM_free(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_count
(
	FTM_ULONG_PTR	pnCount
)
{
	return	FTM_NODE_count(pnCount);
}

FTM_RET	FTDM_NODE_get
(
	FTM_CHAR_PTR		pDID,
	FTDM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	return FTM_NODE_get(pDID, (FTM_NODE_PTR _PTR_)ppNode);
}

FTM_RET	FTDM_NODE_getAt
(
	FTM_ULONG			nIndex,
	FTDM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(ppNode != NULL);

	return	FTM_NODE_getAt(nIndex, (FTM_NODE_PTR _PTR_)ppNode);
}

FTM_RET FTDM_NODE_isExist
(
 	FTM_CHAR_PTR				pDID,
	FTM_BOOL_PTR				pExist
)
{
	ASSERT(pDID != NULL);
	ASSERT(pExist != NULL);

	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_NODE_get(pDID, &pNode);
	*pExist = (xRet == FTM_RET_OK);

	return	xRet;
}

/****************************************************************
 *
 ****************************************************************/

FTM_RET	FTDM_NODE_showList
(
	FTM_VOID
)
{
	FTM_ULONG	i, ulCount;

	MESSAGE("\n# Node Information\n");
	MESSAGE("%16s %16s %16s %8s %8s %16s %16s %16s %16s\n",
			"DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");
	if (FTM_NODE_count(&ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTDM_NODE_PTR	pNode;

			FTDM_NODE_getAt(i, &pNode);
			MESSAGE("%16s %16s %16s %8d %8d %16s %16s %16s %16s\n",
				pNode->xInfo.pDID,
				FTM_NODE_typeString(pNode->xInfo.xType),
				pNode->xInfo.pLocation,
				pNode->xInfo.ulInterval,
				pNode->xInfo.ulTimeout,
				FTDM_CFG_SNMP_getVersionString(pNode->xInfo.xOption.xSNMP.ulVersion),
				pNode->xInfo.xOption.xSNMP.pURL,
				pNode->xInfo.xOption.xSNMP.pCommunity,
				pNode->xInfo.xOption.xSNMP.pMIB);
		}
	}

	return	FTM_RET_OK;
}
