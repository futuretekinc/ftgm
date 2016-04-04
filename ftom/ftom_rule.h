#ifndef	_FTOM_RULE_H_
#define	_FTOM_RULE_H_

#include <pthread.h>
#include "ftm.h"

typedef	FTM_RULE_ID	FTOM_RULE_ID;
typedef	struct FTOM_STRUCT _PTR_ FTOM_PTR;
typedef	struct	FTOM_RULEM_STRUCT _PTR_ FTOM_RULEM_PTR;

typedef	struct
{
	FTM_RULE		xInfo;
	FTM_BOOL		bActive;
	FTM_LOCK		xLock;
	FTOM_RULEM_PTR	pRuleM;
}	FTOM_RULE, _PTR_ FTOM_RULE_PTR;

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

FTM_RET	FTOM_RULEM_add
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_RULE_PTR pRule
);

FTM_RET	FTOM_RULEM_del
(
	FTOM_RULEM_PTR pRuleM, 
	FTOM_RULE_ID  xRuleID
);

FTM_RET	FTOM_RULEM_count
(
	FTOM_RULEM_PTR pRuleM, 
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTOM_RULEM_get
(
	FTOM_RULEM_PTR pRuleM, 
	FTOM_RULE_ID xRuleID, 
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
	FTM_TRIGGER_ID xTrigger
);

FTM_RET	FTOM_RULEM_activate
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_RULE_ID 	xRuleID
);

FTM_RET	FTOM_RULEM_deactivate
(
	FTOM_RULEM_PTR 	pRuleM, 
	FTM_RULE_ID 	xRuleID
);
#endif
