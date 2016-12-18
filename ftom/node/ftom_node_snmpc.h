#ifndef	__FTOM_NODE_SNMP_CLIENT_H__
#define	__FTOM_NODE_SNMP_CLIENT_H__

#include <semaphore.h>
#include <net-snmp/library/oid.h>
#include "ftom.h"
#include "ftm_timer.h"
#include "ftom_node.h"
#include "ftom_node_class.h"
#include "ftom_snmpc.h"

typedef struct	
{
	FTM_SNMP_OID	xOID;
}	FTOM_NODE_SNMPC_EP_OPTS, _PTR_ FTOM_NODE_SNMPC_EP_OPTS_PTR;

typedef	struct  FTOM_NODE_SNMPC_STRUCT
{
	FTOM_NODE				xCommon;
	FTM_CHAR				pIP[FTM_URL_LEN+1];
	struct snmp_session 	*pSession;		/* SNMP session data */
	FTOM_SNMPC_PTR			pSNMPC;
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

FTM_RET	FTOM_NODE_SNMPC_init
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_final
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_prestart
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_prestop
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_getModel
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_CHAR_PTR		pModel,
	FTM_ULONG			ulMaxLen
);

FTM_RET	FTOM_NODE_SNMPC_getEPCount
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pulCount
);


FTM_RET	FTOM_NODE_SNMPC_getEPID
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				ulMaxLen
);

FTM_RET	FTOM_NODE_SNMPC_getEPName
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pName,
	FTM_ULONG				ulMaxLen
);

FTM_RET	FTOM_NODE_SNMPC_getEPState
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_BOOL_PTR			pbEnable
);

FTM_RET	FTOM_NODE_SNMPC_getEPUpdateInterval
(
	FTOM_NODE_SNMPC_PTR	pNode,
	FTM_ULONG			ulEPType,
	FTM_ULONG			ulIndex,
	FTM_ULONG_PTR		pulInterval
);

FTM_RET	FTOM_NODE_SNMPC_getEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_NODE_SNMPC_setEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_NODE_SNMPC_getEPDataAsync
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP
);

FTM_RET	FTOM_NODE_SNMPC_setEPDataAsync
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
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

FTM_RET	FTOM_NODE_SNMPC_getOID
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_CHAR_PTR		pFieldName,
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
);

FTM_RET	FTOM_NODE_SNMPC_getOIDForUpdateInterval
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	FTM_SNMP_OID_PTR		pOID
);

FTM_RET	FTOM_NODE_SNMPC_getEPID
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				ulMaxLen
);

FTM_RET	FTOM_NODE_SNMPC_getEPName
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pName,
	FTM_ULONG				ulMaxLen
);

FTM_RET	FTOM_NODE_SNMPC_getEPState
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_BOOL_PTR			pbEnable
);

FTM_RET	FTOM_NODE_SNMPC_getEPInterval
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_ULONG_PTR			pulInterval
);

FTM_RET	FTOM_NODE_SNMPC_set
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_NODE_FIELD			xFields,
	FTM_NODE_PTR			pInfo
);

FTM_RET	FTOM_NODE_SNMPC_attachEP
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTOM_EP_PTR				pEP
);

FTM_RET	FTOM_NODE_SNMPC_detachEP
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTOM_EP_PTR				pEP
);

FTM_RET	FTOM_NODE_SNMPC_printOpts
(
	FTOM_NODE_SNMPC_PTR	pNode
);
#endif

