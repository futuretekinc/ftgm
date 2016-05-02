#ifndef	_FTOM_RULE_H_
#define	_FTOM_RULE_H_

#include <pthread.h>
#include "ftm.h"

typedef	struct FTOM_STRUCT _PTR_ FTOM_PTR;
typedef	struct	FTOM_RULEM_STRUCT _PTR_ FTOM_RULEM_PTR;

typedef	struct
{
	FTM_RULE		xInfo;
	FTM_BOOL		bActive;
	FTM_LOCK		xLock;
	FTOM_RULEM_PTR	pRuleM;
}	FTOM_RULE, _PTR_ FTOM_RULE_PTR;

FTM_RET	FTOM_RULE_create
(
	FTM_RULE_PTR pInfo,
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_destroy
(
	FTOM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTOM_RULE_init
(
	FTOM_RULE_PTR pRule,
	FTM_RULE_PTR pInfo
);

FTM_RET	FTOM_RULE_final
(
	FTOM_RULE_PTR pRule
);

#endif
