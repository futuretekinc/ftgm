#ifndef	__FTDM_SERVER_H__
#define	__FTDM_SERVER_H__

#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftm_types.h"

typedef	FTM_RET	(*FTDM_SERVICE_CALLBACK)(FTDM_REQ_PARAMS_PTR, FTDM_RESP_PARAMS_PTR);

#define	FTDM_PACKET_LEN					2048

typedef	struct
{
	pthread_t 			xPthread;	
	FTM_INT				hSocket;
	sem_t				xSemaphore;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;



FTM_RET	FTDMS_run
(
	FTDM_CFG_SERVER_PTR 	pConfig,
	pthread_t 				*pPThread 
);

FTM_RET	FTDMS_serviceCall
(
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getSessionCount
(	
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTDMS_getSessionInfo
(
	FTM_ULONG 			ulIndex, 
	FTDM_SESSION_PTR 	pSession
);

FTM_RET	FTDMS_NODE_INFO_add
(
	FTDM_REQ_NODE_INFO_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_INFO_del
(
	FTDM_REQ_NODE_INFO_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_NODE_INFO_count
(
 	FTDM_REQ_NODE_INFO_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_INFO_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_INFO_get
(
 	FTDM_REQ_NODE_INFO_GET_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_NODE_INFO_getAt
(
 	FTDM_REQ_NODE_INFO_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getNodeType
(
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getNodeURL
(
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_setNodeURL
(
 	FTDM_REQ_SET_NODE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_NODE_URL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_getNodeLocation
(
 	FTDM_REQ_GET_NODE_LOCATION_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_LOCATION_PARAMS_PTR 	pResp
);

FTM_RET	FTDMS_setNodeLocation
(
 	FTDM_REQ_SET_NODE_LOCATION_PARAMS_PTR	pReq,
 	FTDM_RESP_SET_NODE_LOCATION_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_INFO_add
(
 	FTDM_REQ_EP_INFO_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_INFO_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_INFO_del
(
 	FTDM_REQ_EP_INFO_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_EP_INFO_DEL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_INFO_count
(
 	FTDM_REQ_EP_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_INFO_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_INFO_get
(
 	FTDM_REQ_EP_INFO_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_INFO_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_INFO_getAt
(
 	FTDM_REQ_EP_INFO_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_INFO_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_INFO_add
(
	FTDM_REQ_EP_CLASS_INFO_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_INFO_del
(
	FTDM_REQ_EP_CLASS_INFO_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_DEL_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_INFO_count
(
	FTDM_REQ_EP_CLASS_INFO_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_EP_CLASS_INFO_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_INFO_get
(
	FTDM_REQ_EP_CLASS_INFO_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_CLASS_INFO_getAt
(
	FTDM_REQ_EP_CLASS_INFO_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_add
(
 	FTDM_REQ_EP_DATA_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_info
(
 	FTDM_REQ_EP_DATA_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_INFO_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_get
(
 	FTDM_REQ_EP_DATA_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EP_DATA_getWithTime
(
 	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_del
(
 	FTDM_REQ_EP_DATA_DEL_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_delWithTime
(
 	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_count
(
 	FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR	pResp
);

FTM_RET 	FTDMS_EP_DATA_countWithTime
(
 	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EVENT_add
(
	FTDM_REQ_EVENT_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_ADD_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EVENT_del
(
	FTDM_REQ_EVENT_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_DEL_PARAMS_PTR	presp
);

FTM_RET	FTDMS_EVENT_count
(
 	FTDM_REQ_EVENT_COUNT_PARAMS_PTR		pReq,
	FTDM_RESP_EVENT_COUNT_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EVENT_get
(
 	FTDM_REQ_EVENT_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_GET_PARAMS_PTR	pResp
);

FTM_RET	FTDMS_EVENT_getAt
(
 	FTDM_REQ_EVENT_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_GET_AT_PARAMS_PTR	pResp
);

#endif
