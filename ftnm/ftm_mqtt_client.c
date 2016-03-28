#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "ftm.h"
#include "nxjson.h"
#include "ftm_mqtt_client.h"

static FTM_VOID FTM_MQTT_CLIENT_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID	FTM_MQTT_CLIENT_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
static FTM_VOID FTM_MQTT_CLIENT_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);
static FTM_VOID_PTR FTM_MQTT_CLIENT_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTM_MQTT_CLIENT_connector(FTM_VOID_PTR pData);
static FTM_RET	FTM_MQTT_CLIENT_publishEPData(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_EP_DATA_PTR	pData);
static FTM_RET	FTM_MQTT_CLIENT_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
static FTM_RET	FTM_MQTT_CLIENT_requestMessageParser(FTM_CHAR_PTR pMessage, FTNM_MSG_PTR _PTR_	pMsg);

FTM_RET	FTM_MQTT_CLIENT_create
(
	FTNM_CONTEXT_PTR 			pCTX, 
	FTM_MQTT_CLIENT_PTR _PTR_ 	ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET				xRet;
	FTM_MQTT_CLIENT_PTR	pClient;

	pClient = (FTM_MQTT_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTM_MQTT_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_MQTT_CLIENT_init(pCTX, pClient);
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

FTM_RET	FTM_MQTT_CLIENT_destroy(FTM_MQTT_CLIENT_PTR _PTR_ ppClient)
{
	ASSERT(ppClient != NULL);

	FTM_MQTT_CLIENT_final(*ppClient);

	FTM_MEM_free(*ppClient);
	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_init(FTNM_CONTEXT_PTR pCTX, FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);
	
	if (pClient->pMsgQ != NULL)
	{
		ERROR("Already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	memset(pClient, 0, sizeof(FTM_MQTT_CLIENT));

	pClient->pCTX = pCTX;

	FTNM_getDID(pClient->pDID, FTM_DID_LEN);
	strncpy(pClient->xConfig.pClientID, pClient->pDID, sizeof(pClient->xConfig.pClientID) - 1);
	strcpy(pClient->xConfig.xBroker.pHost, FTM_MQTT_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.xBroker.usPort = FTM_MQTT_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulReconnectionTime = FTM_MQTT_CLIENT_DEFAULT_RECONNECTION_TIME;

	pClient->bStop = FTM_TRUE;
	FTM_MSGQ_create(&pClient->pMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_final(FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);

	if (pClient->pMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_MQTT_CLIENT_stop(pClient);

	FTM_MSGQ_destroy(pClient->pMsgQ);
	pClient->pMsgQ = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_loadConfig(FTM_MQTT_CLIENT_PTR pClient, FTM_MQTT_CLIENT_CONFIG_PTR pConfig)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTM_MQTT_CLIENT_CONFIG));

	return	FTM_RET_OK;
}


FTM_RET	FTM_MQTT_CLIENT_loadFromFile(FTM_MQTT_CLIENT_PTR pClient, FTM_CHAR_PTR pFileName)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_showConfig(FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_setCallback(FTM_MQTT_CLIENT_PTR pClient, FTNM_SERVICE_ID xServiceID, FTNM_SERVICE_CALLBACK fServiceCB)
{
	ASSERT(pClient != NULL);

	pClient->xServiceID	= xServiceID;
	pClient->fServiceCB	= fServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_notify(FTM_MQTT_CLIENT_PTR pClient, FTNM_MSG_PTR pMsg)
{
	ASSERT(pClient != NULL);

	switch(pMsg->xType)
	{
	case	FTNM_MSG_TYPE_EP_DATA_UPDATED:
		{	
			
			TRACE("EP[%08x] data is updated.\n", pMsg->xParams.xEPDataUpdated.xEPID);
			FTM_MQTT_CLIENT_publishEPData(pClient, pMsg->xParams.xEPDataUpdated.xEPID, &pMsg->xParams.xEPDataUpdated.xData);
		}
		break;

	default:
		{
			ERROR("Not supported message[%08x]!\n", pMsg->xType);	
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_start(FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);
	
	if (pClient->bStop != FTM_TRUE)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	pthread_create(&pClient->xMain, NULL, FTM_MQTT_CLIENT_process, pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_stop(FTM_MQTT_CLIENT_PTR pClient)
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

FTM_VOID_PTR FTM_MQTT_CLIENT_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pData;

	mosquitto_lib_init();

	TRACE("CLIENT ID : %s\n", pClient->xConfig.pClientID);
	pClient->pMosquitto = mosquitto_new(pClient->xConfig.pClientID, true, pClient);
	if(pClient->pMosquitto == NULL)
	{
		ERROR("Can't create mosquitto!\n");	
	}
	else
	{
		mosquitto_connect_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_connectCB);
		mosquitto_disconnect_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_disconnectCB);
		mosquitto_publish_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_publishCB);
		mosquitto_subscribe_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_subscribeCB);
		mosquitto_message_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_messageCB);

		pthread_create(&pClient->xConnector, NULL, FTM_MQTT_CLIENT_connector, pClient);

		while(!pClient->bStop)
		{
			usleep(1000);
		}

		pthread_join(pClient->xConnector, NULL);

	}

	if (pClient->pMosquitto != NULL)
	{
		mosquitto_destroy(pClient->pMosquitto);
		pClient->pMosquitto = NULL;
	}

	mosquitto_lib_cleanup();

	TRACE("CLIENT main stopped.\n");

	return 0;
}

FTM_VOID_PTR FTM_MQTT_CLIENT_connector(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pData;
	FTM_INT	nRet;

	pClient->bStop = FTM_FALSE;

	nRet = mosquitto_connect_async(pClient->pMosquitto, pClient->xConfig.xBroker.pHost, pClient->xConfig.xBroker.usPort, 60);
	TRACE("MQTT client is connected async[%d].\n", nRet);

	mosquitto_loop_start(pClient->pMosquitto);

	TRACE("%s[%d]\n", __func__, __LINE__);
	while(!pClient->bStop)
	{
		if(!pClient->bStop && nRet)
		{
	TRACE("%s[%d]\n", __func__, __LINE__);
			sleep(pClient->xConfig.ulReconnectionTime);
			//mosquitto_reconnect_async(pClient->pMosquitto);
		}
	}
	TRACE("CLIENT connector was stopped.\n");

	return 0;
}

FTM_VOID FTM_MQTT_CLIENT_connectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);
	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;
	FTM_CHAR			pTopic[FTM_MQTT_TOPIC_LEN + 1];

	TRACE("MQTT is connected.\n");

	sprintf(pTopic, "v/a/g/%s/res", pClient->pDID);
	mosquitto_subscribe(pClient->pMosquitto, NULL, pTopic, 0);
	sprintf(pTopic, "v/a/g/%s/req", pClient->pDID);
	mosquitto_subscribe(pClient->pMosquitto, NULL, pTopic, 0);
}

FTM_VOID FTM_MQTT_CLIENT_disconnectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);
	//FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT is disconnected.\n");
}

FTM_VOID FTM_MQTT_CLIENT_publishCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
}

