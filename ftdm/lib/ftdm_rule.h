#ifndef	_FTDM_RULE_H_
#define	_FTDM_RULE_H_

#include "ftm.h"

struct FTDM_CONTEXT_STRUCT;
typedef struct FTDM_RULE_STRUCT _PTR_ FTDM_RULE_PTR;

FTM_RET	FTDM_RULE_create
(
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM,
	FTM_RULE_PTR pInfo,
	FTDM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTDM_RULE_destroy
(
	FTDM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTDM_RULE_init
(
	FTDM_RULE_PTR pRule
);

FTM_RET FTDM_RULE_final
(
	FTDM_RULE_PTR pRule
);

FTM_RET	FTDM_RULE_get
(
	FTDM_RULE_PTR	pRule,
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTDM_RULE_set
(
	FTDM_RULE_PTR pRule,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTDM_RULE_getID
(
	FTDM_RULE_PTR	pRule,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
);

FTM_RET	FTDM_RULE_show
(
	FTDM_RULE_PTR	pRule	
);

FTM_BOOL	FTDM_RULE_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_BOOL	FTDM_RULE_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

#endif
