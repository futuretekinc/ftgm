#ifndef	_FTOM_AZURE_CLIENT_H_
#define	_FTOM_AZURE_CLIENT_H_


#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"

#include "ftom.h"

#define	FTOM_AZURE_SHARED_ACCESS_KEY_LEN	64

typedef	struct
{
	FTM_CHAR	pHostName[FTM_HOST_LEN + 1];
	FTM_CHAR	pDeviceID[FTM_ID_LEN + 1];
	FTM_CHAR	pSharedAccessKey[FTOM_AZURE_SHARED_ACCESS_KEY_LEN + 1];
}	FTOM_AZURE_CLIENT_CONFIG, _PTR_ FTOM_AZURE_CLIENT_CONIG_PTR;

typedef struct
{
	FTOM_AZURE_CLIENT_CONFIG	xConfig;
	IOTHUB_CLIENT_LL_HANDLE		hClient;

}	FTOM_AZURE_CLIENT, _PTR_ FTOM_AZURE_CLIENT_PTR;

FTM_RET	FTOM_AZURE_CLIENT_create(FTOM_AZURE_CLIENT_PTR _PTR_ ppClient);
FTM_RET	FTOM_AZURE_CLIENT_destroy(FTOM_AZURE_CLIENT_PTR _PTR_ ppClient);
FTM_RET	FTOM_AZURE_CLIENT_loadConfigFromFile
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pConfigFileName
);

FTM_RET	FTOM_AZURE_CLIENT_showConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_start
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_stop
(
	FTOM_AZURE_CLIENT_PTR	pClient
);


FTM_RET	FTOM_AZURE_CLIENT_waitingForFinished
(
	FTOM_AZURE_CLIENT_PTR	pClient
);

FTM_RET	FTOM_AZURE_CLIENT_connect
(
	FTOM_AZURE_CLIENT_PTR	pClient
);


#endif

