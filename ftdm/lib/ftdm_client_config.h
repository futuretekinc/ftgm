#ifndef	__FTDM_CLIENT_CFG_H__
#define	__FTDM_CLIENT_CFG_H__

#include <time.h>
#include "ftm.h"

#define	FTDM_CLIENT_SERVER_IP_LEN	32

#define	FTDM_DEFAULT_SERVER_IP		"127.0.0.1"
#define	FTDM_DEFAULT_SERVER_PORT	8888

typedef	struct
{
	struct
	{
		FTM_CHAR	pServerIP[FTDM_CLIENT_SERVER_IP_LEN];
		FTM_USHORT	usPort;
		FTM_BOOL	bAutoConnect;
	}	xNetwork;

	struct
	{
		FTM_LIST	xEPList;
		FTM_TIME	xStartTM;
		FTM_TIME	xEndTM;
	}	xDiagnostic;

}	FTDM_CLIENT_CFG, _PTR_ FTDM_CLIENT_CFG_PTR;

FTM_RET	FTDM_CLIENT_CFG_init(FTDM_CLIENT_CFG_PTR pConfig);
FTM_RET	FTDM_CLIENT_CFG_load(FTDM_CLIENT_CFG_PTR pConfig, FTM_CHAR_PTR pFileName);
FTM_RET	FTDM_CLIENT_CFG_final(FTDM_CLIENT_CFG_PTR pConfig);

#endif

