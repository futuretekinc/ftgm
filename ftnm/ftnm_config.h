#ifndef	__FTNM_CONFIG_H__
#define	__FTNM_CONFIG_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftm_list.h"
#include "ftdm_client.h"
typedef	struct
{
	FTM_USHORT		usPort;
	FTM_ULONG		ulMaxSession;
}	FTNM_CFG_SERVER, _PTR_ FTNM_CFG_SERVER_PTR;

typedef	struct
{
	FTM_LIST		xMIBList;
}	FTNM_CFG_SNMPC, _PTR_ FTNM_CFG_SNMPC_PTR;
	
typedef	struct
{
	FTNM_CFG_SERVER	xServer;	
	FTDMC_CFG		xClient;
	FTNM_CFG_SNMPC	xSNMPC;	
}	FTNM_CFG, _PTR_ FTNM_CFG_PTR;

FTM_RET	FTNM_CFG_create(FTNM_CFG_PTR _PTR_ ppConfig);
FTM_RET	FTNM_CFG_copyCreate(FTNM_CFG_PTR _PTR_ ppConfig, FTNM_CFG_PTR pOldConfig);
FTM_RET	FTNM_CFG_destroy(FTNM_CFG_PTR pConfig);

FTM_RET	FTNM_CFG_init(FTNM_CFG_PTR pConfig);
FTM_RET FTNM_CFG_load(FTNM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTNM_CFG_final(FTNM_CFG_PTR pConfig);

FTM_RET	FTNM_CFG_copy(FTNM_CFG_PTR pDestCfg, FTNM_CFG_PTR pSrcCfg);

FTM_RET	FTNM_CFG_show(FTNM_CFG_PTR pConfig);
#endif
