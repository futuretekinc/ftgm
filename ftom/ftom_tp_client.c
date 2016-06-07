#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "nxjson.h"
#include "ftom_tp_client.h"
#include "ftom_mqtt_client.h"
#include "ftom_mqtt_client_tpgw.h"

static 
FTM_VOID_PTR FTOM_TP_CLIENT_process
(
	FTM_VOID_PTR 		pData
);

static
FTM_RET	FTOM_TP_CLIENT_pushMsg
(
	FTOM_TP_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
);

FTM_RET	FTOM_TP_CLIENT_create
(
	FTOM_TP_CLIENT_PTR _PTR_ 	ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET				xRet;
	FTOM_TP_CLIENT_PTR	pClient;

	pClient = (FTOM_TP_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_MQTT_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_TP_CLIENT_init(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client initialization was failed.\n");	
		FTM_MEM_free(pClient);
	}
	else
	{
		*ppClient = pClient;	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_destroy
(
	FTOM_TP_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_TP_CLIENT_final(*ppClient);

	FTM_MEM_free(*ppClient);
	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_init
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;	
	if (pClient->pMsgQ != NULL)
	{
		ERROR("Already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	memset(pClient, 0, sizeof(FTOM_MQTT_CLIENT));

	FTOM_getDID(pClient->pDID, FTM_DID_LEN);

	strcpy(pClient->xConfig.pHost, FTOM_TP_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.usPort = FTOM_TP_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulReconnectionTime = FTOM_TP_CLIENT_DEFAULT_RECONNECTION_TIME;
	pClient->bStop = FTM_TRUE;

	xRet = FTOM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		goto error;	
	}

	return	FTM_RET_OK;

error:
	if (pClient->pMsgQ != NULL)
	{
		FTOM_MSGQ_destroy(&pClient->pMsgQ);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->pMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTOM_TP_CLIENT_stop(pClient);

	FTOM_MSGQ_destroy(&pClient->pMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_TP_CLIENT_CONFIG_PTR 	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_TP_CLIENT_CONFIG));

	return	FTM_RET_OK;
}


FTM_RET	FTOM_TP_CLIENT_loadFromFile
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet !=  FTM_RET_OK)
	{
		ERROR("Configration loading failed!\n");
		return	xRet;	
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "tpclient", &xSection);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "cert", pClient->xConfig.pCertFile, FTM_FILE_NAME_LEN);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "host", pClient->xConfig.pHost, FTM_HOST_LEN);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	
		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &pClient->xConfig.usPort);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "apikey", pClient->xConfig.pAPIKey, FTM_PASSWD_LEN);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	
	}

	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_showConfig
(
	FTOM_TP_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("\n[ ThingPlus Client Configuration ]\n");
	MESSAGE("%16s : %s\n", "Host", pClient->xConfig.pHost);
	MESSAGE("%16s : %d\n", "Port", pClient->xConfig.usPort);
	MESSAGE("%16s : %s\n", "API Key", pClient->xConfig.pAPIKey);
	MESSAGE("%16s : %s\n", "Cert File", pClient->xConfig.pCertFile);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_notify
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pClient != NULL);

	switch(pMsg->xType)
	{
	case	FTOM_MSG_TYPE_SEND_EP_DATA:
		{	
			
			//FTOM_TP_CLIENT_onSendEPData(pClient, (FTOM_MSG_SEND_EP_DATA_PTR)pMsg);
		}
		break;

	default:
		{
			ERROR("Not supported message[%08x]!\n", pMsg->xType);	
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_start
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	if (pClient->bStop != FTM_TRUE)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	pthread_create(&pClient->xMain, NULL, FTOM_TP_CLIENT_process, pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_stop
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	if (pClient->bStop == FTM_TRUE)
	{
		return	FTM_RET_NOT_START;	
	}

	pClient->bStop = FTM_TRUE;

	pthread_join(pClient->xMain, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_TP_CLIENT_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTOM_MQTT_CLIENT_CONFIG	xMQTTConfig;

	pClient->bConnected = FTM_FALSE;

	strncpy(xMQTTConfig.pHost, pClient->xConfig.pHost, FTM_HOST_LEN);
	xMQTTConfig.usPort = pClient->xConfig.usPort;
	strncpy(xMQTTConfig.pUserID, pClient->pDID, FTM_USER_ID_LEN);
	strncpy(xMQTTConfig.pPasswd, pClient->xConfig.pAPIKey, FTM_PASSWD_LEN);
	strncpy(xMQTTConfig.pCertFile, pClient->xConfig.pCertFile, FTM_FILE_NAME_LEN);
	xMQTTConfig.ulReconnectionTime = pClient->xConfig.ulReconnectionTime;
	xMQTTConfig.bTLS = FTM_TRUE;
	xMQTTConfig.ulCBSet = 1;

	xRet = FTOM_MQTT_CLIENT_create(&pClient->pMQTTClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client creation failed!\n");
		return	0;
	}

	xRet = FTOM_MQTT_CLIENT_loadConfig(pClient->pMQTTClient, &xMQTTConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client configation loading failed!\n");
		return	0;	
	}

	pClient->bStop 		= FTM_FALSE;
	TRACE("TPClient[%s] started.\n", pClient->pDID);

	FTOM_MQTT_CLIENT_start(pClient->pMQTTClient);
	FTM_TIMER_init(&pClient->xReconnectionTimer, pClient->xConfig.ulReconnectionTime * 1000000);

	while(!pClient->bStop)
	{
		FTOM_MSG_PTR	pMsg;

		xRet = FTOM_MSGQ_timedPop(pClient->pMsgQ, 1000000, &pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			default:
				{
					ERROR("Not supported msg[%08x]\n", pMsg->xType);	
				}
			}
			FTOM_MSG_destroy(&pMsg);
		}

		if (FTM_TIMER_isExpired(&pClient->xReconnectionTimer))
		{
			TRACE("TPClient reconnection !\n");
			FTM_TIMER_addSeconds(&pClient->xReconnectionTimer, pClient->xConfig.ulReconnectionTime);
		}
	}

	FTOM_MQTT_CLIENT_stop(pClient->pMQTTClient);
	FTOM_MQTT_CLIENT_destroy(&pClient->pMQTTClient);
	TRACE("TPClient[%s] stopped.\n", pClient->pDID);

	return 0;
}

FTM_RET	FTOM_TP_CLIENT_pushMsg
(
	FTOM_TP_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(pClient->pMsgQ, pMsg);
}