FTM_VOID FTM_MQTT_CLIENT_messageCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	const struct mosquitto_message *pMessage
)
{
	ASSERT(pObj != NULL);

	FTM_RET			xRet;
	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;
	FTM_CHAR		pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR_PTR	pIDs[10];
	FTM_INT			nIDs = 0;
	
	MESSAGE("TOPIC - %s\n", pMessage->topic);
	MESSAGE("MESSAGE - %s\n", (FTM_CHAR_PTR)pMessage->payload);

	memset(pTopic, 0, sizeof(pTopic));
	strncpy(pTopic, pMessage->topic, FTM_MQTT_CLIENT_TOPIC_LENGTH);

	FTM_MQTT_CLIENT_topicParser(pTopic, pIDs, 10, &nIDs);
	if ((nIDs != 5) || (strcmp(pClient->pDID, pIDs[3]) != 0))
	{
		ERROR("Invalid Topic[%s]\n", pMessage->topic);
		return;
	}

	if(strcmp(pIDs[4], "req") == 0)
	{
		FTNM_MSG_PTR pMsg;

		xRet = FTM_MQTT_CLIENT_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Invalid message.\n");
			return;
		}

		xRet = FTNM_MSGQ_push(pClient->pCTX->pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pMsg);
			ERROR("Message queue push failed.\n");
			return;
		}
	}
}

FTM_VOID FTM_MQTT_CLIENT_subscribeCB
(
	struct mosquitto 	*mosq, 
	void				*pObj,
	int					nMID, 
	int					nQoS, 
	const int			*pGrantedQoS
)
{
	ASSERT(pObj != NULL);
	//FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;
}

