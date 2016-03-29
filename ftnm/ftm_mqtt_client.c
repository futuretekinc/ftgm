#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "ftm.h"
#include "nxjson.h"
#include "ftm_mqtt_client.h"


typedef	FTM_VOID (*FTM_MQTT_CLIENT_CONNECT_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTM_MQTT_CLIENT_DISCONNECT_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTM_MQTT_CLIENT_PUBLISH_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTM_MQTT_CLIENT_MESSAGE_CB)(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
typedef FTM_VOID (*FTM_MQTT_CLIENT_SUBSCRIBE_CB)(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);
typedef FTM_VOID (*FTM_MQTT_CLIENT_TIMER_CB)(struct mosquitto *mosq, void *pObj);
typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_INT)(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue, FTM_INT nAverage, FTM_INT nCount, FTM_INT nMax, FTM_INT nMin);
typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_ULONG)(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue,FTM_ULONG nAverage, FTM_INT nCount, FTM_ULONG ulMax, FTM_ULONG ulMin);
typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_FLOAT)(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_FLOAT fValue, FTM_FLOAT nAverage, FTM_INT nCount, FTM_FLOAT fMax, FTM_FLOAT fMin);
typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_BOOL)(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_BOOL bValu);

typedef	struct
{
	FTM_MQTT_CLIENT_CONNECT_CB				fConnect;
	FTM_MQTT_CLIENT_DISCONNECT_CB			fDisconnect;
	FTM_MQTT_CLIENT_PUBLISH_CB				fPublish;
	FTM_MQTT_CLIENT_MESSAGE_CB				fMessage;
	FTM_MQTT_CLIENT_SUBSCRIBE_CB			fSubscribe;
	FTM_MQTT_CLIENT_TIMER_CB				fTimer;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_INT		fPublishEPDataINT;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_ULONG	fPublishEPDataULONG;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_FLOAT	fPublishEPDataFLOAT;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_BOOL	fPublishEPDataBOOL;
}	FTM_MQTT_CLIENT_CALLBACK_SET, _PTR_ FTM_MQTT_CLIENT_CALLBACK_SET_PTR;

static FTM_VOID_PTR FTM_MQTT_CLIENT_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTM_MQTT_CLIENT_connector(FTM_VOID_PTR pData);
static FTM_RET	FTM_MQTT_CLIENT_publishEPData(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_EP_DATA_PTR	pData);

static FTM_VOID FTM_MQTT_CLIENT_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID	FTM_MQTT_CLIENT_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
static FTM_VOID FTM_MQTT_CLIENT_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);

static FTM_VOID FTM_MQTT_CLIENT_FT_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_FT_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_FT_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID	FTM_MQTT_CLIENT_FT_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
static FTM_VOID FTM_MQTT_CLIENT_FT_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);

static FTM_RET	FTM_MQTT_CLIENT_FT_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
static FTM_RET	FTM_MQTT_CLIENT_FT_requestMessageParser(FTM_CHAR_PTR pMessage, FTNM_MSG_PTR _PTR_	pMsg);
static FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataINT(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue, FTM_INT nAverage, FTM_INT nCount, FTM_INT nMax, FTM_INT nMin);
static FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataULONG(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue, FTM_ULONG nAverage, FTM_INT nCount, FTM_ULONG ulMax, FTM_ULONG ulMin);
static FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataFLOAT(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_FLOAT fValue, FTM_FLOAT nAverage, FTM_INT nCount, FTM_FLOAT fMax, FTM_FLOAT fMin);
static FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataBOOL(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_BOOL bValu);

static FTM_VOID FTM_MQTT_CLIENT_TPGW_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_TPGW_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID FTM_MQTT_CLIENT_TPGW_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
static FTM_VOID	FTM_MQTT_CLIENT_TPGW_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
static FTM_VOID FTM_MQTT_CLIENT_TPGW_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);

static FTM_RET	FTM_MQTT_CLIENT_TPGW_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
static FTM_RET	FTM_MQTT_CLIENT_TPGW_requestMessageParser(FTM_CHAR_PTR pMessage, FTNM_MSG_PTR _PTR_	pMsg);
static FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataINT(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue, FTM_INT nAverage, FTM_INT nCount, FTM_INT nMax, FTM_INT nMin);
static FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataULONG(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue, FTM_ULONG nAverage, FTM_INT nCount, FTM_ULONG ulMax, FTM_ULONG ulMin);
static FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataFLOAT(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_FLOAT fValue, FTM_FLOAT nAverage, FTM_INT nCount, FTM_FLOAT fMax, FTM_FLOAT fMin);
static FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataBOOL(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_BOOL bValu);

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

	FTNM_getDID(pClient->pDID, FTM_DID_LEN);
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


/**************************************************************
 * Futuretek MQTT interface
 **************************************************************/
FTM_VOID FTM_MQTT_CLIENT_FT_connectCB
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

