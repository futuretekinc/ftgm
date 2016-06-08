#include <string.h>
#include "ftom.h"
#include "ftom_mqtt_client.h"
#include <nxjson.h>

FTM_RET	FTOM_MQTT_CLIENT_TPGW_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
FTM_RET	FTOM_MQTT_CLIENT_TPGW_requestMessageParser(FTM_CHAR_PTR pMessage, FTOM_MSG_PTR _PTR_	pMsg);

struct
{
	FTM_CHAR_PTR	pString;
	FTM_ULONG		xMethod;
} static xReqMethod[] =
{
	{
		.pString	= "timeSync",
		.xMethod		= FTOM_MQTT_METHOD_REQ_TIME_SYNC
	},
	{
		.pString	= "controlActuator",
		.xMethod		= FTOM_MQTT_METHOD_REQ_CONTROL_ACTUATOR,
	},
	{
		.pString	= "setProperty",
		.xMethod		= FTOM_MQTT_METHOD_REQ_SET_PROPERTY,
	},
	{
		.pString	= "poweroff",
		.xMethod		= FTOM_MQTT_METHOD_REQ_POWER_OFF,
	},
	{
		.pString	= "reboot",
		.xMethod		= FTOM_MQTT_METHOD_REQ_REBOOT,
	},
	{
		.pString	= "restart",
		.xMethod		= FTOM_MQTT_METHOD_REQ_RESTART,
	},
	{
		.pString	= "swUpdate",
		.xMethod		= FTOM_MQTT_METHOD_REQ_SW_UPDATE,
	},
	{
		.pString	= "swInfo",
		.xMethod		= FTOM_MQTT_METHOD_REQ_SW_INFO,
	}
};

/***********************************************************************************
 * Thing+Gateway MQTT interface
 ***********************************************************************************/

FTM_VOID FTOM_MQTT_CLIENT_TPGW_connectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;
	FTM_CHAR			pTopic[FTM_MQTT_TOPIC_LEN + 1];
	FTM_CHAR			pMessage[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG			ulMessageLen = 0;

	TRACE("MQTT is connected.\n");

	pClient->bConnected = FTM_TRUE;

	sprintf(pTopic, "v/a/g/%s/status", pClient->pDID);
	ulMessageLen = sprintf(pMessage, "on,%lu", pClient->xConfig.ulReconnectionTime);
	FTOM_MQTT_CLIENT_publish(pClient, pTopic, pMessage, ulMessageLen);
}

FTM_VOID FTOM_MQTT_CLIENT_TPGW_disconnectCB
(
	struct mosquitto 	*mosq, 
	void				*pObj, 
	int					nResult
)
{
	ASSERT(pObj != NULL);
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;

	FTM_TIMER_init(&pClient->xLinkTimer, 0);
	FTM_TIMER_addSeconds(&pClient->xLinkTimer, pClient->xConfig.ulReconnectionTime);

	pClient->bConnected = FTM_FALSE;

	TRACE("MQTT is disconnected.\n");
}

FTM_VOID FTOM_MQTT_CLIENT_TPGW_publishCB
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


	xRet = FTM_LIST_get(pClient->pPublishList, &nMID, (FTM_VOID_PTR _PTR_)&pPublish);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pPublishList, pPublish);	
		FTOM_MQTT_PUBLISH_destroy(&pPublish);
	}
	else
	{
		WARN("Publish[%08x] not found!\n", nMID);
	}
}

FTM_VOID FTOM_MQTT_CLIENT_TPGW_messageCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	const struct mosquitto_message *pMessage
)
{
	ASSERT(pObj != NULL);

	FTM_RET			xRet;
	FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;
	FTM_CHAR		pTopic[FTOM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR_PTR	pIDs[10];
	FTM_INT			nIDs = 0;
	
	memset(pTopic, 0, sizeof(pTopic));
	strncpy(pTopic, pMessage->topic, FTOM_MQTT_CLIENT_TOPIC_LENGTH);

	FTOM_MQTT_CLIENT_TPGW_topicParser(pTopic, pIDs, 10, &nIDs);
	if ((nIDs != 5) || (strcmp(pClient->pDID, pIDs[3]) != 0))
	{
		ERROR("Invalid Topic[%s]\n", pMessage->topic);
		return;
	}

	if(strcmp(pIDs[4], "req") == 0)
	{
		FTOM_MSG_PTR pMsg;

		xRet = FTOM_MQTT_CLIENT_TPGW_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Invalid message.\n");
			return;
		}

		xRet = FTOM_sendMessage(pMsg);
		if (xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pMsg);
			ERROR("Message queue push failed.\n");
			return;
		}
	}
}

FTM_VOID FTOM_MQTT_CLIENT_TPGW_subscribeCB
(
	struct mosquitto 	*mosq, 
	void				*pObj,
	int					nMID, 
	int					nQoS, 
	const int			*pGrantedQoS
)
{
	ASSERT(pObj != NULL);
	//FTOM_MQTT_CLIENT_PTR	pClient = (FTOM_MQTT_CLIENT_PTR)pObj;
}

FTM_RET	FTOM_MQTT_CLIENT_TPGW_topicParser
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

