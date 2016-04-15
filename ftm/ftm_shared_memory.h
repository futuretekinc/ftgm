#ifndef	_FTM_SHARED_MEMORY_H_
#define	_FTM_SHARED_MEMORY_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include "ftm_types.h"

typedef	struct	FTM_SM_STRUCT
{
	key_t		xKey;	
	FTM_INT		nID;
	FTM_ULONG	ulSize;
	FTM_ULONG	ulReference;
	FTM_CHAR	pMem[];
}	FTM_SM, _PTR_ FTM_SM_PTR;

FTM_RET	FTM_SM_create
(
	key_t		xKey,
	FTM_ULONG	ulSize,
	FTM_SM_PTR	_PTR_ ppSM
);

FTM_RET	FTM_SM_destroy
(
	FTM_SM_PTR	_PTR_ ppSM
);

FTM_RET	FTM_SM_isValid
(
	FTM_SM_PTR	pSM
);

#endif
