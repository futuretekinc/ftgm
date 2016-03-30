#include <string.h>
#include <unistd.h>
#include "ftm_om.h"
#include "ftm_om_msg.h"
#include "ftm_om_rule.h"
#include "ftm_om_trigger.h"
#include "ftm_om_action.h"

#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTM_OM_RULEM_process(FTM_VOID_PTR pData);
static FTM_RET	FTM_OM_RULE_activate(FTM_OM_RULE_PTR pRule);
static FTM_RET	FTM_OM_RULE_deactivate(FTM_OM_RULE_PTR pRule);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTM_OM_RULEM_create(FTM_OM_PTR pOM, FTM_OM_RULEM_PTR _PTR_ ppRuleM)
{
	ASSERT(ppRuleM != NULL);

	FTM_RET			xRet;
	FTM_OM_RULEM_PTR	pRuleM;

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

	pRuleM = (FTM_OM_RULEM_PTR)FTM_MEM_malloc(sizeof(FTM_OM_RULEM));
	if (pRuleM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_OM_RULEM_init(pOM, pRuleM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRuleM);

		return	xRet;	
	}

	FTM_LIST_append(&xList, pRuleM);

	*ppRuleM = pRuleM;

	return	FTM_RET_OK;
}


FTM_RET	FTM_OM_RULEM_destroy(FTM_OM_RULEM_PTR _PTR_ ppRuleM)
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

	FTM_OM_RULEM_final(*ppRuleM);
	FTM_MEM_free(*ppRuleM);

	*ppRuleM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_RULEM_init
