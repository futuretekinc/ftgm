#include <string.h>
#include <unistd.h>
#include "ftom.h"
#include "ftom_msg.h"
#include "ftom_rule.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_utils.h"

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

	xRet = FTOM_RULE_init(pRule, pInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRule);	
		return	xRet;
	}

	*ppRule = pRule;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_destroy
(
	FTOM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(ppRule != NULL);

	FTOM_RULE_final(*ppRule);
	FTM_MEM_free(*ppRule);

	*ppRule = NULL;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_RULE_init
(
	FTOM_RULE_PTR	pRule,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRule != NULL);
	ASSERT(pInfo != NULL);
	
	memset(pRule, 0, sizeof(FTOM_RULE));
	memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
	
	if (strlen(pRule->xInfo.pID) == 0)
	{
		FTOM_genNewID(pRule->xInfo.pID, 16);
	}

	FTM_LOCK_init(&pRule->xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_RULE_final
(
	FTOM_RULE_PTR pRule
)
{
	return	FTM_LOCK_final(&pRule->xLock);
}

