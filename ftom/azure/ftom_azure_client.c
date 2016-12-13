#include <stdio.h>
#include <stdlib.h>

#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"


#include "ftom_azure_client.h"
#include "ftm_mem.h"
#include "ftom_client_net.h"
#include "ftom_json.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

typedef	struct
{
	FTOM_AZURE_CLIENT_PTR	pClient;
	IOTHUB_MESSAGE_HANDLE	hMessage;
	FTM_ULONG				ulTrackingID;
	FTM_CHAR				pMsg[];
}	FTOM_AZURE_CLIENT_EVENT, _PTR_ FTOM_AZURE_CLIENT_EVENT_PTR;

static
FTM_RET	FTOM_AZURE_CLIENT_sendEPData
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				ulDataCount
);

static 
IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCB
(
	IOTHUB_MESSAGE_HANDLE 	message, 
	FTM_VOID_PTR			pData
);

static
FTM_VOID_PTR	FTOM_AZURE_CLIENT_threadMain
(
	FTM_VOID_PTR	pData
);

static
FTOM_CLIENT_FUNCTION_SET	_xFunctionSet =
{
	.fInit  = (FTOM_CLIENT_INIT)FTOM_AZURE_CLIENT_init,
	.fFinal	= (FTOM_CLIENT_FINAL)FTOM_AZURE_CLIENT_final,

	.fStart = (FTOM_CLIENT_START)FTOM_AZURE_CLIENT_start,

	.fLoadConfig = (FTOM_CLIENT_LOAD_CONFIG)FTOM_AZURE_CLIENT_loadConfig,
	.fSaveConfig = (FTOM_CLIENT_SAVE_CONFIG)FTOM_AZURE_CLIENT_saveConfig,
	.fShowConfig = (FTOM_CLIENT_SHOW_CONFIG)FTOM_AZURE_CLIENT_showConfig,
	.fMessageProcess= (FTOM_CLIENT_MESSAGE)FTOM_AZURE_CLIENT_messageProcess
};

static	FTOM_AZURE_CLIENT_CONFIG	_xDefaultConfig =
{
	.bTraceON = FTM_FALSE,
	.ulLoopInterval = 1000,	// 1000ms
	.ulReconnectionInterval = 5000
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

	memcpy(&pClient->xConfig, &_xDefaultConfig, sizeof(FTOM_AZURE_CLIENT_CONFIG));

	xRet = FTM_LIST_create(&pClient->pEventList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The event list creation failed.\n");
		goto error;	
	}

	xRet = FTOM_AZURE_CLIENT_init(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The azclient initialization was failed!\n");
		FTM_MEM_free(pClient);

		return	xRet;
	}

	*ppClient = pClient;

	return	FTM_RET_OK;

error:

	return	xRet;
}

FTM_RET	FTOM_AZURE_CLIENT_destroy
(
	FTOM_AZURE_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	FTM_RET	xRet;

	xRet = FTOM_AZURE_CLIENT_final(*ppClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Finalize was failed.\n");	
	}

	xRet = FTM_LIST_destroy((*ppClient)->pEventList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "List destruction failed.\n");	
	}

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

	FTM_TIME_getCurrent(&pClient->xStats.xSend.xLastCompletedTime);

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

FTM_RET	FTOM_AZURE_CLIENT_start
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	xRet = FTOM_CLIENT_NET_start((FTOM_CLIENT_NET_PTR)pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start net client!\n");
		return	xRet;	
	}

	if (pthread_create(&pClient->xThreadMain, NULL, FTOM_AZURE_CLIENT_threadMain, pClient) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The client azure connection task creation failed!\n");

		return	xRet;
	}

	
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

FTM_RET	FTOM_AZURE_CLIENT_messageProcess
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
)
{
	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_EP_DATA:
		{
			FTOM_MSG_EP_DATA_PTR	pMsg = (FTOM_MSG_EP_DATA_PTR)pBaseMsg;

			FTOM_AZURE_CLIENT_sendEPData(pClient,pMsg->pEPID, pMsg->pData, pMsg->ulCount);
		}
		break;

	default:
		return	FTOM_CLIENT_NET_messageProcess((FTOM_CLIENT_NET_PTR)pClient, pBaseMsg);
	}
	return	FTM_RET_OK; 
}

FTM_RET	FTOM_AZURE_CLIENT_connect
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{

	FTM_CHAR	pConnectionString[256];

	if (platform_init() != 0)
	{
		TRACE("Platform initialize failed!\n");	
		return	FTM_RET_ERROR;
	}

	sprintf(pConnectionString, 
			"HostName=%s;DeviceId=%s;SharedAccessKey=%s",
			pClient->xConfig.pHostName,
			pClient->xConfig.pDeviceID,
			pClient->xConfig.pSharedAccessKey);


	TRACE("CONNECTION STRING : %s\n", pConnectionString);
	pClient->hAzureClient = IoTHubClient_LL_CreateFromConnectionString(pConnectionString, MQTT_Protocol);
	if (pClient->hAzureClient == NULL)
	{
		printf("ERROR: hClient is NULL!\n");	
		return	FTM_RET_ERROR;
	}

	IoTHubClient_LL_SetOption(pClient->hAzureClient, "logtrace", &pClient->xConfig.bTraceON);

	if (IoTHubClient_LL_SetMessageCallback(pClient->hAzureClient, ReceiveMessageCB, pClient) != IOTHUB_CLIENT_OK)
	{
		TRACE("Failed to set message callback!\r\n");
	}

	TRACE("Azure connected![%08x]\n", pClient->hAzureClient);
	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_AZURE_CLIENT_threadMain
(
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_TIMER	xLoopTimer;

	FTOM_AZURE_CLIENT_PTR	pClient = (FTOM_AZURE_CLIENT_PTR)pData;

	FTM_TIMER_initMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);

	pClient->bStop = FTM_FALSE;

	while(!pClient->bStop)
	{

		if (pClient->hAzureClient == NULL)		
		{
			xRet = FTOM_AZURE_CLIENT_connect(pClient);	
			if (xRet != FTM_RET_OK)
			{
				FTM_TIMER_waitForExpired(&xLoopTimer);
				FTM_TIMER_addMS(&xLoopTimer, pClient->xConfig.ulReconnectionInterval * 1000);
			}
		}
		else
		{
			while(FTM_TIMER_isExpired(&xLoopTimer) == FTM_FALSE)
			{
				FTM_ULONG	ulWaitingEventCount = 0;
				
				FTM_LIST_count(pClient->pEventList, &ulWaitingEventCount);
				if (ulWaitingEventCount == 0)
				{
					break;	
				}

				IoTHubClient_LL_DoWork(pClient->hAzureClient);
				usleep(1000);
			}

			FTM_TIMER_waitForExpired(&xLoopTimer);
			FTM_TIMER_addMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);
		}

	}
	return	0;
}

