#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom_action.h"
#include "ftom_msg.h"
#include "libconfig.h"

#define	FTOM_ACTIONM_LOOP_INTERVAL	100000	// 1000 us

#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTOM_ACTIONM_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTOM_ACTIONM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTOM_ACTIONM_create
(
	FTOM_PTR 	pOM, 
	FTOM_ACTIONM_PTR _PTR_ ppActionM
)
{
	ASSERT(ppActionM != NULL);

	FTM_RET				xRet;
	FTOM_ACTIONM_PTR	pActionM;

	if (!bInit)
	{
		xRet = FTM_LIST_init(&xList);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
		bInit = FTM_TRUE;	

		if (sem_init(&xLock, 0, 1) < 0)
		{
			return	FTM_RET_ERROR;	
		}
	}


	pActionM = (FTOM_ACTIONM_PTR)FTM_MEM_malloc(sizeof(FTOM_ACTIONM));
	if (pActionM == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_ACTIONM_init(pActionM, pOM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pActionM);	
	}
	else
	{
		FTM_LIST_append(&xList, pActionM);

		*ppActionM = pActionM;	
	}

	return	xRet;
}

FTM_RET	FTOM_ACTIONM_destroy
(
	FTOM_ACTIONM_PTR _PTR_ ppActionM
)
{
	ASSERT(ppActionM != NULL);
	
	FTM_RET	xRet;
	
	if (*ppActionM == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTM_LIST_remove(&xList, *ppActionM);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTOM_ACTIONM_final(*ppActionM);
	FTM_MEM_free(*ppActionM);

	*ppActionM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTIONM_init
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTOM_PTR 			pOM
)
{
	ASSERT(pActionM != NULL);
	FTM_RET	xRet;

	TRACE_CALL();

	memset(pActionM, 0, sizeof(FTOM_ACTIONM));

	xRet = FTOM_MSGQ_create(&pActionM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pActionM->pActionList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor list creation failed[%08x].\n", xRet);
		FTOM_MSGQ_destroy(&pActionM->pMsgQ);
		return	xRet;	
	}

	pActionM->pOM = pOM;
	FTM_LIST_setSeeker(pActionM->pActionList, FTOM_ACTIONM_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTIONM_final
(
	FTOM_ACTIONM_PTR	pActionM
)
{
	ASSERT(pActionM != NULL);

	FTM_RET			xRet;
	FTOM_ACTION_PTR	pAction;

	TRACE_CALL();

	FTOM_MSGQ_destroy(&pActionM->pMsgQ);
	pActionM->pMsgQ = NULL;


	FTM_LIST_iteratorStart(pActionM->pActionList);
	while(FTM_LIST_iteratorNext(pActionM->pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pActionM->pActionList, pAction);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pAction);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTIONM_loadConfig
(
	FTOM_ACTIONM_PTR	pActionM, 
	FTOM_ACTIONM_CONFIG_PTR pConfig
)
{
	ASSERT(pActionM != NULL);
	ASSERT(pConfig != NULL);

	TRACE_CALL();

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTIONM_loadConfigFromFile
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pActionM != NULL);
	ASSERT(pFileName != NULL);

	TRACE_CALL();
	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTIONM_start
(
	FTOM_ACTIONM_PTR pActionM
)
{
	ASSERT(pActionM != NULL);
	
	FTM_INT	nRet;

	if (pActionM->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	nRet = pthread_create(&pActionM->xThread, NULL, FTOM_ACTIONM_process, pActionM);
	if (nRet < 0)
	{
		ERROR("Can't start Act Manager!\n");
		return	FTM_RET_ERROR;
	}

	TRACE("Action management started.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTIONM_stop
(
	FTOM_ACTIONM_PTR pActionM
)
{
	ASSERT(pActionM != NULL);

	if (!pActionM->bStop)
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

	xRet = FTOM_MSGQ_push(pActionM->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		pthread_cancel(pActionM->xThread);
	}

	pthread_join(pActionM->xThread, NULL);
	TRACE("Action management stopped.\n");

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_ACTIONM_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTOM_ACTIONM_PTR		pActionM = (FTOM_ACTIONM_PTR)pData;
	FTM_RET					xRet;
	FTOM_MSG_ACTION_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	pActionM->bStop = FTM_FALSE;

	while(!pActionM->bStop)
	{
		FTM_TIMER_add(&xTimer, FTOM_ACTIONM_LOOP_INTERVAL);
	
		do
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);

			xRet = FTOM_MSGQ_timedPop(pActionM->pMsgQ, ulRemain, (FTOM_MSG_PTR _PTR_)&pMsg);
			if (xRet == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTOM_MSG_TYPE_QUIT:
					{
						pActionM->bStop = FTM_TRUE;
					}
					break;

				case	FTOM_MSG_TYPE_ACTION:
					{
						TRACE("Actor[%08x] is updated.\n",	pMsg->xActionID);
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
		while (!pActionM->bStop && (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE));
	}

	return	0;
}

FTM_RET FTOM_ACTIONM_count
(
	FTOM_ACTIONM_PTR	pActionM, 
	FTM_ULONG_PTR 		pulCount
)
{
	TRACE_CALL();

	return	FTM_LIST_count(pActionM->pActionList, pulCount);
}

FTM_RET	FTOM_ACTIONM_add
(
	FTOM_ACTIONM_PTR	pActionM, 
	FTM_ACTION_PTR 		pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTOM_ACTION_PTR	pAction;

	TRACE_CALL();

	pAction = (FTOM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTOM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));

	xRet = FTM_LIST_append(pActionM->pActionList, pAction);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pAction);	
	}

	return	xRet;
}

FTM_RET	FTOM_ACTIONM_del
(
	FTOM_ACTIONM_PTR	pActionM, 
	FTM_ACTION_ID  		xActionID
)
{
	FTM_RET			xRet;
	FTOM_ACTION_PTR	pAction;

	TRACE_CALL();

	xRet = FTM_LIST_get(pActionM->pActionList, (FTM_VOID_PTR)&xActionID, (FTM_VOID_PTR _PTR_)&pAction);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pActionM->pActionList, pAction);
		FTM_MEM_free(pAction);
	}

	return	xRet;	
}

FTM_RET	FTOM_ACTIONM_get
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_ACTION_ID 		xActionID, 
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	TRACE_CALL();

	return	FTM_LIST_get(pActionM->pActionList, (FTM_VOID_PTR)&xActionID, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTOM_ACTIONM_getAt
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_ULONG 			ulIndex, 
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	TRACE_CALL();

	return	FTM_LIST_getAt(pActionM->pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTOM_ACTIONM_active
(
	FTOM_ACTIONM_PTR pActionM, 
	FTM_ACTION_ID 	xActionID,
	FTM_BOOL		bActivate
)
{
	FTM_RET				xRet;
	FTOM_MSG_ACTION_PTR	pMsg;

	xRet = FTOM_MSG_createAction(xActionID, bActivate, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action message creation failed[%08x].\n", xRet);
		return	xRet;
	}

	xRet = FTOM_MSGQ_push(pActionM->pMsgQ, (FTM_VOID_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);
	}

	return	xRet;
}

FTM_BOOL	FTOM_ACTIONM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTOM_ACTION_PTR)pElement)->xInfo.xID == *((FTM_ACTION_ID_PTR)pIndicator);
}

