#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_rule.h"
#include "ftdm_action.h"

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

FTM_RET	FTDM_RULE_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xRule;
	FTM_CONFIG_ITEM		xRules;
	FTM_CONFIG_ITEM		xRuleItem;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "event", &xRule);
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
						FTM_RULE	xRule;
						FTM_CONFIG_ITEM	xTriggersItem;
						FTM_CONFIG_ITEM	xActionsItem;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xRuleItem, "id", (FTM_INT_PTR)&xRule.xID);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get trigger id error!\n");
							continue;
						}
			
						xRule.ulTriggers = 0;
						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "triggers", &xTriggersItem);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get triggers error!\n");
							continue;
						}
				
						FTM_INT		nTriggerIndex;
						FTM_ULONG	j, ulTriggerCount;

						xRet = FTM_CONFIG_LIST_getItemCount(&xTriggersItem, &ulTriggerCount);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get trigger count error!\n");
							continue;
						}
				
						for(j = 0 ; j < ulTriggerCount ; j++)
						{
							FTM_CONFIG_ITEM	xTriggerItem;

							xRet = FTM_CONFIG_LIST_getItemAt(&xTriggersItem, j, &xTriggerItem);	
							if (xRet == FTM_RET_OK)
							{
								xRet = FTM_CONFIG_ITEM_getINT(&xTriggerItem, (FTM_INT_PTR)&nTriggerIndex);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Get trigger index error!\n");
									continue;	
								}
								xRule.pTriggers[xRule.ulTriggers++] = nTriggerIndex;
							}
						
						}

						xRule.ulActions = 0;
						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "actions", &xActionsItem);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
				
						FTM_INT		nActionIndex;
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
								xRet = FTM_CONFIG_ITEM_getINT(&xActionItem, (FTM_INT_PTR)&nActionIndex);
								if (xRet != FTM_RET_OK)
								{
									continue;	
								}

								xRule.pActions[xRule.ulActions++] = nActionIndex;
							}
						
						}

						FTM_RULE_createCopy(&xRule, NULL);
					
					}
				}
			}
		}
	}
	else
	{
		TRACE("Section [action] not found.\n");
	}


	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_loadFromDB
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_add
(
	FTM_RULE_PTR 	pRule
)
{
	return	FTM_RULE_createCopy(pRule, NULL);
}

FTM_RET	FTDM_RULE_del
(
	FTM_RULE_ID	xID
)
{
	FTM_RET			xRet;
	FTM_RULE_PTR	pAct = NULL;

	xRet = FTM_RULE_get(xID, &pAct);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RULE_destroy(pAct);
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
	FTM_RULE_ID	xID,
	FTM_RULE_PTR	_PTR_ 	ppAct
)
{
	return	FTM_RULE_get(xID, ppAct);
}

FTM_RET	FTDM_RULE_getAt
(
	FTM_ULONG				nIndex,
	FTM_RULE_PTR	_PTR_ 	ppAct
)
{
	return	FTM_RULE_getAt(nIndex, ppAct);
}

FTM_RET	FTDM_RULE_showList
(
	FTM_VOID
)
{
	FTM_RULE_PTR	pRule;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# RULE INFORMATION\n");
	MESSAGE("\t%4s %8s %8s\n", "ID", "TRIGGER", "ACTION");

	FTM_RULE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_RULE_getAt(i, &pRule) == FTM_RET_OK)
		{
			FTM_CHAR	pBuff[64];
			FTM_ULONG	ulLen = 0;
			FTM_ULONG	j;

			MESSAGE("\t%4d ", pRule->xID);
			for(j = 0 ; j < pRule->ulTriggers ; j++)
			{
				if (pRule->pTriggers[j] == 0)
				{
					break;
				}
				if (ulLen != 0)
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, " & %lu", pRule->pTriggers[j]);
				}
				else
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, "%lu", pRule->pTriggers[j]);
				}
			}
			MESSAGE("%8s ", pBuff);
			
			ulLen = 0;
			for(j = 0 ; j < pRule->ulActions ; j++)
			{
				if (pRule->pActions[j] == 0)
				{
					break;
				}
				if (ulLen != 0)
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, " & %lu", pRule->pActions[j]);
				}
				else
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, "%lu", pRule->pActions[j]);
				}
			}
			MESSAGE("%8s\n", pBuff);

		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_RULE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_RULE_PTR	pRule = (FTM_RULE_PTR)pElement;
	FTM_RULE_ID_PTR	pRuleID = (FTM_RULE_ID_PTR)pIndicator;

	return	(pRule->xID == *pRuleID);
}
