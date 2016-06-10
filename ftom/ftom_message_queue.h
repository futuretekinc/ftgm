#ifndef	_FTOM_MESSAGE_QUEUE_H_
#define	_FTOM_MESSAGE_QUEUE_H_

#include "ftm.h"
#include "ftom_msg.h"

typedef struct
{
	FTM_MSG_QUEUE	xQueue;
} FTOM_MSG_QUEUE, _PTR_ FTOM_MSG_QUEUE_PTR;

FTM_RET FTOM_MSGQ_create
(
	FTOM_MSG_QUEUE_PTR _PTR_ ppMsgQ
);

FTM_RET FTOM_MSGQ_destroy
(
	FTOM_MSG_QUEUE_PTR _PTR_ ppMsgQ
);

FTM_RET FTOM_MSGQ_init
(
	FTOM_MSG_QUEUE_PTR pMsgQ
);

FTM_RET FTOM_MSGQ_final
(
	FTOM_MSG_QUEUE_PTR pMsgQ
);

FTM_RET	FTOM_MSGQ_push
(
	FTOM_MSG_QUEUE_PTR 	pMsgQ, 
	FTOM_MSG_PTR 		pMsg
);

FTM_RET	FTOM_MSGQ_pop
(
	FTOM_MSG_QUEUE_PTR	pMsgQ, 
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

FTM_RET	FTOM_MSGQ_timedPop
(
	FTOM_MSG_QUEUE_PTR 	pMsgQ, 
	FTM_ULONG 				ulTimeout, 
	FTOM_MSG_PTR _PTR_ 	ppMsg
);

#endif
