#include <stdlib.h>
#include <string.h>
#include "ftnm_node.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"

static FTM_LIST	xNodeList;

FTM_VOID_PTR 	FTNM_NODE_task(FTM_VOID_PTR pData);
static FTM_RET			FTNM_NODE_taskInit(FTNM_NODE_PTR pNode);
static FTM_RET			FTNM_NODE_taskSync(FTNM_NODE_PTR pNode);
static FTM_RET			FTNM_NODE_taskWait(FTNM_NODE_PTR pNode);
static FTM_RET			FTNM_NODE_taskProcessing(FTNM_NODE_PTR pNode);
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
			pNewNode = (FTNM_NODE_PTR)calloc(1, sizeof(FTNM_NODE_SNMP));
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
	pthread_mutex_init(&pNewNode->xMutexLock, NULL);

	nRet = FTM_LIST_append(&xNodeList, pNewNode);
	if (nRet != FTM_RET_OK)
	{
		free(pNewNode);	
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
		free(pNode);
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
	FTNM_EP_PTR	pTempEP;

	ASSERT((pNOde != NULL) && (pEP != NULL));

	pthread_mutex_lock(&pNode->xMutexLock);

	if (FTM_LIST_get(&pNode->xEPList, &pEP->xInfo.xEPID, (FTM_VOID_PTR _PTR_)&pTempEP) == FTM_RET_OK)
	{
		pthread_mutex_unlock(&pNode->xMutexLock);
		
		return	FTM_RET_ALREADY_EXISTS;
	}

	FTM_LIST_append(&pNode->xEPList, pEP);
	//FTNM_EP_setNode(pEP, pNode);

	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_unlinkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP)
{
	ASSERT((pNode != NULL) && (pEP != NULL));

	pthread_mutex_lock(&pNode->xMutexLock);

	//FTNM_EP_setNode(pEP, NULL);
	FTM_LIST_remove(&pNode->xEPList, pEP);
	
	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
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
			{
				FTNM_NODE_taskWait(pNode);
			}
			break;

		case	FTNM_NODE_STATE_CALL_FOR_PROCESSING:
			{
				FTNM_NODE_taskProcessing(pNode);	
			}
			break;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskInit(FTNM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	pNode->xState = FTNM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskSync(FTNM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	pNode->xState = FTNM_NODE_STATE_SYNCING;


	pNode->xState = FTNM_NODE_STATE_SYNCHRONIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskWait(FTNM_NODE_PTR pNode)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_taskProcessing(FTNM_NODE_PTR pNode)
{
	FTM_RET				nRet;
	FTNM_NODE_TASK_PTR	pTask;

	ASSERT(pNode != NULL);

	pNode->xState = FTNM_NODE_STATE_PROCESSING;
	while(pNode->xState == FTNM_NODE_STATE_PROCESSING)
	{
		nRet = 	FTM_LIST_getAt(&pNode->xTaskList, 0, (FTM_VOID_PTR _PTR_)&pTask);
		if (nRet != FTM_RET_OK)
		{
			pNode->xState = FTNM_NODE_STATE_PROCESS_FINISHED;
			break;	
		}
		
	}

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

