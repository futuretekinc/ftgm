#include <string.h>
#include "ftm.h"
#include "ftnm_ep.h"
#include "ftnm_msg.h"

FTM_RET	FTNM_MSG_create(FTNM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(ppMsg != NULL);
	FTNM_MSG_PTR	pMsg;

	pMsg = (FTNM_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_MSG));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pMsg, 0, sizeof(FTNM_MSG));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_MSG_destroy(FTNM_MSG_PTR pMsg)
{
	ASSERT(pMsg != NULL);

	FTM_MEM_free(pMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_MSGQ_create(FTNM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	ASSERT(ppMsgQ != NULL);

	FTM_RET				xRet;
	FTNM_MSG_QUEUE_PTR	pMsgQ;

	pMsgQ = (FTNM_MSG_QUEUE_PTR)FTM_MEM_malloc(sizeof(FTNM_MSG_QUEUE));
	if (pMsgQ == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pMsgQ, 0, sizeof(FTNM_MSG_QUEUE));

	xRet = FTNM_MSGQ_init(pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsgQ);
		return	xRet;
	}

	*ppMsgQ = pMsgQ;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_MSGQ_destroy(FTNM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	ASSERT(ppMsgQ != NULL);

	if (*ppMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTNM_MSGQ_final(*ppMsgQ);
	FTM_MEM_free(*ppMsgQ);

	*ppMsgQ = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTNM_MSGQ_init(FTNM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	return	FTM_MSGQ_init(&pMsgQ->xQueue);	
}

FTM_RET	FTNM_MSGQ_final(FTNM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	return	FTM_MSGQ_final(&pMsgQ->xQueue);	
}

FTM_RET	FTNM_MSGQ_sendSNMPTrap(FTNM_MSG_QUEUE_PTR pMsgQ, FTM_CHAR_PTR pTrapMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pTrapMsg != NULL);
	FTM_RET			xRet;
	FTNM_MSG_PTR 	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	pMsg->xType = FTNM_MSG_TYPE_SNMPTRAP;
	strncpy(pMsg->xParams.xSNMPTrap.pString, pTrapMsg, sizeof(pMsg->xParams.xSNMPTrap.pString) - 1);

	xRet = FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTNM_MSG_destroy(pMsg);
	}

	return	xRet;
}

FTM_RET FTNM_MSGQ_sendEPChanged(FTNM_MSG_QUEUE_PTR pMsgQ, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pData != NULL);
	FTM_RET			xRet;
	FTNM_MSG_PTR pMsg;

	TRACE("Send EP[%08x] changed!\n", xEPID);
	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	pMsg->xType = FTNM_MSG_TYPE_EP_CHANGED;
	pMsg->xParams.xEPChanged.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPChanged.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTNM_MSG_destroy(pMsg);
	}

	return	xRet;
}

FTM_RET	FTNM_MSGQ_sendQuit(FTNM_MSG_QUEUE_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);
	FTM_RET		xRet;
	FTNM_MSG_PTR pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	pMsg->xType = FTNM_MSG_TYPE_QUIT;
	xRet = FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTNM_MSG_destroy(pMsg);
	}

	return	xRet;
}

FTM_RET	FTNM_MSGQ_push(FTNM_MSG_QUEUE_PTR pMsgQ, FTNM_MSG_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	return	FTM_MSGQ_push(&pMsgQ->xQueue, pMsg);
}

FTM_RET	FTNM_MSGQ_pop(FTNM_MSG_QUEUE_PTR pMsgQ, FTNM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	return	FTM_MSGQ_pop(&pMsgQ->xQueue, (FTM_VOID_PTR _PTR_)ppMsg);	
}

FTM_RET	FTNM_MSGQ_timedPop(FTNM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG ulTimeout, FTNM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	return	FTM_MSGQ_timedPop(&pMsgQ->xQueue, ulTimeout, (FTM_VOID_PTR _PTR_)ppMsg);	
}
