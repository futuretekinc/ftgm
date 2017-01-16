#ifndef	__FTDM_LOG_H__
#define	__FTDM_LOG_H__

struct FTDM_CONTEXT_STRUCT;

FTM_RET	FTDM_createNodeLog
(
	struct FTDM_CONTEXT_STRUCT _PTR_	pFTDM,
	FTM_CHAR_PTR	pDID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createEPLog
(
	struct FTDM_CONTEXT_STRUCT _PTR_	pFTDM,
	FTM_CHAR_PTR	pEPID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createTriggerLog
(
	struct FTDM_CONTEXT_STRUCT _PTR_	pFTDM,
	FTM_CHAR_PTR	pTriggerID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createActionLog
(
	struct FTDM_CONTEXT_STRUCT _PTR_	pFTDM,
	FTM_CHAR_PTR	pActionID,
	FTM_RET			xResult
);

FTM_RET	FTDM_createRuleLog
(
	struct FTDM_CONTEXT_STRUCT _PTR_	pFTDM,
	FTM_CHAR_PTR	pRuleID,
	FTM_RET			xResult
);

#endif