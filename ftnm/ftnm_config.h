#ifndef	__FTNM_CONFIG_H__
#define	__FTNM_CONFIG_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftdm_client.h"
typedef	struct
{
	FTM_USHORT		usPort;
	FTM_ULONG		ulMaxSession;
}	FTNM_CFG_SERVER, _PTR_ FTNM_CFG_SERVER_PTR;

typedef	struct
{
	FTNM_CFG_SERVER	xServer;	
	FTDM_CFG_CLIENT	xClient;
}	FTNM_CFG, _PTR_ FTNM_CFG_PTR;

FTM_RET	FTNM_CFG_init(FTNM_CFG_PTR pConfig);
FTM_RET FTNM_CFG_load(FTNM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTNM_CFG_final(FTNM_CFG_PTR pConfig);

FTM_RET	FTNM_CFG_copy(FTNM_CFG_PTR pDestCfg, FTNM_CFG_PTR pSrcCfg);

FTM_RET	FTNM_CFG_show(FTNM_CFG_PTR pConfig);
#endif
