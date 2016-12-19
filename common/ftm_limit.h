#ifndef	__FTM_LIMIT_H__
#define __FTM_LIMIT_H__

#include "ftm.h"

typedef	enum
{
	FTM_LIMIT_TYPE_COUNT = 0,
	FTM_LIMIT_TYPE_TIME,
	FTM_LIMIT_TYPE_HOURS,
	FTM_LIMIT_TYPE_DAYS,
	FTM_LIMIT_TYPE_MONTHS
}	FTM_LIMIT_TYPE, _PTR_ FTM_LIMIT_TYPE_PTR;

typedef	struct
{
	FTM_ULONG		ulStart;
	FTM_ULONG		ulEnd;
}	FTM_LIMIT_TIME, _PTR_ FTM_LIMIT_TIME_PTR;

typedef	struct
{
	FTM_LIMIT_TYPE	xType;
	struct
	{
		FTM_ULONG		ulCount;
		FTM_LIMIT_TIME	xTime;
		FTM_ULONG		ulHours;
		FTM_ULONG		ulDays;
		FTM_ULONG		ulMonths;
	}	xParams;
}	FTM_LIMIT, _PTR_ FTM_LIMIT_PTR;

#endif
