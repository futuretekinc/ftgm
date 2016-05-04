#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom_action.h"
#include "ftom_msg.h"
#include "libconfig.h"

#define	FTOM_ACTION_LOOP_INTERVAL	100000	// 1000 us

static FTM_VOID_PTR FTOM_ACTION_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTOM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static sem_t		xLock;
static pthread_t	xThread;
static FTM_BOOL		bStop = FTM_TRUE;
static FTM_LIST_PTR	pActionList = NULL;
static FTOM_MSG_QUEUE_PTR	pMsgQ = NULL;

FTM_RET	FTOM_ACTION_init
(
	FTM_VOID
)
{
	FTM_RET				xRet;

	if (pActionList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}
		
	if (sem_init(&xLock, 0, 1) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	xRet = FTOM_MSGQ_create(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pActionList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor list creation failed[%08x].\n", xRet);
		FTOM_MSGQ_destroy(&pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pActionList, FTOM_ACTION_seeker);

	return	xRet;
}

FTM_RET	FTOM_ACTION_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;
	
	if (pActionList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTOM_MSGQ_destroy(&pMsgQ);

	FTM_LIST_iteratorStart(pActionList);
	while(FTM_LIST_iteratorNext(pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pActionList, pAction);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pAction);	
		}
	}

	FTM_LIST_destroy(pActionList);
	pActionList = NULL;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_ACTION_create
(
	FTM_ACTION_PTR 		pInfo,
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppAction != NULL);

	FTOM_ACTION_PTR	pAction;

	pAction = (FTOM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTOM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pAction, 0, sizeof(FTOM_ACTION));
	memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));

	if (strlen(pAction->xInfo.pID) == 0)
	{
		FTM_makeID(pAction->xInfo.pID, 16);
	}

	FTM_LIST_append(pActionList, pAction);

	*ppAction = pAction;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTION_destroy
(
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	FTM_LIST_remove(pActionList, *ppAction);

	FTM_MEM_free(*ppAction);
	
	*ppAction = NULL;

	return	FTM_RET_OK;	
}


FTM_RET	FTOM_ACTION_start
(
	FTM_VOID
)
{
	FTM_INT	nRet;

	if (bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	nRet = pthread_create(&xThread, NULL, FTOM_ACTION_process, NULL);
	if (nRet < 0)
	{
		ERROR("Can't start Act Manager!\n");
		return	FTM_RET_ERROR;
	}

	TRACE("Action management started.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTION_stop
(
	FTM_VOID
)
{
	if (!bStop)
	{
		return	FTM_RET_NOT_START;	
	}
	
	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createQuit(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		pthread_cancel(xThread);
	}

	pthread_join(xThread, NULL);
	TRACE("Action management stopped.\n");

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_ACTION_process
(
	FTM_VOID_PTR	pData
)
{
	FTM_RET					xRet;
	FTOM_MSG_ACTION_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	bStop = FTM_FALSE;

	while(!bStop)
	{
		FTM_TIMER_add(&xTimer, FTOM_ACTION_LOOP_INTERVAL);
	
		do
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);

			xRet = FTOM_MSGQ_timedPop(pMsgQ, ulRemain, (FTOM_MSG_PTR _PTR_)&pMsg);
			if (xRet == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTOM_MSG_TYPE_QUIT:
					{
						bStop = FTM_TRUE;
					}
					break;

				case	FTOM_MSG_TYPE_ACTION:
					{
						TRACE("Actor[%s] is updated.\n",	pMsg->pActionID);
					}
					break;

				default:
					{
						TRACE("Unknown message[%08x].\n", pMsg->xType);	
					}
				}
				FTM_MEM_free(pMsg);
			}
		}	
		while (!bStop && (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE));
	}

	return	0;
}

FTM_RET FTOM_ACTION_count
(
	FTM_ULONG_PTR 		pulCount
)
{
	return	FTM_LIST_count(pActionList, pulCount);
}

FTM_RET	FTOM_ACTION_get
(
	FTM_CHAR_PTR		pActionID,
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	return	FTM_LIST_get(pActionList, (FTM_VOID_PTR)pActionID, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTOM_ACTION_getAt
(
	FTM_ULONG 			ulIndex, 
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	return	FTM_LIST_getAt(pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTOM_ACTION_active
(
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL		bActivate
)
{
	FTM_RET				xRet;
	FTOM_MSG_ACTION_PTR	pMsg;

	xRet = FTOM_MSG_createAction(pActionID, bActivate, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action message creation failed[%08x].\n", xRet);
		return	xRet;
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTM_VOID_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);
	}

	return	xRet;
}

FTM_BOOL	FTOM_ACTION_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_ACTION_PTR	pAction = (FTOM_ACTION_PTR)pElement;
	FTM_CHAR_PTR	pActionID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pAction->xInfo.pID, pActionID) == 0;
}

