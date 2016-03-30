#ifndef	_FTM_RULE_H_
#define	_FTM_RULE_H_

#include "ftm_types.h"
#include "ftm_ep.h"
#include "ftm_trigger.h"
#include "ftm_action.h"
#include "ftm_time.h"
#include "ftm_list.h"

typedef	FTM_ULONG	FTM_RULE_ID, _PTR_ FTM_RULE_ID_PTR;

typedef	enum
{
	FTM_RULE_STATE_ACTIVATE,
	FTM_RULE_STATE_DEACTIVATE
}	FTM_RULE_STATE, _PTR_ FTM_RULE_STATE_PTR;

typedef	struct
{
	FTM_RULE_ID		xID;
	FTM_RULE_STATE	xState;
	struct 
	{
		FTM_ULONG		ulTriggers;
		FTM_TRIGGER_ID	pTriggers[8];
		FTM_ULONG		ulActions;
		FTM_ACTION_ID	pActions[8];
	}	xParams;
}	FTM_RULE, _PTR_ FTM_RULE_PTR;


FTM_RET	FTM_RULE_init(FTM_VOID);
FTM_RET	FTM_RULE_final(FTM_VOID);

FTM_RET	FTM_RULE_create(FTM_RULE_PTR pRule);
FTM_RET	FTM_RULE_destroy(FTM_RULE_PTR pRule);

FTM_RET	FTM_RULE_append(FTM_RULE_PTR pRule);
FTM_RET	FTM_RULE_remove(FTM_RULE_PTR pRule);

FTM_RET FTM_RULE_count(FTM_ULONG_PTR pulCount);
FTM_RET FTM_RULE_get(FTM_RULE_ID xID, FTM_RULE_PTR _PTR_ ppRule);
FTM_RET FTM_RULE_getAt(FTM_ULONG ulIndex, FTM_RULE_PTR _PTR_ ppRule);

#endif
