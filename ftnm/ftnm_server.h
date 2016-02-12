#ifndef	__FTNM_SERVER_H__
#define	__FTNM_SERVER_H__

#include <semaphore.h>
#include <pthread.h>
#include "ftnm_params.h"

typedef	FTM_RET	(*FTNM_SERVICE_CALLBACK)(FTNM_REQ_PARAMS_PTR, FTNM_RESP_PARAMS_PTR);

typedef struct
{
	FTNM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTNM_SERVICE_CALLBACK	fService;
}	FTNM_SRV_CMD_SET, _PTR_ FTNM_SRV_CMD_SET_PTR;

typedef	struct
{
	FTM_USHORT		usPort;
	FTM_ULONG		ulMaxSession;
}	FTNM_SRV_CONFIG, _PTR_ FTNM_SRV_CONFIG_PTR;


typedef	struct
{
	FTNM_SRV_CONFIG	xConfig;
	pthread_t 		xPThread;
	sem_t			xSemaphore;

	FTM_LIST		xSessionList;
}	FTNM_SERVER, _PTR_ FTNM_SERVER_PTR;

FTM_RET	FTNM_SRV_init(FTNM_SERVER_PTR 		pSRV);
FTM_RET	FTNM_SRV_final(FTNM_SERVER_PTR 		pSRV);

FTM_RET	FTNM_SRV_run(FTNM_SERVER_PTR		pSRV);

FTM_RET	FTNM_SRV_initConfig(FTNM_SERVER_PTR pSRV);
FTM_RET	FTNM_SRV_finalConfig(FTNM_SERVER_PTR pSRV);
FTM_RET	FTNM_SRV_loadConfig(FTNM_SERVER_PTR pSRV, FTM_CHAR_PTR	pFileName);
FTM_RET	FTNM_SRV_showConfig(FTNM_SERVER_PTR pSRV);

FTM_RET	FTNM_SRV_serviceCall
(
	FTNM_REQ_PARAMS_PTR		pReq,
	FTNM_RESP_PARAMS_PTR	pResp
);

FTM_RET	FTNM_SRV_NODE_create
(
	FTNM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_CREATE_PARAMS_PTR	pResp
);

FTM_RET	FTNM_SRV_NODE_destroy
(
	FTNM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTNM_RESP_NODE_DESTROY_PARAMS_PTR	presp
);

FTM_RET	FTNM_SRV_NODE_count
(
 	FTNM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_COUNT_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_NODE_get
(
 	FTNM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_GET_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_NODE_getAt
(
 	FTNM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTNM_SRV_EP_create
(
 	FTNM_REQ_EP_CREATE_PARAMS_PTR		pReq,
	FTNM_RESP_EP_CREATE_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_EP_destroy
(
 	FTNM_REQ_EP_DESTROY_PARAMS_PTR		pReq,
	FTNM_RESP_EP_DESTROY_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_EP_count
(
 	FTNM_REQ_EP_COUNT_PARAMS_PTR		pReq,
	FTNM_RESP_EP_COUNT_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_EP_getList
(
 	FTNM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTNM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
);

FTM_RET	FTNM_SRV_EP_get
(
 	FTNM_REQ_EP_GET_PARAMS_PTR		pReq,
	FTNM_RESP_EP_GET_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_EP_getAt
(
 	FTNM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTNM_RESP_EP_GET_AT_PARAMS_PTR	pResp
);

FTM_RET	FTNM_SRV_EP_DATA_info
(
	FTNM_REQ_EP_DATA_INFO_PARAMS_PTR pReq,
	FTNM_RESP_EP_DATA_INFO_PARAMS_PTR pResp
);

FTM_RET	FTNM_SRV_EP_DATA_count
(
 	FTNM_REQ_EP_DATA_COUNT_PARAMS_PTR		pReq,
	FTNM_RESP_EP_DATA_COUNT_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_EP_DATA_getLast
(
 	FTNM_REQ_EP_DATA_GET_LAST_PARAMS_PTR		pReq,
	FTNM_RESP_EP_DATA_GET_LAST_PARAMS_PTR		pResp
);

FTM_RET	FTNM_SRV_EP_DATA_getList
(
 	FTNM_REQ_EP_DATA_GET_LIST_PARAMS_PTR		pReq,
	FTNM_RESP_EP_DATA_GET_LIST_PARAMS_PTR		pResp
);

#endif
