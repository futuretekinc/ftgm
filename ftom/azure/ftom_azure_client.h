#ifndef	_FTOM_AZURE_CLIENT_H_
#define	_FTOM_AZURE_CLIENT_H_


#include "ftom.h"
#include "ftom_client.h"
#include "ftom_client_net.h"

#define	FTOM_AZURE_SHARED_ACCESS_KEY_LEN	64

typedef	struct
{
	FTM_CHAR	pHostName[FTM_HOST_LEN + 1];
	FTM_CHAR	pDeviceID[FTM_ID_LEN + 1];
	FTM_CHAR	pSharedAccessKey[FTOM_AZURE_SHARED_ACCESS_KEY_LEN + 1];
	FTM_BOOL	bTraceON;
	FTM_ULONG	ulLoopInterval;
	FTM_ULONG	ulReconnectionInterval;
}	FTOM_AZURE_CLIENT_CONFIG, _PTR_ FTOM_AZURE_CLIENT_CONFIG_PTR;

typedef struct
{
	FTOM_CLIENT_NET				xParent;

	FTOM_AZURE_CLIENT_CONFIG	xConfig;
	pthread_t					xThreadMain;
	FTM_BOOL					bConnected;
	FTM_BOOL					bStop;	
	FTM_VOID_PTR				hAzureClient;
	
	FTM_LIST_PTR				pEventList;

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

FTM_RET	FTOM_AZURE_CLIENT_start
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_loadConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_AZURE_CLIENT_saveConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_AZURE_CLIENT_showConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_messageProcess
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pmsg
);

FTM_RET	FTOM_AZURE_CLIENT_connect
(
	FTOM_AZURE_CLIENT_PTR	pClient
);


FTM_RET	FTOM_AZURE_CLIENT_trace
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_BOOL				bTraceON
);

#endif

