#ifndef	__FTOM_AZURE_BLOCKER_H__
#define	__FTOM_AZURE_BLOCKER_H__

#include "ftom.h"
#include "ftom_net_client.h"
#include "ftom_cloud_client.h"
#include "ftom_message_queue.h"

typedef	struct
{
	FTOM_NET_CLIENT_PTR			pNetClient;

	FTOM_CLOUD_CLIENT_PTR		pCloudClient;
	FTOM_CLOUD_CLIENT_MODULE_PTR	pCloudClientModule;

	FTOM_MSG_QUEUE_PTR		pMsgQ;

	pthread_t				xThreadMain;
	pthread_t				xThreadTimer;
	FTM_BOOL				bStop;


}	FTOM_BLOCKER, _PTR_ FTOM_BLOCKER_PTR;

FTM_RET	FTOM_BLOCKER_create
(
	FTM_CHAR_PTR	pCloudName,
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
