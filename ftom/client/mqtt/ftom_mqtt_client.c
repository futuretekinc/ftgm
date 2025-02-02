#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "ftm.h"
#include "ftom_message_queue.h"
#include "ftom_mqtt_client.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_MQTTC

static FTM_VOID_PTR FTOM_MQTT_CLIENT_process(FTM_VOID_PTR pData);

static FTM_RET	FTOM_MQTT_CLIENT_onGWStatus
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTOM_MSG_GW_STATUS_PTR	pMsg
);

static FTM_VOID_PTR FTOM_MQTT_CLIENT_connector
(
	FTM_VOID_PTR 		pData
);

static
FTM_RET	FTOM_MQTT_CLIENT_subscribeAll
(
	FTOM_MQTT_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_MQTT_CLIENT_unsubscribeAll
(
	FTOM_MQTT_CLIENT_PTR	pClient
);

static 
FTM_VOID FTOM_MQTT_CLIENT_connectCB
(
	struct mosquitto *mosq, 
	void *pObj, 
	int nResult
);

static 
FTM_VOID FTOM_MQTT_CLIENT_disconnectCB
(
	struct mosquitto *mosq, 
	void *pObj, 
	int nResult
);

static 
FTM_VOID FTOM_MQTT_CLIENT_publishCB
(
	struct mosquitto *mosq, 
	void *pObj, 
	int nResult
);

static 
FTM_VOID FTOM_MQTT_CLIENT_messageCB
(
	struct mosquitto *mosq, 
	void *pObj, 
	const struct mosquitto_message *message
);

static 
FTM_VOID FTOM_MQTT_CLIENT_subscribeCB
(
	struct mosquitto *mosq, 
	void *pObj, 
	int nMID, 
	int nQoS, 
	const int *pGrantedQoS
);

static
FTM_BOOL FTOM_MQTT_SUBSCRIBE_LIST_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static
FTM_BOOL FTOM_MQTT_PUBLISH_LIST_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
);

static 
FTOM_MQTT_CLIENT_CBSET	xCBDefaultSet =
{
	.fConnect 	= NULL,
	.fDisconnect= NULL,
	.fPublish 	= NULL,
	.fMessage 	= NULL,
	.fSubscribe = NULL,
	.fGWStatus	= NULL,
	.fEPStatus	= NULL,
	.fEPData	= NULL,
	.fTPResponse= NULL,
};

static 	FTM_ULONG	ulClientInstance = 0;

