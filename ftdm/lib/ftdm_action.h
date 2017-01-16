#ifndef	_FTDM_ACTION_H_
#define	_FTDM_ACTION_H_

#include "ftm.h"

struct FTDM_CONTEXT_STRUCT;

typedef	struct
{
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM;
	FTM_ACTION			xInfo;
	FTM_ULONG			ulIndex;
}	FTDM_ACTION, _PTR_ FTDM_ACTION_PTR;

FTM_RET	FTDM_ACTION_create
(
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM,
	FTM_ACTION_PTR 		pInfo,
	FTDM_ACTION_PTR	_PTR_ ppAction
);

FTM_RET	FTDM_ACTION_destroy
(
	FTDM_ACTION_PTR	_PTR_ ppAction
);

FTM_RET	FTDM_ACTION_init
(
	FTDM_ACTION_PTR		pAction
);

FTM_RET FTDM_ACTION_final
(
	FTDM_ACTION_PTR		pAction
);

FTM_RET	FTDM_ACTION_loadConfig
(
	FTDM_ACTION_PTR		pAction,
	FTM_CONFIG_ITEM_PTR		xSection
);

FTM_RET	FTDM_ACTION_get
(
	FTDM_ACTION_PTR	pAction,
	FTM_ACTION_PTR	pInfo
);

FTM_RET	FTDM_ACTION_set
(
	FTDM_ACTION_PTR		pAction,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
);

FTM_RET	FTDM_ACTION_getID
(
	FTDM_ACTION_PTR	pAction,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
);

FTM_BOOL	FTDM_ACTION_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_BOOL	FTDM_ACTION_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

#endif
