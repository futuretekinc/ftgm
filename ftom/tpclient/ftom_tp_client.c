#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftom_client.h"
#include "ftom_client_net.h"
#include "ftom_tp_client.h"
#include "ftom_message_queue.h"
#include "ftom_mqttc.h"
#include "ftom_mqtt_client_tpgw.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static 
FTM_VOID_PTR FTOM_TP_CLIENT_threadMain
(
	FTM_VOID_PTR 		pData
);

static 
FTM_VOID_PTR FTOM_TP_CLIENT_threadEvent
(
	FTM_VOID_PTR 		pData
);

static
FTM_RET	FTOM_TP_CLIENT_serverSync
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
);

static
FTM_RET	FTOM_TP_CLIENT_NODE_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_NODE_PTR		pNode
);

static
FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_EP_PTR			pEPInfo
);

static
FTM_RET	FTOM_TP_CLIENT_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

static 
FTOM_MQTT_CLIENT_CBSET	xMQTTCBSet =
{
	.fConnect 			= FTOM_MQTT_CLIENT_TPGW_connectCB,
	.fDisconnect		= FTOM_MQTT_CLIENT_TPGW_disconnectCB,
	.fPublish 			= FTOM_MQTT_CLIENT_TPGW_publishCB,
	.fMessage 			= FTOM_MQTT_CLIENT_TPGW_messageCB,
	.fSubscribe 		= FTOM_MQTT_CLIENT_TPGW_subscribeCB,
	.fGWStatus			= FTOM_MQTT_CLIENT_TPGW_GWStatus,
	.fEPStatus			= FTOM_MQTT_CLIENT_TPGW_publishEPStatus,
	.fEPData			= FTOM_MQTT_CLIENT_TPGW_publishEPData,
	.fTPResponse		= FTOM_MQTT_CLIENT_TPGW_response,
};

static
FTOM_TP_CLIENT_CONFIG	xTPClientDefaultConfig = 
{
	.pGatewayID	= "0123456789abcdef",
	.pAPIKey = "",
	.pCertFile = "",
	.pUserID= "",
	.pPasswd= "",
	.ulReportInterval = FTOM_TP_CLIENT_DEFAULT_REPORT_INTERVAL,

	.xFTOMC=
	{
		.pHost = "127.0.0.1",
		.usPort = 8888
	},

	.xMQTT = 
	{
		.pHost = FTOM_TP_CLIENT_DEFAULT_BROKER,
		.usPort= FTOM_TP_CLIENT_DEFAULT_PORT,
	
		.bSecure = FTM_TRUE,
	
		.ulRetryInterval  = FTOM_TP_CLIENT_DEFAULT_RETRY_INTERVAL
	},

	.xRESTApi = 
	{
		.pBaseURL = "https://api.thingplus.net/v1",
	
		.bSecure = FTM_TRUE
	}
};

FTM_RET	FTOM_TP_CLIENT_create
(
	FTOM_TP_CLIENT_PTR _PTR_ 	ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET				xRet;
	FTOM_TP_CLIENT_PTR	pClient;

	pClient = (FTOM_TP_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_TP_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_TP_CLIENT_init(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT Client initialization was failed.\n");	
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

	memset(pClient, 0, sizeof(FTOM_TP_CLIENT));
	memcpy(&pClient->xConfig, &xTPClientDefaultConfig, sizeof(xTPClientDefaultConfig));

	pClient->bStop = FTM_TRUE;

	xRet = FTOM_TP_GATEWAY_create(&pClient->pGateway);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize gateway!\n");
		goto error;	
	}

	xRet = FTM_LOCK_create(&pClient->pGatewayLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize lock!\n");
		goto error;	
	}

	xRet = FTOM_CLIENT_NET_create((FTOM_CLIENT_NET_PTR _PTR_)&pClient->pFTOMC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create client!\n");
		goto error;	
	}
	
	xRet =FTOM_CLIENT_setNotifyCB((FTOM_CLIENT_PTR)pClient->pFTOMC, FTOM_TP_CLIENT_notifyCB, pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set notify callback\n");	
	}

	xRet = FTOM_TP_RESTAPI_init(&pClient->xRESTApi);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "TP REST API initialize failed!\n");
		goto error;	
	}

	xRet = FTOM_MQTT_CLIENT_init(&pClient->xMQTT);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT Client initialize failed!\n");
		goto error;
	}

	FTOM_MQTT_CLIENT_setParent(&pClient->xMQTT, pClient);

	xRet = FTOM_MQTT_CLIENT_setMessageCB(&pClient->xMQTT, (FTOM_MQTT_CLIENT_MESSAGE_CB)FTOM_TP_CLIENT_sendMessage, pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set MQTT client message callback!\n");
	}

	xRet = FTOM_MSGQ_init(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MsgQ init failed!\n");
		goto error;	
	}

	xRet = FTOM_MQTT_CLIENT_setCBSet(&pClient->xMQTT, &xMQTTCBSet);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set MQTT client callback set!\n");
	}

	return	FTM_RET_OK;

