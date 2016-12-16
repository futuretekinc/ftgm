#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "ftm_timer.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_mem.h"

static
FTM_VOID_PTR	FTM_EVENT_TIMER_MANAGER_threadMain
(
	FTM_VOID_PTR	pData
);

static
FTM_INT		FTM_EVENT_TIMER_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

FTM_RET	FTM_TIMER_initS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG 		ulTimeoutS
)
{
	return	FTM_TIMER_initUS(pTimer, ulTimeoutS * 1000000);
}

FTM_RET	FTM_TIMER_initMS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG 		ulTimeoutMS
)
{
	return	FTM_TIMER_initUS(pTimer, ulTimeoutMS * 1000);
}

FTM_RET	FTM_TIMER_initUS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG 		ulTimeoutUS
)
{
	ASSERT(pTimer != NULL);
	struct timeval	xCurrentTime;
	struct timeval	xTimeout;

	xTimeout.tv_sec = ulTimeoutUS / 1000000;
	xTimeout.tv_usec = ulTimeoutUS % 1000000;

	gettimeofday(&xCurrentTime, NULL);
	timeradd(&xCurrentTime, &xTimeout, &pTimer->xTime);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIMER_initTime
(	
	FTM_TIMER_PTR 	pTimer, 
	FTM_TIME_PTR 	pTimeout
)
{
	FTM_RET	xRet;

	xRet = FTM_TIMER_initUS(pTimer, 0);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTM_TIMER_addTime(pTimer, pTimeout);
}

FTM_RET	FTM_TIMER_addUS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG 		ulTimeUS
)
{
	ASSERT(pTimer != NULL);
	struct timeval	xTimeout;

	xTimeout.tv_sec = ulTimeUS / 1000000;
	xTimeout.tv_usec = ulTimeUS % 1000000;

	timeradd(&pTimer->xTime, &xTimeout, &pTimer->xTime);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIMER_addMS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG 		ulTimeMS
)
{
	return	FTM_TIMER_addUS(pTimer, ulTimeMS * 1000);
}

FTM_RET	FTM_TIMER_addS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG 		ulTimeS
)
{
	return	FTM_TIMER_addUS(pTimer, ulTimeS * 1000000);
}

FTM_RET		FTM_TIMER_addTime
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_TIME_PTR 	pTimeout
)
{
	ASSERT(pTimer != NULL);
	ASSERT(pTimeout != NULL);

	timeradd(&pTimer->xTime, &pTimeout->xTimeval, &pTimer->xTime);

	return	FTM_RET_OK;
}

FTM_BOOL FTM_TIMER_isExpired
(
	FTM_TIMER_PTR pTimer
)
{
	ASSERT(pTimer != NULL);
	struct timeval xCurrentTime;

	gettimeofday(&xCurrentTime, NULL);

	return	timercmp(&pTimer->xTime, &xCurrentTime, <);
}

