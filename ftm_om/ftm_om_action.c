#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftm_om_action.h"
#include "ftm_om_msg.h"
#include "libconfig.h"

#define	FTM_OM_ACTIONM_LOOP_INTERVAL	1000	// 1000 us

#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTM_OM_ACTIONM_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTM_OM_ACTIONM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTM_OM_ACTIONM_create(FTM_OM_ACTIONM_PTR _PTR_ ppActionM)
{
	ASSERT(ppActionM != NULL);

	FTM_RET				xRet;
	FTM_OM_ACTIONM_PTR	pActionM;

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


	pActionM = (FTM_OM_ACTIONM_PTR)FTM_MEM_malloc(sizeof(FTM_OM_ACTIONM));
	if (pActionM == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_OM_ACTIONM_init(pActionM);
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

FTM_RET	FTM_OM_ACTIONM_destroy(FTM_OM_ACTIONM_PTR _PTR_ ppActionM)
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

	FTM_OM_ACTIONM_final(*ppActionM);
	FTM_MEM_free(*ppActionM);

	*ppActionM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_ACTIONM_init(FTM_OM_ACTIONM_PTR pActionM)
{
	ASSERT(pActionM != NULL);
	FTM_RET	xRet;

	TRACE_CALL();

	memset(pActionM, 0, sizeof(FTM_OM_ACTIONM));

	xRet = FTM_MSGQ_create(&pActionM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pActionM->pActionList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pActionM->pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pActionM->pActionList, FTM_OM_ACTIONM_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_ACTIONM_final(FTM_OM_ACTIONM_PTR pActionM)
{
	ASSERT(pActionM != NULL);

	FTM_RET			xRet;
	FTM_OM_ACTION_PTR	pAction;

	TRACE_CALL();

	FTM_MSGQ_destroy(pActionM->pMsgQ);
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

FTM_RET	FTM_OM_ACTIONM_loadConfig(FTM_OM_ACTIONM_PTR pActionM, FTM_OM_ACTIONM_CONFIG_PTR pConfig)
{
	ASSERT(pActionM != NULL);
	ASSERT(pConfig != NULL);

	TRACE_CALL();

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_ACTIONM_loadConfigFromFile(FTM_OM_ACTIONM_PTR pActionM, FTM_CHAR_PTR pFileName)
{
	ASSERT(pActionM != NULL);
	ASSERT(pFileName != NULL);

	TRACE_CALL();
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_ACTIONM_start(FTM_OM_ACTIONM_PTR pActionM)
{
	ASSERT(pActionM != NULL);
	
	FTM_INT	nRet;

	TRACE_CALL();

	nRet = pthread_create(&pActionM->xThread, NULL, FTM_OM_ACTIONM_process, pActionM);
	if (nRet < 0)
	{
		ERROR("Can't start Act Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_ACTIONM_stop(FTM_OM_ACTIONM_PTR pActionM)
{
	ASSERT(pActionM != NULL);

	TRACE_CALL();

	pActionM->bStop = FTM_TRUE;
	pthread_join(pActionM->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_OM_ACTIONM_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTM_OM_ACTIONM_PTR		pActionM = (FTM_OM_ACTIONM_PTR)pData;
	FTM_RET					xRet;
	FTM_OM_ACTION_MSG_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	TRACE_CALL();

	FTM_TIMER_init(&xTimer, 0);

	pActionM->bStop = FTM_FALSE;

	while(!pActionM->bStop)
	{
		FTM_TIMER_add(&xTimer, FTM_OM_ACTIONM_LOOP_INTERVAL);
		
		xRet = FTM_MSGQ_timedPop(pActionM->pMsgQ, FTM_OM_ACTIONM_LOOP_INTERVAL, (FTM_VOID_PTR _PTR_)&pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTM_OM_ACTION_MSG_TYPE_RUN:
				{
					TRACE("Actor[%08x] is updated.\n",	pMsg->xActionID);
				}
				break;

			default:
				{
					TRACE("Unknown message.\n");	
				}
			}
			FTM_MEM_free(pMsg);
		}
		
		if (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE)
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);
			usleep(ulRemain);
		}
	}
	return	0;
}

FTM_RET FTM_OM_ACTIONM_count(FTM_OM_ACTIONM_PTR pActionM, FTM_ULONG_PTR pulCount)
{
	TRACE_CALL();

	return	FTM_LIST_count(pActionM->pActionList, pulCount);
}

FTM_RET	FTM_OM_ACTIONM_add(FTM_OM_ACTIONM_PTR pActionM, FTM_ACTION_PTR pInfo)
{
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTM_OM_ACTION_PTR	pAction;

	TRACE_CALL();

	pAction = (FTM_OM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTM_OM_ACTION));
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

FTM_RET	FTM_OM_ACTIONM_del(FTM_OM_ACTIONM_PTR pActionM, FTM_ACTION_ID  xActionID)
{
	FTM_RET			xRet;
	FTM_OM_ACTION_PTR	pAction;

	TRACE_CALL();

	xRet = FTM_LIST_get(pActionM->pActionList, (FTM_VOID_PTR)&xActionID, (FTM_VOID_PTR _PTR_)&pAction);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pActionM->pActionList, pAction);
		FTM_MEM_free(pAction);
	}

	return	xRet;	
}

FTM_RET	FTM_OM_ACTIONM_get(FTM_OM_ACTIONM_PTR pActionM, FTM_ACTION_ID xActionID, FTM_OM_ACTION_PTR _PTR_ ppAction)
{
	TRACE_CALL();

	return	FTM_LIST_get(pActionM->pActionList, (FTM_VOID_PTR)&xActionID, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTM_OM_ACTIONM_getAt(FTM_OM_ACTIONM_PTR pActionM, FTM_ULONG ulIndex, FTM_OM_ACTION_PTR _PTR_ ppAction)
{
	TRACE_CALL();

	return	FTM_LIST_getAt(pActionM->pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTM_OM_ACTIONM_run(FTM_OM_ACTIONM_PTR pActionM, FTM_ACTION_ID xActionID)
{
	FTM_RET				xRet;
	FTM_OM_ACTION_MSG_PTR	pMsg;

	TRACE_CALL();

	pMsg = (FTM_OM_ACTION_MSG_PTR)FTM_MEM_malloc(sizeof(FTM_OM_ACTION_MSG));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory.\n");	
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType = FTM_OM_ACTION_MSG_TYPE_RUN;
	pMsg->xActionID = xActionID;

	xRet = FTM_MSGQ_push(pActionM->pMsgQ, (FTM_VOID_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);
	}

	return	xRet;
}

FTM_BOOL	FTM_OM_ACTIONM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTM_OM_ACTION_PTR)pElement)->xInfo.xID == *((FTM_ACTION_ID_PTR)pIndicator);
}

