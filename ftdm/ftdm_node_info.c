#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_node_info.h"
#include "ftdm_sqlite.h"
#include "simclist.h"

static int FTDM_nodeSeeker
(
	const void *pElement, 
	const void *pKey
);

static FTM_RET	FTDM_CACHE_appendNodeInfo
(
 	FTM_NODE_INFO_PTR pNodeInfo
);

static FTM_RET	FTDM_CACHE_removeNodeInfo
(
 	FTM_NODE_INFO_PTR pNodeInfo
);

static FTM_RET	FTDM_CACHE_getNodeInfo
(
 	FTM_CHAR_PTR				pDID,
	FTM_NODE_INFO_PTR _PTR_ 	ppNodeInfo
);

static list_t	xNodeList;

FTM_RET	FTDM_initNodeInfo
(
 	FTM_VOID
)
{
	FTM_ULONG	nMaxNodeCount = 0;

	list_init(&xNodeList);
	list_attributes_seeker(&xNodeList, FTDM_nodeSeeker);

	if ((FTDM_DBIF_getNodeCount(&nMaxNodeCount) == FTM_RET_OK) &&
		(nMaxNodeCount > 0))
	{

		FTM_NODE_INFO_PTR	pNodeInfos;
		FTM_ULONG				nNodeCount = 0;
		
		pNodeInfos = (FTM_NODE_INFO_PTR)calloc(nMaxNodeCount, sizeof(FTM_NODE_INFO));
		if (pNodeInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_getNodeList(pNodeInfos, nMaxNodeCount, &nNodeCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nNodeCount ; i++)
			{
				FTM_NODE_INFO_PTR pNodeInfo;
				
				pNodeInfo = (FTM_NODE_INFO_PTR)malloc(sizeof(FTM_NODE_INFO));
				if (pNodeInfo == NULL)
				{
					return	FTM_RET_NOT_ENOUGH_MEMORY;	
				}
			
				memcpy(pNodeInfo, &pNodeInfos[i], sizeof(FTM_NODE_INFO));

				FTDM_CACHE_appendNodeInfo(pNodeInfo);
			}
		}

		free(pNodeInfos);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_finalNodeInfo
(
	FTM_VOID
)
{
	while(list_size(&xNodeList))
	{
		FTM_NODE_INFO_PTR pNodeInfo = (FTM_NODE_INFO_PTR)list_get_at(&xNodeList, 0);
		if (pNodeInfo != NULL)
		{
			list_delete_at(&xNodeList, 0);
			free(pNodeInfo);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET    FTDM_appendNodeInfo
(   
	FTM_NODE_INFO_PTR	pNodeInfo
)   
{
	FTM_RET    			nRet;
	FTM_NODE_INFO_PTR	pNewNodeInfo	;

	if (pNodeInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (FTDM_CACHE_getNodeInfo(pNodeInfo->pDID, &pNewNodeInfo) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	pNewNodeInfo = (FTM_NODE_INFO_PTR)malloc(sizeof(FTM_NODE_INFO));
	if (pNewNodeInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));

	nRet = FTDM_DBIF_insertNodeInfo(pNewNodeInfo);
	if (nRet == FTM_RET_OK)
	{
		nRet = FTDM_CACHE_appendNodeInfo(pNewNodeInfo);
	}

	if (nRet != FTM_RET_OK)
	{
		free(pNewNodeInfo);
	}

	return  nRet;
}	  

FTM_RET 	FTDM_removeNodeInfo
(
	FTM_CHAR_PTR			pDID
)
{
	FTM_RET				nRet;
	FTM_NODE_INFO_PTR	pNodeInfo;

	if (pDID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_CACHE_getNodeInfo(pDID, &pNodeInfo);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTDM_DBIF_removeNodeInfo(pDID);
	FTDM_CACHE_removeNodeInfo(pNodeInfo);
	free(pNodeInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getNodeInfoCount
(
	FTM_ULONG_PTR	pnCount
)
{
	*pnCount = list_size(&xNodeList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getNodeInfo
(
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
)
{
	FTM_RET				nRet;

	if ((pDID == NULL) || (ppNodeInfo == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_CACHE_getNodeInfo(pDID, ppNodeInfo);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getNodeInfoByIndex
(
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
)
{
	*ppNodeInfo = list_get_at(&xNodeList, nIndex);

	if (*ppNodeInfo != NULL)
	{
		return	FTM_RET_OK;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

int FTDM_nodeSeeker(const void *pElement, const void *pKey)
{
	FTM_NODE_INFO_PTR	pNodeInfo = (FTM_NODE_INFO_PTR)pElement;
	FTM_CHAR_PTR		pDID = (FTM_CHAR_PTR)pKey;

	if (strcmp(pNodeInfo->pDID, pDID) == 0)
	{
		return	1;	
	}
	return	0;
}
/****************************************************************
 *
 ****************************************************************/

FTM_RET	FTDM_CACHE_appendNodeInfo
(
 	FTM_NODE_INFO_PTR pNodeInfo
)
{
	list_append(&xNodeList, pNodeInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CACHE_removeNodeInfo
(
 	FTM_NODE_INFO_PTR pNodeInfo
)
{
	list_delete(&xNodeList, pNodeInfo->pDID);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CACHE_getNodeInfo
(
 	FTM_CHAR_PTR				pDID,
	FTM_NODE_INFO_PTR _PTR_ 	ppNodeInfo
)
{
	*ppNodeInfo = list_seek(&xNodeList, pDID);
	if (*ppNodeInfo != NULL)
	{
		return	FTM_RET_OK;	
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

