#ifndef	_FTM_SHARED_MEMORY_H_
#define	_FTM_SHARED_MEMORY_H_

#include <sys/types.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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

typedef	struct	FTM_SMQ_HEADER
{
	FTM_ULONG	ulProcessID;
	sem_t		xSemaphore;
	FTM_ULONG	ulReference;
	FTM_ULONG	ulSlotSize;
	FTM_ULONG	ulSlotCount;
	FTM_ULONG	ulFirst;
	FTM_ULONG	ulCount;
	FTM_CHAR	pMem[];
}	FTM_SMQ_HEAD, _PTR_ FTM_SMQ_HEAD_PTR;

typedef	struct FTM_SMQ_STRUCT
{
	key_t		xMemKey;
	FTM_INT		nShmID;
	sem_t		*pLocker;
	
	FTM_SMQ_HEAD_PTR	pHead;
}	FTM_SMQ, _PTR_ FTM_SMQ_PTR;

FTM_RET	FTM_SMQ_create
(
	key_t		xKey,
	FTM_ULONG	ulSize,
	FTM_ULONG	ulCount,
	FTM_SMQ_PTR	_PTR_ ppSMQ
);

FTM_RET	FTM_SMQ_destroy
(
	FTM_SMQ_PTR	_PTR_ ppSMQ
);

FTM_RET	FTM_SMQ_push
(
	FTM_SMQ_PTR		pSMQ,
	FTM_VOID_PTR	pData,
	FTM_ULONG		ulLen,
	FTM_ULONG		ulTimeout
);

FTM_RET	FTM_SMQ_pop
(
	FTM_SMQ_PTR		pSMQ,
	FTM_VOID_PTR	pBuff,
	FTM_ULONG		ulBuffLen,
	FTM_ULONG_PTR	pulLen,
	FTM_ULONG		ulTimeout
);

FTM_RET	FTM_SMQ_print
(
	FTM_SMQ_PTR	pSMQ
);

typedef	struct	FTM_SMP_BLOCK_STRUCT
{
	FTM_CHAR	pLockerName[64];
	sem_t		xNotUse;
	sem_t		xReq;
	sem_t		xResp;
	FTM_ULONG	ulReference;
	FTM_ULONG	ulReqLen;
	FTM_ULONG	ulRespLen;
	FTM_CHAR	pBuff[4096 - sizeof(sem_t) * 3 - sizeof(FTM_ULONG) * 3 - 64];
}	FTM_SMP_BLOCK, _PTR_ FTM_SMP_BLOCK_PTR;

typedef	struct	FTM_SMP_STRUCT
{
	key_t		xKey;
	FTM_INT		nShmID;
	sem_t		*pLocker;

	FTM_SMP_BLOCK_PTR	pBlock;
}	FTM_SMP, _PTR_ FTM_SMP_PTR;


FTM_RET	FTM_SMP_createServer
(
	key_t		xKey,
	FTM_SMP_PTR	_PTR_	ppSMP
);


FTM_RET	FTM_SMP_createClient
(
	key_t		xKey,
	FTM_INT		nShmID,
	FTM_SMP_PTR	_PTR_	ppSMP
);

FTM_RET	FTM_SMP_destroy
(
	FTM_SMP_PTR	_PTR_ ppSMP
);

FTM_RET	FTM_SMP_call
(
	FTM_SMP_PTR		pSMP,
	FTM_VOID_PTR	pReqData,
	FTM_ULONG		ulReqLen,
	FTM_VOID_PTR	pRespBuff,
	FTM_ULONG		ulRespBuffLen,
	FTM_ULONG_PTR	pulRespLen,
	FTM_ULONG		ulTimeout
);

FTM_RET	FTM_SMP_receiveReq
(
	FTM_SMP_PTR		pSMP,
	FTM_VOID_PTR	pReqBuff,
	FTM_ULONG		ulBuffLen,
	FTM_ULONG_PTR	pulRespLen,
	FTM_ULONG		ulTimeout
);

FTM_RET	FTM_SMP_sendResp
(
	FTM_SMP_PTR		pSMP,
	FTM_VOID_PTR	pRespData,
	FTM_ULONG		ulDataLen,
	FTM_ULONG		ulTimeout
);

FTM_RET	FTM_SMP_createKeyFile
(
	FTM_SMP_PTR		pSMP,
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTM_SMP_print
(
	FTM_SMP_PTR	pSMP
);

#endif
