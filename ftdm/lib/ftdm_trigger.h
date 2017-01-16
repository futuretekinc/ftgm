#ifndef	_FTDM_TRIGGER_H_
#define	_FTDM_TRIGGER_H_

#include "ftdm_types.h"

FTM_RET	FTDM_TRIGGER_create
(
	FTDM_PTR pFTDM,
	FTM_TRIGGER_PTR pInfo,
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTDM_TRIGGER_destroy
(
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTDM_TRIGGER_init
(
	FTDM_TRIGGER_PTR	pTrigger
);

FTM_RET FTDM_TRIGGER_final
(
	FTDM_TRIGGER_PTR	pTrigger
);

FTM_RET	FTDM_TRIGGER_loadConfig
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_CONFIG_ITEM_PTR	xSection
);

FTM_RET	FTDM_TRIGGER_get
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_PTR		pInfo
);

FTM_RET	FTDM_TRIGGER_set
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
);

FTM_RET	FTDM_TRIGGER_getID
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			ulBuffLen
);

FTM_RET	FTDM_TRIGGER_show
(
	FTDM_TRIGGER_PTR	pTrigger
);


FTM_BOOL	FTDM_TRIGGER_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_BOOL	FTDM_TRIGGER_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

#endif
