#ifndef	__FTOM_SERVER_H__
#define	__FTOM_SERVER_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftom_params.h"
#include "ftom_node.h"
#include "ftom_service.h"

typedef	struct FTOM_STRUCT _PTR_ FTOM_PTR;
typedef	struct FTOM_SERVER_STRUCT _PTR_ FTOM_SERVER_PTR;
typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTOM_SERVER_PTR		pServer;
	FTM_BOOL			bStop;
	FTM_BYTE			pReqBuff[FTOM_DEFAULT_PACKET_SIZE];
	FTM_BYTE			pRespBuff[FTOM_DEFAULT_PACKET_SIZE];
}	FTOM_SESSION, _PTR_ FTOM_SESSION_PTR;

typedef	FTM_RET	(*FTOM_SERVER_CALLBACK)(FTOM_SERVER_PTR pServer, FTOM_REQ_PARAMS_PTR, FTM_ULONG, FTOM_RESP_PARAMS_PTR, FTM_ULONG);

typedef struct
{
	FTOM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTOM_SERVER_CALLBACK	fService;
}	FTOM_SERVER_CMD_SET, _PTR_ FTOM_SERVER_CMD_SET_PTR;

typedef	struct
{
	FTM_USHORT		usPort;
	FTM_ULONG		ulMaxSession;
}	FTOM_SERVER_CONFIG, _PTR_ FTOM_SERVER_CONFIG_PTR;

typedef	struct FTOM_SERVER_STRUCT
{
	FTOM_SERVER_CONFIG	xConfig;
	pthread_t 			xPThread;
	sem_t				xLock;
	FTM_BOOL			bStop;	
	FTM_LIST			xSessionList;
	FTM_INT				hSocket;
	FTOM_SERVICE_ID		xServiceID;
	FTOM_SERVICE_CALLBACK	fServiceCB;
}	FTOM_SERVER;

FTM_RET	FTOM_SERVER_create
(
	FTOM_SERVER_PTR _PTR_ 	ppServer
);
	
FTM_RET	FTOM_SERVER_destroy
(
	FTOM_SERVER_PTR _PTR_ 	ppServer
);

FTM_RET	FTOM_SERVER_init
(
	FTOM_SERVER_PTR 	pServer
);

FTM_RET	FTOM_SERVER_final
(
	FTOM_SERVER_PTR 	pServer
);

FTM_RET	FTOM_SERVER_start
(
	FTOM_SERVER_PTR	pServer
);

FTM_RET	FTOM_SERVER_stop
(
	FTOM_SERVER_PTR	pServer
);

FTM_RET	FTOM_SERVER_loadFromFile
(
	FTOM_SERVER_PTR	pServer, 
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_SERVER_showConfig
(
	FTOM_SERVER_PTR 	pServer
);

FTM_RET	FTOM_SERVER_notify
(
	FTOM_SERVER_PTR 	pServer, 
	FTOM_MSG_PTR 		pMsg
);

FTM_RET	FTOM_SERVER_setServiceCallback
(
	FTOM_SERVER_PTR 	pServer, 
	FTOM_SERVICE_ID 	xID, 
	FTOM_SERVICE_CALLBACK fServiceCB
);

#endif
