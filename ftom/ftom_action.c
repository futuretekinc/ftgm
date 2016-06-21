#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom_action.h"
#include "ftom_msg.h"
#include "ftom_message_queue.h"
#include "ftom_logger.h"
#include "libconfig.h"


static FTM_VOID_PTR FTOM_ACTION_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTOM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTM_LOCK		xLock;
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
	
	xRet = FTM_LOCK_init(&xLock);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
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

	FTM_LOCK_final(&xLock);

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
	
	FTM_RET	xRet;
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

	xRet = FTOM_DB_ACTION_add(&pAction->xInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pAction);
		return	xRet;
	}

	xRet = FTM_LIST_append(pActionList, pAction);
	if (xRet != FTM_RET_OK)
	{
		FTOM_DB_ACTION_remove(pAction->xInfo.pID);
		FTM_MEM_free(pAction);
		ERROR("Action[%s] failed to add to list[%08x].\n", pAction->xInfo.pID, xRet);
		return	xRet;	
	}

	FTOM_LOG_createAction(&pAction->xInfo);

	*ppAction = pAction;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTION_createFromDB
(
	FTM_CHAR_PTR	pID,
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pID != NULL);
	ASSERT(ppAction != NULL);
	
	FTM_RET	xRet;
	FTM_ACTION		xInfo;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_DB_ACTION_getInfo(pID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	pAction = (FTOM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTOM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pAction, 0, sizeof(FTOM_ACTION));
	memcpy(&pAction->xInfo, &xInfo, sizeof(FTM_ACTION));

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
	FTOM_LOG_destroyAction(&(*ppAction)->xInfo);

	FTM_LIST_remove(pActionList, *ppAction);

	FTM_MEM_free(*ppAction);
	
	*ppAction = NULL;

	return	FTM_RET_OK;	
}


FTM_RET	FTOM_ACTION_start
(
	FTOM_ACTION_PTR pAction
)
{
	FTM_INT	nRet;

	if (pAction == NULL)
	{
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
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTION_stop
(
	FTOM_ACTION_PTR pAction
)
{
	if (pAction == NULL)
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
	}

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
	
	FTM_TIMER_initS(&xTimer, 0);

	bStop = FTM_FALSE;

	while(!bStop)
	{
		FTM_TIMER_addMS(&xTimer, FTOM_ACTION_LOOP_INTERVAL);
	
		do
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remainMS(&xTimer, &ulRemain);

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

FTM_RET	FTOM_ACTION_setInfo
(
	FTOM_ACTION_PTR		pAction,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pAction != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	
	xRet = FTOM_DB_ACTION_setInfo(pAction->xInfo.pID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	switch(pAction->xInfo.xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			if (xFields & FTM_ACTION_FIELD_NAME)
			{
				strcpy(pAction->xInfo.pName, pInfo->pName);
			}

			if (xFields & FTM_ACTION_FIELD_EPID)
			{
				strcpy(pAction->xInfo.xParams.xSet.pEPID, pInfo->xParams.xSet.pEPID);
			}
			
			if (xFields & FTM_ACTION_FIELD_VALUE)
			{
				memcpy(&pAction->xInfo.xParams.xSet.xValue, &pInfo->xParams.xSet.xValue, sizeof(FTM_VALUE));
			}
		}
		break;

	default:
		{
			return	FTM_RET_ERROR;	
		}
	}

	return	FTM_RET_OK;
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

FTM_RET	FTOM_ACTION_print
(
	FTOM_ACTION_PTR	pAction
)
{
	MESSAGE("%16s : %s\n", "ID", pAction->xInfo.pID);
	MESSAGE("%16s : %s\n", "Type", FTM_ACTION_typeString(pAction->xInfo.xType));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTION_printList
(
	FTM_VOID
)
{
	FTM_RET	xRet;
	FTM_INT	i;
	FTM_ULONG	ulCount;
	
	MESSAGE("\n# Action Information\n");
	FTOM_ACTION_count(&ulCount);
	MESSAGE("%16s %16s\n", "ID","TYPE");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_ACTION_PTR	pAction;

		xRet = FTOM_ACTION_getAt(i, &pAction);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("%16s %16s\n", 
				pAction->xInfo.pID, 
				FTM_ACTION_typeString(pAction->xInfo.xType));
		}

	}

	return	FTM_RET_OK;
}
