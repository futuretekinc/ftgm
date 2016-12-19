#ifndef	_FTOM_UTILS_H_
#define	_FTOM_UTILS_H_

#include "ftom.h"

FTM_RET	FTOM_getDefaultDeviceID
(
	FTM_CHAR	pDID[FTM_DID_LEN + 1]
);

FTM_RET	FTOM_genNewID
(
	FTM_CHAR_PTR	pID,
	FTM_ULONG		ulIDLen
);
#endif
