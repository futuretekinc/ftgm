#ifndef	__FTOM_SERVER_H__
#define	__FTOM_SERVER_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftom_params.h"
#include "ftom_node.h"
#include "ftom_service.h"

struct FTOM_SERVER_STRUCT _PTR_		pServer;

typedef	FTM_RET	(*FTOM_SERVER_CALLBACK)(struct FTOM_SERVER_STRUCT _PTR_ pServer, FTOM_REQ_PARAMS_PTR, FTM_ULONG, FTOM_RESP_PARAMS_PTR, FTM_ULONG);

typedef struct
{
	FTOM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTOM_SERVER_CALLBACK	fService;
}	FTOM_SERVER_CMD_SET, _PTR_ FTOM_SERVER_CMD_SET_PTR;

typedef	struct
{
	struct
	{	
		FTM_BOOL	bEnabled;
		FTM_USHORT	usPort;
		FTM_ULONG	ulMaxSession;
	}	xTCP;

	struct
	{
		FTM_BOOL	bEnabled;
		FTM_USHORT	usPort;
	}	xUDP;
		
	struct	
	{
		FTM_BOOL	bEnabled;
		FTM_USHORT	usPort;
		FTM_ULONG	ulMaxSubscribe;
	}	xPublisher;

	struct
	{
		FTM_BOOL	bEnabled;
	}	xPipe;

	struct
	{
		FTM_BOOL	bEnabled;
		FTM_CHAR	pKeyFile[FTM_FILE_NAME_LEN + 1];
	}	xSM;
}	FTOM_SERVER_CONFIG, _PTR_ FTOM_SERVER_CONFIG_PTR;

typedef	struct FTOM_SERVER_STRUCT
{
	FTOM_SERVER_CONFIG	xConfig;

	FTM_BOOL			bStop;	
	FTOM_SERVICE_ID		xServiceID;
	FTOM_SERVICE_CB		fServiceCB;
	FTM_ULONG			ulReqID;

	struct
	{
		pthread_t 		xThread;
		sem_t			xLock;
		FTM_INT			hSocket;
		FTM_LIST		xSessionList;
	}	xTCP;

	struct
	{
		pthread_t 		xThread;
		FTM_INT			hSocket;
		FTM_LIST		xClientList;
	}	xUDP;

	struct
	{
		pthread_t 		xThread;
	}	xPIPE;

	struct
	{
		pthread_t 		xThread;
	}	xSM;
}	FTOM_SERVER, _PTR_ FTOM_SERVER_PTR;

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
	FTOM_SERVER_PTR pServer
);

FTM_RET	FTOM_SERVER_final
(
	FTOM_SERVER_PTR pServer
);

FTM_RET	FTOM_SERVER_start
(
	FTOM_SERVER_PTR	pServer
);

FTM_RET	FTOM_SERVER_stop
(
	FTOM_SERVER_PTR	pServer
);

FTM_RET	FTOM_SERVER_isRun
(
	FTOM_SERVER_PTR	pServer,
	FTM_BOOL_PTR	pbRun
);

FTM_RET FTOM_SERVER_loadConfig
(
	FTOM_SERVER_PTR	pServer,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTOM_SERVER_loadConfigFromFile
(
	FTOM_SERVER_PTR	pServer, 
	FTM_CHAR_PTR	pFileName
);

FTM_RET FTOM_SERVER_saveConfig
(
	FTOM_SERVER_PTR	pServer,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTOM_SERVER_showConfig
(
	FTOM_SERVER_PTR pServer
);

FTM_RET	FTOM_SERVER_sendMessage
(
	FTOM_SERVER_PTR pServer, 
	FTOM_MSG_PTR 	pMsg
);

FTM_RET	FTOM_SERVER_setServiceCallback
(
	FTOM_SERVER_PTR pServer, 
	FTOM_SERVICE_ID xID, 
	FTOM_SERVICE_CB	fServiceCB
);

#endif
