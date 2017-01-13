#ifndef	__FTM_SERVICE_INTERFACE_CLIENT_H__
#define	__FTM_SERVICE_INTERFACE_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftm_msg_queue.h"
#include "ftm_sim.h"

#define	FTM_SIC_DEFAULT_PACKET_SIZE		2000
#define	FTM_SIC_DEFAULT_SERVER_PORT		8890
#define	FTM_SIC_DEFAULT_SERVER_TIMEOUT	10000

typedef FTM_ULONG	FTM_SIC_ID, _PTR_ FTM_SIC_ID_PTR;

typedef FTM_RET 	(*FTM_SIC_CALLBACK)(FTM_SIM_PTR pMsg, FTM_VOID_PTR pData);

typedef	struct
{
	FTM_CHAR	pHostName[FTM_URL_LEN+1];
	FTM_USHORT	usPort;
}	FTM_SIC_CONFIG, _PTR_ FTM_SIC_CONFIG_PTR;

typedef	struct
{
	FTM_SIC_CONFIG		xConfig;

	FTM_SIC_CALLBACK	fNotifyCB;
	FTM_VOID_PTR		pNotifyData;

	pthread_t			xThreadMain;
	struct sockaddr_in	xRemoteAddr;
	struct sockaddr_in	xLocalAddr;

	FTM_MSG_QUEUE_PTR	pMsgQ;

	FTM_BOOL			bInit;

	FTM_BOOL			bStop;
	FTM_BOOL			bConnected;
	FTM_INT				hSock;
	FTM_ULONG			ulTimeout;
	FTM_ULONG			ulReqID;

	FTM_LIST			xTransList;
}	FTM_SIC, _PTR_ FTM_SIC_PTR;

FTM_RET	FTM_SIC_create
(
	FTM_SIC_PTR _PTR_ ppClient
);

FTM_RET	FTM_SIC_destroy
(
	FTM_SIC_PTR _PTR_ ppClient
);

FTM_RET	FTM_SIC_init
(
	FTM_SIC_PTR	pClient
);

FTM_RET	FTM_SIC_final
(
	FTM_SIC_PTR	pClient
);

FTM_RET	FTM_SIC_CONFIG_load
(
	FTM_SIC_PTR	pClient,
	FTM_CONFIG_ITEM_PTR		pConfig
);

FTM_RET	FTM_SIC_CONFIG_save
(
	FTM_SIC_PTR	pClient,
	FTM_CONFIG_ITEM_PTR		pConfig
);

FTM_RET	FTM_SIC_CONFIG_show
(
	FTM_SIC_PTR	pClient
);

FTM_RET	FTM_SIC_setNotifyCB
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_CALLBACK	fNotifyCB,
	FTM_VOID_PTR		pData
);

FTM_RET	FTM_SIC_start
(
	FTM_SIC_PTR	pClient
);

FTM_RET	FTM_SIC_stop
(
	FTM_SIC_PTR	pClient
);

FTM_RET	FTM_SIC_isRunning
(
	FTM_SIC_PTR pClient,
	FTM_BOOL_PTR	pIsRunning
);

FTM_RET	FTM_SIC_waitingForFinished
(
	FTM_SIC_PTR	pClient
);

FTM_RET FTM_SIC_isConnected
(
	FTM_SIC_PTR	pClient,
	FTM_BOOL_PTR	pbConnected
);

FTM_RET FTM_SIC_request
(
	FTM_SIC_PTR			pClient, 
	FTM_SIM_REQ_PTR		pReq,
	FTM_ULONG			ulReqLen,
	FTM_SIM_RESP_PTR	pResp,
	FTM_ULONG			ulRespLen,
	FTM_ULONG_PTR		pulRespLen
);

FTM_RET	FTM_SIC_MESSAGE_send
(
	FTM_SIC_PTR	pClient,
	FTM_SIM_PTR	pMsg
);

FTM_RET	FTM_SIC_MESSAGE_process
(
	FTM_SIC_PTR	pClient,
	FTM_SIM_PTR	pMsg
);

#endif

