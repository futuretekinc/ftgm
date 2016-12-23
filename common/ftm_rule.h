#ifndef	_FTM_RULE_H_
#define	_FTM_RULE_H_

#include "ftm_types.h"
#include "ftm_ep.h"
#include "ftm_event.h"
#include "ftm_action.h"
#include "ftm_time.h"
#include "ftm_list.h"

typedef	FTM_ULONG	FTM_RULE_FIELD, _PTR_ FTM_RULE_FIELD_PTR;

#define	FTM_RULE_FIELD_ID			(1 << 0)
#define	FTM_RULE_FIELD_NAME			(1 << 1)
#define	FTM_RULE_FIELD_STATE		(1 << 2)
#define	FTM_RULE_FIELD_EVENTS		(1 << 3)
#define	FTM_RULE_FIELD_ACTIONS		(1 << 4)
#define	FTM_RULE_FIELD_RULE			(1 << 5)
#define	FTM_RULE_FIELD_ALL			(0xFFFF)

typedef	enum FTM_RULE_ENUM
{
	FTM_RULE_STATE_PRESET,
	FTM_RULE_STATE_SET,
	FTM_RULE_STATE_PRERESET,
	FTM_RULE_STATE_RESET,
	FTM_RULE_STATE_ACTIVATE,
	FTM_RULE_STATE_DEACTIVATE,
}	FTM_RULE_STATE, _PTR_ FTM_RULE_STATE_PTR;

typedef	struct
{
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_RULE_STATE	xState;
	struct 
	{
		FTM_ULONG		ulEvents;
		FTM_CHAR		pEvents[8][FTM_ID_LEN+1];
		FTM_ULONG		ulActions;
		FTM_CHAR		pActions[8][FTM_ID_LEN+1];
	}	xParams;
}	FTM_RULE, _PTR_ FTM_RULE_PTR;


FTM_RET	FTM_RULE_init(FTM_VOID);
FTM_RET	FTM_RULE_final(FTM_VOID);

FTM_RET	FTM_RULE_create(FTM_RULE_PTR pRule);
FTM_RET	FTM_RULE_destroy(FTM_RULE_PTR pRule);

FTM_RET	FTM_RULE_setDefault(FTM_RULE_PTR pRule);

FTM_RET	FTM_RULE_append(FTM_RULE_PTR pRule);
FTM_RET	FTM_RULE_remove(FTM_RULE_PTR pRule);

FTM_RET FTM_RULE_count(FTM_ULONG_PTR pulCount);
FTM_RET FTM_RULE_get(FTM_CHAR_PTR pID, FTM_RULE_PTR _PTR_ ppRule);
FTM_RET FTM_RULE_getAt(FTM_ULONG ulIndex, FTM_RULE_PTR _PTR_ ppRule);

#endif