error:
	
	if (pClient->pGateway != NULL)
	{
		FTOM_TP_GATEWAY_destroy(&pClient->pGateway);	
	}

	if (pClient->pFTOMC != NULL)
	{
		FTOM_CLIENT_NET_destroy((FTOM_CLIENT_NET_PTR _PTR_)&pClient->pFTOMC);
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTOM_TP_CLIENT_stop(pClient);

	FTOM_TP_RESTAPI_final(&pClient->xRESTApi);
	FTOM_MQTT_CLIENT_final(&pClient->xMQTT);
	FTOM_MSGQ_final(&pClient->xMsgQ);

	if (pClient->pGatewayLock != NULL)
	{
		FTM_LOCK_destroy(&pClient->pGatewayLock);	
	}

	if (pClient->pGateway != NULL)
	{
		FTOM_TP_GATEWAY_destroy(&pClient->pGateway);
	}

	if (pClient->pFTOMC != NULL)
	{
		FTOM_CLIENT_NET_destroy((FTOM_CLIENT_NET_PTR _PTR_)&pClient->pFTOMC);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_getConfig
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTOM_TP_CLIENT_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &pClient->xConfig, sizeof(FTOM_TP_CLIENT_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_setConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_TP_CLIENT_CONFIG_PTR 	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTOM_CLIENT_NET_CONFIG	xFTOMC;
	FTOM_MQTT_CLIENT_CONFIG	xMQTTConfig;
	FTOM_TP_RESTAPI_CONFIG	xRESTApiConfig;

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_TP_CLIENT_CONFIG));

	strncpy(xFTOMC.xServer.pHost,		pClient->xConfig.xFTOMC.pHost,		FTM_URL_LEN);
	xFTOMC.xServer.usPort = pClient->xConfig.xFTOMC.usPort;

	xRet = FTOM_CLIENT_NET_setConfig((FTOM_CLIENT_NET_PTR)pClient->pFTOMC, &xFTOMC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTOM Client configation loading failed!\n");
		return	0;	
	}

	strncpy(xRESTApiConfig.pGatewayID, 	pClient->xConfig.pGatewayID,		FTM_GWID_LEN);
	if (strlen(pClient->xConfig.pUserID) != 0)
	{
		strncpy(xRESTApiConfig.pUserID, 	pClient->xConfig.pUserID,			FTM_USER_ID_LEN);
	}
	else
	{
		strncpy(xRESTApiConfig.pUserID, 	pClient->xConfig.pGatewayID,			FTM_USER_ID_LEN);
	}
	strncpy(xRESTApiConfig.pPasswd,		pClient->xConfig.pAPIKey, 			FTM_PASSWD_LEN);
	strncpy(xRESTApiConfig.pBaseURL,	pClient->xConfig.xRESTApi.pBaseURL,	FTM_URL_LEN);
	xRESTApiConfig.bSecure = pClient->xConfig.xRESTApi.bSecure;

	xRet = FTOM_TP_RESTAPI_setConfig(&pClient->xRESTApi, &xRESTApiConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT Client configation loading failed!\n");
		return	0;	
	}

	strncpy(xMQTTConfig.pGatewayID,	pClient->xConfig.pGatewayID,	FTM_GWID_LEN);
	if (strlen(pClient->xConfig.pUserID) != 0)
	{
		strncpy(xMQTTConfig.pUserID, 	pClient->xConfig.pUserID,		FTM_USER_ID_LEN);
	}
	else
	{
		strncpy(xMQTTConfig.pUserID, 	pClient->xConfig.pGatewayID,		FTM_USER_ID_LEN);
	}
	strncpy(xMQTTConfig.pPasswd, 	pClient->xConfig.pAPIKey, 		FTM_PASSWD_LEN);
	strncpy(xMQTTConfig.pCertFile, 	pClient->xConfig.pCertFile, 	FTM_FILE_NAME_LEN);
	strncpy(xMQTTConfig.pHost, 		pClient->xConfig.xMQTT.pHost, 	FTM_HOST_LEN);
	xMQTTConfig.usPort 			= pClient->xConfig.xMQTT.usPort;
	xMQTTConfig.ulRetryInterval = pClient->xConfig.xMQTT.ulRetryInterval;
	xMQTTConfig.bTLS 			= pClient->xConfig.xMQTT.bSecure;

	xRet = FTOM_MQTT_CLIENT_setConfig(&pClient->xMQTT, &xMQTTConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT Client configation loading failed!\n");
		return	0;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;
	FTOM_TP_CLIENT_CONFIG	xTPConfig;

	xRet = FTOM_TP_CLIENT_getConfig(pClient, &xTPConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get client configuration!\n");
		return	FTM_RET_ERROR;	
	}

	xRet = FTM_CONFIG_getItem(pConfig, "tpclient", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM xFTOMCConfig;
		FTM_CONFIG_ITEM	xMQTTConfig;
		FTM_CONFIG_ITEM	xRESTApiConfig;

		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "gatewayid", xTPConfig.pGatewayID, FTM_GWID_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find the gateway id for the TPClient!\n");
		}

		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "apikey", xTPConfig.pAPIKey, FTM_PASSWD_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find a APIKEY information for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "cert", xTPConfig.pCertFile, FTM_FILE_NAME_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find the certificate information for the TPClient!\n");
		}

		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "userid", xTPConfig.pUserID, FTM_USER_ID_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find a userid for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "passwd", xTPConfig.pPasswd, FTM_PASSWD_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find a passwd for the TPClient!\n");
		}
		
		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "report_interval", &xTPConfig.ulReportInterval);
		if (xRet != FTM_RET_OK)
		{
			INFO("Can not find a report interval for the TPClient!\n");
		}
		
		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "server", &xFTOMCConfig);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_CONFIG_ITEM_getItemString(&xFTOMCConfig, "host", xTPConfig.xFTOMC.pHost, FTM_HOST_LEN);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find a host for the TPClient!\n");
			}
		
			xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xFTOMCConfig, "port", &xTPConfig.xFTOMC.usPort);
			if (xRet != FTM_RET_OK)
			{
				INFO("Can not find a port for the TPClient!\n");
			}
		}

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "restapi", &xRESTApiConfig);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_CONFIG_ITEM_getItemString(&xRESTApiConfig, "base_url", xTPConfig.xRESTApi.pBaseURL, FTM_URL_LEN);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find a host for the TPClient!\n");
			}
		
			xRet = FTM_CONFIG_ITEM_getItemBOOL(&xRESTApiConfig, "secure", &xTPConfig.xRESTApi.bSecure);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find secure mode !\n");
			}
		}		

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "mqtt", &xMQTTConfig);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_CONFIG_ITEM_getItemString(&xMQTTConfig, "host", xTPConfig.xMQTT.pHost, FTM_HOST_LEN);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find a host for the TPClient!\n");
			}
		
			xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xMQTTConfig, "port", &xTPConfig.xMQTT.usPort);
			if (xRet != FTM_RET_OK)
			{
				INFO("Can not find a port for the TPClient!\n");
			}
		
			xRet = FTM_CONFIG_ITEM_getItemBOOL(&xMQTTConfig, "secure", &xTPConfig.xMQTT.bSecure);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find secure mode !\n");
			}
	
			xRet = FTM_CONFIG_ITEM_getItemULONG(&xMQTTConfig, "retry_interval", &xTPConfig.xMQTT.ulRetryInterval);
			if (xRet != FTM_RET_OK)
			{
				INFO("Can not find max connection retry interval for the TPClient!\n");
			}
		}

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "restapi", &xRESTApiConfig);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_CONFIG_ITEM_getItemString(&xRESTApiConfig, "base_url", xTPConfig.xRESTApi.pBaseURL, FTM_URL_LEN);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find a host for the TPClient!\n");
			}
		
			xRet = FTM_CONFIG_ITEM_getItemBOOL(&xRESTApiConfig, "secure", &xTPConfig.xRESTApi.bSecure);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can not find secure mode !\n");
			}
		}		
	}

	xRet = FTOM_TP_CLIENT_setConfig(pClient, &xTPConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set TP Client configuration!\n");
	}

	FTM_TRACE_loadConfig(pConfig);

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_loadConfigFromFile
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet !=  FTM_RET_OK)
	{
		ERROR2(xRet, "Configration loading failed!\n");
		return	xRet;	
	}

	xRet = FTOM_TP_CLIENT_loadConfig(pClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration!\n");	
	}

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_saveConfigToFile
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_TRUE);
	if (xRet !=  FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to save configuration!\n");
		return	xRet;	
	}

	xRet = FTOM_TP_CLIENT_saveConfig(pClient, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_saveConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;
	FTM_CONFIG_ITEM		xFTOMCConfig;
	FTM_CONFIG_ITEM		xMQTTConfig;
	FTM_CONFIG_ITEM		xRESTApiConfig;

	xRet = FTM_CONFIG_getItem(pConfig, "tpclient", &xSection);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_addItem(pConfig, "tpclient", &xSection);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "gatewayid", pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save the gateway id for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "cert", pClient->xConfig.pCertFile);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save the certificate information for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "apikey", pClient->xConfig.pAPIKey);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a APIKEY information for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "userid", pClient->xConfig.pUserID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a userid for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "passwd", pClient->xConfig.pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a passwd for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemULONG(&xSection, "report_interval", pClient->xConfig.ulReportInterval);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a report interval for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "server", &xFTOMCConfig);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildItem(&xSection, "server", &xFTOMCConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to create FTOM client section!\n");
			return	xRet;	
		}
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xFTOMCConfig, "host", pClient->xConfig.xFTOMC.pHost);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a host for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemUSHORT(&xFTOMCConfig, "port", pClient->xConfig.xFTOMC.usPort);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a port for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "mqtt", &xMQTTConfig);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildItem(&xSection, "mqtt", &xMQTTConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to create MQTT section!\n");
			return	xRet;	
		}
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xMQTTConfig, "host", pClient->xConfig.xMQTT.pHost);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a host for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemUSHORT(&xMQTTConfig, "port", pClient->xConfig.xMQTT.usPort);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a port for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xMQTTConfig, "secure", (pClient->xConfig.xMQTT.bSecure)?"on":"off");
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a secure mode for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemULONG(&xMQTTConfig, "retry_interval", pClient->xConfig.xMQTT.ulRetryInterval);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a max retry interval for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "restapi", &xRESTApiConfig);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildItem(&xSection, "restapi", &xRESTApiConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to create RESTAPI section!\n");
			return	xRet;	
		}
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xRESTApiConfig, "base_url", pClient->xConfig.xRESTApi.pBaseURL);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a host for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xRESTApiConfig, "secure", (pClient->xConfig.xRESTApi.bSecure)?"on":"off");
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a secure mode for the TPClient!\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_showConfig
(
	FTOM_TP_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("\n[ ThingPlus Client Configuration ]\n");
	MESSAGE("%16s : %s\n", "Gateay ID", pClient->xConfig.pGatewayID);
	MESSAGE("%16s : %s\n", "API Key", pClient->xConfig.pAPIKey);
	MESSAGE("%16s : %s\n", "Cert File", pClient->xConfig.pCertFile);
	MESSAGE("%16s : %s\n", "User ID", pClient->xConfig.pUserID);
	MESSAGE("%16s : %s\n", "Password", pClient->xConfig.pPasswd);
	MESSAGE("%16s : %lu\n","Report Interval", pClient->xConfig.ulReportInterval);

	MESSAGE("\n# %s \n", "Server");
	MESSAGE("%16s : %s\n", "Host", pClient->xConfig.xFTOMC.pHost);
	MESSAGE("%16s : %d\n", "Port", pClient->xConfig.xFTOMC.usPort);

	MESSAGE("\n# %s\n", "ThingPlus MQTT");
	MESSAGE("%16s : %s\n", "Host", pClient->xConfig.xMQTT.pHost);
	MESSAGE("%16s : %d\n", "Port", pClient->xConfig.xMQTT.usPort);
	MESSAGE("%16s : %s\n", "Secure Mode", (pClient->xConfig.xMQTT.bSecure)?"on":"off");
	MESSAGE("%16s : %lu\n","Retry Interval", pClient->xConfig.xMQTT.ulRetryInterval);

	MESSAGE("\n# %s\n", "ThingPlus RESTAPI");
	MESSAGE("%16s : %s\n", "Base URL", pClient->xConfig.xRESTApi.pBaseURL);
	MESSAGE("%16s : %s\n", "Secure Mode", (pClient->xConfig.xRESTApi.bSecure)?"on":"off");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_sendMessage
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);
}

