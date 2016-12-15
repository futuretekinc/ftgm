#ifndef	_FTOM_MSG_H_
#define	_FTOM_MSG_H_

#include "ftm.h"

#define	FTOM_MSG_STRING_LENGTH	1024
#define	FTOM_MSG_REQ_ID_LENGTH	32

typedef	FTM_ULONG		FTOM_MSG_ID, _PTR_ FTOM_MSG_ID_PTR;
typedef	FTM_VOID_PTR	FTOM_MSG_SENDER_ID, _PTR_ FTOM_MSG_SENDER_ID_PTR;

typedef	enum
{
	FTOM_MSG_TYPE_QUIT	=	0,
	FTOM_MSG_TYPE_INITIALIZE_DONE,

	FTOM_MSG_TYPE_CONNECTED,
	FTOM_MSG_TYPE_DISCONNECTED,
	FTOM_MSG_TYPE_CONNECTION_STATUS,

	FTOM_MSG_TYPE_GW_STATUS,

	FTOM_MSG_TYPE_EP_STATUS,
	FTOM_MSG_TYPE_EP_DATA,
	FTOM_MSG_TYPE_EP_CTRL,
	FTOM_MSG_TYPE_EP_DATA_SERVER_TIME,

	FTOM_MSG_TYPE_DISCOVERY,
	FTOM_MSG_TYPE_DISCOVERY_INFO,
	FTOM_MSG_TYPE_DISCOVERY_DONE,

	FTOM_MSG_TYPE_SERVER_SYNC,
	FTOM_MSG_TYPE_TIME_SYNC,

	FTOM_MSG_TYPE_ALERT,

	FTOM_MSG_TYPE_EVENT,

	FTOM_MSG_TYPE_ACTION,
	FTOM_MSG_TYPE_ACTION_ACTIVATION,

	FTOM_MSG_TYPE_RULE,
	FTOM_MSG_TYPE_RULE_ACTIVATION,

	FTOM_MSG_TYPE_TP_REQ_SET_REPORT_INTERVAL,
	FTOM_MSG_TYPE_TP_REQ_RESTART,
	FTOM_MSG_TYPE_TP_REQ_REBOOT,
	FTOM_MSG_TYPE_TP_REQ_POWER_OFF,
	FTOM_MSG_TYPE_TP_REQ_CONTROL_ACTUATOR,
	FTOM_MSG_TYPE_TP_RESPONSE,
	FTOM_MSG_TYPE_TP_REPORT,
	
	FTOM_MSG_TYPE_SNMPC_GET_EP_DATA,
	FTOM_MSG_TYPE_SNMPC_SET_EP_DATA,

	FTOM_MSG_TYPE_NET_STAT,

	FTOM_MSG_TYPE_MAX
}	FTOM_MSG_TYPE, _PTR_ FTOM_MSG_TYPE_PTR;

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
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

FTM_CHAR_PTR	FTOM_MSG_printType
(
	FTOM_MSG_PTR	pMsg
);
/**************************************************************
 * System initialize done 
 **************************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
}	FTOM_MSG_INITIALIZE_DONE, _PTR_ FTOM_MSG_INITIALIZE_DONE_PTR;

FTM_RET FTOM_MSG_createInitializeDone
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/**************************************************************
 * Quit
 **************************************************************/
