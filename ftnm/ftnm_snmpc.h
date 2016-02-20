#ifndef	__FTNM_SNMP_CLIENT_H__
#define	__FTNM_SNMP_CLIENT_H__

#include "ftnm.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"

typedef	FTM_ULONG		FTNM_SNMPC_STATE, _PTR_ FTNM_SNMPC_STATE_PTR;

#define	FTNM_SNMPC_STATE_UNKNOWN		0x00000000
#define	FTNM_SNMPC_STATE_INITIALIZED	0x00000001
#define	FTNM_SNMPC_STATE_RUNNING		0x00000002
#define	FTNM_SNMPC_STATE_TIMEOUT		0x00000003
#define	FTNM_SNMPC_STATE_ERROR			0x00000004
#define	FTNM_SNMPC_STATE_COMPLETED		0x00000005

typedef	struct
{
	FTM_CHAR		pName[1024];
	FTM_LIST		xMIBList;

	FTM_ULONG		ulMaxRetryCount;
}	FTNM_SNMPC_CONFIG, _PTR_ FTNM_SNMPC_CONFIG_PTR;

typedef	struct 
{
	FTNM_SNMPC_CONFIG	xConfig;
	pthread_t			xPThread;
}	FTNM_SNMPC, _PTR_ FTNM_SNMPC_PTR;

FTM_RET	FTNM_SNMPC_init(void);
FTM_RET	FTNM_SNMPC_final(void);

FTM_RET FTNM_SNMPC_run(void);

FTM_RET	FTNM_SNMPC_initConfig(void);
FTM_RET	FTNM_SNMPC_finalConfig(void);
FTM_RET FTNM_SNMPC_loadConfig(FTM_CHAR_PTR pFileName);
FTM_RET	FTNM_SNMPC_showConfig(void);


FTM_ULONG	FTNM_SNMPC_getMaxRetryCount(void);
FTM_RET		FTNM_SNMPC_updateEP(FTNM_NODE_SNMPC_PTR pNode, FTNM_EP_PTR pEP);

#endif

