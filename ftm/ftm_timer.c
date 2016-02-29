#include <unistd.h>
#include "ftm_timer.h"
#include "ftm_trace.h"

FTM_RET	FTM_TIMER_init(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeout)
{
	ASSERT(pTimer != NULL);
	struct timeval	xCurrentTime;
	struct timeval	xTimeout;

	xTimeout.tv_sec = ulTimeout / 1000000;
	xTimeout.tv_usec = ulTimeout % 1000000;

	gettimeofday(&xCurrentTime, NULL);
	timeradd(&xCurrentTime, &xTimeout, &pTimer->xTime);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIMER_add(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeout)
{
	ASSERT(pTimer != NULL);
	struct timeval	xTimeout;

	xTimeout.tv_sec = ulTimeout / 1000000;
	xTimeout.tv_usec = ulTimeout % 1000000;

	timeradd(&pTimer->xTime, &xTimeout, &pTimer->xTime);

	return	FTM_RET_OK;
}

FTM_BOOL FTM_TIMER_isExpired(FTM_TIMER_PTR pTimer)
{
	ASSERT(pTimer != NULL);
	struct timeval xCurrentTime;

	gettimeofday(&xCurrentTime, NULL);

	return	timercmp(&pTimer->xTime, &xCurrentTime, <);
}

FTM_RET FTM_TIMER_waitForExpired(FTM_TIMER_PTR pTimer)
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

FTM_RET FTM_TIMER_remain(FTM_TIMER_PTR pTimer, FTM_ULONG_PTR pulTime)
{
	ASSERT(pTimer != NULL);
	struct timeval xCurrentTime;
	struct timeval xDiffTime;

	gettimeofday(&xCurrentTime, NULL);
	if (timercmp(&pTimer->xTime, &xCurrentTime, >))
	{
		timersub(&pTimer->xTime, &xCurrentTime, &xDiffTime);
		*pulTime = xDiffTime.tv_sec * 1000000 + xDiffTime.tv_usec;
	}
	else
	{
		*pulTime = 0;	
	}

	return	FTM_RET_OK;
}
