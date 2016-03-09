#ifndef	__FTDM_NODE_H__
#define	__FTDM_NODE_H__

#include "ftm.h"
#include "ftdm_config.h"

#define	FTDM_DT_ROOT	0x00000001

FTM_RET	FTDM_NODE_init
(
	FTM_VOID
);

FTM_RET	FTDM_NODE_final
(
 	FTM_VOID
);

FTM_RET	FTDM_NODE_loadConfig
(
	FTDM_CFG_NODE_PTR	pConfig
);

FTM_RET	FTDM_NODE_loadFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_NODE_loadFromFile
(
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTDM_NODE_add
(
	FTM_NODE_PTR	pInfo
);

FTM_RET FTDM_NODE_del
(
	FTM_CHAR_PTR		pDID
);

FTM_RET	FTDM_NODE_count
(
	FTM_ULONG_PTR		pnCount
);

FTM_RET	FTDM_NODE_get
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_PTR _PTR_	ppNodeInfo
);

FTM_RET	FTDM_NODE_getAt
(
	FTM_ULONG			nIndex,
	FTM_NODE_PTR _PTR_	ppNodeInfo
);

FTM_RET	FTDM_NODE_type
(
	FTM_CHAR_PTR		pDID,
	FTM_NODE_TYPE_PTR 	pType
);

FTM_RET	FTDM_NODE_showList
(
	FTM_VOID
);

#endif
