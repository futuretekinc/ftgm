#include <string.h>
#include "ftm.h"
#include "ftom_ep.h"
#include "ftom_msg.h"

FTM_RET FTOM_MSG_createInitializeDone
(
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG);

	pMsg = (FTOM_MSG_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_INITIALIZE_DONE;
	pMsg->ulLen = ulMsgLen;
	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET FTOM_MSG_createQuit
(
	FTOM_MSG_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PTR	pMsg;
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG);

	pMsg = (FTOM_MSG_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType 	= FTOM_MSG_TYPE_CONNECTION_STATUS;
	pMsg->ulLen 	= ulMsgLen;
	pMsg->xObjectID = xObjectID;
	pMsg->bConnected= bConnected;

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/*******************************************************************
 * Gateway management
 *******************************************************************/
FTM_RET	FTOM_MSG_createReportGWStatus
(
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_REPORT_GW_STATUS_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_REPORT_GW_STATUS_PTR pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_REPORT_GW_STATUS);

	pMsg = (FTOM_MSG_REPORT_GW_STATUS_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_REPORT_GW_STATUS;
	pMsg->ulLen 	= ulMsgLen;
	strncpy(pMsg->pGatewayID, pGatewayID, FTM_GWID_LEN);
	pMsg->bStatus 	= bStatus;
	pMsg->ulTimeout = ulTimeout;

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
	FTM_ULONG		ulMsgLen = sizeof(FTOM_MSG_ADD_EP_DATA);

	pMsg = (FTOM_MSG_ADD_EP_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType = FTOM_MSG_TYPE_ADD_EP_DATA;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createSendEPStatus
(
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_SEND_EP_STATUS_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_SEND_EP_STATUS_PTR pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_SEND_EP_STATUS);

	pMsg = (FTOM_MSG_SEND_EP_STATUS_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_SEND_EP_STATUS;
	pMsg->ulLen 	= ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->bStatus 	= bStatus;
	pMsg->ulTimeout = ulTimeout;

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
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_SEND_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount;

	pMsg = (FTOM_MSG_SEND_EP_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_SEND_EP_DATA;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->ulCount	= ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = pMsg;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MSG_createPublishEPStatus
(
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_PUBLISH_EP_STATUS_PTR _PTR_ ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_PUBLISH_EP_STATUS_PTR pMsg;
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_PUBLISH_EP_STATUS);

	pMsg = (FTOM_MSG_PUBLISH_EP_STATUS_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_PUBLISH_EP_STATUS;
	pMsg->ulLen = ulMsgLen;
	strncpy(pMsg->pEPID, pEPID, FTM_EPID_LEN);
	pMsg->bStatus = bStatus;
	pMsg->ulTimeout = ulTimeout;

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
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_PUBLISH_EP_DATA) + sizeof(FTM_EP_DATA) * ulCount ;

	pMsg = (FTOM_MSG_PUBLISH_EP_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType 	= FTOM_MSG_TYPE_PUBLISH_EP_DATA;
	pMsg->ulLen = ulMsgLen;
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
	FTM_ULONG ulMsgLen = sizeof(FTOM_MSG_TIME_SYNC);

	pMsg = (FTOM_MSG_TIME_SYNC_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
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


FTM_RET FTOM_MSG_createRule
(
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
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
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
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_SERVER_SYNC;
	pMsg->ulLen = ulMsgLen;
	pMsg->bAutoRegister = bAutoRegister;

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

/********************************************************************************
 * EP
 ********************************************************************************/
FTM_RET	FTOM_MSG_EP_createInsertData
(
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_PTR _PTR_ 	ppMsg
)
{
	ASSERT(ppMsg != NULL);

	FTOM_MSG_EP_INSERT_DATA_PTR	pMsg;
	FTM_ULONG	ulMsgLen = sizeof(FTOM_MSG_EP_INSERT_DATA) + sizeof(FTM_EP_DATA) * ulCount;

	pMsg = (FTOM_MSG_EP_INSERT_DATA_PTR)FTM_MEM_malloc(ulMsgLen);
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsg->xType	= FTOM_MSG_TYPE_EP_INSERT_DATA;
	pMsg->ulLen = ulMsgLen;
	pMsg->ulCount = ulCount;
	memcpy(pMsg->pData, pData, sizeof(FTM_EP_DATA) * ulCount);

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/********************************************************************************
 * Event  
 ********************************************************************************/
FTM_RET	FTOM_MSG_createEvent
(
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
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_EP_DATA_TYPE	xDataType,
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
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_EP_DATA_PTR		pData,
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
	memcpy(&pMsg->xData, pData, sizeof(FTM_EP_DATA));

	*ppMsg = (FTOM_MSG_PTR)pMsg;

	return	FTM_RET_OK;
}

/********************************************************************************
 * Requested from the thing+
 ********************************************************************************/
FTM_RET	FTOM_MSG_TP_createReqSetReportInterval
(
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
		ERROR("Not enough memory!\n");
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