FTM_RET	FTOM_MQTT_CLIENT_TPGW_requestMessageParser
(
	FTM_CHAR_PTR		pMessage,
	FTOM_MSG_PTR _PTR_	ppMsg
)
{
	FTM_RET			xRet;
	FTOM_MSG		xMsg;
	FTM_INT			i;
	FTM_CHAR		pReqID[FTOM_MSG_REQ_ID_LENGTH+1];
	FTM_ULONG		ulMethod = 0;
	
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
	memset(pReqID, 0, sizeof(pReqID));
	strncpy(pReqID, pItem->text_value, FTOM_MSG_REQ_ID_LENGTH);

	pItem = nx_json_get(pJSON, "method");
	if ((pItem == NULL) || (pItem->type != NX_JSON_STRING))
	{
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
		goto error;
	}	

	for(i = 0 ; i < sizeof(xReqMethod) / sizeof(xReqMethod[0]) ; i++)
	{
		if (strcasecmp(xReqMethod[i].pString, pItem->text_value) == 0)
		{
			ulMethod = xReqMethod[i].xMethod;
			break;
		}
	}

	switch(ulMethod)
	{
	case	FTOM_MQTT_METHOD_REQ_TIME_SYNC:
		{
			FTM_ULONG	ulTime;
			const nx_json *pParams = nx_json_get(pJSON, "params");
			if (pParams== NULL)
			{
				WARN("MQTT REQ : Params is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	

			const nx_json *pTime = nx_json_get(pParams, "time");
			if ((pTime == NULL) || (pTime->type != NX_JSON_INTEGER))
			{
				WARN("MQTT REQ : Time is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	
		
			ulTime = pTime->int_value;

			xRet = FTOM_MSG_createTimeSync(ulTime, (FTOM_MSG_TIME_SYNC_PTR _PTR_)ppMsg);
		}
		break;

	case	FTOM_MQTT_METHOD_REQ_CONTROL_ACTUATOR:
		{
			FTM_CHAR		pEPID[FTM_EPID_LEN+1];
			FTM_EP_CTRL		xEPCtrl;
			FTM_ULONG		ulDuration = 0;

			const nx_json *pParams = nx_json_get(pJSON, "params");
			if (pParams== NULL)
			{
				WARN("MQTT REQ : Params is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	

			const nx_json *pID =  nx_json_get(pParams, "id");
			if ((pID == NULL) || (pID->type != NX_JSON_STRING))
			{
				WARN("MQTT REQ : ID is not exist or invalid format.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			memset(pEPID, 0, sizeof(pEPID));
			strncpy(pEPID, pID->text_value, FTM_EPID_LEN);

			const nx_json *pCMD = nx_json_get(pParams, "cmd");
			if ((pCMD == NULL) || (pCMD->type != NX_JSON_STRING))
			{
				WARN("MQTT REQ : Command is not exist or invalid format.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			if (strcasecmp(pCMD->text_value, "on") == 0)
			{
				xEPCtrl = FTM_EP_CTRL_ON;
			}
			else if (strcasecmp(pCMD->text_value, "off") == 0)
			{
				xEPCtrl = FTM_EP_CTRL_OFF;
			}
			else if (strcasecmp(pCMD->text_value, "blink") == 0)
			{
				xEPCtrl = FTM_EP_CTRL_BLINK;
			}
			else
			{
				WARN("MQTT REQ : Invalid command[%s]\n", pCMD->text_value);
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			const nx_json *pOptions = nx_json_get(pParams, "options");
			if (pOptions->type != NX_JSON_NULL)
			{
				const nx_json *pDuration = nx_json_get(pOptions, "duration");
				if (pDuration ->type != NX_JSON_NULL)
				{
					if (pOptions->type == NX_JSON_INTEGER) 
					{
						ulDuration = pOptions->int_value;
					}
					else
					{
						xRet = FTM_RET_MQTT_INVALID_MESSAGE;
							goto error;
					}
				}	
			}

			xRet = FTOM_MSG_createEPCtrl(pEPID, xEPCtrl, ulDuration, (FTOM_MSG_EP_CTRL_PTR _PTR_)ppMsg);

		}
		break;

	case	FTOM_MQTT_METHOD_REQ_SET_PROPERTY:
	case	FTOM_MQTT_METHOD_REQ_POWER_OFF:
	case	FTOM_MQTT_METHOD_REQ_REBOOT:
	case	FTOM_MQTT_METHOD_REQ_RESTART:
	case	FTOM_MQTT_METHOD_REQ_SW_UPDATE:
	case	FTOM_MQTT_METHOD_REQ_SW_INFO:
		{
			xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
		}
		break;
	}


error:

	nx_json_free(pJSON);

	return	xRet;
}

FTM_RET	FTOM_MQTT_CLIENT_TPGW_publishEPStatus
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTOM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pBuff[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulLen = 0;

	pBuff[sizeof(pBuff) - 1] = '\0';
	
	sprintf(pTopic, "v/a/g/%s/s/%s/status", pClient->pDID, pEPID);
	ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, "%s,%lu", bStatus?"on":"off", ulTimeout);

	return	FTOM_MQTT_CLIENT_publish(pClient, pTopic, pBuff, ulLen);
}

FTM_RET	FTOM_MQTT_CLIENT_TPGW_publishEPData
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_CHAR	pTopic[FTOM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pBuff[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulLen = 0;
	FTM_INT		i;

	pBuff[sizeof(pBuff) - 1] = '\0';
	
	sprintf(pTopic, "v/a/g/%s/s/%s", pClient->pDID, pEPID);

	ulLen = sprintf(pBuff, "[");
	for(i = 0 ; i < ulCount ; i++)
	{
		if (i == 0)
		{
			ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, "%llu", pData[i].ulTime*(FTM_UINT64)1000);
		}
		else
		{
			ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, ",%llu", pData[i].ulTime*(FTM_UINT64)1000);
		}

		ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, ",%s", FTM_VALUE_print(&pData[i].xValue));
	}

	ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, "]");

	return	FTOM_MQTT_CLIENT_publish(pClient, pTopic, pBuff, ulLen);
}

