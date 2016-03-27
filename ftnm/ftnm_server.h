#ifndef	__FTNM_SERVER_H__
#define	__FTNM_SERVER_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftnm_params.h"
#include "ftnm_service.h"

#define	FTNM_PACKET_LEN					2048

typedef	struct FTNM_SERVER_STRUCT _PTR_ FTNM_SERVER_PTR;
typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTNM_SERVER_PTR		pServer;
	FTM_BOOL			bStop;
	FTM_BYTE			pReqBuff[FTNM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTNM_PACKET_LEN];
}	FTNM_SESSION, _PTR_ FTNM_SESSION_PTR;

typedef	FTM_RET	(*FTNM_SERVER_CALLBACK)(FTNM_SESSION_PTR pSession, FTNM_REQ_PARAMS_PTR, FTNM_RESP_PARAMS_PTR);

typedef struct
{
	FTNM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTNM_SERVER_CALLBACK	fService;
}	FTNM_SERVER_CMD_SET, _PTR_ FTNM_SERVER_CMD_SET_PTR;

typedef	struct
{
	FTM_USHORT		usPort;
	FTM_ULONG		ulMaxSession;
}	FTNM_SERVER_CONFIG, _PTR_ FTNM_SERVER_CONFIG_PTR;


typedef	struct FTNM_SERVER_STRUCT
{
	FTNM_SERVER_CONFIG		xConfig;
	pthread_t 				xPThread;
	sem_t					xLock;

	FTM_LIST				xSessionList;
	FTNM_SERVICE_ID			xServiceID;
	FTNM_SERVICE_CALLBACK	fServiceCB;
}	FTNM_SERVER;

FTM_RET	FTNM_SERVER_init(FTNM_SERVER_PTR 	pServer);
FTM_RET	FTNM_SERVER_final(FTNM_SERVER_PTR 	pServer);

FTM_RET	FTNM_SERVER_start(FTNM_SERVER_PTR	pServer);
FTM_RET	FTNM_SERVER_stop(FTNM_SERVER_PTR	pServer);

FTM_RET	FTNM_SERVER_loadFromFile(FTNM_SERVER_PTR pServer, FTM_CHAR_PTR	pFileName);
FTM_RET	FTNM_SERVER_showConfig(FTNM_SERVER_PTR pServer);

FTM_RET	FTNM_SERVER_notify(FTNM_SERVER_PTR pServer, FTNM_MSG_PTR pMsg);

FTM_RET	FTNM_SERVER_setServiceCallback(FTNM_SERVER_PTR pServer, FTNM_SERVICE_ID xID, FTNM_SERVICE_CALLBACK fServiceCB);
#endif