FTM_RET	FTOM_TP_CLIENT_start
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;
	FTM_INT	nRet;
	FTM_CHAR				pTopic[FTM_MQTT_TOPIC_LEN + 1];

	if (pClient->xThreadMain != 0)
	{
		xRet = FTM_RET_ALREADY_STARTED;	
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_setUserID(&pClient->xRESTApi, pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set User ID!\n");
	}

	xRet = FTOM_TP_RESTAPI_setPasswd(&pClient->xRESTApi, pClient->xConfig.pAPIKey);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set password!\n");
	}

	xRet = FTOM_TP_RESTAPI_setGatewayID(&pClient->xRESTApi, pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set gateway id!\n");
	}

	FTOM_TP_RESTAPI_setVerbose(&pClient->xRESTApi, FTM_TRUE);

	FTM_TIMER_initS(&pClient->xRetryTimer, 0);
	FTM_TIMER_initS(&pClient->xReportTimer,	0);

	sprintf(pTopic, "v/a/g/%s/res", pClient->xConfig.pGatewayID);
	xRet = FTOM_MQTT_CLIENT_subscribe(&pClient->xMQTT, pTopic);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to subscribe topic[%s]\n",	pTopic);
	}

	sprintf(pTopic, "v/a/g/%s/req", pClient->xConfig.pGatewayID);
	xRet = FTOM_MQTT_CLIENT_subscribe(&pClient->xMQTT, pTopic);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to subscribe topic[%s]\n",	pTopic);
	}

	nRet = pthread_create(&pClient->xThreadMain, NULL, FTOM_TP_CLIENT_threadMain, pClient);
	if (nRet != 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		goto finish;
	}


	return	FTM_RET_OK;

