#ifndef	_FTOM_SERVER_CLIENT_WRAPPER_H_
#define	_FTOM_SERVER_CLIENT_WRAPPER_H_

#include "ftom.h"
#include "ftom_client.h"

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_create
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_destroy
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_init
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_final
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_start
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_stop
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_isRunning
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_waitingForFinished
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_load
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_save
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_show
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);


FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_setNotifyCB
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

FTM_RET FTOM_BLOCKER_SERVER_CLIENT_MESSAGE_send
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_MESSAGE_process
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_connect
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
);


FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_trace
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_BOOL				bTraceON
);

/**********************************************************
 *	Gateway management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_GATEWAY_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_BOOL				bStatus
);

/**********************************************************
 *	Node management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_NODE_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_NODE_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_NODE_PTR			pNode
);
	
FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_NODE_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pID
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_NODE_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pID,
	FTM_BOOL				bStatus
);

/**********************************************************
 *	End Point management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_getEPIDAt
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				ulBuffLen
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_EP_PTR			pEP
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_BOOL				bStatus
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_DATA_getLastTime
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG_PTR			pulLastTime
);

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_EP_DATA_send
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pDatas,
	FTM_ULONG				ulCount
);

#endif

