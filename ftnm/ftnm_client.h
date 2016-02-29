#ifndef	__FTNM_CLIENT_H__
#define	__FTNM_CLIENT_H__

#include <semaphore.h>
#include "ftm.h"
#include "ftnm_params.h"
#include "ftnm_client_config.h"

typedef	FTM_RET (*FTNMC_NOTIFY_CALLBACK)(FTM_VOID_PTR);

typedef struct 
{
	FTNM_REQ_PARAMS_PTR		pReq;
	FTM_ULONG				ulReqLen;
	FTNM_RESP_PARAMS_PTR	pResp;
	FTM_ULONG				ulRespLen;
	sem_t					xDone;
}	FTNMC_TRANS, _PTR_ FTNMC_TRANS_PTR;

typedef	struct
{
	FTM_INT					hSock;
	FTM_ULONG				ulTimeout;
	FTM_ULONG				ulReqID;
	FTM_BOOL				bRequested;
	sem_t					xReqLock;
	FTM_LIST				xTransList;
	FTNMC_NOTIFY_CALLBACK 	fNotifyCallback;
}	FTNMC_SESSION, _PTR_ FTNMC_SESSION_PTR;

FTM_RET	FTNMC_init
(
	FTNM_CFG_CLIENT_PTR	pConfig
);

FTM_RET	FTNMC_final
(
	FTM_VOID
);

FTM_RET	FTNMC_SESSION_create
(
	FTNMC_SESSION_PTR _PTR_ ppSession
);

FTM_RET	FTNMC_SESSION_destroy
(
	FTNMC_SESSION_PTR 		pSession
);

FTM_RET	FTNMC_SESSION_connect
(
	FTNMC_SESSION_PTR		pSession,
	FTM_IP_ADDR				xIP,
	FTM_USHORT				usPort
);

FTM_RET FTNMC_SESSION_disconnect
(
	FTNMC_SESSION_PTR		pSession
);

FTM_RET FTNMC_SESSION_isConnected
(
	FTNMC_SESSION_PTR		pSession,
	FTM_BOOL_PTR			pbConnected
);

FTM_RET	FTNMC_setNotifyCallback
(
	FTNMC_SESSION_PTR		pSession,
	FTNMC_NOTIFY_CALLBACK	pCB
);

FTM_RET FTNMC_NODE_create
(
	FTNMC_SESSION_PTR		pSession,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTNMC_NODE_destroy
(
	FTNMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
);

FTM_RET FTNMC_NODE_count
(
	FTNMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET FTNMC_NODE_getAt
(
	FTNMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTNMC_NODE_get
(
	FTNMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTNMC_EP_create
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_PTR			pInfo
);

FTM_RET FTNMC_EP_destroy
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID
);

FTM_RET FTNMC_EP_count
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTNMC_EP_getList
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_TYPE			xType,
	FTM_EP_ID_PTR			pEPIDList,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTNMC_EP_get
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTNMC_EP_getAt
(
	FTNMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTNMC_EP_DATA_add
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pEPData
);

FTM_RET	FTNMC_EP_DATA_getLast
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pEPData
);

FTM_RET FTNMC_EP_DATA_getList
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nStartIndex,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTNMC_EP_DATA_getWithTime
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTNMC_EP_DATA_del
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
);

FTM_RET FTNMC_EP_DATA_delWithTime
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime
);

FTM_RET	FTNMC_EP_DATA_count
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pCount
);

#endif

