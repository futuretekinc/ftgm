#ifndef	__FTM_OM_SERVER_H__
#define	__FTM_OM_SERVER_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm_om_params.h"
#include "ftm_om_service.h"

#define	FTM_OM_PACKET_LEN					2048

typedef	struct FTM_OM_STRUCT _PTR_ FTM_OM_PTR;
typedef	struct FTM_OM_SERVER_STRUCT _PTR_ FTM_OM_SERVER_PTR;
typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTM_OM_SERVER_PTR		pServer;
	FTM_BOOL			bStop;
	FTM_BYTE			pReqBuff[FTM_OM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTM_OM_PACKET_LEN];
}	FTM_OM_SESSION, _PTR_ FTM_OM_SESSION_PTR;

typedef	FTM_RET	(*FTM_OM_SERVER_CALLBACK)(FTM_OM_SESSION_PTR pSession, FTM_OM_REQ_PARAMS_PTR, FTM_OM_RESP_PARAMS_PTR);

typedef struct
{
	FTM_OM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTM_OM_SERVER_CALLBACK	fService;
}	FTM_OM_SERVER_CMD_SET, _PTR_ FTM_OM_SERVER_CMD_SET_PTR;

typedef	struct
{
	FTM_USHORT		usPort;
	FTM_ULONG		ulMaxSession;
}	FTM_OM_SERVER_CONFIG, _PTR_ FTM_OM_SERVER_CONFIG_PTR;


typedef	struct FTM_OM_SERVER_STRUCT
{
	FTM_OM_SERVER_CONFIG		xConfig;
	pthread_t 				xPThread;
	sem_t					xLock;
	
	FTM_OM_PTR		pOM;
	FTM_LIST				xSessionList;
	FTM_OM_SERVICE_ID			xServiceID;
	FTM_OM_SERVICE_CALLBACK	fServiceCB;
}	FTM_OM_SERVER;

FTM_RET	FTM_OM_SERVER_init(FTM_OM_PTR pOM, FTM_OM_SERVER_PTR 	pServer);
FTM_RET	FTM_OM_SERVER_final(FTM_OM_SERVER_PTR 	pServer);

FTM_RET	FTM_OM_SERVER_start(FTM_OM_SERVER_PTR	pServer);
FTM_RET	FTM_OM_SERVER_stop(FTM_OM_SERVER_PTR	pServer);

FTM_RET	FTM_OM_SERVER_loadFromFile(FTM_OM_SERVER_PTR pServer, FTM_CHAR_PTR	pFileName);
FTM_RET	FTM_OM_SERVER_showConfig(FTM_OM_SERVER_PTR pServer);

FTM_RET	FTM_OM_SERVER_notify(FTM_OM_SERVER_PTR pServer, FTM_OM_MSG_PTR pMsg);

FTM_RET	FTM_OM_SERVER_setServiceCallback(FTM_OM_SERVER_PTR pServer, FTM_OM_SERVICE_ID xID, FTM_OM_SERVICE_CALLBACK fServiceCB);
#endif
