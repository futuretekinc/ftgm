#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftm_om.h"
#include "ftm_om_node.h"
#include "ftm_om_ep.h"
#include "ftm_om_node_snmpc.h"

static FTM_LIST_PTR	pNodeList = NULL;


FTM_VOID_PTR 	FTM_OM_NODE_process(FTM_VOID_PTR pData);
static FTM_RET	FTM_OM_NODE_lock(FTM_OM_NODE_PTR pNode);
static FTM_RET	FTM_OM_NODE_unlock(FTM_OM_NODE_PTR pNode);
static FTM_INT	FTM_OM_NODE_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTM_OM_NODE_comparator(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET FTM_OM_NODE_init(void)
{
	FTM_RET	xRet;

	pNodeList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pNodeList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_LIST_init(pNodeList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(pNodeList, FTM_OM_NODE_seek);
	FTM_LIST_setComparator(pNodeList, FTM_OM_NODE_comparator);

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_NODE_final(void)
{	
	ASSERT(pNodeList != NULL);
	
	FTM_OM_NODE_PTR	pNode = NULL;

	FTM_LIST_iteratorStart(pNodeList);
	while(FTM_LIST_iteratorNext(pNodeList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		FTM_OM_NODE_destroy(pNode);	
	}

	FTM_MEM_free(pNodeList);
	pNodeList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_NODE_create(FTM_NODE_PTR pInfo, FTM_OM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTM_OM_NODE_PTR	pNewNode;

	switch(pInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			xRet = FTM_OM_NODE_SNMPC_create(pInfo, &pNewNode);
		}
		break;

	default:
		{
			ERROR("pInfo->xType = %08lx", pInfo->xType);
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_LIST_append(pNodeList, pNewNode);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNewNode);	
	}

	FTM_ULONG	ulCount;
	FTM_LIST_count(pNodeList, &ulCount);
	pNewNode->xState = FTM_OM_NODE_STATE_CREATED;

	*ppNode = pNewNode;

	return	xRet;
}

FTM_RET	FTM_OM_NODE_destroy(FTM_OM_NODE_PTR	pNode)
{
	ASSERT(pNode != NULL);

	FTM_RET			xRet;
	FTM_OM_EP_PTR		pEP;

	FTM_OM_NODE_lock(pNode);

	FTM_LIST_iteratorStart(&pNode->xEPList);
	while(FTM_LIST_iteratorNext(&pNode->xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTM_OM_EP_detach(pEP);
	}
	FTM_LIST_final(&pNode->xEPList);

	FTM_OM_NODE_unlock(pNode);

	pthread_mutex_destroy(&pNode->xMutexLock);

	xRet = FTM_LIST_remove(pNodeList, (FTM_VOID_PTR)pNode);
	if (xRet == FTM_RET_OK)
	{
		TRACE("FTM_OM_NODE_destroy Success\n");
		FTM_MEM_free(pNode);
	}

	return	xRet;
}

FTM_RET FTM_OM_NODE_get(FTM_CHAR_PTR pDID, FTM_OM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTM_OM_NODE_PTR	pNode;
	
	xRet = FTM_LIST_get(pNodeList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET FTM_OM_NODE_getAt(FTM_ULONG ulIndex, FTM_OM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTM_OM_NODE_PTR	pNode;

	xRet = FTM_LIST_getAt(pNodeList, ulIndex, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET	FTM_OM_NODE_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(pNodeList, pulCount);
}

FTM_RET	FTM_OM_NODE_linkEP(FTM_OM_NODE_PTR pNode, FTM_OM_EP_PTR pEP)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	FTM_OM_NODE_lock(pNode);

	FTM_LIST_append(&pNode->xEPList, pEP);
	FTM_OM_EP_attach(pEP, pNode);

	FTM_OM_NODE_unlock(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_NODE_unlinkEP(FTM_OM_NODE_PTR pNode, FTM_OM_EP_PTR pEP)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	FTM_OM_NODE_lock(pNode);

	FTM_OM_EP_detach(pEP);
	FTM_LIST_remove(&pNode->xEPList, pEP);
	
	FTM_OM_NODE_unlock(pNode);

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_NODE_getEPData(FTM_OM_NODE_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->fGetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->fGetEPData(pNode, pEP, pData);
}

FTM_RET	FTM_OM_NODE_setEPData(FTM_OM_NODE_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->fSetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->fSetEPData(pNode, pEP, pData);
}

FTM_RET	FTM_OM_NODE_getEPCount(FTM_OM_NODE_PTR pNode, FTM_ULONG_PTR pulCount)
{
	ASSERT(pNode != NULL);
	ASSERT(pulCount != NULL);

	return FTM_LIST_count(&pNode->xEPList, pulCount);
}

FTM_RET	FTM_OM_NODE_getEP(FTM_OM_NODE_PTR pNode, FTM_EP_ID xEPID, FTM_OM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_get(&pNode->xEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTM_OM_NODE_getEPAt(FTM_OM_NODE_PTR pNode, FTM_ULONG ulIndex, FTM_OM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_getAt(&pNode->xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTM_OM_NODE_start(FTM_OM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xState != FTM_OM_NODE_STATE_CREATED)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	if (pNode->fStart == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->fStart(pNode);
}

FTM_RET FTM_OM_NODE_stop(FTM_OM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xState != FTM_OM_NODE_STATE_RUNNING)
	{
		return	FTM_RET_NOT_START;
	}


	return	pNode->fStop(pNode);
}


FTM_RET	FTM_OM_NODE_lock(FTM_OM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	pthread_mutex_lock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}


FTM_RET	FTM_OM_NODE_unlock(FTM_OM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_INT	FTM_OM_NODE_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTM_OM_NODE_PTR	pNode = (FTM_OM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(strcmp(pNode->xInfo.pDID, pDID) == 0);
}

FTM_INT	FTM_OM_NODE_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	FTM_OM_NODE_PTR	pNode1 = (FTM_OM_NODE_PTR)pElement1;
	FTM_OM_NODE_PTR	pNode2 = (FTM_OM_NODE_PTR)pElement2;

	if ((pElement1 == NULL) || (pElement2 == NULL))
	{
		return	0;	
	}

	return	strcmp(pNode1->xInfo.pDID, pNode2->xInfo.pDID);
}


FTM_CHAR_PTR	FTM_OM_NODE_stateToStr(FTM_OM_NODE_STATE xState)
{
	switch(xState)
	{
	case	FTM_OM_NODE_STATE_CREATED:	return	"CREATED";
	case	FTM_OM_NODE_STATE_INITIALIZED: return "INITIALIZED";
	case	FTM_OM_NODE_STATE_SYNCHRONIZED:	return	"SYNCHRONIZED";
	case	FTM_OM_NODE_STATE_PROCESS_INIT:	return	"PROCECSS_INIT";
	case	FTM_OM_NODE_STATE_RUN:			return	"RUN";
	case	FTM_OM_NODE_STATE_RUNNING:		return	"RUNNING";
	case	FTM_OM_NODE_STATE_PROCESS_FINISHED:return	"PROCESS_FINISHED";
	case	FTM_OM_NODE_STATE_FINISHED:		return	"FINISHED";
	case	FTM_OM_NODE_STATE_ABORT:			return	"ABORT";
	}

	return	"UNKNOWN";
}

