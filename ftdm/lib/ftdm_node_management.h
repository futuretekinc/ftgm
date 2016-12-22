#ifndef	__FTDM_NODE_MANAGEMENT_H__
#define	__FTDM_NODE_MANAGEMENT_H__

#include "ftm.h"
#include "ftdm_node.h"
#include "ftdm_config.h"
#include "ftdm_dbif.h"

typedef struct
{
	FTDM_DBIF_PTR	pDBIF;
	FTM_LIST_PTR	pList;
}	FTDM_NODEM, _PTR_ FTDM_NODEM_PTR;

FTM_RET	FTDM_NODEM_create
(
	FTDM_DBIF_PTR	pDBIF,
	FTDM_NODEM_PTR _PTR_ ppNodeM
);

FTM_RET	FTDM_NODEM_destroy
(
	FTDM_NODEM_PTR _PTR_ ppNodeM
);

FTM_RET	FTDM_NODEM_init
(
	FTDM_NODEM_PTR	pNodeM
);

FTM_RET	FTDM_NODEM_final
(
	FTDM_NODEM_PTR	pNodeM
);

FTM_RET	FTDM_NODEM_loadConfig
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTDM_NODEM_loadFromDB
(
	FTDM_NODEM_PTR	pNodeM
);

FTM_RET	FTDM_NODEM_loadFromFile
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTDM_NODEM_saveToDB
(
	FTDM_NODEM_PTR	pNodeM
);

FTM_RET	FTDM_NODEM_append
(
	FTDM_NODEM_PTR	pNodeM,
	FTDM_NODE_PTR	pNode
);

FTM_RET	FTDM_NODEM_remove
(
	FTDM_NODEM_PTR	pNodeM,
	FTDM_NODE_PTR	pNode
);

FTM_RET	FTDM_NODEM_count
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_NODEM_get
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR	pDID,
	FTDM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTDM_NODEM_getAt
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_ULONG			nIndex,
	FTDM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTDM_NODEM_getDIDList
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_NODEM_type
(
	FTDM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_TYPE_PTR 	pType
);

FTM_RET	FTDM_NODEM_showList
(
	FTDM_NODEM_PTR	pNodeM
);

#endif