finish:

	pClient->bStop = FTM_TRUE;

	if (pClient->xThreadMain != 0)
	{
		pthread_cancel(pClient->xThreadMain);
		pClient->xThreadMain = 0;
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_stop
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	pClient->bStop = FTM_TRUE;

	pthread_join(pClient->xThreadMain, NULL);
	pClient->xThreadMain = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_isRun
(
	FTOM_TP_CLIENT_PTR pClient,
	FTM_BOOL_PTR		pbRun
)
{
	if ((pClient == NULL) || pClient->bStop)
	{
		*pbRun = FTM_FALSE	;
	}

	*pbRun = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_waitingForFinished
(
	FTOM_TP_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	pthread_join(pClient->xThreadMain, NULL);
	pClient->xThreadMain = 0;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_TP_CLIENT_threadMain
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET					xRet;
	FTM_INT					nRet;
	FTOM_TP_CLIENT_PTR		pClient = (FTOM_TP_CLIENT_PTR)pData;

	pClient->bConnected	= FTM_FALSE;
	pClient->bStop		= FTM_FALSE;

	xRet = FTOM_CLIENT_start(pClient->pFTOMC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start FTOM client\n");
	}

	FTOM_MQTT_CLIENT_start(&pClient->xMQTT);

	nRet = pthread_create(&pClient->xThreadEvent, NULL, FTOM_TP_CLIENT_threadEvent, pClient);
	if (nRet != 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		goto finish;
	}

	while(!pClient->bStop)
	{
		FTOM_MSG_PTR	pBaseMsg;

		xRet = FTOM_MSGQ_timedPop(&pClient->xMsgQ, 1000, &pBaseMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pBaseMsg->xType)
			{
			case	FTOM_MSG_TYPE_CONNECTED:
				{
					xRet = FTOM_TP_CLIENT_serverSync(pClient, FTM_FALSE);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to sync with sever!\n");	
						break;
					}

					pClient->bReportON = FTM_TRUE;
				}
				break;


			case	FTOM_MSG_TYPE_DISCONNECTED:
				{
					pClient->bReportON = FTM_FALSE;
				}
				break;

			case	FTOM_MSG_TYPE_TP_REPORT:
				{
					FTM_ULONG	ulSensorCount;

					FTM_LOCK_set(pClient->pGatewayLock);

					if (pClient->pGateway != NULL)
					{
						FTM_TIME			xTime;
						FTOM_TP_CLIENT_reportGWStatus(pClient, pClient->xConfig.pGatewayID, FTM_TRUE, pClient->xConfig.ulReportInterval);

						FTM_TIME_getCurrent(&xTime);

						xRet = FTM_LIST_count(pClient->pGateway->pSensorList, &ulSensorCount);
						TRACE("Current Time : %s\n", FTM_TIME_printf(&xTime, NULL));
						TRACE("Sensor Count : %lu\n", ulSensorCount);
						if (xRet == FTM_RET_OK)
						{
							FTM_ULONG			i;
							FTM_ULONG			ulCurrentTime = 0;
							FTM_EP				xEPInfo;
							FTOM_TP_SENSOR_PTR	pSensor;
							FTM_ULONG			ulDataCount, ulMaxDataCount = 100;
							FTM_EP_DATA_PTR		pData;

							pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxDataCount);
							if (pData == NULL)
							{
								ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]\n", sizeof(FTM_EP_DATA) * ulMaxDataCount);	
							}

							for(i = 0 ; i < ulSensorCount ; i++)
							{
								xRet = FTM_LIST_getAt(pClient->pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensor);
								if (xRet == FTM_RET_OK)
								{
									xRet = FTOM_CLIENT_EP_get(pClient->pFTOMC, pSensor->pID, &xEPInfo);
									if (xRet == FTM_RET_OK)
									{
										FTM_BOOL	bRun = FTM_FALSE;

										xRet = FTOM_CLIENT_EP_isRun(pClient->pFTOMC, pSensor->pID, &bRun);
										if (xRet == FTM_RET_OK)
										{
											FTOM_TP_CLIENT_sendEPStatus(pClient, xEPInfo.pEPID, bRun, pClient->xConfig.ulReportInterval);
										}
									}

									if (xRet == FTM_RET_OK)
									{
										FTM_TIME_toSecs(&xTime, &ulCurrentTime);
									}

									if (pData != NULL)
									{
										xRet = FTOM_CLIENT_EP_DATA_getListWithTime(pClient->pFTOMC, pSensor->pID, pSensor->ulServerDataTime, ulCurrentTime, pData, ulMaxDataCount, &ulDataCount);
										if (xRet == FTM_RET_OK)
										{
											if (ulDataCount > 0)
											{
												TRACE("Send EP Data : %lu\n", ulDataCount);
												xRet = FTOM_TP_CLIENT_sendEPData(pClient, pSensor->pID, pData, ulDataCount);
												if (xRet != FTM_RET_OK)
												{
													ERROR2(xRet, "Failed to send EP data!\n");	
												}
											}
										}
										else
										{
											ERROR2(xRet, "Failed to get ep data with time!\n");
										}
									}

								}
							}

							if (pData != NULL)
							{
								FTM_MEM_free(pData);
							}
						}
					}

					FTM_LOCK_reset(pClient->pGatewayLock);

				}			
				break;

			case	FTOM_MSG_TYPE_GW_STATUS:
				{
					FTOM_MSG_GW_STATUS_PTR	pMsg = (FTOM_MSG_GW_STATUS_PTR)pBaseMsg;

					FTOM_TP_CLIENT_reportGWStatus(pClient, pMsg->pGatewayID, pMsg->bStatus, pMsg->ulTimeout);
				}
				break;

			case	FTOM_MSG_TYPE_EP_STATUS:
				{
					FTOM_MSG_EP_STATUS_PTR	pMsg = (FTOM_MSG_EP_STATUS_PTR)pBaseMsg;

					FTOM_TP_CLIENT_sendEPStatus(pClient, pMsg->pEPID, pMsg->bStatus, pMsg->ulTimeout);
				}
				break;
			case	FTOM_MSG_TYPE_EP_DATA:
				{
					FTOM_MSG_EP_DATA_PTR	pMsg = (FTOM_MSG_EP_DATA_PTR)pBaseMsg;

					FTOM_TP_CLIENT_sendEPData(pClient, pMsg->pEPID, pMsg->pData, pMsg->ulCount);
				}
				break;

			case	FTOM_MSG_TYPE_SERVER_SYNC:
				{
					FTOM_MSG_SERVER_SYNC_PTR	pMsg = (FTOM_MSG_SERVER_SYNC_PTR)pBaseMsg;

					FTOM_TP_CLIENT_serverSync(pClient, pMsg->bAutoRegister);
				}
				break;

			case	FTOM_MSG_TYPE_TP_REQ_RESTART:
				{
					FTOM_MSG_TP_REQ_RESTART_PTR pMsg = (FTOM_MSG_TP_REQ_RESTART_PTR)pBaseMsg;

					FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, 0, "");

					pClient->bReportON = FTM_FALSE;

					FTOM_TP_CLIENT_serverSync(pClient, FTM_FALSE);

					pClient->bReportON = FTM_TRUE;
				}
				break;

			case	FTOM_MSG_TYPE_TP_REQ_SET_REPORT_INTERVAL:
				{
					FTM_ULONG	ulNodeCount = 0;
					FTM_ULONG	ulEPCount = 0;
					FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR pMsg = (FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR)pBaseMsg;

					if (pMsg->ulReportIntervalMS < 1000)
					{
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, -10000, "Invalid report interval");
					}
					else
					{
						if (pClient->xConfig.ulReportInterval != pMsg->ulReportIntervalMS / 1000)
						{
							pClient->xConfig.ulReportInterval = pMsg->ulReportIntervalMS / 1000;
							FTM_TIMER_initS(&pClient->xReportTimer, 0);
						}
	
						xRet = FTOM_CLIENT_NODE_count(pClient->pFTOMC, &ulNodeCount);
						if (xRet == FTM_RET_OK)
						{
							FTM_INT	i;
	
							for(i = 0 ; i < ulNodeCount ; i++)
							{
								FTM_NODE	xNode;
				
								xRet = FTOM_CLIENT_NODE_getAt(pClient->pFTOMC, i, &xNode);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Can't get EP info at %d\n", i);
									continue;	
								}
			
								FTOM_CLIENT_NODE_setReportInterval(pClient->pFTOMC, xNode.pDID, pMsg->ulReportIntervalMS / 1000);
	
								xRet = FTOM_CLIENT_EP_count(pClient->pFTOMC, 0, xNode.pDID, &ulEPCount);
								if (xRet == FTM_RET_OK)
								{
									FTM_INT	j;
			
									for(j = 0 ; j < ulEPCount ; j++)
									{
										FTM_EP	xEPInfo;
						
										xRet = FTOM_CLIENT_EP_getAt(pClient->pFTOMC, j, &xEPInfo);
										if (xRet != FTM_RET_OK)
										{
											ERROR2(xRet, "Can't get EP info at %d\n", j);
											continue;	
										}
						
										FTOM_CLIENT_EP_setReportInterval(pClient->pFTOMC, xEPInfo.pEPID, pMsg->ulReportIntervalMS / 1000);
									}
								}
			
							}
						}

						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, 0, "");
					}
				}
				break;

			case	FTOM_MSG_TYPE_TP_REQ_CONTROL_ACTUATOR:
				{
					FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR pMsg = (FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR)pBaseMsg;
					FTM_EP		xEP;
					FTM_EP_DATA	xData;

					xRet = FTOM_CLIENT_EP_get(pClient->pFTOMC, pMsg->pEPID, &xEP);
					if (xRet != FTM_RET_OK)
					{
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, xRet, "Invalid EPID");
						break;
					}

					switch(pMsg->xCtrl)
					{
					case	FTM_EP_CTRL_OFF:
						FTM_EP_DATA_initINT(&xData, 0);
						break;
				
					case	FTM_EP_CTRL_ON:
						FTM_EP_DATA_initINT(&xData, 1);
						break;

					case	FTM_EP_CTRL_BLINK:
						FTM_EP_DATA_initINT(&xData, 2);
						break;
					};

					xRet = FTOM_CLIENT_EP_remoteSet(pClient->pFTOMC, xEP.pEPID, &xData);
					if (xRet != FTM_RET_OK)
					{
					
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, xRet, "Remote set error!");
						break;
					}

					FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, 0, "");
				}
				break;

			case	FTOM_MSG_TYPE_EP_DATA_SERVER_TIME:
				{
					FTOM_MSG_EP_DATA_SERVER_TIME_PTR pMsg = (FTOM_MSG_EP_DATA_SERVER_TIME_PTR)pBaseMsg;

					xRet = FTOM_CLIENT_EP_DATA_setServerTime(pClient->pFTOMC, pMsg->pEPID, pMsg->ulTime);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to set EP[%s] data server time!\n", pMsg->pEPID);
					}
				}
				break;

			default:
				{
					ERROR2(FTM_RET_INVALID_MESSAGE_TYPE, "Not supported msg[%08x]\n", pBaseMsg->xType);	
				}
			}
			FTOM_MSG_destroy(&pBaseMsg);
		}
	}

