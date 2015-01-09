#ifndef	__FTDM_CFG_CLIENT_H__
#define	__FTDM_CFG_CLIENT_H__

#include "ftm_types.h"
#include "simclist.h"
#include <time.h>

#define	FTDMC_SERVER_IP_LEN	32

typedef	struct
{
	struct
	{
		FTM_CHAR	pServerIP[FTDMC_SERVER_IP_LEN];
		FTM_USHORT	usPort;
	}	xNetwork;

	struct
	{
		list_t		xEPList;
		struct tm	xStartTM;
		struct tm	xEndTM;
	}	xDiagnostic;

}	FTDM_CFG_CLIENT, _PTR_ FTDM_CFG_CLIENT_PTR;

FTM_RET	FTDMC_initConfig(FTDM_CFG_CLIENT_PTR pConfig);
FTM_RET	FTDMC_loadConfig(FTDM_CFG_CLIENT_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDMC_finalConfig(FTDM_CFG_CLIENT_PTR pConfig);

#endif

