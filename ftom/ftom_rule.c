#include <string.h>
#include <unistd.h>
#include "ftom.h"
#include "ftom_msg.h"
#include "ftom_rule.h"
#include "ftom_trigger.h"
#include "ftom_action.h"

#define	FTOM_RULE_LOOP_INTERVAL	100000
#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTOM_RULE_process(FTM_VOID_PTR pData);
static 
FTM_BOOL	FTOM_RULE_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static sem_t	xLock;
static	pthread_t			xThread;
static	FTM_BOOL			bStop = FTM_TRUE;
static	FTOM_MSG_QUEUE_PTR	pMsgQ = NULL;
static	FTM_LIST_PTR		pRuleList = NULL;


FTM_RET	FTOM_RULE_init
(
	FTM_VOID
)
{
	FTM_RET			xRet;

	if (sem_init(&xLock, 0, 1) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	xRet = FTOM_MSGQ_create(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pRuleList);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSGQ_destroy(&pMsgQ);
		pMsgQ = NULL;

		return	xRet;	
	}

	FTM_LIST_setSeeker(pRuleList, FTOM_RULE_seeker);

	bStop = FTM_TRUE;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_RULE_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	if (!bStop)
	{
		FTOM_RULE_stop();	
	}

	if (pMsgQ)
	{
		FTOM_MSGQ_destroy(&pMsgQ);
		pMsgQ = NULL;
	}

	if (pRuleList)
	{
		FTOM_RULE_PTR pRule;

		FTM_LIST_iteratorStart(pRuleList);
		while(FTM_LIST_iteratorNext(pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
		{
			xRet = FTM_LIST_remove(pRuleList, pRule);	
			if (xRet == FTM_RET_OK)
			{
				FTM_MEM_free(pRule);	
			}
		}
		FTM_LIST_destroy(pRuleList);
		pRuleList = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_create
(
	FTM_RULE_PTR 	pInfo,
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppRule != NULL);

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
	
	if (strlen(pRule->xInfo.pID) == 0)
	{
		FTM_makeID(pRule->xInfo.pID, 16);
	}

	xRet = FTOM_DB_RULE_add(&pRule->xInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRule);
		return	xRet;
	}

	FTM_LOCK_init(&pRule->xLock);

	xRet = FTM_LIST_append(pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		FTOM_DB_RULE_remove(pRule->xInfo.pID);
		FTM_MEM_free(pRule);
		ERROR("Rule[%s] failed to add to list[%08x].\n", pRule->xInfo.pID, xRet);
		return	xRet;	
	}
	else
	{
		*ppRule = pRule;
	}

	return	xRet;
}

FTM_RET	FTOM_RULE_createFromDB
(
	FTM_CHAR_PTR	pID,
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(pID != NULL);
	ASSERT(ppRule != NULL);

	FTM_RET			xRet;
	FTM_RULE		xInfo;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_DB_RULE_getInfo(pID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Rule[%s] not found.\n", pID);
		return	xRet;
	}

	pRule = (FTOM_RULE_PTR)FTM_MEM_malloc(sizeof(FTOM_RULE));
	if (pRule == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pRule, 0, sizeof(FTOM_RULE));
	memcpy(&pRule->xInfo, &xInfo, sizeof(FTM_RULE));
	
	if (strlen(pRule->xInfo.pID) == 0)
	{
		FTM_makeID(pRule->xInfo.pID, 16);
	}

	FTM_LOCK_init(&pRule->xLock);

	xRet = FTM_LIST_append(pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		FTOM_DB_RULE_remove(pRule->xInfo.pID);
		FTM_MEM_free(pRule);
		ERROR("Rule[%s] failed to add to list[%08x].\n", pRule->xInfo.pID, xRet);
		return	xRet;	
	}
	else
	{
		*ppRule = pRule;
	}

	return	xRet;
}

FTM_RET	FTOM_RULE_destroy
(
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(ppRule != NULL);

	FTM_LOCK_final(&(*ppRule)->xLock);
	FTM_MEM_free(*ppRule);

	*ppRule = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_start
(
	FTM_VOID
)
{
	FTM_INT	nRet;

	if (!bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	nRet = pthread_create(&xThread, NULL, FTOM_RULE_process, NULL);
	if (nRet < 0)
	{
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	TRACE("Rule management started.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_stop
(
	FTM_VOID
)
{
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

	TRACE("Rule management stopped.\n");

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_RULE_process
(
	FTM_VOID_PTR 	pData
)
{
	FTM_RET					xRet;
	FTOM_MSG_RULE_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	bStop = FTM_FALSE;

	while(!bStop)
	{
		FTM_TIMER_add(&xTimer, FTOM_RULE_LOOP_INTERVAL);
	
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
		while(!bStop && (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE));

	}

	return	0;
}

FTM_RET	FTOM_RULE_notifyChanged
(
	FTM_CHAR_PTR	pTriggerID
)
{
	FTM_RET			xRet;
	FTOM_RULE_PTR	pRule;

	sem_wait(&xLock);

	FTM_LIST_iteratorStart(pRuleList);
	while(FTM_LIST_iteratorNext(pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
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
					xRet = FTOM_TRIGGER_get(pRule->xInfo.xParams.pTriggers[j], &pTrigger);
					if ((xRet != FTM_RET_OK) || (pTrigger->xState != FTOM_TRIGGER_STATE_SET))
					{
						bActive = FTM_FALSE;
						break;
					}
				}
	
	
				if (bActive)
				{
					FTOM_RULE_activate(pRule->xInfo.pID);
				}
				else
				{
					FTOM_RULE_deactivate(pRule->xInfo.pID);
				}
			}
		}
	}

	sem_post(&xLock);

	return	FTM_RET_OK;
}

FTM_RET FTOM_RULE_count
(
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTM_LIST_count(pRuleList, pulCount);
}

FTM_RET	FTOM_RULE_get
(
	FTM_CHAR_PTR	pRuleID,
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	return	FTM_LIST_get(pRuleList, (FTM_VOID_PTR)pRuleID, (FTM_VOID_PTR _PTR_)ppRule);
}

FTM_RET	FTOM_RULE_getAt
(
	FTM_ULONG 		ulIndex, 
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	return	FTM_LIST_getAt(pRuleList, ulIndex, (FTM_VOID_PTR _PTR_)ppRule);
}

FTM_RET	FTOM_RULE_setInfo
(
	FTOM_RULE_PTR	pRule,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRule != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;

	xRet = FTOM_DB_RULE_setInfo(pRule->xInfo.pID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule[%s] DB update failed.\n", pRule->xInfo.pID);	
		return	xRet;
	}

	if (xFields & FTM_RULE_FIELD_NAME)
	{
		strcpy(pRule->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_RULE_FIELD_STATE)
	{
		pRule->xInfo.xState = pInfo->xState;
	}
	
	if (xFields & FTM_RULE_FIELD_TRIGGERS)
	{
		pRule->xInfo.xParams.ulTriggers = pInfo->xParams.ulTriggers;
		memcpy(pRule->xInfo.xParams.pTriggers, pInfo->xParams.pTriggers, sizeof(pInfo->xParams.pTriggers));
	}

	if (xFields & FTM_RULE_FIELD_ACTIONS)
	{
		pRule->xInfo.xParams.ulActions = pInfo->xParams.ulActions;
		memcpy(pRule->xInfo.xParams.pActions, pInfo->xParams.pActions, sizeof(pInfo->xParams.pActions));
	}

	return	xRet;
}


FTM_RET	FTOM_RULE_activate
(
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleID != NULL);

	FTM_RET				xRet;
	FTOM_MSG_RULE_PTR	pMsg;

	xRet = FTOM_MSG_createRule(pRuleID, FTM_TRUE, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
	}

	return	xRet;
}

FTM_RET	FTOM_RULE_deactivate
(
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleID != NULL);

	FTM_RET				xRet;
	FTOM_MSG_RULE_PTR	pMsg;

	xRet = FTOM_MSG_createRule(pRuleID, FTM_FALSE, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
	}

	return	xRet;
}

FTM_BOOL	FTOM_RULE_seeker
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

FTM_RET	FTOM_RULE_printList
(
	FTM_VOID
)
{
	FTM_RET	xRet;
	FTM_INT	i;
	FTM_ULONG	ulCount;

	MESSAGE("\n# Rule Information\n");
	FTOM_RULE_count(&ulCount);
	MESSAGE("\t%16s %24s %24s\n", "ID","TRIGGER", "ACTION");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_RULE_PTR	pRule;

		xRet = FTOM_RULE_getAt(i, &pRule);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	j;

			MESSAGE("\t%16s", pRule->xInfo.pID);
			
			for(j = 0 ; j < sizeof(pRule->xInfo.xParams.pTriggers) / (FTM_ID_LEN+1) ; j++)
			{
				if (j < pRule->xInfo.xParams.ulTriggers)
				{
					MESSAGE(" %2d", pRule->xInfo.xParams.pTriggers[j]);
				}
				else
				{
					MESSAGE(" %2d", 0);
				}
			}

			for(j = 0 ; j < sizeof(pRule->xInfo.xParams.pActions) / (FTM_ID_LEN+1) ; j++)
			{
				if (j < pRule->xInfo.xParams.ulActions)
				{
					MESSAGE(" %2d", pRule->xInfo.xParams.pActions[j]);
				}
				else
				{
					MESSAGE(" %2d", 0);
				}
			}
			MESSAGE("\n");
		}

	}

	return	FTM_RET_OK;
}