FTM_VOID FTM_MQTT_CLIENT_FT_disconnectCB
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

FTM_VOID FTM_MQTT_CLIENT_FT_publishCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
}

FTM_VOID FTM_MQTT_CLIENT_FT_messageCB
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
	
	memset(pTopic, 0, sizeof(pTopic));
	strncpy(pTopic, pMessage->topic, FTM_MQTT_CLIENT_TOPIC_LENGTH);

	FTM_MQTT_CLIENT_FT_topicParser(pTopic, pIDs, 10, &nIDs);
	if ((nIDs != 5) || (strcmp(pClient->pDID, pIDs[3]) != 0))
	{
		ERROR("Invalid Topic[%s]\n", pMessage->topic);
		return;
	}

	if(strcmp(pIDs[4], "req") == 0)
	{
		FTNM_MSG_PTR pMsg;

		xRet = FTM_MQTT_CLIENT_FT_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
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

FTM_VOID FTM_MQTT_CLIENT_FT_subscribeCB
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

FTM_RET	FTM_MQTT_CLIENT_FT_topicParser
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

FTM_RET	FTM_MQTT_CLIENT_FT_requestMessageParser
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

FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataINT
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_INT 			nValue, 
	FTM_INT 			nAverage, 
	FTM_INT 			nCount, 
	FTM_INT 			nMax, 
	FTM_INT 			nMin
)
{
	ASSERT(pClient != NULL);
	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, nValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataULONG
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_ULONG 			ulValue, 
	FTM_ULONG 			ulAverage, 
	FTM_INT 			nCount, 
	FTM_ULONG 			ulMax, 
	FTM_ULONG 			ulMin
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%lu]", ulTime, ulValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataFLOAT
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_FLOAT			fValue,
	FTM_FLOAT 			fAverage, 
	FTM_INT 			nCount, 
	FTM_FLOAT 			fMax, 
	FTM_FLOAT 			fMin
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%5.3f]", ulTime, fValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_FT_publishEPDataBOOL
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_BOOL 			bValue
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, bValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

/***********************************************************************************
 * Thing+Gateway MQTT interface
 ***********************************************************************************/

FTM_VOID FTM_MQTT_CLIENT_TPGW_connectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);
	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pObj;
	FTM_CHAR			pTopic[FTM_MQTT_TOPIC_LEN + 1];
	FTM_CHAR			pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG			ulMessageLen = 0;

	TRACE("MQTT is connected.\n");

	sprintf(pTopic, "v/a/g/%s/mqtt/status", pClient->pDID);
	ulMessageLen = sprintf(pMessage, "on");
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	sprintf(pTopic, "v/a/g/%s/res", pClient->pDID);
	mosquitto_subscribe(pClient->pMosquitto, NULL, pTopic, 0);
	sprintf(pTopic, "v/a/g/%s/req", pClient->pDID);
	mosquitto_subscribe(pClient->pMosquitto, NULL, pTopic, 0);
}

FTM_VOID FTM_MQTT_CLIENT_TPGW_disconnectCB
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

FTM_VOID FTM_MQTT_CLIENT_TPGW_publishCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
}

