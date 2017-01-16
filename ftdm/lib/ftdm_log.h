#ifndef	__FTDM_LOG_H__
#define	__FTDM_LOG_H__


FTM_RET	FTDM_createNodeLog
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pDID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createEPLog
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pEPID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createTriggerLog
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pTriggerID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createActionLog
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pActionID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createRuleLog
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pRuleID,
	FTM_RET			xResult
);

#endif
