#include <stdio.h>
#include <stdlib.h>

#include "azureiot/iothub_client.h"
#include "azureiot/iothub_message.h"
#include "azureiot/inc/azure_c_shared_utility/platform.h"
#include "azureiot/iothubtransportmqtt.h"


#include "ftom_azure_client.h"
#include "ftm_mem.h"
#include "ftom_json.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

typedef	struct
{
	IOTHUB_MESSAGE_HANDLE	hMessage;
	FTM_ULONG				ulTrackingID;
	FTM_VOID_PTR			pData;
	FTM_CHAR				pMsg[];
}	FTOM_AZURE_CLIENT_TRANS, _PTR_ FTOM_AZURE_CLIENT_TRANS_PTR;

static
FTM_RET	FTOM_AZURE_CLIENT_TRANS_create
(
	FTM_CHAR_PTR	pMsg,
	FTM_VOID_PTR	pData,
	FTOM_AZURE_CLIENT_TRANS_PTR	_PTR_ ppMsg
);

static
FTM_RET	FTOM_AZURE_CLIENT_TRANS_destroy
(
	FTOM_AZURE_CLIENT_TRANS_PTR	_PTR_ ppTrans
);

static
FTM_RET	FTOM_AZURE_CLIENT_sendEPData
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				ulDataCount
);

static
FTM_VOID FTOM_AZURE_CLIENT_sendConfirmationCB
(
 	IOTHUB_CLIENT_CONFIRMATION_RESULT xResult, 
	FTM_VOID_PTR	pData
);

static
FTM_VOID	FTOM_AZURE_CLIENT_connectionStatusCB
(
	IOTHUB_CLIENT_CONNECTION_STATUS 		xResult, 
	IOTHUB_CLIENT_CONNECTION_STATUS_REASON 	xReason, 
	FTM_VOID_PTR							pUserContextCallback
);

static 
IOTHUBMESSAGE_DISPOSITION_RESULT FTOM_AZURE_CLIENT_receiveCB
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
FTOM_AZURE_CLIENT_CONFIG	_xDefaultConfig =
{
	.bTraceON = FTM_FALSE,
	.ulLoopInterval = 1000,	// 1000ms
	.ulReconnectionInterval = 5000
};

FTM_RET	FTOM_AZURE_CLIENT_create
(	
	FTM_CHAR_PTR	pName,
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

	memcpy(&pClient->xConfig, &_xDefaultConfig, sizeof(FTOM_AZURE_CLIENT_CONFIG));
	strncpy(pClient->xConfig.pName, pName, sizeof(pClient->xConfig.pName) - 1);

	xRet = FTM_LIST_create(&pClient->pCompletionWaitingList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The event list creation failed.\n");
		goto error;	
	}

	xRet = FTOM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create message queue!\n");
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

	if (pClient != NULL)
	{
		if (pClient->pMsgQ != NULL)
		{
			FTOM_MSGQ_destroy(&pClient->pMsgQ);	
		}

		if (pClient->pCompletionWaitingList != NULL)
		{
			FTM_LIST_destroy(pClient->pCompletionWaitingList);	
		}

		FTM_MEM_free(pClient);
	}

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

	if ((*ppClient)->pMsgQ != NULL)
	{
		FTOM_MSGQ_destroy(&(*ppClient)->pMsgQ);	
	}

	if ((*ppClient)->pCompletionWaitingList != NULL)
	{
		xRet = FTM_LIST_destroy((*ppClient)->pCompletionWaitingList);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "List destruction failed.\n");	
		}
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

	FTM_TIME_getCurrent(&pClient->xStats.xSend.xLastCompletedTime);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_final
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTOM_AZURE_CLIENT_TRANS_PTR	pTrans;

	while(FTM_LIST_getFirst(pClient->pCompletionWaitingList, (FTM_VOID_PTR _PTR_)&pTrans) == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pCompletionWaitingList, pTrans);
		FTOM_AZURE_CLIENT_TRANS_destroy(&pTrans);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_CONFIG_load
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

	xRet = FTM_CONFIG_getItem(pConfig, pClient->xConfig.pName, &xSection);
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

