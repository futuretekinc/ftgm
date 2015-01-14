#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftnm.h"
#include "ftm_list.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_node_snmpc.h"

static FTM_LIST	xNodeList;

FTM_VOID_PTR 	FTNM_NODE_task(FTM_VOID_PTR pData);
static FTM_RET	FTNM_NODE_taskInit(FTNM_NODE_PTR pNode);
static FTM_RET	FTNM_NODE_taskSync(FTNM_NODE_PTR pNode);
static FTM_RET	FTNM_NODE_taskRun(FTNM_NODE_PTR pNode);
static FTM_RET	FTNM_NODE_taskWaitingForComplete(FTNM_NODE_PTR pNode);
FTM_INT			FTNM_NODE_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_INT			FTNM_NODE_comparator(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET FTNM_NODE_MNGR_init(FTM_VOID)
{
	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&xNodeList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&xNodeList, FTNM_NODE_seek);
	FTM_LIST_setComparator(&xNodeList, FTNM_NODE_comparator);

	return	nRet;
}

FTM_RET FTNM_NODE_MNGR_final(FTM_VOID)
{
	FTNM_NODE_PTR	pNode;

	while(FTM_LIST_getAt(&xNodeList, 0, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		TRACE("Destroy Node : %s\n", pNode->xInfo.pDID);
		FTNM_NODE_destroy(pNode);
	}
	
	FTM_LIST_destroy(&xNodeList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_create(FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTM_RET			nRet;
	FTNM_NODE_PTR	pNewNode;

	ASSERT((pInfo != NULL) && (ppNode != NULL));

	switch(pInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			pNewNode = (FTNM_NODE_PTR)FTM_MEM_calloc(1, sizeof(FTNM_NODE_SNMPC));
			if (pNewNode == NULL)
			{
				ERROR("Not enough memory!\n");
				return	FTM_RET_NOT_ENOUGH_MEMORY;
			}
		}
		break;

	default:
		{
			ERROR("pInfo->xType = %08lx", pInfo->xType);
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	pNewNode->xState = FTNM_NODE_STATE_CREATING;
	memcpy(&pNewNode->xInfo, pInfo, sizeof(FTM_NODE_INFO));
	FTM_LIST_init(&pNewNode->xEPList);
	pthread_mutex_init(&pNewNode->xMutexLock, NULL);

	nRet = FTM_LIST_append(&xNodeList, pNewNode);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNewNode);	
	}

	pNewNode->xState = FTNM_NODE_STATE_CREATED;

	*ppNode = pNewNode;

	return	nRet;
}

FTM_RET	FTNM_NODE_destroy(FTNM_NODE_PTR	pNode)
{
	FTM_RET			nRet;

	nRet = FTM_LIST_remove(&xNodeList, (FTM_VOID_PTR)pNode);
	if (nRet == FTM_RET_OK)
	{
		TRACE("FTNM_NODE_destroy Success\n");
		FTM_MEM_free(pNode);
	}

	return	nRet;
}

FTM_RET FTNM_NODE_get(FTM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTM_RET			nRet;
	FTNM_NODE_PTR	pNode;
	
	nRet = FTM_LIST_get(&xNodeList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (nRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	nRet;
}

FTM_RET FTNM_NODE_getAt(FTM_ULONG ulIndex, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTM_RET			nRet;
	FTNM_NODE_PTR	pNode;
	
	nRet = FTM_LIST_getAt(&xNodeList, ulIndex, (FTM_VOID_PTR _PTR_)&pNode);
	if (nRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	nRet;
}

FTM_RET	FTNM_NODE_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(&xNodeList, pulCount);

}

FTM_RET	FTNM_NODE_linkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP)
{
	ASSERT((pNode != NULL) && (pEP != NULL));

	pthread_mutex_lock(&pNode->xMutexLock);
	FTM_LIST_append(&pNode->xEPList, pEP);
	FTNM_EP_setNode(pEP, pNode);
	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_unlinkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP)
{
	ASSERT((pNode != NULL) && (pEP != NULL));

	pthread_mutex_lock(&pNode->xMutexLock);

	FTNM_EP_setNode(pEP, NULL);
	FTM_LIST_remove(&pNode->xEPList, pEP);
	
	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_EP_count(FTNM_NODE_PTR pNode, FTM_ULONG_PTR pulCount)
{
	ASSERT((pNode != NULL) && (pulCount != NULL));

	return FTM_LIST_count(&pNode->xEPList, pulCount);
}

FTM_RET	FTNM_NODE_EP_get(FTNM_NODE_PTR pNode, FTM_EPID xEPID, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT((pNode != NULL) && (ppEP != NULL));

	return	FTM_LIST_get(&pNode->xEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTNM_NODE_EP_getAt(FTNM_NODE_PTR pNode, FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT((pNode != NULL) && (ppEP != NULL));

	return	FTM_LIST_getAt(&pNode->xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTNM_NODE_run(FTNM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xState != FTNM_NODE_STATE_CREATED)
	{
		return	FTM_RET_ALREADY_RUNNING;
	}

	pthread_create(&pNode->xPThread, NULL, FTNM_NODE_task, pNode);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_NODE_task(FTM_VOID_PTR pData)
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pData;

	while(1)
	{
		switch(pNode->xState)
		{
		case	FTNM_NODE_STATE_CREATED:
			{
				FTNM_NODE_taskInit(pNode);
			}
			break;

		case	FTNM_NODE_STATE_INITIALIZED:
			{
				FTNM_NODE_taskSync(pNode);
			}
			break;

		case	FTNM_NODE_STATE_SYNCHRONIZED:
		case	FTNM_NODE_STATE_RUN:
			{
				FTNM_NODE_taskRun(pNode);
			}
			break;
		
		case	FTNM_NODE_STATE_RUNNING:
			{
				FTNM_NODE_taskWaitingForComplete(pNode);
			}
			break;

		case	FTNM_NODE_STATE_PROCESS_FINISHED:
			{
				struct	timespec	xTime;
				int64_t				xCurrentTime;

				if (clock_gettime(CLOCK_REALTIME, &xTime) == 0)
				{
					xCurrentTime = xTime.tv_sec * 1000000 + xTime.tv_nsec / 1000;	
				}
				else
				{
					xCurrentTime = time(NULL) * 1000000 ;
				}

				if (xCurrentTime < pNode->xTimeout)
				{
					usleep(pNode->xTimeout - xCurrentTime);
				}
				pNode->xState = FTNM_NODE_STATE_RUN;
			}
			break;

		case	FTNM_NODE_STATE_ABORT:
			return	FTM_RET_OK;
		}

		usleep(100000);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskInit(FTNM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (FTNM_NODE_SNMPC_init((FTNM_NODE_SNMPC_PTR)pNode) != FTM_RET_OK)
	{
		ERROR("SNMP Client initialization failed.\n");	
		pNode->xState = FTNM_NODE_STATE_ABORT;
	}
	
	pNode->xState = FTNM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskSync(FTNM_NODE_PTR pNode)
{
	struct timespec	xTime;

	ASSERT(pNode != NULL);

	if (clock_gettime(CLOCK_REALTIME, &xTime) == 0)
	{
		pNode->xTimeout = xTime.tv_sec * 1000000 + xTime.tv_nsec / 1000;	
	}
	else
	{
		pNode->xTimeout = time(NULL) * 1000000 ;
	}

	pNode->xState = FTNM_NODE_STATE_SYNCHRONIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskRun(FTNM_NODE_PTR pNode)
{
	FTM_RET				nRet;
	struct timespec	xTime;

	ASSERT(pNode != NULL);

	if (clock_gettime(CLOCK_REALTIME, &xTime) == 0)
	{
		TRACE("TIME STAMP : %d.%03d\n", (FTM_INT)xTime.tv_sec, xTime.tv_nsec / 1000000);
	}

	if (pNode->xInfo.ulInterval != 0 && pNode->xInfo.ulInterval < 3600)
	{
		pNode->xTimeout += pNode->xInfo.ulInterval * 1000000;
	}
	else
	{
		pNode->xTimeout += 60 * 1000000;
	}

	pNode->ulRetry 	= 0;
	switch(pNode->xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			nRet = FTNM_NODE_SNMPC_startAsync((FTNM_NODE_SNMPC_PTR)pNode);
			if (nRet == FTM_RET_OK)
			{
				pNode->xState = FTNM_NODE_STATE_RUNNING;
			}
			else
			{
				pNode->xState = FTNM_NODE_STATE_PROCESS_FINISHED;
			}
		}
		break;

	default:
		pNode->xState = FTNM_NODE_STATE_PROCESS_FINISHED;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskWaitingForComplete(FTNM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	TRACE("Node[%s] waiting for complete\n", pNode->xInfo.pDID);
	while(1)
	{
		int64_t			xCurrentTime;
		struct timespec	xTime;

		if (FTNM_NODE_SNMPC_isRunning((FTNM_NODE_SNMPC_PTR)pNode) == FTM_BOOL_FALSE)
		{
			break;		
		}

		if (clock_gettime(CLOCK_REALTIME, &xTime) == 0)
		{
			xCurrentTime = xTime.tv_sec * 1000000 + xTime.tv_nsec / 1000;	
		}
		else
		{
			xCurrentTime = time(NULL) * 1000000 ;
		}

		if (xCurrentTime >= pNode->xTimeout)
		{
			TRACE("Node[%s] timeout %d\n", pNode->xInfo.pDID, pNode->ulRetry);
			if (++pNode->ulRetry > 3)
			{
				break;
			}

			if (pNode->xInfo.ulInterval != 0 && pNode->xInfo.ulInterval < 3600)
			{
				pNode->xTimeout += pNode->xInfo.ulInterval * 1000000;
			}
			else
			{
				pNode->xTimeout += 60 * 1000000;
			}
		}

		usleep(100000);
	}

	FTNM_NODE_SNMPC_stop((FTNM_NODE_SNMPC_PTR)pNode);

	TRACE("Node[%s] completed\n", pNode->xInfo.pDID);
	pNode->xState = FTNM_NODE_STATE_PROCESS_FINISHED;	

	return	FTM_RET_OK;
}



FTM_INT	FTNM_NODE_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(strcmp(pNode->xInfo.pDID, pDID) == 0);
}

FTM_INT	FTNM_NODE_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	FTNM_NODE_PTR	pNode1 = (FTNM_NODE_PTR)pElement1;
	FTNM_NODE_PTR	pNode2 = (FTNM_NODE_PTR)pElement2;

	if ((pElement1 == NULL) || (pElement2 == NULL))
	{
		return	0;	
	}

	return	strcmp(pNode1->xInfo.pDID, pNode2->xInfo.pDID);
}

