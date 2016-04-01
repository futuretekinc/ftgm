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
	return	FTM_RET_MQTT_INVALID_MESSAGE;
}

