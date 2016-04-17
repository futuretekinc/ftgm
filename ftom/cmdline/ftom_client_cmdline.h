#ifndef	__FTOM_CLIENT_CL_H__
#define	__FTOM_CLIENT_CL_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom_params.h"
#include "ftom_client.h"

typedef	struct
{
	FTM_CHAR_PTR		pRead[1024];
}	FTOM_CLIENT_CL_CONFIG, _PTR_ FTOM_CLIENT_CL_CONFIG_PTR;

typedef	struct
{
	FTOM_CLIENT				xCommon;

	FTOM_CLIENT_CL_CONFIG	xConfig;
}	FTOM_CLIENT_CL, _PTR_ FTOM_CLIENT_CL_PTR;

FTM_RET	FTOM_CLIENT_CL_init
(
	FTOM_CLIENT_CL_PTR	pClient
);

FTM_RET	FTOM_CLIENT_CL_final
(
	FTOM_CLIENT_CL_PTR	pClient
);

FTM_RET	FTOM_CLIENT_CL_create
(
	FTOM_CLIENT_CL_PTR _PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_CL_destroy
(
	FTOM_CLIENT_CL_PTR _PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_CL_start
(
	FTOM_CLIENT_CL_PTR	pClient
);

FTM_RET	FTOM_CLIENT_CL_stop
(
	FTOM_CLIENT_CL_PTR	pClient
);

FTM_RET	FTOM_CLIENT_CL_ReadConfig
(
	FTOM_CLIENT_CL_CONFIG_PTR 	pConfig, 
	FTM_CHAR_PTR 			pFileName
);

FTM_RET	FTOM_CLIENT_CL_loadConfig
(
	FTOM_CLIENT_CL_PTR			pClient,
	FTOM_CLIENT_CL_CONFIG_PTR	pConfig
);


FTM_RET	FTOM_CLIENT_CL_loadConfigFromFile
(
	FTOM_CLIENT_CL_PTR	pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_CLIENT_CL_setNotifyCallback
(
	FTOM_CLIENT_CL_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CALLBACK	pCB
);

FTM_RET FTOM_CLIENT_CL_request
(
	FTOM_CLIENT_CL_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
);

#endif

