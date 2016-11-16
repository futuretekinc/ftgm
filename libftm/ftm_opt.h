#ifndef	__FTM_OPT__
#define	__FTM_OPT__

#include "ftm_types.h"

typedef	struct
{
	FTM_CHAR		xOpt;
	FTM_CHAR_PTR	pParam;
}	FTM_OPT, _PTR_ FTM_OPT_PTR;

FTM_RET	FTM_getOptions
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_CHAR_PTR	pIndicators,
	FTM_OPT_PTR		pOpts,
	FTM_ULONG		ulMaxOpts,
	FTM_ULONG_PTR	pulOptCount
);

#endif
