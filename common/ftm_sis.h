#ifndef	__FTM_SERVICE_INTERFACE_H__
#define	__FTM_SERVICE_INTERFACE_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm_sim.h"
#include "ftm_list.h"

struct FTM_SIS_STRUCT;

#define	FTM_SIS_DEFAULT_PORT			8888
#define	FTM_SIS_DEFAULT_SESSION_COUNT	10

#define	FTM_SIS_DEFAULT_PACKET_SIZE		2000

#define	FTM_SIS_MAKE_CMD(CMD,FUN)	{CMD, #CMD, (FTM_SIS_FUNCTION)FUN }

typedef	FTM_UINT32	FTM_SIS_ID,  _PTR_ FTM_SIS_ID_PTR;

typedef FTM_RET 	(*FTM_SIS_CALLBACK)(FTM_SIS_ID xID, FTM_ULONG xMsg, FTM_VOID_PTR pData);
typedef	FTM_RET		(*FTM_SIS_FUNCTION)(struct FTM_SIS_STRUCT _PTR_ pSIS, FTM_VOID_PTR pReq, FTM_ULONG ulReqLen, FTM_VOID_PTR pResp, FTM_ULONG ulRespLen);

typedef struct
{
	FTM_SIM_TYPE		xType;
	FTM_CHAR_PTR		pName;
	FTM_SIS_FUNCTION	fService;
}	FTM_SIS_CMD, _PTR_ FTM_SIS_CMD_PTR;

typedef	struct FTM_SIS_CONFIG_STRUCT
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

}	FTM_SIS_CONFIG, _PTR_ FTM_SIS_CONFIG_PTR;

typedef	struct FTM_SIS_STRUCT
{
	FTM_SIS_CONFIG	xConfig;

	FTM_VOID_PTR	pParent;

	FTM_BOOL		bStop;	
	FTM_SIS_ID		xServiceID;
	FTM_SIS_CALLBACK	fServiceCB;
	FTM_ULONG		ulReqID;

	FTM_LIST_PTR	pList;
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
}	FTM_SIS, _PTR_ FTM_SIS_PTR;

FTM_RET	FTM_SIS_create
(
	FTM_VOID_PTR	pParent,
	FTM_SIS_PTR _PTR_ 	ppSIS
);
	
FTM_RET	FTM_SIS_destroy
(
	FTM_SIS_PTR _PTR_ 	ppSIS
);

FTM_RET	FTM_SIS_init
(
	FTM_SIS_PTR pSIS,
	FTM_VOID_PTR	pParent
);

FTM_RET	FTM_SIS_final
(
	FTM_SIS_PTR pSIS
);

FTM_RET	FTM_SIS_start
(
	FTM_SIS_PTR	pSIS
);

FTM_RET	FTM_SIS_stop
(
	FTM_SIS_PTR	pSIS
);

FTM_RET	FTM_SIS_isRun
(
	FTM_SIS_PTR	pSIS,
	FTM_BOOL_PTR	pbRun
);

FTM_RET FTM_SIS_CONFIG_load
(
	FTM_SIS_PTR		pSIS,
	FTM_CONFIG_ITEM_PTR	pConfig
);

FTM_RET FTM_SIS_CONFIG_save
(
	FTM_SIS_PTR	pSIS,
	FTM_CONFIG_ITEM_PTR	pConfig
);

FTM_RET	FTM_SIS_CONFIG_show
(
	FTM_SIS_PTR pSIS
);

FTM_RET	FTM_SIS_CMD_append
(
	FTM_SIS_PTR		pSIS,
	FTM_SIS_CMD_PTR	pCmd
);

FTM_RET	FTM_SIS_CMD_appendList
(
	FTM_SIS_PTR		pSIS,
	FTM_SIS_CMD_PTR	pCmdSet,
	FTM_ULONG		ulCount
);

FTM_RET	FTM_SIS_CMD_count
(
	FTM_SIS_PTR		pSIS,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTM_SIS_CMD_getAt
(
	FTM_SIS_PTR		pSIS,
	FTM_ULONG		ulIndex,
	FTM_SIS_CMD_PTR _PTR_ ppCmd
);

FTM_RET	FTM_SIS_CMD_print
(
	FTM_SIS_PTR		pSIS,
	FTM_SIM_TYPE	xType,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
);

FTM_RET	FTOM_SERVER_setCallback
(
	FTM_SIS_PTR			pSIS, 
	FTM_SIS_ID			xServiceID, 
	FTM_SIS_CALLBACK	fCallback
);


typedef	FTM_UINT64		FTM_SIS_SESSION_ID, _PTR_ FTM_SIS_SESSION_ID_PTR;

typedef	struct
{
	FTM_SIS_SESSION_ID	xID;
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t		xPThread;
	FTM_VOID_PTR	pData;
	FTM_BOOL		bStop;
	FTM_ULONG		ulReqID;
	FTM_BYTE		pReqBuff[FTM_SIS_DEFAULT_PACKET_SIZE];
	FTM_BYTE		pRespBuff[FTM_SIS_DEFAULT_PACKET_SIZE];
}	FTM_SIS_SESSION, _PTR_ FTM_SIS_SESSION_PTR;

FTM_RET	FTM_SIS_SESSION_create
(
	FTM_INT		hSocket,
	struct sockaddr_in _PTR_ pPeer,
	FTM_VOID_PTR	pData,
	FTM_SIS_SESSION_PTR _PTR_ ppSession
);

FTM_RET FTM_SIS_SESSION_destroy
(
	FTM_SIS_SESSION_PTR _PTR_ ppSession
);

#endif
