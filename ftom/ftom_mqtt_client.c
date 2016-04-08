#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "ftm.h"
#include "nxjson.h"
#include "ftom_mqtt_client.h"
#include "ftom_mqtt_client_tpgw.h"
#include "ftom_mqtt_client_ft.h"

typedef	struct
{
	FTM_INT			nMessageID;
	FTM_CHAR_PTR	pTopic;
	FTM_CHAR_PTR	pMessage;
	FTM_ULONG		ulMessageLen;
	FTM_ULONG		ulQoS;
}	FTOM_MQTT_PUBLISH, _PTR_ FTOM_MQTT_PUBLISH_PTR;

static FTM_VOID_PTR FTOM_MQTT_CLIENT_process(FTM_VOID_PTR pData);

static FTM_RET	FTOM_MQTT_CLIENT_onPublishEPData
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTOM_MSG_PUBLISH_EP_DATA_PTR	pMsg
);

static FTM_VOID_PTR FTOM_MQTT_CLIENT_connector
(
	FTM_VOID_PTR 		pData
);
static FTM_VOID FTOM_MQTT_CLIENT_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTOM_MQTT_CLIENT_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTOM_MQTT_CLIENT_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID	FTOM_MQTT_CLIENT_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
static FTM_VOID FTOM_MQTT_CLIENT_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);

static FTM_RET	FTOM_MQTT_PUBLISH_create
(
	FTM_CHAR_PTR	pTopic,
	FTM_CHAR_PTR	pMessage,
	FTM_ULONG		ulMessageLen,
	FTM_ULONG		ulQoS,
	FTOM_MQTT_PUBLISH_PTR _PTR_ ppPublish
);

static FTM_RET	FTOM_MQTT_PUBLISH_destroy
(
	FTOM_MQTT_PUBLISH_PTR _PTR_ ppPublish
);

FTM_BOOL FTOM_MQTT_PUBLISH_LIST_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static FTOM_MQTT_CLIENT_CALLBACK_SET	pCBSet[] =
{
	{
		.fConnect 	= FTOM_MQTT_CLIENT_FT_connectCB,
		.fDisconnect= FTOM_MQTT_CLIENT_FT_disconnectCB,
		.fPublish 	= FTOM_MQTT_CLIENT_FT_publishCB,
		.fMessage 	= FTOM_MQTT_CLIENT_FT_messageCB,
		.fSubscribe = FTOM_MQTT_CLIENT_FT_subscribeCB,
		.fPublishEPData		= FTOM_MQTT_CLIENT_TPGW_publishEPData,
	},
	{
		.fConnect 	= FTOM_MQTT_CLIENT_TPGW_connectCB,
		.fDisconnect= FTOM_MQTT_CLIENT_TPGW_disconnectCB,
		.fPublish 	= FTOM_MQTT_CLIENT_TPGW_publishCB,
		.fMessage 	= FTOM_MQTT_CLIENT_TPGW_messageCB,
		.fSubscribe = FTOM_MQTT_CLIENT_TPGW_subscribeCB,
		.fPublishEPData		= FTOM_MQTT_CLIENT_TPGW_publishEPData,
	},
};

static 	FTM_ULONG	ulClientInstance = 0;

