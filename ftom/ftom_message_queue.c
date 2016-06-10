#include <string.h>
#include "ftm.h"
#include "ftom_ep.h"
#include "ftom_msg.h"
#include "ftom_message_queue.h"

FTM_RET	FTOM_MSGQ_create(FTOM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	ASSERT(ppMsgQ != NULL);

	FTM_RET				xRet;
	FTOM_MSG_QUEUE_PTR	pMsgQ;

	pMsgQ = (FTOM_MSG_QUEUE_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_QUEUE));
	if (pMsgQ == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pMsgQ, 0, sizeof(FTOM_MSG_QUEUE));

	xRet = FTOM_MSGQ_init(pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsgQ);
		return	xRet;
	}

	*ppMsgQ = pMsgQ;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSGQ_destroy(FTOM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	ASSERT(ppMsgQ != NULL);

	if (*ppMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTOM_MSGQ_final(*ppMsgQ);
	FTM_MEM_free(*ppMsgQ);

	*ppMsgQ = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSGQ_init(FTOM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	return	FTM_MSGQ_init(&pMsgQ->xQueue);	
}

FTM_RET	FTOM_MSGQ_final(FTOM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	return	FTM_MSGQ_final(&pMsgQ->xQueue);	
}

FTM_RET	FTOM_MSGQ_push(FTOM_MSG_QUEUE_PTR pMsgQ, FTOM_MSG_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	return	FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
}

FTM_RET	FTOM_MSGQ_pop(FTOM_MSG_QUEUE_PTR pMsgQ, FTOM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	return	FTM_MSGQ_pop(&pMsgQ->xQueue, (FTM_VOID_PTR _PTR_)ppMsg);	
}

FTM_RET	FTOM_MSGQ_timedPop(FTOM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG ulTimeout, FTOM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	return	FTM_MSGQ_timedPop(&pMsgQ->xQueue, ulTimeout, (FTM_VOID_PTR _PTR_)ppMsg);	
}
