#ifndef	__FTOM_NODE_SNMP_CLIENT_H__
#define	__FTOM_NODE_SNMP_CLIENT_H__

#include <semaphore.h>
#include <net-snmp/library/oid.h>
#include "ftom.h"
#include "ftm_timer.h"
#include "ftom_node.h"
#include "ftom_node_class.h"

typedef	struct  FTOM_NODE_SNMPC_STRUCT
{
	FTOM_NODE				xCommon;
	struct snmp_session 	*pSession;		/* SNMP session data */
	FTM_LOCK_PTR			pLock;
}	FTOM_NODE_SNMPC, _PTR_ FTOM_NODE_SNMPC_PTR;

FTM_RET	FTOM_NODE_SNMPC_getClass
(
	FTM_CHAR_PTR	pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
);

FTM_RET	FTOM_NODE_SNMPC_create
(
	FTM_NODE_PTR 	pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_SNMPC_destroy
(
	FTOM_NODE_SNMPC_PTR _PTR_ pNode
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForID
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForType
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForName
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForSerialNumber
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForValue
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForTime
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForUpdateInterval
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);
#endif

