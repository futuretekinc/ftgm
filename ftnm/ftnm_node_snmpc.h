#ifndef	__FTNM_NODE_SNMP_CLIENT_H__
#define	__FTNM_NODE_SNMP_CLIENT_H__

#include <semaphore.h>
#include "ftnm.h"
#include "ftm_timer.h"
#include "ftnm_node.h"

typedef	struct 
{
	FTNM_NODE				xCommon;
	struct snmp_session 	*pSession;		/* SNMP session data */
	sem_t					xLock;
	struct 
	{
		FTM_ULONG			ulRequest;
		FTM_ULONG			ulResponse;
	}	xStatistics;	
}	FTNM_NODE_SNMPC, _PTR_ FTNM_NODE_SNMPC_PTR;

FTM_RET		FTNM_NODE_SNMPC_create(FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode);
FTM_RET		FTNM_NODE_SNMPC_destroy(FTNM_NODE_SNMPC_PTR pNode);

FTM_RET		FTNM_NODE_SNMPC_init(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET		FTNM_NODE_SNMPC_final(FTNM_NODE_SNMPC_PTR pNode);

FTM_BOOL	FTNM_NODE_SNMPC_isRunning(FTNM_NODE_SNMPC_PTR pNode);

FTM_RET 	FTNM_NODE_SNMPC_start(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET 	FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode);

FTM_RET		FTNM_NODE_SNMPC_getOID(FTNM_NODE_SNMPC_PTR pNode, FTM_ULONG ulType, FTM_ULONG ulIndex, oid *pOID, size_t *pnOIDLen);
#endif