FTM_VOID FTM_MQTT_CLIENT_TPGW_messageCB
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
	
	INFO("TOPIC - %s\n", pMessage->topic);
	INFO("MESSAGE - %s\n", (FTM_CHAR_PTR)pMessage->payload);

	memset(pTopic, 0, sizeof(pTopic));
	strncpy(pTopic, pMessage->topic, FTM_MQTT_CLIENT_TOPIC_LENGTH);

	FTM_MQTT_CLIENT_TPGW_topicParser(pTopic, pIDs, 10, &nIDs);
	if ((nIDs != 5) || (strcmp(pClient->pDID, pIDs[3]) != 0))
	{
		ERROR("Invalid Topic[%s]\n", pMessage->topic);
		return;
	}

	if(strcmp(pIDs[4], "req") == 0)
	{
		FTNM_MSG_PTR pMsg;

		xRet = FTM_MQTT_CLIENT_TPGW_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
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

FTM_VOID FTM_MQTT_CLIENT_TPGW_subscribeCB
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

FTM_RET	FTM_MQTT_CLIENT_TPGW_topicParser
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

FTM_RET	FTM_MQTT_CLIENT_TPGW_requestMessageParser
(
	FTM_CHAR_PTR		pMessage,
	FTNM_MSG_PTR _PTR_	ppMsg
)
{
	FTM_RET			xRet;
	FTNM_MSG		xMsg;
	FTM_INT			i;

	memset(&xMsg, 0, sizeof(xMsg));

	const nx_json * pJSON = nx_json_parse((FTM_CHAR_PTR)pMessage, nx_json_unicode_to_utf8);
	if (pJSON == NULL)
	{
		ERROR("Message is not json format.[%s]\n", pMessage);
		return	FTM_RET_MQTT_INVALID_MESSAGE;
	}

	const nx_json *pItem = nx_json_get(pJSON, "id");
	if ((pItem == NULL) || (pItem->type != NX_JSON_STRING))
	{
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
		goto error;
	}	
	strncpy(xMsg.xParams.xMQTTReq.pReqID, pItem->text_value,sizeof(xMsg.xParams.xMQTTReq.pReqID) - 1);

	pItem = nx_json_get(pJSON, "method");
	if ((pItem == NULL) || (pItem->type != NX_JSON_STRING))
	{
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
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
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
		goto error;
	}	

	switch(xMsg.xParams.xMQTTReq.ulMethod)
	{
	case	FTM_MQTT_METHOD_REQ_TIME_SYNC:
		{
			const nx_json *pTime = nx_json_get(pJSON, "time");
			if ((pTime == NULL) || (pTime->type != NX_JSON_INTEGER))
			{
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	
			
			xMsg.xParams.xMQTTReq.xParams.xTimeSync.ulTime = pTime->int_value;
		}
		break;

	case	FTM_MQTT_METHOD_REQ_CONTROL_ACTUATOR:
		{
			const nx_json *pParams = nx_json_get(pJSON, "params");
			if (pParams== NULL)
			{
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	

			const nx_json *pID =  nx_json_get(pParams, "id");
			if ((pID == NULL) || (pID->type != NX_JSON_STRING))
			{
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}
			strncpy(xMsg.xParams.xMQTTReq.xParams.xControlActuator.pID, pID->text_value, sizeof(xMsg.xParams.xMQTTReq.xParams.xControlActuator.pID) - 1);

			const nx_json *pCMD = nx_json_get(pParams, "cmd");
			if ((pCMD == NULL) || (pCMD->type != NX_JSON_STRING))
			{
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			if (strcasecmp(pCMD->text_value, "on") == 0)
			{
				xMsg.xParams.xMQTTReq.xParams.xControlActuator.xCmd = FTNM_MSG_MQTT_REQ_CMD_ON;
			}
			else if (strcasecmp(pCMD->text_value, "off") == 0)
			{
				xMsg.xParams.xMQTTReq.xParams.xControlActuator.xCmd = FTNM_MSG_MQTT_REQ_CMD_OFF;
			}
			else if (strcasecmp(pCMD->text_value, "blink") == 0)
			{
				xMsg.xParams.xMQTTReq.xParams.xControlActuator.xCmd = FTNM_MSG_MQTT_REQ_CMD_BLINK;
			}
			else
			{
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			const nx_json *pOptions = nx_json_get(pParams, "options");
			if (pOptions != NULL)
			{
				if (pOptions->type != NX_JSON_INTEGER) 
				{
					xMsg.xParams.xMQTTReq.xParams.xControlActuator.xOptions.ulDuration = pOptions->int_value;
				}
				else
				{
					xRet = FTM_RET_MQTT_INVALID_MESSAGE;
					goto error;
				}
			}	
		}
		break;

	case	FTM_MQTT_METHOD_REQ_SET_PROPERTY:
		{
		}
		break;

	case	FTM_MQTT_METHOD_REQ_POWER_OFF:
		{
		}
		break;

	case	FTM_MQTT_METHOD_REQ_REBOOT:
		{
		}
		break;

	case	FTM_MQTT_METHOD_REQ_RESTART:
		{
		}
		break;

	case	FTM_MQTT_METHOD_REQ_SW_UPDATE:
		{
		}
		break;

	case	FTM_MQTT_METHOD_REQ_SW_INFO:
		{
		}
		break;
	}

	xMsg.xParams.xMQTTReq.ulTime = pItem->int_value;
	xMsg.xType = FTNM_MSG_TYPE_MQTT_REQ;

	xRet = FTNM_MSG_create(ppMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Not enough memory!\n");
		return	xRet;	
	}

	memcpy(*ppMsg, &xMsg, sizeof(xMsg));

error:

	nx_json_free(pJSON);

	return	xRet;
}

FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataINT
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_INT 			nValue, 
	FTM_INT 			nAverage, 
	FTM_INT 			nCount, 
	FTM_INT 			nMax, 
	FTM_INT 			nMin
)
{
	ASSERT(pClient != NULL);
	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, nValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataULONG
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_ULONG 			ulValue, 
	FTM_ULONG 			ulAverage, 
	FTM_INT 			nCount, 
	FTM_ULONG 			ulMax, 
	FTM_ULONG 			ulMin
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%lu]", ulTime, ulValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataFLOAT
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_FLOAT			fValue,
	FTM_FLOAT 			fAverage, 
	FTM_INT 			nCount, 
	FTM_FLOAT 			fMax, 
	FTM_FLOAT 			fMin
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%5.3f]", ulTime, fValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataBOOL
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_BOOL 			bValue
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, bValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}
