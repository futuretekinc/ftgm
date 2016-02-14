#ifndef	__FTNM_SNMP_CLIENT_H__
#define	__FTNM_SNMP_CLIENT_H__

#include "ftnm.h"

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
}	FTNM_SNMPC_CONFIG, _PTR_ FTNM_SNMPC_CONFIG_PTR;

typedef	struct 
{
	FTNM_SNMPC_CONFIG	xConfig;
	pthread_t			xPThread;
}	FTNM_SNMPC, _PTR_ FTNM_SNMPC_PTR;

FTM_RET	FTNM_SNMPC_init(FTNM_SNMPC_PTR pSNMPC);
FTM_RET	FTNM_SNMPC_final(FTNM_SNMPC_PTR pSNMPC);
FTM_RET	FTNM_SNMPC_loadConfig(FTNM_SNMPC_PTR pSNMPC, FTM_CHAR_PTR pConfigFileName);
FTM_RET FTNM_SNMPC_run(FTNM_SNMPC_PTR pSNMPC);

FTM_RET	FTNM_SNMPC_CFG_create(FTNM_SNMPC_CONFIG_PTR _PTR_ ppConfig);
FTM_RET	FTNM_SNMPC_CFG_copyCreate(FTNM_SNMPC_CONFIG_PTR _PTR_ ppConfig, FTNM_SNMPC_CONFIG_PTR pOldConfig);
FTM_RET	FTNM_SNMPC_CFG_destroy(FTNM_SNMPC_CONFIG_PTR pConfig);

FTM_RET	FTNM_SNMPC_CFG_init(FTNM_SNMPC_CONFIG_PTR pConfig);
FTM_RET FTNM_SNMPC_CFG_load(FTNM_SNMPC_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTNM_SNMPC_CFG_final(FTNM_SNMPC_CONFIG_PTR pConfig);

FTM_RET	FTNM_SNMPC_CFG_copy(FTNM_SNMPC_CONFIG_PTR pDestCfg, FTNM_SNMPC_CONFIG_PTR pSrcCfg);

FTM_RET	FTNM_SNMPC_CFG_show(FTNM_SNMPC_CONFIG_PTR pConfig);

#endif

