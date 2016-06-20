#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_node.h"
#include "ftdm_sqlite.h"
#include "ftm_mem.h"
#include "ftdm_log.h"

FTM_RET    FTDM_NODE_create
(   
	FTM_NODE_PTR	pInfo,
	FTDM_NODE_PTR _PTR_ ppNode
)   
{
	ASSERT(pInfo != NULL);

	FTM_RET    		xRet;
	FTDM_NODE_PTR	pNode;
	
	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_NODE_isStatic(pInfo);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_NODE_create(pInfo);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;
		}

	}

	pNode = (FTDM_NODE_PTR)FTM_MEM_malloc(sizeof(FTDM_NODE));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		xRet = FTM_NODE_isStatic(pInfo);
		if (xRet == FTM_RET_OK)
		{
			FTDM_DBIF_NODE_destroy(pInfo->pDID);
		}
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pNode, 0, sizeof(FTDM_NODE));
	memcpy(&pNode->xInfo, pInfo, sizeof(FTM_NODE));

	*ppNode = pNode;

	return  FTM_RET_OK;
}	  

FTM_RET 	FTDM_NODE_destroy
(
	FTDM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);
	FTM_RET	xRet;

	xRet = FTM_NODE_isStatic(&(*ppNode)->xInfo);
	if (xRet == FTM_RET_OK)
	{
		FTDM_DBIF_NODE_destroy((*ppNode)->xInfo.pDID);
	}

	FTM_MEM_free(*ppNode);
	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET 	FTDM_NODE_destroy2
(
	FTDM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);
	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_setInfo
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_NODE_isStatic(&pNode->xInfo);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_NODE_isStatic(pInfo);
		if (xRet != FTM_RET_OK)
		{
			xRet = FTDM_DBIF_NODE_destroy(pInfo->pDID);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Node[%s] can't change from static to dynamic.\n", pInfo->pDID);	
				return	xRet;
			}
		}
	}
	else
	{
		xRet = FTM_NODE_isStatic(pInfo);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTDM_DBIF_NODE_create(pInfo);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Node[%s] can't change from dynamic to static.\n", pInfo->pDID);	
				return	xRet;
			}
		}
	}
	
	memcpy(&pNode->xInfo, pInfo, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

