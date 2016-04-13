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
	FTM_RET	xRet;

	if (pNodeList != NULL)
	{
		ERROR("Node list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pNodeList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
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

	FTM_LIST_destroy(pNodeList);
	pNodeList = NULL;

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

FTM_RET	FTM_NODE_append(FTM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_RET		xRet;

	if (pNodeList == NULL)
	{
		ERROR("Node list is not initialized.\n");
		FTM_NODE_init();
	}

	xRet = FTM_LIST_append(pNodeList, pNode);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_setDefault(FTM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	memset(pNode, 0, sizeof(FTM_NODE));
	
	pNode->xType		= FTM_NODE_TYPE_SNMP;
	pNode->ulInterval	= 60;
	pNode->ulTimeout	= 10;
	pNode->xOption.xSNMP.ulVersion = 1;
	strcpy(pNode->xOption.xSNMP.pURL, "127.0.0.1");
	strcpy(pNode->xOption.xSNMP.pCommunity, "public");
	pNode->xOption.xSNMP.ulMaxRetryCount = 3;

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_remove(FTM_NODE_PTR pNODE)
{
	ASSERT(pNODE != NULL);

	return	FTM_LIST_remove(pNodeList, pNODE);
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

FTM_RET	FTM_NODE_isValid(FTM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_RET	xRet;

	xRet = FTM_isValidDID(pNode->pDID);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_isValidLocation(pNode->pLocation);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_isValidInterval(pNode->ulInterval);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet= FTM_NODE_isValidType(pNode->xType);
	if(xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_NODE_isValidTimeout(pNode, pNode->ulTimeout);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	
	xRet = FTM_NODE_isValidSNMPOpt(pNode, &pNode->xOption.xSNMP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_NODE_isValidType(FTM_NODE_TYPE xType)
{
	if ((xType == FTM_NODE_TYPE_SNMP) ||
		(xType == FTM_NODE_TYPE_MODBUS_OVER_TCP) ||
		(xType == FTM_NODE_TYPE_MODBUS_OVER_SERIAL))
	{
		return	FTM_RET_OK;	
	}

	return	FTM_RET_INVALID_TYPE;
}

FTM_RET	FTM_NODE_isValidSNMPOpt(FTM_NODE_PTR pNode, FTM_NODE_OPT_SNMP_PTR pOpts)
{
	ASSERT(pNode != NULL);
	ASSERT(pOpts != NULL);
	
	FTM_RET	xRet;

	if (pOpts->ulVersion >= 3)
	{
		return	FTM_RET_INVALID_VERSION;	
	}

	xRet = FTM_isValidURL(pOpts->pURL);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_isValidCommunity(pOpts->pCommunity);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_isValidTimeout(FTM_NODE_PTR pNode, FTM_ULONG	ulTimeout)
{
	ASSERT(pNode != NULL);

	if (pNode->ulInterval < ulTimeout)
	{
		return	FTM_RET_INVALID_TIMEOUT;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_isStatic(FTM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xFlags & FTM_NODE_FLAG_STATIC)
	{
		return	FTM_RET_TRUE;
	}

	return	FTM_RET_FALSE;
}

FTM_RET	FTM_NODE_setDID(FTM_NODE_PTR pNode, FTM_CHAR_PTR pDID)
{
	ASSERT(pNode != NULL);

	if (pDID == NULL)
	{
		memset(pNode->pDID, 0, sizeof(pNode->pDID));
	}

	strcpy(pNode->pDID, pDID);

	return	FTM_RET_OK;
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
