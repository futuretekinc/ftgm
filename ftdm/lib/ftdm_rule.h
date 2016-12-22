#ifndef	_FTDM_RULE_H_
#define	_FTDM_RULE_H_

#include "ftm.h"
#include "ftdm_dbif.h"

typedef	struct
{
	FTM_RULE	xInfo;

	FTDM_DBIF_PTR	pDBIF;
}	FTDM_RULE, _PTR_ FTDM_RULE_PTR;

FTM_RET	FTDM_RULE_init
(
	FTM_VOID
);

FTM_RET FTDM_RULE_final
(
	FTM_VOID
);

FTM_RET	FTDM_RULE_showList
(
	FTM_VOID
);

FTM_RET	FTDM_RULE_count
(
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_RULE_get
(
	FTM_CHAR_PTR	pRuleID, 
	FTDM_RULE_PTR _PTR_ 	ppRule
);

FTM_RET	FTDM_RULE_getAt
(
	FTM_ULONG	nIndex, 
	FTDM_RULE_PTR _PTR_	ppRule
);

FTM_RET	FTDM_RULE_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

#endif
