#ifndef	_FTM_TIME_H_
#define	_FTM_TIME_H_

#include <sys/time.h>
#include "ftm_types.h"

typedef	enum
{
	FTM_TIME_ALIGN_1S,
	FTM_TIME_ALIGN_10S,
	FTM_TIME_ALIGN_1M,
	FTM_TIME_ALIGN_10M,
	FTM_TIME_ALIGN_1H,
	FTM_TIME_ALIGN_1D
}	FTM_TIME_ALIGN, _PTR_ FTM_TIME_ALIGN_PTR;

typedef	struct
{
	struct timeval	xTimeval;
}	FTM_TIME, _PTR_ FTM_TIME_PTR;

FTM_RET	FTM_TIME_getCurrent
(
	FTM_TIME_PTR pTime
);

FTM_RET	FTM_TIME_set
(
	FTM_TIME_PTR 	pTime, 
	FTM_ULONG 		ulMilliSeconds
);

FTM_RET	FTM_TIME_setString
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pString
);

FTM_RET	FTM_TIME_setSeconds
(
	FTM_TIME_PTR 	pTime, 
	FTM_ULONG 		ulSeconds
);

FTM_RET	FTM_TIME_add
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_TIME_PTR 	pTimeB, 
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addMS
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeMS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addSecs
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addMins
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeM,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addHours
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeH,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addDays
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeD,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_sub
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_TIME_PTR 	pTimeB, 
	FTM_TIME_PTR 	pTimeR
);

FTM_RET	FTM_TIME_subMS
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeMS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subSecs
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subMins
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeM,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subHours
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeH,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subDays
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_ULONG		ulTimeD,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_isValid
(
	FTM_TIME_PTR	pTime,
	FTM_TIME_PTR	pBegin,
	FTM_TIME_PTR	pEnd
);

FTM_RET	FTM_TIME_align
(
	FTM_TIME_PTR	pTimeS,
	FTM_TIME_ALIGN	xAlign,
	FTM_TIME_PTR	pTimeR
);

FTM_RET	FTM_TIME_toMS
(
	FTM_TIME_PTR	pTime,
	FTM_UINT64_PTR	pullMS
);

FTM_RET	FTM_TIME_toSecs
(
	FTM_TIME_PTR	pTime,
	FTM_ULONG_PTR	pulSecs
);

FTM_BOOL		FTM_TIME_isZero
(
	FTM_TIME_PTR 	pTime
);

FTM_CHAR_PTR	FTM_TIME_printf
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pFormat
);

FTM_CHAR_PTR	FTM_TIME_printf2
(
	FTM_ULONG		ulTime,
	FTM_CHAR_PTR	pFormat
);

FTM_CHAR_PTR	FTM_TIME_printfCurrent
(
	FTM_CHAR_PTR	pFormat
);

#endif
