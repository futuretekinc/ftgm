#include <string.h>
#include <unistd.h>
#include "ftnm_rule.h"
#include "ftnm_trigger.h"
#include "ftnm_action.h"

#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTNM_RULEM_process(FTM_VOID_PTR pData);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTNM_RULEM_create(FTNM_RULEM_PTR _PTR_ ppRuleM)
{
	ASSERT(ppRuleM != NULL);

	FTM_RET			xRet;
	FTNM_RULEM_PTR	pRuleM;

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

	pRuleM = (FTNM_RULEM_PTR)FTM_MEM_malloc(sizeof(FTNM_RULEM));
	if (pRuleM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTNM_RULEM_init(pRuleM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRuleM);

		return	xRet;	
	}

	FTM_LIST_append(&xList, pRuleM);

	*ppRuleM = pRuleM;

	return	FTM_RET_OK;
}


FTM_RET	FTNM_RULEM_destroy(FTNM_RULEM_PTR _PTR_ ppRuleM)
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

	FTNM_RULEM_final(*ppRuleM);
	FTM_MEM_free(*ppRuleM);

	*ppRuleM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_RULEM_init
(
	FTNM_RULEM_PTR pRuleM
)
{
	ASSERT(pRuleM != NULL);
	FTM_RET	xRet;

	TRACE_CALL();

	memset(pRuleM, 0, sizeof(FTNM_RULEM));

	xRet = FTM_MSGQ_create(&pRuleM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pRuleM->pRuleList);
	if (xRet != FTM_RET_OK)
	{
		FTM_MSGQ_destroy(pRuleM->pMsgQ);
		pRuleM->pMsgQ = NULL;

		return	xRet;	
	}
	
	pRuleM->bStop = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_RULEM_final
(
	FTNM_RULEM_PTR pRuleM
)
{
	ASSERT(pRuleM != NULL);

	TRACE_CALL();

	if (!pRuleM->bStop)
	{
		FTNM_RULEM_stop(pRuleM);	
	}

	if (pRuleM->pMsgQ)
	{
		FTM_MSGQ_destroy(pRuleM->pMsgQ);
		pRuleM->pMsgQ = NULL;
	}

	if (pRuleM->pRuleList)
	{
		FTM_LIST_destroy(pRuleM->pRuleList);
		pRuleM->pRuleList = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_RULEM_loadConfig(FTNM_RULEM_PTR pRuleM, FTNM_RULEM_CONFIG_PTR pConfig)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pRuleM->xConfig, pConfig, sizeof(FTNM_RULEM_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTNM_RULEM_start(FTNM_RULEM_PTR pRuleM)
{
	ASSERT(pRuleM != NULL);
	
	FTM_INT	nRet;

	TRACE_CALL();

	if (!pRuleM->bStop)
	{
		return	FTM_RET_ALREADY_RUNNING;	
	}

	nRet = pthread_create(&pRuleM->xThread, NULL, FTNM_RULEM_process, pRuleM);
	if (nRet < 0)
	{
		return	FTM_RET_THREAD_CREATION_ERROR;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTNM_RULEM_stop(FTNM_RULEM_PTR pRuleM)
{
	ASSERT(pRuleM != NULL);

	TRACE_CALL();

	if (pRuleM->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pRuleM->bStop = FTM_TRUE;
	pthread_join(pRuleM->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_RULEM_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTNM_RULEM_PTR	pRuleM = (FTNM_RULEM_PTR)pData;

	TRACE_CALL();

	pRuleM->bStop = FTM_FALSE;

	while(!pRuleM->bStop)
	{
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_RULEM_notifyChanged(FTM_TRIGGER_ID xTriggerID)
{
	FTNM_RULEM_PTR	pRuleM = NULL;
	FTM_RET			xRet;

	sem_wait(&xLock);

	FTM_LIST_iteratorStart(&xList);
	while(FTM_LIST_iteratorNext(&xList, (FTM_VOID_PTR _PTR_)&pRuleM) == FTM_RET_OK)
	{
		if (pRuleM->pTriggerM != NULL)
		{
			FTNM_RULE_PTR	pRule;
	
			FTM_LIST_iteratorStart(pRuleM->pRuleList);
			while(FTM_LIST_iteratorNext(pRuleM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
			{
				FTM_INT	i;
	
				for(i = 0 ; i < pRule->xInfo.xParams.ulTriggers; i++)
				{
					if (pRule->xInfo.xParams.pTriggers[i] == xTriggerID)
					{
						FTM_BOOL	bActive = FTM_TRUE;
						FTM_INT		j;
		
						for(j = 0 ; j < pRule->xInfo.xParams.ulTriggers; j++)
						{
							FTNM_TRIGGER_PTR	pTrigger;
							xRet = FTNM_TRIGGERM_get(pRuleM->pTriggerM, pRule->xInfo.xParams.pTriggers[j], &pTrigger);
							if ((xRet != FTM_RET_OK) || (pTrigger->xState != FTNM_TRIGGER_STATE_SET))
							{
								bActive = FTM_FALSE;
								break;
							}
						}
	
						if (!pRule->bActive && bActive)
						{
							pRule->bActive = FTM_TRUE;
							MESSAGE("RULE %d activated\n", pRule->xInfo.xID);	
						}
						if (pRule->bActive && !bActive)
						{
							pRule->bActive = FTM_FALSE;
							MESSAGE("RULE %d inactivated\n", pRule->xInfo.xID);	
						
						}
					}
				}
			}
		}
	}

	sem_post(&xLock);

	return	FTM_RET_OK;
}

FTM_RET FTNM_RULEM_count(FTNM_RULEM_PTR pRuleM, FTM_ULONG_PTR pulCount)
{
	ASSERT(pRuleM != NULL);

	TRACE_CALL();

	return	FTM_LIST_count(pRuleM->pRuleList, pulCount);
}

FTM_RET	FTNM_RULEM_append(FTNM_RULEM_PTR pRuleM, FTM_RULE_PTR pInfo)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTNM_RULE_PTR	pRule;

	TRACE_CALL();

	pRule = (FTNM_RULE_PTR)FTM_MEM_malloc(sizeof(FTNM_RULE));
	if (pRule == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pRule, 0, sizeof(FTNM_RULE));
	memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
	
	FTM_LOCK_init(&pRule->xLock);

	xRet = FTM_LIST_append(pRuleM->pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRule);	
	}

	return	xRet;
}

FTM_RET	FTNM_RULEM_del(FTNM_RULEM_PTR pRuleM, FTNM_RULE_ID  xEventID)
{
	ASSERT(pRuleM != NULL);

	FTM_RET			xRet;
	FTNM_RULE_PTR	pRule;

	TRACE_CALL();

	xRet = FTM_LIST_get(pRuleM->pRuleList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)&pRule);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pRuleM->pRuleList, pRule);

		FTM_LOCK_final(&pRule->xLock);
		FTM_MEM_free(pRule);
	}

	return	xRet;	
}


FTM_RET	FTNM_RULEM_getAt(FTNM_RULEM_PTR pRuleM, FTM_ULONG ulIndex, FTNM_RULE_PTR _PTR_ ppRule)
{
	TRACE_CALL();

	return	FTM_LIST_getAt(pRuleM->pRuleList, ulIndex, (FTM_VOID_PTR _PTR_)ppRule);
}


FTM_RET	FTNM_RULEM_setTriggerM(FTNM_RULEM_PTR pRuleM, struct FTNM_TRIGGERM_STRUCT *pTriggerM)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pTriggerM != NULL);
	
	pRuleM->pTriggerM = pTriggerM;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_RULEM_setActionM(FTNM_RULEM_PTR pRuleM, struct FTNM_ACTIONM_STRUCT *pActionM)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pActionM != NULL);
	
	pRuleM->pActionM = pActionM;

	return	FTM_RET_OK;
}
