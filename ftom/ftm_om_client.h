#ifndef	__FTM_OM_CLIENT_H__
#define	__FTM_OM_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftm_om_params.h"
#include "ftm_om_client_config.h"

typedef	FTM_RET (*FTM_OM_CLIENT_NOTIFY_CALLBACK)(FTM_VOID_PTR);

typedef struct 
{
	FTM_OM_REQ_PARAMS_PTR		pReq;
	FTM_ULONG				ulReqLen;
	FTM_OM_RESP_PARAMS_PTR	pResp;
	FTM_ULONG				ulRespLen;
	sem_t					xDone;
}	FTM_OM_CLIENT_TRANS, _PTR_ FTM_OM_CLIENT_TRANS_PTR;

typedef	struct
{
	struct
	{
		FTM_CHAR	pHost[FTM_OM_CLIENT_SERVER_IP_LEN];
		FTM_USHORT	usPort;
	}	xServer;
}	FTM_OM_CLIENT_CONFIG, _PTR_ FTM_OM_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTM_OM_CLIENT_CONFIG	xConfig;

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
	FTM_OM_CLIENT_NOTIFY_CALLBACK 	fNotifyCallback;
}	FTM_OM_CLIENT, _PTR_ FTM_OM_CLIENT_PTR;

FTM_RET	FTM_OM_CLIENT_init
(
	FTM_OM_CLIENT_PTR	pClient
);

FTM_RET	FTM_OM_CLIENT_final
(
	FTM_OM_CLIENT_PTR	pClient
);

FTM_RET	FTM_OM_CLIENT_create
(
	FTM_OM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_OM_CLIENT_destroy
(
	FTM_OM_CLIENT_PTR 	pClient
);

FTM_RET	FTM_OM_CLIENT_start
(
	FTM_OM_CLIENT_PTR	pClient
);

FTM_RET	FTM_OM_CLIENT_stop
(
	FTM_OM_CLIENT_PTR	pClient
);

FTM_RET	FTM_OM_CLIENT_ReadConfig
(
	FTM_OM_CLIENT_CONFIG_PTR 	pConfig, 
	FTM_CHAR_PTR 			pFileName
);

FTM_RET	FTM_OM_CLIENT_loadConfig
(
	FTM_OM_CLIENT_PTR			pClient,
	FTM_OM_CLIENT_CONFIG_PTR	pConfig
);


FTM_RET	FTM_OM_CLIENT_loadConfigFromFile
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTM_OM_CLIENT_connect
(
	FTM_OM_CLIENT_PTR	pClient
);

FTM_RET FTM_OM_CLIENT_disconnect
(
	FTM_OM_CLIENT_PTR	pClient
);

FTM_RET FTM_OM_CLIENT_isConnected
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_BOOL_PTR		pbConnected
);

FTM_RET	FTM_OM_CLIENT_setNotifyCallback
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_OM_CLIENT_NOTIFY_CALLBACK	pCB
);

FTM_RET FTM_OM_CLIENT_NODE_create
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTM_OM_CLIENT_NODE_destroy
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pDID
);

FTM_RET FTM_OM_CLIENT_NODE_count
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTM_OM_CLIENT_NODE_getAt
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_ULONG			ulIndex,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTM_OM_CLIENT_NODE_get
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_PTR		pInfo
);

FTM_RET FTM_OM_CLIENT_EP_create
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo
);

FTM_RET FTM_OM_CLIENT_EP_destroy
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID
);

FTM_RET FTM_OM_CLIENT_EP_count
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTM_OM_CLIENT_EP_getList
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_TYPE			xType,
	FTM_EP_ID_PTR		pEPIDList,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTM_OM_CLIENT_EP_get
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTM_OM_CLIENT_EP_getAt
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_ULONG			nIndex,
	FTM_EP_PTR			pEPInfo
);

FTM_RET FTM_OM_CLIENT_EP_DATA_add
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
);

FTM_RET	FTM_OM_CLIENT_EP_DATA_getLast
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
);

FTM_RET FTM_OM_CLIENT_EP_DATA_getList
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTM_OM_CLIENT_EP_DATA_getWithTime
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
);

FTM_RET FTM_OM_CLIENT_EP_DATA_del
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nIndex,
	FTM_ULONG			nCount
);

FTM_RET FTM_OM_CLIENT_EP_DATA_delWithTime
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime
);

FTM_RET	FTM_OM_CLIENT_EP_DATA_count
(
	FTM_OM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pCount
);

#endif

