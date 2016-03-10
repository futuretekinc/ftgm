#include <string.h>
#include "ftm_action.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_list.h"

FTM_LIST_PTR	pActionList = NULL;

static FTM_BOOL	FTM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTM_ACTION_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pActionList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pActionList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(pActionList, FTM_ACTION_seeker);	
	}

	return	xRet;
}

FTM_RET	FTM_ACTION_final(FTM_VOID)
{
	FTM_ACTION_PTR	pAction;

	if (pActionList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_LIST_iteratorStart(pActionList);
	while(FTM_LIST_iteratorNext(pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
	{
		FTM_ACTION_destroy(pAction);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_ACTION_createCopy(FTM_ACTION_PTR pAction, FTM_ACTION_PTR _PTR_ ppAction)
{
	ASSERT(pActionList != NULL);

	FTM_RET		xRet;
	FTM_ACTION_PTR	pNewAction;

	pNewAction = (FTM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTM_ACTION));
	if (pNewAction == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewAction, pAction, sizeof(FTM_ACTION));

	xRet = FTM_LIST_append(pActionList, pNewAction);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNewAction);	
	}

	if (ppAction != NULL)
	{
		*ppAction = pAction;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_ACTION_createSet(FTM_ACTION_ID xID, FTM_ACTION_TYPE xType, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_ACTION_PTR _PTR_ ppAction)
{
	ASSERT(pActionList != NULL);

	FTM_RET		xRet;
	FTM_ACTION_PTR	pAction;

	pAction = (FTM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pAction->xID = xID;
	pAction->xType = xType;
	pAction->xParams.xSet.xEPID = xEPID;
	memcpy(&pAction->xParams.xSet.xValue, pData, sizeof(FTM_EP_DATA));

	xRet = FTM_LIST_append(pActionList, pAction);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pAction);	
	}

	if (ppAction != NULL)
	{
		*ppAction = pAction;
	}

	return	xRet;
}

FTM_RET	FTM_ACTION_destroy(FTM_ACTION_PTR pAction)
{
	ASSERT(pActionList != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pActionList, pAction);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pAction);	
	}

	return	xRet;
}

FTM_RET FTM_ACTION_count(FTM_ULONG_PTR pulCount)
{
	ASSERT(pActionList != NULL);

	return	FTM_LIST_count(pActionList, pulCount);
}

FTM_RET FTM_ACTION_get(FTM_ACTION_ID xID, FTM_ACTION_PTR _PTR_ ppAction)
{
	ASSERT(pActionList != NULL);

	return	FTM_LIST_get(pActionList, (FTM_VOID_PTR)&xID, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET FTM_ACTION_getAt(FTM_ULONG ulIndex, FTM_ACTION_PTR _PTR_ ppAction)
{
	ASSERT(pActionList != NULL);

	return	FTM_LIST_getAt(pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);

}

FTM_BOOL	FTM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTM_ACTION_PTR)pElement)->xID == *((FTM_ACTION_ID_PTR)pIndicator);
}