(
	FTM_OM_PTR	pOM,
	FTM_OM_RULEM_PTR 		pRuleM
)
{
	ASSERT(pRuleM != NULL);
	FTM_RET	xRet;

	TRACE_CALL();

	memset(pRuleM, 0, sizeof(FTM_OM_RULEM));

	pRuleM->pOM = pOM;
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

FTM_RET	FTM_OM_RULEM_final
(
	FTM_OM_RULEM_PTR pRuleM
)
{
	ASSERT(pRuleM != NULL);

	TRACE_CALL();

	if (!pRuleM->bStop)
	{
		FTM_OM_RULEM_stop(pRuleM);	
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

FTM_RET	FTM_OM_RULEM_loadConfig(FTM_OM_RULEM_PTR pRuleM, FTM_OM_RULEM_CONFIG_PTR pConfig)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pRuleM->xConfig, pConfig, sizeof(FTM_OM_RULEM_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_RULEM_start(FTM_OM_RULEM_PTR pRuleM)
{
	ASSERT(pRuleM != NULL);
	
	FTM_INT	nRet;

	TRACE_CALL();

	if (!pRuleM->bStop)
	{
		return	FTM_RET_ALREADY_RUNNING;	
	}

	nRet = pthread_create(&pRuleM->xThread, NULL, FTM_OM_RULEM_process, pRuleM);
	if (nRet < 0)
	{
		return	FTM_RET_THREAD_CREATION_ERROR;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_RULEM_stop(FTM_OM_RULEM_PTR pRuleM)
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

FTM_VOID_PTR FTM_OM_RULEM_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_OM_RULEM_PTR	pRuleM = (FTM_OM_RULEM_PTR)pData;

	TRACE_CALL();

	pRuleM->bStop = FTM_FALSE;

	while(!pRuleM->bStop)
	{
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTM_OM_RULEM_notifyChanged(FTM_TRIGGER_ID xTriggerID)
{
	FTM_OM_RULEM_PTR	pRuleM = NULL;
	FTM_RET			xRet;

	sem_wait(&xLock);

	FTM_LIST_iteratorStart(&xList);
	while(FTM_LIST_iteratorNext(&xList, (FTM_VOID_PTR _PTR_)&pRuleM) == FTM_RET_OK)
	{
		if (pRuleM->pTriggerM != NULL)
		{
			FTM_OM_RULE_PTR	pRule;
	
			FTM_LIST_iteratorStart(pRuleM->pRuleList);
			while(FTM_LIST_iteratorNext(pRuleM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
			{
				FTM_INT	i;
	
				for(i = 0 ; i < pRule->xInfo.xParams.ulTriggers; i++)
				{
					if (pRule->xInfo.xParams.pTriggers[i] == xTriggerID)
					{
						FTM_BOOL		bActive = FTM_TRUE;
						FTM_INT			j;
		
						for(j = 0 ; j < pRule->xInfo.xParams.ulTriggers; j++)
						{
							FTM_OM_TRIGGER_PTR	pTrigger;
							xRet = FTM_OM_TRIGGERM_get(pRuleM->pTriggerM, pRule->xInfo.xParams.pTriggers[j], &pTrigger);
							if ((xRet != FTM_RET_OK) || (pTrigger->xState != FTM_OM_TRIGGER_STATE_SET))
							{
								bActive = FTM_FALSE;
								break;
							}
						}


						if (bActive)
						{
							FTM_OM_RULE_activate(pRule);
						}
						else
						{
							FTM_OM_RULE_deactivate(pRule);
						}
					}
				}
			}
		}
	}

	sem_post(&xLock);

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_RULEM_count(FTM_OM_RULEM_PTR pRuleM, FTM_ULONG_PTR pulCount)
{
	ASSERT(pRuleM != NULL);

	TRACE_CALL();

	return	FTM_LIST_count(pRuleM->pRuleList, pulCount);
}

FTM_RET	FTM_OM_RULEM_add(FTM_OM_RULEM_PTR pRuleM, FTM_RULE_PTR pInfo)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTM_OM_RULE_PTR	pRule;

	TRACE_CALL();

	pRule = (FTM_OM_RULE_PTR)FTM_MEM_malloc(sizeof(FTM_OM_RULE));
	if (pRule == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pRule, 0, sizeof(FTM_OM_RULE));
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

FTM_RET	FTM_OM_RULEM_del(FTM_OM_RULEM_PTR pRuleM, FTM_OM_RULE_ID  xEventID)
{
	ASSERT(pRuleM != NULL);

	FTM_RET			xRet;
	FTM_OM_RULE_PTR	pRule;

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


FTM_RET	FTM_OM_RULEM_getAt(FTM_OM_RULEM_PTR pRuleM, FTM_ULONG ulIndex, FTM_OM_RULE_PTR _PTR_ ppRule)
{
	TRACE_CALL();

	return	FTM_LIST_getAt(pRuleM->pRuleList, ulIndex, (FTM_VOID_PTR _PTR_)ppRule);
}


FTM_RET	FTM_OM_RULEM_setTriggerM(FTM_OM_RULEM_PTR pRuleM, struct FTM_OM_TRIGGERM_STRUCT *pTriggerM)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pTriggerM != NULL);
	
	pRuleM->pTriggerM = pTriggerM;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_RULEM_setActionM(FTM_OM_RULEM_PTR pRuleM, struct FTM_OM_ACTIONM_STRUCT *pActionM)
{
	ASSERT(pRuleM != NULL);
	ASSERT(pActionM != NULL);
	
	pRuleM->pActionM = pActionM;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_RULE_activate(FTM_OM_RULE_PTR pRule)
{
	ASSERT(pRule != NULL);

	FTM_RET			xRet;
	FTM_OM_MSG_PTR	pMsg;

	if (!pRule->bActive)
	{
		pRule->bActive = FTM_TRUE;
		MESSAGE("RULE %d activated\n", pRule->xInfo.xID);	
	}

	xRet = FTM_OM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}

	pMsg->xType = FTM_OM_MSG_TYPE_RULE;
	pMsg->xParams.xRule.xRuleID 	= pRule->xInfo.xID;
	pMsg->xParams.xRule.xRuleState 	= FTM_RULE_STATE_ACTIVATE;

	xRet = FTM_OM_MSGQ_push(pRule->pRuleM->pOM->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message send failed.\n");
		FTM_OM_MSG_destroy(&pMsg);		
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_RULE_deactivate(FTM_OM_RULE_PTR pRule)
{
	ASSERT(pRule != NULL);

	FTM_RET	xRet;
	FTM_OM_MSG_PTR	pMsg;

	if (pRule->bActive)
	{
		pRule->bActive = FTM_FALSE;
		MESSAGE("RULE %d inactivated\n", pRule->xInfo.xID);	
	}

	xRet = FTM_OM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}

	pMsg->xType = FTM_OM_MSG_TYPE_RULE;
	pMsg->xParams.xRule.xRuleID 	= pRule->xInfo.xID;
	pMsg->xParams.xRule.xRuleState 	= FTM_RULE_STATE_DEACTIVATE;

	xRet = FTM_OM_MSGQ_push(pRule->pRuleM->pOM->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message send failed.\n");
		FTM_OM_MSG_destroy(&pMsg);		
		return	xRet;
	}

	return	FTM_RET_OK;
}
