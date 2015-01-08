#ifndef	__FTDM_CLIENT_CONFIG_H__
#define	__FTDM_CLIENT_CONFIG_H__

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

}	FTDM_CLIENT_CONFIG, _PTR_ FTDM_CLIENT_CONFIG_PTR;

FTM_RET	FTDMC_initConfig(FTDM_CLIENT_CONFIG_PTR pConfig);
FTM_RET	FTDMC_loadConfig(FTDM_CLIENT_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDMC_finalConfig(FTDM_CLIENT_CONFIG_PTR pConfig);

#endif

