#include <unistd.h>
#include "ftnm_timer.h"
#include "ftm_error.h"
#include "ftm_debug.h"

FTM_RET	FTNM_TIMER_init(FTNM_TIMER_PTR pTimer, FTM_ULONG ulTimeout)
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

FTM_RET	FTNM_TIMER_add(FTNM_TIMER_PTR pTimer, FTM_ULONG ulTimeout)
{
	ASSERT(pTimer != NULL);
	struct timeval	xTimeout;

	xTimeout.tv_sec = ulTimeout / 1000000;
	xTimeout.tv_usec = ulTimeout % 1000000;

	timeradd(&pTimer->xTime, &xTimeout, &pTimer->xTime);

	return	FTM_RET_OK;
}

FTM_BOOL FTNM_TIMER_isExpired(FTNM_TIMER_PTR pTimer)
{
	ASSERT(pTimer != NULL);
	struct timeval xCurrentTime;

	gettimeofday(&xCurrentTime, NULL);

	return	timercmp(&pTimer->xTime, &xCurrentTime, <);
}

FTM_RET FTNM_TIMER_waitForExpired(FTNM_TIMER_PTR pTimer)
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
