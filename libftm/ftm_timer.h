#ifndef	__FTM_TIMER_H__
#define	__FTM_TIMER_H__

#include "ftm_types.h"
#include <sys/time.h>
#include "ftm_time.h"

typedef	enum
{
	FTM_TIMER_TYPE_RELATIVE=0,
	FTM_TIMER_TYPE_ABSOLUTE=1
}	FTM_TIMER_TYPE, _PTR_ FTM_TIMER_TYPE_PTR;

typedef	struct
{
	FTM_TIMER_TYPE		xType;
	struct timeval		xTime;
}	FTM_TIMER, _PTR_ FTM_TIMER_PTR;

FTM_RET		FTM_TIMER_initS(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeoutS);
FTM_RET		FTM_TIMER_initMS(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeoutMS);
FTM_RET		FTM_TIMER_initUS(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeoutUS);
FTM_RET		FTM_TIMER_initTime(FTM_TIMER_PTR pTimer, FTM_TIME_PTR pTimeout);
FTM_RET		FTM_TIMER_addS(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeS);
FTM_RET		FTM_TIMER_addMS(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeMS);
FTM_RET		FTM_TIMER_addUS(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeUS);
FTM_RET		FTM_TIMER_addTime(FTM_TIMER_PTR pTimer, FTM_TIME_PTR pTimeout);
FTM_BOOL 	FTM_TIMER_isExpired(FTM_TIMER_PTR pTimer);
FTM_RET 	FTM_TIMER_waitForExpired(FTM_TIMER_PTR pTimer);
FTM_RET 	FTM_TIMER_remainS(FTM_TIMER_PTR pTimer, FTM_ULONG_PTR pulTimeS);
FTM_RET 	FTM_TIMER_remainMS(FTM_TIMER_PTR pTimer, FTM_ULONG_PTR pulTimeMS);
FTM_RET 	FTM_TIMER_remainUS(FTM_TIMER_PTR pTimer, FTM_UINT64_PTR pulTimeUS);
FTM_RET		FTM_TIMER_getTime(FTM_TIMER_PTR pTimer, FTM_ULONG_PTR pulTime);
#endif

