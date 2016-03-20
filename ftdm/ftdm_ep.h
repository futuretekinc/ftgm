#ifndef	__FTDM_EP_H__
#define	__FTDM_EP_H__

#include "ftdm_config.h"

typedef	struct
{
	FTM_EP		xInfo;
	FTM_ULONG	ulCount;
	FTM_ULONG	ulFirstTime;
	FTM_ULONG	ulLastTime;
}	FTDM_EP, _PTR_ FTDM_EP_PTR;
	
FTM_RET	FTDM_EP_init
(
	FTM_VOID
);

FTM_RET FTDM_EP_final
(
	FTM_VOID
);

FTM_RET	FTDM_EP_loadConfig
(
	FTDM_CFG_EP_PTR	pConfig
);

FTM_RET	FTDM_EP_loadFromFile
(
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTDM_EP_loadFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_EP_saveToDB
(
	FTM_VOID
);

FTM_RET	FTDM_EP_create
(
	FTM_EP_PTR 		pInfo
);

FTM_RET	FTDM_EP_destroy
(
	FTM_EP_ID 		xEPID
);

FTM_RET	FTDM_EP_count
(
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_EP_get
(
	FTM_EP_ID			xEPID,
	FTDM_EP_PTR	_PTR_ 	ppEP
);

FTM_RET	FTDM_EP_getAt
(
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR	_PTR_ 	ppEP
);

FTM_RET	FTDM_EP_showList
(
	FTM_VOID
);
#endif
