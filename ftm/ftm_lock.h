#ifndef	_FTM_LOCK_H_
#define	_FTM_LOCK_H_

#include "ftm_types.h"
#include <semaphore.h>

typedef	struct
{
	sem_t	xKey;
}	FTM_LOCK, _PTR_ FTM_LOCK_PTR;

FTM_RET	FTM_LOCK_create(FTM_LOCK_PTR _PTR_ ppLock);
FTM_RET	FTM_LOCK_destroy(FTM_LOCK_PTR _PTR_ ppLock);

FTM_RET	FTM_LOCK_init(FTM_LOCK_PTR pLock);
FTM_RET	FTM_LOCK_final(FTM_LOCK_PTR pLock);

FTM_RET	FTM_LOCK_set(FTM_LOCK_PTR pLock);
FTM_RET	FTM_LOCK_reset(FTM_LOCK_PTR pLock);

#endif
