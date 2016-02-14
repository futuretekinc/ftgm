#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_error.h"
#include "ftm_queue.h"
#include "ftm_mem.h"
#include "ftm_msg_queue.h"

FTM_RET	FTM_MSGQ_create(FTM_ULONG ulMsgSize, FTM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	FTM_MSG_QUEUE_PTR	pMsgQ;

	pMsgQ = (FTM_MSG_QUEUE_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_QUEUE));
	if (pMsgQ == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_MSGQ_init(pMsgQ);

	*ppMsgQ = pMsgQ;
	
	return	FTM_RET_OK;
}

FTM_RET FTM_MSGQ_init(FTM_MSG_QUEUE_PTR pMsgQ)
{
	memset(pMsgQ, 0, sizeof(FTM_MSG_QUEUE));
	FTM_QUEUE_init(&pMsgQ->xQueue);	
	sem_init(&pMsgQ->xLock, 0, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MSGQ_final(FTM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);
	
	FTM_VOID_PTR pItem;

	while (FTM_QUEUE_pop(&pMsgQ->xQueue, &pItem) == FTM_RET_OK)
	{
		FTM_MEM_free(pItem);	
	}

	sem_destroy(&pMsgQ->xLock);
	FTM_QUEUE_final(&pMsgQ->xQueue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MSGQ_destroy(FTM_MSG_QUEUE_PTR pMsgQ)
{

	FTM_MSGQ_final(pMsgQ);

	FTM_MEM_free(pMsgQ);

	return	FTM_RET_OK;

}

FTM_RET FTM_MSGQ_push(FTM_MSG_QUEUE_PTR pMsgQ, FTM_VOID_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;
	FTM_VOID_PTR pNewMsg;

	pNewMsg = FTM_MEM_malloc(pMsgQ->ulMsgSize);
	if (pNewMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewMsg, pMsg, pMsgQ->ulMsgSize);

	xRet = FTM_QUEUE_push(&pMsgQ->xQueue, pNewMsg);
	if (xRet == FTM_RET_OK)
	{
		sem_post(&pMsgQ->xLock);
	}
	else
	{
		FTM_MEM_free(pNewMsg);	
	}

	return	xRet;
}

FTM_RET FTM_MSGQ_pop(FTM_MSG_QUEUE_PTR pMsgQ, FTM_VOID_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet;
	FTM_VOID_PTR	pTmpMsg;

	sem_wait(&pMsgQ->xLock);

	xRet = FTM_QUEUE_pop(&pMsgQ->xQueue, &pTmpMsg);
	if (xRet != FTM_RET_OK)
	{
		sem_post(&pMsgQ->xLock);
	}
	else	
	{
		memcpy(pMsg, pTmpMsg, pMsgQ->ulMsgSize);
		FTM_MEM_free(pTmpMsg);		
	}

	return	xRet;
}

FTM_RET FTM_MSGQ_timedPop(FTM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG ulTimeout, FTM_VOID_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;
	FTM_VOID_PTR	pTmpMsg;
	struct timespec	xTime;

	xTime.tv_sec = ulTimeout / 1000000;
	xTime.tv_nsec = (ulTimeout % 1000000) * 1000;

	if (sem_timedwait(&pMsgQ->xLock, &xTime) != 0)
	{
		return	FTM_RET_TIMEOUT;
	}

	xRet = FTM_QUEUE_pop(&pMsgQ->xQueue, &pTmpMsg);
	if (xRet != FTM_RET_OK)
	{
		sem_post(&pMsgQ->xLock);
	}
	else	
	{
		memcpy(pMsg, pTmpMsg, pMsgQ->ulMsgSize);
		FTM_MEM_free(pTmpMsg);		
	}

	return	xRet;
}

FTM_RET FTM_MSGQ_count(FTM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG_PTR pulCount)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_QUEUE_count(&pMsgQ->xQueue, pulCount);
}

