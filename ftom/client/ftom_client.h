#ifndef	__FTOM_CLIENT_H__
#define	__FTOM_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom_params.h"
#include "ftom_client_config.h"

typedef	FTM_RET (*FTOM_CLIENT_NOTIFY_CALLBACK)(FTM_VOID_PTR);

typedef struct 
{
	FTOM_REQ_PARAMS_PTR		pReq;
	FTM_ULONG				ulReqLen;
	FTOM_RESP_PARAMS_PTR	pResp;
	FTM_ULONG				ulRespLen;
	sem_t					xDone;
}	FTOM_CLIENT_TRANS, _PTR_ FTOM_CLIENT_TRANS_PTR;

typedef	struct
{
	struct
	{
		FTM_CHAR	pHost[FTOM_CLIENT_SERVER_IP_LEN];
		FTM_USHORT	usPort;
	}	xServer;
}	FTOM_CLIENT_CONFIG, _PTR_ FTOM_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTOM_CLIENT_CONFIG	xConfig;

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
	FTOM_CLIENT_NOTIFY_CALLBACK 	fNotifyCallback;
}	FTOM_CLIENT, _PTR_ FTOM_CLIENT_PTR;

FTM_RET	FTOM_CLIENT_init
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_final
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_create
(
	FTOM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_destroy
(
	FTOM_CLIENT_PTR 	pClient
);

FTM_RET	FTOM_CLIENT_start
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_stop
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_ReadConfig
(
	FTOM_CLIENT_CONFIG_PTR 	pConfig, 
	FTM_CHAR_PTR 			pFileName
);

FTM_RET	FTOM_CLIENT_loadConfig
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_CONFIG_PTR	pConfig
);


FTM_RET	FTOM_CLIENT_loadConfigFromFile
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_CLIENT_connect
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET FTOM_CLIENT_disconnect
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET FTOM_CLIENT_isConnected
(
	FTOM_CLIENT_PTR	pClient,
	FTM_BOOL_PTR		pbConnected
);

FTM_RET	FTOM_CLIENT_setNotifyCallback
(
	FTOM_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CALLBACK	pCB
);

FTM_RET FTOM_CLIENT_NODE_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTOM_CLIENT_NODE_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pDID
);

FTM_RET FTOM_CLIENT_NODE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTOM_CLIENT_NODE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG			ulIndex,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTOM_CLIENT_NODE_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTOM_CLIENT_EP_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo
);

FTM_RET FTOM_CLIENT_EP_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID
);

FTM_RET FTOM_CLIENT_EP_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTOM_CLIENT_EP_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE			xType,
	FTM_EP_ID_PTR		pEPIDList,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTOM_CLIENT_EP_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTOM_CLIENT_EP_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG			nIndex,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTOM_CLIENT_EP_DATA_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
);

FTM_RET	FTOM_CLIENT_EP_DATA_getLast
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
);

FTM_RET FTOM_CLIENT_EP_DATA_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTOM_CLIENT_EP_DATA_getWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTOM_CLIENT_EP_DATA_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nIndex,
	FTM_ULONG			nCount
);

FTM_RET FTOM_CLIENT_EP_DATA_delWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime
);

FTM_RET	FTOM_CLIENT_EP_DATA_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pCount
);

#endif

