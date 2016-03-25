#ifndef	_FTNM_RULE_H_
#define	_FTNM_RULE_H_

#include <pthread.h>
#include "ftm.h"

typedef	FTM_RULE_ID	FTNM_RULE_ID;

typedef	enum
{
	FTNM_RULE_STATE_RESET,
	FTNM_RULE_STATE_PRESET,
	FTNM_RULE_STATE_SET,
	FTNM_RULE_STATE_PRERESET
}	FTNM_RULE_STATE, _PTR_ FTM_RULE_STATE_PTR;

typedef	struct
{
	FTM_RULE		xInfo;
	FTM_BOOL		bActive;
	FTM_LOCK		xLock;
}	FTNM_RULE, _PTR_ FTNM_RULE_PTR;

typedef	struct
{
	FTM_ULONG		ulTimeout;
}	FTNM_RULEM_CONFIG, _PTR_ FTNM_RULEM_CONFIG_PTR;

typedef	struct
{
	FTNM_RULEM_CONFIG	xConfig;
	pthread_t			xThread;

	FTM_BOOL			bStop;
	FTM_MSG_QUEUE_PTR	pMsgQ;
	FTM_LIST_PTR		pRuleList;

	struct FTNM_ACTIONM_STRUCT *pTriggerM;
	struct FTNM_TRIGGERM_STRUCT *pActionM;
}	FTNM_RULEM, _PTR_ FTNM_RULEM_PTR;
	
FTM_RET	FTNM_RULEM_create(FTNM_RULEM_PTR _PTR_ ppRuleM);
FTM_RET	FTNM_RULEM_destroy(FTNM_RULEM_PTR _PTR_ ppRuleM);

FTM_RET	FTNM_RULEM_init(FTNM_RULEM_PTR pRuleM);
FTM_RET	FTNM_RULEM_final(FTNM_RULEM_PTR pRuleM);

FTM_RET	FTNM_RULEM_loadConfig(FTNM_RULEM_PTR pRuleM, FTNM_RULEM_CONFIG_PTR pConfig);
FTM_RET	FTNM_RULEM_loadConfigFromFile(FTNM_RULEM_PTR pRuleM, FTM_CHAR_PTR pFileName);

FTM_RET	FTNM_RULEM_start(FTNM_RULEM_PTR pRuleM);
FTM_RET	FTNM_RULEM_stop(FTNM_RULEM_PTR pRuleM);

FTM_RET	FTNM_RULEM_setTriggerM(FTNM_RULEM_PTR pRuleM, struct FTNM_TRIGGERM_STRUCT *pTriggerM);
FTM_RET	FTNM_RULEM_setActionM(FTNM_RULEM_PTR pRuleM, struct FTNM_ACTIONM_STRUCT *pActionM);

FTM_RET	FTNM_RULEM_append(FTNM_RULEM_PTR pRuleM, FTM_RULE_PTR pRule);
FTM_RET	FTNM_RULEM_del(FTNM_RULEM_PTR pRuleM, FTNM_RULE_ID  xRuleID);
FTM_RET	FTNM_RULEM_count(FTNM_RULEM_PTR pRuleM, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_RULEM_get(FTNM_RULEM_PTR pRuleM, FTNM_RULE_ID xRuleID, FTNM_RULE_PTR _PTR_ ppRule);
FTM_RET	FTNM_RULEM_getAt(FTNM_RULEM_PTR pRuleM, FTM_ULONG ulIndex, FTNM_RULE_PTR _PTR_ ppRule);

FTM_RET	FTNM_RULEM_notifyChanged(FTM_TRIGGER_ID xTrigger);
#endif
