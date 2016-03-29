#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_node_snmpc.h"

static FTM_RET FTOM_NODEM_init
(
	FTOM_NODEM_PTR pNodeM
);

static FTM_RET FTOM_NODEM_final
(
	FTOM_NODEM_PTR pNodeM
);

static FTM_RET	FTOM_NODE_lock
(
	FTOM_NODE_PTR pNode
);

static FTM_RET	FTOM_NODE_unlock
(
	FTOM_NODE_PTR pNode
);

static FTM_INT	FTOM_NODE_seek
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_INT	FTOM_NODE_comparator
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

/********************************************************************************
 *	NODE Manager
 ********************************************************************************/

FTM_RET	FTOM_NODEM_create
(
	FTOM_NODEM_PTR _PTR_ ppNodeM
)
{
	ASSERT(ppNodeM != NULL);

	FTM_RET			xRet;
	FTOM_NODEM_PTR	pNodeM;

	pNodeM = (FTOM_NODEM_PTR)FTM_MEM_malloc(sizeof(FTOM_NODEM));
	if (pNodeM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_NODEM_init(pNodeM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNodeM);
		return	xRet;	
	}

	*ppNodeM = pNodeM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODEM_destroy
(
	FTOM_NODEM_PTR _PTR_ ppNodeM
)
{
	ASSERT(ppNodeM != NULL);
	
	FTM_RET	xRet;

	xRet = FTOM_NODEM_final(*ppNodeM);
	if (xRet != FTM_RET_OK)
	{
		WARN("Node manager finalize failed.");
	}

	FTM_MEM_free(*ppNodeM);

	*ppNodeM = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODEM_init
(
	FTOM_NODEM_PTR pNodeM
)
{
	ASSERT(pNodeM != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_create(&pNodeM->pList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(pNodeM->pList, FTOM_NODE_seek);
	FTM_LIST_setComparator(pNodeM->pList, FTOM_NODE_comparator);

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODEM_final
(
	FTOM_NODEM_PTR	pNodeM
)
{	
	ASSERT(pNodeM != NULL);
	
	FTOM_NODE_PTR	pNode = NULL;

	FTM_LIST_iteratorStart(pNodeM->pList);
	while(FTM_LIST_iteratorNext(pNodeM->pList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		FTOM_NODE_destroy(&pNode);	
	}

	FTM_LIST_destroy(pNodeM->pList);
	pNodeM->pList = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODEM_getNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR pDID, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	
	xRet = FTM_LIST_get(pNodeM->pList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET FTOM_NODEM_getNodeAt
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_ULONG ulIndex, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	xRet = FTM_LIST_getAt(pNodeM->pList, ulIndex, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET	FTOM_NODEM_countNode
(	
	FTOM_NODEM_PTR	pNodeM,
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTM_LIST_count(pNodeM->pList, pulCount);
}

/********************************************************************************
 *	NODE Manager
 ********************************************************************************/

FTM_RET	FTOM_NODE_create
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_NODE_PTR 	pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	switch(pInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			xRet = FTOM_NODE_SNMPC_create(pInfo, &pNode);
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

	pNode->pNodeM = pNodeM;
	pNode->xState = FTOM_NODE_STATE_CREATED;

	xRet = FTM_LIST_append(pNodeM->pList, pNode);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNode);	
	}

	*ppNode = pNode;

	return	xRet;
}

FTM_RET	FTOM_NODE_destroy
(
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;
	FTOM_NODE_PTR	pNode = *ppNode;

	FTOM_NODE_lock(pNode);

	FTM_LIST_iteratorStart(&pNode->xEPList);
	while(FTM_LIST_iteratorNext(&pNode->xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EP_detach(pEP);
	}
	FTM_LIST_final(&pNode->xEPList);

	FTOM_NODE_unlock(pNode);

	pthread_mutex_destroy(&pNode->xMutexLock);

	xRet = FTM_LIST_remove(pNode->pNodeM->pList, (FTM_VOID_PTR)pNode);
	if (xRet == FTM_RET_OK)
	{
		TRACE("FTOM_NODE_destroy Success\n");
		FTM_MEM_free(pNode);
	}

	*ppNode = pNode;

	return	xRet;
}

FTM_RET	FTOM_NODE_linkEP
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	FTOM_NODE_lock(pNode);

	FTM_LIST_append(&pNode->xEPList, pEP);
	FTOM_EP_attach(pEP, pNode);

	FTOM_NODE_unlock(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_unlinkEP
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	FTOM_NODE_lock(pNode);

	FTOM_EP_detach(pEP);
	FTM_LIST_remove(&pNode->xEPList, pEP);
	
	FTOM_NODE_unlock(pNode);

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_getEPData
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->fGetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->fGetEPData(pNode, pEP, pData);
}

FTM_RET	FTOM_NODE_setEPData
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->fSetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->fSetEPData(pNode, pEP, pData);
}

FTM_RET	FTOM_NODE_getEPCount
(
	FTOM_NODE_PTR pNode, 
	FTM_ULONG_PTR pulCount
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulCount != NULL);

	return FTM_LIST_count(&pNode->xEPList, pulCount);
}

FTM_RET	FTOM_NODE_getEP
(
	FTOM_NODE_PTR pNode, 
	FTM_EP_ID xEPID, 
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_get(&pNode->xEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_NODE_getEPAt
(
	FTOM_NODE_PTR pNode, 
	FTM_ULONG ulIndex, 
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_getAt(&pNode->xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_NODE_start
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->xState != FTOM_NODE_STATE_CREATED)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	if (pNode->fStart == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->fStart(pNode);
}

FTM_RET FTOM_NODE_stop
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->xState != FTOM_NODE_STATE_RUNNING)
	{
		return	FTM_RET_NOT_START;
	}


	return	pNode->fStop(pNode);
}


FTM_RET	FTOM_NODE_lock
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	pthread_mutex_lock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}


FTM_RET	FTOM_NODE_unlock
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_INT	FTOM_NODE_seek
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	FTOM_NODE_PTR	pNode = (FTOM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(strcmp(pNode->xInfo.pDID, pDID) == 0);
}

FTM_INT	FTOM_NODE_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	FTOM_NODE_PTR	pNode1 = (FTOM_NODE_PTR)pElement1;
	FTOM_NODE_PTR	pNode2 = (FTOM_NODE_PTR)pElement2;

	if ((pElement1 == NULL) || (pElement2 == NULL))
	{
		return	0;	
	}

	return	strcmp(pNode1->xInfo.pDID, pNode2->xInfo.pDID);
}


FTM_CHAR_PTR	FTOM_NODE_stateToStr
(
	FTOM_NODE_STATE xState
)
{
	switch(xState)
	{
	case	FTOM_NODE_STATE_CREATED:	return	"CREATED";
	case	FTOM_NODE_STATE_INITIALIZED: return "INITIALIZED";
	case	FTOM_NODE_STATE_SYNCHRONIZED:	return	"SYNCHRONIZED";
	case	FTOM_NODE_STATE_PROCESS_INIT:	return	"PROCECSS_INIT";
	case	FTOM_NODE_STATE_RUN:			return	"RUN";
	case	FTOM_NODE_STATE_RUNNING:		return	"RUNNING";
	case	FTOM_NODE_STATE_PROCESS_FINISHED:return	"PROCESS_FINISHED";
	case	FTOM_NODE_STATE_FINISHED:		return	"FINISHED";
	case	FTOM_NODE_STATE_ABORT:			return	"ABORT";
	}

	return	"UNKNOWN";
}

