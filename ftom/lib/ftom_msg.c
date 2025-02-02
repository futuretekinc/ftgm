#include <string.h>
#include "ftm.h"
#include "ftom_ep.h"
#include "ftom_msg.h"
#include "ftom_json.h"

static
FTM_CHAR_PTR	pMsgTypeString[] =
{
	"FTOM_MSG_TYPE_QUIT",
	"FTOM_MSG_TYPE_INITIALIZE_DONE",

	"FTOM_MSG_TYPE_CONNECTED",
	"FTOM_MSG_TYPE_DISCONNECTED",
	"FTOM_MSG_TYPE_CONNECTION_STATUS",

	"FTOM_MSG_TYPE_GW_STATUS",

	"FTOM_MSG_TYPE_EP_STATUS",
	"FTOM_MSG_TYPE_EP_DATA",
	"FTOM_MSG_TYPE_EP_CTRL",
	"FTOM_MSG_TYPE_EP_DATA_SERVER_TIME",

	"FTOM_MSG_TYPE_DISCOVERY",
	"FTOM_MSG_TYPE_DISCOVERY_INFO",
	"FTOM_MSG_TYPE_DISCOVERY_DONE",

	"FTOM_MSG_TYPE_SERVER_SYNC",
	"FTOM_MSG_TYPE_TIME_SYNC",

	"FTOM_MSG_TYPE_ALERT",

	"FTOM_MSG_TYPE_EVENT",

	"FTOM_MSG_TYPE_ACTION",
	"FTOM_MSG_TYPE_ACTION_ACTIVATION",

	"FTOM_MSG_TYPE_RULE",
	"FTOM_MSG_TYPE_RULE_ACTIVATION",

	"FTOM_MSG_TYPE_TP_REQ_SET_REPORT_INTERVAL",
	"FTOM_MSG_TYPE_TP_REQ_RESTART",
	"FTOM_MSG_TYPE_TP_REQ_REBOOT",
	"FTOM_MSG_TYPE_TP_REQ_POWER_OFF",
	"FTOM_MSG_TYPE_TP_REQ_CONTROL_ACTUATOR",
	"FTOM_MSG_TYPE_TP_RESPONSE",
	"FTOM_MSG_TYPE_REPORT",

	"FTOM_MSG_TYPE_SNMPC_GET_EP_DATA",
	"FTOM_MSG_TYPE_SNMPC_SET_EP_DATA",

	"FTOM_MSG_TYPE_NET_STAT",
	"FTOM_MSG_TYPE_MAX"
};

FTM_RET FTOM_MSG_createInitializeDone
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG);

	pMsg = (FTOM_MSG_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_INITIALIZE_DONE;
	pMsg->ulLen = ulMsgLen;
	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createQuit
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG);

	pMsg = (FTOM_MSG_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_QUIT;
	pMsg->ulLen = ulMsgLen;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}
/*******************************************************************
 * Connection management
 *******************************************************************/
FTM_RET	FTOM_MSG_createConnectionStatus
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_ULONG		xObjectID,
	FTM_BOOL		bConnected,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_CONNECTION_STATUS_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG_CONNECTION_STATUS);

	pMsg = (FTOM_MSG_CONNECTION_STATUS_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType 	= FTOM_MSG_TYPE_CONNECTION_STATUS;
	pMsg->ulLen 	= ulMsgLen;
	pMsg->xObjectID = xObjectID;
	pMsg->bConnected= bConnected;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createConnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_CONNECTED_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG_CONNECTED);

	pMsg = (FTOM_MSG_CONNECTED_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType 	= FTOM_MSG_TYPE_CONNECTED;
	pMsg->ulLen 	= ulMsgLen;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_MSG_createDisconnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_DISCONNECTED_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG_DISCONNECTED);

	pMsg = (FTOM_MSG_DISCONNECTED_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType 	= FTOM_MSG_TYPE_DISCONNECTED;
	pMsg->ulLen 	= ulMsgLen;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}


/*******************************************************************
 * Gateway management
 *******************************************************************/
