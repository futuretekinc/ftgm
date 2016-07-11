#ifndef	__FTDM_SERVER_H__
#define	__FTDM_SERVER_H__

#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftm_types.h"
#include "ftdm_params.h"


#define	FTDM_PACKET_LEN					2048

typedef	struct
{
	struct FTDM_CONTEXT_STRUCT *pDM;
	FTDM_CFG_SERVER		xConfig;
	pthread_t 			*pThread ;
	FTM_BOOL			bStop;
	sem_t				xSemaphore;
	pthread_t			xThread;
	FTM_INT				hSocket;
	FTM_LIST			xSessionList;
}	FTDM_SERVER, _PTR_ FTDM_SERVER_PTR;

typedef	struct
{
	FTDM_SERVER_PTR		pServer;
	pthread_t 			xThread;	
	FTM_INT				hSocket;
	FTM_BOOL			bStop;
	sem_t				xSemaphore;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTDM_PACKET_LEN];
	FTM_TIME			xStartTime;
	FTM_TIME			xLastTime;
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

typedef	FTM_RET	(*FTDM_SERVICE_CALLBACK)(FTDM_SERVER_PTR pServer, FTDM_REQ_PARAMS_PTR, FTDM_RESP_PARAMS_PTR);

FTM_RET	FTDMS_init
(
	FTDM_SERVER_PTR			pServer,
	struct FTDM_CONTEXT_STRUCT *pParent
);

FTM_RET	FTDMS_final
(
	FTDM_SERVER_PTR			pServer
);

FTM_RET	FTDMS_loadConfig
(
	FTDM_SERVER_PTR			pServer,
	FTDM_CFG_SERVER_PTR		pConfig
);

FTM_RET	FTDMS_loadFromFile
(
	FTDM_SERVER_PTR			pServer,
	FTM_CHAR_PTR			pFileName
);

FTM_RET	FTDMS_start
(
	FTDM_SERVER_PTR			pServer
);

FTM_RET	FTDMS_stop
(
	FTDM_SERVER_PTR			pServer
);

FTM_RET	FTDMS_waitingForFinished
(
	FTDM_SERVER_PTR			pServer
);

FTM_RET	FTDMS_serviceCall
(
	FTDM_SERVER_PTR			pServer,
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_createSession
(
	FTDM_SERVER_PTR pServer,
	FTM_INT			hClient,
	struct sockaddr *pSockAddr,
	FTDM_SESSION_PTR _PTR_ ppSession
);

FTM_RET	FTDMS_destroySession
(
	FTDM_SERVER_PTR pServer,
	FTDM_SESSION_PTR _PTR_ ppSession
);

FTM_RET	FTDMS_getSessionCount
(
	FTDM_SERVER_PTR pServer, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTDMS_getSessionInfo
(
	FTDM_SERVER_PTR pServer, 
	FTM_ULONG 			ulIndex, 
	FTDM_SESSION_PTR 	pSession
);

FTM_RET	FTDMS_NODE_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_NODE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_del
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_NODE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_NODE_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_SET_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_getDIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_GET_DID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_DID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getNodeType
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getNodeURL
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_setNodeURL
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_SET_NODE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_NODE_URL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getNodeLocation
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_GET_NODE_LOCATION_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_LOCATION_PARAMS_PTR 	pResp
);

FTM_RET	FTDMS_setNodeLocation
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_SET_NODE_LOCATION_PARAMS_PTR	pReq,
 	FTDM_RESP_SET_NODE_LOCATION_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_add
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DEL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_SET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_getEPIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_GET_EPID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_EPID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_CLASS_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_del
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_CLASS_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_DEL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_count
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_CLASS_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_EP_CLASS_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_get
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_CLASS_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_getAt
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_CLASS_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_add
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_info
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_INFO_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_setLimit
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_DATA_SET_LIMIT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_SET_LIMIT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_getWithTime
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_delWithTime
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_countWithTime
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_TRIGGER_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_TRIGGER_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_TRIGGER_del
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_TRIGGER_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_TRIGGER_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_TRIGGER_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_TRIGGER_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_TRIGGER_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_SET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_TRIGGER_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_ACTION_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_ACTION_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_ACTION_del
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_ACTION_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_ACTION_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_ACTION_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_GET_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_ACTION_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_ACTION_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_SET_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_ACTION_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_ID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_RULE_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_RULE_del
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_RULE_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_RULE_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_GET_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_RULE_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_GET_AT_PARAMS_PTR		pReq,
	FTDM_RESP_RULE_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_RULE_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_SET_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_SET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_RULE_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_ID_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_LOG_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_LOG_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_LOG_del
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_LOG_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_LOG_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_LOG_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_LOG_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_LOG_GET_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_LOG_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_LOG_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_GET_AT_PARAMS_PTR	pResp
);


#endif
