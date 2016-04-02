#ifndef	__FTOM_NODE_MANAGEMENT_H__
#define	__FTOM_NODE_MANAGEMENT_H__

#include <pthread.h>
#include "ftm.h"
#include "ftom_node.h"

typedef	struct FTOM_NODEM_STRUCT
{
	FTOM_PTR		pOM;
	FTM_LIST_PTR	pList;
}	FTOM_NODEM, _PTR_ FTOM_NODEM_PTR;
	
FTM_RET	FTOM_NODEM_create
(
	FTOM_PTR		pOM,
	FTOM_NODEM_PTR _PTR_ ppNodeM
);

FTM_RET	FTOM_NODEM_destroy
(
	FTOM_NODEM_PTR _PTR_ ppNodeM
);

FTM_RET	FTOM_NODEM_countNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTOM_NODEM_getNode
(
	FTOM_NODEM_PTR 	pNodeM,
	FTM_CHAR_PTR 	pDID, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTOM_NODEM_getNodeAt
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_ULONG 		ulIndex, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODEM_attachNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTOM_NODE_PTR	pNode
);

FTM_RET	FTOM_NODEM_detachNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTOM_NODE_PTR	pNode
);

#endif