FTM_RET FTOM_MSG_createQuit
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/**************************************************************
 * Time synchronization
 **************************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_ULONG		ulTime;
}	FTOM_MSG_TIME_SYNC, _PTR_ FTOM_MSG_TIME_SYNC_PTR;

FTM_RET	FTOM_MSG_createTimeSync
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_ULONG		ulTime,
	FTOM_MSG_TIME_SYNC_PTR _PTR_ ppMsg
);

/**************************************************************
 * Connection management
 **************************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_ULONG		xObjectID;
	FTM_BOOL		bConnected;
}	FTOM_MSG_CONNECTION_STATUS, _PTR_ FTOM_MSG_CONNECTION_STATUS_PTR;

FTM_RET	FTOM_MSG_createConnectionStatus
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_ULONG		xObjectID,
	FTM_BOOL		bConnected,
	FTOM_MSG_PTR _PTR_ ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
}	FTOM_MSG_CONNECTED, _PTR_ FTOM_MSG_CONNECTED_PTR;

FTM_RET	FTOM_MSG_createConnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
}	FTOM_MSG_DISCONNECTED, _PTR_ FTOM_MSG_DISCONNECTED_PTR;

FTM_RET	FTOM_MSG_createDisconnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/**************************************************************
 * Report gateway status
 **************************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pGatewayID[FTM_DID_LEN+1];
	FTM_BOOL		bStatus;
	FTM_ULONG		ulTimeout;
}	FTOM_MSG_GW_STATUS, _PTR_ FTOM_MSG_GW_STATUS_PTR;

FTM_RET	FTOM_MSG_createGWStatus
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pGatewayID,
	FTM_BOOL		bStatus,
	FTM_ULONG		ulTimeout,
	FTOM_MSG_GW_STATUS_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSensorID;
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_EP_INSERT_DATA, _PTR_ FTOM_MSG_EP_INSERT_DATA_PTR;


typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_CTRL		xCtrl;
	FTM_ULONG		ulDuration;	
}	FTOM_MSG_EP_CTRL, _PTR_ FTOM_MSG_EP_CTRL_PTR;

FTM_RET FTOM_MSG_createEPCtrl
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_CTRL		xCtrl,
	FTM_ULONG		ulDuration,
	FTOM_MSG_EP_CTRL_PTR _PTR_ ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulTime;	
}	FTOM_MSG_EP_DATA_SERVER_TIME, _PTR_ FTOM_MSG_EP_DATA_SERVER_TIME_PTR;

FTM_RET FTOM_MSG_createEPDataServerTime
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulTime,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	FTM_RULE_STATE	xRuleState;
}	FTOM_MSG_RULE, _PTR_ FTOM_MSG_RULE_PTR;

FTM_RET FTOM_MSG_createRule
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_STATE	xRuleState,
	FTOM_MSG_RULE_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSensorID;
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	FTM_BOOL		bActivate;
}	FTOM_MSG_ACTION, _PTR_ FTOM_MSG_ACTION_PTR;

FTM_RET FTOM_MSG_createAction
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL		bActivate,
	FTOM_MSG_ACTION_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_BOOL		bStatus;
	FTM_ULONG		ulTimeout;
}	FTOM_MSG_EP_STATUS, _PTR_ FTOM_MSG_EP_STATUS_PTR;

FTM_RET	FTOM_MSG_createEPStatus
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL		bStatus,
	FTM_ULONG		ulTimeout,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulCount;
	FTM_EP_DATA		pData[];
}	FTOM_MSG_EP_DATA, _PTR_ FTOM_MSG_EP_DATA_PTR;

FTM_RET	FTOM_MSG_createEPData
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulCount,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA		xData;
}	FTOM_MSG_ALERT, _PTR_ FTOM_MSG_ALERT_PTR;

FTM_RET	FTOM_MSG_createAlert
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData,
	FTOM_MSG_ALERT_PTR _PTR_ ppMsg
);

/******************************************************
 * Gateway & Sensor discovery
 ******************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR_PTR	pNetwork;
	FTM_USHORT		usPort;
	FTM_ULONG		ulRetryCount;
}	FTOM_MSG_DISCOVERY, _PTR_ FTOM_MSG_DISCOVERY_PTR;

FTM_RET	FTOM_MSG_createDiscovery
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount,
	FTOM_MSG_DISCOVERY_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pName[FTM_DEVICE_NAME_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pIP[32];
	FTM_ULONG		ulCount;
	FTM_EP_TYPE		pTypes[];
}	FTOM_MSG_DISCOVERY_INFO, _PTR_ FTOM_MSG_DISCOVERY_INFO_PTR;

FTM_RET	FTOM_MSG_createDiscoveryInfo
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount,
	FTOM_MSG_DISCOVERY_INFO_PTR _PTR_ ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_ULONG		ulNodeCount;
	FTM_NODE_PTR	pNodeInfos;
	FTM_ULONG		ulEPCount;
	FTM_EP_PTR		pEPInfos;
}	FTOM_MSG_DISCOVERY_DONE, _PTR_ FTOM_MSG_DISCOVERY_DONE_PTR;

FTM_RET	FTOM_MSG_createDiscoveryDone
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_NODE_PTR	pNodeInfos,
	FTM_ULONG		ulNodeCount,
	FTM_EP_PTR		pEPInfos,
	FTM_ULONG		ulEPInfos,
	FTOM_MSG_DISCOVERY_DONE_PTR _PTR_ ppMsg
);

/***********************************************
 * Server sync
 ***********************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_BOOL		bAutoRegister;
}	FTOM_MSG_SERVER_SYNC, _PTR_ FTOM_MSG_SERVER_SYNC_PTR;

FTM_RET	FTOM_MSG_createServerSync
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_BOOL		bAutoRegister,
	FTOM_MSG_SERVER_SYNC_PTR _PTR_ 	ppMsg
);

/************************************************
 * Event  
 ************************************************/
typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pTriggerID[FTM_ID_LEN+1];
	FTM_BOOL		bOccurred;
}	FTOM_MSG_EVENT, _PTR_ FTOM_MSG_EVENT_PTR;

FTM_RET	FTOM_MSG_createEvent
(
	FTOM_MSG_SENDER_ID	xSenderID,
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
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	FTM_BOOL		bActivation;
}	FTOM_MSG_RULE_ACTIVATION, _PTR_ FTOM_MSG_RULE_ACTIVATION_PTR;

