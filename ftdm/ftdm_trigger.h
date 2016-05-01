#ifndef	_FTDM_TRIGGER_H_
#define	_FTDM_TRIGGER_H_

#include "ftm.h"

typedef	struct
{
	FTM_TRIGGER		xInfo;
	FTM_ULONG		ulIndex;
}	FTDM_TRIGGER, _PTR_ FTDM_TRIGGER_PTR;

FTM_RET	FTDM_TRIGGER_init
(
	FTM_VOID
);

FTM_RET FTDM_TRIGGER_final
(
	FTM_VOID
);

FTM_RET	FTDM_TRIGGER_loadFromFile
(
	FTM_CHAR_PTR pFileName
);

FTM_RET	FTDM_TRIGGER_loadFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_TRIGGER_saveToDB
(
	FTM_VOID
);

FTM_RET	FTDM_TRIGGER_create
(
	FTM_TRIGGER_PTR pInfo,
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTDM_TRIGGER_destroy
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTDM_TRIGGER_count
(
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_TRIGGER_get
(
	FTM_CHAR_PTR	pID,
	FTDM_TRIGGER_PTR _PTR_ 	ppTrigger
);

FTM_RET	FTDM_TRIGGER_getAt
(
	FTM_ULONG				ulIndex, 
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTDM_TRIGGER_getByIndex
(
	FTM_ULONG				ulIndex, 
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTDM_TRIGGER_set
(
	FTM_CHAR_PTR		pID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
);

FTM_RET	FTDM_RULE_show
(
	FTDM_RULE_PTR	pRule	
);

FTM_RET	FTDM_TRIGGER_showList
(
	FTM_VOID
);

#endif