FTM_RET	FTOM_AZURE_CLIENT_CONFIG_save
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pClient!= NULL);
	ASSERT(pConfig != NULL);

	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xSection;

	xRet = FTM_CONFIG_getItem(pConfig, pClient->xConfig.pName, &xSection);
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


FTM_RET	FTOM_AZURE_CLIENT_CONFIG_show
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("[ Azure Client ]\n");
	MESSAGE("%16s : %s\n", "Host Name", pClient->xConfig.pHostName);
	MESSAGE("%16s : %s\n", "Device ID", pClient->xConfig.pDeviceID);
	MESSAGE("%16s : %s\n", "Key", pClient->xConfig.pSharedAccessKey);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_setNotifyCB
(
	FTOM_AZURE_CLIENT_PTR		pClient,
	FTOM_CLIENT_NOTIFY_CB		fNotifyCB,
	FTM_VOID_PTR				pData
)
{
	ASSERT(pClient != NULL);

	pClient->fNotifyCB = fNotifyCB;
	pClient->pNotifyData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_start
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;


	if (pClient->xThreadMain != 0)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	pClient->bStop = FTM_FALSE;

	if (pthread_create(&pClient->xThreadMain, NULL, FTOM_AZURE_CLIENT_threadMain, pClient) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The client azure connection task creation failed!\n");

		return	xRet;
	}

	
	return	FTM_RET_OK;

}

FTM_RET	FTOM_AZURE_CLIENT_stop
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->xThreadMain == 0)
	{
		return	FTM_RET_NOT_START;	
	}

	pClient->bStop = FTM_TRUE;

	return	FTOM_AZURE_CLIENT_waitingForFinished(pClient);
}

FTM_RET	FTOM_AZURE_CLIENT_isRunning
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIsRunning != NULL);

	*pIsRunning = (pClient->xThreadMain != 0);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_waitingForFinished
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	pthread_join(pClient->xThreadMain, NULL);
	pClient->xThreadMain = 0;

	return	FTM_RET_OK;
}

FTM_RET FTOM_AZURE_CLIENT_MESSAGE_send
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pClient != NULL);

	return	FTOM_MSGQ_push(pClient->pMsgQ, pBaseMsg);
}

FTM_RET	FTOM_AZURE_CLIENT_MESSAGE_process
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
		return	FTM_RET_INVALID_MESSAGE_TYPE;
	}

	FTOM_MSG_destroy(&pBaseMsg);

	return	FTM_RET_OK; 
}

