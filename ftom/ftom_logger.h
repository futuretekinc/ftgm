#ifndef	__FTOM_LOGGER_H__
#define	__FTOM_LOGGER_H__

#include "ftom_logger.h"

FTM_RET	FTOM_LOGGER_init
(
	FTM_VOID
);

FTM_RET	FTOM_LOGGER_final
(
	FTM_VOID
);

FTM_RET	FTOM_LOGGER_remove
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulRemovedCount
);

FTM_RET	FTOM_LOGGER_count
(
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_LOGGER_getAt
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_LOG_createNode
(
	FTM_NODE_PTR	pNodeInfo
);

FTM_RET	FTOM_LOG_destroyNode
(
	FTM_NODE_PTR	pNodeInfo
);

FTM_RET	FTOM_LOG_createEP
(
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTOM_LOG_destroyEP
(
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTOM_LOG_createTrigger
(
	FTM_TRIGGER_PTR	pTriggerInfo
);

FTM_RET	FTOM_LOG_destroyTrigger
(
	FTM_TRIGGER_PTR	pTriggerInfo
);

FTM_RET	FTOM_LOG_createAction
(
	FTM_ACTION_PTR	pActionInfo
);

FTM_RET	FTOM_LOG_destroyAction
(
	FTM_ACTION_PTR	pActionInfo
);

FTM_RET	FTOM_LOG_createRule
(
	FTM_RULE_PTR	pRuleInfo
);

FTM_RET	FTOM_LOG_destroyRule
(
	FTM_RULE_PTR	pRuleInfo
);

FTM_RET	FTOM_LOG_event
(
	FTM_RULE_PTR	pRuleInfo,
	FTM_BOOL		bOccurred
);

#endif