FTM_RET	FTOM_MQTT_CLIENT_create
(
	FTOM_PTR 			pOM,
	FTOM_MQTT_CLIENT_PTR _PTR_ 	ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET				xRet;
	FTOM_MQTT_CLIENT_PTR	pClient;

	pClient = (FTOM_MQTT_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_MQTT_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_MQTT_CLIENT_init(pClient, pOM);
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

FTM_RET	FTOM_MQTT_CLIENT_destroy
(
	FTOM_MQTT_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_MQTT_CLIENT_final(*ppClient);

	FTM_MEM_free(*ppClient);
	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_init
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTOM_PTR 			pOM
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

	pClient->pOM = pOM;

	FTOM_getDID(pOM, pClient->pDID, FTM_DID_LEN);
	strncpy(pClient->xConfig.pClientID, pClient->pDID, sizeof(pClient->xConfig.pClientID) - 1);
	strcpy(pClient->xConfig.xBroker.pHost, FTOM_MQTT_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.xBroker.usPort = FTOM_MQTT_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulReconnectionTime = FTOM_MQTT_CLIENT_DEFAULT_RECONNECTION_TIME;
	pClient->xConfig.ulCBSet = FTOM_MQTT_CLIENT_DEFAULT_CB_SET;

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

FTM_RET	FTOM_MQTT_CLIENT_final
(
	FTOM_MQTT_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTOM_MQTT_PUBLISH_PTR pPublish;

	if (pClient->pMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTOM_MQTT_CLIENT_stop(pClient);

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

FTM_RET	FTOM_MQTT_CLIENT_loadConfig
(
	FTOM_MQTT_CLIENT_PTR 		pClient, 
	FTOM_MQTT_CLIENT_CONFIG_PTR 	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_MQTT_CLIENT_CONFIG));

	return	FTM_RET_OK;
}


FTM_RET	FTOM_MQTT_CLIENT_loadFromFile
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR 			pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_showConfig
(
	FTOM_MQTT_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_setCallback
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_SERVICE_ID 		xServiceID, 
	FTOM_SERVICE_CALLBACK fServiceCB
)
{
	ASSERT(pClient != NULL);

	pClient->xServiceID	= xServiceID;
	pClient->fServiceCB	= fServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_notify
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pClient != NULL);

	switch(pMsg->xType)
	{
	case	FTOM_MSG_TYPE_SEND_EP_DATA:
		{	
			
			//FTOM_MQTT_CLIENT_onSendEPData(pClient, (FTOM_MSG_SEND_EP_DATA_PTR)pMsg);
		}
		break;

	default:
		{
			ERROR("Not supported message[%08x]!\n", pMsg->xType);	
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_onPublishEPData
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTOM_MSG_PUBLISH_EP_DATA_PTR	pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	if (pCBSet[pClient->xConfig.ulCBSet].fPublishEPData != NULL)
	{
		xRet = pCBSet[pClient->xConfig.ulCBSet].fPublishEPData(pClient, pMsg->xEPID, pMsg->pData, pMsg->ulCount);
	}
	else
	{
		xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
	}

	return	xRet;
}

FTM_RET	FTOM_MQTT_CLIENT_start
(
	FTOM_MQTT_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	if (pClient->bStop != FTM_TRUE)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	pthread_create(&pClient->xMain, NULL, FTOM_MQTT_CLIENT_process, pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_stop
(
	FTOM_MQTT_CLIENT_PTR pClient
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

FTM_VOID_PTR FTOM_MQTT_CLIENT_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pData;

	pClient->bStop 		= FTM_FALSE;
	pClient->bConnected = FTM_FALSE;
	pClient->pMosquitto = mosquitto_new(pClient->xConfig.pClientID, true, pClient);
	if(pClient->pMosquitto != NULL)
	{
		TRACE("MQTT CLIENT[%s] started.\n", pClient->xConfig.pClientID);

		mosquitto_connect_callback_set(pClient->pMosquitto, 	FTOM_MQTT_CLIENT_connectCB);
		mosquitto_disconnect_callback_set(pClient->pMosquitto, 	FTOM_MQTT_CLIENT_disconnectCB);
		mosquitto_publish_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_publishCB);
		mosquitto_subscribe_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_subscribeCB);
		mosquitto_message_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_messageCB);

		pthread_create(&pClient->xConnector, NULL, FTOM_MQTT_CLIENT_connector, pClient);

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
						xRet = FTOM_MQTT_CLIENT_onPublishEPData(pClient, (FTOM_MSG_PUBLISH_EP_DATA_PTR)pMsg);	
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

		TRACE("MQTT CLIENT[%s] stopped.\n", pClient->xConfig.pClientID);

		mosquitto_destroy(pClient->pMosquitto);
		pClient->pMosquitto = NULL;
		
	}
	else
	{
		ERROR("Can't create mosquitto!\n");	
	}

	return 0;
}

FTM_RET	FTOM_MQTT_CLIENT_pushMsg
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(pClient->pMsgQ, pMsg);
}

FTM_VOID_PTR FTOM_MQTT_CLIENT_connector
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pData;

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

FTM_RET	FTOM_MQTT_CLIENT_publishEPData
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet;
	FTOM_MSG_PUBLISH_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createPublishEPData(xEPID, pData, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MQTT_CLIENT_pushMsg(pClient, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}


FTM_RET	FTOM_MQTT_CLIENT_publish
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
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
			TRACE("Publish[%04d] - %s:%s\n", pPublish->nMessageID, pTopic, pMessage);
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

FTM_VOID FTOM_MQTT_CLIENT_connectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);

	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT is connected.\n");
	pClient->bConnected = FTM_TRUE;

	if (pCBSet[pClient->xConfig.ulCBSet].fConnect != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fConnect(mosq, pObj, nResult);
	}
}

FTM_VOID FTOM_MQTT_CLIENT_disconnectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);

	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT is disconnected.\n");
	if (pCBSet[pClient->xConfig.ulCBSet].fDisconnect != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fDisconnect(mosq, pObj, nResult);
	}

	FTM_TIMER_init(&pClient->xReconnectionTimer, 0);
	FTM_TIMER_addSeconds(&pClient->xReconnectionTimer, pClient->xConfig.ulReconnectionTime);

	pClient->bConnected = FTM_FALSE;

}

