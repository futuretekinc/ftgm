#ifndef	__FTOM_AZURE_BLOCKER_H__
#define	__FTOM_AZURE_BLOCKER_H__

#include "ftom.h"
#include "ftom_server_client.h"
#include "ftom_cloud_client.h"
#include "ftom_message_queue.h"

typedef	enum
{
	FTOM_BLOCKER_STATE_UNINITIALIZED = 0,
	FTOM_BLOCKER_STATE_INITIALIZED,
	FTOM_BLOCKER_STATE_CONNECTED,
	FTOM_BLOCKER_STATE_DISCONNECTED
}	FTOM_BLOCKER_STATE, _PTR_ FTOM_BLOCKER_STATE_PTR;

typedef	struct
{
	FTM_CHAR	pName[FTM_NAME_LEN+1];

	struct
	{
		FTM_CHAR	pName[FTM_NAME_LEN+1];
	}	xServerClient;

	struct
	{
		FTM_CHAR	pName[FTM_NAME_LEN+1];
	}	xCloudClient;

	struct
	{
		FTM_BOOL	bEnabled;
	}	xServerSync;

	struct
	{
		FTM_BOOL	bEnabled;
		FTM_ULONG	ulInterval;
	}	xAutoStatusPublish;
}	FTOM_BLOCKER_CONFIG, _PTR_ FTOM_BLOCKER_CONFIG_PTR;

typedef	struct
{
	FTOM_BLOCKER_CONFIG				xConfig;

	FTOM_BLOCKER_STATE				xState;

	FTOM_SERVER_CLIENT_PTR			pServerClient;
	FTOM_SERVER_CLIENT_MODULE_PTR	pServerClientModule;

	FTOM_CLOUD_CLIENT_PTR			pCloudClient;
	FTOM_CLOUD_CLIENT_MODULE_PTR	pCloudClientModule;

	FTOM_MSG_QUEUE_PTR		pMsgQ;

	pthread_t				xThreadMain;
	FTM_BOOL				bStop;

	FTM_EVENT_TIMER_MANAGER_PTR	pETM;

}	FTOM_BLOCKER, _PTR_ FTOM_BLOCKER_PTR;

FTM_RET	FTOM_BLOCKER_create
(
	FTM_CHAR_PTR	pCloudName,
	FTM_CONFIG_PTR	pConfig,
	FTOM_BLOCKER_PTR _PTR_ ppBlocker
);

FTM_RET	FTOM_BLOCKER_destroy
(
	FTOM_BLOCKER_PTR _PTR_ ppBlocker
);

FTM_RET	FTOM_BLOCKER_init
(
	FTOM_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_BLOCKER_final
(
	FTOM_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_BLOCKER_start
(
	FTOM_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_BLOCKER_stop
(
	FTOM_BLOCKER_PTR pBlocker
);

FTM_RET FTOM_BLOCKER_waitingForFinished
(	
	FTOM_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_BLOCKER_CONFIG_load
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_BLOCKER_CONFIG_save
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_BLOCKER_CONFIG_show
(
	FTOM_BLOCKER_PTR	pBlocker
);

FTM_RET	FTOM_BLOCKER_MESSAGE_send
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR		pMsg
);

FTM_RET	FTOM_BLOCKER_MESSAGE_process
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR		pBaseMsg
);

FTM_RET	FTOM_BLOCKER_SERVER_sync
(
	FTOM_BLOCKER_PTR	pBlocker
);

FTM_RET	FTOM_BLOCKER_SERVER_updateStatus
(
	FTOM_BLOCKER_PTR	pBlocker
);
#endif
