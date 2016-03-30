#include "ftm.h"
#include "ftm_om.h"
#include "ftm_mqtt_client.h"
#include <nxjson.h>

static FTM_RET	FTM_MQTT_CLIENT_FT_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
static FTM_RET	FTM_MQTT_CLIENT_FT_requestMessageParser(FTM_CHAR_PTR pMessage, FTM_OM_MSG_PTR _PTR_	pMsg);

struct
{
	FTM_CHAR_PTR	pString;
	FTM_ULONG		xMethod;
} static xReqMethod[] =
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
		FTM_OM_MSG_PTR pMsg;

		xRet = FTM_MQTT_CLIENT_FT_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Invalid message.\n");
			return;
		}

		xRet = FTM_OM_MSGQ_push(pClient->pOM->pMsgQ, pMsg);
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
	FTM_OM_MSG_PTR _PTR_	ppMsg
)
{
	FTM_OM_MSG		xMsg;
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
	xMsg.xType = FTM_OM_MSG_TYPE_MQTT_REQ;

	*ppMsg = (FTM_OM_MSG_PTR)FTM_MEM_malloc(sizeof(FTM_OM_MSG));
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

FTM_RET	FTM_MQTT_CLIENT_FT_publishEPData
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR		pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;
	FTM_ULONG	ulTime;

	if (pData->ulTime != 0)
	{
		ulTime = pData->ulTime;
	}
	else
	{
		FTM_TIME	xTime;

		FTM_TIME_getCurrent(&xTime);
		ulTime = xTime.xTimeval.tv_sec;
	}
	
	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	switch(pData->xType)
	{
	case	FTM_EP_DATA_TYPE_INT:
		{
			ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, pData->xValue.nValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_ULONG:
		{
			ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%lu]", ulTime, pData->xValue.ulValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_FLOAT:
		{
			ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%5.3f]", ulTime, pData->xValue.fValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_BOOL:
		{
			ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, pData->xValue.bValue);
		}
		break;

	default:
		{
			FATAL("Invalid EP data type.!\n");	
			return	FTM_RET_ERROR;
		}
	}

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
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

	if (ulTime == 0)
	{
		FTM_TIME	xTime;

		FTM_TIME_getCurrent(&xTime);
		ulTime = xTime.xTimeval.tv_sec;
	}

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

	if (ulTime == 0)
	{
		FTM_TIME	xTime;

		FTM_TIME_getCurrent(&xTime);
		ulTime = xTime.xTimeval.tv_sec;
	}

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

	if (ulTime == 0)
	{
		FTM_TIME	xTime;

		FTM_TIME_getCurrent(&xTime);
		ulTime = xTime.xTimeval.tv_sec;
	}

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

	if (ulTime == 0)
	{
		FTM_TIME	xTime;

		FTM_TIME_getCurrent(&xTime);
		ulTime = xTime.xTimeval.tv_sec;
	}

	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);
	ulMessageLen += sprintf(&pMessage[ulMessageLen], "[%lu,%d]", ulTime, bValue);

	TRACE("MESSAGE : %s\n", pMessage);
	mosquitto_publish(pClient->pMosquitto, NULL, pTopic, ulMessageLen, pMessage, 1, 0);

	return	FTM_RET_OK;
}

