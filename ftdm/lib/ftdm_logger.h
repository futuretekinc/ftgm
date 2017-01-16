#ifndef	__FTDM_LOGGER_H__
#define	__FTDM_LOGGER_H__

#include "ftdm_types.h"
#include "ftm_logger.h"

FTM_RET	FTDM_LOGGER_create
(
	FTDM_PTR	pFTDM,
	FTDM_LOGGER_PTR	_PTR_ 	ppLogger
);

FTM_RET	FTDM_LOGGER_destroy
(
	FTDM_LOGGER_PTR	_PTR_ ppLogger
);

FTM_RET	FTDM_LOGGER_init
(
	FTDM_LOGGER_PTR	pLogger
);

FTM_RET	FTDM_LOGGER_final
(
	FTDM_LOGGER_PTR	pLogger
);

FTM_RET	FTDM_LOGGER_add
(
	FTDM_LOGGER_PTR	pLogger,
	FTM_LOG_PTR		pLog
);

FTM_RET	FTDM_LOGGER_del
(
	FTDM_LOGGER_PTR	pLogger,
	FTM_INT 		nIndex, 
	FTM_ULONG		nCount,
	FTM_ULONG_PTR	pulCount
) ;

FTM_RET	FTDM_LOGGER_count
(
	FTDM_LOGGER_PTR	pLogger,
	FTM_ULONG_PTR	pulCount
);

FTM_RET FTDM_LOGGER_info
(	
	FTDM_LOGGER_PTR	pLogger,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_LOGGER_get
(
	FTDM_LOGGER_PTR	pLogger,
	FTM_ULONG		nStartIndex,
	FTM_LOG_PTR 	pLoggerData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pCount 
);

FTM_RET	FTDM_LOGGER_getWithTime
(
	FTDM_LOGGER_PTR			pLogger,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_LOG_PTR 	pLoggerData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount 
);

#endif