FTM_RET	FTOM_MSG_RULE_createActivation
(
	FTOM_MSG_SENDER_ID	xSenderID,
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
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	FTM_BOOL		bActivation;
}	FTOM_MSG_ACTION_ACTIVATION, _PTR_ FTOM_MSG_ACTION_ACTIVATION_PTR;

FTM_RET	FTOM_MSG_ACTION_createActivation
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL		bActivation,
	FTOM_MSG_PTR _PTR_ ppMsg
);

/************************************************
 * SNMP Client
 ************************************************/

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_CHAR		pEPID[FTM_DID_LEN+1];
	FTM_ULONG		ulVersion;
	FTM_CHAR		pURL[FTM_URL_LEN+1];
	FTM_CHAR		pCommunity[FTM_SNMP_COMMUNITY_LEN+1];
	FTM_SNMP_OID	xOID;
	FTM_VALUE_TYPE	xDataType;
	FTM_ULONG		ulTimeout;
}	FTOM_MSG_SNMPC_GET_EP_DATA, _PTR_ FTOM_MSG_SNMPC_GET_EP_DATA_PTR;

FTM_RET	FTOM_MSG_SNMPC_createGetEPData
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTM_CHAR_PTR		pDID,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_VALUE_TYPE		xDataType,
	FTM_ULONG			ulTimeout,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_CHAR		pEPID[FTM_DID_LEN+1];
	FTM_ULONG		ulVersion;
	FTM_CHAR		pURL[FTM_URL_LEN+1];
	FTM_CHAR		pCommunity[FTM_SNMP_COMMUNITY_LEN+1];
	FTM_SNMP_OID	xOID;
	FTM_ULONG		ulTimeout;
	FTM_VALUE		xValue;
}	FTOM_MSG_SNMPC_SET_EP_DATA, _PTR_ FTOM_MSG_SNMPC_SET_EP_DATA_PTR;

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
);

/************************************************
 * Network management
 ************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_BOOL		bConnected;
}	FTOM_MSG_NET_STAT, _PTR_ FTOM_MSG_NET_STAT_PTR;

FTM_RET	FTOM_MSG_createNetConnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

FTM_RET	FTOM_MSG_createNetDisconnected
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

/************************************************
 * Thing+ 
 ************************************************/
typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_REBOOT, _PTR_ FTOM_MSG_TP_REQ_REBOOT_PTR;

FTM_RET	FTOM_MSG_TP_createReqReboot
(
	FTOM_MSG_SENDER_ID		xSenderID,
	FTM_CHAR_PTR		pReqID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_POWER_OFF, _PTR_ FTOM_MSG_TP_REQ_POWER_OFF_PTR;

FTM_RET	FTOM_MSG_TP_createReqPowerOff
(
	FTOM_MSG_SENDER_ID		xSenderID,
	FTM_CHAR_PTR		pReqID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_RESTART, _PTR_ FTOM_MSG_TP_REQ_RESTART_PTR;

FTM_RET	FTOM_MSG_TP_createReqRestart
(
	FTOM_MSG_SENDER_ID		xSenderID,
	FTM_CHAR_PTR		pReqID,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_ULONG		ulReportIntervalMS;
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL, _PTR_ FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR;

FTM_RET	FTOM_MSG_TP_createReqSetReportInterval
(
	FTOM_MSG_SENDER_ID		xSenderID,
	FTM_CHAR_PTR		pReqID,
	FTM_ULONG			ulReportIntervalMS,
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

typedef struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_CTRL		xCtrl;
	FTM_ULONG		ulDuration;	
	FTM_CHAR		pReqID[];
}	FTOM_MSG_TP_REQ_CONTROL_ACTUATOR, _PTR_ FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR;

FTM_RET	FTOM_MSG_TP_createReqControlActuator
(
	FTOM_MSG_SENDER_ID		xSenderID,
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
	FTOM_MSG_SENDER_ID	xSenderID;
	FTM_CHAR_PTR	pMsgID;
	FTM_INT			nCode;
	FTM_CHAR_PTR	pMessage;
}	FTOM_MSG_TP_RESPONSE, _PTR_ FTOM_MSG_TP_RESPONSE_PTR;

FTM_RET	FTOM_MSG_TP_createResponse
(
	FTOM_MSG_SENDER_ID		xSenderID,
	FTM_CHAR_PTR	pMsgID,
	FTM_INT			nCode,
	FTM_CHAR_PTR	pMessage,
	FTOM_MSG_PTR _PTR_	ppMsg
);

typedef	struct
{
	FTOM_MSG_TYPE	xType;
	FTM_ULONG		ulLen;
}	FTOM_MSG_TP_REPORT, _PTR_ FTOM_MSG_TP_REPORT_PTR;

FTM_RET	FTOM_MSG_TP_createReport
(
	FTOM_MSG_SENDER_ID	xSenderID,
	FTOM_MSG_PTR _PTR_	ppMsg
);

/****************************************************************
 *
 ****************************************************************/
FTM_RET	FTOM_MSG_JSON_printEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			ulBuffLen
);
#endif
