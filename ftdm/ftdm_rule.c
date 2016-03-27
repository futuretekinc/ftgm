#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_rule.h"
#include "ftdm_action.h"
#include "ftdm_sqlite.h"

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
						FTM_RULE		xInfo;
						FTM_CONFIG_ITEM	xRulesItem;
						FTM_CONFIG_ITEM	xActionsItem;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xRuleItem, "id", (FTM_INT_PTR)&xInfo.xID);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get trigger id error!\n");
							continue;
						}
			
						xInfo.xParams.ulTriggers = 0;

						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "triggers", &xRulesItem);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Get triggers error!\n");
							continue;
						}
				
						FTM_INT		nRuleIndex;
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
								xRet = FTM_CONFIG_ITEM_getINT(&xRuleItem, (FTM_INT_PTR)&nRuleIndex);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Get trigger index error!\n");
									continue;	
								}
								xInfo.xParams.pTriggers[xInfo.xParams.ulTriggers++] = nRuleIndex;
							}
						
						}

						xInfo.xParams.ulActions = 0;
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

								xInfo.xParams.pActions[xInfo.xParams.ulActions++] = nActionIndex;
							}
						
						}

						FTDM_RULE_create(&xInfo);
					
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

			xRet = FTDM_DBIF_RULE_get(pRule->xID, &xInfo);
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

	if (FTDM_RULE_get(pInfo->xID, &pRule) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	xRet = FTDM_DBIF_RULE_create(pInfo);
	if (xRet == FTM_RET_OK)
	{
		pRule = (FTDM_RULE_PTR)FTM_MEM_malloc(sizeof(FTDM_RULE));
		if (pRule == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_RULE_destroy(pInfo->xID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pRule, 0, sizeof(FTDM_RULE));
		memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
		xRet = FTM_RULE_append((FTM_RULE_PTR)pRule);
		if (xRet != FTM_RET_OK)
		{
			FTDM_DBIF_RULE_destroy(pInfo->xID);
			FTM_MEM_free(pRule);
		}
	}

	return	xRet;
}

FTM_RET	FTDM_RULE_destroy
(
	FTM_RULE_ID	xID
)
{
	FTM_RET				xRet;
	FTDM_RULE_PTR	pRule = NULL;

	xRet = FTDM_RULE_get(xID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_RULE_destroy(xID);
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
	FTM_RULE_ID	xID,
	FTDM_RULE_PTR	_PTR_ 	ppRule
)
{
	return	FTM_RULE_get(xID, (FTM_RULE_PTR _PTR_)ppRule);
}

FTM_RET	FTDM_RULE_getAt
(
	FTM_ULONG				nIndex,
	FTDM_RULE_PTR	_PTR_ 	ppRule
)
{
	return	FTM_RULE_getAt(nIndex, (FTM_RULE_PTR _PTR_)ppRule);
}

FTM_RET	FTDM_RULE_showList
(
	FTM_VOID
)
{
	FTM_RULE_PTR	pRule;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# Rule Information\n");
	MESSAGE("\t%4s %8s %8s\n", "ID", "RULE", "ACTION");

	FTM_RULE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_RULE_getAt(i, &pRule) == FTM_RET_OK)
		{
			FTM_CHAR	pBuff[64];
			FTM_ULONG	ulLen = 0;
			FTM_ULONG	j;

			MESSAGE("\t%4d ", pRule->xID);
			for(j = 0 ; j < pRule->xParams.ulTriggers; j++)
			{
				if (pRule->xParams.pTriggers[j] == 0)
				{
					break;
				}
				if (ulLen != 0)
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, " & %lu", pRule->xParams.pTriggers[j]);
				}
				else
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, "%lu", pRule->xParams.pTriggers[j]);
				}
			}
			MESSAGE("%8s ", pBuff);
			
			ulLen = 0;
			for(j = 0 ; j < pRule->xParams.ulActions ; j++)
			{
				if (pRule->xParams.pActions[j] == 0)
				{
					break;
				}
				if (ulLen != 0)
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, " & %lu", pRule->xParams.pActions[j]);
				}
				else
				{
					ulLen += snprintf(pBuff, sizeof(pBuff) - ulLen, "%lu", pRule->xParams.pActions[j]);
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
