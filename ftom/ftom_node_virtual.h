#ifndef	__FTOM_NODE_VIRTUAL_H__
#define	__FTOM_NODE_VIRTUAL_H__

#include <semaphore.h>
#include <net-snmp/library/oid.h>
#include "ftom.h"
#include "ftm_timer.h"
#include "ftom_node.h"
#include "ftom_node_class.h"

typedef	struct  FTOM_NODE_VIRTUAL_STRUCT
{
	FTOM_NODE				xCommon;
	struct snmp_session 	*pSession;		/* SNMP session data */
	FTM_LOCK_PTR			pLock;
}	FTOM_NODE_VIRTUAL, _PTR_ FTOM_NODE_VIRTUAL_PTR;

FTM_RET	FTOM_NODE_VIRTUAL_getClass
(
	FTM_CHAR_PTR	pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
);

FTM_RET	FTOM_NODE_VIRTUAL_create
(
	FTM_NODE_PTR 	pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_VIRTUAL_destroy
(
	FTOM_NODE_VIRTUAL_PTR _PTR_ pNode
);

FTM_RET	FTOM_NODE_VIRTUAL_getOID
(
	FTOM_NODE_VIRTUAL_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	oid 					*pOID, 
	size_t 					*pnOIDLen
);
#endif

