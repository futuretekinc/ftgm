#include "ftm_act.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftm_list.h"

FTM_LIST_PTR	pActorList = NULL;

static FTM_BOOL	FTM_ACT_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTM_ACT_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pActorList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pActorList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(pActorList, FTM_ACT_seeker);	
	}

	return	xRet;
}

FTM_RET	FTM_ACT_final(FTM_VOID)
{
	FTM_ACT_PTR	pActor;

	if (pActorList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_LIST_iteratorStart(pActorList);
	while(FTM_LIST_iteratorNext(pActorList, (FTM_VOID_PTR _PTR_)&pActor) == FTM_RET_OK)
	{
		FTM_ACT_destroy(pActor);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_ACT_create(FTM_ACT_ID xID, FTM_ACT_TYPE xType, FTM_EPID xTargetID, FTM_ACT_PTR _PTR_ ppActor)
{
	ASSERT(pActorList != NULL);

	FTM_RET		xRet;
	FTM_ACT_PTR	pActor;

	pActor = (FTM_ACT_PTR)FTM_MEM_malloc(sizeof(FTM_ACT));
	if (pActor == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pActor->xID = xID;
	pActor->xType = xType;
	pActor->xTargetID = xTargetID;

	xRet = FTM_LIST_append(pActorList, pActor);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pActor);	
	}

	if (ppActor != NULL)
	{
		*ppActor = pActor;
	}

	return	xRet;
}

FTM_RET	FTM_ACT_destroy(FTM_ACT_PTR pActor)
{
	ASSERT(pActorList != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pActorList, pActor);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pActor);	
	}

	return	xRet;
}

FTM_RET FTM_ACT_count(FTM_ULONG_PTR pulCount)
{
	ASSERT(pActorList != NULL);

	return	FTM_LIST_count(pActorList, pulCount);
}

FTM_RET FTM_ACT_get(FTM_ACT_ID xID, FTM_ACT_PTR _PTR_ ppActor)
{
	ASSERT(pActorList != NULL);

	return	FTM_LIST_get(pActorList, (FTM_VOID_PTR)&xID, (FTM_VOID_PTR _PTR_)ppActor);
}

FTM_RET FTM_ACT_getAt(FTM_ULONG ulIndex, FTM_ACT_PTR _PTR_ ppActor)
{
	ASSERT(pActorList != NULL);

	return	FTM_LIST_getAt(pActorList, ulIndex, (FTM_VOID_PTR _PTR_)ppActor);

}

FTM_BOOL	FTM_ACT_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTM_ACT_PTR)pElement)->xID == *((FTM_ACT_ID_PTR)pIndicator);
}
