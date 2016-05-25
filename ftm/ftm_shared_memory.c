#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "ftm_shared_memory.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

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


FTM_RET	FTM_SMQ_create
(
	key_t		xKey,
	FTM_ULONG	ulSize,
	FTM_ULONG	ulCount,
	FTM_SMQ_PTR	_PTR_ ppSMQ
)
{
	ASSERT(ppSMQ != NULL);

	FTM_SMQ_PTR			pSMQ;
	FTM_SMQ_HEAD_PTR	pHead;
	FTM_INT			nShmID;
	FTM_BOOL		bNew = FTM_FALSE;
	sem_t			*pLocker;
	FTM_ULONG		ulMemSize = (sizeof(FTM_SMQ_HEAD) + (ulSize + sizeof(FTM_ULONG)) * ulCount + (4096 - 1)) / 4096 * 4096;
	FTM_CHAR		pLockerName[64];

	pSMQ = (FTM_SMQ_PTR)FTM_MEM_malloc(sizeof(FTM_SMQ));
	if (pSMQ == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nShmID = shmget(xKey, ulMemSize, 0666);
	if (nShmID < 0)
	{
		nShmID = shmget(xKey, ulMemSize, IPC_CREAT | 0666);
		if (nShmID < 0)
		{
			ERROR("shared memory creation failed.\n");	
		}
		
		bNew = FTM_TRUE;
	}

	if (nShmID < 0)
	{
		ERROR("Shared memory allocation failed[%d].\n", nShmID);
		return	FTM_RET_ERROR;
	}

	pHead = (FTM_SMQ_HEAD_PTR)shmat(nShmID, NULL, 0);
	if ((FTM_INT)pHead== -1)
	{
		ERROR("Shared memory don't attached the segment to our space.\n");
		return	FTM_RET_ERROR;
	}


	sprintf(pLockerName, "ftom%d", xKey);

	pLocker = sem_open(pLockerName, 0, 0777, 0);
	if (pLocker == SEM_FAILED)
	{
		pLocker = sem_open(pLockerName, O_CREAT, 0777, 1);
		if (pLocker == SEM_FAILED)
		{
			TRACE("Semaphore creation failed.\n");
			return	FTM_RET_ERROR;	
		}
	}

	if (bNew)
	{
		memset(pHead, 0, ulMemSize);
		pHead->ulReference 	= 0;
		pHead->ulSlotSize 	= ulSize;
		pHead->ulSlotCount 	= ulCount;
		pHead->ulFirst 		= 0;
		pHead->ulCount 		= 0;
		sem_init(&pHead->xSemaphore, 1, ulCount);
		TRACE("New shared memory registed!\n");
	}

	pSMQ->xMemKey 		= xKey;;
	pSMQ->nShmID		= nShmID;
	pSMQ->pLocker		= pLocker;
	pSMQ->pHead 		= pHead;

	pSMQ->pHead->ulReference++;

	*ppSMQ = pSMQ;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMQ_destroy
(
	FTM_SMQ_PTR	_PTR_ ppSMQ
)
{
	ASSERT(ppSMQ != NULL);

	--(*ppSMQ)->pHead->ulReference;

	if ((*ppSMQ)->pHead->ulReference == 0)
	{
		shmctl((*ppSMQ)->nShmID, IPC_RMID, (struct shmid_ds *)0);
	}
	sem_close((*ppSMQ)->pLocker);
	FTM_MEM_free(*ppSMQ);

	*ppSMQ = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMQ_push
(
	FTM_SMQ_PTR		pSMQ,
	FTM_VOID_PTR	pData,
	FTM_ULONG		ulLen,
	FTM_ULONG		ulTimeout
)
{
	ASSERT(pSMQ != NULL);
	ASSERT(pData != NULL);
	struct timespec	ts;
	FTM_RET	xRet;
	FTM_INT	nRet;

	if (pSMQ->pHead->ulSlotSize < ulLen)
	{
		return	FTM_RET_DATA_IS_TOO_LARGE;	
	}

	ts.tv_sec = ulTimeout / 1000000;
	ts.tv_nsec = ulTimeout % 1000000 * 1000;
	nRet = sem_timedwait(pSMQ->pLocker, &ts);
	if (nRet < 0)
	{
		TRACE("SMQ Push timeout!\n");
		return	FTM_RET_TIMEOUT;	
	}

	if (pSMQ->pHead->ulCount < pSMQ->pHead->ulSlotCount)	
	{
		FTM_ULONG	ulOffset;

		ulOffset = ((pSMQ->pHead->ulFirst + pSMQ->pHead->ulCount) % pSMQ->pHead->ulSlotCount) * (pSMQ->pHead->ulSlotSize + sizeof(FTM_ULONG));

		*((FTM_ULONG_PTR)&pSMQ->pHead->pMem[ulOffset]) = ulLen;
		memcpy(&pSMQ->pHead->pMem[ulOffset + sizeof(FTM_ULONG)], pData, ulLen);
		pSMQ->pHead->ulCount++;

		sem_post(&pSMQ->pHead->xSemaphore);

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_RET_QUEUE_IS_FULL;	
	}

	sem_post(pSMQ->pLocker);

	return	xRet;
}

FTM_RET	FTM_SMQ_pop
(
	FTM_SMQ_PTR		pSMQ,
	FTM_VOID_PTR	pBuff,
	FTM_ULONG		ulBuffLen,
	FTM_ULONG_PTR	pulLen,
	FTM_ULONG		ulTimeout
)
{
	ASSERT(pSMQ != NULL);
	ASSERT(pBuff != NULL);
	ASSERT(pulLen != NULL);

	struct timespec	xCurrentTime;
	struct timespec	xTimeout;
	FTM_RET	xRet;
	FTM_INT	nRet;
	FTM_INT	nDiffTime;

    clock_gettime(CLOCK_REALTIME, &xTimeout);
	
	xTimeout.tv_nsec += (ulTimeout % 1000000) * 1000;
	xTimeout.tv_sec  += (ulTimeout / 1000000 + xTimeout.tv_nsec / 1000000000);
	xTimeout.tv_nsec %= 1000000000;

	nRet =sem_timedwait(&pSMQ->pHead->xSemaphore, &xTimeout);
	if (nRet != 0)
	{
		return	FTM_RET_TIMEOUT;	
	}

    clock_gettime(CLOCK_REALTIME, &xCurrentTime);
	
	nDiffTime = (xTimeout.tv_sec - xCurrentTime.tv_sec) * 1000000000 + (xTimeout.tv_nsec - xCurrentTime.tv_nsec);

	if (nDiffTime < 0)
	{
		sem_post(&pSMQ->pHead->xSemaphore);
		return	FTM_RET_TIMEOUT;	
	}

	xTimeout.tv_sec = nDiffTime / 1000000000;
	xTimeout.tv_nsec = nDiffTime % 1000000000;
	nRet = sem_timedwait(pSMQ->pLocker, &xTimeout);
	if (nRet < 0)
	{
		sem_post(&pSMQ->pHead->xSemaphore);
		return	FTM_RET_TIMEOUT;	
	}

	if (pSMQ->pHead->ulCount > 0)
	{
		FTM_ULONG	ulOffset;

		ulOffset = (pSMQ->pHead->ulFirst  * (pSMQ->pHead->ulSlotSize + sizeof(FTM_ULONG)));
		if (ulBuffLen < *((FTM_ULONG_PTR)&pSMQ->pHead->pMem[ulOffset]))
		{
			*pulLen = 	*((FTM_ULONG_PTR)&pSMQ->pHead->pMem[ulOffset]);
			xRet = FTM_RET_BUFFER_TOO_SMALL;
		}
		else
		{
			memcpy(pBuff, &pSMQ->pHead->pMem[ulOffset + sizeof(FTM_ULONG)], *((FTM_ULONG_PTR)&pSMQ->pHead->pMem[ulOffset]));
			*pulLen = 	*((FTM_ULONG_PTR)&pSMQ->pHead->pMem[ulOffset]);

			pSMQ->pHead->ulFirst = (pSMQ->pHead->ulFirst + 1) % pSMQ->pHead->ulSlotCount; 
			pSMQ->pHead->ulCount--;
			xRet = FTM_RET_OK;
		}
	}
	else
	{
		xRet = FTM_RET_QUEUE_IS_EMPTY;	
	}

	sem_post(pSMQ->pLocker);

	return	xRet;
}

FTM_RET	FTM_SMQ_print
(
	FTM_SMQ_PTR	pSMQ
)
{
	MESSAGE("%16s : %08x\n","Key", 			pSMQ->xMemKey);
	MESSAGE("%16s : %d\n", 	"ID", 			pSMQ->nShmID);
	MESSAGE("%16s : %08x\n","Locker",  		pSMQ->pLocker);
	MESSAGE("%16s : %d\n", 	"Reference",  	pSMQ->pHead->ulReference);
	MESSAGE("%16s : %d\n", 	"Slot Size", 	pSMQ->pHead->ulSlotSize);
	MESSAGE("%16s : %d\n", 	"Slot Count", 	pSMQ->pHead->ulSlotCount);
	MESSAGE("%16s : %d\n", 	"Head Index", 	pSMQ->pHead->ulFirst);
	MESSAGE("%16s : %d\n", 	"Packet Count",	pSMQ->pHead->ulCount);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMP_createServer
(
	key_t		xKey,
	FTM_SMP_PTR	_PTR_	ppSMP
)
{
	ASSERT(ppSMP != NULL);
	
	FTM_RET			xRet;
	FTM_SMP_PTR		pSMP;

	pSMP = (FTM_SMP_PTR)FTM_MEM_malloc(sizeof(FTM_SMP));
	if (pSMP == NULL)
	{
		ERROR("Not enouch memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pSMP->xKey = xKey;
	pSMP->nShmID = shmget(pSMP->xKey, sizeof(FTM_SMP_BLOCK), IPC_CREAT | 0666);
	if (pSMP->nShmID < 0)
	{
		ERROR("shared memory creation failed.\n");	
		xRet = FTM_RET_CANT_CREATE_SM;
		goto error;
	}

	TRACE("SM[%d:%d] creation success!\n", pSMP->xKey, pSMP->nShmID);

	pSMP->pBlock = (FTM_SMP_BLOCK_PTR)shmat(pSMP->nShmID, NULL, 0);
	if ((FTM_INT)pSMP->pBlock == -1)
	{
		ERROR("Shared memory don't attached the segment to our space.\n");
		shmctl(pSMP->nShmID, IPC_RMID, (struct shmid_ds *)0);
		xRet = FTM_RET_CANT_CREATE_SM;
		goto error;
	}

	memset(pSMP->pBlock, 0, sizeof(FTM_SMP_BLOCK));

	FTM_INT	nSemCount = 0;

	sprintf(pSMP->pBlock->pLockerName, "ftom.lock.%d", pSMP->xKey);
	pSMP->pLocker = sem_open(pSMP->pBlock->pLockerName, O_CREAT, 0666, 1);
	if (pSMP->pLocker == SEM_FAILED)
	{
		shmctl(pSMP->nShmID, IPC_RMID, (struct shmid_ds *)0);
		ERROR("Locker creation failed.\n");
		xRet = FTM_RET_CANT_CREATE_SEMAPHORE;
		goto error;
	}
	else
	{
		sem_getvalue(pSMP->pLocker, &nSemCount);
		sem_post(pSMP->pLocker);
	}

	TRACE("Locker[%s] created.\n", pSMP->pBlock->pLockerName);
	pSMP->pBlock->ulReference = 1;
	sem_init(&pSMP->pBlock->xNotUse, 1, 1);
	sem_init(&pSMP->pBlock->xReq, 2, 0);
	sem_init(&pSMP->pBlock->xResp, 3, 0);

	TRACE("SMP creation done.\n");
	*ppSMP = pSMP;

	return	FTM_RET_OK;

error:
	if (pSMP != NULL)
	{
		FTM_MEM_free(pSMP);	
	}

	return	xRet;
}

FTM_RET	FTM_SMP_createClient
(
	key_t		xKey,
	FTM_INT		nShmID,
	FTM_SMP_PTR	_PTR_	ppSMP
)
{
	ASSERT(ppSMP != NULL);

	FTM_RET			xRet;
	FTM_SMP_PTR		pSMP;

	pSMP = (FTM_SMP_PTR)FTM_MEM_malloc(sizeof(FTM_SMP));
	if (pSMP == NULL)
	{
		ERROR("Not enouch memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pSMP->xKey = xKey;
	if (nShmID > 0)
	{
		pSMP->nShmID = nShmID;
	}
	else
	{
		pSMP->nShmID = shmget(pSMP->xKey, sizeof(FTM_SMP_BLOCK), 0666);
		if (pSMP->nShmID < 0)
		{
			ERROR("shared memory creation failed.\n");	
			xRet = FTM_RET_SM_IS_NOT_EXIST;
			goto error;
		}
	}

	TRACE("KEY : %d, SHMID : %d\n", pSMP->xKey, pSMP->nShmID);
	pSMP->pBlock = (FTM_SMP_BLOCK_PTR)shmat(pSMP->nShmID, NULL, 0);
	if ((FTM_INT)pSMP->pBlock == -1)
	{
		ERROR("Shared memory don't attached the segment to our space.\n");
		xRet = FTM_RET_SM_IS_NOT_EXIST;
		goto error;
	}

	pSMP->pLocker = sem_open(pSMP->pBlock->pLockerName, 0, 0666, 0);
	if (pSMP->pLocker == SEM_FAILED)
	{
		ERROR("Locker[%s] open failed[%s].!\n", pSMP->pBlock->pLockerName, strerror(errno));
		xRet = FTM_RET_SM_IS_NOT_EXIST;
		goto error;
	}

	pSMP->pBlock->ulReference++;

	*ppSMP = pSMP;

	return	FTM_RET_OK;

error:
	if (pSMP != NULL)
	{
		FTM_MEM_free(pSMP);	
	}

	return	xRet;
}


FTM_RET	FTM_SMP_destroy
(
	FTM_SMP_PTR	_PTR_ ppSMP
)
{
	ASSERT(ppSMP != NULL);

	sem_wait((*ppSMP)->pLocker);
	--(*ppSMP)->pBlock->ulReference;
	sem_post((*ppSMP)->pLocker);

	if ((*ppSMP)->pBlock->ulReference == 0)
	{
		sem_destroy(&(*ppSMP)->pBlock->xReq);
		sem_destroy(&(*ppSMP)->pBlock->xResp);

		sem_close((*ppSMP)->pLocker);
		(*ppSMP)->pLocker = NULL;

		shmctl((*ppSMP)->nShmID, IPC_RMID, (struct shmid_ds *)0);
	}

	FTM_MEM_free(*ppSMP);

	*ppSMP = NULL;

	return	FTM_RET_OK;
}


FTM_RET	FTM_SMP_call
(
	FTM_SMP_PTR		pSMP,
	FTM_VOID_PTR	pReqData,
	FTM_ULONG		ulReqLen,
	FTM_VOID_PTR	pRespBuff,
	FTM_ULONG		ulRespBuffLen,
	FTM_ULONG_PTR	pulRespLen,
	FTM_ULONG		ulTimeout
)
{
	ASSERT(pSMP != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;
	struct timespec	xTimeout;

	
	clock_gettime(CLOCK_REALTIME, &xTimeout);
	xTimeout.tv_nsec = xTimeout.tv_nsec + ulTimeout*1000;
	xTimeout.tv_sec = xTimeout.tv_sec + xTimeout.tv_nsec / 1000000000;
	xTimeout.tv_nsec = xTimeout.tv_nsec % 1000000000;
	
	nRet = sem_timedwait(&pSMP->pBlock->xNotUse, &xTimeout);
	if (nRet != 0)
	{
		ERROR("Shared memory is used.\n");
		xRet = FTM_RET_TIMEOUT;	
		goto finish;
	}

	nRet = sem_timedwait(pSMP->pLocker, &xTimeout);
	if (nRet != 0)
	{
		ERROR("Request send failed. Shared memory is locked.\n");
		xRet = FTM_RET_TIMEOUT;	
		goto error;
	}

	memcpy(pSMP->pBlock->pBuff, pReqData, ulReqLen);
	pSMP->pBlock->ulReqLen = ulReqLen;
	sem_post(&pSMP->pBlock->xReq);

	sem_post(pSMP->pLocker);

	nRet = sem_timedwait(&pSMP->pBlock->xResp, &xTimeout);
	if (nRet != 0)
	{
		ERROR("Respose timeout.\n");
		xRet = FTM_RET_TIMEOUT;	
		goto error;
	}

	nRet = sem_timedwait(pSMP->pLocker, &xTimeout);
	if (nRet != 0)
	{
		ERROR("Response get failed. Shared memory is locked.\n");
		xRet = FTM_RET_TIMEOUT;	
		goto error;
	}

	memcpy(pRespBuff, pSMP->pBlock->pBuff, pSMP->pBlock->ulRespLen);
	*pulRespLen = pSMP->pBlock->ulRespLen;

	sem_post(pSMP->pLocker);

error:
	sem_post(&pSMP->pBlock->xNotUse);

finish:

	return	xRet;
}

FTM_RET	FTM_SMP_receiveReq
(
	FTM_SMP_PTR		pSMP,
	FTM_VOID_PTR	pReqBuff,
	FTM_ULONG		ulBuffLen,
	FTM_ULONG_PTR	pulReqLen,
	FTM_ULONG		ulTimeout
)
{
	ASSERT(pSMP != NULL);
	
	struct timespec	xTimeout;
	FTM_INT	nRet;

	clock_gettime(CLOCK_REALTIME, &xTimeout);
	xTimeout.tv_nsec = xTimeout.tv_nsec + ulTimeout*1000;
	xTimeout.tv_sec = xTimeout.tv_sec + xTimeout.tv_nsec / 1000000000;
	xTimeout.tv_nsec = xTimeout.tv_nsec % 1000000000;

	nRet = sem_timedwait(&pSMP->pBlock->xReq, &xTimeout);
	if (nRet != 0)
	{
		return	FTM_RET_TIMEOUT;	
	}

	nRet = sem_timedwait(pSMP->pLocker, &xTimeout);
	if (nRet != 0)
	{
		return	FTM_RET_TIMEOUT;	
	}

	memcpy(pReqBuff, pSMP->pBlock->pBuff, pSMP->pBlock->ulReqLen);
	*pulReqLen = pSMP->pBlock->ulReqLen;

	sem_post(pSMP->pLocker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMP_sendResp
(
	FTM_SMP_PTR		pSMP,
	FTM_VOID_PTR	pRespData,
	FTM_ULONG		ulDataLen,
	FTM_ULONG		ulTimeout
)
{
	ASSERT(pSMP != NULL);
	
	struct timespec	xTimeout;
	FTM_INT	nRet;

	if (ulDataLen > sizeof(pSMP->pBlock->pBuff))
	{
		ERROR("ulDataLen : %d\n", ulDataLen);
		return	FTM_RET_INTERNAL_ERROR;	
	}

	clock_gettime(CLOCK_REALTIME, &xTimeout);
	xTimeout.tv_nsec = xTimeout.tv_nsec + ulTimeout*1000;
	xTimeout.tv_sec = xTimeout.tv_sec + xTimeout.tv_nsec / 1000000000;
	xTimeout.tv_nsec = xTimeout.tv_nsec % 1000000000;

	nRet = sem_timedwait(pSMP->pLocker, &xTimeout);
	if (nRet != 0)
	{
		return	FTM_RET_TIMEOUT;	
	}

	memcpy(pSMP->pBlock->pBuff, pRespData, ulDataLen);
	pSMP->pBlock->ulRespLen = ulDataLen;
	sem_post(&pSMP->pBlock->xResp);

	sem_post(pSMP->pLocker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMP_createKeyFile
(
	FTM_SMP_PTR	pSMP,
	FTM_CHAR_PTR	pFileName
)
{
	FILE *pFile;

	pFile = fopen(pFileName, "w");
	if (pFile == NULL)
	{
		ERROR("Can't creation key file[%s]!\n", pFileName);
		return	FTM_RET_ERROR;	
	}

	fprintf(pFile, "%d %d\n", pSMP->xKey, pSMP->nShmID);
	fflush(pFile);
	fclose(pFile);

	return	FTM_RET_OK;
}


FTM_RET	FTM_SMP_print
(
	FTM_SMP_PTR	pSMP
)
{
	FTM_INT	nValue;

	MESSAGE("%16s : %08x\n",	"Key", 	pSMP->xKey);
	MESSAGE("%16s : %08x\n",	"ID", 	pSMP->nShmID);
	MESSAGE("%16s : %08x\n",	"Locker", pSMP->pLocker);

	MESSAGE("%16s : %s\n", 		"Name", pSMP->pBlock->pLockerName);
	sem_getvalue(&pSMP->pBlock->xReq, &nValue);
	MESSAGE("%16s : %d\n",		"Req",	nValue); 
	sem_getvalue(&pSMP->pBlock->xResp, &nValue);
	MESSAGE("%16s : %d\n",		"Resp",	nValue); 
	MESSAGE("%16s : %d\n", 		"Reference", pSMP->pBlock->ulReference);
	MESSAGE("%16s : %d\n", 		"Req Length", pSMP->pBlock->ulReqLen);
	MESSAGE("%16s : %d\n", 		"Resp Length",pSMP->pBlock->ulRespLen);

	return	FTM_RET_OK;
}