FTM_RET	FTOM_AZURE_CLIENT_connect
(
	FTOM_AZURE_CLIENT_PTR	pClient
)
{
	FTM_CHAR	pConnectionString[256];

	sprintf(pConnectionString, "HostName=%s;DeviceId=%s;SharedAccessKey=%s",
			pClient->xConfig.pHostName,
			pClient->xConfig.pDeviceID,
			pClient->xConfig.pSharedAccessKey);

	pClient->hAzureClient = IoTHubClient_CreateFromConnectionString(pConnectionString, MQTT_Protocol);
	if (pClient->hAzureClient == NULL)
	{
		ERROR2(FTM_RET_ERROR, "Failed to connect to azure.\n");
		return	FTM_RET_ERROR;
	}

	if (IoTHubClient_SetConnectionStatusCallback(pClient->hAzureClient, FTOM_AZURE_CLIENT_connectionStatusCB, pClient) != IOTHUB_CLIENT_OK)
	{
		TRACE("Failed to set connection status callback!\n");
	}

	if (IoTHubClient_SetMessageCallback(pClient->hAzureClient, FTOM_AZURE_CLIENT_receiveCB, pClient) != IOTHUB_CLIENT_OK)
	{
		TRACE("Failed to set message callback!\n");
	}

	TRACE("Azure connected![%08x]\n", pClient->hAzureClient);
	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_AZURE_CLIENT_threadMain
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	FTM_TIMER	xLoopTimer;
	FTOM_AZURE_CLIENT_PTR	pClient = (FTOM_AZURE_CLIENT_PTR)pData;
	FTM_ULONG	ulLoopInterval;

	if (platform_init() != 0)
	{
		TRACE("Platform initialize failed!\n");	
		return	0;
	}

	FTM_TIMER_initMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);

	while(!pClient->bStop)
	{

        FTOM_MSG_PTR    pBaseMsg;

		FTM_TIMER_remainMS(&xLoopTimer, &ulLoopInterval);

		while (!pClient->bStop)
		{    
			xRet = FTOM_MSGQ_timedPop(pClient->pMsgQ, ulLoopInterval, &pBaseMsg);
			if (xRet != FTM_RET_OK)
			{    
				break;  
			}    

			xRet = FTOM_AZURE_CLIENT_MESSAGE_process(pClient, pBaseMsg);
			if (xRet != FTM_RET_OK)
			{   
				FTOM_MSG_destroy(&pBaseMsg);
			}   
		}    

		if (pClient->hAzureClient == NULL)		
		{
			xRet = FTOM_AZURE_CLIENT_connect(pClient);	
			if (xRet != FTM_RET_OK)
			{
				FTM_TIMER_addMS(&xLoopTimer, pClient->xConfig.ulReconnectionInterval * 1000);
			}
		}
		else
		{
			while(FTM_TIMER_isExpired(&xLoopTimer) == FTM_FALSE)
			{
				FTM_ULONG	ulWaitingEventCount = 0;
				
				FTM_LIST_count(pClient->pCompletionWaitingList, &ulWaitingEventCount);
				if (ulWaitingEventCount == 0)
				{
					break;	
				}

				IoTHubClient_LL_DoWork(pClient->hAzureClient);
				usleep(1000);
			}

			FTM_TIMER_addMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);
		}

	}

	return	0;
}

FTM_VOID	FTOM_AZURE_CLIENT_connectionStatusCB
(
	IOTHUB_CLIENT_CONNECTION_STATUS 		xResult, 
	IOTHUB_CLIENT_CONNECTION_STATUS_REASON 	xReason, 
	FTM_VOID_PTR							pUserContextCallback
)
{
	if (xResult == IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)
	{
		switch(xReason)
		{
		case	IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
			WARN2(FTM_RET_ERROR, "The client connection expired sas token.\n");
			break;

		case	IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
			WARN2(FTM_RET_ERROR, "The client connection device disabled.\n");
			break;

		case	IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
			WARN2(FTM_RET_ERROR, "The client connection bad credential.\n");
			break;

		case	IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED: 
			WARN2(FTM_RET_ERROR, "The client connection retry expired.\n");
			break;

		case	IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
			WARN2(FTM_RET_ERROR, "The client connection eno network.\n");
			break;

		case	IOTHUB_CLIENT_CONNECTION_OK:
			WARN2(FTM_RET_ERROR, "The client connection ok.\n");
			break;

		}
	}
}

