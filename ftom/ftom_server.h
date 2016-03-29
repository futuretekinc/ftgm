#ifndef	__FTOM_SERVER_H__
#define	__FTOM_SERVER_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftom_params.h"
#include "ftom_node.h"
#include "ftom_service.h"

#define	FTOM_PACKET_LEN					2048

typedef	struct FTOM_STRUCT _PTR_ FTOM_PTR;
typedef	struct FTOM_SERVER_STRUCT _PTR_ FTOM_SERVER_PTR;
typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTOM_SERVER_PTR		pServer;
	FTM_BOOL			bStop;
	FTM_BYTE			pReqBuff[FTOM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTOM_PACKET_LEN];
}	FTOM_SESSION, _PTR_ FTOM_SESSION_PTR;

typedef	FTM_RET	(*FTOM_SERVER_CALLBACK)(FTOM_SESSION_PTR pSession, FTOM_REQ_PARAMS_PTR, FTOM_RESP_PARAMS_PTR);

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
	pthread_t 				xPThread;
	sem_t					xLock;
	
	FTOM_PTR				pOM;
	FTM_LIST				xSessionList;
	FTOM_SERVICE_ID		xServiceID;
	FTOM_SERVICE_CALLBACK	fServiceCB;
}	FTOM_SERVER;

FTM_RET	FTOM_SERVER_init
(
	FTOM_SERVER_PTR 	pServer,
	FTOM_PTR pOM 
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

FTM_RET	FTOM_SERVER_createNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_NODE_PTR		pInfo,
	FTOM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTOM_SERVER_destroyNode
(
	FTOM_SERVER_PTR		pServer,
	FTOM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTOM_SERVER_countNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_SERVER_getNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_CHAR			pDID[FTM_DID_LEN + 1],
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_SERVER_getNodeAt
(
	FTOM_SERVER_PTR		pServer,
	FTM_ULONG			ulIndex,
	FTOM_NODE_PTR _PTR_ ppNode
);

#endif
