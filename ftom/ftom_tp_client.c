#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
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
FTM_RET	FTOM_TP_CLIENT_publishEPData
(
	FTOM_TP_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
);

static 
FTM_VOID_PTR FTOM_TP_CLIENT_connector
(
	FTM_VOID_PTR 		pData
);

static
FTM_BOOL FTOM_MQTT_PUBLISH_LIST_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static 	FTM_ULONG	ulClientInstance = 0;

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
	strncpy(pClient->xConfig.pGatewayID, pClient->pDID, sizeof(pClient->xConfig.pGatewayID) - 1);
	strcpy(pClient->xConfig.xBroker.pHost, FTOM_TP_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.xBroker.usPort = FTOM_TP_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulReconnectionTime = FTOM_TP_CLIENT_DEFAULT_RECONNECTION_TIME;
	pClient->xConfig.ulCBSet = FTOM_TP_CLIENT_DEFAULT_CB_SET;

	pClient->bStop = FTM_TRUE;
	FTOM_MSGQ_create(&pClient->pMsgQ);

	xRet = FTM_LIST_create(&pClient->pPublishList);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSGQ_destroy(&pClient->pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pClient->pPublishList, FTOM_MQTT_PUBLISH_LIST_seeker);

	if (ulClientInstance++ == 0)
	{
		mosquitto_lib_init();
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTOM_MQTT_PUBLISH_PTR pPublish;

	if (pClient->pMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTOM_TP_CLIENT_stop(pClient);

	FTM_LIST_iteratorStart(pClient->pPublishList);
	while(FTM_LIST_iteratorNext(pClient->pPublishList, (FTM_VOID_PTR _PTR_)&pPublish) == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pPublishList, pPublish);
		FTOM_MQTT_PUBLISH_destroy(&pPublish);
	}

	FTM_LIST_destroy(pClient->pPublishList);

	FTOM_MSGQ_destroy(&pClient->pMsgQ);

	if (--ulClientInstance == 0)
	{
		mosquitto_lib_cleanup();
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 		pClient, 
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

	config_t			xConfig;
	config_setting_t	*pSection;

	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "tpclient");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "id");
		if (pField != NULL)
		{
			FTM_CONST_CHAR_PTR pGatewayID;
			
			pGatewayID = config_setting_get_string(pField);

			strncpy(pClient->xConfig.pGatewayID, pGatewayID, FTM_ID_LEN);
			TRACE("ID : %s\n", pClient->xConfig.pGatewayID);
		}
		else
		{
			FTOM_getDID(pClient->xConfig.pGatewayID, FTM_ID_LEN);
		}

		pField = config_setting_get_member(pSection, "cert");
		if (pField != NULL)
		{
			FTM_CONST_CHAR_PTR pCertFile;
			
			pCertFile = config_setting_get_string(pField);

			strncpy(pClient->xConfig.xBroker.pCertFile, pCertFile, FTM_FILE_NAME_LEN);
		}
	
		pField = config_setting_get_member(pSection, "host");
		if (pField != NULL)
		{
			FTM_CONST_CHAR_PTR	pHost;

			pHost = config_setting_get_string(pField);

			strncpy(pClient->xConfig.xBroker.pHost, pHost, sizeof(pClient->xConfig.xBroker.pHost));
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pClient->xConfig.xBroker.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	
		pField = config_setting_get_member(pSection, "apikey");
		if (pField != NULL)
		{
			FTM_CONST_CHAR_PTR	pAPIKey;

			pAPIKey = config_setting_get_string(pField);

			strncpy(pClient->xConfig.pAPIKey, pAPIKey, sizeof(pClient->xConfig.pAPIKey));
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_showConfig
(
	FTOM_TP_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("\n[ ThingPlus Client Configuration ]\n");
	MESSAGE("%16s : %s\n", "Host", pClient->xConfig.xBroker.pHost);
	MESSAGE("%16s : %d\n", "Port", pClient->xConfig.xBroker.usPort);
	MESSAGE("%16s : %s\n", "User ID", pClient->xConfig.xBroker.pUserID);
	MESSAGE("%16s : %s\n", "Password", pClient->xConfig.xBroker.pPasswd);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_setCallback
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_SERVICE_ID 		xServiceID, 
	FTOM_SERVICE_CALLBACK fServiceCB
)
{
	ASSERT(pClient != NULL);

	pClient->xServiceID	= xServiceID;
	pClient->fServiceCB	= fServiceCB;

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

	pClient->bConnected = FTM_FALSE;
	pClient->pMosquitto = mosquitto_new(pClient->xConfig.pGatewayID, true, pClient);
	if(pClient->pMosquitto == NULL)
	{
		ERROR("Can't create mosquitto!\n");	
		return	0;
	}

	pClient->bStop 		= FTM_FALSE;
	TRACE("TPClient[%s] started.\n", pClient->xConfig.pGatewayID);

	mosquitto_username_pw_set(pClient->pMosquitto, pClient->xConfig.xBroker.pUserID, pClient->xConfig.xBroker.pPasswd);
	mosquitto_tls_set(pClient->pMosquitto, pClient->xConfig.xBroker.pCertFile, NULL, NULL, NULL, NULL);
	mosquitto_tls_insecure_set(pClient->pMosquitto, 1);
	mosquitto_tls_opts_set(pClient->pMosquitto, 1, NULL, NULL);

	mosquitto_connect_callback_set(pClient->pMosquitto, 	FTOM_MQTT_CLIENT_TPGW_connectCB);
	mosquitto_disconnect_callback_set(pClient->pMosquitto, 	FTOM_MQTT_CLIENT_TPGW_disconnectCB);
	mosquitto_publish_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_TPGW_publishCB);
	mosquitto_subscribe_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_TPGW_subscribeCB);
	mosquitto_message_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_TPGW_messageCB);

	pthread_create(&pClient->xConnector, NULL, FTOM_TP_CLIENT_connector, pClient);

	while(!pClient->bStop)
	{
		FTOM_MSG_PTR	pMsg;

		xRet = FTOM_MSGQ_timedPop(pClient->pMsgQ, 1000000, &pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTOM_MSG_TYPE_PUBLISH_EP_DATA:
				{
					xRet = FTOM_TP_CLIENT_publishEPData(pClient, 
						((FTOM_MSG_PUBLISH_EP_DATA_PTR)pMsg)->pEPID, 
						((FTOM_MSG_PUBLISH_EP_DATA_PTR)pMsg)->pData, 
						((FTOM_MSG_PUBLISH_EP_DATA_PTR)pMsg)->ulCount);
				}
				break;

			default:
				{
					ERROR("Not supported msg[%08x]\n", pMsg->xType);	
				}
			}
			FTOM_MSG_destroy(&pMsg);
		}
		else
		{
			usleep(1000);
		}
	}

	pthread_join(pClient->xConnector, NULL);

	TRACE("TPClient[%s] stopped.\n", pClient->xConfig.pGatewayID);

	mosquitto_destroy(pClient->pMosquitto);
	pClient->pMosquitto = NULL;
	
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

