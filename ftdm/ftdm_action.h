#ifndef	_FTDM_ACTION_H_
#define	_FTDM_ACTION_H_

#include "ftm.h"

typedef	struct
{
	FTM_ACTION	xInfo;
	FTM_ULONG	ulIndex;
}	FTDM_ACTION, _PTR_ FTDM_ACTION_PTR;

FTM_RET	FTDM_ACTION_init
(
	FTM_VOID
);

FTM_RET FTDM_ACTION_final
(
	FTM_VOID
);

FTM_RET	FTDM_ACTION_showList
(
	FTM_VOID
);

FTM_RET	FTDM_ACTION_loadFromFile
(
	FTM_CHAR_PTR pFileName
);

FTM_RET	FTDM_ACTION_loadFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_ACTION_saveToDB
(
	FTM_VOID
);

FTM_RET	FTDM_ACTION_create
(
	FTM_ACTION_PTR pInfo,
	FTDM_ACTION_PTR	_PTR_ ppAction
);

FTM_RET	FTDM_ACTION_destroy
(
	FTM_CHAR_PTR	pActionID
);

FTM_RET	FTDM_ACTION_count
(
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_ACTION_get
(
	FTM_CHAR_PTR			pActionID,
	FTDM_ACTION_PTR _PTR_ 	ppAction
);

FTM_RET	FTDM_ACTION_getAt
(
	FTM_ULONG		nIndex, 
	FTDM_ACTION_PTR _PTR_	ppAction
);

FTM_RET	FTDM_ACTION_getByIndex
(
	FTM_ULONG				ulIndex,
	FTDM_ACTION_PTR _PTR_ 	ppAction
);

FTM_RET	FTDM_ACTION_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

#endif
