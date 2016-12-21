#include <string.h>
#include <unistd.h>
#include "ftom.h"
#include "ftom_db.h"
#include "ftom_msg.h"
#include "ftom_message_queue.h"
#include "ftom_rule.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_logger.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_RULE

static FTM_VOID_PTR FTOM_RULE_process(FTM_VOID_PTR pData);
static 
FTM_BOOL	FTOM_RULE_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static 	FTM_LOCK			xLock;
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

	xRet = FTM_LOCK_init(&xLock);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
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
		FTOM_RULE_stop(NULL);	
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


	FTM_LOCK_final(&xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_create
(
	FTM_RULE_PTR 	pInfo,
	FTM_BOOL		bNew,
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
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Not enough memory\n");
		return	xRet;
	}

	memset(pRule, 0, sizeof(FTOM_RULE));
	memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
	
	if (strlen(pRule->xInfo.pID) == 0)
	{
		FTM_makeID(pRule->xInfo.pID, 16);
	}

	FTM_LOCK_init(&pRule->xLock);

	xRet = FTM_LIST_append(pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Rule[%s] failed to add to list.\n", pRule->xInfo.pID);
		goto error;
	}

	if (bNew)
	{
		xRet = FTOM_DB_RULE_add(&pRule->xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add rule[%s] to DB!\n", pRule->xInfo.pID);
		}
		else
		{
			FTOM_addRuleCreationLog(pRule);
		}
	}

	*ppRule = pRule;

	return	FTM_RET_OK;

error:
	FTM_MEM_free(pRule);

	return	xRet;
}

