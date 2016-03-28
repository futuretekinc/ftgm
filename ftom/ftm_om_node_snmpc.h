#ifndef	__FTM_OM_NODE_SNMP_CLIENT_H__
#define	__FTM_OM_NODE_SNMP_CLIENT_H__

#include <semaphore.h>
#include <net-snmp/library/oid.h>
#include "ftm_om.h"
#include "ftm_timer.h"
#include "ftm_om_node.h"

typedef	struct 
{
	FTM_OM_NODE				xCommon;
	struct snmp_session 	*pSession;		/* SNMP session data */
	sem_t					xLock;

	struct 
	{
		FTM_ULONG			ulRequest;
		FTM_ULONG			ulResponse;
	}	xStatistics;	
}	FTM_OM_NODE_SNMPC, _PTR_ FTM_OM_NODE_SNMPC_PTR;

FTM_RET	FTM_OM_NODE_SNMPC_create
(
	FTM_NODE_PTR 	pInfo, 
	FTM_OM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTM_OM_NODE_SNMPC_destroy
(
	FTM_OM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTM_OM_NODE_SNMPC_init
(
	FTM_OM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTM_OM_NODE_SNMPC_final
(
	FTM_OM_NODE_SNMPC_PTR pNode
);

FTM_BOOL	FTM_OM_NODE_SNMPC_isRunning
(
	FTM_OM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTM_OM_NODE_SNMPC_start
(
	FTM_OM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTM_OM_NODE_SNMPC_stop
(
	FTM_OM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTM_OM_NODE_SNMPC_getOID
(
	FTM_OM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	oid 					*pOID, 
	size_t 					*pnOIDLen
);
#endif