finish:
	if (pClient->xThreadEvent != 0)
	{
		pthread_join(pClient->xThreadEvent, NULL);
		pClient->xThreadEvent = 0;
	}

	FTOM_MQTT_CLIENT_stop(&pClient->xMQTT);
	TRACE("TPClient[%s] stopped.\n", pClient->xConfig.pGatewayID);

	FTOM_CLIENT_NET_stop((FTOM_CLIENT_NET_PTR)pClient->pFTOMC);

	return 0;
}

FTM_VOID_PTR FTOM_TP_CLIENT_threadEvent
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	
	FTM_RET	xRet;
	FTOM_TP_CLIENT_PTR		pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTM_TIME				xBaseTime, xCurrentTime, xDiffTime;

	FTM_TIMER_initS(&pClient->xRetryTimer, 0);
	FTM_TIMER_initS(&pClient->xReportTimer,	0);

	FTM_TIME_getCurrent(&xBaseTime);
	
	while(!pClient->bStop)
	{
		FTM_BOOL	bConnected = pClient->bConnected;

		xRet = FTOM_MQTT_CLIENT_isConnected(&pClient->xMQTT, &bConnected);
		if (xRet == FTM_RET_OK)
		{
			if (bConnected != pClient->bConnected)
			{
				if(bConnected)
				{
					FTOM_MSG_PTR	pMsg;
	
					xRet = FTOM_MSG_createConnected(&pMsg);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to create message!\n");	
					}
					else
					{
						xRet = FTOM_TP_CLIENT_sendMessage(pClient, pMsg);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to send message!\n");	
							FTOM_MSG_destroy(&pMsg);
						}
					}
	
				}
				else
				{
					FTOM_MSG_PTR	pMsg;
	
					xRet = FTOM_MSG_createDisconnected(&pMsg);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to create message!\n");	
					}
					else
					{
						xRet = FTOM_TP_CLIENT_sendMessage(pClient, pMsg);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to send message!\n");	
							FTOM_MSG_destroy(&pMsg);
						}
					}
				}

				pClient->bConnected = bConnected;
			}
		}

		if (pClient->bReportON && FTM_TIMER_isExpired(&pClient->xReportTimer))
		{
			FTOM_MSG_PTR	pMsg;

			xRet = FTOM_MSG_TP_createReport(&pMsg);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to create message!\n");	
			}

			xRet = FTOM_TP_CLIENT_sendMessage(pClient, pMsg);
			if (xRet != FTM_RET_OK)
			{
				FTOM_MSG_destroy(&pMsg);
			}

			FTM_TIMER_addS(&pClient->xReportTimer, pClient->xConfig.ulReportInterval);
		}
	
		xRet = FTM_TIME_getCurrent(&xCurrentTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get current time!\n");	
		}
		FTM_TIME_sub(&xCurrentTime, &xBaseTime, &xDiffTime);
	
		FTM_UINT64	ullDiffTime = 0;
		FTM_TIME_toMS(&xDiffTime, &ullDiffTime);

		if (ullDiffTime < 1000)
		{
			usleep((1000 - ullDiffTime) * 1000);
		}
		FTM_TIME_addMS(&xBaseTime, 1000, &xBaseTime);
	}

	return 0;
}

