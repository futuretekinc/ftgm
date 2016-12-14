#ifndef	__FTOM_TP_BLOCKER_H__
#define	__FTOM_TP_BLOCKER_H__

#include "ftom.h"
#include "ftom_net_client.h"
#include "ftom_tp_client.h"
#include "ftom_message_queue.h"

typedef	struct
{
	FTOM_NET_CLIENT_PTR	pNetClient;
	FTOM_TP_CLIENT_PTR	pTPClient;

	FTOM_MSG_QUEUE_PTR	pMsgQ;

	pthread_t			xThreadMain;
	FTM_BOOL			bStop;
}	FTOM_TP_BLOCKER, _PTR_ FTOM_TP_BLOCKER_PTR;

FTM_RET	FTOM_TP_BLOCKER_create
(
	FTOM_TP_BLOCKER_PTR _PTR_ ppBlocker
);

FTM_RET	FTOM_TP_BLOCKER_destroy
(
	FTOM_TP_BLOCKER_PTR _PTR_ ppBlocker
);

FTM_RET	FTOM_TP_BLOCKER_start
(
	FTOM_TP_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_TP_BLOCKER_stop
(
	FTOM_TP_BLOCKER_PTR pBlocker
);

FTM_RET FTOM_TP_BLOCKER_waitingForFinished
(	
	FTOM_TP_BLOCKER_PTR pBlocker
);

FTM_RET	FTOM_TP_BLOCKER_CONFIG_load
(
	FTOM_TP_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_TP_BLOCKER_CONFIG_save
(
	FTOM_TP_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_TP_BLOCKER_CONFIG_show
(
	FTOM_TP_BLOCKER_PTR	pBlocker
);

FTM_RET	FTOM_TP_BLOCKER_MESSAGE_process
(
	FTOM_TP_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR		pBaseMsg
);

#endif
