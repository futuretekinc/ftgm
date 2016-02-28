#ifndef	__FTDM_EP_H__
#define	__FTDM_EP_H__

#include "ftdm_config.h"

FTM_RET	FTDM_EP_init
(
	FTDM_CFG_EP_PTR	pConfig
);

FTM_RET FTDM_EP_final
(
	FTM_VOID
);

FTM_RET	FTDM_EP_add
(
	FTM_EP_PTR 	pInfo
);

FTM_RET	FTDM_EP_del
(
	FTM_EP_ID 			xEPID
);

FTM_RET	FTDM_EP_count
(
	FTM_EP_CLASS		xClass,
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTDM_EP_get
(
	FTM_EP_ID				xEPID,
	FTM_EP_PTR	_PTR_ 	ppEPInfo
);

FTM_RET	FTDM_EP_getAt
(
	FTM_ULONG				nIndex,
	FTM_EP_PTR	_PTR_ 	ppEPInfo
);

#endif
