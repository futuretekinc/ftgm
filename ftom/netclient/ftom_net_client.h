#ifndef	__FTOM_NET_CLIENT_H__
#define	__FTOM_NET_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_client.h"

typedef	struct
{
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_CHAR	pHostName[FTM_URL_LEN+1];
	FTM_USHORT	usPort;
}	FTOM_NET_CLIENT_CONFIG, _PTR_ FTOM_NET_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTOM_CLIENT			xCommon;

	FTOM_NET_CLIENT_CONFIG	xConfig;

	FTOM_CLIENT_NOTIFY_CB 	fNotifyCB;
	FTM_VOID_PTR			pNotifyData;

	pthread_t			xThreadMain;
	struct sockaddr_in	xRemoteAddr;
	struct sockaddr_in	xLocalAddr;

	FTM_BOOL			bInit;

	FTM_BOOL			bStop;
	FTM_BOOL			bConnected;
	FTM_INT				hSock;
	FTM_ULONG			ulTimeout;
	FTM_ULONG			ulReqID;

	FTM_LIST			xTransList;
}	FTOM_NET_CLIENT, _PTR_ FTOM_NET_CLIENT_PTR;

FTM_RET	FTOM_NET_CLIENT_create
(
	FTM_CHAR_PTR	pName,
	FTOM_NET_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_NET_CLIENT_destroy
(
	FTOM_NET_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_NET_CLIENT_init
(
	FTOM_NET_CLIENT_PTR	pClient
);

FTM_RET	FTOM_NET_CLIENT_final
(
	FTOM_NET_CLIENT_PTR	pClient
);

FTM_RET	FTOM_NET_CLIENT_CONFIG_load
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_NET_CLIENT_CONFIG_save
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_NET_CLIENT_CONFIG_show
(
	FTOM_NET_CLIENT_PTR	pClient
);

FTM_RET	FTOM_NET_CLIENT_setNotifyCB
(
	FTOM_NET_CLIENT_PTR		pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

FTM_RET	FTOM_NET_CLIENT_start
(
	FTOM_NET_CLIENT_PTR	pClient
);

FTM_RET	FTOM_NET_CLIENT_stop
(
	FTOM_NET_CLIENT_PTR	pClient
);

FTM_RET	FTOM_NET_CLIENT_isRunning
(
	FTOM_NET_CLIENT_PTR pClient,
	FTM_BOOL_PTR		pIsRunning
);

FTM_RET	FTOM_NET_CLIENT_waitingForFinished
(
	FTOM_NET_CLIENT_PTR	pClient
);

FTM_RET FTOM_NET_CLIENT_isConnected
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTM_BOOL_PTR		pbConnected
);

FTM_RET FTOM_NET_CLIENT_request
(
	FTOM_NET_CLIENT_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
);

FTM_RET	FTOM_NET_CLIENT_MESSAGE_send
(
	FTOM_NET_CLIENT_PTR		pClient,
	FTOM_MSG_PTR			pMsg
);

FTM_RET	FTOM_NET_CLIENT_MESSAGE_process
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTOM_MSG_PTR		pMsg
);

#endif