IOTHUBMESSAGE_DISPOSITION_RESULT FTOM_AZURE_CLIENT_receiveCB
(
	IOTHUB_MESSAGE_HANDLE 	hMessage, 
	FTM_VOID_PTR			pData
)
{
	const FTM_CHAR_PTR pBuffer;
	size_t size;

	if (IoTHubMessage_GetByteArray(hMessage, (const unsigned char **)&pBuffer, &size) != IOTHUB_MESSAGE_OK)
	{   
		ERROR2(FTM_RET_ERROR, "unable to retrieve the message data.\n");
	}   

	TRACE("Recv Msg[%s]!\n", pBuffer);

	// Retrieve properties from the message
	MAP_HANDLE mapProperties = IoTHubMessage_Properties(hMessage);
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


FTM_VOID FTOM_AZURE_CLIENT_sendConfirmationCB
(
 	IOTHUB_CLIENT_CONFIRMATION_RESULT xResult, 
	FTM_VOID_PTR	pData
)
{
	FTOM_AZURE_CLIENT_TRANS_PTR	pTrans = (FTOM_AZURE_CLIENT_TRANS_PTR)pData;
	FTOM_AZURE_CLIENT_PTR		pClient= (FTOM_AZURE_CLIENT_PTR)pTrans->pData;
	FTM_ULONG	ulRemainCount = 0;

	FTM_TIME_getCurrent(&pClient->xStats.xSend.xLastCompletedTime);

	FTM_LIST_remove(pClient->pCompletionWaitingList, pTrans);
	FTM_LIST_count(pClient->pCompletionWaitingList, &ulRemainCount);

	++pClient->xStats.xSend.ulCompleted;

	FTOM_AZURE_CLIENT_TRANS_destroy(&pTrans);
}

FTM_RET	FTOM_AZURE_CLIENT_sendToAzure
(
	FTOM_AZURE_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet;
	FTOM_AZURE_CLIENT_TRANS_PTR	pTrans;

	xRet = FTOM_AZURE_CLIENT_TRANS_create(pMsg, pClient, &pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create azure event!\n");
		return	xRet;
	}

	if (IoTHubClient_SendEventAsync(pClient->hAzureClient, pTrans->hMessage, FTOM_AZURE_CLIENT_sendConfirmationCB, pTrans) != IOTHUB_CLIENT_OK)
	{   
		FTOM_AZURE_CLIENT_TRANS_destroy(&pTrans);

		xRet = FTM_RET_ERROR;
		ERROR2(xRet, "Failed to send event async to azure!\n");

		return	xRet;
	}   

	pTrans->ulTrackingID = ++pClient->xStats.xSend.ulTotal;

	FTM_LIST_append(pClient->pCompletionWaitingList, pTrans);

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

	xRet = FTOM_AZURE_CLIENT_sendToAzure(pClient, FTOM_JSON_print(pJSON));
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
		IoTHubClient_SetOption(pClient->hAzureClient, "logtrace", &pClient->xConfig.bTraceON);
	}

	MESSAGE("TRACE : %s\n", (bTraceON)?"ON":"OFF");

	return	FTM_RET_OK;
}


FTM_RET	FTOM_AZURE_CLIENT_TRANS_create
(
	FTM_CHAR_PTR	pMsg,
	FTM_VOID_PTR	pData,
	FTOM_AZURE_CLIENT_TRANS_PTR	_PTR_ ppTrans
)
{
	ASSERT(pMsg != NULL);
	ASSERT(ppTrans != NULL);
	FTM_RET	xRet;	
	FTOM_AZURE_CLIENT_TRANS_PTR	pTrans;
	IOTHUB_MESSAGE_HANDLE		hMessage;

	hMessage = IoTHubMessage_CreateFromByteArray((const FTM_UINT8_PTR)pMsg, strlen(pMsg));
	if (hMessage == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Failed to create IoTHub message!\n");
		return	xRet;
	}

	pTrans = (FTOM_AZURE_CLIENT_TRANS_PTR)FTM_MEM_malloc(sizeof(FTOM_AZURE_CLIENT_TRANS) + strlen(pMsg) + 1);
	if (pTrans == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pTrans->pData = pData;
	pTrans->hMessage = hMessage;
	strcpy(pTrans->pMsg, pMsg);

	*ppTrans = pTrans;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_CLIENT_TRANS_destroy
(
	FTOM_AZURE_CLIENT_TRANS_PTR	_PTR_ ppTrans
)
{
	ASSERT(ppTrans != NULL);
	
	IoTHubMessage_Destroy((*ppTrans)->hMessage);

	FTM_MEM_free(*ppTrans);

	*ppTrans = NULL;

	return	FTM_RET_OK;

}

