#ifndef	__FTNM_TIMER_H__
#define	__FTNM_TIMER_H__

#include "ftm_types.h"
#include <sys/time.h>

typedef	enum
{
	FTNM_TIMER_TYPE_RELATIVE=0,
	FTNM_TIMER_TYPE_ABSOLUTE=1
}	FTNM_TIMER_TYPE, _PTR_ FTNM_TIMER_TYPE_PTR;

typedef	struct
{
	FTNM_TIMER_TYPE		xType;
	struct timeval		xTime;
}	FTNM_TIMER, _PTR_ FTNM_TIMER_PTR;

FTM_RET		FTNM_TIMER_init(FTNM_TIMER_PTR pTimer, FTM_ULONG ulTimeout);
FTM_RET		FTNM_TIMER_add(FTNM_TIMER_PTR pTimer, FTM_ULONG ulTimeout);
FTM_BOOL 	FTNM_TIMER_isExpired(FTNM_TIMER_PTR pTimer);
FTM_RET 	FTNM_TIMER_waitForExpired(FTNM_TIMER_PTR pTimer);

#endif

