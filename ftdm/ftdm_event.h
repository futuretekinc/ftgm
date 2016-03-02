#ifndef	_FTDM_EVENT_H_
#define	_FTDM_EVENT_H_

#include "ftm_types.h"
#include "ftm_list.h"
#include "ftm_event.h"

FTM_RET	FTDM_EVENT_init
(
	FTM_VOID
);

FTM_RET FTDM_EVENT_final
(
	FTM_VOID
);

FTM_RET	FTDM_EVENT_showList
(
	FTM_VOID
);

FTM_RET	FTDM_EVENT_loadFromFile
(
	FTM_CHAR_PTR			pFileName
);

FTM_RET	FTDM_EVENT_loadFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_EVENT_add
(
	FTM_EVENT_PTR 			pInfo
);

FTM_RET	FTDM_EVENT_del
(
	FTM_EPID 				xEPID
);

FTM_RET	FTDM_EVENT_count
(
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTDM_EVENT_get
(
	FTM_EPID				xEPID,
	FTM_EVENT_PTR	_PTR_ 	ppEPInfo
);

FTM_RET	FTDM_EVENT_getAt
(
	FTM_ULONG				nIndex,
	FTM_EVENT_PTR	_PTR_ 	ppEPInfo
);

#endif