FTM_RET	FTOM_MQTT_CLIENT_create
(
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

	xRet = FTOM_MQTT_CLIENT_init(pClient);
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
	FTOM_MQTT_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;	
	if (pClient->pMsgQ != NULL)
	{
		ERROR2(FTM_RET_ALREADY_INITIALIZED, "Already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	memset(pClient, 0, sizeof(FTOM_MQTT_CLIENT));

	//FTOM_getDID(pClient->pDID, FTM_DID_LEN);
	strcpy(pClient->xConfig.pHost, FTOM_MQTT_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.usPort = FTOM_MQTT_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulRetryInterval = FTOM_MQTT_CLIENT_DEFAULT_RECONNECTION_TIME;
	pClient->pCBSet = &xCBDefaultSet;

	pClient->bStop = FTM_TRUE;
	FTOM_MSGQ_create(&pClient->pMsgQ);

	xRet = FTM_LIST_create(&pClient->pPublishList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT publish list creation failed!\n");	
		FTOM_MSGQ_destroy(&pClient->pMsgQ);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pClient->pSubscribeList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT subscribe list creation failed!\n");	
		FTM_LIST_destroy(pClient->pPublishList);
		pClient->pPublishList = NULL;
		FTOM_MSGQ_destroy(&pClient->pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pClient->pPublishList, FTOM_MQTT_PUBLISH_LIST_seeker);
	FTM_LIST_setSeeker(pClient->pSubscribeList, FTOM_MQTT_SUBSCRIBE_LIST_seeker);

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

	FTOM_MQTT_PUBLISH_PTR 	pPublish;
	FTM_CHAR_PTR			pSubscribe;

	if (pClient->pMsgQ == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTOM_MQTT_CLIENT_stop(pClient);

	FTM_LIST_iteratorStart(pClient->pSubscribeList);
	while(FTM_LIST_iteratorNext(pClient->pSubscribeList, (FTM_VOID_PTR _PTR_)&pSubscribe) == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pSubscribeList, pSubscribe);
		FTM_MEM_free(pSubscribe);
	}

	FTM_LIST_iteratorStart(pClient->pPublishList);
	while(FTM_LIST_iteratorNext(pClient->pPublishList, (FTM_VOID_PTR _PTR_)&pPublish) == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pPublishList, pPublish);
		FTOM_MQTT_PUBLISH_destroy(&pPublish);
	}

	FTM_LIST_destroy(pClient->pSubscribeList);
	pClient->pSubscribeList = NULL;

	FTM_LIST_destroy(pClient->pPublishList);
	pClient->pPublishList = NULL;

	FTOM_MSGQ_destroy(&pClient->pMsgQ);

	if (--ulClientInstance == 0)
	{
		mosquitto_lib_cleanup();
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_setConfig
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

FTM_RET	FTOM_MQTT_CLIENT_setMessageCB
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_MQTT_CLIENT_MESSAGE_CB	fMessageCB,
	FTM_VOID_PTR			pObject
)
{
	ASSERT(pClient != NULL);

	pClient->pMessageCBObject = pObject;
	pClient->fMessageCB	= fMessageCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_setCBSet
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_MQTT_CLIENT_CBSET_PTR	pCBSet
)
{
	ASSERT(pClient != NULL);

	pClient->pCBSet = pCBSet;

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
	default:
		{
			ERROR2(FTM_RET_INVALID_MESSAGE_TYPE, "Not supported message[%08x]!\n", pMsg->xType);	
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_onGWStatus
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTOM_MSG_GW_STATUS_PTR	pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	TRACE("pClient = %08x, pClient->pCBSet = %08x\n", pClient, pClient->pCBSet);
	if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fGWStatus != NULL))
	{
		xRet = pClient->pCBSet->fGWStatus(pClient, pMsg->pGatewayID, pMsg->bStatus, pMsg->ulTimeout);
	}
	else
	{
		xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
	}

	return	xRet;
}

FTM_RET	FTOM_MQTT_CLIENT_onTPResponse
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTOM_MSG_TP_RESPONSE_PTR	pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fTPResponse != NULL))
	{
		xRet = pClient->pCBSet->fTPResponse(pClient, pMsg->pMsgID, pMsg->nCode, pMsg->pMessage);
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
	
	TRACE("MQTT CLIENT[%s] started.\n", pClient->xConfig.pGatewayID);
	if (pClient->bStop != FTM_TRUE)
	{
		INFO("MQTT client is already started!\n");
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
	
	pClient->bStop = FTM_TRUE;

	pthread_join(pClient->xMain, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_setParent
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTM_VOID_PTR			pParent
)
{
	ASSERT(pClient != NULL);
	
	pClient->pParent = pParent;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_isConnected
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_BOOL_PTR		 pbConnected
)
{
	ASSERT(pClient != NULL);
	ASSERT(pbConnected != NULL);

	*pbConnected = pClient->bConnected;
	
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
	pClient->pMosquitto = mosquitto_new(pClient->xConfig.pGatewayID, true, pClient);
	if(pClient->pMosquitto == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "MQTT instance creation failed!\n");
		return	0;
	}

	TRACE("MQTT CLIENT[%s] started.\n", pClient->xConfig.pGatewayID);

	if (pClient->xConfig.bTLS)
	{
		TRACE("UserID : %s\n", pClient->xConfig.pUserID);
		TRACE("Passwd : %s\n", pClient->xConfig.pPasswd);
		TRACE("  Cert : %s\n", pClient->xConfig.pCertFile);
		mosquitto_username_pw_set(pClient->pMosquitto, pClient->xConfig.pUserID, pClient->xConfig.pPasswd);
		mosquitto_tls_set(pClient->pMosquitto, pClient->xConfig.pCertFile, NULL, NULL, NULL, NULL);
		mosquitto_tls_insecure_set(pClient->pMosquitto, 1L);
		mosquitto_tls_opts_set(pClient->pMosquitto, 1, NULL, NULL);
	}

	mosquitto_connect_callback_set(pClient->pMosquitto, 	FTOM_MQTT_CLIENT_connectCB);
	mosquitto_disconnect_callback_set(pClient->pMosquitto, 	FTOM_MQTT_CLIENT_disconnectCB);
	mosquitto_publish_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_publishCB);
	mosquitto_subscribe_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_subscribeCB);
	mosquitto_message_callback_set(pClient->pMosquitto,  	FTOM_MQTT_CLIENT_messageCB);

	pthread_create(&pClient->xLinkManager, NULL, FTOM_MQTT_CLIENT_connector, pClient);

	while(!pClient->bStop)
	{
		FTOM_MSG_PTR	pBaseMsg;

		xRet = FTOM_MSGQ_timedPop(pClient->pMsgQ, 100, &pBaseMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pBaseMsg->xType)
			{
			case	FTOM_MSG_TYPE_GW_STATUS:
				{
					xRet = FTOM_MQTT_CLIENT_onGWStatus(pClient, (FTOM_MSG_GW_STATUS_PTR)pBaseMsg);	
				}
				break;

			case	FTOM_MSG_TYPE_EP_STATUS:
				{
					if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fEPStatus != NULL))
					{
						FTOM_MSG_EP_STATUS_PTR	pMsg =(FTOM_MSG_EP_STATUS_PTR)pBaseMsg;

						xRet = pClient->pCBSet->fEPStatus(pClient, pMsg->pEPID, pMsg->bStatus, pMsg->ulTimeout);
					}
					else
					{
						xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
					}
				}
				break;

			case	FTOM_MSG_TYPE_EP_DATA:
				{
					if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fEPData != NULL))
					{
						FTOM_MSG_EP_DATA_PTR	pMsg =(FTOM_MSG_EP_DATA_PTR)pBaseMsg;

						xRet = pClient->pCBSet->fEPData(pClient, pMsg->pEPID, pMsg->pData, pMsg->ulCount);
					}
					else
					{
						xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;	
					}
				}
				break;

			case	FTOM_MSG_TYPE_TP_RESPONSE:
				{
					xRet = FTOM_MQTT_CLIENT_onTPResponse(pClient, (FTOM_MSG_TP_RESPONSE_PTR)pBaseMsg);
				}
				break;

			default:
				{
					ERROR2(FTM_RET_INVALID_MESSAGE_TYPE, "Not supported msg[%08x]\n", pBaseMsg->xType);	
				}
			}
			FTOM_MSG_destroy(&pBaseMsg);
		}

		if (pClient->ulNewSubscribe != 0)
		{
			FTOM_MQTT_CLIENT_subscribeAll(pClient);	
		}
	}

	pthread_join(pClient->xLinkManager, NULL);

	TRACE("MQTT CLIENT[%s] stopped.\n", pClient->xConfig.pGatewayID);

	mosquitto_destroy(pClient->pMosquitto);
	pClient->pMosquitto = NULL;
	
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

	TRACE("Host : %s\n", pClient->xConfig.pHost);
	TRACE("Port : %d\n", pClient->xConfig.usPort);
	mosquitto_connect_async(pClient->pMosquitto, pClient->xConfig.pHost, pClient->xConfig.usPort, 60);
	mosquitto_loop_start(pClient->pMosquitto);
	FTM_TIMER_initS(&pClient->xLinkTimer, pClient->xConfig.ulRetryInterval);

	while(!pClient->bStop)
	{
		if(!pClient->bStop && !pClient->bConnected)
		{
			if (FTM_TIMER_isExpired(&pClient->xLinkTimer))
			{
				TRACE("MQTT try reconnection !\n");
				mosquitto_reconnect_async(pClient->pMosquitto);
				FTM_TIMER_addS(&pClient->xLinkTimer, pClient->xConfig.ulRetryInterval);
			}
		}

		usleep(1000);
	}
	TRACE("MQTT connector was stopped.\n");

	return 0;
}

