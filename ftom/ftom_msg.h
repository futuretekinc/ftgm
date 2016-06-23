#ifndef	_FTOM_MSG_H_
#define	_FTOM_MSG_H_

#include "ftm.h"

#define	FTOM_MSG_STRING_LENGTH	1024
#define	FTOM_MSG_REQ_ID_LENGTH	32

typedef	FTM_ULONG	FTOM_MSG_ID, _PTR_ FTOM_MSG_ID_PTR;

typedef	enum
{
	FTOM_MSG_TYPE_QUIT				=	0,
	FTOM_MSG_TYPE_CONNECTION_STATUS,
	FTOM_MSG_TYPE_REPORT_GW_STATUS,
	FTOM_MSG_TYPE_RULE,
	FTOM_MSG_TYPE_RULE_ACTIVATION,
	FTOM_MSG_TYPE_SEND_EP_STATUS,
	FTOM_MSG_TYPE_SEND_EP_DATA,
	FTOM_MSG_TYPE_ADD_EP_DATA,
	FTOM_MSG_TYPE_PUBLISH_EP_STATUS,
	FTOM_MSG_TYPE_PUBLISH_EP_DATA,
	FTOM_MSG_TYPE_PUBLISH_EP_LAST_DATA,
	FTOM_MSG_TYPE_TIME_SYNC,
	FTOM_MSG_TYPE_CONNECTED,
	FTOM_MSG_TYPE_DISCONNECTED,
	FTOM_MSG_TYPE_ACTION,
	FTOM_MSG_TYPE_ACTION_ACTIVATION,
	FTOM_MSG_TYPE_ALERT,
	FTOM_MSG_TYPE_DISCOVERY,
	FTOM_MSG_TYPE_DISCOVERY_INFO,
	FTOM_MSG_TYPE_DISCOVERY_DONE,
	FTOM_MSG_TYPE_SERVER_SYNC,
	FTOM_MSG_TYPE_INITIALIZE_DONE,
	FTOM_MSG_TYPE_EP_INSERT_DATA,
	FTOM_MSG_TYPE_EP_CTRL,
	FTOM_MSG_TYPE_EVENT,
	FTOM_MSG_TYPE_TP_REQ_SET_REPORT_INTERVAL,
	FTOM_MSG_TYPE_TP_REQ_RESTART,
	FTOM_MSG_TYPE_TP_REQ_CONTROL_ACTUATOR,
	FTOM_MSG_TYPE_TP_RESPONSE,
	FTOM_MSG_TYPE_MAX
}	FTOM_MSG_TYPE, _PTR_ FTOM_MSG_TYPE_PTR;

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
} FTOM_MSG, _PTR_ FTOM_MSG_PTR;

FTM_RET	FTOM_MSG_destroy
(
	FTOM_MSG_PTR _PTR_ ppMsg
);

FTM_RET	FTOM_MSG_copy
(
	FTOM_MSG_PTR	pSrcMsg,
	FTOM_MSG_PTR _PTR_ ppNewMsg
);

/**************************************************************
 * System initialize done 
 **************************************************************/
FTM_RET FTOM_MSG_createInitializeDone
(
	FTOM_MSG_PTR _PTR_ ppMsg
);

/**************************************************************
 * Quit
 **************************************************************/
FTM_RET FTOM_MSG_createQuit
(
	FTOM_MSG_PTR _PTR_ ppMsg
);

