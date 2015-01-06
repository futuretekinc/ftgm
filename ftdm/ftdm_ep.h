#ifndef	__FTM_EP_H__
#define	__FTM_EP_H__

FTM_RET	FTDM_initEP
(
	FTM_VOID
);


FTM_RET	FTDM_insertEP
(
	FTM_EP_INFO_PTR	pEPInfo
);

FTM_RET	FTDM_appendEPData
(
	FTM_EP_DATA_PTR	pData
);
#endif
