#ifndef	__FTDM_NODE_H__
#define	__FTDM_NODE_H__

#include "ftm.h"
#include "ftdm_config.h"

#define	FTDM_DT_ROOT	0x00000001

struct FTDM_CONTEXT_STRUCT;
typedef struct FTDM_NODE_STRUCT _PTR_ FTDM_NODE_PTR;


FTM_RET	FTDM_NODE_create
(
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM,
	FTM_NODE_PTR		pInfo,
	FTM_BOOL			bWithDB,
	FTDM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTDM_NODE_destroy
(
	FTDM_NODE_PTR _PTR_ ppNode,
	FTM_BOOL			bWithDB
);

FTM_RET	FTDM_NODE_init
(
	FTDM_NODE_PTR	pNode
);

FTM_RET	FTDM_NODE_final
(
	FTDM_NODE_PTR	pNode
);

FTM_RET	FTDM_NODE_getInfo
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTDM_NODE_setInfo
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTDM_NODE_getID
(
	FTDM_NODE_PTR	pNode,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
);

FTM_RET	FTDM_NODE_getType
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_TYPE_PTR	pType
);

FTM_RET	FTDM_NODE_isInfoChanged
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo,
	FTM_BOOL_PTR	pbChanged
);

FTM_BOOL FTDM_NODE_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pKey
);

FTM_BOOL FTDM_NODE_comparator
(
	const FTM_VOID_PTR pElement1,
	const FTM_VOID_PTR pElement2
);
#endif

