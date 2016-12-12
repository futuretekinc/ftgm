#include <stdio.h>
#include <stdlib.h>

#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"


#include "ftom_azure_client.h"
#include "ftm_mem.h"
#include "ftom_client_net.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static
FTOM_CLIENT_FUNCTION_SET	_xFunctionSet =
{
	.fInit  = (FTOM_CLIENT_INIT)FTOM_AZURE_CLIENT_init,
	.fFinal	= (FTOM_CLIENT_INIT)FTOM_AZURE_CLIENT_final,
	.fLoadConfig = (FTOM_CLIENT_LOAD_CONFIG)FTOM_AZURE_CLIENT_loadConfig,
	.fSaveConfig = (FTOM_CLIENT_SAVE_CONFIG)FTOM_AZURE_CLIENT_saveConfig,
	.fShowConfig = (FTOM_CLIENT_SHOW_CONFIG)FTOM_AZURE_CLIENT_showConfig,
};

FTM_RET	FTOM_AZURE_CLIENT_create
(	
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET	xRet;
	FTOM_AZURE_CLIENT_PTR	pClient;

	pClient= (FTOM_AZURE_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_AZURE_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pClient->xParent.xCommon.xType = FTOM_CLIENT_TYPE_AZURE;

	xRet = FTOM_AZURE_CLIENT_init(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The azclient initialization was failed!\n");
		FTM_MEM_free(pClient);

		return	xRet;
	}

	*ppClient = pClient;

	return	xRet;
}

FTM_RET	FTOM_AZURE_CLIENT_destroy
(
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_AZURE_CLIENT_final(*ppClient);

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_init
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;

	xRet = FTOM_CLIENT_NET_init(&pClient->xParent);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize net client!\n");
		return	xRet;
	}


	FTOM_CLIENT_setFunctionSet((FTOM_CLIENT_PTR)pClient, &_xFunctionSet);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_final
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	xRet = FTOM_CLIENT_NET_setNotifyCB((FTOM_CLIENT_NET_PTR)pClient, NULL, NULL);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set notify callback!\n");	
	}

	xRet = FTOM_CLIENT_NET_final(&pClient->xParent);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finaizlie net client!\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_AZURE_CLIENT_loadConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pClient!= NULL);
	ASSERT(pConfig != NULL);

	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xSection;
	FTOM_AZURE_CLIENT_CONFIG	xAzureConfig;

	memcpy(&xAzureConfig, &pClient->xConfig, sizeof(FTOM_AZURE_CLIENT_CONFIG));

	xRet = FTM_CONFIG_getItem(pConfig, "client", &xSection);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_OK;	
	}

	xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "hostname", xAzureConfig.pHostName, FTM_HOST_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not find a azure host!\n");
	}

	xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "deviceid", xAzureConfig.pDeviceID, FTM_ID_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not find a device id!\n");
	}

	xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "key", xAzureConfig.pSharedAccessKey, FTOM_AZURE_SHARED_ACCESS_KEY_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not find a shared access key!\n");	
	}

	memcpy(&pClient->xConfig, &xAzureConfig, sizeof(FTOM_AZURE_CLIENT_CONFIG));

	
	FTM_TRACE_loadConfig(pConfig);

	return	xRet;
}

FTM_RET	FTOM_AZURE_CLIENT_saveConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pClient!= NULL);
	ASSERT(pConfig != NULL);

	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "azclient", &xSection);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_OK;	
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "hostname", pClient->xConfig.pHostName);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not find a azure host!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "deviceid", pClient->xConfig.pDeviceID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not find a device id!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "key", pClient->xConfig.pSharedAccessKey);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not find a shared access key!\n");	
	}

	return	xRet;
}


FTM_RET	FTOM_AZURE_CLIENT_showConfig
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("%16s : %s\n", "Host Name", pClient->xConfig.pHostName);
	MESSAGE("%16s : %s\n", "Device ID", pClient->xConfig.pDeviceID);
	MESSAGE("%16s : %s\n", "Key", pClient->xConfig.pSharedAccessKey);

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


