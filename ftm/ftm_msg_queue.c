#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ftm_msg_queue.h"

typedef	struct FTM_MSG_QUEUE_STRUCT
{
	key_t		xKey;
	FTM_INT		nQueueID;

}	FTM_MSG_QUEUE;

FTM_RET	FTM_MSGQ_create(FTM_ULONG ulMsgSize, FTM_MSG_QUEUE_PTR _PTR_ ppMsgQ)
{
	FTM_MSG_QUEUE_PTR	pMsgQ;
	key_t				xKey;
	FTM_INT				nQueueID;

	pMsgQ = (FTM_MSG_QUEUE_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_QUEUE));
	if (pMsgQ == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pMsgQ->xKey = (key_t)pMsgQ;
	pMsgQ->nQueueID = msgget(pMsgQ->xKey, IPC_CREAT | 0600);

	*ppMsgQ = pMsgQ;

	return	FTM_RET_OK;
}

FTM_RET	FTM_MSGQ_destroy(FTM_MSG_QUEUE_PTR _PTR_ pMsgQ);

FTM_RET FTM_MSGQ_push(FTM_MSG_QUEUE_PTR _PTR_ pMsgQ, FTM_VOID_PTR pMsg);
FTM_RET FTM_MSGQ_pop(FTM_MSG_QUEUE_PTR _PTR_ pMsgQ, FTM_VOID_PTR _PTR_ ppMsg);

