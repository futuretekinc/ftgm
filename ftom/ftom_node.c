#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_node.h"
#include "ftom_node_management.h"
#include "ftom_ep.h"
#include "ftom_node_snmpc.h"

static FTM_RET	FTOM_NODE_lock
(
	FTOM_NODE_PTR pNode
);

static FTM_RET	FTOM_NODE_unlock
(
	FTOM_NODE_PTR pNode
);

static FTM_VOID_PTR FTOM_NODE_process
(
	FTM_VOID_PTR pData
);

/********************************************************************************
 *	NODE Manager
 ********************************************************************************/

FTM_RET	FTOM_NODE_create
(
	FTM_NODE_PTR 	pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
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

	pNode->xState = FTOM_NODE_STATE_CREATED;

	FTOM_NODE_init(pNode, pInfo);

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

	xRet = FTOM_NODE_final(*ppNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node finalize failed.[%08x]\n", xRet);
	}

	switch((*ppNode)->xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			xRet = FTOM_NODE_SNMPC_destroy((FTOM_NODE_SNMPC_PTR _PTR_)ppNode);
		}
		break;

	default:
		{
			ERROR("pInfo->xType = %08lx", (*ppNode)->xInfo.xType);
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_init
(
	FTOM_NODE_PTR 	pNode,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;

	memcpy(&pNode->xInfo, pInfo, sizeof(FTM_NODE));
	xRet = FTM_LIST_init(&pNode->xEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("List initialize failed[%08x].\n", xRet);
		return	xRet;	
	}
	
	xRet = FTOM_MSGQ_init(&pNode->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message queue initialize failed[%08x].\n", xRet);
		return	xRet;	
	}

	pthread_mutex_init(&pNode->xMutexLock, NULL);
	
	pNode->bStop = FTM_TRUE;
	pNode->xState = FTOM_NODE_STATE_INITIALIZED;
	pNode->ulRetry = 10;
	pNode->xTimeout = 10;

	if (pNode->xDescript.fInit != NULL)
	{
		pNode->xDescript.fInit(pNode, pInfo);
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_final
(
	FTOM_NODE_PTR 	pNode
)
{
	ASSERT(pNode != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;

	FTOM_NODE_lock(pNode);

	FTM_LIST_iteratorStart(&pNode->xEPList);
	while(FTM_LIST_iteratorNext(&pNode->xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EP_detach(pEP);
	}
	FTM_LIST_final(&pNode->xEPList);

	FTOM_NODE_unlock(pNode);

	pthread_mutex_destroy(&pNode->xMutexLock);

	if (pNode->pNodeM != NULL)
	{
		xRet = FTOM_NODEM_detachNode(pNode->pNodeM, pNode);
		if (xRet != FTM_RET_OK)
		{
			WARN("Node detach failed[%08x].\n", xRet);	
		}
	}

	pNode->xState = FTOM_NODE_STATE_FINALIZED;

	return	FTM_RET_OK;
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

	if (pNode->xDescript.fGetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->xDescript.fGetEPData(pNode, pEP, pData);
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

	if (pNode->xDescript.fSetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->xDescript.fSetEPData(pNode, pEP, pData);
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
	FTM_CHAR_PTR	pEPID,
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_get(&pNode->xEPList, pEPID, (FTM_VOID_PTR _PTR_)ppEP);
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

	FTM_INT	nRet;

	if (!pNode->bStop)
	{
		ERROR("Node is already started.\n");
		return	FTM_RET_ALREADY_STARTED;
	}

	if (pNode->xDescript.fPrestart != NULL)
	{
		pNode->xDescript.fPrestart(pNode);
	}

	if (pNode->xDescript.fProcess != NULL)
	{
		nRet = pthread_create(&pNode->xThread, NULL, pNode->xDescript.fProcess, pNode);
	}
	else
	{
		TRACE("Run default process\n");
		nRet = pthread_create(&pNode->xThread, NULL, FTOM_NODE_process, pNode);
	}

	if (nRet != 0)
	{
		ERROR("Node start failed.[%d]\n", nRet);
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	if (pNode->xDescript.fPoststart != NULL)
	{
		pNode->xDescript.fPoststart(pNode);
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_stop
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);
	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg = NULL;

	if (pNode->bStop)
	{
		ERROR("Node[%s] not started.\n", pNode->xInfo.pDID);
		return	FTM_RET_NOT_START;
	}

	if (pNode->xDescript.fPrestop != NULL)
	{
		pNode->xDescript.fPrestop(pNode);
	}

	xRet = FTOM_MSG_createQuit(&pMsg);
	if (xRet == FTM_RET_OK)
	{
		ERROR("Can't create quit message!\n");
	}
	else
	{
		xRet = FTOM_MSGQ_push(&pNode->xMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			FTOM_MSG_destroy(&pMsg);	
		}
	}

	if (pMsg == NULL)
	{
		FTM_ULONG	ulCount;

		xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < ulCount ; i++)
			{
				FTOM_EP_PTR	pEP;

				xRet = FTM_LIST_getAt(&pNode->xEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
				if (xRet == FTM_RET_OK)
				{
					FTOM_EP_stop(pEP, FTM_TRUE);
				}
			}
		}

		pthread_cancel(pNode->xThread);
	}
	
	pthread_join(pNode->xThread, NULL);
	pNode->xThread = 0;

	if (pNode->xDescript.fPoststop != NULL)
	{
		pNode->xDescript.fPoststop(pNode);
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_NODE_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR pNode = (FTOM_NODE_PTR)pData;
	FTOM_MSG_PTR	pMsg;
	FTM_TIMER		xReportTimer;
	FTM_ULONG		ulCount;

	pNode->xState = FTOM_NODE_STATE_RUN;

	TRACE("Node[%s] start.\n", pNode->xInfo.pDID);

	xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_INT	i;

		for(i = 0 ; i < ulCount ; i++)
		{
			FTOM_EP_PTR	pEP;

			xRet = FTM_LIST_getAt(&pNode->xEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
			if (xRet == FTM_RET_OK)
			{
				FTOM_EP_start(pEP);
			}
		}
	}

	pNode->bStop = FTM_FALSE;
	FTM_TIMER_init(&xReportTimer, pNode->xInfo.ulInterval * 1000000);

	while(!pNode->bStop)
	{
		FTM_ULONG		ulRemainTime = 0;
	
		FTM_TIMER_remain(&xReportTimer, &ulRemainTime);
		while (!pNode->bStop && (FTOM_MSGQ_timedPop(&pNode->xMsgQ, ulRemainTime, &pMsg) == FTM_RET_OK))
		{
			TRACE("Message received[%08x]\n", pMsg->xType);
			switch(pMsg->xType)
			{
			case	FTOM_MSG_TYPE_QUIT:
				{	
					pNode->bStop = FTM_TRUE;
				}
				break;

			default:
				{
					WARN("Invalid message[%08x]\n", pMsg->xType);	
				}
			}
			FTM_MEM_free(pMsg);

			FTM_TIMER_remain(&xReportTimer, &ulRemainTime);
		}
	
		if (!pNode->bStop)
		{
			FTM_TIMER_waitForExpired(&xReportTimer);
		}

		FTM_TIMER_add(&xReportTimer, pNode->xInfo.ulInterval * 1000000);
	} 

	xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_INT	i;

		for(i = 0 ; i < ulCount ; i++)
		{
			FTOM_EP_PTR	pEP;

			xRet = FTM_LIST_getAt(&pNode->xEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
			if (xRet == FTM_RET_OK)
			{
				FTOM_EP_stop(pEP, FTM_TRUE);
			}
		}
	}

	TRACE("Node[%s] stopped.\n", pNode->xInfo.pDID);

	return	FTM_RET_OK;
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

FTM_CHAR_PTR	FTOM_NODE_stateToStr
(
	FTOM_NODE_STATE xState
)
{
	switch(xState)
	{
	case	FTOM_NODE_STATE_CREATED:		return	"CREATED";
	case	FTOM_NODE_STATE_INITIALIZED: 	return	"INITIALIZED";
	case	FTOM_NODE_STATE_RUN:			return	"RUN";
	case	FTOM_NODE_STATE_FINALIZED:		return	"FINALIZED";
	}

	return	"UNKNOWN";
}