FTM_RET FTM_TIMER_waitForExpired
(
	FTM_TIMER_PTR pTimer
)
{
	ASSERT(pTimer != NULL);
	struct timeval xCurrentTime;

	gettimeofday(&xCurrentTime, NULL);

	if (timercmp(&pTimer->xTime, &xCurrentTime, >))
	{
		struct timeval xDiffTime;
		FTM_ULONG	ulSleepTime;

		timersub(&pTimer->xTime, &xCurrentTime, &xDiffTime);
		
		ulSleepTime = xDiffTime.tv_sec * 1000000 + xDiffTime.tv_usec;

		usleep(ulSleepTime);
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_TIMER_remainUS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_UINT64_PTR 	pullTimeUS
)
{
	ASSERT(pTimer != NULL);
	struct timeval xCurrentTime;
	struct timeval xDiffTime;

	gettimeofday(&xCurrentTime, NULL);
	if (timercmp(&pTimer->xTime, &xCurrentTime, >))
	{
		timersub(&pTimer->xTime, &xCurrentTime, &xDiffTime);
		*pullTimeUS = xDiffTime.tv_sec * (FTM_UINT64)1000000 + xDiffTime.tv_usec;
	}
	else
	{
		*pullTimeUS = 0;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_TIMER_remainMS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG_PTR 	pulTimeMS
)
{
	FTM_RET		xRet;
	FTM_UINT64	ulTimeUS;

	xRet = FTM_TIMER_remainUS(pTimer, &ulTimeUS);
	if (xRet == FTM_RET_OK)
	{
		*pulTimeMS = ulTimeUS / 1000;	
	}

	return	xRet;
}

FTM_RET FTM_TIMER_remainS
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_ULONG_PTR 	pulTimeS
)
{
	FTM_RET		xRet;
	FTM_UINT64	ulTimeUS;

	xRet = FTM_TIMER_remainUS(pTimer, &ulTimeUS);
	if (xRet == FTM_RET_OK)
	{
		*pulTimeS = ulTimeUS / 1000000;	
	}

	return	xRet;
}

FTM_CHAR_PTR	FTM_TIMER_toString
(
	FTM_TIMER_PTR	pTimer,
	FTM_CHAR_PTR	pFormat
)
{
	ASSERT(pTimer != NULL);

	static	FTM_CHAR	pString[128];
	struct	tm*			pTM;

	pTM = localtime(&pTimer->xTime.tv_sec);
	if (pFormat != NULL)
	{
		strftime(pString, sizeof(pString), pFormat, pTM);
	}
	else
	{
		strftime(pString, sizeof(pString), "%Y-%m-%d %H:%M:%S", pTM);
	}

	return	pString;
}

FTM_RET		FTM_TIMER_getTime
(
	FTM_TIMER_PTR pTimer, 
	FTM_ULONG_PTR pulTime
)
{
	ASSERT(pTimer != NULL);
	ASSERT(pulTime != NULL);
	
	*pulTime = pTimer->xTime.tv_sec;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_TIMER_MANAGER_create
(
	FTM_EVENT_TIMER_MANAGER_PTR _PTR_ ppETM
)
{
	ASSERT(ppETM != NULL);
	FTM_RET	xRet;
	FTM_EVENT_TIMER_MANAGER_PTR	pETM;

	pETM = (FTM_EVENT_TIMER_MANAGER_PTR)FTM_MEM_malloc(sizeof(FTM_EVENT_TIMER_MANAGER));
	if (pETM == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Failed to create event timer manager!\n");
		return	xRet;
	}

	pETM->ulLoopInterval = 10;
	xRet = FTM_LIST_create(&pETM->pEventList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to creat event list!\n");
		FTM_MEM_free(pETM);
		return	xRet;
	}
	FTM_LIST_setComparator(pETM->pEventList, FTM_EVENT_TIMER_comparator);

	FTM_LOCK_init(&pETM->xLock);
	pETM->bStop = FTM_TRUE;
	
	*ppETM = pETM;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_TIMER_MANAGER_destroy
(
	FTM_EVENT_TIMER_MANAGER_PTR _PTR_ ppETM
)
{	
	ASSERT(ppETM != NULL);
	FTM_EVENT_TIMER_PTR	pEvent;

	FTM_EVENT_TIMER_MANAGER_stop((*ppETM));

	FTM_LOCK_set(&(*ppETM)->xLock);
	
	while(FTM_LIST_getFirst((*ppETM)->pEventList, (FTM_VOID_PTR _PTR_)&pEvent) == FTM_RET_OK)
	{
		FTM_LIST_remove((*ppETM)->pEventList, pEvent);	

		FTM_MEM_free(pEvent);
	}

	FTM_LIST_destroy((*ppETM)->pEventList);

	FTM_LOCK_reset(&(*ppETM)->xLock);

	FTM_LOCK_final(&(*ppETM)->xLock);

	FTM_MEM_free(*ppETM);
	*ppETM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_TIMER_MANAGER_start
(
	FTM_EVENT_TIMER_MANAGER_PTR	pETM
)
{
	ASSERT(pETM != NULL);
	FTM_RET	xRet;

	pETM->bStop = FTM_FALSE;
	
	if (pthread_create(&pETM->xThreadMain, NULL, FTM_EVENT_TIMER_MANAGER_threadMain, pETM) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The blocker main thread creation failed!\n");
		return  xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_TIMER_MANAGER_stop
(
	FTM_EVENT_TIMER_MANAGER_PTR	pETM
)
{
	ASSERT(pETM != NULL);

	pETM->bStop = FTM_TRUE;
	
	pthread_join(pETM->xThreadMain, NULL);
	pETM->xThreadMain = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_TIMER_MANAGER_add
(
	FTM_EVENT_TIMER_MANAGER_PTR	pETM,
	FTM_EVENT_TIMER_TYPE		xType,
	FTM_ULONG					ulExpiredTime,
	FTM_EVENT_TIMER_CB			fCallback,
	FTM_VOID_PTR				pData,
	FTM_EVENT_TIMER_PTR _PTR_	ppEvent
)
{
	ASSERT(pETM != NULL);

	FTM_EVENT_TIMER_PTR	pEvent;

	pEvent = (FTM_EVENT_TIMER_PTR)FTM_MEM_malloc(sizeof(FTM_EVENT_TIMER));
	if (pEvent == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pEvent->xType 		= xType;
	pEvent->ulTimeMS	= (ulExpiredTime > pETM->ulLoopInterval)?ulExpiredTime:pETM->ulLoopInterval;
	pEvent->fCallback	= fCallback;
	pEvent->pData		= pData;
	FTM_TIMER_initMS(&pEvent->xTimer, ulExpiredTime);

	
	FTM_LOCK_set(&pETM->xLock);

	FTM_LIST_insert(pETM->pEventList, pEvent, FTM_LIST_POS_ASSENDING);

	FTM_LOCK_reset(&pETM->xLock);

	if (ppEvent != NULL)
	{
		*ppEvent = pEvent;
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_EVENT_TIMER_MANAGER_threadMain
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_EVENT_TIMER_MANAGER_PTR	pETM = (FTM_EVENT_TIMER_MANAGER_PTR)pData;
	FTM_TIMER	xLoopTimer;

	FTM_TIMER_initMS(&xLoopTimer, 10);

	while(!pETM->bStop)
	{
		FTM_ULONG	ulRemainTime = 0;
		FTM_EVENT_TIMER_PTR	pEvent;

		FTM_LOCK_set(&pETM->xLock);
		while(FTM_TRUE)
		{
			FTM_RET	xRet;

			xRet = FTM_LIST_getFirst(pETM->pEventList, (FTM_VOID_PTR _PTR_)&pEvent);
			if ((xRet != FTM_RET_OK) || (FTM_TIMER_isExpired(&pEvent->xTimer) == FTM_FALSE))
			{
				break;
			}

			FTM_LIST_remove(pETM->pEventList, pEvent);

			xRet = pEvent->fCallback(&pEvent->xTimer, pEvent->pData);

			if (pEvent->xType == FTM_EVENT_TIMER_TYPE_REPEAT)
			{
				FTM_TIMER_addMS(&pEvent->xTimer, pEvent->ulTimeMS);

				FTM_LIST_insert(pETM->pEventList, pEvent, FTM_LIST_POS_ASSENDING);
			}
			else
			{
				FTM_MEM_free(pEvent);
			}
		}
		FTM_LOCK_reset(&pETM->xLock);

		FTM_TIMER_remainMS(&xLoopTimer, &ulRemainTime);
		usleep(ulRemainTime * 1000);
		FTM_TIMER_addMS(&xLoopTimer, pETM->ulLoopInterval);
	}

	return	0;
}


FTM_INT		FTM_EVENT_TIMER_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	FTM_EVENT_TIMER_PTR	pEvent1 = (FTM_EVENT_TIMER_PTR)pElement1;
	FTM_EVENT_TIMER_PTR	pEvent2 = (FTM_EVENT_TIMER_PTR)pElement2;
	FTM_ULONG			ulRemainTime1;
	FTM_ULONG			ulRemainTime2;

	FTM_TIMER_remainMS(&pEvent1->xTimer, &ulRemainTime1);
	FTM_TIMER_remainMS(&pEvent2->xTimer, &ulRemainTime2);

	return	(ulRemainTime1 > ulRemainTime2)?1:(ulRemainTime1 == ulRemainTime2)?0:-1;
}
