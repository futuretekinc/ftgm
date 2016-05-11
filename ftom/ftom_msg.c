#include <string.h>
#include "ftm.h"
#include "ftom_ep.h"
#include "ftom_msg.h"

FTM_RET FTOM_MSG_createQuit
(
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PTR	pMsg;

	pMsg = (FTOM_MSG_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_QUIT;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createAddEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_ADD_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(pData != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_ADD_EP_DATA_PTR	pMsg;

	pMsg = (FTOM_MSG_ADD_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_ADD_EP_DATA));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_ADD_EP_DATA;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createSendEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_SEND_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_SEND_EP_DATA_PTR pMsg;

	pMsg = (FTOM_MSG_SEND_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_SEND_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount) ;
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_SEND_EP_DATA;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->ulCount	= ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createPublishEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_PUBLISH_EP_DATA_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PUBLISH_EP_DATA_PTR pMsg;

	pMsg = (FTOM_MSG_PUBLISH_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_PUBLISH_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount) ;
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_PUBLISH_EP_DATA;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->ulCount	= ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createTimeSync
(
	FTM_ULONG			ulTime,
	FTOM_MSG_TIME_SYNC_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_TIME_SYNC_PTR	pMsg;

	pMsg = (FTOM_MSG_TIME_SYNC_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_TIME_SYNC));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_TIME_SYNC;
	pMsg->ulTime= ulTime;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createEPCtrl
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTOM_MSG_EP_CTRL_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EP_CTRL_PTR	pMsg;

	pMsg = (FTOM_MSG_EP_CTRL_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_EP_CTRL));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_EP_CTRL;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->xCtrl		= xCtrl;
	pMsg->ulDuration= ulDuration;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}


FTM_RET FTOM_MSG_createRule
(
	FTM_CHAR_PTR		pRuleID,
	FTM_RULE_STATE		xRuleState,
	FTOM_MSG_RULE_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_RULE_PTR	pMsg;

	pMsg = (FTOM_MSG_RULE_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_RULE));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_RULE;
	strncpy(pMsg->pRuleID, pRuleID, FTM_ID_LEN);
	pMsg->xRuleState= xRuleState;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createAction
(
	FTM_CHAR_PTR		pActionID,
	FTM_BOOL			bActivate,
	FTOM_MSG_ACTION_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_ACTION_PTR	pMsg;

	pMsg = (FTOM_MSG_ACTION_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_ACTION));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_ACTION;
	strncpy(pMsg->pActionID, pActionID, FTM_ID_LEN);
	pMsg->bActivate = bActivate;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createAlert
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_ALERT_PTR _PTR_ ppMsg
)
{
	ASSERT(pData != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_ALERT_PTR	pMsg;

	pMsg = (FTOM_MSG_ALERT_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_ALERT));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_ALERT;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createDiscovery
(
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort,
	FTOM_MSG_DISCOVERY_PTR	_PTR_ ppMsg
)
{
	ASSERT(pNetwork != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_DISCOVERY_PTR	pMsg;

	pMsg = (FTOM_MSG_DISCOVERY_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_DISCOVERY) + strlen(pNetwork) + 1);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_DISCOVERY;
	pMsg->pNetwork 	= (FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_DISCOVERY);
	strcpy(pMsg->pNetwork, pNetwork);
	pMsg->usPort	= usPort;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createDiscoveryInfo
(
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pIP,
	FTM_EP_TYPE_PTR		pTypes,
	FTM_ULONG			ulCount,
	FTOM_MSG_DISCOVERY_INFO_PTR _PTR_ ppMsg
)
{
	ASSERT(pDID != NULL);
	ASSERT(pTypes != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg;

	pMsg = (FTOM_MSG_DISCOVERY_INFO_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_DISCOVERY_INFO) + sizeof(FTM_EP_TYPE) * ulCount);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_DISCOVERY_INFO;
	if (pName != NULL)
	{
		strncpy(pMsg->pName, pName, FTM_DEVICE_NAME_LEN);
	}
	strncpy(pMsg->pDID, pDID, FTM_DID_LEN);
	strncpy(pMsg->pIP, pIP, sizeof(pMsg->pIP)-1);
	pMsg->ulCount 	= ulCount;
	memcpy(pMsg->pTypes, pTypes, sizeof(FTM_EP_TYPE) * ulCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createDiscoveryDone
(
	FTM_NODE_PTR		pNodeInfos,
	FTM_ULONG			ulNodeCount,
	FTM_EP_PTR			pEPInfos,
	FTM_ULONG			ulEPInfoCount,
	FTOM_MSG_DISCOVERY_DONE_PTR _PTR_ ppMsg
)
{
	ASSERT(pNodeInfos != NULL);
	ASSERT(pEPInfos != NULL);

	FTOM_MSG_DISCOVERY_DONE_PTR pMsg;

	pMsg = (FTOM_MSG_DISCOVERY_DONE_PTR)FTM_MEM_malloc(sizeof(FTOM_MSG_DISCOVERY_DONE) + sizeof(FTM_NODE) * ulNodeCount + sizeof(FTM_EP) * ulEPInfoCount);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 		= FTOM_MSG_TYPE_DISCOVERY_DONE;
	pMsg->ulNodeCount	= ulNodeCount;
	pMsg->pNodeInfos	= (FTM_NODE_PTR)((FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_DISCOVERY_DONE));
	pMsg->ulEPCount		= ulEPInfoCount;
	pMsg->pEPInfos		= (FTM_EP_PTR)((FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_DISCOVERY_DONE) + sizeof(FTM_NODE) * ulNodeCount);

	memcpy(pMsg->pNodeInfos, pNodeInfos, sizeof(FTM_NODE) * ulNodeCount);
	memcpy(pMsg->pEPInfos, pEPInfos, sizeof(FTM_EP) * ulEPInfoCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_destroy(FTOM_MSG_PTR _PTR_ ppMsg)
{
	ASSERT(ppMsg != NULL);

	if (*ppMsg != NULL)
	{
		FTM_MEM_free(*ppMsg);
		*ppMsg = NULL;
	}

	return	FTM_RET_OK;
}

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
