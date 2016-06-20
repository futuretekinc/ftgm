#ifndef	__FTM_LOGGER_H__
#define	__FTM_LOGGER_H__

#include "ftm_log.h"
#include "ftm_limit.h"

typedef	struct	FTM_LOGGER_CONFIG_STRUCT
{
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_LIMIT	xLimit;
}	FTM_LOGGER_CONFIG, _PTR_ FTM_LOGGER_CONFIG_PTR;

typedef	struct	FTM_LOGGER_STRUCT
{
	FTM_LIST	xLogList;
}	FTM_LOGGER, _PTR_ FTM_LOGGER_PTR;

FTM_RET	FTM_LOGGER_init
(
	FTM_LOGGER_PTR	pLogger
);

FTM_RET	FTM_LOGGER_final
(
	FTM_LOGGER_PTR	pLoger
);

FTM_RET	FTM_LOGGER_count
(
	FTM_LOGGER_PTR	pLogger,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTM_LOGGER_add
(
	FTM_LOGGER_PTR	pLogger,
	FTM_LOG_PTR		pLog
);

FTM_RET	FTM_LOGGER_remove
(
	FTM_LOGGER_PTR	pLogger,
	FTM_LOG_PTR		pLog
);

FTM_RET	FTM_LOGGER_removeAt
(
	FTM_LOGGER_PTR	pLogger,
	FTM_ULONG		ulIndex
);

FTM_RET	FTM_LOGGER_getAt
(
	FTM_LOGGER_PTR	pLogger,
	FTM_ULONG		ulIndex,
	FTM_LOG_PTR _PTR_ ppLog
);
#endif