FTM_VOID FTOM_MQTT_CLIENT_publishCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nMID
)
{
	ASSERT(pObj != NULL);

	FTM_RET	xRet;
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;
	FTOM_MQTT_PUBLISH_PTR	pPublish;


	TRACE("MQTT published.\n");
	if (pCBSet[pClient->xConfig.ulCBSet].fPublish != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fPublish(mosq, pObj, nMID);
	}

	xRet = FTM_LIST_get(pClient->pPublishList, &nMID, (FTM_VOID_PTR _PTR_)&pPublish);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pPublishList, pPublish);	
		FTOM_MQTT_PUBLISH_destroy(&pPublish);
	}
	else
	{
		WARN("Publish[%08x] not found!\n");
	}
}

FTM_VOID FTOM_MQTT_CLIENT_messageCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	const struct mosquitto_message *pMessage
)
{
	ASSERT(pObj != NULL);

	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT received message.\n");
	if (pCBSet[pClient->xConfig.ulCBSet].fMessage != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fMessage(mosq, pObj, pMessage);
	}
}

FTM_VOID FTOM_MQTT_CLIENT_subscribeCB
(
	struct mosquitto 	*mosq, 
	void				*pObj,
	int					nMID, 
	int					nQoS, 
	const int			*pGrantedQoS
)
{
	ASSERT(pObj != NULL);

	TRACE("MQTT subscribe.\n");
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;

	if (pCBSet[pClient->xConfig.ulCBSet].fSubscribe != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fSubscribe(mosq, pObj, nMID, nQoS, pGrantedQoS);
	}
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

FTM_RET	FTOM_MQTT_PUBLISH_create
(
	FTM_CHAR_PTR	pTopic,
	FTM_CHAR_PTR	pMessage,
	FTM_ULONG		ulMessageLen,
	FTM_ULONG		ulQoS,
	FTOM_MQTT_PUBLISH_PTR _PTR_ ppPublish
)
{
	ASSERT(pTopic != NULL);
	ASSERT(pMessage != NULL);
	ASSERT(ppPublish != NULL);

	FTOM_MQTT_PUBLISH_PTR pPublish;
	
	pPublish = FTM_MEM_malloc(sizeof(FTOM_MQTT_PUBLISH) + strlen(pTopic) + 1 + ulMessageLen);
	if (pPublish == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pPublish->pTopic 		= (FTM_CHAR_PTR)pPublish + sizeof(FTOM_MQTT_PUBLISH);
	pPublish->pMessage		= (FTM_CHAR_PTR)pPublish + sizeof(FTOM_MQTT_PUBLISH) + strlen(pTopic) + 1;

	strcpy(pPublish->pTopic, pTopic);
	memcpy(pPublish->pMessage, pMessage, ulMessageLen);
	pPublish->ulMessageLen 	= ulMessageLen;
	pPublish->ulQoS     	= ulQoS;
	pPublish->nMessageID	= 0;

	*ppPublish = pPublish;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_PUBLISH_destroy
(
	FTOM_MQTT_PUBLISH_PTR _PTR_ ppPublish
)
{
	ASSERT(ppPublish != NULL);

	FTM_MEM_free(*ppPublish);

	*ppPublish = NULL;

	return	FTM_RET_OK;
}