FTM_RET	FTOM_MQTT_CLIENT_reportGWStatus
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;
	FTOM_MSG_GW_STATUS_PTR	pMsg;

	xRet = FTOM_MSG_createGWStatus(pClient, pGatewayID, bStatus, ulTimeout, &pMsg);
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


FTM_RET	FTOM_MQTT_CLIENT_publishEPStatus
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createEPStatus(pClient, pEPID, bStatus, ulTimeout, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MQTT_CLIENT_pushMsg(pClient, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}


FTM_RET	FTOM_MQTT_CLIENT_publishEPData
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createEPData(pClient, pEPID, pData, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MQTT_CLIENT_pushMsg(pClient, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}


FTM_RET	FTOM_MQTT_CLIENT_response
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nCode,
	FTM_CHAR_PTR		pMessage
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsgID != NULL);
	ASSERT(pMessage != NULL);

	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_TP_createResponse(pClient, pMsgID, nCode, pMessage, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MQTT_CLIENT_pushMsg(pClient, pMsg);
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
	FTM_ULONG				ulMessageLen,
	FTM_VOID_PTR			pData,
	FTM_ULONG_PTR			pulMessageID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTopic != NULL);
	ASSERT(pMessage != NULL);

	FTM_RET	xRet;
	FTM_INT	nRet;
	FTOM_MQTT_PUBLISH_PTR	pPublish;

	xRet = FTOM_MQTT_PUBLISH_create(pTopic, pMessage, ulMessageLen, 1, pData, &pPublish);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	nRet = mosquitto_publish(pClient->pMosquitto, &pPublish->nMessageID, 
				pPublish->pTopic, pPublish->ulMessageLen, pPublish->pMessage, pPublish->ulQoS, 0);
	switch(nRet)
	{
	case	MOSQ_ERR_SUCCESS:
		{
			if (pulMessageID != NULL)
			{
				*pulMessageID = pPublish->nMessageID;	
			}

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
			xRet = FTM_RET_INVALID_ARGUMENTS;
			ERROR2(xRet, "Invalid arguments.\n");
		}
		break;

	case	MOSQ_ERR_NOMEM:
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR2(xRet, "Not enoguh memory!.\n");
		}
		break;

	case	MOSQ_ERR_NO_CONN:
		{
			xRet = FTM_RET_NOT_CONNECTED;
			ERROR2(xRet, "Not connected.\n");
		}
		break;

	case	MOSQ_ERR_PROTOCOL:
		{
			xRet = FTM_RET_ERROR;
			ERROR2(xRet, "Protocol error!\n");
		}
		break;

	case	MOSQ_ERR_PAYLOAD_SIZE:
		{
			xRet = FTM_RET_PAYLOAD_IS_TOO_LARGE;
			ERROR2(xRet, "Payload is too large.\n");
		}
		break;

	default:
		{
			xRet = FTM_RET_ERROR;
			ERROR2(xRet, "Unknown error[%d]\n", nRet);
		}
	}
	
	return	xRet;
}

