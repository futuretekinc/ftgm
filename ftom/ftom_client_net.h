#ifndef	__FTOM_CLIENT_NET_H__
#define	__FTOM_CLIENT_NET_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom_params.h"
#include "ftom_client_config.h"
#include "ftom_client.h"

typedef struct 
{
	FTOM_REQ_PARAMS_PTR		pReq;
	FTM_ULONG				ulReqLen;
	FTOM_RESP_PARAMS_PTR	pResp;
	FTM_ULONG				ulRespLen;
	sem_t					xDone;
}	FTOM_CLIENT_NET_TRANS, _PTR_ FTOM_CLIENT_NET_TRANS_PTR;

typedef	struct
{
	struct
	{
		FTM_CHAR	pHost[FTOM_CLIENT_SERVER_IP_LEN];
		FTM_USHORT	usPort;
	}	xServer;
}	FTOM_CLIENT_NET_CONFIG, _PTR_ FTOM_CLIENT_NET_CONFIG_PTR;

typedef	struct
{
	FTOM_CLIENT			xCommon;

	FTOM_CLIENT_NET_CONFIG	xConfig;

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
}	FTOM_CLIENT_NET, _PTR_ FTOM_CLIENT_NET_PTR;

FTM_RET	FTOM_CLIENT_NET_init
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_final
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_create
(
	FTOM_CLIENT_NET_PTR _PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_NET_destroy
(
	FTOM_CLIENT_NET_PTR _PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_NET_start
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_stop
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_ReadConfig
(
	FTOM_CLIENT_NET_CONFIG_PTR 	pConfig, 
	FTM_CHAR_PTR 			pFileName
);

FTM_RET	FTOM_CLIENT_NET_setConfig
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_CONFIG_PTR	pConfig
);


FTM_RET	FTOM_CLIENT_NET_loadConfigFromFile
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_CLIENT_NET_connect
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET FTOM_CLIENT_NET_disconnect
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET FTOM_CLIENT_NET_isConnected
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_BOOL_PTR		pbConnected
);

FTM_RET FTOM_CLIENT_NET_request
(
	FTOM_CLIENT_NET_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
);

#endif

