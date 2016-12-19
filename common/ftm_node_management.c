#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftm_node_management.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_OBJECT

static FTM_BOOL		FTM_NODEM_seekNode(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator);

FTM_RET	FTM_NODEM_init(FTM_NODEM_PTR pNodeM)
{
	ASSERT(pNodeM != NULL);

	FTM_RET	xRet;

	if (pNodeM->pList != NULL)
	{
		WARN("Already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pNodeM->pList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	FTM_LIST_init(pNodeM->pList);
	FTM_LIST_setSeeker(pNodeM->pList, FTM_NODEM_seekNode);

	TRACE("Node management is initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_final(FTM_NODEM_PTR pNodeM)
{
	ASSERT(pNodeM != NULL);

	FTM_NODE_PTR	pNode;
	FTM_ULONG		ulCount;

	if (pNodeM->pList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_LIST_count(pNodeM->pList, &ulCount);

 	FTM_LIST_iteratorStart(pNodeM->pList);
  	while(FTM_LIST_iteratorNext(pNodeM->pList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
   	{   
    	FTM_NODE_destroy(&pNode);
	}

	FTM_LIST_destroy(pNodeM->pList);
	pNodeM->pList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODEM_append(FTM_NODEM_PTR pNodeM, FTM_NODE_PTR pNode)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pNode != NULL);

	FTM_RET		xRet;

	if (pNodeM->pList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;
	}

	xRet = FTM_LIST_append(pNodeM->pList, pNode);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODEM_remove(FTM_NODEM_PTR pNodeM, FTM_NODE_PTR pNode)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pNode != NULL);

	if (pNodeM->pList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;
	}

	return	FTM_LIST_remove(pNodeM->pList, pNode);
}

FTM_RET	FTM_NODEM_count(FTM_NODEM_PTR pNodeM, FTM_ULONG_PTR pulCount)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pulCount != NULL);

	if (pNodeM->pList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;
	}

	return	FTM_LIST_count(pNodeM->pList, pulCount);
}

FTM_RET	FTM_NODEM_get(FTM_NODEM_PTR pNodeM, FTM_CHAR_PTR pDID, FTM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	if (pNodeM->pList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;
	}

	return	FTM_LIST_get(pNodeM->pList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)ppNode);
}

FTM_RET	FTM_NODEM_getAt(FTM_NODEM_PTR pNodeM, FTM_ULONG ulIndex, FTM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pNodeM != NULL);
	ASSERT(ppNode != NULL);

	if (pNodeM->pList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;
	}

	return	FTM_LIST_getAt(pNodeM->pList, ulIndex, (FTM_VOID_PTR _PTR_)ppNode);
}

FTM_BOOL		FTM_NODEM_seekNode(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_NODE_PTR	pNode = (FTM_NODE_PTR)pItem;
	FTM_CHAR_PTR	pDID  = (FTM_CHAR_PTR)pIndicator;

	return	(strcmp(pNode->pDID, pDID) == 0);
}
