#ifndef	_FTM_UTIL_H_
#define	_FTM_UTIL_H_

#include <time.h>
#include "ftm.h"

FTM_RET	FTM_makeID
(
	FTM_CHAR_PTR	pID,
	FTM_ULONG		ulLen
);

FTM_CHAR_PTR	FTM_getProgramName
(
	FTM_VOID
);

FTM_RET	FTM_getDefaultDeviceID
(
	FTM_CHAR	pDID[FTM_DID_LEN + 1]
);

FTM_RET	FTM_genNewID
(
	FTM_CHAR_PTR	pID,
	FTM_ULONG		ulIDLen
);
#endif
