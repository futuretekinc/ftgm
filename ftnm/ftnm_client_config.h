#ifndef	__FTNM_CFG_CLIENT_H__
#define	__FTNM_CFG_CLIENT_H__

#include "ftm_types.h"
#include <time.h>

#define	FTNMC_SERVER_IP_LEN	32

#define	FTNM_DEFAULT_SERVER_IP		"127.0.0.1"
#define	FTNM_DEFAULT_SERVER_PORT	8888

typedef	struct
{
	struct
	{
		FTM_CHAR	pServerIP[FTNMC_SERVER_IP_LEN];
		FTM_USHORT	usPort;
	}	xNetwork;

}	FTNM_CFG_CLIENT, _PTR_ FTNM_CFG_CLIENT_PTR;

FTM_RET	FTNMC_initConfig(FTNM_CFG_CLIENT_PTR pConfig);
FTM_RET	FTNMC_loadConfig(FTNM_CFG_CLIENT_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTNMC_finalConfig(FTNM_CFG_CLIENT_PTR pConfig);

#endif

