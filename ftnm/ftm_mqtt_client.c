#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "ftm.h"
#include "nxjson.h"
#include "ftm_mqtt_client.h"
#include "ftm_mqtt_client_tpgw.h"
#include "ftm_mqtt_client_ft.h"

static FTM_VOID_PTR FTM_MQTT_CLIENT_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTM_MQTT_CLIENT_connector(FTM_VOID_PTR pData);
static FTM_RET	FTM_MQTT_CLIENT_publishEPData(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_EP_DATA_PTR	pData);

static FTM_VOID FTM_MQTT_CLIENT_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID	FTM_MQTT_CLIENT_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
static FTM_VOID FTM_MQTT_CLIENT_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);

static FTM_MQTT_CLIENT_CALLBACK_SET	pCBSet[] =
{
	{
		.fConnect 	= FTM_MQTT_CLIENT_FT_connectCB,
		.fDisconnect= FTM_MQTT_CLIENT_FT_disconnectCB,
		.fPublish 	= FTM_MQTT_CLIENT_FT_publishCB,
		.fMessage 	= FTM_MQTT_CLIENT_FT_messageCB,
		.fSubscribe = FTM_MQTT_CLIENT_FT_subscribeCB,
		.fPublishEPDataINT	= FTM_MQTT_CLIENT_FT_publishEPDataINT,
		.fPublishEPDataULONG= FTM_MQTT_CLIENT_FT_publishEPDataULONG,
		.fPublishEPDataFLOAT= FTM_MQTT_CLIENT_FT_publishEPDataFLOAT,
		.fPublishEPDataBOOL	= FTM_MQTT_CLIENT_FT_publishEPDataBOOL
	},
	{
		.fConnect 	= FTM_MQTT_CLIENT_TPGW_connectCB,
		.fDisconnect= FTM_MQTT_CLIENT_TPGW_disconnectCB,
		.fPublish 	= FTM_MQTT_CLIENT_TPGW_publishCB,
		.fMessage 	= FTM_MQTT_CLIENT_TPGW_messageCB,
		.fSubscribe = FTM_MQTT_CLIENT_TPGW_subscribeCB,
		.fPublishEPData		= FTM_MQTT_CLIENT_TPGW_publishEPData,
		.fPublishEPDataINT	= FTM_MQTT_CLIENT_TPGW_publishEPDataINT,
		.fPublishEPDataULONG= FTM_MQTT_CLIENT_TPGW_publishEPDataULONG,
		.fPublishEPDataFLOAT= FTM_MQTT_CLIENT_TPGW_publishEPDataFLOAT,
		.fPublishEPDataBOOL	= FTM_MQTT_CLIENT_TPGW_publishEPDataBOOL
	},
};

