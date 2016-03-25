#ifndef	__FTNM_CLIENT_H__
#define	__FTNM_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftnm_params.h"
#include "ftnm_client_config.h"

typedef	FTM_RET (*FTNM_CLIENT_NOTIFY_CALLBACK)(FTM_VOID_PTR);

typedef struct 
{
	FTNM_REQ_PARAMS_PTR		pReq;
	FTM_ULONG				ulReqLen;
	FTNM_RESP_PARAMS_PTR	pResp;
	FTM_ULONG				ulRespLen;
	sem_t					xDone;
}	FTNM_CLIENT_TRANS, _PTR_ FTNM_CLIENT_TRANS_PTR;

typedef	struct
{
	struct
	{
		FTM_CHAR	pHost[FTNM_CLIENT_SERVER_IP_LEN];
		FTM_USHORT	usPort;
	}	xServer;
}	FTNM_CLIENT_CONFIG, _PTR_ FTNM_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTNM_CLIENT_CONFIG	xConfig;

	pthread_t			xThread;
	FTM_MSG_QUEUE_PTR	pMsgQ;

	FTM_BOOL			bInit;
	FTM_BOOL			bStop;
	FTM_BOOL			bConnected;
	FTM_INT				hSock;
	FTM_ULONG			ulTimeout;
	FTM_ULONG			ulReqID;
	FTM_BOOL			bRequested;
	sem_t				xReqLock;
	FTM_LIST			xTransList;
	FTNM_CLIENT_NOTIFY_CALLBACK 	fNotifyCallback;
}	FTNM_CLIENT, _PTR_ FTNM_CLIENT_PTR;

FTM_RET	FTNM_CLIENT_init
(
	FTNM_CLIENT_PTR	pClient
);

FTM_RET	FTNM_CLIENT_final
(
	FTNM_CLIENT_PTR	pClient
);

FTM_RET	FTNM_CLIENT_create
(
	FTNM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTNM_CLIENT_destroy
(
	FTNM_CLIENT_PTR 	pClient
);

FTM_RET	FTNM_CLIENT_start
(
	FTNM_CLIENT_PTR	pClient
);

FTM_RET	FTNM_CLIENT_stop
(
	FTNM_CLIENT_PTR	pClient
);

FTM_RET	FTNM_CLIENT_ReadConfig
(
	FTNM_CLIENT_CONFIG_PTR 	pConfig, 
	FTM_CHAR_PTR 			pFileName
);

FTM_RET	FTNM_CLIENT_loadConfig
(
	FTNM_CLIENT_PTR			pClient,
	FTNM_CLIENT_CONFIG_PTR	pConfig
);


FTM_RET	FTNM_CLIENT_loadConfigFromFile
(
	FTNM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTNM_CLIENT_connect
(
	FTNM_CLIENT_PTR		pClient
);

FTM_RET FTNM_CLIENT_disconnect
(
	FTNM_CLIENT_PTR		pClient
);

FTM_RET FTNM_CLIENT_isConnected
(
	FTNM_CLIENT_PTR		pClient,
	FTM_BOOL_PTR		pbConnected
);

FTM_RET	FTNM_CLIENT_setNotifyCallback
(
	FTNM_CLIENT_PTR		pClient,
	FTNM_CLIENT_NOTIFY_CALLBACK	pCB
);

FTM_RET FTNM_CLIENT_NODE_create
(
	FTNM_CLIENT_PTR		pClient,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTNM_CLIENT_NODE_destroy
(
	FTNM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID
);

FTM_RET FTNM_CLIENT_NODE_count
(
	FTNM_CLIENT_PTR		pClient,
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTNM_CLIENT_NODE_getAt
(
	FTNM_CLIENT_PTR		pClient,
	FTM_ULONG			ulIndex,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTNM_CLIENT_NODE_get
(
	FTNM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTNM_CLIENT_EP_create
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_PTR			pInfo
);

FTM_RET FTNM_CLIENT_EP_destroy
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID
);

FTM_RET FTNM_CLIENT_EP_count
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTNM_CLIENT_EP_getList
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_EP_ID_PTR		pEPIDList,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTNM_CLIENT_EP_get
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTNM_CLIENT_EP_getAt
(
	FTNM_CLIENT_PTR		pClient,
	FTM_ULONG			nIndex,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTNM_CLIENT_EP_DATA_add
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
);

FTM_RET	FTNM_CLIENT_EP_DATA_getLast
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
);

FTM_RET FTNM_CLIENT_EP_DATA_getList
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTNM_CLIENT_EP_DATA_getWithTime
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTNM_CLIENT_EP_DATA_del
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nIndex,
	FTM_ULONG			nCount
);

FTM_RET FTNM_CLIENT_EP_DATA_delWithTime
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime
);

FTM_RET	FTNM_CLIENT_EP_DATA_count
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pCount
);

#endif

