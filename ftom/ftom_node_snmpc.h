#ifndef	__FTOM_NODE_SNMP_CLIENT_H__
#define	__FTOM_NODE_SNMP_CLIENT_H__

#include <semaphore.h>
#include <net-snmp/library/oid.h>
#include "ftom.h"
#include "ftm_timer.h"
#include "ftom_node.h"

typedef	struct 
{
	FTOM_NODE				xCommon;
	struct snmp_session 	*pSession;		/* SNMP session data */
	sem_t					xLock;

	struct 
	{
		FTM_ULONG			ulRequest;
		FTM_ULONG			ulResponse;
	}	xStatistics;	
}	FTOM_NODE_SNMPC, _PTR_ FTOM_NODE_SNMPC_PTR;

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

FTM_BOOL	FTOM_NODE_SNMPC_isRunning
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_start
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_stop
(
	FTOM_NODE_SNMPC_PTR pNode
);

FTM_RET	FTOM_NODE_SNMPC_getOID
(
	FTOM_NODE_SNMPC_PTR 	pNode, 
	FTM_ULONG 				ulType, 
	FTM_ULONG 				ulIndex, 
	oid 					*pOID, 
	size_t 					*pnOIDLen
);
#endif
