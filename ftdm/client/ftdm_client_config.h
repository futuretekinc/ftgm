#ifndef	__FTDMC_CFG_H__
#define	__FTDMC_CFG_H__

#include <time.h>
#include "ftm.h"

#define	FTDMC_SERVER_IP_LEN	32

#define	FTDM_DEFAULT_SERVER_IP		"127.0.0.1"
#define	FTDM_DEFAULT_SERVER_PORT	8888

typedef	struct
{
	struct
	{
		FTM_CHAR	pServerIP[FTDMC_SERVER_IP_LEN];
		FTM_USHORT	usPort;
		FTM_BOOL	bAutoConnect;
	}	xNetwork;

	struct
	{
		FTM_LIST	xEPList;
		struct tm	xStartTM;
		struct tm	xEndTM;
	}	xDiagnostic;

}	FTDMC_CFG, _PTR_ FTDMC_CFG_PTR;

FTM_RET	FTDMC_CFG_init(FTDMC_CFG_PTR pConfig);
FTM_RET	FTDMC_CFG_load(FTDMC_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDMC_CFG_final(FTDMC_CFG_PTR pConfig);

#endif