static 	FTM_ULONG	ulClientInstance = 0;

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

	FTNM_getDID(pCTX, pClient->pDID, FTM_DID_LEN);
	strncpy(pClient->xConfig.pClientID, pClient->pDID, sizeof(pClient->xConfig.pClientID) - 1);
	strcpy(pClient->xConfig.xBroker.pHost, FTM_MQTT_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.xBroker.usPort = FTM_MQTT_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulReconnectionTime = FTM_MQTT_CLIENT_DEFAULT_RECONNECTION_TIME;
	pClient->xConfig.ulCBSet = FTM_MQTT_CLIENT_DEFAULT_CB_SET;

	pClient->bStop = FTM_TRUE;
	FTNM_MSGQ_create(&pClient->pMsgQ);

	if (ulClientInstance++ == 0)
	{
		mosquitto_lib_init();
	}

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

	FTNM_MSGQ_destroy(&pClient->pMsgQ);

	if (--ulClientInstance == 0)
	{
		mosquitto_lib_cleanup();
	}

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

	case	FTNM_MSG_TYPE_EP_CHANGED:
		{
			pCBSet[pClient->xConfig.ulCBSet].fPublishEPData(pClient, 
														pMsg->xParams.xEPChanged.xEPID, 
														&pMsg->xParams.xEPChanged.xData);

		}
		break;

	case	FTNM_MSG_TYPE_EP_DATA_TRANS:
		{
			switch(pMsg->xParams.xEPDataTrans.nType)
			{
			case	FTM_EP_DATA_TYPE_INT:
				{
					
					pCBSet[pClient->xConfig.ulCBSet].fPublishEPDataINT(	pClient, 
														pMsg->xParams.xEPDataTrans.xEPID, 
														pMsg->xParams.xEPDataTrans.ulTime,
														pMsg->xParams.xEPDataTrans.xValue.xINT.nValue,
														pMsg->xParams.xEPDataTrans.xValue.xINT.nAverage,
														pMsg->xParams.xEPDataTrans.xValue.xINT.nCount,
														pMsg->xParams.xEPDataTrans.xValue.xINT.nMax,
														pMsg->xParams.xEPDataTrans.xValue.xINT.nMin);
				};
				break;

			case	FTM_EP_DATA_TYPE_ULONG:
				{
					pCBSet[pClient->xConfig.ulCBSet].fPublishEPDataULONG(	pClient, 
														pMsg->xParams.xEPDataTrans.xEPID, 
														pMsg->xParams.xEPDataTrans.ulTime,
														pMsg->xParams.xEPDataTrans.xValue.xULONG.ulValue,
														pMsg->xParams.xEPDataTrans.xValue.xULONG.ulAverage,
														pMsg->xParams.xEPDataTrans.xValue.xULONG.nCount,
														pMsg->xParams.xEPDataTrans.xValue.xULONG.ulMax,
														pMsg->xParams.xEPDataTrans.xValue.xULONG.ulMin);
				};
				break;

			case	FTM_EP_DATA_TYPE_FLOAT:
				{
					pCBSet[pClient->xConfig.ulCBSet].fPublishEPDataFLOAT(	pClient, 
														pMsg->xParams.xEPDataTrans.xEPID, 
														pMsg->xParams.xEPDataTrans.ulTime,
														pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fValue,
														pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fAverage,
														pMsg->xParams.xEPDataTrans.xValue.xFLOAT.nCount,
														pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fMax,
														pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fMin);
				};
				break;

			case	FTM_EP_DATA_TYPE_BOOL:
				{
					pCBSet[pClient->xConfig.ulCBSet].fPublishEPDataBOOL(	pClient, 
														pMsg->xParams.xEPDataTrans.xEPID, 
														pMsg->xParams.xEPDataTrans.ulTime,
														pMsg->xParams.xEPDataTrans.xValue.xBOOL.bValue);
				};
				break;
			}

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

	FTM_RET				xRet;
	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pData;

	pClient->bStop 		= FTM_FALSE;
	pClient->bConnected = FTM_FALSE;
	pClient->pMosquitto = mosquitto_new(pClient->xConfig.pClientID, true, pClient);
	if(pClient->pMosquitto != NULL)
	{
		TRACE("MQTT CLIENT[%s] started.\n", pClient->xConfig.pClientID);

		mosquitto_connect_callback_set(pClient->pMosquitto, 	FTM_MQTT_CLIENT_connectCB);
		mosquitto_disconnect_callback_set(pClient->pMosquitto, 	FTM_MQTT_CLIENT_disconnectCB);
		mosquitto_publish_callback_set(pClient->pMosquitto,  	FTM_MQTT_CLIENT_publishCB);
		mosquitto_subscribe_callback_set(pClient->pMosquitto,  	FTM_MQTT_CLIENT_subscribeCB);
		mosquitto_message_callback_set(pClient->pMosquitto,  	FTM_MQTT_CLIENT_messageCB);

		pthread_create(&pClient->xConnector, NULL, FTM_MQTT_CLIENT_connector, pClient);

		while(!pClient->bStop)
		{
			FTNM_MSG_PTR	pMsg;

			xRet = FTNM_MSGQ_timedPop(pClient->pMsgQ, 1000000, &pMsg);
			if (xRet == FTM_RET_OK)
			{
				FTNM_MSG_destroy(&pMsg);
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

FTM_VOID_PTR FTM_MQTT_CLIENT_connector(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pData;

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

FTM_RET	FTM_MQTT_CLIENT_publishEPData
(
	FTM_MQTT_CLIENT_PTR pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	switch(pData->xType)
	{
	case	FTM_EP_DATA_TYPE_INT:	
		return	FTM_MQTT_CLIENT_TPGW_publishEPDataINT(pClient, xEPID, pData->ulTime, pData->xValue.nValue, pData->xValue.nValue, 1, pData->xValue.nValue, pData->xValue.nValue);
	case	FTM_EP_DATA_TYPE_ULONG:	
		return	FTM_MQTT_CLIENT_TPGW_publishEPDataULONG(pClient, xEPID, pData->ulTime, pData->xValue.ulValue, pData->xValue.ulValue, 1, pData->xValue.ulValue, pData->xValue.ulValue);
	case	FTM_EP_DATA_TYPE_FLOAT:	
		return	FTM_MQTT_CLIENT_TPGW_publishEPDataFLOAT(pClient, xEPID, pData->ulTime, pData->xValue.fValue, pData->xValue.fValue, 1, pData->xValue.fValue, pData->xValue.fValue);
	case	FTM_EP_DATA_TYPE_BOOL:	
		return	FTM_MQTT_CLIENT_TPGW_publishEPDataBOOL(pClient,xEPID,  pData->ulTime, pData->xValue.bValue);
	}

	return	FTM_RET_OK;
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

	TRACE("MQTT is connected.\n");
	pClient->bConnected = FTM_TRUE;

	if (pCBSet[pClient->xConfig.ulCBSet].fConnect != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fConnect(mosq, pObj, nResult);
	}
}

FTM_VOID FTM_MQTT_CLIENT_disconnectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT is disconnected.\n");
	if (pCBSet[pClient->xConfig.ulCBSet].fDisconnect != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fDisconnect(mosq, pObj, nResult);
	}

	FTM_TIMER_init(&pClient->xReconnectionTimer, 0);
	FTM_TIMER_addSeconds(&pClient->xReconnectionTimer, pClient->xConfig.ulReconnectionTime);

	pClient->bConnected = FTM_FALSE;

}

FTM_VOID FTM_MQTT_CLIENT_publishCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT published.\n");
	if (pCBSet[pClient->xConfig.ulCBSet].fPublish != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fPublish(mosq, pObj, nResult);
	}
}

FTM_VOID FTM_MQTT_CLIENT_messageCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	const struct mosquitto_message *pMessage
)
{
	ASSERT(pObj != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;

	TRACE("MQTT received message.\n");
	if (pCBSet[pClient->xConfig.ulCBSet].fMessage != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fMessage(mosq, pObj, pMessage);
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

	TRACE("MQTT subscribe.\n");
	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;

	if (pCBSet[pClient->xConfig.ulCBSet].fSubscribe != NULL)
	{
		pCBSet[pClient->xConfig.ulCBSet].fSubscribe(mosq, pObj, nMID, nQoS, pGrantedQoS);
	}
}


