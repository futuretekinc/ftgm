#ifndef	__FTOM_LOGGER_H__
#define	__FTOM_LOGGER_H__

#include "ftm_logger.h"

typedef	struct	FTOM_LOG_MANAGER_STRUCT 
{
	FTM_LOGGER	xLogger;
}	FTOM_LOG_MANAGER, _PTR_ FTOM_LOG_MANAGER_PTR;

FTM_RET	FTOM_LOGGER_init
(
	FTOM_LOG_MANAGER_PTR	pManager
);

FTM_RET	FTOM_LOGGER_final
(
	FTOM_LOG_MANAGER_PTR	pManager
);

FTM_RET	FTOM_LOGGER_add
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_LOG_PTR		pLog
);

FTM_RET	FTOM_LOGGER_remove
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulRemovedCount
);

FTM_RET	FTOM_LOGGER_count
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_LOGGER_getAt
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
);

#endif
