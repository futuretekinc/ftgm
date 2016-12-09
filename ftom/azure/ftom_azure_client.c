#include <stdio.h>
#include <stdlib.h>

#include "ftom_azure_client.h"
#include "ftm_mem.h"

FTM_CHAR_PTR	pConnectionString="HostName=ftiot2.azure-devices.net;DeviceId=Device1;SharedAccessKey=qybocP7TqAF4Sc8V+ezRVGiUF5NNIQcadkkRgvfyq0w=";
static
FTOM_AZURE_CLIENT_CONFIG	xClientDefaultConfig = 
{
	.pHostName = "ftiot2.azure-devices.net",
	.pDeviceID = "Device1",
	.pSharedAccessKey = "qybocP7TqAF4Sc8V+ezRVGiUF5NNIQcadkkRgvfyq0w="
};


FTM_RET	FTOM_AZURE_CLIENT_create
(	
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_AZURE_CLIENT_PTR	pClient;

	pClient= (FTOM_AZURE_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_AZURE_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	*ppClient = pClient;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_destroy
(
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_loadConfigFromFile
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pConfigFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfigFileName != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_saveConfigToFile
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pConfigFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfigFileName != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_showConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_start
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_stop
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_waitingForFinished
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_serverSyncStart
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_BOOL				bAutoRegister
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_connect
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{

	FTM_CHAR	pConnectionString[256];

	sprintf(pConnectionString, 
			"HostName=%s;DeviceId=%s;SharedAccessKey=%s",
			pClient->xConfig.pHostName,
			pClient->xConfig.pDeviceID,
			pClient->xConfig.pSharedAccessKey);


	pClient->hClient = IoTHubClient_LL_CreateFromConnectionString(pConnectionString, MQTT_Protocol);
	if (pClient->hClient == NULL)
	{
		printf("ERROR: hClient is NULL!\n");	
	}

	return	0;
}


