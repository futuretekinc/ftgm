#include "ftm.h"
#include "ftom_utils.h"
#include "ftom_trigger.h"

FTM_RET	FTOM_TRIGGER_create
(
	FTM_TRIGGER_PTR	pInfo,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppTrigger != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	pTrigger = (FTOM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTOM_TRIGGER));
	if (pTrigger == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_TRIGGER_init(pTrigger, pInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);	
		return	xRet;
	}

	*ppTrigger = pTrigger; 

	return	xRet;
}

FTM_RET	FTOM_TRIGGER_destroy
(
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(ppTrigger != NULL);

	FTOM_TRIGGER_final(*ppTrigger);

	FTM_MEM_free(*ppTrigger);
	*ppTrigger = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_init
(
	FTOM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_PTR		pInfo
)
{
	memset(pTrigger, 0, sizeof(FTOM_TRIGGER));
	memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));

	FTM_LOCK_init(&pTrigger->xLock);
	if (strlen(pTrigger->xInfo.pID) == 0)
	{
		FTOM_genNewID(pTrigger->xInfo.pID, 16);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_final
(
	FTOM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pTrigger != NULL);

	FTM_LOCK_final(&pTrigger->xLock);

	return	FTM_RET_OK;
}

