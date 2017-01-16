#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_rule.h"
#include "ftdm_action.h"
#include "ftdm_trigger.h"
#include "ftdm_sqlite.h"
#include "ftdm_log.h"

typedef	struct FTDM_RULE_STRUCT
{
	struct FTDM_CONTEXT_STRUCT _PTR_	pFTDM;
	FTM_RULE	xInfo;
}	FTDM_RULE, _PTR_ FTDM_RULE_PTR;

FTM_RET	FTDM_RULE_create
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_RULE_PTR 	pInfo,
	FTDM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET			xRet;
	FTDM_RULE_PTR	pRule;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF(pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTDM_getRule(pFTDM, pInfo->pID, &pRule);
	if (xRet == FTM_RET_OK)
	{
		ERROR2(FTM_RET_ALREADY_EXIST_OBJECT, "Rule[%s] already exist.\n", pInfo->pID);
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
		while (FTDM_getRule(pFTDM, pInfo->pID, &pRule) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_addRule(pDBIF, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Rule[%s] DB append failed.\n", pInfo->pID);	
		return	xRet;
	}
	
	pRule = (FTDM_RULE_PTR)FTM_MEM_malloc(sizeof(FTDM_RULE));
	if (pRule == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		FTDM_DBIF_deleteRule(pDBIF, pInfo->pID);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pRule, 0, sizeof(FTDM_RULE));
	memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
	pRule->pFTDM = pFTDM;

	return	xRet;
}

FTM_RET	FTDM_RULE_destroy
(
	FTDM_RULE_PTR _PTR_	ppRule
)
{
	ASSERT((*ppRule) != NULL);

	FTM_RET			xRet;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF((*ppRule)->pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTDM_DBIF_deleteRule(pDBIF, (*ppRule)->xInfo.pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy rule in DB.\n");	
	}

	FTM_MEM_free(*ppRule);

	*ppRule = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_init
(
	FTDM_RULE_PTR	pRule
)
{
	ASSERT(pRule != NULL);

	return	FTM_RET_OK;
}

FTM_RET FTDM_RULE_final
(
	FTDM_RULE_PTR	pRule
)
{
	ASSERT(pRule != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_get
(
	FTDM_RULE_PTR	pRule,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRule != NULL);
	ASSERT(pInfo != NULL);

	memcpy(pInfo, &pRule->xInfo, sizeof(FTM_RULE));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_RULE_set
(
	FTDM_RULE_PTR	pRule,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRule != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF(pRule->pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
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

	xRet = FTDM_DBIF_setRule(pDBIF, pRule->xInfo.pID, &pRule->xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set rule attribute to DB.\n");
	}

	return	xRet;
}

FTM_RET	FTDM_RULE_getID
(
	FTDM_RULE_PTR	pRule,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pRule != NULL);
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(pRule->xInfo.pID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strncpy(pBuff, pRule->xInfo.pID, ulBuffLen);

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

FTM_BOOL	FTDM_RULE_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTDM_RULE_PTR		pRule = (FTDM_RULE_PTR)pElement;
	FTM_CHAR_PTR		pRuleID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pRule->xInfo.pID, pRuleID) == 0;
}

FTM_BOOL	FTDM_RULE_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTDM_RULE_PTR		pRule1 = (FTDM_RULE_PTR)pElement1;
	FTDM_RULE_PTR		pRule2 = (FTDM_RULE_PTR)pElement2;

	return	strcasecmp(pRule1->xInfo.pID, pRule2->xInfo.pID);
}


