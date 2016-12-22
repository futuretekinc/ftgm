#ifndef	_FTDM_EVENT_H_
#define	_FTDM_EVENT_H_

#include "ftm.h"
#include "ftdm_dbif.h"

typedef	struct
{
	FTM_EVENT		xInfo;
	FTM_ULONG		ulIndex;

	FTDM_DBIF_PTR	pDBIF;
}	FTDM_EVENT, _PTR_ FTDM_EVENT_PTR;

FTM_RET	FTDM_EVENT_init
(
	FTM_VOID
);

FTM_RET FTDM_EVENT_final
(
	FTM_VOID
);

FTM_RET	FTDM_EVENT_create
(
	FTM_EVENT_PTR pInfo,
	FTDM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTDM_EVENT_destroy
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTDM_EVENT_count
(
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_EVENT_get
(
	FTM_CHAR_PTR	pID,
	FTDM_EVENT_PTR _PTR_ 	ppEvent
);

FTM_RET	FTDM_EVENT_getAt
(
	FTM_ULONG				ulIndex, 
	FTDM_EVENT_PTR _PTR_	ppEvent
);

FTM_RET	FTDM_EVENT_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_EVENT_getByIndex
(
	FTM_ULONG				ulIndex, 
	FTDM_EVENT_PTR _PTR_	ppEvent
);

FTM_RET	FTDM_EVENT_set
(
	FTM_CHAR_PTR		pID,
	FTM_EVENT_FIELD	xFields,
	FTM_EVENT_PTR		pInfo
);

FTM_RET	FTDM_EVENT_show
(
	FTDM_EVENT_PTR	pEvent
);

FTM_RET	FTDM_EVENT_showList
(
	FTM_VOID
);

#endif