FTM_RET	FTOM_TP_CLIENT_serverSyncStart
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createServerSync(bAutoRegister, (FTOM_MSG_SERVER_SYNC_PTR _PTR_)&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy(&pMsg);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_serverSync
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
)
{
	FTM_RET	xRet;
	FTM_INT     i;  
	FTM_TIME	xTime;
	FTM_ULONG	ulCurrentTime;
	FTOM_TP_GATEWAY_PTR	pGateway = NULL;
	FTM_ULONG   ulDeviceCount = 0;
	FTM_ULONG   ulSensorCount = 0;

	xRet = FTOM_TP_GATEWAY_create(&pGateway);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize gateway information.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_GW_getInfo(&pClient->xRESTApi, pGateway);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get gateway information.\n");
		goto finish;
	}

	FTM_TIME_getCurrent(&xTime);
	FTM_TIME_toSecs(&xTime, &ulCurrentTime);

	TRACE("%16s : %s\n",  "ID",   pGateway->pID);    
	TRACE("%16s : %s\n",  "Name", pGateway->pName);    
	TRACE("%16s : %lu\n", "Report Interval", pGateway->ulReportInterval);    
	TRACE("%16s : %s\n","Current Time", FTM_TIME_printf(&xTime, NULL));    
	FTM_TIME_setSeconds(&xTime, (FTM_ULONG)(pGateway->ullCTime / 1000));
	TRACE("%16s : %s\n","Installed Time", FTM_TIME_printf(&xTime, NULL));    
	FTM_TIME_setSeconds(&xTime, (FTM_ULONG)(pGateway->ullMTime / 1000));
	TRACE("%16s : %s\n","Modified Time",FTM_TIME_printf(&xTime, NULL));   
	FTM_LIST_count(pGateway->pDeviceList, &ulDeviceCount);
	TRACE("%16s : %lu\n", "Devices", ulDeviceCount);
	for(i = 0 ; i < ulDeviceCount ; i++)
	{   
		FTOM_TP_DEVICE_PTR	pDevice;

		xRet = FTM_LIST_getAt(pGateway->pDeviceList, i, (FTM_VOID_PTR _PTR_)&pDevice);
		if (xRet == FTM_RET_OK)
		{   
			TRACE("%16s   %s\n", "", pDevice->pID);
		}   
	}   

	FTM_LIST_count(pGateway->pSensorList, &ulSensorCount);
	TRACE("%16s : %lu\n", "Sensors", ulSensorCount);

	for(i = 0 ; i < ulSensorCount ; i++)
	{   
		FTOM_TP_SENSOR_PTR	pSensor;

		xRet = FTM_LIST_getAt(pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensor);
		if (xRet == FTM_RET_OK)
		{   
			TRACE("%16s   %s\n", "", pSensor->pID);
		}   
	}   

	if (bAutoRegister || pGateway->bAutoCreateCoverable)
	{
		FTM_ULONG	ulNodeCount = 0;
		FTM_ULONG	ulEPCount = 0;
		FTM_ULONG	ulRegisteredDeviceCount = 0;
		FTM_ULONG	ulRegisteredSensorCount = 0;

		FTM_LIST_count(pGateway->pDeviceList, &ulRegisteredDeviceCount);
		FTM_LIST_count(pGateway->pSensorList, &ulRegisteredSensorCount);

		xRet = FTOM_CLIENT_NODE_count(pClient->pFTOMC, &ulNodeCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Fiailed to get Node count!\n");
			goto finish;	
		}

		for(i = 0 ; i < ulNodeCount ; i++)
		{
			FTM_NODE	xNode;
			FTM_INT		j;
			FTOM_TP_DEVICE_PTR	pDevice = NULL;
			
			xRet = FTOM_CLIENT_NODE_getAt(pClient->pFTOMC, i, &xNode);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can't get EP info at %d\n", i);
				continue;	
			}

			
			for(j = 0 ; j < ulRegisteredDeviceCount ; j++)
			{
				
				xRet = FTM_LIST_getAt(pGateway->pDeviceList, j, (FTM_VOID_PTR _PTR_)&pDevice);
				if (xRet == FTM_RET_OK)
				{
					if (strcmp(pDevice->pID, xNode.pDID) == 0)
					{
						break;	
					}
				}
			}

			if (j == ulRegisteredDeviceCount)
			{
				xRet = FTOM_TP_CLIENT_NODE_register(pClient, &xNode);	
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to register node!\n");
				}
			}

			xRet = FTOM_CLIENT_EP_count(pClient->pFTOMC, 0, xNode.pDID, &ulEPCount);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Fiailed to get EP count!\n");
				goto finish;	
			}

			for(j = 0 ; j < ulEPCount ;j++)
			{
				FTM_EP	xEP;
				FTM_INT	k;

				xRet = FTOM_CLIENT_EP_getAt(pClient->pFTOMC, j, &xEP);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Can't get EP info at %d\n", i);
					continue;	
				}
			
				for(k = 0 ; k < ulRegisteredSensorCount ; k++)
				{
					
					FTOM_TP_SENSOR_PTR	pSensor;
	
					xRet = FTM_LIST_getAt(pGateway->pSensorList, k, (FTM_VOID_PTR _PTR_)&pSensor);
					if (xRet == FTM_RET_OK)
					{
						if (strcmp(pSensor->pID, xEP.pEPID) == 0)
						{
							break;	
						}
					}
				}
	
				if (k == ulRegisteredSensorCount)
				{
					xRet =FTOM_TP_CLIENT_EP_register(pClient, &xEP);	
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to register EP!\n");
					}
				}
			}
		}
	}

	FTM_LOCK_set(pClient->pGatewayLock);

	FTOM_TP_GATEWAY_destroy(&pClient->pGateway);
	pClient->pGateway = pGateway;

	FTM_LOCK_reset(pClient->pGatewayLock);

	return	FTM_RET_OK;
