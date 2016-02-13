#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_node_info.h"
#include "ftdm_sqlite.h"
#include "ftm_mem.h"

static int FTDM_NODE_INFO_seeker
(
	const void *pElement, 
	const void *pKey
);

static FTM_RET	FTDM_NODE_INFO_CACHE_add
(
 	FTM_NODE_INFO_PTR pNodeInfo
);

static FTM_RET	FTDM_NODE_INFO_CACHE_del
(
 	FTM_NODE_INFO_PTR pNodeInfo
);

static FTM_RET	FTDM_NODE_INFO_CACHE_get
(
 	FTM_CHAR_PTR				pDID,
	FTM_NODE_INFO_PTR _PTR_ 	ppNodeInfo
);

static FTM_RET	FTDM_NODE_INFO_CACHE_isExist
(
 	FTM_CHAR_PTR				pDID,
	FTM_BOOL_PTR				pExist
);

static FTM_LIST	xNodeList;

FTM_RET	FTDM_NODE_INFO_init
(
	FTDM_CFG_NODE_PTR	pConfig
)
{
	FTM_ULONG	nMaxNodeCount = 0;

	FTM_LIST_init(&xNodeList);
	FTM_LIST_setSeeker(&xNodeList, FTDM_NODE_INFO_seeker);

	if ((FTDM_DBIF_NODE_INFO_count(&nMaxNodeCount) == FTM_RET_OK) &&
		(nMaxNodeCount > 0))
	{

		FTM_NODE_INFO_PTR	pNodeInfos;
		FTM_ULONG			nNodeCount = 0;
		
		pNodeInfos = (FTM_NODE_INFO_PTR)FTM_MEM_malloc(nMaxNodeCount * sizeof(FTM_NODE_INFO));
		if (pNodeInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_NODE_INFO_getList(pNodeInfos, nMaxNodeCount, &nNodeCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nNodeCount ; i++)
			{
				FTM_NODE_INFO_PTR pNodeInfo;
				
				pNodeInfo = (FTM_NODE_INFO_PTR)FTM_MEM_malloc(sizeof(FTM_NODE_INFO));
				if (pNodeInfo == NULL)
				{
					return	FTM_RET_NOT_ENOUGH_MEMORY;	
				}
			
				memcpy(pNodeInfo, &pNodeInfos[i], sizeof(FTM_NODE_INFO));

				FTDM_NODE_INFO_CACHE_add(pNodeInfo);
			}
		}

		free(pNodeInfos);
	}

	if (FTDM_CFG_NODE_INFO_count(pConfig, &nMaxNodeCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < nMaxNodeCount ; i++)
		{
			FTM_NODE_INFO	xNodeInfo;

			if (FTDM_CFG_NODE_INFO_getAt(pConfig, i, &xNodeInfo) == FTM_RET_OK)
			{
				FTM_BOOL	bExist;

				FTDM_NODE_INFO_CACHE_isExist(xNodeInfo.pDID, &bExist);
				if (!bExist)
				{
					FTDM_NODE_INFO_add(&xNodeInfo);	
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_INFO_final
(
	FTM_VOID
)
{
	FTM_ULONG	i, nCount;

	FTM_LIST_count(&xNodeList, &nCount);
	for(i = 0 ; i < nCount ; i++)
	{
		FTM_NODE_INFO_PTR	pNodeInfo;

		if (FTM_LIST_getAt(&xNodeList, i, (FTM_VOID_PTR _PTR_)&pNodeInfo) == FTM_RET_OK)
		{
			free(pNodeInfo);
		}
	}

	FTM_LIST_final(&xNodeList);

	return	FTM_RET_OK;
}

FTM_RET    FTDM_NODE_INFO_add
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

	if (FTDM_NODE_INFO_CACHE_get(pNodeInfo->pDID, &pNewNodeInfo) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	pNewNodeInfo = (FTM_NODE_INFO_PTR)FTM_MEM_malloc(sizeof(FTM_NODE_INFO));
	if (pNewNodeInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));

	nRet = FTDM_DBIF_NODE_INFO_append(pNewNodeInfo);
	if (nRet == FTM_RET_OK)
	{
		nRet = FTDM_NODE_INFO_CACHE_add(pNewNodeInfo);
	}

	if (nRet != FTM_RET_OK)
	{
		free(pNewNodeInfo);
	}

	return  nRet;
}	  

FTM_RET 	FTDM_NODE_INFO_del
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

	nRet = FTDM_NODE_INFO_CACHE_get(pDID, &pNodeInfo);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTDM_DBIF_NODE_INFO_del(pDID);
	FTDM_NODE_INFO_CACHE_del(pNodeInfo);
	free(pNodeInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_INFO_count
(
	FTM_ULONG_PTR	pnCount
)
{
	return	FTM_LIST_count(&xNodeList, pnCount);
}

FTM_RET	FTDM_NODE_INFO_get
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

	nRet = FTDM_NODE_INFO_CACHE_get(pDID, ppNodeInfo);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_INFO_getAt
(
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
)
{
	return	FTM_LIST_getAt(&xNodeList, nIndex, (FTM_VOID_PTR _PTR_)ppNodeInfo);
}

FTM_RET FTDM_NODE_INFO_isExist
(
 	FTM_CHAR_PTR				pDID,
	FTM_BOOL_PTR				pExist
)
{
	return	FTDM_NODE_INFO_CACHE_isExist(pDID, pExist);
}

int FTDM_NODE_INFO_seeker(const void *pElement, const void *pKey)
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

FTM_RET	FTDM_NODE_INFO_CACHE_add
(
 	FTM_NODE_INFO_PTR pNodeInfo
)
{
	FTM_LIST_append(&xNodeList, pNodeInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_INFO_CACHE_del
(
 	FTM_NODE_INFO_PTR pNodeInfo
)
{
	FTM_LIST_remove(&xNodeList, pNodeInfo->pDID);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_INFO_CACHE_get
(
 	FTM_CHAR_PTR				pDID,
	FTM_NODE_INFO_PTR _PTR_ 	ppNodeInfo
)
{
	return	FTM_LIST_get(&xNodeList, pDID, (FTM_VOID_PTR _PTR_)ppNodeInfo);
}

FTM_RET	FTDM_NODE_INFO_CACHE_isExist
(
 	FTM_CHAR_PTR				pDID,
	FTM_BOOL_PTR				pExist
)
{
	if (FTM_LIST_seek(&xNodeList, pDID) == FTM_RET_OK)
	{
		*pExist = FTM_TRUE;
	}
	else
	{
		*pExist = FTM_FALSE;
	}
	
	return	FTM_RET_OK;
}

