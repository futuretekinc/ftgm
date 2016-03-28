#ifndef	_FTM_OM_RULE_H_
#define	_FTM_OM_RULE_H_

#include <pthread.h>
#include "ftm.h"

typedef	FTM_RULE_ID	FTM_OM_RULE_ID;
typedef	struct FTM_OM_STRUCT _PTR_ FTM_OM_PTR;
typedef	struct	FTM_OM_RULEM_STRUCT _PTR_ FTM_OM_RULEM_PTR;

typedef	enum
{
	FTM_OM_RULE_STATE_RESET,
	FTM_OM_RULE_STATE_PRESET,
	FTM_OM_RULE_STATE_SET,
	FTM_OM_RULE_STATE_PRERESET
}	FTM_OM_RULE_STATE, _PTR_ FTM_OM_RULE_STATE_PTR;

typedef	struct
{
	FTM_RULE		xInfo;
	FTM_BOOL		bActive;
	FTM_LOCK		xLock;
	FTM_OM_RULEM_PTR	pRuleM;
}	FTM_OM_RULE, _PTR_ FTM_OM_RULE_PTR;

typedef	struct
{
	FTM_ULONG		ulTimeout;
}	FTM_OM_RULEM_CONFIG, _PTR_ FTM_OM_RULEM_CONFIG_PTR;

typedef	struct	FTM_OM_RULEM_STRUCT
{
	FTM_OM_RULEM_CONFIG	xConfig;
	pthread_t			xThread;
	FTM_OM_PTR	pOM;

	FTM_BOOL			bStop;
	FTM_MSG_QUEUE_PTR	pMsgQ;
	FTM_LIST_PTR		pRuleList;

	struct FTM_OM_TRIGGERM_STRUCT *pTriggerM;
	struct FTM_OM_ACTIONM_STRUCT 	*pActionM;
}	FTM_OM_RULEM, _PTR_ FTM_OM_RULEM_PTR;
	
FTM_RET	FTM_OM_RULEM_create
(
	FTM_OM_PTR 	pOM, 
	FTM_OM_RULEM_PTR _PTR_ ppRuleM
);

FTM_RET	FTM_OM_RULEM_destroy
(
	FTM_OM_RULEM_PTR _PTR_ ppRuleM
);

FTM_RET	FTM_OM_RULEM_init
(
	FTM_OM_PTR pOM, 
	FTM_OM_RULEM_PTR pRuleM
);

FTM_RET	FTM_OM_RULEM_final
(
	FTM_OM_RULEM_PTR pRuleM
);

FTM_RET	FTM_OM_RULEM_loadConfig
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_OM_RULEM_CONFIG_PTR pConfig
);

FTM_RET	FTM_OM_RULEM_loadConfigFromFile
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_CHAR_PTR pFileName
);

FTM_RET	FTM_OM_RULEM_start
(
	FTM_OM_RULEM_PTR pRuleM
);

FTM_RET	FTM_OM_RULEM_stop
(
	FTM_OM_RULEM_PTR pRuleM
);

FTM_RET	FTM_OM_RULEM_setTriggerM
(
	FTM_OM_RULEM_PTR pRuleM, 
	struct FTM_OM_TRIGGERM_STRUCT *pTriggerM
);

FTM_RET	FTM_OM_RULEM_setActionM
(
	FTM_OM_RULEM_PTR pRuleM, 
	struct FTM_OM_ACTIONM_STRUCT *pActionM
);

FTM_RET	FTM_OM_RULEM_add
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_RULE_PTR pRule
);

FTM_RET	FTM_OM_RULEM_del
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_OM_RULE_ID  xRuleID
);

FTM_RET	FTM_OM_RULEM_count
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTM_OM_RULEM_get
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_OM_RULE_ID xRuleID, 
	FTM_OM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTM_OM_RULEM_getAt
(
	FTM_OM_RULEM_PTR pRuleM, 
	FTM_ULONG ulIndex, 
	FTM_OM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTM_OM_RULEM_notifyChanged
(
	FTM_TRIGGER_ID xTrigger
);
#endif