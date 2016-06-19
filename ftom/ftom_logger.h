#ifndef	__FTOM_LOGGER_H__
#define	__FTOM_LOGGER_H__

#include "ftm_logger.h"

typedef	struct	FTOM_LOGGER_STRUCT 
{
	FTM_LOGGER	xCommon;
}	FTOM_LOGGER, _PTR_ FTOM_LOGGER_PTR;

FTM_RET	FTOM_LOGGER_create
(
	FTOM_LOGGER_PTR	_PTR_ ppLogger
);

FTM_RET	FTOM_LOGGER_destroy
(
	FTOM_LOGGER_PTR	_PTR_ ppLogger
);

FTM_RET	FTOM_LOGGER_createNode
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_LOGGER_createEP
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_LOGGER_event
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL		bOccurred
);

#endif
