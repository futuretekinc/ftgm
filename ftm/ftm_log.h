#ifndef	_FTM_LOG_H_
#define	_FTM_LOG_H_

#include "ftm_types.h"

typedef	FTM_ULONG	FTM_LOG_TYPE;

#define	FTM_LOG_TYPE_EVENT		0x00000001

FTM_RET	FTM_LOG_out
(
	FTM_LOG_TYPE		xType,
	const FTM_CHAR_PTR	pFormat,
	...
);

#define	FTM_LOG(type, format, ...) FTM_LOG_out(type, format, ## __VA_ARGS__)

#endif