/**************************************************************
 * Time synchronization
 **************************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_ULONG		ulTime;
}	FTOM_MSG_TIME_SYNC, _PTR_ FTOM_MSG_TIME_SYNC_PTR;

FTM_RET	FTOM_MSG_createTimeSync
(
	FTM_ULONG			ulTime,
	FTOM_MSG_TIME_SYNC_PTR _PTR_ ppMsg
);

/**************************************************************
 * Connection management
 **************************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_ULONG		xObjectID;
	FTM_BOOL		bConnected;
}	FTOM_MSG_CONNECTION_STATUS, _PTR_ FTOM_MSG_CONNECTION_STATUS_PTR;

FTM_RET	FTOM_MSG_createConnectionStatus
(
	FTM_ULONG		xObjectID,
	FTM_BOOL		bConnected,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/**************************************************************
 * Report gateway status
 **************************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pGatewayID[FTM_DID_LEN+1];
	FTM_BOOL		bStatus;
	FTM_ULONG		ulTimeout;
}	FTOM_MSG_REPORT_GW_STATUS, _PTR_ FTOM_MSG_REPORT_GW_STATUS_PTR;

FTM_RET	FTOM_MSG_createReportGWStatus
(
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_REPORT_GW_STATUS_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_EP_INSERT_DATA, _PTR_ FTOM_MSG_EP_INSERT_DATA_PTR;


typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA		xData;
}	FTOM_MSG_ADD_EP_DATA, _PTR_ FTOM_MSG_ADD_EP_DATA_PTR;

FTM_RET	FTOM_MSG_createAddEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_ADD_EP_DATA_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_BOOL		bStatus;
	FTM_ULONG		ulTimeout;
}	FTOM_MSG_PUBLISH_EP_STATUS, _PTR_ FTOM_MSG_PUBLISH_EP_STATUS_PTR;

FTM_RET	FTOM_MSG_createPublishEPStatus
(
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_PUBLISH_EP_STATUS_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_PUBLISH_EP_DATA, _PTR_ FTOM_MSG_PUBLISH_EP_DATA_PTR;

FTM_RET	FTOM_MSG_createPublishEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_PUBLISH_EP_DATA_PTR _PTR_ ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_CTRL		xCtrl;
	FTM_ULONG		ulDuration;	
}	FTOM_MSG_EP_CTRL, _PTR_ FTOM_MSG_EP_CTRL_PTR;

FTM_RET FTOM_MSG_createEPCtrl
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTOM_MSG_EP_CTRL_PTR _PTR_ ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	FTM_RULE_STATE	xRuleState;
}	FTOM_MSG_RULE, _PTR_ FTOM_MSG_RULE_PTR;

FTM_RET FTOM_MSG_createRule
(
	FTM_CHAR_PTR		pRuleID,
	FTM_RULE_STATE		xRuleState,
	FTOM_MSG_RULE_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	FTM_BOOL		bActivate;
}	FTOM_MSG_ACTION, _PTR_ FTOM_MSG_ACTION_PTR;

FTM_RET FTOM_MSG_createAction
(
	FTM_CHAR_PTR		pActionID,
	FTM_BOOL			bActivate,
	FTOM_MSG_ACTION_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_BOOL		bStatus;
	FTM_ULONG		ulTimeout;
}	FTOM_MSG_SEND_EP_STATUS, _PTR_ FTOM_MSG_SEND_EP_STATUS_PTR;

FTM_RET	FTOM_MSG_createSendEPStatus
(
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_SEND_EP_STATUS_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_SEND_EP_DATA, _PTR_ FTOM_MSG_SEND_EP_DATA_PTR;

FTM_RET	FTOM_MSG_createSendEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_SEND_EP_DATA_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA		xData;
}	FTOM_MSG_ALERT, _PTR_ FTOM_MSG_ALERT_PTR;

FTM_RET	FTOM_MSG_createAlert
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTOM_MSG_ALERT_PTR _PTR_ ppMsg
);

/******************************************************
 * Gateway & Sensor discovery
 ******************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR_PTR	pNetwork;
	FTM_USHORT		usPort;
	FTM_ULONG		ulRetryCount;
}	FTOM_MSG_DISCOVERY, _PTR_ FTOM_MSG_DISCOVERY_PTR;

FTM_RET	FTOM_MSG_createDiscovery
(
	FTM_CHAR_PTR		pNetwork,
	FTM_USHORT			usPort,
	FTM_ULONG			ulRetryCount,
	FTOM_MSG_DISCOVERY_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pName[FTM_DEVICE_NAME_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pIP[32];
	FTM_ULONG		ulCount;
	FTM_EP_TYPE		pTypes[];
}	FTOM_MSG_DISCOVERY_INFO, _PTR_ FTOM_MSG_DISCOVERY_INFO_PTR;

FTM_RET	FTOM_MSG_createDiscoveryInfo
(
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pIP,
	FTM_EP_TYPE_PTR		pTypes,
	FTM_ULONG			ulCount,
	FTOM_MSG_DISCOVERY_INFO_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_ULONG		ulNodeCount;
	FTM_NODE_PTR	pNodeInfos;
	FTM_ULONG		ulEPCount;
	FTM_EP_PTR		pEPInfos;
}	FTOM_MSG_DISCOVERY_DONE, _PTR_ FTOM_MSG_DISCOVERY_DONE_PTR;

FTM_RET	FTOM_MSG_createDiscoveryDone
(
	FTM_NODE_PTR		pNodeInfos,
	FTM_ULONG			ulNodeCount,
	FTM_EP_PTR			pEPInfos,
	FTM_ULONG			ulEPInfos,
	FTOM_MSG_DISCOVERY_DONE_PTR _PTR_ ppMsg
);

/***********************************************
 * Server sync
 ***********************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_BOOL		bAutoRegister;
}	FTOM_MSG_SERVER_SYNC, _PTR_ FTOM_MSG_SERVER_SYNC_PTR;

FTM_RET	FTOM_MSG_createServerSync
(
	FTM_BOOL			bAutoRegister,
	FTOM_MSG_SERVER_SYNC_PTR _PTR_ 	ppMsg
);

/*************************************************
 * EP
 *************************************************/