static 
IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCB
(
	IOTHUB_MESSAGE_HANDLE 	message, 
	FTM_VOID_PTR			pData
)
{
	const FTM_CHAR_PTR pBuffer;
	size_t size;

	TRACE("Recv Msg!\n");
	if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&pBuffer, &size) != IOTHUB_MESSAGE_OK)
	{   
		ERROR2(FTM_RET_ERROR, "unable to retrieve the message data.\n");
	}   

	// Retrieve properties from the message
	MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
	if (mapProperties != NULL)
	{   
		const char*const* keys;
		const char*const* values;
		size_t propertyCount = 0;
		if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
		{   
			if (propertyCount > 0)
			{   
				for (size_t index = 0; index < propertyCount; index++)
				{   
					(void)printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
				}   
				(void)printf("\r\n");
			}   
		}   
	}   

	return IOTHUBMESSAGE_ACCEPTED;
}


static 
FTM_VOID FTOM_AZURE_CLIENT_sendConfirmationCB
(
 	IOTHUB_CLIENT_CONFIRMATION_RESULT xResult, 
	FTM_VOID_PTR	pData
)
{
	FTOM_AZURE_CLIENT_EVENT_PTR	pEvent = (FTOM_AZURE_CLIENT_EVENT_PTR)pData;
	FTOM_AZURE_CLIENT_PTR		pClient= pEvent->pClient;
	FTM_ULONG	ulRemainCount = 0;

	FTM_TIME_getCurrent(&pClient->xStats.xSend.xLastCompletedTime);

	FTM_LIST_remove(pClient->pEventList, pEvent);
	FTM_LIST_count(pClient->pEventList, &ulRemainCount);

	++pClient->xStats.xSend.ulCompleted;

	IoTHubMessage_Destroy(pEvent->hMessage);

	FTM_MEM_free(pEvent);
}

FTM_RET	FTOM_AZURE_CLIENT_sendMessage
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet;
	FTOM_AZURE_CLIENT_EVENT_PTR	pEvent ;

	pEvent = (FTOM_AZURE_CLIENT_EVENT_PTR)FTM_MEM_malloc(sizeof(FTOM_AZURE_CLIENT_EVENT) + strlen(pMsg) + 1);
	if (pEvent == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_RET_OK;

	pEvent->pClient = pClient;
	strcpy(pEvent->pMsg, pMsg);

	pEvent->hMessage = IoTHubMessage_CreateFromByteArray((const FTM_UINT8_PTR)pEvent->pMsg, strlen(pEvent->pMsg));
	if (pEvent->hMessage == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "iotHubMessageHandle is NULL!\n");
	}
	else
	{  
		pEvent->ulTrackingID = pClient->xStats.xSend.ulTotal+1;

		if (IoTHubClient_LL_SendEventAsync(pClient->hAzureClient, pEvent->hMessage, FTOM_AZURE_CLIENT_sendConfirmationCB, pEvent) != IOTHUB_CLIENT_OK)
		{   
			xRet = FTM_RET_ERROR;
			ERROR2(xRet, "Failed to send event async to azure!\n");
		}   
		else
		{
			pClient->xStats.xSend.ulTotal++;
		}
	}   

	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEvent);
	}
	else
	{
		FTM_LIST_append(pClient->pEventList, pEvent);
	}

	return	xRet;
}

FTM_RET	FTOM_AZURE_CLIENT_sendEPData
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				ulDataCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_JSON_PTR	pJSON = NULL;

	xRet = FTOM_JSON_createEPData(pEPID, pData, ulDataCount, &pJSON);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to print out to json formatted string!\n");	
		return	xRet;
	}

	xRet = FTOM_AZURE_CLIENT_sendMessage(pClient, FTOM_JSON_print(pJSON));
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message to azure!\n");
	}

	if (pJSON != NULL)
	{
		FTOM_JSON_destroy(&pJSON);	
	}

	return	xRet;
}

FTM_RET	FTOM_AZURE_CLIENT_trace
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_BOOL				bTraceON
)
{
	ASSERT(pClient != NULL);

	pClient->xConfig.bTraceON = bTraceON;
	if (pClient->hAzureClient != NULL)
	{
		IoTHubClient_LL_SetOption(pClient->hAzureClient, "logtrace", &pClient->xConfig.bTraceON);
	}

	MESSAGE("TRACE : %s\n", (bTraceON)?"ON":"OFF");

	return	FTM_RET_OK;
}