finish:

	if (pGateway != NULL)
	{
		FTOM_TP_GATEWAY_destroy(&pGateway);
	}

	return	FTM_RET_OK;
}
			
FTM_RET	FTOM_TP_CLIENT_NODE_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_NODE_PTR		pNode
)
{
	ASSERT(pClient != NULL);
	ASSERT(pNode != NULL);
	
	FTM_RET	xRet;

	TRACE("TPClient[%s] device[%s] creation!\n", pClient->xConfig.pGatewayID, pNode->pDID);

	xRet = FTOM_TP_RESTAPI_NODE_create(&pClient->xRESTApi, pNode);
	if (xRet != FTM_RET_OK)
	{
		TRACE("TPClient[%s] device[%s] creation failed!\n", pClient->xConfig.pGatewayID, pNode->pDID);	
	}
	else
	{
		TRACE("TPClient[%s] device[%s] creation successfully complete!\n", pClient->xConfig.pGatewayID, pNode->pDID);	
	}
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_EP_PTR			pEPInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPInfo != NULL);
	
	FTM_RET	xRet;

	TRACE("TPClient[%s] sensor[%s] creation!\n", pClient->xConfig.pGatewayID, pEPInfo->pEPID);

	xRet = FTOM_TP_RESTAPI_EP_create(&pClient->xRESTApi, pEPInfo);
	if (xRet != FTM_RET_OK)
	{
		TRACE("TPClient[%s] sensor[%s] creation failed!\n", pClient->xConfig.pGatewayID, pEPInfo->pEPID);	
	}
	else
	{
		TRACE("TPClient[%s] sensor[%s] creation successfully complete!\n", pClient->xConfig.pGatewayID, pEPInfo->pEPID);	
	}
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_reportGWStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);
	ASSERT(pGatewayID != NULL);

	return	FTOM_MQTT_CLIENT_reportGWStatus(&pClient->xMQTT, pGatewayID, bStatus, ulTimeout);
}

