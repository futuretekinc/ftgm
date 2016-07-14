#include <string.h>
#include "ftm_action.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_list.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_OBJECT

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

FTM_RET	FTM_ACTION_create(FTM_ACTION_PTR pAction)
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

	return	FTM_RET_OK;
}

FTM_RET	FTM_ACTION_createSet
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_TYPE xType, 
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue, 
	FTM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pActionID != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pValue != NULL);
	ASSERT(pActionList != NULL);

	FTM_RET		xRet;
	FTM_ACTION_PTR	pAction;

	pAction = (FTM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strncpy(pAction->pID, pActionID, FTM_ID_LEN);
	pAction->xType = xType;
	strcpy(pAction->xParams.xSet.pEPID, pEPID);
	memcpy(&pAction->xParams.xSet.xValue, pValue, sizeof(FTM_EP_DATA));

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

FTM_RET	FTM_ACTION_destroy
(
	FTM_ACTION_PTR pAction
)
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

FTM_RET	FTM_ACTION_setDefault
(
	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pAction != NULL);

	memset(pAction, 0, sizeof(FTM_ACTION));

	pAction->xType = FTM_ACTION_TYPE_NONE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_ACTION_append
(
	FTM_ACTION_PTR pAction
)
{
	ASSERT(pActionList != NULL);
	ASSERT(pAction != NULL);

	return	FTM_LIST_append(pActionList, pAction);
}

FTM_RET	FTM_ACTION_remove
(
	FTM_ACTION_PTR pAction
)
{
	ASSERT(pActionList != NULL);
	ASSERT(pAction != NULL);

	return	FTM_LIST_remove(pActionList, pAction);
}

FTM_RET FTM_ACTION_count
(
	FTM_ULONG_PTR pulCount
)
{
	ASSERT(pActionList != NULL);

	return	FTM_LIST_count(pActionList, pulCount);
}

FTM_RET FTM_ACTION_get
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pActionList != NULL);

	return	FTM_LIST_get(pActionList, (FTM_VOID_PTR)pActionID, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET FTM_ACTION_getAt
(
	FTM_ULONG ulIndex, 
	FTM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pActionList != NULL);

	return	FTM_LIST_getAt(pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);

}

FTM_BOOL	FTM_ACTION_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_ACTION_PTR 	pAction = (FTM_ACTION_PTR)pElement;
	FTM_CHAR_PTR	pActionID = (FTM_CHAR_PTR)pIndicator;
	
	return	strcasecmp(pAction->pID, pActionID) == 0;
}

FTM_CHAR_PTR	FTM_ACTION_typeString
(
	FTM_ACTION_TYPE xType
)
{
	switch(xType)
	{
	case	FTM_ACTION_TYPE_NONE: 	return	"NONE";
	case	FTM_ACTION_TYPE_SET: 	return	"SET";
	case	FTM_ACTION_TYPE_SMS:	return	"SMS";
	case	FTM_ACTION_TYPE_PUSH:	return	"PUSH";
	case	FTM_ACTION_TYPE_MAIL:	return	"MAIL";
	}

	return	"UNKNOWN";
}
