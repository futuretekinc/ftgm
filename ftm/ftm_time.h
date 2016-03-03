#ifndef	_FTM_TIME_H_
#define	_FTM_TIME_H_

#include <sys/time.h>
#include "ftm_types.h"

typedef	struct
{
	struct timeval	xTimeVal;
}	FTM_TIME, _PTR_ FTM_TIME_PTR;

FTM_RET	FTM_TIME_getCurrentTime(FTM_TIME_PTR pTime);

#endif
