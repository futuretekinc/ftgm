#include <unistd.h>
#include <string.h>
#include <time.h>
#include "ftm_timer.h"
#include "ftm_trace.h"

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
