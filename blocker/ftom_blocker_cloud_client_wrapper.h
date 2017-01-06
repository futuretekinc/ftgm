#ifndef	__FTOM_CLOUD_CLIENT_WRAPPER_H__
#define	__FTOM_CLOUD_CLIENT_WRAPPER_H__

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_create
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_destroy
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_init
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_final
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_start
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_stop
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_isRunning
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_BOOL_PTR			pIsRunning
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_waitingForFinished
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_load
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_save
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_show
(
	FTOM_BLOCKER_PTR		pBlocker
);


FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_setNotifyCB
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

FTM_RET FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_send
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_MSG_PTR			pBaseMsg
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_process
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_MSG_PTR			pmsg
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_connect
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_disconnect
(
	FTOM_BLOCKER_PTR		pBlocker
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_isConnected
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_BOOL_PTR			pIsConnected
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_trace
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_BOOL				bTraceON
);

/**********************************************************
 *	Gateway management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_GATEWAY_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_BOOL				bStatus
);

/**********************************************************
 *	Node management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_NODE_PTR			pNode
);
	
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CHAR_PTR			pID
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_CHAR_PTR			pID,
	FTM_BOOL				bStatus
);

/**********************************************************
 *	End Point management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_getEPIDAt
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				ulBuffLen
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_EP_PTR			pEP
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CHAR_PTR			pEPID
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_CHAR_PTR			pEPID,
	FTM_BOOL				bStatus
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_getLastTime
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG_PTR			pulLastTime
);

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_send
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pDatas,
	FTM_ULONG				ulCount
);

#endif
