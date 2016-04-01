#include <string.h>
#include "ftm_om.h"
#include "ftm_mqtt_client.h"
#include <nxjson.h>

FTM_RET	FTM_MQTT_CLIENT_TPGW_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
FTM_RET	FTM_MQTT_CLIENT_TPGW_requestMessageParser(FTM_CHAR_PTR pMessage, FTM_OM_MSG_PTR _PTR_	pMsg);

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
		.pString	= "controlActuator",
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
		FTM_OM_MSG_PTR pMsg;

		xRet = FTM_MQTT_CLIENT_TPGW_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
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
	FTM_OM_MSG_PTR _PTR_	ppMsg
)
{
	FTM_RET			xRet;
	FTM_OM_MSG		xMsg;
	FTM_INT			i;
	FTM_CHAR		pReqID[FTM_OM_MSG_REQ_ID_LENGTH+1];
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
	strncpy(pReqID, pItem->text_value, FTM_OM_MSG_REQ_ID_LENGTH);

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
	case	FTM_MQTT_METHOD_REQ_TIME_SYNC:
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

			xRet = FTM_OM_MSG_createTimeSync(ulTime, (FTM_OM_MSG_TIME_SYNC_PTR _PTR_)ppMsg);
		}
		break;

	case	FTM_MQTT_METHOD_REQ_CONTROL_ACTUATOR:
		{
			FTM_CHAR	pTemp[128];
			FTM_EP_ID	xEPID;
			FTM_EP_CTRL	xEPCtrl;
			FTM_ULONG	ulDuration = 0;

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

			strncpy(pTemp, pID->text_value, sizeof(pTemp) - 1);
			pTemp[sizeof(pTemp)-1] = '\0';

			FTM_CHAR_PTR	pDID = strtok(pTemp, "-");
			if (pDID == NULL)
			{
				WARN("MQTT REQ : Invalid ID[%s]\n", pTemp);
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			FTM_CHAR_PTR	pEPID = strtok(NULL, "-");
			if (pEPID == NULL)
			{
				WARN("MQTT REQ : Invalid ID[%s]\n", pTemp);
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}
			
			xEPID = strtoul(pEPID, 0, 16);

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

			xRet = FTM_OM_MSG_createEPCtrl(xEPID, xEPCtrl, ulDuration, (FTM_OM_MSG_EP_CTRL_PTR _PTR_)ppMsg);

		}
		break;

	case	FTM_MQTT_METHOD_REQ_SET_PROPERTY:
	case	FTM_MQTT_METHOD_REQ_POWER_OFF:
	case	FTM_MQTT_METHOD_REQ_REBOOT:
	case	FTM_MQTT_METHOD_REQ_RESTART:
	case	FTM_MQTT_METHOD_REQ_SW_UPDATE:
	case	FTM_MQTT_METHOD_REQ_SW_INFO:
		{
			xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
		}
		break;
	}


error:

	nx_json_free(pJSON);

	return	xRet;
}

FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPData
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_CHAR	pTopic[FTM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pMessage[FTM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulMessageLen = 0;
	FTM_INT		i;

	pMessage[sizeof(pMessage) - 1] = '\0';
	
	sprintf(pTopic, "v/a/g/%s/s/%08lx", pClient->pDID, xEPID);

	ulMessageLen = sprintf(pMessage, "[");
	for(i = 0 ; i < ulCount ; i++)
	{
		TRACE("%d,%lu\n", pData[i].ulTime, pData[i].xValue.ulValue);
		if (i == 0)
		{
			ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, "%lu", pData[i].ulTime);
		}
		else
		{
			ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, ",%lu", pData[i].ulTime);
		}

		switch(pData[i].xType)
		{
		case	FTM_EP_DATA_TYPE_INT:
			{
				ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, ",%d", pData[i].xValue.nValue);
			}
			break;

		case	FTM_EP_DATA_TYPE_ULONG:
			{
				ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, ",%lu", pData[i].xValue.ulValue);
			}
			break;

		case	FTM_EP_DATA_TYPE_FLOAT:
			{
				ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, ",%5.3f", pData[i].xValue.fValue);
			}
			break;

		case	FTM_EP_DATA_TYPE_BOOL:
			{
				ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, ",%d", pData[i].xValue.bValue);
			}
			break;

		default:
			{
				FATAL("Invalid EP data type.!\n");	
				return	FTM_RET_ERROR;
			}
		}
	}

	ulMessageLen += snprintf(&pMessage[ulMessageLen], FTM_MQTT_CLIENT_MESSAGE_LENGTH - ulMessageLen, "]");

	return	FTM_MQTT_CLIENT_publish(pClient, pTopic, pMessage, ulMessageLen);
}