FTM_RET	FTOM_TP_CLIENT_sendEPStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	return	FTOM_MQTT_CLIENT_publishEPStatus(&pClient->xMQTT, pEPID, bStatus, ulTimeout);
}

FTM_RET	FTOM_TP_CLIENT_sendEPData
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pDatas, 
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pDatas != NULL);

	return	FTOM_MQTT_CLIENT_publishEPData(&pClient->xMQTT, pEPID, pDatas, ulCount);
}

FTM_RET	FTOM_TP_CLIENT_respose
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nErrorCode,
	FTM_CHAR_PTR		pMessage
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsgID != NULL);
	ASSERT(pMessage != NULL);

	return	FTOM_MQTT_CLIENT_response(&pClient->xMQTT, pMsgID, nErrorCode, pMessage);
}

FTM_RET	FTOM_TP_CLIENT_controlActuator
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration
)
{
	ASSERT(pClient != NULL);


	return	FTM_RET_OK;
}


FTM_RET	FTOM_TP_CLIENT_notifyCB
(
	FTOM_MSG_PTR	pBaseMsg,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pBaseMsg != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTOM_MSG_PTR		pNewMsg = NULL;

	xRet = FTOM_MSG_copy(pBaseMsg, &pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to copy message!\n");
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to push message!\n");
		FTOM_MSG_destroy(&pNewMsg);	
	}

	return	xRet;
}
