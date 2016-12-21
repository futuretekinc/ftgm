#ifndef	__FTOM_CLIENT_H__
#define	__FTOM_CLIENT_H__

#include "ftdm_types.h"
#include "ftdm_cmd.h"
#include "ftdm_params.h"
#include "ftdm_client_config.h"

typedef	struct
{
	FTM_ULONG	ulID;
}	FTOM_FINS_CFG, _PTR_ FTOM_FINS_CFG_PTR;

typedef struct 
{
	FTM_INT		hSock;
	FTM_INT		nTimeout;
	FTM_LOCK	xLock;
}	FTOM_FINS_SESSION, _PTR_ FTOM_FINS_SESSION_PTR;

FTM_RET	FTOM_FINS_init
(
	FTOM_FINS_CFG_PTR pConfig
);

FTM_RET FTOM_FINS_final
(
	FTM_VOID
);

FTM_RET FTOM_FINS_connect
(
	FTOM_FINS_SESSION_PTR 	pSession,
	FTM_IP_ADDR				xIP,
	FTM_USHORT 				usPort 
);

FTM_RET FTOM_FINS_disconnect
(
	FTOM_FINS_SESSION_PTR	pSession
);

FTM_RET FTOM_FINS_isConnected
(
	FTOM_FINS_SESSION_PTR	pSession,
	FTM_BOOL_PTR			pbConnected
);

#endif

