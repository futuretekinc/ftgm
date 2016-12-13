#ifndef	__FTOM_CLIENT_NET_H__
#define	__FTOM_CLIENT_NET_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_client.h"

typedef	struct
{
	FTM_CHAR	pHostName[FTM_URL_LEN+1];
	FTM_USHORT	usPort;
}	FTOM_CLIENT_NET_CONFIG, _PTR_ FTOM_CLIENT_NET_CONFIG_PTR;

typedef	struct
{
	FTOM_CLIENT			xCommon;

	FTOM_CLIENT_NET_CONFIG	xConfig;

	pthread_t			xThreadNet;
	struct sockaddr_in	xRemoteAddr;
	struct sockaddr_in	xLocalAddr;

	FTM_BOOL			bInit;

	FTM_BOOL			bNetStop;
	FTM_BOOL			bConnected;
	FTM_INT				hSock;
	FTM_ULONG			ulTimeout;
	FTM_ULONG			ulReqID;

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

FTM_RET	FTOM_CLIENT_NET_loadConfig
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_CLIENT_NET_saveConfig
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_CLIENT_NET_showConfig
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_setNotifyCB
(
	FTOM_CLIENT_NET_PTR		pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

FTM_RET	FTOM_CLIENT_NET_start
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_stop
(
	FTOM_CLIENT_NET_PTR	pClient
);

FTM_RET	FTOM_CLIENT_NET_waitingForFinished
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


FTM_RET	FTOM_CLIENT_NET_loadConfig
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
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

FTM_RET	FTOM_CLIENT_NET_sendMessage
(
	FTOM_CLIENT_NET_PTR		pClient,
	FTOM_MSG_PTR			pMsg
);

FTM_RET	FTOM_CLIENT_NET_messageProcess
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTOM_MSG_PTR		pMsg
);

#endif

