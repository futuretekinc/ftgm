#include <string.h>
#include "ftm.h"
#include "ftm_om_ep.h"
#include "ftm_om_msg.h"

FTM_RET FTM_OM_MSG_createQuit
(
	FTM_OM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_PTR	pMsg;

	pMsg = (FTM_OM_MSG_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTM_OM_MSG_TYPE_QUIT;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSG_createSetEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_OM_MSG_SET_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(pData != NULL);
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_SET_EP_DATA_PTR	pMsg;

	pMsg = (FTM_OM_MSG_SET_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_SET_EP_DATA));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTM_OM_MSG_TYPE_SET_EP_DATA;
	pMsg->xEPID = xEPID;
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSG_createSaveEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_OM_MSG_SAVE_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(pData != NULL);
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_SAVE_EP_DATA_PTR	pMsg;

	pMsg = (FTM_OM_MSG_SAVE_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_SAVE_EP_DATA));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTM_OM_MSG_TYPE_SAVE_EP_DATA;
	pMsg->xEPID = xEPID;
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSG_createSendEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTM_OM_MSG_SEND_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_SEND_EP_DATA_PTR pMsg;

	pMsg = (FTM_OM_MSG_SEND_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_SEND_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount) ;
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTM_OM_MSG_TYPE_SEND_EP_DATA;
	pMsg->xEPID 	= xEPID;
	pMsg->ulCount	= ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSG_createPublishEPData
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTM_OM_MSG_PUBLISH_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_PUBLISH_EP_DATA_PTR pMsg;

	pMsg = (FTM_OM_MSG_PUBLISH_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_PUBLISH_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount) ;
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTM_OM_MSG_TYPE_PUBLISH_EP_DATA;
	pMsg->xEPID 	= xEPID;
	pMsg->ulCount	= ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_MSG_createTimeSync
(
	FTM_ULONG			ulTime,
	FTM_OM_MSG_TIME_SYNC_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_TIME_SYNC_PTR	pMsg;

	pMsg = (FTM_OM_MSG_TIME_SYNC_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_TIME_SYNC));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTM_OM_MSG_TYPE_TIME_SYNC;
	pMsg->ulTime= ulTime;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_MSG_createEPCtrl
(
	FTM_EP_ID			xEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTM_OM_MSG_EP_CTRL_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_EP_CTRL_PTR	pMsg;

	pMsg = (FTM_OM_MSG_EP_CTRL_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_EP_CTRL));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTM_OM_MSG_TYPE_EP_CTRL;
	pMsg->xEPID 	= xEPID;
	pMsg->xCtrl		= xCtrl;
	pMsg->ulDuration= ulDuration;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}


FTM_RET FTM_OM_MSG_createRule
(
	FTM_RULE_ID			xRuleID,
	FTM_RULE_STATE		xRuleState,
	FTM_OM_MSG_RULE_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTM_OM_MSG_RULE_PTR	pMsg;

	pMsg = (FTM_OM_MSG_RULE_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG_RULE));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTM_OM_MSG_TYPE_RULE;
	pMsg->xRuleID	= xRuleID;
	pMsg->xRuleState= xRuleState;

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
