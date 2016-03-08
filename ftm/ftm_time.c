#include "ftm_time.h"
#include "ftm_trace.h"

FTM_RET	FTM_TIME_getCurrent(FTM_TIME_PTR pTime)
{
	ASSERT(pTime != NULL);
	
	gettimeofday(&pTime->xTimeval, NULL);

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_TIME_isZero(FTM_TIME_PTR pTime)
{
	return	((pTime == NULL) || ((pTime->xTimeval.tv_sec == 0) && (pTime->xTimeval.tv_usec == 0)));
}

FTM_RET	FTM_TIME_set(FTM_TIME_PTR pTime, FTM_ULONG ulMilliSeconds)
{
	ASSERT(pTime != NULL);
	
	pTime->xTimeval.tv_sec = ulMilliSeconds / 1000;
	pTime->xTimeval.tv_usec = ulMilliSeconds % 1000 * 1000;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_add(FTM_TIME_PTR pTime1, FTM_TIME_PTR pTime2, FTM_TIME_PTR pTimeR)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTime2 != NULL);
	ASSERT(pTimeR != NULL);
	
	pTimeR->xTimeval.tv_usec = pTime1->xTimeval.tv_usec + pTime2->xTimeval.tv_usec;
	pTimeR->xTimeval.tv_sec = pTime1->xTimeval.tv_sec + pTime2->xTimeval.tv_sec + pTimeR->xTimeval.tv_usec / 1000000;
	pTimeR->xTimeval.tv_usec = pTimeR->xTimeval.tv_usec % 1000000;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_sub(FTM_TIME_PTR pTime1, FTM_TIME_PTR pTime2, FTM_TIME_PTR pTimeR)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTime2 != NULL);
	ASSERT(pTimeR != NULL);

	FTM_INT64 	nValue1, nValue2, nValueR;

	nValue1 = (FTM_INT64)pTime1->xTimeval.tv_sec * 1000000 + pTime1->xTimeval.tv_usec;
	nValue2 = (FTM_INT64)pTime2->xTimeval.tv_sec * 1000000 + pTime2->xTimeval.tv_usec;

	nValueR = nValue1 - nValue2;

	pTimeR->xTimeval.tv_sec = nValueR / 1000000;
	pTimeR->xTimeval.tv_usec = nValueR % 1000000;

	return	FTM_RET_OK;
}

