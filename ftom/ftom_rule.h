#ifndef	_FTOM_RULE_MANAGEMENT_H_
#define	_FTOM_RULE_MANAGEMENT_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom_rule.h"

typedef	struct
{
	FTM_RULE		xInfo;
	FTM_BOOL		bActive;
	FTM_LOCK		xLock;
}	FTOM_RULE, _PTR_ FTOM_RULE_PTR;
	
FTM_RET	FTOM_RULE_init
(
	FTM_VOID
);

FTM_RET	FTOM_RULE_final
(
	FTM_VOID
);

FTM_RET	FTOM_RULE_create
(
	FTM_RULE_PTR	pInfo,
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_createFromDB
(
	FTM_CHAR_PTR	pID,
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_destroy
(
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_start
(
	FTOM_RULE_PTR pRule
);

FTM_RET	FTOM_RULE_stop
(
	FTOM_RULE_PTR pRule
);

FTM_RET	FTOM_RULE_count
(
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTOM_RULE_get
(
	FTM_CHAR_PTR	pRuleID,
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_setInfo
(
	FTOM_RULE_PTR	pRule,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTOM_RULE_notifyChanged
(
	FTM_CHAR_PTR	pTriggerID
);

FTM_RET	FTOM_RULE_activate
(
	FTM_CHAR_PTR	pRuleID
);

FTM_RET	FTOM_RULE_deactivate
(
	FTM_CHAR_PTR	pRuleID
);

FTM_RET	FTOM_RULE_sendMessage
(
	FTOM_MSG_PTR	pMsg
);

FTM_RET	FTOM_RULE_print
(
	FTOM_RULE_PTR	pRule
);

FTM_RET	FTOM_RULE_printList
(
	FTM_VOID
);

#endif