FTM_RET	FTM_MQTT_CLIENT_topicParser
(
	FTM_CHAR_PTR	pTopic,
	FTM_CHAR_PTR	pArgv[],
	FTM_INT			nMaxArgc,
	FTM_INT_PTR		pnArgc
)
{
	ASSERT(pTopic != NULL);
	ASSERT(pArgv != NULL);
	ASSERT(pnArgc != NULL);

	FTM_CHAR_PTR	pPointer;
	FTM_CHAR_PTR	pToken;
	FTM_INT			nArgc = 0;

	pPointer = pTopic;

	while((pToken = strtok(pPointer, "/")) != NULL)
	{
		pArgv[nArgc++] = pToken;
		if (nMaxArgc <= nArgc)
		{
			break;
		}
		pPointer = pPointer + strlen(pToken) + 1;
	}

	*pnArgc = nArgc;

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_publishEPData
(
	FTM_MQTT_CLIENT_PTR pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;
	FTM_CHAR	pValue[32];

	FTM_EP_DATA_snprint(pValue, sizeof(pValue), pData);

	sprintf(pTopic, "v/s/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "{");
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "'id': '%08lx',", xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "'value': '%s',", pValue);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "'time': %lu", pData->ulTime);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "}" );

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

struct
{
	FTM_CHAR_PTR	pString;
	FTM_ULONG		xMethod;
} xReqMethod[] =
{
	{
		.pString	= "timeSync",
		.xMethod		= FTM_MQTT_METHOD_REQ_TIME_SYNC
	},
	{
		.pString	= "controlACtuator",
		.xMethod		= FTM_MQTT_METHOD_REQ_CONTROL_ACTUATOR,
	},
	{
		.pString	= "setProperty",
		.xMethod		= FTM_MQTT_METHOD_REQ_SET_PROPERTY,
	},
	{
		.pString	= "poweroff",
		.xMethod		= FTM_MQTT_METHOD_REQ_POWER_OFF,
	},
	{
		.pString	= "reboot",
		.xMethod		= FTM_MQTT_METHOD_REQ_REBOOT,
	},
	{
		.pString	= "restart",
		.xMethod		= FTM_MQTT_METHOD_REQ_RESTART,
	},
	{
		.pString	= "swUpdate",
		.xMethod		= FTM_MQTT_METHOD_REQ_SW_UPDATE,
	},
	{
		.pString	= "swInfo",
		.xMethod		= FTM_MQTT_METHOD_REQ_SW_INFO,
	}
};

FTM_RET	FTM_MQTT_CLIENT_requestMessageParser
(
	FTM_CHAR_PTR		pMessage,
	FTNM_MSG_PTR _PTR_	ppMsg
)
{
	FTNM_MSG		xMsg;
	FTM_INT			i;

	memset(&xMsg, 0, sizeof(xMsg));

	const nx_json * pJSON = nx_json_parse((FTM_CHAR_PTR)pMessage, nx_json_unicode_to_utf8);
	if (pJSON == NULL)
	{
		ERROR("Invalid Message[%s]\n", pMessage);
		return	FTM_RET_MQTT_INVALID_MESSAGE;
	}

	const nx_json *pItem = nx_json_get(pJSON, "id");
	if ((pItem == NULL) || (pItem->type == NX_JSON_STRING))
	{
		nx_json_free(pJSON);
		goto error;
	}	
	strncpy(xMsg.xParams.xMQTTReq.pReqID, pItem->text_value,sizeof(xMsg.xParams.xMQTTReq.pReqID) - 1);

	pItem = nx_json_get(pJSON, "method");
	if ((pItem == NULL) || (pItem->type == NX_JSON_STRING))
	{
		nx_json_free(pJSON);
		goto error;
	}	

	for(i = 0 ; i < sizeof(xReqMethod) / sizeof(xReqMethod[0]) ; i++)
	{
		if (strcmp(xReqMethod[i].pString, pItem->text_value) == 0)
		{
			xMsg.xParams.xMQTTReq.ulMethod = xReqMethod[i].xMethod;
			break;
		}
	}
	
	pItem = nx_json_get(pJSON, "time");
	if (pItem == NULL)
	{
		nx_json_free(pJSON);
		goto error;
	}	

	xMsg.xParams.xMQTTReq.ulTime = pItem->int_value;
	xMsg.xType = FTNM_MSG_TYPE_MQTT_REQ;

	*ppMsg = (FTNM_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_MSG));
	if (*ppMsg == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(*ppMsg, &xMsg, sizeof(xMsg));

	return	FTM_RET_OK;
error:

	nx_json_free(pJSON);

	return	FTM_RET_MQTT_INVALID_MESSAGE;
}

