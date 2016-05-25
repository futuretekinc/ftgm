#include "ftm.h"

FTM_RET	FTM_LOCK_create
(
	FTM_LOCK_PTR _PTR_ ppLock
)
{
	ASSERT(ppLock != NULL);
	FTM_RET			xRet;
	FTM_LOCK_PTR	pLock;

	pLock = (FTM_LOCK_PTR)FTM_MEM_malloc(sizeof(FTM_LOCK));
	if (pLock == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;				
	}

	xRet = FTM_LOCK_init(pLock);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pLock);
		return xRet;
	}

	*ppLock = pLock;

	return	xRet;
}

FTM_RET	FTM_LOCK_destroy
(
	FTM_LOCK_PTR _PTR_ ppLock
)
{
	ASSERT(ppLock != NULL);

	if (*ppLock != NULL)
	{
		FTM_LOCK_final(*ppLock);
		FTM_MEM_free(*ppLock);

		*ppLock = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOCK_init
(
	FTM_LOCK_PTR pLock
)
{
	ASSERT(pLock != NULL);

	if (sem_init(&pLock->xKey, 0, 1) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOCK_final
(
	FTM_LOCK_PTR pLock
)
{
	ASSERT(pLock != NULL);

	sem_destroy(&pLock->xKey);

	return	FTM_RET_OK;
}


FTM_RET	FTM_LOCK_set
(
	FTM_LOCK_PTR pLock
)
{
	ASSERT(pLock != NULL);

	sem_wait(&pLock->xKey);

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOCK_reset
(
	FTM_LOCK_PTR pLock
)
{
	ASSERT(pLock != NULL);

	sem_post(&pLock->xKey);

	return	FTM_RET_OK;
}

