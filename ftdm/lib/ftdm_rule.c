#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_rule.h"
#include "ftdm_action.h"
#include "ftdm_event.h"
#include "ftdm_dbif.h"
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

	if (xFields & FTM_RULE_FIELD_EVENTS)
	{
		pRule->xInfo.xParams.ulEvents = pInfo->xParams.ulEvents;
		memcpy(pRule->xInfo.xParams.pEvents, pInfo->xParams.pEvents, sizeof(pInfo->xParams.pEvents));
	}

	if (xFields & FTM_RULE_FIELD_ACTIONS)
	{
		pRule->xInfo.xParams.ulActions = pInfo->xParams.ulActions;
		memcpy(pRule->xInfo.xParams.pActions, pInfo->xParams.pActions, sizeof(pInfo->xParams.pActions));
	}

	FTDM_DBIF_setRule(pRule->pDBIF, pID, &pRule->xInfo);

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
	for(j = 0 ; j < pRule->xInfo.xParams.ulEvents ; j++)
	{
		MESSAGE(" %16s", pRule->xInfo.xParams.pEvents[j]);
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

			for(j = 0 ; j < pRule->xParams.ulEvents || j < pRule->xParams.ulActions ; j++)
			{
				if (j == 0)
				{
					MESSAGE("\t%16s %16s %8s", pRule->pID, pRule->pName, (pRule->xState == FTM_RULE_STATE_ACTIVATE)?"Running":"Stopped");
				}
				else
				{
					MESSAGE("\t%16s %16s %8s", pSpace, pSpace, pSpace);
				}
				if (pRule->xParams.pEvents[j] != 0)
				{
					MESSAGE(" %16s", pRule->xParams.pEvents[j]);
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
