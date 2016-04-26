#include <string.h>
#include "ftm_rule.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

FTM_LIST_PTR	pRuleList = NULL;

static FTM_BOOL	FTM_RULE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTM_RULE_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pRuleList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pRuleList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(pRuleList, FTM_RULE_seeker);	
	}

	return	xRet;
}

FTM_RET	FTM_RULE_final(FTM_VOID)
{
	FTM_RULE_PTR	pRule;

	if (pRuleList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_LIST_iteratorStart(pRuleList);
	while(FTM_LIST_iteratorNext(pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
	{
		FTM_RULE_destroy(pRule);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_RULE_create(FTM_RULE_PTR pRule)
{
	ASSERT(pRuleList != NULL);

	FTM_RET			xRet;
	FTM_RULE_PTR	pNewRule;

	pNewRule = (FTM_RULE_PTR)FTM_MEM_malloc(sizeof(FTM_RULE));
	if (pNewRule == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewRule, pRule, sizeof(FTM_RULE));

	xRet = FTM_LIST_append(pRuleList, pNewRule);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNewRule);	
	}

	return	FTM_RET_OK;
}
#if 0
FTM_RET	FTM_RULE_createSet(FTM_RULE_ID xID, FTM_RULE_TYPE xType, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_RULE_PTR _PTR_ ppRule)
{
	ASSERT(pRuleList != NULL);

	FTM_RET		xRet;
	FTM_RULE_PTR	pRule;

	pRule = (FTM_RULE_PTR)FTM_MEM_malloc(sizeof(FTM_RULE));
	if (pRule == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pRule->xID = xID;
	pRule->xType = xType;
	pRule->xParams.xSet.xEPID = xEPID;
	memcpy(&pRule->xParams.xSet.xValue, pData, sizeof(FTM_EP_DATA));

	xRet = FTM_LIST_append(pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pRule);	
	}

	if (ppRule != NULL)
	{
		*ppRule = pRule;
	}

	return	xRet;
}
#endif

FTM_RET	FTM_RULE_destroy(FTM_RULE_PTR pRule)
{
	ASSERT(pRuleList != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pRuleList, pRule);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pRule);	
	}

	return	xRet;
}

FTM_RET	FTM_RULE_setDefault(FTM_RULE_PTR pRule)
{
	ASSERT(pRule != NULL);

	memset(pRule, 0, sizeof(FTM_RULE));
	pRule->xState = FTM_RULE_STATE_DEACTIVATE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_RULE_append(FTM_RULE_PTR pRule)
{
	ASSERT(pRuleList != NULL);
	ASSERT(pRule != NULL);

	return	FTM_LIST_append(pRuleList, pRule);
}

FTM_RET	FTM_RULE_remove(FTM_RULE_PTR pRule)
{
	ASSERT(pRuleList != NULL);
	ASSERT(pRule != NULL);

	return	FTM_LIST_remove(pRuleList, pRule);
}

FTM_RET FTM_RULE_count(FTM_ULONG_PTR pulCount)
{
	ASSERT(pRuleList != NULL);

	return	FTM_LIST_count(pRuleList, pulCount);
}

FTM_RET FTM_RULE_get(FTM_RULE_ID xID, FTM_RULE_PTR _PTR_ ppRule)
{
	ASSERT(pRuleList != NULL);

	return	FTM_LIST_get(pRuleList, (FTM_VOID_PTR)&xID, (FTM_VOID_PTR _PTR_)ppRule);
}

FTM_RET FTM_RULE_getAt(FTM_ULONG ulIndex, FTM_RULE_PTR _PTR_ ppRule)
{
	ASSERT(pRuleList != NULL);

	return	FTM_LIST_getAt(pRuleList, ulIndex, (FTM_VOID_PTR _PTR_)ppRule);

}

FTM_BOOL	FTM_RULE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTM_RULE_PTR)pElement)->xID == *((FTM_RULE_ID_PTR)pIndicator);
}
