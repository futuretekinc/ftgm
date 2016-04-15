#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include "ftm_shared_memory.h"
#include "ftm_trace.h"

FTM_RET	FTM_SM_create
(
	key_t		xKey,
	FTM_ULONG	ulSize,
	FTM_SM_PTR	_PTR_ ppSM
)
{
	ASSERT(ppSM != NULL);

	FTM_SM_PTR		pSM;
	FTM_INT			nID;
	FTM_BOOL		bNew = FTM_FALSE;

	nID = shmget(xKey, sizeof(FTM_SM) + ulSize, 0666);
	if (nID < 0)
	{
		MESSAGE("SM[%d] is exist.\n", nID);
		nID = shmget(xKey, ulSize, IPC_CREAT | 0666);
		bNew = FTM_TRUE;
	}

	if (nID < 0)
	{
		ERROR("Shared memory allocation failed[%d].\n", nID);
		return	FTM_RET_ERROR;
	}

	pSM = (FTM_SM_PTR)shmat(nID, NULL, 0);
	if ((FTM_INT)pSM == -1)
	{
		ERROR("Shared memory don't attached the segment to our space.\n");
		return	FTM_RET_ERROR;
	}

	if (bNew)
	{
		memset(pSM, 0, sizeof(FTM_SM) + ulSize);

		pSM->xKey 	= xKey;	
		pSM->nID	= nID;
		pSM->ulSize	= ulSize;
	}
	
	pSM->ulReference++;

	*ppSM = pSM;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SM_destroy
(
	FTM_SM_PTR	_PTR_ ppSM
)
{
	ASSERT(ppSM != NULL);

	--(*ppSM)->ulReference;

	if ((*ppSM)->ulReference == 0)
	{
		shmctl((*ppSM)->nID, IPC_RMID, (struct shmid_ds *)0);
	}

	*ppSM = NULL;

	return	FTM_RET_OK;
}


FTM_RET	FTM_SM_isValid
(
	FTM_SM_PTR	pSM
)
{
	ASSERT(pSM != NULL);
	FTM_INT	nID;

	nID = shmget(pSM->xKey, sizeof(FTM_SM) + pSM->ulSize, 0666);
	if ((nID > 0) && (nID == pSM->nID))
	{
		return	FTM_RET_TRUE;
	}

	return	FTM_RET_FALSE;
}

