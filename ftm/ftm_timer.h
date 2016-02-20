#ifndef	__FTM_TIMER_H__
#define	__FTM_TIMER_H__

#include "ftm_types.h"
#include <sys/time.h>

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

FTM_RET		FTM_TIMER_init(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeout);
FTM_RET		FTM_TIMER_add(FTM_TIMER_PTR pTimer, FTM_ULONG ulTimeout);
FTM_BOOL 	FTM_TIMER_isExpired(FTM_TIMER_PTR pTimer);
FTM_RET 	FTM_TIMER_waitForExpired(FTM_TIMER_PTR pTimer);

#endif

