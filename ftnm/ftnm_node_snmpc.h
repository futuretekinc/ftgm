#ifndef	__FTNM_SNMP_CLIENT_H__
#define	__FTNM_SNMP_CLIENT_H__

#include "ftnm.h"

FTM_RET		FTNM_SNMPC_init(FTM_VOID);

FTM_RET		FTNM_NODE_SNMPC_init(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET		FTNM_NODE_SNMPC_final(FTNM_NODE_SNMPC_PTR pNode);
FTM_BOOL	FTNM_NODE_SNMPC_isRunning(FTNM_NODE_SNMPC_PTR pNode);

FTM_RET 	FTNM_NODE_SNMPC_startAsync(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET 	FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode);
#endif

