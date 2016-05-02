#ifndef	_FTOM_RULE_MANAGEMENT_H_
#define	_FTOM_RULE_MANAGEMENT_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom_rule.h"

typedef	struct FTOM_STRUCT _PTR_ FTOM_PTR;

typedef	struct
{
	FTM_ULONG		ulTimeout;
}	FTOM_RULEM_CONFIG, _PTR_ FTOM_RULEM_CONFIG_PTR;

typedef	struct	FTOM_RULEM_STRUCT
{
	FTOM_RULEM_CONFIG	xConfig;
	pthread_t			xThread;
	FTOM_PTR	pOM;

	FTM_BOOL			bStop;
	FTOM_MSG_QUEUE_PTR	pMsgQ;
	FTM_LIST_PTR		pRuleList;

	struct FTOM_TRIGGERM_STRUCT *pTriggerM;
	struct FTOM_ACTIONM_STRUCT 	*pActionM;
}	FTOM_RULEM, _PTR_ FTOM_RULEM_PTR;
	
FTM_RET	FTOM_RULEM_create
(
	FTOM_PTR 	pOM, 
	FTOM_RULEM_PTR _PTR_ ppRuleM
);

FTM_RET	FTOM_RULEM_destroy
(
	FTOM_RULEM_PTR _PTR_ ppRuleM
);

FTM_RET	FTOM_RULEM_init
(
	FTOM_PTR pOM, 
	FTOM_RULEM_PTR pRuleM
);

FTM_RET	FTOM_RULEM_final
(
	FTOM_RULEM_PTR pRuleM
);

FTM_RET	FTOM_RULEM_loadConfig
(
	FTOM_RULEM_PTR pRuleM, 
	FTOM_RULEM_CONFIG_PTR pConfig
);

FTM_RET	FTOM_RULEM_loadConfigFromFile
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_CHAR_PTR pFileName
);

FTM_RET	FTOM_RULEM_start
(
	FTOM_RULEM_PTR pRuleM
);

FTM_RET	FTOM_RULEM_stop
(
	FTOM_RULEM_PTR pRuleM
);

FTM_RET	FTOM_RULEM_setTriggerM
(
	FTOM_RULEM_PTR pRuleM, 
	struct FTOM_TRIGGERM_STRUCT *pTriggerM
);

FTM_RET	FTOM_RULEM_setActionM
(
	FTOM_RULEM_PTR pRuleM, 
	struct FTOM_ACTIONM_STRUCT *pActionM
);

FTM_RET	FTOM_RULEM_createRule
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_RULE_PTR pInfo,
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULEM_destroyRule
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULEM_count
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTOM_RULEM_get
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_CHAR_PTR	pRuleID,
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULEM_getAt
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_ULONG ulIndex, 
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULEM_notifyChanged
(
	FTM_CHAR_PTR	pTriggerID
);

FTM_RET	FTOM_RULEM_activate
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_CHAR_PTR	pRuleID
);

FTM_RET	FTOM_RULEM_deactivate
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_CHAR_PTR	pRuleID
);
#endif
