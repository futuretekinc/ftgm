#ifndef	__FTDM_SIS_H__
#define	__FTDM_SIS_H__

#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm_types.h"
#include "ftdm_params.h"


#define	FTDM_PACKET_LEN					2048

typedef struct FTDM_SIS_STRUCT _PTR_ FTDM_SIS_PTR;
typedef struct FTDM_SESSION_STRUCT _PTR_ FTDM_SESSION_PTR;

typedef	FTM_RET	(*FTDM_SERVICE_CALLBACK)(FTDM_SIS_PTR pServer, FTDM_REQ_PARAMS_PTR, FTDM_RESP_PARAMS_PTR);

FTM_RET	FTDM_SIS_create
(
	FTDM_PTR	pFTDM,
	FTDM_SIS_PTR _PTR_ ppSIS
);

FTM_RET	FTDM_SIS_destroy
(
	FTDM_SIS_PTR _PTR_ ppSIS
);

FTM_RET	FTDM_SIS_init
(
	FTDM_SIS_PTR	pServer
);

FTM_RET	FTDM_SIS_final
(
	FTDM_SIS_PTR	pServer
);

FTM_RET	FTDM_SIS_loadConfig
(
	FTDM_SIS_PTR			pServer,
	FTM_CONFIG_ITEM_PTR		pConfig
);

FTM_RET	FTDM_SIS_start
(
	FTDM_SIS_PTR	pServer
);

FTM_RET	FTDM_SIS_stop
(
	FTDM_SIS_PTR	pServer
);

FTM_RET	FTDM_SIS_waitingForFinished
(
	FTDM_SIS_PTR	pServer
);

FTM_RET	FTDM_SIS_serviceCall
(
	FTDM_SIS_PTR			pServer,
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_createSession
(
	FTDM_SIS_PTR pServer,
	FTM_INT			hClient,
	struct sockaddr *pSockAddr,
	FTDM_SESSION_PTR _PTR_ ppSession
);

FTM_RET	FTDM_SIS_destroySession
(
	FTDM_SIS_PTR pServer,
	FTDM_SESSION_PTR _PTR_ ppSession
);

FTM_RET	FTDM_SIS_getSessionCount
(
	FTDM_SIS_PTR pServer, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTDM_SIS_getSessionInfo
(
	FTDM_SIS_PTR pServer, 
	FTM_ULONG 			ulIndex, 
	FTDM_SESSION_PTR 	pSession
);

FTM_RET	FTDM_SIS_addNode
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_NODE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteNode
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_NODE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDM_SIS_getNodeCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getNode
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getNodeAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setNode
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_SET_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getNodeIDList
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_GET_DID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_DID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getNodeType
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getNodeURL
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setNodeURL
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_SET_NODE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_NODE_URL_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getNodeLocation
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_GET_NODE_LOCATION_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_LOCATION_PARAMS_PTR 	pResp
);

FTM_RET	FTDM_SIS_setNodeLocation
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_SET_NODE_LOCATION_PARAMS_PTR	pReq,
 	FTDM_RESP_SET_NODE_LOCATION_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addEP
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteEP
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DEL_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEP
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setEP
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_SET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPIDList
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_GET_EPID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_EPID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addEPClass
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_CLASS_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteEPClass
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_CLASS_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_DEL_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPClassCount
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_CLASS_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_EP_CLASS_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPClass
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_CLASS_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPClassAt
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_CLASS_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addEPData
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPDataInfo
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_INFO_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setEPDataLimit
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_DATA_SET_LIMIT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_SET_LIMIT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPData
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getEPDataWithTime
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET 	FTDM_SIS_deleteEPData
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR	pResp
);

FTM_RET 	FTDM_SIS_deleteEPDataWithTime
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET 	FTDM_SIS_getEPDataCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR	pResp
);

FTM_RET 	FTDM_SIS_getEPDataCountWithTime
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addTrigger
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_TRIGGER_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteTrigger
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_TRIGGER_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDM_SIS_getTriggerCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getTrigger
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getTriggerAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setTrigger
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_SET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getTriggerIDList
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addAction
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_ACTION_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteAction
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_ACTION_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDM_SIS_getActionCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getAction
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_GET_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getActionAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setAction
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_SET_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getActionIDList
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_ID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addRule
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteRule
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDM_SIS_getRuleCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getRule
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_GET_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getRuleAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_GET_AT_PARAMS_PTR		pReq,
	FTDM_RESP_RULE_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_setRule
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_SET_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getRuleIDList
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_ID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_addLog
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_LOG_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_deleteLog
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_LOG_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDM_SIS_getLogCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_LOG_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getLog
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_LOG_GET_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDM_SIS_getLogAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_LOG_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_GET_AT_PARAMS_PTR	pResp
);


#endif
