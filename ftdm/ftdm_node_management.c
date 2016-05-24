#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_node.h"
#include "ftdm_node_management.h"
#include "ftdm_sqlite.h"
#include "ftm_mem.h"

static FTM_BOOL FTDM_NODEM_seekNode
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pKey
)
{
	FTDM_NODE_PTR	pNode = (FTDM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pNode->xInfo.pDID, pDID) == 0);
}

FTM_RET	FTDM_NODEM_create
(
	FTDM_NODEM_PTR _PTR_ ppNodeM
)
{
	ASSERT(ppNodeM != NULL);
	FTM_RET			xRet;	
	FTDM_NODEM_PTR	pNodeM;

	pNodeM = (FTDM_NODEM_PTR)FTM_MEM_malloc(sizeof(FTDM_NODEM));
	if (pNodeM == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTDM_NODEM_init(pNodeM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNodeM);
	}
	else
	{
		*ppNodeM = pNodeM;
	}

	return	xRet;
}

FTM_RET	FTDM_NODEM_destroy
(
	FTDM_NODEM_PTR _PTR_ ppNodeM
)
{
	ASSERT(ppNodeM != NULL);

	FTDM_NODEM_final(*ppNodeM);
	FTM_MEM_free(*ppNodeM);

	*ppNodeM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_init
(
	FTDM_NODEM_PTR	pNodeM
)
{
	ASSERT(pNodeM != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_LIST_create(&pNodeM->pList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(pNodeM->pList, FTDM_NODEM_seekNode);	
	}

	return	xRet;
}

FTM_RET	FTDM_NODEM_final
(
	FTDM_NODEM_PTR	pNodeM
)
{
	ASSERT(pNodeM != NULL);
	FTDM_NODE_PTR	pNode;

	FTM_LIST_iteratorStart(pNodeM->pList);
	while(FTM_LIST_iteratorNext(pNodeM->pList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		FTDM_NODE_destroy2(&pNode);	
	}

	FTM_LIST_destroy(pNodeM->pList);
	pNodeM->pList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_loadConfig
(
	FTDM_NODEM_PTR	pNodeM,
	FTDM_CFG_NODE_PTR	pConfig
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_loadFromDB
(
	FTDM_NODEM_PTR	pNodeM
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

				xRet = FTDM_NODE_create(&pInfos[i], &pNode);
				if (xRet == FTM_RET_OK)
				{
					xRet = FTM_LIST_append(pNodeM->pList, pNode);
					if (xRet != FTM_RET_OK)
					{
						FTDM_NODE_destroy(&pNode);	
					}
				}
			}
		}

		FTM_MEM_free(pInfos);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_loadFromFile
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR	pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xNodeSection;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Config initialize failed[%08x].\n", xRet);
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
						FTM_NODE		xInfo;
						FTDM_NODE_PTR	pNode;

						xRet = FTM_CONFIG_ITEM_getNode(&xNodeItem, &xInfo);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Node info read failed.[%08x]\n", xRet);
							continue;
						}
				
						xRet = FTDM_NODE_create(&xInfo, &pNode);
						if (xRet == FTM_RET_OK)
						{
							FTM_LIST_append(pNodeM->pList, pNode);
						}
						else
						{
							ERROR("Node[%s] creation failed[%08x].\n", xInfo.pDID, xRet);	
						
						}
					}
				}
			}
			else
			{
				ERROR("Node count read failed[%08x].\n", xRet);	
			}
		}
		else
		{
			TRACE("Node list not found!\n");	
		}
	}
	else
	{
		ERROR("NODE section not found!\n");	
	}

	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_saveToDB
(
	FTDM_NODEM_PTR	pNodeM
)
{
	ASSERT(pNodeM != NULL);

	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTDM_NODE_PTR	pNode;
	
	xRet = FTDM_NODEM_count(pNodeM, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't get NODE count.\n");
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTDM_NODEM_getAt(pNodeM, i, &pNode);
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

FTM_RET	FTDM_NODEM_count
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pNodeM->pList, pulCount);
}

FTM_RET	FTDM_NODEM_append
(
	FTDM_NODEM_PTR	pNodeM,
	FTDM_NODE_PTR	pNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pNode != NULL);

	return	FTM_LIST_append(pNodeM->pList, pNode);
}

FTM_RET	FTDM_NODEM_remove
(
	FTDM_NODEM_PTR	pNodeM,
	FTDM_NODE_PTR	pNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pNode != NULL);

	return	FTM_LIST_remove(pNodeM->pList, pNode);
}

FTM_RET	FTDM_NODEM_get
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR	pDID,
	FTDM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);
	
	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTM_LIST_get(pNodeM->pList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("List get error[%08x]\n", xRet);
		return	xRet;	
	}

	*ppNode = pNode;

	return FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_getAt
(	
	FTDM_NODEM_PTR	pNodeM,
	FTM_ULONG		nIndex,
	FTDM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTM_LIST_getAt(pNodeM->pList, nIndex, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*ppNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODEM_getDIDList
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pDIDs != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulCount, i;

	xRet = FTM_LIST_count(pNodeM->pList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulCount && (*pulCount < ulMaxCount) ; i++)
	{
		FTDM_NODE_PTR	pNode;

		xRet = FTM_LIST_getAt(pNodeM->pList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pNode);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		strcpy(pDIDs[(*pulCount)++], pNode->xInfo.pDID);
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_NODEM_isExist
(
	FTDM_NODEM_PTR	pNodeM,
 	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pExist
)
{
	ASSERT(pDID != NULL);
	ASSERT(pExist != NULL);

	FTM_RET			xRet;

	xRet = FTM_LIST_seek(pNodeM->pList, (FTM_VOID_PTR)pDID);
	*pExist = (xRet == FTM_RET_OK);

	return	xRet;
}

/****************************************************************
 *
 ****************************************************************/

FTM_RET	FTDM_NODEM_showList
(
	FTDM_NODEM_PTR	pNodeM
)
{
	ASSERT(pNodeM != NULL);

	FTM_ULONG	i, ulCount;

	MESSAGE("\n# Node Information\n");
	MESSAGE("\t%16s %16s %16s %8s %8s %16s %16s %16s %16s\n",
			"DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");
	if (FTM_LIST_count(pNodeM->pList, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTDM_NODE_PTR	pNode;

			FTDM_NODEM_getAt(pNodeM, i, &pNode);
			MESSAGE("\t%16s %16s %16s %8d %8d ",
				pNode->xInfo.pDID,
				FTM_NODE_typeString(pNode->xInfo.xType),
				pNode->xInfo.pLocation,
				pNode->xInfo.ulInterval,
				pNode->xInfo.ulTimeout);

			switch(pNode->xInfo.xType)
			{
			case	FTM_NODE_TYPE_SNMP:
				{
					MESSAGE(" %16s %16s %16s %16s\n",
						FTDM_CFG_SNMP_getVersionString(pNode->xInfo.xOption.xSNMP.ulVersion),
						pNode->xInfo.xOption.xSNMP.pURL,
						pNode->xInfo.xOption.xSNMP.pCommunity,
						pNode->xInfo.xOption.xSNMP.pMIB);
				}
				break;


			case	FTM_NODE_TYPE_MODBUS_OVER_TCP:
				{
					MESSAGE(" %16s %16s %16d %16d\n",
						pNode->xInfo.xOption.xMB.pModel,
						pNode->xInfo.xOption.xMB.pURL,
						pNode->xInfo.xOption.xMB.ulPort,
						pNode->xInfo.xOption.xMB.ulSlaveID);
				}
				break;
			}

		}
	}

	return	FTM_RET_OK;
}