FTM_RET	FTOM_MSG_EP_createInsertData
(
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

/************************************************
 * Event  
 ************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pTriggerID[FTM_ID_LEN+1];
	FTM_BOOL		bOccurred;
}	FTOM_MSG_EVENT, _PTR_ FTOM_MSG_EVENT_PTR;

FTM_RET	FTOM_MSG_createEvent
(
	FTM_CHAR_PTR	pTriggerID,
	FTM_BOOL		bOccurred,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/************************************************
 * Rule 
 ************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	FTM_BOOL		bActivation;
}	FTOM_MSG_RULE_ACTIVATION, _PTR_ FTOM_MSG_RULE_ACTIVATION_PTR;

FTM_RET	FTOM_MSG_RULE_createActivation
(
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL		bActivation,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/************************************************
 * Action
 ************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	FTM_BOOL		bActivation;
}	FTOM_MSG_ACTION_ACTIVATION, _PTR_ FTOM_MSG_ACTION_ACTIVATION_PTR;

FTM_RET	FTOM_MSG_ACTION_createActivation
(
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL		bActivation,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/************************************************
 * Thing+ 
 ************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_RESTART, _PTR_ FTOM_MSG_TP_REQ_RESTART_PTR;

FTM_RET	FTOM_MSG_TP_createReqRestart
(
	FTM_CHAR_PTR		pReqID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_ULONG		ulReportIntervalMS;
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL, _PTR_ FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR;

FTM_RET	FTOM_MSG_TP_createReqSetReportInterval
(
	FTM_CHAR_PTR		pReqID,
	FTM_ULONG			ulReportIntervalMS,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_ID		xMsgID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_CTRL		xCtrl;
	FTM_ULONG		ulDuration;	
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_CONTROL_ACTUATOR, _PTR_ FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR;

FTM_RET	FTOM_MSG_TP_createReqControlActuator
(
	FTM_CHAR_PTR		pReqID,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTM_CHAR_PTR	pMsgID;
	FTM_INT			nCode;
	FTM_CHAR_PTR	pMessage;
}	FTOM_MSG_TP_RESPONSE, _PTR_ FTOM_MSG_TP_RESPONSE_PTR;

FTM_RET	FTOM_MSG_TP_createResponse
(
	FTM_CHAR_PTR	pMsgID,
	FTM_INT			nCode,
	FTM_CHAR_PTR	pMessage,
	FTOM_MSG_PTR _PTR_	ppMsg
);


#endif