FTM_RET	FTOM_RULE_destroy
(
	FTOM_RULE_PTR _PTR_ ppRule,
	FTM_BOOL	bIncludeDB
)
{
	ASSERT(ppRule != NULL);
	FTM_RET		xRet;

	if (bIncludeDB)
	{
		xRet = FTOM_DB_RULE_remove((*ppRule)->xInfo.pID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove rule[%s] from DB!\n", (*ppRule)->xInfo.pID);
		}
		else
		{
			FTOM_addRuleRemovalLog((*ppRule)->xInfo.pID);
		}
	}

	xRet = FTM_LIST_remove(pRuleList, *ppRule);
	if (xRet == FTM_RET_OK)
	{
		FTM_LOCK_final(&(*ppRule)->xLock);
		FTM_MEM_free(*ppRule);
	}

	*ppRule = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_start
(
	FTOM_RULE_PTR	pRule
)
{
	FTM_INT	nRet;
	
	if (pRule == NULL)
	{
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
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_stop
(
	FTOM_RULE_PTR	pRule
)
{
	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	if (pRule == NULL)
	{
		xRet = FTOM_MSG_createQuit(pRule, &pMsg);
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
	}
	else
	{
		pRule->xInfo.xState = FTM_RULE_STATE_DEACTIVATE;
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_RULE_process
(
	FTM_VOID_PTR 	pData
)
{
	FTM_RET					xRet;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_initS(&xTimer, 0);

	bStop = FTM_FALSE;

	while(!bStop)
	{
		FTM_TIMER_addMS(&xTimer, FTOM_RULE_LOOP_INTERVAL);
	
		do
		{
			FTM_ULONG		ulRemain = 0;	
			FTOM_MSG_PTR	pBaseMsg;

			FTM_TIMER_remainMS(&xTimer, &ulRemain);

			xRet = FTOM_MSGQ_timedPop(pMsgQ, ulRemain, (FTOM_MSG_PTR _PTR_)&pBaseMsg);
			if (xRet == FTM_RET_OK)
			{
				switch(pBaseMsg->xType)
				{
				case	FTOM_MSG_TYPE_QUIT:
					{
						bStop = FTM_TRUE;
					}
					break;

				case	FTOM_MSG_TYPE_EVENT:
					{	
						FTOM_MSG_EVENT_PTR	pMsg = (FTOM_MSG_EVENT_PTR)pBaseMsg;
						FTOM_RULE_PTR	pRule;

						FTM_LOCK_set(&xLock);
					
						FTM_LIST_iteratorStart(pRuleList);
						while(FTM_LIST_iteratorNext(pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
						{
							FTM_INT	i;
					
							for(i = 0 ; i < pRule->xInfo.xParams.ulTriggers; i++)
							{
								if (strcasecmp(pRule->xInfo.xParams.pTriggers[i], pMsg->pTriggerID) == 0)
								{
									FTM_BOOL		bActivation  = FTM_TRUE;
									FTM_INT			j;
						
									for(j = 0 ; j < pRule->xInfo.xParams.ulTriggers; j++)
									{
										FTOM_TRIGGER_PTR	pTrigger;
										xRet = FTOM_TRIGGER_get(pRule->xInfo.xParams.pTriggers[j], &pTrigger);
										if ((xRet != FTM_RET_OK) || (pTrigger->xState != FTOM_TRIGGER_STATE_SET))
										{
											bActivation  = FTM_FALSE;
											break;
										}
									}
						
					
									FTOM_RULE_activation(pRule->xInfo.pID, bActivation);
			
								}
							}
						}

						FTM_LOCK_reset(&xLock);
					}
					break;

				case	FTOM_MSG_TYPE_RULE_ACTIVATION:
					{
						FTOM_MSG_RULE_ACTIVATION_PTR	pMsg = (FTOM_MSG_RULE_ACTIVATION_PTR)pBaseMsg;
						FTOM_RULE_PTR	pRule;
						FTM_INT			i;

						xRet = FTOM_RULE_get(pMsg->pRuleID, &pRule);
						if (xRet != FTM_RET_OK)
						{
							break;	
						}

						if (pRule->bActive == pMsg->bActivation)
						{
							INFO("The rule[%s] have been already activated.\n", pMsg->pRuleID);
							break;
						}

						pRule->bActive = pMsg->bActivation;
						FTOM_addEventCreationLog(pRule);

						for(i = 0 ; i < pRule->xInfo.xParams.ulActions ; i++)
						{
							FTOM_ACTION_activation(pRule->xInfo.xParams.pActions[i], pRule->bActive);
						}
					}
					break;

				default:
					{
						TRACE("Unknown message[%08x].\n",	pBaseMsg->xType);	
					}
				}

				FTOM_MSG_destroy(&pBaseMsg);
			}
		}
		while(!bStop && (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE));

	}

	return	0;
}

FTM_RET	FTOM_RULE_sendMessage
(
	FTOM_MSG_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	xRet = FTOM_MSGQ_push(pMsgQ, pMsg);

	return	xRet;
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
		ERROR2(xRet, "Failed to set Rule[%s] to DB.\n", pRule->xInfo.pID);	
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


FTM_RET	FTOM_RULE_activation
(
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL		bActivation
)
{
	ASSERT(pRuleID != NULL);

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_RULE_createActivation(NULL, pRuleID, bActivation, &pMsg);
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

FTM_RET	FTOM_RULE_print
(
	FTOM_RULE_PTR	pRule
)
{
	FTM_INT	i;

	MESSAGE("%16s : %s\n", "ID", pRule->xInfo.pID);
	MESSAGE("%16s : %s\n", "Trigger", pRule->xInfo.xParams.pTriggers[0]);
	for(i = 1 ; i < pRule->xInfo.xParams.ulTriggers ; i++)
	{
		MESSAGE("%16s   %s\n", "", pRule->xInfo.xParams.pTriggers[i]);
	}
	MESSAGE("%16s : %s\n", "Action", pRule->xInfo.xParams.pActions[0]);
	for(i = 1 ; i < pRule->xInfo.xParams.ulActions ; i++)
	{
		MESSAGE("%16s   %s\n", "", pRule->xInfo.xParams.pActions[i]);
	}

	return	FTM_RET_OK;
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
	MESSAGE("%16s %16s %16s\n", "ID","TRIGGER", "ACTION");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_RULE_PTR	pRule;

		xRet = FTOM_RULE_getAt(i, &pRule);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	j;

			MESSAGE("%16s", pRule->xInfo.pID);
			
			for(j = 0 ; j < pRule->xInfo.xParams.ulTriggers || j < pRule->xInfo.xParams.ulActions ; j++)
			{
				if (j != 0)
				{
					MESSAGE("%16s", "");
				}

				if (j < pRule->xInfo.xParams.ulTriggers)
				{
					MESSAGE(" %16s", pRule->xInfo.xParams.pTriggers[j]);
				}
				else
				{
					MESSAGE(" %16s", "");
				}

				if (j < pRule->xInfo.xParams.ulActions)
				{
					MESSAGE(" %16s", pRule->xInfo.xParams.pActions[j]);
				}
				else
				{
					MESSAGE(" %16s", "");
				}
			}
			MESSAGE("\n");
		}

	}

	return	FTM_RET_OK;
}
