#ifndef	__FTOM_AZURE_BLOCKER_H__
#define	__FTOM_AZURE_BLOCKER_H__

#include "ftom.h"
#include "ftom_net_client.h"
#include "ftom_azure_client.h"
#include "ftom_message_queue.h"

typedef	struct
{
	FTOM_NET_CLIENT_PTR		pNetClient;
	FTOM_AZURE_CLIENT_PTR	pAzureClient;

	FTOM_MSG_QUEUE_PTR		pMsgQ;

	pthread_t				xThreadMain;
	FTM_BOOL				bStop;
}	FTOM_AZURE_BLOCKER, _PTR_ FTOM_AZURE_BLOCKER_PTR;

FTM_RET	FTOM_AZURE_BLOCKER_create
(
	FTOM_AZURE_BLOCKER_PTR _PTR_ ppBlocker
);

FTM_RET	FTOM_AZURE_BLOCKER_destroy
(
	FTOM_AZURE_BLOCKER_PTR _PTR_ ppBlocker
);

FTM_RET	FTOM_AZURE_BLOCKER_start
(
	FTOM_AZURE_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_AZURE_BLOCKER_stop
(
	FTOM_AZURE_BLOCKER_PTR pBlocker
);

FTM_RET FTOM_AZURE_BLOCKER_waitingForFinished
(	
	FTOM_AZURE_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_AZURE_BLOCKER_CONFIG_load
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_AZURE_BLOCKER_CONFIG_save
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR			pConfig
);

FTM_RET	FTOM_AZURE_BLOCKER_CONFIG_show
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker
);

FTM_RET	FTOM_AZURE_BLOCKER_MESSAGE_process
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR			pBaseMsg
);

#endif
