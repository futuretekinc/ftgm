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
	FTM_ULONG	nMaxNodeCount = 0;

	if ((FTDM_DBIF_NODE_count(&nMaxNodeCount) == FTM_RET_OK) &&
		(nMaxNodeCount > 0))
	{

		FTM_NODE_PTR	pNodes;
		FTM_ULONG		nNodeCount = 0;
		
		pNodes = (FTM_NODE_PTR)FTM_MEM_malloc(nMaxNodeCount * sizeof(FTM_NODE));
		if (pNodes == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_NODE_getList(pNodes, nMaxNodeCount, &nNodeCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nNodeCount ; i++)
			{
				FTM_NODE_createCopy(&pNodes[i], NULL);
			}
		}

		free(pNodes);
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
						FTM_NODE	xNode;
						xRet = FTM_CONFIG_ITEM_getNode(&xNodeItem, &xNode);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
				
						xRet = FTM_NODE_createCopy(&xNode, NULL);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Cant not append NODE[%s]\n", xNode.pDID);
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
	FTM_NODE_PTR	pNode;
	
	xRet = FTM_NODE_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_NODE_getAt(i, &pNode);
		if (xRet == FTM_RET_OK)
		{
			FTM_NODE	xInfo;

			xRet = FTDM_DBIF_NODE_get(pNode->pDID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_NODE_append(pNode);	
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

FTM_RET    FTDM_NODE_add
(   
	FTM_NODE_PTR	pNode
)   
{
	ASSERT(pNode != NULL);

	FTM_RET    		nRet;
	FTM_NODE_PTR	pTempNode;
	
	if (FTM_NODE_get(pNode->pDID, &pTempNode) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	nRet = FTDM_DBIF_NODE_append(pNode);
	if (nRet == FTM_RET_OK)
	{
		nRet = FTM_NODE_createCopy(pNode, NULL);
	}

	return  nRet;
}	  

FTM_RET 	FTDM_NODE_del
(
	FTM_CHAR_PTR	pDID
)
{
	FTM_RET			nRet;
	FTM_NODE_PTR	pNode;

	if (pDID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_NODE_get(pDID, &pNode);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTDM_DBIF_NODE_del(pDID);
	FTM_NODE_destroy(pNode);

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
	FTM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	return FTM_NODE_get(pDID, ppNode);
}

FTM_RET	FTDM_NODE_getAt
(
	FTM_ULONG			nIndex,
	FTM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(ppNode != NULL);

	return	FTM_NODE_getAt(nIndex, ppNode);
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
	FTM_NODE_PTR	pNode;

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
	MESSAGE("\t%16s %16s %16s %8s %8s %16s %16s %16s %16s\n",
			"DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");
	if (FTM_NODE_count(&ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_NODE_PTR	pNode;

			FTM_NODE_getAt(i, &pNode);
			MESSAGE("\t%16s %16s %16s %8d %8d %16s %16s %16s %16s\n",
				pNode->pDID,
				FTM_NODE_typeString(pNode->xType),
				pNode->pLocation,
				pNode->ulInterval,
				pNode->ulTimeout,
				FTDM_CFG_SNMP_getVersionString(pNode->xOption.xSNMP.ulVersion),
				pNode->xOption.xSNMP.pURL,
				pNode->xOption.xSNMP.pCommunity,
				pNode->xOption.xSNMP.pMIB);
		}
	}

	return	FTM_RET_OK;
}