FTM_RET	FTOM_MSG_createGWStatus
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_GW_STATUS_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_GW_STATUS_PTR pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_GW_STATUS);

	pMsg = (FTOM_MSG_GW_STATUS_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_GW_STATUS;
	pMsg->ulLen 	= ulMsgLen;
	strncpy(pMsg->pGatewayID, pGatewayID, FTM_GWID_LEN);
	pMsg->bStatus 	= bStatus;
	pMsg->ulTimeout = ulTimeout;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createEPStatus
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EP_STATUS_PTR pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_EP_STATUS);

	pMsg = (FTOM_MSG_EP_STATUS_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_EP_STATUS;
	pMsg->ulLen 	= ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->bStatus 	= bStatus;
	pMsg->ulTimeout = ulTimeout;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createEPData
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EP_DATA_PTR pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount;

	pMsg = (FTOM_MSG_EP_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_EP_DATA;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->ulCount	= ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createTimeSync
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_ULONG			ulTime,
	FTOM_MSG_TIME_SYNC_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_TIME_SYNC_PTR	pMsg;
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_TIME_SYNC);

	pMsg = (FTOM_MSG_TIME_SYNC_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_TIME_SYNC;
	pMsg->ulLen = ulMsgLen;
	pMsg->ulTime= ulTime;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createEPCtrl
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTOM_MSG_EP_CTRL_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EP_CTRL_PTR	pMsg;
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_EP_CTRL);

	pMsg = (FTOM_MSG_EP_CTRL_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_EP_CTRL;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->xCtrl		= xCtrl;
	pMsg->ulDuration= ulDuration;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createEPDataServerTime
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG			ulTime,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EP_DATA_SERVER_TIME_PTR	pMsg;
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_EP_DATA_SERVER_TIME);

	pMsg = (FTOM_MSG_EP_DATA_SERVER_TIME_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_EP_DATA_SERVER_TIME;
	pMsg->ulLen 	= ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->ulTime 	= ulTime;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}


FTM_RET FTOM_MSG_createRule
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pRuleID,
	FTM_RULE_STATE		xRuleState,
	FTOM_MSG_RULE_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_RULE_PTR	pMsg;
	FTM_ULONG			ulMsgLen = sizeof(FTOM_MSG_RULE);

	pMsg = (FTOM_MSG_RULE_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_RULE;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pRuleID, pRuleID, FTM_ID_LEN);
	pMsg->xRuleState= xRuleState;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createAction
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pActionID,
	FTM_BOOL			bActivate,
	FTOM_MSG_ACTION_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_ACTION_PTR	pMsg;
	FTM_ULONG			ulMsgLen = sizeof(FTOM_MSG_ACTION);

	pMsg = (FTOM_MSG_ACTION_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_ACTION;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pActionID, pActionID, FTM_ID_LEN);
	pMsg->bActivate = bActivate;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createAlert
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_ALERT_PTR _PTR_ ppMsg
)
{
	ASSERT(pData != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_ALERT_PTR	pMsg;
	FTM_ULONG			ulMsgLen = sizeof(FTOM_MSG_ALERT);

	pMsg = (FTOM_MSG_ALERT_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_ALERT;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createDiscovery
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount,
	FTOM_MSG_DISCOVERY_PTR	_PTR_ ppMsg
)
{
	ASSERT(pNetwork != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_DISCOVERY_PTR	pMsg;
	FTM_ULONG				ulMsgLen = sizeof(FTOM_MSG_DISCOVERY) + strlen(pNetwork) + 1;

	pMsg = (FTOM_MSG_DISCOVERY_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_DISCOVERY;
	pMsg->ulLen = ulMsgLen;
	pMsg->pNetwork 	= (FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_DISCOVERY);
	strcpy(pMsg->pNetwork, pNetwork);
	pMsg->usPort	= usPort;
	pMsg->ulRetryCount = ulRetryCount;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createDiscoveryInfo
(
	FTOM_MSG_SENDER_ID	xSenderID,
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
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_DISCOVERY_INFO) + sizeof(FTM_EP_TYPE) * ulCount;

	pMsg = (FTOM_MSG_DISCOVERY_INFO_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_DISCOVERY_INFO;
	pMsg->ulLen = ulMsgLen;
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
	FTOM_MSG_SENDER_ID	xSenderID,
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
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_DISCOVERY_DONE) + sizeof(FTM_NODE) * ulNodeCount + sizeof(FTM_EP) * ulEPInfoCount;

	pMsg = (FTOM_MSG_DISCOVERY_DONE_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 		= FTOM_MSG_TYPE_DISCOVERY_DONE;
	pMsg->ulLen = ulMsgLen;
	pMsg->ulNodeCount	= ulNodeCount;
	pMsg->pNodeInfos	= (FTM_NODE_PTR)((FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_DISCOVERY_DONE));
	pMsg->ulEPCount		= ulEPInfoCount;
	pMsg->pEPInfos		= (FTM_EP_PTR)((FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_DISCOVERY_DONE) + sizeof(FTM_NODE) * ulNodeCount);

	memcpy(pMsg->pNodeInfos, pNodeInfos, sizeof(FTM_NODE) * ulNodeCount);
	memcpy(pMsg->pEPInfos, pEPInfos, sizeof(FTM_EP) * ulEPInfoCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createServerSync
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_BOOL			bAutoRegister,
	FTOM_MSG_SERVER_SYNC_PTR _PTR_ 	ppMsg
)
{
	ASSERT(ppMsg != NULL);
	
	FTOM_MSG_SERVER_SYNC_PTR pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_SERVER_SYNC);

	pMsg = (FTOM_MSG_SERVER_SYNC_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_SERVER_SYNC;
	pMsg->ulLen = ulMsgLen;
	pMsg->bAutoRegister = bAutoRegister;

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_destroy
(
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	if (*ppMsg != NULL)
	{
		FTM_MEM_free(*ppMsg);
		*ppMsg = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_copy
(
	FTOM_MSG_PTR	pSrcMsg,
	FTOM_MSG_PTR _PTR_ ppNewMsg
)
{
	ASSERT(pSrcMsg != NULL);
	ASSERT(ppNewMsg != NULL);

	FTOM_MSG_PTR	pNewMsg;

	pNewMsg = (FTOM_MSG_PTR)FTM_MEM_malloc(pSrcMsg->ulLen);
	if (pNewMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewMsg, pSrcMsg, pSrcMsg->ulLen);

	*ppNewMsg = pNewMsg;

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTOM_MSG_printType
(
	FTOM_MSG_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);

	static FTM_CHAR	pBuff[32];

	if (pMsg->xType < FTOM_MSG_TYPE_MAX)
	{
		return	pMsgTypeString[pMsg->xType];
	}

	sprintf(pBuff, "%08lx", (FTM_ULONG)pMsg->xType);

	return	pBuff;
}
/********************************************************************************
 * Event  
 ********************************************************************************/
FTM_RET	FTOM_MSG_createEvent
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pTriggerID,
	FTM_BOOL		bOccurred,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EVENT_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_EVENT);

	pMsg = (FTOM_MSG_EVENT_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_EVENT;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pTriggerID, pTriggerID, FTM_ID_LEN);
	pMsg->bOccurred = bOccurred;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/********************************************************************************
 * Rule 
 ********************************************************************************/
FTM_RET	FTOM_MSG_RULE_createActivation
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL		bActivation,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_RULE_ACTIVATION_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_RULE_ACTIVATION);

	pMsg = (FTOM_MSG_RULE_ACTIVATION_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_RULE_ACTIVATION;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pRuleID, pRuleID, FTM_ID_LEN);
	pMsg->bActivation = bActivation;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/********************************************************************************
 * Action
 ********************************************************************************/
FTM_RET	FTOM_MSG_ACTION_createActivation
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL		bActivation,
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_ACTION_ACTIVATION_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_ACTION_ACTIVATION);

	pMsg = (FTOM_MSG_ACTION_ACTIVATION_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_ACTION_ACTIVATION;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pActionID, pActionID, FTM_ID_LEN);
	pMsg->bActivation = bActivation;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/********************************************************************************
 * Requested from the thing+
 ********************************************************************************/
FTM_RET	FTOM_MSG_SNMPC_createGetEPData
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_VALUE_TYPE		xDataType,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(pDID != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pURL != NULL);
	ASSERT(pCommunity != NULL);
	ASSERT(pOID != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_SNMPC_GET_EP_DATA_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_SNMPC_GET_EP_DATA);

	pMsg = (FTOM_MSG_SNMPC_GET_EP_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_SNMPC_GET_EP_DATA;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pDID, pDID, FTM_DID_LEN);
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	strncpy(pMsg->pURL, pURL, FTM_URL_LEN);
	strncpy(pMsg->pCommunity, pCommunity, FTM_SNMP_COMMUNITY_LEN);
	memcpy(&pMsg->xOID, pOID, sizeof(FTM_SNMP_OID));
	pMsg->ulTimeout = ulTimeout;
	pMsg->xDataType = xDataType;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_SNMPC_createSetEPData
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_VALUE_PTR		pValue,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(pDID != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pURL != NULL);
	ASSERT(pCommunity != NULL);
	ASSERT(pOID != NULL);
	ASSERT(ppMsg != NULL);

	FTOM_MSG_SNMPC_SET_EP_DATA_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_SNMPC_SET_EP_DATA);

	pMsg = (FTOM_MSG_SNMPC_SET_EP_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_SNMPC_SET_EP_DATA;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pDID, pDID, FTM_DID_LEN);
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	strncpy(pMsg->pURL, pURL, FTM_URL_LEN);
	strncpy(pMsg->pCommunity, pCommunity, FTM_SNMP_COMMUNITY_LEN);
	memcpy(&pMsg->xOID, pOID, sizeof(FTM_SNMP_OID));
	pMsg->ulTimeout = ulTimeout;
	memcpy(&pMsg->xValue, pValue, sizeof(FTM_VALUE));

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}
/********************************************************************************
 * Network management
 ********************************************************************************/
FTM_RET	FTOM_MSG_createNetConnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	FTOM_MSG_NET_STAT_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_NET_STAT);

	pMsg = (FTOM_MSG_NET_STAT_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_NET_STAT;
	pMsg->bConnected = FTM_TRUE;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createNetDisconnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	FTOM_MSG_NET_STAT_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_NET_STAT);

	pMsg = (FTOM_MSG_NET_STAT_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_NET_STAT;
	pMsg->bConnected = FTM_FALSE;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/********************************************************************************
 * Requested from the thing+
 ********************************************************************************/
FTM_RET	FTOM_MSG_TP_createReqSetReportInterval
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pReqID,
 	FTM_ULONG	ulReportIntervalMS,
	FTOM_MSG_PTR _PTR_	ppMsg

)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL) + strlen(pReqID) + 1;

	pMsg = (FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_TP_REQ_SET_REPORT_INTERVAL;
	pMsg->ulLen = ulMsgLen;
	strcpy(pMsg->pReqID, pReqID);
	pMsg->ulReportIntervalMS = ulReportIntervalMS;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_TP_createReqControlActuator
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pReqID,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR	pMsg;
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_TP_REQ_CONTROL_ACTUATOR) + strlen(pReqID) + 1;

	pMsg = (FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_TP_REQ_CONTROL_ACTUATOR;
	pMsg->ulLen 	= ulMsgLen;
	strcpy(pMsg->pReqID, pReqID);
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->xCtrl		= xCtrl;
	pMsg->ulDuration= ulDuration;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_TP_createResponse
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pMsgID,
	FTM_INT			nCode,
	FTM_CHAR_PTR	pMessage,
	FTOM_MSG_PTR _PTR_	ppMsg
)
{
	ASSERT(ppMsg != NULL);
	ASSERT(pMsgID != NULL);
	ASSERT(pMessage != NULL);

	FTOM_MSG_TP_RESPONSE_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_TP_RESPONSE) + strlen(pMsgID) + strlen(pMessage) + 1;

	pMsg = (FTOM_MSG_TP_RESPONSE_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	pMsg->pMsgID = (FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_TP_RESPONSE);
	pMsg->pMessage = (FTM_CHAR_PTR)pMsg + sizeof(FTOM_MSG_TP_RESPONSE) + strlen(pMsgID) + 1;

	pMsg->xType	= FTOM_MSG_TYPE_TP_RESPONSE;
	pMsg->ulLen = ulMsgLen;
	pMsg->nCode = nCode;
	strcpy(pMsg->pMsgID, pMsgID);
	strcpy(pMsg->pMessage, pMessage);

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_TP_createReqRestart
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pReqID,
	FTOM_MSG_PTR _PTR_	ppMsg
)
{
	ASSERT(ppMsg != NULL);
	ASSERT(pReqID != NULL);

	FTOM_MSG_TP_REQ_RESTART_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_TP_REQ_RESTART) + strlen(pReqID) + 1;

	pMsg = (FTOM_MSG_TP_REQ_RESTART_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_TP_REQ_RESTART;
	pMsg->ulLen = ulMsgLen;
	strcpy(pMsg->pReqID, pReqID);

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_TP_createReqReboot
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pReqID,
	FTOM_MSG_PTR _PTR_	ppMsg
)
{
	ASSERT(ppMsg != NULL);
	ASSERT(pReqID != NULL);

	FTOM_MSG_TP_REQ_REBOOT_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_TP_REQ_REBOOT) + strlen(pReqID) + 1;

	pMsg = (FTOM_MSG_TP_REQ_REBOOT_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_TP_REQ_REBOOT;
	pMsg->ulLen = ulMsgLen;
	strcpy(pMsg->pReqID, pReqID);

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_TP_createReqPowerOff
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pReqID,
	FTOM_MSG_PTR _PTR_	ppMsg
)
{
	ASSERT(ppMsg != NULL);
	ASSERT(pReqID != NULL);

	FTOM_MSG_TP_REQ_POWER_OFF_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_TP_REQ_POWER_OFF) + strlen(pReqID) + 1;

	pMsg = (FTOM_MSG_TP_REQ_POWER_OFF_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_TP_REQ_POWER_OFF;
	pMsg->ulLen = ulMsgLen;
	strcpy(pMsg->pReqID, pReqID);

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createReport
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_	ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_REPORT_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_REPORT);

	pMsg = (FTOM_MSG_REPORT_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", ulMsgLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_REPORT;
	pMsg->ulLen = ulMsgLen;
	pMsg->xSenderID=xSenderID;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/***********************************************************************
 * Print out to json formatted string
 ***********************************************************************/
FTM_RET	FTOM_MSG_JSON_createEPData
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTM_CHAR_PTR _PTR_	ppBuff
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);
	ASSERT(ppBuff != NULL);

	FTM_RET         xRet;
	FTM_ULONG		ulBuffSize;
	FTM_CHAR_PTR	pBuff;
	FTOM_JSON_PTR	pObject = NULL;

	xRet = FTOM_JSON_createEPData(pEPID, pData, ulCount, &pObject);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_JSON_getBufferSize(pObject, &ulBuffSize);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get buffer size!\n");
		goto finished;	
	}

	pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(ulBuffSize);
	if (pBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		goto finished;
	}

	strcpy(pBuff, FTOM_JSON_print(pObject));
	
	*ppBuff =pBuff;

finished:

	if (pBuff != NULL)
	{
		FTM_MEM_free(pBuff);
	}

	FTOM_JSON_destroy(&pObject);

	return	xRet;
}

FTM_RET	FTOM_MSG_JSON_printEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTM_CHAR_PTR 		pBuff,
	FTM_ULONG			ulBuffLen
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);
	ASSERT(pBuff != NULL);

	FTM_RET         xRet;
	FTOM_JSON_PTR	pObject = NULL;

	xRet = FTOM_JSON_createEPData(pEPID, pData, ulCount, &pObject);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	strncpy(pBuff, FTOM_JSON_print(pObject), ulBuffLen);

	
	FTOM_JSON_destroy(&pObject);

	return	xRet;
}
