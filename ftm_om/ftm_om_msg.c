#include <string.h>
#include "ftm.h"
#include "ftm_om_ep.h"
#include "ftm_om_msg.h"

FTM_RET	FTM_OM_MSG_create(FTM_OM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(ppMsg != NULL);
	FTM_OM_MSG_PTR	pMsg;

	pMsg = (FTM_OM_MSG_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pMsg, 0, sizeof(FTM_OM_MSG));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSG_destroy(FTM_OM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(ppMsg != NULL);

	if (*ppMsg != NULL)
	{
		FTM_MEM_free(*ppMsg);
		*ppMsg = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSGQ_create(FTM_OM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	ASSERT(ppMsgQ != NULL);

	FTM_RET				xRet;
	FTM_OM_MSG_QUEUE_PTR	pMsgQ;

	pMsgQ = (FTM_OM_MSG_QUEUE_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_QUEUE));
	if (pMsgQ == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pMsgQ, 0, sizeof(FTM_OM_MSG_QUEUE));

	xRet = FTM_OM_MSGQ_init(pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsgQ);
		return	xRet;
	}

	*ppMsgQ = pMsgQ;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSGQ_destroy(FTM_OM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	ASSERT(ppMsgQ != NULL);

	if (*ppMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_OM_MSGQ_final(*ppMsgQ);
	FTM_MEM_free(*ppMsgQ);

	*ppMsgQ = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_MSGQ_init(FTM_OM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	return	FTM_MSGQ_init(&pMsgQ->xQueue);	
}

FTM_RET	FTM_OM_MSGQ_final(FTM_OM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	return	FTM_MSGQ_final(&pMsgQ->xQueue);	
}

FTM_RET	FTM_OM_MSGQ_sendQuit(FTM_OM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);
	FTM_RET		xRet;
	FTM_OM_MSG_PTR pMsg;

	xRet = FTM_OM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	pMsg->xType = FTM_OM_MSG_TYPE_QUIT;
	xRet = FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_OM_MSG_destroy(&pMsg);
	}

	return	xRet;
}

FTM_RET	FTM_OM_MSGQ_push(FTM_OM_MSG_QUEUE_PTR pMsgQ, FTM_OM_MSG_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	return	FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
}

FTM_RET	FTM_OM_MSGQ_pop(FTM_OM_MSG_QUEUE_PTR pMsgQ, FTM_OM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	return	FTM_MSGQ_pop(&pMsgQ->xQueue, (FTM_VOID_PTR _PTR_)ppMsg);	
}

FTM_RET	FTM_OM_MSGQ_timedPop(FTM_OM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG ulTimeout, FTM_OM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	return	FTM_MSGQ_timedPop(&pMsgQ->xQueue, ulTimeout, (FTM_VOID_PTR _PTR_)ppMsg);	
}
