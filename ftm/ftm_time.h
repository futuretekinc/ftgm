#ifndef	_FTM_TIME_H_
#define	_FTM_TIME_H_

#include <sys/time.h>
#include "ftm_types.h"

typedef	struct
{
	struct timeval	xTimeVal;
}	FTM_DATE, _PTR_ FTM_DATE_PTR;

FTM_RET	FTM_DATE_getCurrent(FTM_DATE_PTR pDate);

typedef	struct
{
	struct timeval	xTimeval;
}	FTM_TIME, _PTR_ FTM_TIME_PTR;

FTM_BOOL	FTM_TIME_isZero(FTM_TIME_PTR pTime);
FTM_RET		FTM_TIME_set(FTM_TIME_PTR pTime, FTM_ULONG ulMilliSeconds);
FTM_RET		FTM_TIME_add(FTM_TIME_PTR pTime1, FTM_TIME_PTR pTime2, FTM_TIME_PTR pTimeR);
FTM_RET		FTM_TIME_sub(FTM_TIME_PTR pTime1, FTM_TIME_PTR pTime2, FTM_TIME_PTR pTimeR);

#endif