FTM_RET	FTOM_MQTT_CLIENT_subscribeAll
(
	FTOM_MQTT_CLIENT_PTR	pClient
)
{
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount = 0;

	if (pClient->bConnected == FTM_FALSE)
	{
		return	FTM_RET_OK;	
	}

	FTM_LIST_count(pClient->pSubscribeList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_MQTT_SUBSCRIBE_PTR	pSubscribe;

		xRet = FTM_LIST_getAt(pClient->pSubscribeList, i, (FTM_VOID_PTR _PTR_)&pSubscribe);	
		if (xRet == FTM_RET_OK)
		{
			mosquitto_subscribe(pClient->pMosquitto, &pSubscribe->nMessageID, pSubscribe->pTopic, 0);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_unsubscribeAll
(
	FTOM_MQTT_CLIENT_PTR	pClient
)
{
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount = 0;

	FTM_LIST_count(pClient->pSubscribeList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_MQTT_SUBSCRIBE_PTR	pSubscribe;

		xRet = FTM_LIST_getAt(pClient->pSubscribeList, i, (FTM_VOID_PTR _PTR_)&pSubscribe);	
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	nRet;
		
			if (pClient->bConnected)
			{
				nRet = mosquitto_unsubscribe(pClient->pMosquitto, NULL, pSubscribe->pTopic);
				if (nRet == MOSQ_ERR_SUCCESS)
				{
					pSubscribe->bRegisted = FTM_FALSE;
				}
			}
			else
			{
				pSubscribe->bRegisted = FTM_FALSE;
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_CLIENT_subscribe
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pTopic
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTopic != NULL);
	FTM_RET			xRet;
	FTOM_MQTT_SUBSCRIBE_PTR	pSubscribe;

	FTM_LIST_iteratorStart(pClient->pSubscribeList);
	while(FTM_LIST_iteratorNext(pClient->pSubscribeList, (FTM_VOID_PTR _PTR_)&pSubscribe) == FTM_RET_OK)
	{
		if (strcmp(pSubscribe->pTopic, pTopic) == 0)
		{
			return	FTM_RET_ALREADY_EXISTS;	
		}
	}

	xRet = FTOM_MQTT_SUBSCRIBE_create(pTopic, &pSubscribe);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_LIST_append(pClient->pSubscribeList, pSubscribe);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MQTT_SUBSCRIBE_destroy(&pSubscribe);
	}
	else
	{
		pClient->ulNewSubscribe++;
		if (pClient->bConnected)
		{
			mosquitto_subscribe(pClient->pMosquitto, &pSubscribe->nMessageID, pSubscribe->pTopic, 0);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_MQTT_CLIENT_unsubscribe
(
	FTOM_MQTT_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pTopic
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTopic != NULL);

	FTM_RET			xRet;
	FTOM_MQTT_SUBSCRIBE_PTR	pSubscribe;

	FTM_LIST_iteratorStart(pClient->pSubscribeList);
	while(FTM_LIST_iteratorNext(pClient->pSubscribeList, (FTM_VOID_PTR _PTR_)&pSubscribe) == FTM_RET_OK)
	{
		if (strcmp(pSubscribe->pTopic, pTopic) == 0)
		{
			if (pClient->bConnected)
			{
				mosquitto_unsubscribe(pClient->pMosquitto, NULL, pSubscribe->pTopic);
			}

			xRet = FTM_LIST_remove(pClient->pSubscribeList, pSubscribe);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "MQTT subscribe remove failed!\n");
				return	FTM_RET_ERROR;	
			}

			if (pSubscribe->bRegisted != FTM_TRUE)
			{
				if (pClient->ulNewSubscribe > 0)
				{
					pClient->ulNewSubscribe--;
				}
			}

			xRet = FTOM_MQTT_SUBSCRIBE_destroy(&pSubscribe);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "MQTT subscribe destroy failed!\n");
			}

			return	xRet ;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
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

	TRACE("MQTT is connected!\n");
	if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fConnect != NULL))
	{
		pClient->pCBSet->fConnect(mosq, pObj, nResult);
	}
	else
	{
		TRACE("MQTT client[%s] is connected.\n", pClient->xConfig.pGatewayID);
		pClient->bConnected = FTM_TRUE;
	}

	FTOM_MQTT_CLIENT_subscribeAll(pClient);
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
	if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fDisconnect != NULL))
	{
		pClient->pCBSet->fDisconnect(mosq, pObj, nResult);
	}
	else
	{
		FTM_TIMER_initS(&pClient->xLinkTimer, 0);
		FTM_TIMER_addS(&pClient->xLinkTimer, pClient->xConfig.ulRetryInterval);

		pClient->bConnected = FTM_FALSE;
	}

	FTOM_MQTT_CLIENT_unsubscribeAll(pClient);
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


	if ((pClient->pCBSet != NULL) &&(pClient->pCBSet->fPublish != NULL))
	{
		pClient->pCBSet->fPublish(mosq, pObj, nMID);
	}
	else
	{
		xRet = FTM_LIST_get(pClient->pPublishList, &nMID, (FTM_VOID_PTR _PTR_)&pPublish);
		if (xRet == FTM_RET_OK)
		{
			TRACE("Publish[%04d] success!\n", pPublish->nMessageID);
			FTM_LIST_remove(pClient->pPublishList, pPublish);	
			FTOM_MQTT_PUBLISH_destroy(&pPublish);
		}
		else
		{
			WARN("Publish[%08x] not found!\n", nMID);
		}
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
	if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fMessage != NULL))
	{
		pClient->pCBSet->fMessage(mosq, pObj, pMessage);
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

	FTM_RET					xRet;
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;
	FTOM_MQTT_SUBSCRIBE_PTR	pSubscribe = NULL;

	if ((pClient->pCBSet != NULL) && (pClient->pCBSet->fSubscribe != NULL))
	{
		pClient->pCBSet->fSubscribe(mosq, pObj, nMID, nQoS, pGrantedQoS);
	}

	xRet = FTM_LIST_get(pClient->pSubscribeList, &nMID, (FTM_VOID_PTR _PTR_)&pSubscribe); 
	if (xRet == FTM_RET_OK)
	{
		TRACE("Subscription for a topic[%s] has been completed.\n", pSubscribe->pTopic);
		if (pSubscribe->bRegisted == FTM_FALSE)
		{
			if (pClient->ulNewSubscribe > 0)
			{
				pClient->ulNewSubscribe--;
			}
			pSubscribe->bRegisted = FTM_TRUE;	
		}
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

FTM_BOOL FTOM_MQTT_SUBSCRIBE_LIST_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pKey
)
{
	ASSERT(pElement != NULL);
	ASSERT(pKey != NULL);

	FTOM_MQTT_SUBSCRIBE_PTR pSubscribe = (FTOM_MQTT_SUBSCRIBE_PTR)pElement;
	FTM_INT_PTR		 pMessageID= (FTM_INT_PTR)pKey;

	return	(pSubscribe->nMessageID == *pMessageID);
}

FTM_RET	FTOM_MQTT_PUBLISH_create
(
	FTM_CHAR_PTR	pTopic,
	FTM_CHAR_PTR	pMessage,
	FTM_ULONG		ulMessageLen,
	FTM_ULONG		ulQoS,
	FTM_VOID_PTR	pData,
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
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pPublish->pTopic 		= (FTM_CHAR_PTR)pPublish + sizeof(FTOM_MQTT_PUBLISH);
	pPublish->pMessage		= (FTM_CHAR_PTR)pPublish + sizeof(FTOM_MQTT_PUBLISH) + strlen(pTopic) + 1;

	strcpy(pPublish->pTopic, pTopic);
	memcpy(pPublish->pMessage, pMessage, ulMessageLen);
	pPublish->ulMessageLen 	= ulMessageLen;
	pPublish->ulQoS     	= ulQoS;
	pPublish->nMessageID	= 0;
	pPublish->pData			= pData;

	*ppPublish = pPublish;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_PUBLISH_destroy
(
	FTOM_MQTT_PUBLISH_PTR _PTR_ ppPublish
)
{
	ASSERT(ppPublish != NULL);

	if ((*ppPublish)->pData != NULL)
	{
		FTM_MEM_free((*ppPublish)->pData);
	}

	FTM_MEM_free(*ppPublish);

	*ppPublish = NULL;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_MQTT_SUBSCRIBE_create
(
	FTM_CHAR_PTR	pTopic,
	FTOM_MQTT_SUBSCRIBE_PTR _PTR_ ppSubscribe
)
{
	ASSERT(pTopic != NULL);
	ASSERT(ppSubscribe != NULL);

	FTOM_MQTT_SUBSCRIBE_PTR	pSubscribe;

	pSubscribe = (FTOM_MQTT_SUBSCRIBE_PTR)FTM_MEM_malloc(sizeof(FTOM_MQTT_SUBSCRIBE) + strlen(pTopic) + 1);
	if (pSubscribe == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pSubscribe->pTopic = (FTM_CHAR_PTR)pSubscribe + sizeof(FTOM_MQTT_SUBSCRIBE);
	
	pSubscribe->bRegisted = FTM_FALSE;
	strcpy(pSubscribe->pTopic, pTopic);

	*ppSubscribe = pSubscribe;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_MQTT_SUBSCRIBE_destroy
(
	FTOM_MQTT_SUBSCRIBE_PTR _PTR_ ppSubscribe
)
{
	ASSERT(ppSubscribe != NULL);

	FTM_MEM_free(*ppSubscribe);

	*ppSubscribe = NULL;

	return	FTM_RET_OK;
}
