#ifndef	__FTDM_LOG_H__
#define	__FTDM_LOG_H__

FTM_RET	FTDM_LOG_createNode
(
	FTM_CHAR_PTR	pDID,
	FTM_RET			xResult
);

FTM_RET	FTDM_LOG_createEP
(
	FTM_CHAR_PTR	pEPID,
	FTM_RET			xResult
);

FTM_RET	FTDM_LOG_createTrigger
(
	FTM_CHAR_PTR	pTriggerID,
	FTM_RET			xResult
);

FTM_RET	FTDM_LOG_createAction
(
	FTM_CHAR_PTR	pActionID,
	FTM_RET			xResult
);

FTM_RET	FTDM_LOG_createRule
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RET			xResult
);

#endif
