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
	FTM_EP_PTR		pEPInfo
);

FTM_RET	FTOM_LOG_destroyEP
(
	FTM_EP_PTR		pEPInfo
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
