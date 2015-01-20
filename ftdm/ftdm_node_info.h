#ifndef	__FTDM_NODE_INFO_H__
#define	__FTDM_NODE_INFO_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftdm_config.h"

#define	FTDM_DT_ROOT	0x00000001

FTM_RET	FTDM_NODE_INFO_init
(
	FTDM_CFG_NODE_PTR	pConfig
);

FTM_RET	FTDM_NODE_INFO_final
(
 	FTM_VOID
);

FTM_RET	FTDM_NODE_INFO_add
(
	FTM_NODE_INFO_PTR	pInfo
);

FTM_RET FTDM_NODE_INFO_del
(
	FTM_CHAR_PTR		pDID
);

FTM_RET	FTDM_NODE_INFO_count
(
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTDM_NODE_INFO_get
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
);

FTM_RET	FTDM_NODE_INFO_getAt
(
	FTM_ULONG			nIndex,
	FTM_NODE_INFO_PTR _PTR_	ppNodeInfo
);

FTM_RET	FTDM_NODE_INFO_type
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_TYPE_PTR 	pType
);

#endif

