#include <string.h>
#include "ftom.h"
#include "ftom_tp_client.h"
#include "ftom_mqtt_client.h"
#include "ftom_mqtt_client_tpgw.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT


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
//	FTM_CHAR			pTopic[FTM_MQTT_TOPIC_LEN + 1];
//	FTM_CHAR			pMessage[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];

	TRACE("MQTT TPGW is connected.\n");

	pClient->bConnected = FTM_TRUE;
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

	FTM_TIMER_initS(&pClient->xLinkTimer, 0);
	FTM_TIMER_addS(&pClient->xLinkTimer, pClient->xConfig.ulRetryInterval);

	pClient->bConnected = FTM_FALSE;

	TRACE("MQTT TPGW is disconnected.\n");
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
	FTOM_TP_CLIENT_PTR		pParent = (FTOM_TP_CLIENT_PTR)pClient->pParent;
	FTOM_MQTT_PUBLISH_PTR	pPublish;


	xRet = FTM_LIST_get(pClient->pPublishList, &nMID, (FTM_VOID_PTR _PTR_)&pPublish);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pClient->pPublishList, pPublish);	

		if (pPublish->pData != NULL)
		{
			FTOM_MQTT_TPGW_PUBLISH_INFO_PTR	pInfo = (FTOM_MQTT_TPGW_PUBLISH_INFO_PTR)pPublish->pData;
			switch(pInfo->xType)
			{
			case	FTOM_MQTT_TPGW_PUBLISH_TYPE_EP_DATA:
				{
					FTM_ULONG	i, ulCount;

					ASSERT (pParent->pGateway != NULL);

					xRet = FTM_LIST_count(pParent->pGateway->pSensorList, &ulCount);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to get sensor count!\n");
						break;
					}

					for(i = 0 ; i < ulCount ; i++)
					{
						FTOM_TP_SENSOR_PTR	pSensor;

						xRet = FTM_LIST_getAt(pParent->pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensor);
						if (xRet != FTM_RET_OK)
						{
							continue;	
						}

						if (strcasecmp(pSensor->pID, pInfo->pEPID) == 0)
						{
							if (pSensor->ulServerDataTime < pInfo->ulTime)
							{
								pSensor->ulServerDataTime = (FTM_UINT64)pInfo->ulTime;
							}
							break;
						}
					}
				}
				break;

			default:
				{
					TRACE("Unknown publish done!\n");
				}
				break;

			}
		}
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

	TRACE("Topic : %s\n", pTopic);
	TRACE("Message : %s\n", pMessage->payload);

	FTOM_MQTT_CLIENT_TPGW_topicParser(pTopic, pIDs, 10, &nIDs);
	if ((nIDs != 5) || (strcmp(pClient->xConfig.pGatewayID, pIDs[3]) != 0))
	{
		ERROR2(FTM_RET_MQTT_INVALID_TOPIC, "Invalid Topic[%s]\n", pMessage->topic);
		return;
	}

	if((strcmp(pIDs[4], "req") == 0) && (pClient->fMessageCB != NULL))
	{
		FTOM_MSG_PTR pMsg;

		xRet = FTOM_MQTT_CLIENT_TPGW_requestMessageParser((FTM_CHAR_PTR)pMessage->payload, &pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(FTM_RET_MQTT_INVALID_MESSAGE, "TPGW : Invalid message[%s].\n", pMessage->payload);
			return;
		}

		xRet = pClient->fMessageCB(pClient->pMessageCBObject, pMsg);
		if (xRet != FTM_RET_OK)
		{
			FTOM_MSG_destroy(&pMsg);
			ERROR2(xRet, "Message queue push failed.\n");
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
	cJSON _PTR_		pRoot;
	cJSON _PTR_		pItem;

	memset(&xMsg, 0, sizeof(xMsg));

	pRoot = cJSON_Parse((FTM_CHAR_PTR)pMessage);
	if (pRoot== NULL)
	{
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
		ERROR2(xRet, "Message is not json format.[%s]\n", pMessage);
		return	xRet;
	}

	pItem = cJSON_GetObjectItem(pRoot, "id");
	if ((pItem == NULL) || (pItem->type != cJSON_String))
	{
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
		ERROR2(xRet, "Message ID is not exist or invalid type!\n");
		goto error;
	}
	memset(pReqID, 0, sizeof(pReqID));
	strncpy(pReqID, pItem->valuestring, FTOM_MSG_REQ_ID_LENGTH);

	pItem = cJSON_GetObjectItem(pRoot, "method");
	if ((pItem == NULL) || (pItem->type != cJSON_String))
	{
		xRet = FTM_RET_MQTT_INVALID_MESSAGE;
		ERROR2(xRet, "Message method is not exist or invalid!\n");
		goto error;
	}	

	for(i = 0 ; i < sizeof(xReqMethod) / sizeof(xReqMethod[0]) ; i++)
	{
		if (strcasecmp(xReqMethod[i].pString, pItem->valuestring) == 0)
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
			cJSON _PTR_	pParams;
			cJSON _PTR_	pTime;

			pParams = cJSON_GetObjectItem(pRoot, "params");
			if (pParams== NULL)
			{
				WARN("MQTT REQ : Params is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	

			pTime = cJSON_GetObjectItem(pParams, "time");
			if ((pTime == NULL) || (pTime->type != cJSON_Number))
			{
				WARN("MQTT REQ : Time is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	
		
			ulTime = pTime->valueint;

			xRet = FTOM_MSG_createTimeSync(NULL, ulTime, (FTOM_MSG_TIME_SYNC_PTR _PTR_)ppMsg);
		}
		break;

	case	FTOM_MQTT_METHOD_REQ_CONTROL_ACTUATOR:
		{
			FTM_CHAR		pEPID[FTM_EPID_LEN+1];
			FTM_EP_CTRL		xEPCtrl;
			FTM_ULONG		ulDuration = 0;
			cJSON _PTR_		pParams;
			cJSON _PTR_		pID;
			cJSON _PTR_		pCMD;
			cJSON _PTR_		pOptions;
			
			pParams = cJSON_GetObjectItem(pRoot, "params");
			if (pParams== NULL)
			{
				WARN("MQTT REQ : Params is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	

			pID =  cJSON_GetObjectItem(pParams, "id");
			if ((pID == NULL) || (pID->type != cJSON_String))
			{
				WARN("MQTT REQ : ID is not exist or invalid format.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			memset(pEPID, 0, sizeof(pEPID));
			strncpy(pEPID, pID->valuestring, FTM_EPID_LEN);

			pCMD = cJSON_GetObjectItem(pParams, "cmd");
			if ((pCMD == NULL) || (pCMD->type != cJSON_String))
			{
				WARN("MQTT REQ : Command is not exist or invalid format.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			if (strcasecmp(pCMD->valuestring, "on") == 0)
			{
				xEPCtrl = FTM_EP_CTRL_ON;
			}
			else if (strcasecmp(pCMD->valuestring, "off") == 0)
			{
				xEPCtrl = FTM_EP_CTRL_OFF;
			}
			else if (strcasecmp(pCMD->valuestring, "blink") == 0)
			{
				xEPCtrl = FTM_EP_CTRL_BLINK;
			}
			else
			{
				WARN("MQTT REQ : Invalid command[%s]\n", pCMD->valuestring);
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}

			pOptions = cJSON_GetObjectItem(pParams, "options");
			if (pOptions != NULL)
			{
				cJSON _PTR_ pDuration;

				pDuration = cJSON_GetObjectItem(pOptions, "duration");
				if (pDuration != NULL)
				{
					if (pOptions->type == cJSON_Number) 
					{
						ulDuration = pOptions->valueint;
					}
					else
					{
						xRet = FTM_RET_MQTT_INVALID_MESSAGE;
							goto error;
					}
				}	
			}

			xRet = FTOM_MSG_TP_createReqControlActuator(NULL, pReqID, pEPID, xEPCtrl, ulDuration, ppMsg);

		}
		break;

	case	FTOM_MQTT_METHOD_REQ_SET_PROPERTY:
		{
			cJSON _PTR_ pParams;
			cJSON _PTR_ pReportInterval;

			pParams = cJSON_GetObjectItem(pRoot, "params");
			if (pParams== NULL)
			{
				WARN("MQTT REQ : Params is not exist.\n");
				xRet = FTM_RET_MQTT_INVALID_MESSAGE;
				goto error;
			}	

			pReportInterval = cJSON_GetObjectItem(pParams, "reportInterval");
			if ((pReportInterval != NULL) && (pReportInterval->type == cJSON_String))
			{
				FTM_ULONG	ulReportIntervalMS = strtoul(pReportInterval->valuestring,0,10);

				xRet = FTOM_MSG_TP_createReqSetReportInterval(NULL, pReqID, ulReportIntervalMS, ppMsg);
			}
		}
		break;

	case	FTOM_MQTT_METHOD_REQ_POWER_OFF:
		{
			xRet = FTOM_MSG_TP_createReqPowerOff(NULL, pReqID, ppMsg);
		}
		break;

	case	FTOM_MQTT_METHOD_REQ_REBOOT:
		{
			xRet = FTOM_MSG_TP_createReqReboot(NULL, pReqID, ppMsg);
		}
		break;

	case	FTOM_MQTT_METHOD_REQ_RESTART:
		{
			xRet = FTOM_MSG_TP_createReqRestart(NULL, pReqID, ppMsg);
		}
		break;

	case	FTOM_MQTT_METHOD_REQ_SW_UPDATE:
	case	FTOM_MQTT_METHOD_REQ_SW_INFO:
		{
			xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
		}
		break;
	}


error:

	cJSON_Delete(pRoot);

	return	xRet;
}

FTM_RET	FTOM_MQTT_CLIENT_TPGW_GWStatus
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTOM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pBuff[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulLen = 0;

	pBuff[sizeof(pBuff) - 1] = '\0';
	
	sprintf(pTopic, "v/a/g/%s/status", pGatewayID);
	ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, "%s,%lu", bStatus?"on":"off", ulTimeout);

	return	FTOM_MQTT_CLIENT_publish(pClient, pTopic, pBuff, ulLen, NULL,  NULL);
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
	
	sprintf(pTopic, "v/a/g/%s/s/%s/status", pClient->xConfig.pGatewayID, pEPID);
	ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, "%s,%lu", bStatus?"on":"off", ulTimeout);

	return	FTOM_MQTT_CLIENT_publish(pClient, pTopic, pBuff, ulLen, NULL, NULL);
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

	FTM_RET		xRet;
	FTM_CHAR	pTopic[FTOM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pBuff[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulLen = 0;
	FTM_ULONG	ulPublishedCount = 0;
	FTM_INT		i;

	do
	{
		FTM_ULONG	ulStartTime = 0xFFFFFFFF;
		FTM_ULONG	ulEndTime = 0;
		FTM_ULONG	ulPartialCount = 0;
		ulLen  = 0;
		pBuff[sizeof(pBuff) - 1] = '\0';
	
		sprintf(pTopic, "v/a/g/%s/s/%s", pClient->xConfig.pGatewayID, pEPID);
	
		ulLen = sprintf(pBuff, "[");
		for(i = ulPublishedCount ; i < ulCount ; i++)
		{
			FTM_CHAR	pTemp[128];
			FTM_ULONG	ulTempLen = 0;
			if (ulPartialCount != 0)
			{
				ulTempLen += sprintf(pTemp, ",");
			}

			ulTempLen += sprintf(&pTemp[ulTempLen], "%llu,", pData[i].ulTime*(long long unsigned int)1000);

			FTM_VALUE_snprint(&pTemp[ulTempLen], sizeof(pTemp) - ulTempLen, &pData[i].xValue);
			ulTempLen += strlen(&pTemp[ulTempLen]);
	
			if (ulTempLen >= (FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen - 1))
			{
				break;	
			}

			if (ulStartTime > pData[i].ulTime)
			{
				ulStartTime = pData[i].ulTime;	
			}

			if (ulEndTime < pData[i].ulTime)
			{
				ulEndTime = pData[i].ulTime;	
			}

			strcpy(&pBuff[ulLen], pTemp);
			ulLen += ulTempLen;
			ulPartialCount++;
		}


		ulPublishedCount += ulPartialCount;

		ulLen += snprintf(&pBuff[ulLen], FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, "]");
	
		FTOM_MQTT_TPGW_PUBLISH_INFO_PTR pInfo = FTM_MEM_malloc(sizeof(FTOM_MQTT_TPGW_PUBLISH_INFO));
		if (pInfo != NULL)
		{
			pInfo->xType = FTOM_MQTT_TPGW_PUBLISH_TYPE_EP_DATA;
			strncpy(pInfo->pEPID, pEPID, FTM_EPID_LEN);

			pInfo->ulTime = ulEndTime;
		}
		else
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR2(xRet, "Not enough memory!\n");
			break;
		}

		TRACE("Send EP[%s] DATA[ %lu, %lu ]\n", pInfo->pEPID, ulStartTime,  ulEndTime);

		xRet = FTOM_MQTT_CLIENT_publish(pClient, pTopic, pBuff, ulLen, pInfo, NULL);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to publish!\n");
			break;
		}
	}
	while(ulPublishedCount < ulCount);

	return	xRet;
}


FTM_RET	FTOM_MQTT_CLIENT_TPGW_response
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nCode,
	FTM_CHAR_PTR		pMessage
)
{
	ASSERT(pClient != NULL);

	FTM_CHAR	pTopic[FTOM_MQTT_CLIENT_TOPIC_LENGTH+1];
	FTM_CHAR	pBuff[FTOM_MQTT_CLIENT_MESSAGE_LENGTH+1];
	FTM_ULONG	ulLen = 0;

	pBuff[sizeof(pBuff) - 1] = '\0';
	
	sprintf(pTopic, "v/a/g/%s/res", pClient->xConfig.pGatewayID);
	if (nCode == 0)
	{
		ulLen += snprintf(	&pBuff[ulLen], 
							FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, 
							"{\"id\":\"%s\",\"result\":\"\"}", 
							pMsgID);
	}
	else
	{
		ulLen += snprintf(	&pBuff[ulLen], 
							FTOM_MQTT_CLIENT_MESSAGE_LENGTH - ulLen, 
							"{\"id\":\"%s\",\"error\":{\"code\":%d,\"message\":\"%s\"}}", 
							pMsgID, 
							nCode, 
							(pMessage == NULL)?"":pMessage);
	}

	return	FTOM_MQTT_CLIENT_publish(pClient, pTopic, pBuff, ulLen, NULL, NULL);
}

