#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftm_list.h"
#include "ftm_mem.h"

FTM_BOOL		FTM_NODE_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator);
static FTM_LIST_PTR	pNodeList = NULL;

FTM_RET	FTM_NODE_init(FTM_VOID)
{
	if (pNodeList != NULL)
	{
		ERROR("Node list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pNodeList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pNodeList == NULL)
	{
		ERROR("Node list is not allocated.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_LIST_init(pNodeList);
	FTM_LIST_setSeeker(pNodeList, FTM_NODE_seeker);

	TRACE("Node list is initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_final(FTM_VOID)
{
	FTM_NODE_PTR	pNode;

	if (pNodeList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_ULONG	ulCount;
	FTM_LIST_count(pNodeList, &ulCount);

 	FTM_LIST_iteratorStart(pNodeList);
  	while(FTM_LIST_iteratorNext(pNodeList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
   	{   
    	FTM_NODE_destroy(pNode);
	}

	FTM_LIST_final(pNodeList); 
	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_create(FTM_NODE_PTR pBase)
{
	ASSERT(pBase != NULL);

	FTM_RET			xRet;
	FTM_NODE_PTR	pNode;

	if (pNodeList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		FTM_NODE_init();
	}

	pNode = (FTM_NODE_PTR)FTM_MEM_malloc(sizeof(FTM_NODE));
	if (pNode == NULL)
	{
		ERROR("Can't not allocation Node.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pNode, pBase, sizeof(FTM_NODE));

	xRet = FTM_LIST_append(pNodeList, pNode);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNode);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_destroy(FTM_NODE_PTR pNode)
{
	ASSERT(pNodeList != NULL);
	ASSERT(pNode != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pNodeList, pNode);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pNode);	
	}

	return	xRet;
}

FTM_RET	FTM_NODE_count(FTM_ULONG_PTR pulCount)
{
	ASSERT(pNodeList != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pNodeList, pulCount);
}

FTM_RET	FTM_NODE_get(FTM_CHAR_PTR pDID, FTM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pNodeList != NULL);
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	return	FTM_LIST_get(pNodeList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)ppNode);
}

FTM_RET	FTM_NODE_getAt(FTM_ULONG ulIndex, FTM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pNodeList != NULL);
	ASSERT(ppNode != NULL);

	return	FTM_LIST_getAt(pNodeList, ulIndex, (FTM_VOID_PTR _PTR_)ppNode);
}

FTM_CHAR_PTR FTM_NODE_typeString(FTM_NODE_TYPE xType)
{
	switch(xType)
	{
	case	FTM_NODE_TYPE_SNMP: 				return	"SNMP";
	case	FTM_NODE_TYPE_MODBUS_OVER_TCP: 		return	"MODBUS/TCP";
	case	FTM_NODE_TYPE_MODBUS_OVER_SERIAL: 	return	"MODBUS/SERIAL";
	}

	return	"UNKNOWN";
}

FTM_BOOL		FTM_NODE_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_NODE_PTR	pNode = (FTM_NODE_PTR)pItem;
	FTM_CHAR_PTR	pDID  = (FTM_CHAR_PTR)pIndicator;

	return	(strcmp(pNode->pDID, pDID) == 0);
}
