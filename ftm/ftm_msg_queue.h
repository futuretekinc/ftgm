#ifndef	_FTM_MSG_QUEUE_H_
#define	_FTM_MSG_QUEUE_H_

#include "ftm_types.h"
#include "ftm_queue.h"
#include <semaphore.h>

typedef	struct FTM_MSG_QUEUE_STRUCT
{
	FTM_QUEUE	xQueue;

	sem_t		xLock;
}	FTM_MSG_QUEUE, _PTR_ FTM_MSG_QUEUE_PTR;


FTM_RET	FTM_MSGQ_create(FTM_MSG_QUEUE_PTR _PTR_ pMsgQ);
FTM_RET	FTM_MSGQ_destroy(FTM_MSG_QUEUE_PTR pMsgQ);

FTM_RET	FTM_MSGQ_init(FTM_MSG_QUEUE_PTR pMsgQ);
FTM_RET	FTM_MSGQ_final(FTM_MSG_QUEUE_PTR pMsgQ);

FTM_RET FTM_MSGQ_push(FTM_MSG_QUEUE_PTR pMsgQ, FTM_VOID_PTR pMsg);
FTM_RET FTM_MSGQ_pop(FTM_MSG_QUEUE_PTR pMsgQ, FTM_VOID_PTR _PTR_ ppMsg);
FTM_RET FTM_MSGQ_timedPop(FTM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG ulTimeout, FTM_VOID_PTR _PTR_ pMsg);

FTM_RET FTM_MSGQ_count(FTM_MSG_QUEUE_PTR pMsgQ, FTM_ULONG_PTR pulCount);

#endif
