#ifndef	__FTM_EP_H__
#define	__FTM_EP_H__

FTM_RET	FTDM_EP_INFO_init
(
	FTDM_CFG_EP_PTR	pConfig
);

FTM_RET FTDM_EP_INFO_final
(
	FTM_VOID
);

FTM_RET	FTDM_EP_INFO_add
(
	FTM_EP_INFO_PTR 	pInfo
);

FTM_RET	FTDM_EP_INFO_del
(
	FTM_EPID 			xEPID
);

FTM_RET	FTDM_EP_INFO_count
(
	FTM_EP_CLASS		xClass,
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTDM_EP_INFO_get
(
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR	_PTR_ 	ppEPInfo
);

FTM_RET	FTDM_EP_INFO_getAt
(
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR	_PTR_ 	ppEPInfo
);

#endif
