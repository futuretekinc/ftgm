#include "ftm.h"

FTM_RET	FTM_LOCK_init(FTM_LOCK_PTR pLock)
{
	ASSERT(pLock != NULL);

	if (sem_init(&pLock->xKey, 0, 1) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOCK_final(FTM_LOCK_PTR pLock)
{
	ASSERT(pLock != NULL);

	sem_destroy(&pLock->xKey);

	return	FTM_RET_OK;
}


FTM_RET	FTM_LOCK_set(FTM_LOCK_PTR pLock)
{
	ASSERT(pLock != NULL);

	sem_wait(&pLock->xKey);

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOCK_reset(FTM_LOCK_PTR pLock)
{
	ASSERT(pLock != NULL);

	sem_post(&pLock->xKey);

	return	FTM_RET_OK;
}

