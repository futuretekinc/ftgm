#ifndef	_FTOM_AZURE_CLIENT_H_
#define	_FTOM_AZURE_CLIENT_H_


#include "ftom.h"
#include "ftom_client.h"

#define	FTOM_AZURE_SHARED_ACCESS_KEY_LEN	64

typedef	struct
{
	FTM_CHAR	pName[FTM_NAME_LEN + 1];
	FTM_CHAR	pHostName[FTM_HOST_LEN + 1];
	FTM_CHAR	pDeviceID[FTM_ID_LEN + 1];
	FTM_CHAR	pSharedAccessKey[FTOM_AZURE_SHARED_ACCESS_KEY_LEN + 1];
	FTM_BOOL	bTraceON;
	FTM_ULONG	ulLoopInterval;
	FTM_ULONG	ulReconnectionInterval;
}	FTOM_AZURE_CLIENT_CONFIG, _PTR_ FTOM_AZURE_CLIENT_CONFIG_PTR;

typedef struct
{
	FTOM_AZURE_CLIENT_CONFIG	xConfig;

	pthread_t					xThreadMain;

	FTOM_MSG_QUEUE_PTR			pMsgQ;
	FTM_BOOL					bConnected;
	FTM_BOOL					bStop;	
	FTM_VOID_PTR				hAzureClient;

	FTOM_CLIENT_NOTIFY_CB 		fNotifyCB;
	FTM_VOID_PTR				pNotifyData;

	FTM_LIST_PTR				pTransferWaitingList;
	FTM_LIST_PTR				pCompletionWaitingList;

	struct
	{
		struct
		{
			FTM_ULONG	ulTotal;
			FTM_ULONG	ulCompleted;

			FTM_ULONG	ulLastCompletedID;
			FTM_TIME	xLastCompletedTime;
		}	xSend;
	}	xStats;
}	FTOM_AZURE_CLIENT, _PTR_ FTOM_AZURE_CLIENT_PTR;

FTM_RET	FTOM_AZURE_CLIENT_create
(
	FTM_CHAR_PTR	pName,
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_AZURE_CLIENT_destroy
(
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_AZURE_CLIENT_init
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_final
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_CONFIG_load
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_AZURE_CLIENT_CONFIG_save
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_AZURE_CLIENT_CONFIG_show
(
	FTOM_AZURE_CLIENT_PTR	pClient
);


FTM_RET	FTOM_AZURE_CLIENT_setNotifyCB
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
);

FTM_RET FTOM_AZURE_CLIENT_MESSAGE_send
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
);

FTM_RET	FTOM_AZURE_CLIENT_MESSAGE_process
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
);

FTM_RET	FTOM_AZURE_CLIENT_connect
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_disconnect
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_isConnected
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pbConnected
);


FTM_RET	FTOM_AZURE_CLIENT_trace
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_BOOL				bTraceON
);

#endif

