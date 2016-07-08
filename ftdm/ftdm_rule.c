#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_rule.h"
#include "ftdm_action.h"
#include "ftdm_trigger.h"
#include "ftdm_sqlite.h"
#include "ftdm_log.h"

FTM_RET	FTDM_RULE_init
(
	FTM_VOID
)
{
	return	FTM_RULE_init();
}

FTM_RET FTDM_RULE_final
(
	FTM_VOID
)
{
	return	FTM_RULE_final();
}

FTM_RET	FTDM_RULE_loadConfig
(
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xRule;
	FTM_CONFIG_ITEM		xRules;
	FTM_CONFIG_ITEM		xRuleItem;

	xRet = FTM_CONFIG_getItem(pConfig, "event", &xRule);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xRule, "rules", &xRules);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xRules, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xRules, i, &xRuleItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_RULE		xInfo;
						FTM_CONFIG_ITEM	xRulesItem;
						FTM_CONFIG_ITEM	xActionsItem;

						FTM_RULE_setDefault(&xInfo);

						FTM_CONFIG_ITEM_getItemString(&xRuleItem, "id", xInfo.pID, FTM_ID_LEN);
						FTM_CONFIG_ITEM_getItemString(&xRuleItem, "name", xInfo.pName, FTM_NAME_LEN);
						xInfo.xParams.ulTriggers = 0;

						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "triggers", &xRulesItem);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get triggers error!\n");
							continue;
						}
				
						FTM_ULONG	j, ulRuleCount;

						xRet = FTM_CONFIG_LIST_getItemCount(&xRulesItem, &ulRuleCount);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get trigger count error!\n");
							continue;
						}
				
						for(j = 0 ; j < ulRuleCount ; j++)
						{
							FTM_CONFIG_ITEM	xRuleItem;

							xRet = FTM_CONFIG_LIST_getItemAt(&xRulesItem, j, &xRuleItem);	
							if (xRet == FTM_RET_OK)
							{
								FTM_ULONG	ulIndex;
								FTDM_TRIGGER_PTR	pTrigger;

								xRet = FTM_CONFIG_ITEM_getULONG(&xRuleItem, &ulIndex);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Get trigger index error!\n");
									continue;	
								}

								xRet = FTDM_TRIGGER_getByIndex(ulIndex, &pTrigger);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Get trigger index error!\n");
									continue;	
								}

								strcpy(xInfo.xParams.pTriggers[xInfo.xParams.ulTriggers++], pTrigger->xInfo.pID);
							}
							else
							{
								ERROR("Get rule[%d] load failed.\n", j);	
							}
						
						}

						xInfo.xParams.ulActions = 0;
						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "actions", &xActionsItem);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						FTM_ULONG	ulActionCount;

						xRet = FTM_CONFIG_LIST_getItemCount(&xActionsItem, &ulActionCount);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
			
						for(j = 0 ; j < ulActionCount ; j++)
						{
							FTM_CONFIG_ITEM	xActionItem;

							xRet = FTM_CONFIG_LIST_getItemAt(&xActionsItem, j, &xActionItem);	
							if (xRet == FTM_RET_OK)
							{
								FTM_ULONG	ulIndex;
								FTDM_ACTION_PTR	pAction;

								xRet = FTM_CONFIG_ITEM_getULONG(&xActionItem, &ulIndex);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Action index get failed.\n");
									continue;	
								}

								xRet = FTDM_ACTION_getByIndex(ulIndex, &pAction);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Action[%d] get failed.\n");
									continue;	
								}

								strcpy(xInfo.xParams.pActions[xInfo.xParams.ulActions++], pAction->xInfo.pID);
							}
						
						}

						xRet = FTDM_RULE_create(&xInfo);
						FTDM_LOG_createRule(xInfo.pID, xRet);
					
					}
				}
			}
		}
	}
	else
	{
		TRACE("Section [action] not found.\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTDM_RULE_loadConfig(pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTDM_RULE_loadFromDB
(
	FTM_VOID
)
{
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_RULE_count(&nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_RULE_PTR	pRules;
		FTM_ULONG		nRuleCount = 0;
		
		pRules = (FTM_RULE_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_RULE));
		if (pRules == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_RULE_getList(pRules, nMaxCount, &nRuleCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nRuleCount ; i++)
			{
				FTM_RULE_create(&pRules[i]);
			}
		}

		FTM_MEM_free(pRules);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_saveToDB
(
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTM_RULE_PTR	pRule;
	
	xRet = FTM_RULE_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_RULE_getAt(i, &pRule);
		if (xRet == FTM_RET_OK)
		{
			FTM_RULE	xInfo;

			xRet = FTDM_DBIF_RULE_get(pRule->pID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_RULE_create(&xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Failed to save the new trigger.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR("Failed to get trigger information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_create
(
	FTM_RULE_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_RULE_PTR	pRule;

	if (FTDM_RULE_get(pInfo->pID, &pRule) == FTM_RET_OK)
	{
		ERROR("Rule[%s] already exist.\n", pInfo->pID);
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	if (strlen(pInfo->pID) == 0)
	{
		do 
		{	
			struct timeval	xTime;

			gettimeofday(&xTime, NULL);
			sprintf(pInfo->pID, "%08lx%08lx", (FTM_ULONG)xTime.tv_sec, (FTM_ULONG)xTime.tv_usec);
			usleep(10);
		}
		while (FTDM_RULE_get(pInfo->pID, &pRule) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_RULE_create(pInfo);
	if (xRet == FTM_RET_OK)
	{
		pRule = (FTDM_RULE_PTR)FTM_MEM_malloc(sizeof(FTDM_RULE));
		if (pRule == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_RULE_destroy(pInfo->pID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pRule, 0, sizeof(FTDM_RULE));
		memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
		xRet = FTM_RULE_append((FTM_RULE_PTR)pRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Rule[%s] append failed.\n", pRule->xInfo.pID);
			FTDM_DBIF_RULE_destroy(pInfo->pID);
			FTM_MEM_free(pRule);
		}
	}
	else
	{
		ERROR("Rule[%s] DB append failed.\n", pInfo->pID);	
	}

	return	xRet;
}

FTM_RET	FTDM_RULE_destroy
(
	FTM_CHAR_PTR	pRuleID
)
{
	FTM_RET				xRet;
	FTDM_RULE_PTR	pRule = NULL;

	xRet = FTDM_RULE_get(pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_RULE_destroy(pRuleID);
	FTM_RULE_remove((FTM_RULE_PTR)pRule);

	FTM_MEM_free(pRule);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_count
(
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_RULE_count(pulCount);
}

FTM_RET	FTDM_RULE_get
(
	FTM_CHAR_PTR	pRuleID,
	FTDM_RULE_PTR	_PTR_ 	ppRule
)
{
	return	FTM_RULE_get(pRuleID, (FTM_RULE_PTR _PTR_)ppRule);
}

FTM_RET	FTDM_RULE_getAt
(
	FTM_ULONG				nIndex,
	FTDM_RULE_PTR	_PTR_ 	ppRule
)
{
	return	FTM_RULE_getAt(nIndex, (FTM_RULE_PTR _PTR_)ppRule);
}

FTM_RET	FTDM_RULE_set
(
	FTM_CHAR_PTR		pID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR		pInfo
)
{
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_RULE_PTR	pRule;

	xRet = FTM_RULE_get(pID, (FTM_RULE_PTR _PTR_)&pRule);
	if (xRet != FTM_RET_OK)
	{
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

	FTDM_DBIF_RULE_set(pID, &pRule->xInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	i, ulCount;
	FTDM_RULE_PTR	pRule;

	xRet = FTDM_RULE_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		xRet = FTDM_RULE_getAt(ulIndex + i, &pRule);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		strcpy(pIDs[(*pulCount)++], pRule->xInfo.pID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_show
(
	FTDM_RULE_PTR	pRule	
)
{
	ASSERT(pRule != NULL);
	FTM_INT	j;

	MESSAGE("# Rule Information\n");

	MESSAGE("%16s : %s\n", "ID", pRule->xInfo.pID);
	MESSAGE("%16s : %s\n", "Name", pRule->xInfo.pName);
	if (pRule->xInfo.xState == FTM_RULE_STATE_ACTIVATE)
	{
		MESSAGE("%16s : %s\n", "State", "Active");
	}
	else
	{
		MESSAGE("%16s : %s\n", "State", "Deactive");
	}
	MESSAGE("%16s : ", "Triggres");
	for(j = 0 ; j < pRule->xInfo.xParams.ulTriggers ; j++)
	{
		MESSAGE(" %16s", pRule->xInfo.xParams.pTriggers[j]);
	}
	MESSAGE("\n");

	MESSAGE("%16s : ", "Actions");
	for(j = 0 ; j < pRule->xInfo.xParams.ulActions ; j++)
	{
		MESSAGE(" %16s", pRule->xInfo.xParams.pActions[j]);
	}
	MESSAGE("\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_showList
(
	FTM_VOID
)
{
	FTM_RULE_PTR	pRule;
	FTM_CHAR		pSpace[17];
	FTM_ULONG		i, ulCount;

	memset(pSpace, ' ', 16);
	pSpace[16] = 0;

	MESSAGE("\n# Rule Information\n");
	MESSAGE("\t%16s %16s %8s %16s %16s\n", "ID", "NAME", "STATE", "RULE", "ACTION");

	FTM_RULE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_RULE_getAt(i, &pRule) == FTM_RET_OK)
		{
			FTM_ULONG	j;

			for(j = 0 ; j < pRule->xParams.ulTriggers || j < pRule->xParams.ulActions ; j++)
			{
				if (j == 0)
				{
					MESSAGE("\t%16s %16s %8s", pRule->pID, pRule->pName, (pRule->xState == FTM_RULE_STATE_ACTIVATE)?"Running":"Stopped");
				}
				else
				{
					MESSAGE("\t%16s %16s %8s", pSpace, pSpace, pSpace);
				}
				if (pRule->xParams.pTriggers[j] != 0)
				{
					MESSAGE(" %16s", pRule->xParams.pTriggers[j]);
				}
				else
				{
					MESSAGE(" %16s", pSpace);
				}

				if (pRule->xParams.pActions[j] != 0)
				{
					MESSAGE(" %16s", pRule->xParams.pActions[j]);
				}
				else
				{
					MESSAGE(" %16s", pSpace);
				}
				MESSAGE("\n");
			}

		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_RULE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTDM_RULE_PTR	pRule = (FTDM_RULE_PTR)pElement;
	FTM_CHAR_PTR	pRuleID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pRule->xInfo.pID, pRuleID) == 0;
}
