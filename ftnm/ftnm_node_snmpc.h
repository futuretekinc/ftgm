#ifndef	__FTNM_NODE_SNMP_CLIENT_H__
#define	__FTNM_NODE_SNMP_CLIENT_H__

#include "ftnm.h"
#include "ftnm_node.h"
#include <semaphore.h>

typedef	struct 
{
	FTNM_NODE				xCommon;

	FTNM_EP_PTR				pCurrentEP;
	struct snmp_session 	*pSession;		/* SNMP session data */
	FTM_BOOL				bRun;
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
#endif

