#include <string.h>
#include <unistd.h>
#include "ftom.h"
#include "ftom_msg.h"
#include "ftom_rule.h"
#include "ftom_trigger.h"
#include "ftom_action.h"

#define	FTOM_RULEM_LOOP_INTERVAL	100000
#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTOM_RULEM_process(FTM_VOID_PTR pData);
static 
FTM_BOOL	FTOM_RULEM_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTOM_RULEM_create(FTOM_PTR pOM, FTOM_RULEM_PTR _PTR_ ppRuleM)
{
	ASSERT(ppRuleM != NULL);

	FTM_RET			xRet;
	FTOM_RULEM_PTR	pRuleM;

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

	pRuleM = (FTOM_RULEM_PTR)FTM_MEM_malloc(sizeof(FTOM_RULEM));
	if (pRuleM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_RULEM_init(pOM, pRuleM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRuleM);

		return	xRet;	
	}

	FTM_LIST_append(&xList, pRuleM);

	*ppRuleM = pRuleM;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_RULEM_destroy(FTOM_RULEM_PTR _PTR_ ppRuleM)
{
	ASSERT(ppRuleM != NULL);

	FTM_RET			xRet;

	if (!bInit)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	if (*ppRuleM == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTM_LIST_remove(&xList, *ppRuleM);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTOM_RULEM_final(*ppRuleM);
	FTM_MEM_free(*ppRuleM);

	*ppRuleM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_init
(
	FTOM_PTR	pOM,
	FTOM_RULEM_PTR 		pRuleM
)
{
	ASSERT(pRuleM != NULL);
	FTM_RET	xRet;

	TRACE_CALL();

	memset(pRuleM, 0, sizeof(FTOM_RULEM));

	pRuleM->pOM = pOM;
	xRet = FTOM_MSGQ_create(&pRuleM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pRuleM->pRuleList);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSGQ_destroy(&pRuleM->pMsgQ);
		pRuleM->pMsgQ = NULL;

		return	xRet;	
	}

	FTM_LIST_setSeeker(pRuleM->pRuleList, FTOM_RULEM_seeker);

	pRuleM->bStop = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_final
(
	FTOM_RULEM_PTR pRuleM
)
{
	ASSERT(pRuleM != NULL);

	FTM_RET	xRet;

	if (!pRuleM->bStop)
	{
		FTOM_RULEM_stop(pRuleM);	
	}

	if (pRuleM->pMsgQ)
	{
		FTOM_MSGQ_destroy(&pRuleM->pMsgQ);
		pRuleM->pMsgQ = NULL;
	}

	if (pRuleM->pRuleList)
	{
		FTOM_RULE_PTR pRule;

		FTM_LIST_iteratorStart(pRuleM->pRuleList);
		while(FTM_LIST_iteratorNext(pRuleM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
		{
			xRet = FTM_LIST_remove(pRuleM->pRuleList, pRule);	
			if (xRet == FTM_RET_OK)
			{
				FTM_MEM_free(pRule);	
			}
		}
		FTM_LIST_destroy(pRuleM->pRuleList);
		pRuleM->pRuleList = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_loadConfig
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTOM_RULEM_CONFIG_PTR pConfig
)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pRuleM->xConfig, pConfig, sizeof(FTOM_RULEM_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_start
(
	FTOM_RULEM_PTR 	pRuleM
)
{
	ASSERT(pRuleM != NULL);
	
	FTM_INT	nRet;

	TRACE_CALL();

	if (!pRuleM->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	nRet = pthread_create(&pRuleM->xThread, NULL, FTOM_RULEM_process, pRuleM);
	if (nRet < 0)
	{
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	TRACE("Rule management started.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_stop
(
	FTOM_RULEM_PTR 	pRuleM
)
{
	ASSERT(pRuleM != NULL);

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createQuit(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pRuleM->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		pthread_cancel(pRuleM->xThread);
	}

	pthread_join(pRuleM->xThread, NULL);

	TRACE("Rule management stopped.\n");

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_RULEM_process
(
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pData != NULL);
	FTOM_RULEM_PTR		pRuleM = (FTOM_RULEM_PTR)pData;
	FTM_RET					xRet;
	FTOM_MSG_RULE_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	pRuleM->bStop = FTM_FALSE;

	while(!pRuleM->bStop)
	{
		FTM_TIMER_add(&xTimer, FTOM_RULEM_LOOP_INTERVAL);
	
		do
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);

			xRet = FTOM_MSGQ_timedPop(pRuleM->pMsgQ, ulRemain, (FTOM_MSG_PTR _PTR_)&pMsg);
			if (xRet == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTOM_MSG_TYPE_QUIT:
					{
						pRuleM->bStop = FTM_TRUE;
					}
					break;

				case	FTOM_MSG_TYPE_RULE:
					{
						TRACE("Rule [%s] is updated.\n",	pMsg->pRuleID);
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
		while(!pRuleM->bStop && (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE));

	}

	return	0;
}

FTM_RET	FTOM_RULEM_notifyChanged
(
	FTM_CHAR_PTR	pTriggerID
)
{
	FTOM_RULEM_PTR	pRuleM = NULL;
	FTM_RET			xRet;

	sem_wait(&xLock);

	FTM_LIST_iteratorStart(&xList);
	while(FTM_LIST_iteratorNext(&xList, (FTM_VOID_PTR _PTR_)&pRuleM) == FTM_RET_OK)
	{
		if (pRuleM->pTriggerM != NULL)
		{
			FTOM_RULE_PTR	pRule;
	
			FTM_LIST_iteratorStart(pRuleM->pRuleList);
			while(FTM_LIST_iteratorNext(pRuleM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
			{
				FTM_INT	i;
	
				for(i = 0 ; i < pRule->xInfo.xParams.ulTriggers; i++)
				{
					if (strcasecmp(pRule->xInfo.xParams.pTriggers[i], pTriggerID) == 0)
					{
						FTM_BOOL		bActive = FTM_TRUE;
						FTM_INT			j;
		
						for(j = 0 ; j < pRule->xInfo.xParams.ulTriggers; j++)
						{
							FTOM_TRIGGER_PTR	pTrigger;
							xRet = FTOM_TRIGGERM_get(pRuleM->pTriggerM, pRule->xInfo.xParams.pTriggers[j], &pTrigger);
							if ((xRet != FTM_RET_OK) || (pTrigger->xState != FTOM_TRIGGER_STATE_SET))
							{
								bActive = FTM_FALSE;
								break;
							}
						}


						if (bActive)
						{
							FTOM_RULEM_activate(pRuleM, pRule->xInfo.pID);
						}
						else
						{
							FTOM_RULEM_deactivate(pRuleM, pRule->xInfo.pID);
						}
					}
				}
			}
		}
	}

	sem_post(&xLock);

	return	FTM_RET_OK;
}

FTM_RET FTOM_RULEM_count
(
	FTOM_RULEM_PTR	pRuleM, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pRuleM != NULL);

	TRACE_CALL();

	return	FTM_LIST_count(pRuleM->pRuleList, pulCount);
}

FTM_RET	FTOM_RULEM_add
(
	FTOM_RULEM_PTR	pRuleM, 
	FTM_RULE_PTR 	pInfo
)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTOM_RULE_PTR	pRule;

	pRule = (FTOM_RULE_PTR)FTM_MEM_malloc(sizeof(FTOM_RULE));
	if (pRule == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pRule, 0, sizeof(FTOM_RULE));
	memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
	
	FTM_LOCK_init(&pRule->xLock);
	pRule->pRuleM = pRuleM;

	xRet = FTM_LIST_append(pRuleM->pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRule);	
	}

	return	xRet;
}

FTM_RET	FTOM_RULEM_del
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleM != NULL);

	FTM_RET			xRet;
	FTOM_RULE_PTR	pRule;

	TRACE_CALL();

	xRet = FTM_LIST_get(pRuleM->pRuleList, (FTM_VOID_PTR)&pRuleID, (FTM_VOID_PTR _PTR_)&pRule);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pRuleM->pRuleList, pRule);

		FTM_LOCK_final(&pRule->xLock);
		FTM_MEM_free(pRule);
	}

	return	xRet;	
}


FTM_RET	FTOM_RULEM_get
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_CHAR_PTR	pRuleID,
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	TRACE_CALL();

	return	FTM_LIST_get(pRuleM->pRuleList, (FTM_VOID_PTR)pRuleID, (FTM_VOID_PTR _PTR_)ppRule);
}
FTM_RET	FTOM_RULEM_getAt
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_ULONG 		ulIndex, 
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	TRACE_CALL();

	return	FTM_LIST_getAt(pRuleM->pRuleList, ulIndex, (FTM_VOID_PTR _PTR_)ppRule);
}


FTM_RET	FTOM_RULEM_setTriggerM
(
	FTOM_RULEM_PTR pRuleM, 
	struct FTOM_TRIGGERM_STRUCT *pTriggerM
)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pTriggerM != NULL);
	
	pRuleM->pTriggerM = pTriggerM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_setActionM
(
	FTOM_RULEM_PTR pRuleM, 
	struct FTOM_ACTIONM_STRUCT *pActionM
)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pActionM != NULL);
	
	pRuleM->pActionM = pActionM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULEM_activate
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleM != NULL);

	FTM_RET				xRet;
	FTOM_MSG_RULE_PTR	pMsg;

	xRet = FTOM_MSG_createRule(pRuleID, FTM_TRUE, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pRuleM->pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
	}

	return	xRet;
}

FTM_RET	FTOM_RULEM_deactivate
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleM != NULL);

	FTM_RET				xRet;
	FTOM_MSG_RULE_PTR	pMsg;

	xRet = FTOM_MSG_createRule(pRuleID, FTM_FALSE, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pRuleM->pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
	}

	return	xRet;
}

static
FTM_BOOL	FTOM_RULEM_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
)
{
	ASSERT(pElement != NULL);
	ASSERT(pKey != NULL);
	
	FTOM_RULE_PTR	pRule = (FTOM_RULE_PTR)pElement;
	FTM_CHAR_PTR	pRuleID = (FTM_CHAR_PTR)pKey ;

	return	strcasecmp(pRule->xInfo.pID, pRuleID) == 0;
}