FTM_VOID_PTR FTOM_TP_CLIENT_connector
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;

	mosquitto_connect_async(pClient->pMosquitto, pClient->xConfig.xBroker.pHost, pClient->xConfig.xBroker.usPort, 60);
	mosquitto_loop_start(pClient->pMosquitto);
	FTM_TIMER_init(&pClient->xReconnectionTimer, pClient->xConfig.ulReconnectionTime * 1000000);

	while(!pClient->bStop)
	{
		if(!pClient->bStop && !pClient->bConnected)
		{
			if (FTM_TIMER_isExpired(&pClient->xReconnectionTimer))
			{
				TRACE("MQTT try reconnection !\n");
				mosquitto_reconnect_async(pClient->pMosquitto);
				FTM_TIMER_addSeconds(&pClient->xReconnectionTimer, pClient->xConfig.ulReconnectionTime);
			}
		}

		usleep(1000);
	}
	TRACE("MQTT connector was stopped.\n");

	return 0;
}

FTM_RET	FTOM_TP_CLIENT_publishEPData
(
	FTOM_TP_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet;
	FTOM_MSG_PUBLISH_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createPublishEPData(pEPID, pData, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_TP_CLIENT_pushMsg(pClient, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}


FTM_RET	FTOM_TP_CLIENT_publish
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR			pTopic,
	FTM_CHAR_PTR			pMessage,
	FTM_ULONG				ulMessageLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTopic != NULL);
	ASSERT(pMessage != NULL);

	FTM_RET	xRet;
	FTM_INT	nRet;
	FTOM_MQTT_PUBLISH_PTR	pPublish;

	xRet = FTOM_MQTT_PUBLISH_create(pTopic, pMessage, ulMessageLen, 1, &pPublish);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	nRet = mosquitto_publish(pClient->pMosquitto, &pPublish->nMessageID, pPublish->pTopic, pPublish->ulMessageLen, pPublish->pMessage, pPublish->ulQoS, 0);
	switch(nRet)
	{
	case	MOSQ_ERR_SUCCESS:
		{
			//TRACE("Publish[%04d] - %s:%s\n", pPublish->nMessageID, pTopic, pMessage);
			xRet = FTM_LIST_append(pClient->pPublishList, pPublish);
			if (xRet != FTM_RET_OK)
			{
				FTOM_MQTT_PUBLISH_destroy(&pPublish);	
			}
		}
		break;

	case	MOSQ_ERR_INVAL:
		{
			ERROR("Invalid arguments.\n");
			xRet = FTM_RET_INVALID_ARGUMENTS;
		}
		break;

	case	MOSQ_ERR_NOMEM:
		{
			ERROR("Not enoguh memory!.\n");
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		}
		break;

	case	MOSQ_ERR_NO_CONN:
		{
			ERROR("Not connected.\n");
			xRet = FTM_RET_NOT_CONNECTED;
		}
		break;

	case	MOSQ_ERR_PROTOCOL:
		{
			ERROR("Protocol error!\n");
			xRet = FTM_RET_ERROR;
		}
		break;

	case	MOSQ_ERR_PAYLOAD_SIZE:
		{
			ERROR("Payload is too large.\n");
			xRet = FTM_RET_PAYLOAD_IS_TOO_LARGE;
		}
		break;

	default:
		{
			xRet = FTM_RET_ERROR;
		}
	}
	
	return	xRet;
}

FTM_BOOL FTOM_MQTT_PUBLISH_LIST_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
)
{
	ASSERT(pElement != NULL);
	ASSERT(pKey != NULL);

	FTOM_MQTT_PUBLISH_PTR pPublish = (FTOM_MQTT_PUBLISH_PTR)pElement;
	FTM_INT_PTR		 pMessageID= (FTM_INT_PTR)pKey;

	return	(pPublish->nMessageID == *pMessageID);
}


