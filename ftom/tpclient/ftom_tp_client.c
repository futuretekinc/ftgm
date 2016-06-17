#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftom_client.h"
#include "ftom_client_net.h"
#include "ftom_tp_client.h"
#include "ftom_message_queue.h"
#include "ftom_mqtt_client.h"
#include "ftom_mqtt_client_tpgw.h"

static 
FTM_VOID_PTR FTOM_TP_CLIENT_process
(
	FTM_VOID_PTR 		pData
);

static
FTM_RET	FTOM_TP_CLIENT_serverSync
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
);

static
FTM_RET	FTOM_TP_CLIENT_NODE_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_NODE_PTR		pNode
);

static
FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_EP_PTR			pEPInfo
);

static
FTM_RET	FTOM_TP_CLIENT_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

static 
FTOM_MQTT_CLIENT_CBSET	xMQTTCBSet =
{
	.fConnect 			= FTOM_MQTT_CLIENT_TPGW_connectCB,
	.fDisconnect		= FTOM_MQTT_CLIENT_TPGW_disconnectCB,
	.fPublish 			= FTOM_MQTT_CLIENT_TPGW_publishCB,
	.fMessage 			= FTOM_MQTT_CLIENT_TPGW_messageCB,
	.fSubscribe 		= FTOM_MQTT_CLIENT_TPGW_subscribeCB,
	.fReportGWStatus	= FTOM_MQTT_CLIENT_TPGW_reportGWStatus,
	.fPublishEPStatus	= FTOM_MQTT_CLIENT_TPGW_publishEPStatus,
	.fPublishEPData		= FTOM_MQTT_CLIENT_TPGW_publishEPData,
	.fTPResponse		= FTOM_MQTT_CLIENT_TPGW_response,
};


FTM_RET	FTOM_TP_CLIENT_create
(
	FTOM_TP_CLIENT_PTR _PTR_ 	ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET				xRet;
	FTOM_TP_CLIENT_PTR	pClient;

	pClient = (FTOM_TP_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_TP_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_TP_CLIENT_init(pClient);
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

FTM_RET	FTOM_TP_CLIENT_destroy
(
	FTOM_TP_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_TP_CLIENT_final(*ppClient);

	FTM_MEM_free(*ppClient);
	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_init
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;	

	memset(pClient, 0, sizeof(FTOM_TP_CLIENT));

	//FTOM_getDID(pClient->xConfig.pGatewayID, FTM_GWID_LEN);
	strcpy(pClient->xConfig.pHost, FTOM_TP_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.usPort 			= FTOM_TP_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulRetryInterval	= FTOM_TP_CLIENT_DEFAULT_RETRY_INTERVAL;
	pClient->xConfig.ulReportInterval 	= FTOM_TP_CLIENT_DEFAULT_REPORT_INTERVAL;
	pClient->bStop = FTM_TRUE;

	xRet = FTOM_CLIENT_NET_create((FTOM_CLIENT_NET_PTR _PTR_)&pClient->pFTOMC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create client!\n");
		return	0;	
	}
	
	xRet =FTOM_CLIENT_setNotifyCB(pClient->pFTOMC, FTOM_TP_CLIENT_notifyCB, pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set notify callback\n");	
	}

	xRet = FTOM_TP_RESTAPI_init(&pClient->xRESTApi);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "TP REST API initialize failed!\n");
		goto error;	
	}

	xRet = FTOM_MQTT_CLIENT_init(&pClient->xMQTT);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT Client initialize failed!\n");
		goto error;
	}

	xRet = FTOM_MQTT_CLIENT_setMessageCB(&pClient->xMQTT, (FTOM_MQTT_CLIENT_MESSAGE_CB)FTOM_TP_CLIENT_sendMessage, pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set MQTT client message callback!\n");
	}

	xRet = FTOM_MSGQ_init(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MsgQ init failed!\n");
		goto error;	
	}

	xRet = FTOM_MQTT_CLIENT_setCBSet(&pClient->xMQTT, &xMQTTCBSet);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set MQTT client callback set!\n");
	}

error:
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTOM_TP_CLIENT_stop(pClient);

	FTOM_TP_RESTAPI_final(&pClient->xRESTApi);
	FTOM_MQTT_CLIENT_final(&pClient->xMQTT);
	FTOM_MSGQ_final(&pClient->xMsgQ);

	FTOM_CLIENT_NET_destroy((FTOM_CLIENT_NET_PTR _PTR_)&pClient->pFTOMC);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_getConfig
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTOM_TP_CLIENT_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &pClient->xConfig, sizeof(FTOM_TP_CLIENT_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_setConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_TP_CLIENT_CONFIG_PTR 	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTOM_CLIENT_NET_CONFIG	xNetConfig;
	FTOM_MQTT_CLIENT_CONFIG	xMQTTConfig;

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_TP_CLIENT_CONFIG));

	strncpy(xMQTTConfig.pGatewayID,	pClient->xConfig.pGatewayID,FTM_GWID_LEN);
	strncpy(xMQTTConfig.pHost, 		pClient->xConfig.pHost, 	FTM_HOST_LEN);
	strncpy(xMQTTConfig.pUserID, 	pClient->xConfig.pGatewayID,FTM_USER_ID_LEN);
	strncpy(xMQTTConfig.pPasswd, 	pClient->xConfig.pAPIKey, 	FTM_PASSWD_LEN);
	strncpy(xMQTTConfig.pCertFile, 	pClient->xConfig.pCertFile, FTM_FILE_NAME_LEN);
	xMQTTConfig.usPort 			= pClient->xConfig.usPort;
	xMQTTConfig.ulRetryInterval = pClient->xConfig.ulRetryInterval;
	xMQTTConfig.bTLS 			= pClient->xConfig.bSecure;

	xRet = FTOM_MQTT_CLIENT_setConfig(&pClient->xMQTT, &xMQTTConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "MQTT Client configation loading failed!\n");
		return	0;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;
	FTOM_TP_CLIENT_CONFIG	xTPConfig;

	xRet = FTOM_TP_CLIENT_getConfig(pClient, &xTPConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get client configuration!\n");
		return	FTM_RET_ERROR;	
	}

	xRet = FTM_CONFIG_getItem(pConfig, "tpclient", &xSection);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "id", xTPConfig.pGatewayID, FTM_GWID_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find the gateway id for the TPClient!\n");
		}

		xRet = FTM_CONFIG_ITEM_getItemBOOL(&xSection, "secure", &xTPConfig.bSecure);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find secure mode !\n");
		}

		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "cert", xTPConfig.pCertFile, FTM_FILE_NAME_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find the certificate information for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "apikey", xTPConfig.pAPIKey, FTM_PASSWD_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find a APIKEY information for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "host", xTPConfig.pHost, FTM_HOST_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can not find a host for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &xTPConfig.usPort);
		if (xRet != FTM_RET_OK)
		{
			INFO("Can not find a port for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "report_interval", &xTPConfig.ulReportInterval);
		if (xRet != FTM_RET_OK)
		{
			INFO("Can not find a report interval for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "retry_interval", &xTPConfig.ulRetryInterval);
		if (xRet != FTM_RET_OK)
		{
			INFO("Can not find max connection retry interval for the TPClient!\n");
		}
	}

	xRet = FTOM_TP_CLIENT_setConfig(pClient, &xTPConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set TP Client configuration!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_loadConfigFromFile
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig);
	if (xRet !=  FTM_RET_OK)
	{
		ERROR2(xRet, "Configration loading failed!\n");
		return	xRet;	
	}

	xRet = FTOM_TP_CLIENT_loadConfig(pClient, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_saveConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "tpclient", &xSection);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_addItem(pConfig, "tpclient", &xSection);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "id", pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save the gateway id for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "cert", pClient->xConfig.pCertFile);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save the certificate information for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "apikey", pClient->xConfig.pAPIKey);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a APIKEY information for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemString(&xSection, "host", pClient->xConfig.pHost);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can not save a host for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemUSHORT(&xSection, "port", pClient->xConfig.usPort);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a port for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemULONG(&xSection, "report_interval", pClient->xConfig.ulReportInterval);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a report interval for the TPClient!\n");
	}

	xRet = FTM_CONFIG_ITEM_setItemULONG(&xSection, "retry_interval", pClient->xConfig.ulRetryInterval);
	if (xRet != FTM_RET_OK)
	{
		INFO("Can not save a max retry interval for the TPClient!\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_showConfig
(
	FTOM_TP_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("\n[ ThingPlus Client Configuration ]\n");
	MESSAGE("%16s : %s\n", "Gateay ID", pClient->xConfig.pGatewayID);
	MESSAGE("%16s : %s\n", "Host", pClient->xConfig.pHost);
	MESSAGE("%16s : %d\n", "Port", pClient->xConfig.usPort);
	MESSAGE("%16s : %s\n", "API Key", pClient->xConfig.pAPIKey);
	MESSAGE("%16s : %s\n", "Secure Mode", (pClient->xConfig.bSecure)?"on":"off");
	MESSAGE("%16s : %s\n", "Cert File", pClient->xConfig.pCertFile);
	MESSAGE("%16s : %lu\n","Report Interval", pClient->xConfig.ulReportInterval);
	MESSAGE("%16s : %lu\n","Retry Interval", pClient->xConfig.ulRetryInterval);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_sendMessage
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);
}

FTM_RET	FTOM_TP_CLIENT_start
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	if (pClient->bStop != FTM_TRUE)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	pthread_create(&pClient->xMain, NULL, FTOM_TP_CLIENT_process, pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_stop
(
	FTOM_TP_CLIENT_PTR pClient
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

FTM_RET	FTOM_TP_CLIENT_isRun
(
	FTOM_TP_CLIENT_PTR pClient,
	FTM_BOOL_PTR		pbRun
)
{
	if ((pClient == NULL) || pClient->bStop)
	{
		*pbRun = FTM_FALSE	;
	}

	*pbRun = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_waitingForFinished
(
	FTOM_TP_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	pthread_join(pClient->xMain, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_TP_CLIENT_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET					xRet;
	FTOM_TP_CLIENT_PTR		pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTM_TIME				xBaseTime, xCurrentTime, xDiffTime;
	FTM_BOOL				bConnected = FTM_FALSE;
	FTM_CHAR				pTopic[FTM_MQTT_TOPIC_LEN + 1];

	TRACE("TPClient[%s] started.\n", pClient->xConfig.pGatewayID);


	xRet = FTOM_TP_RESTAPI_setUserID(&pClient->xRESTApi, pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set User ID!\n");
	}

	xRet = FTOM_TP_RESTAPI_setPasswd(&pClient->xRESTApi, pClient->xConfig.pAPIKey);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set password!\n");
	}

	xRet = FTOM_TP_RESTAPI_GW_setID(&pClient->xRESTApi, pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set gateway id!\n");
	}

	FTOM_TP_RESTAPI_setVerbose(&pClient->xRESTApi, FTM_TRUE);

	FTM_TIMER_initS(&pClient->xRetryTimer,	0);
	FTM_TIMER_initS(&pClient->xReportTimer, 	0);

	sprintf(pTopic, "v/a/g/%s/res", pClient->xConfig.pGatewayID);
	FTOM_MQTT_CLIENT_subscribe(&pClient->xMQTT, pTopic);
	sprintf(pTopic, "v/a/g/%s/req", pClient->xConfig.pGatewayID);
	FTOM_MQTT_CLIENT_subscribe(&pClient->xMQTT, pTopic);

	pClient->bConnected 	= FTM_FALSE;
	pClient->bStop		= FTM_FALSE;

	xRet = FTOM_CLIENT_start(pClient->pFTOMC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start FTOM client\n");
	}

	FTOM_MQTT_CLIENT_start(&pClient->xMQTT);

	FTM_TIME_getCurrent(&xBaseTime);
	
	while(!pClient->bStop)
	{
		FTOM_MSG_PTR	pBaseMsg;

		FTOM_MQTT_CLIENT_isConnected(&pClient->xMQTT, &pClient->bConnected);

		if(pClient->bConnected)
		{
			if (!bConnected)
			{
				FTOM_TP_CLIENT_reportGWStatus(pClient, pClient->xConfig.pGatewayID, FTM_TRUE, pClient->xConfig.ulReportInterval);
				FTM_TIMER_initS(&pClient->xReportTimer, 	pClient->xConfig.ulReportInterval);
				bConnected = pClient->bConnected;
			}
			else
			{
				if (FTM_TIMER_isExpired(&pClient->xReportTimer))
				{
					FTOM_TP_CLIENT_reportGWStatus(pClient, pClient->xConfig.pGatewayID, FTM_TRUE, pClient->xConfig.ulReportInterval);
					FTM_TIMER_addS(&pClient->xReportTimer, pClient->xConfig.ulReportInterval);
				}
			}
		}
		else
		{
			if (bConnected)
			{
				FTM_TIMER_initS(&pClient->xRetryTimer, pClient->xConfig.ulRetryInterval);
				bConnected = pClient->bConnected;
			}
			else
			{
				if (FTM_TIMER_isExpired(&pClient->xRetryTimer))
				{
					FTM_TIMER_addS(&pClient->xRetryTimer, pClient->xConfig.ulRetryInterval);
				}
			}
		}

		FTM_TIME_getCurrent(&xCurrentTime);
		FTM_TIME_sub(&xCurrentTime, &xBaseTime, &xDiffTime);
		FTM_TIME_addMS(&xBaseTime, FTOM_TPCLIENT_LOOP_INTERVAL, &xBaseTime);

		FTM_UINT64	ullDiffTime = 0;
		FTM_TIME_toMS(&xDiffTime, &ullDiffTime);

		xRet = FTOM_MSGQ_timedPop(&pClient->xMsgQ, ullDiffTime, &pBaseMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pBaseMsg->xType)
			{
			case	FTOM_MSG_TYPE_CONNECTION_STATUS:
				{
					FTOM_MSG_CONNECTION_STATUS_PTR	pMsg = (FTOM_MSG_CONNECTION_STATUS_PTR)pBaseMsg;

					if (pMsg->bConnected)
					{
						FTOM_TP_CLIENT_serverSync(pClient, FTM_FALSE);
					}
					else
					{
					
					}
				}
				break;

			case	FTOM_MSG_TYPE_INITIALIZE_DONE:
				{
					FTOM_TP_CLIENT_serverSync(pClient, FTM_FALSE);
				}
				break;

			case	FTOM_MSG_TYPE_REPORT_GW_STATUS:
				{
					FTOM_MSG_REPORT_GW_STATUS_PTR	pMsg = (FTOM_MSG_REPORT_GW_STATUS_PTR)pBaseMsg;

					FTOM_TP_CLIENT_reportGWStatus(pClient, pMsg->pGatewayID, pMsg->bStatus, pMsg->ulTimeout);
				}
				break;

			case	FTOM_MSG_TYPE_SEND_EP_STATUS:
				{
					FTOM_MSG_SEND_EP_STATUS_PTR	pMsg = (FTOM_MSG_SEND_EP_STATUS_PTR)pBaseMsg;

					FTOM_TP_CLIENT_sendEPStatus(pClient, pMsg->pEPID, pMsg->bStatus, pMsg->ulTimeout);
				}
				break;
			case	FTOM_MSG_TYPE_SEND_EP_DATA:
				{
					FTOM_MSG_SEND_EP_DATA_PTR	pMsg = (FTOM_MSG_SEND_EP_DATA_PTR)pBaseMsg;

					FTOM_TP_CLIENT_sendEPData(pClient, pMsg->pEPID, pMsg->pData, pMsg->ulCount);
				}
				break;

			case	FTOM_MSG_TYPE_SERVER_SYNC:
				{
					FTOM_MSG_SERVER_SYNC_PTR	pMsg = (FTOM_MSG_SERVER_SYNC_PTR)pBaseMsg;

					FTOM_TP_CLIENT_serverSync(pClient, pMsg->bAutoRegister);
				}
				break;

			case	FTOM_MSG_TYPE_TP_REQ_RESTART:
				{
					FTOM_MSG_TP_REQ_RESTART_PTR pMsg = (FTOM_MSG_TP_REQ_RESTART_PTR)pBaseMsg;

					FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, 0, "");
				}
				break;
			case	FTOM_MSG_TYPE_TP_REQ_SET_REPORT_INTERVAL:
				{
					FTM_ULONG	ulCount = 0;
					FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR pMsg = (FTOM_MSG_TP_REQ_SET_REPORT_INTERVAL_PTR)pBaseMsg;

					if (pMsg->ulReportIntervalMS < 1000)
					{
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, -10000, "Invalid report interval");
					}
					else
					{
						if (pClient->xConfig.ulReportInterval != pMsg->ulReportIntervalMS / 1000)
						{
							pClient->xConfig.ulReportInterval = pMsg->ulReportIntervalMS / 1000;
							FTM_TIMER_initS(&pClient->xReportTimer, 0);
						}
	
						xRet = FTOM_CLIENT_NODE_count(pClient->pFTOMC, &ulCount);
						if (xRet == FTM_RET_OK)
						{
							FTM_INT	i;
	
							for(i = 0 ; i < ulCount ; i++)
							{
								FTM_NODE	xNode;
				
								xRet = FTOM_CLIENT_NODE_getAt(pClient->pFTOMC, i, &xNode);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Can't get EP info at %d\n", i);
									continue;	
								}
			
								FTOM_CLIENT_NODE_setReportInterval(pClient->pFTOMC, xNode.pDID, pMsg->ulReportIntervalMS / 1000);
							}
						}
	
						xRet = FTOM_CLIENT_EP_count(pClient->pFTOMC, 0, &ulCount);
						if (xRet == FTM_RET_OK)
						{
							FTM_INT	i;
	
							for(i = 0 ; i < ulCount ; i++)
							{
								FTM_EP	xEPInfo;
				
								xRet = FTOM_CLIENT_EP_getAt(pClient->pFTOMC, i, &xEPInfo);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Can't get EP info at %d\n", i);
									continue;	
								}
				
								FTOM_CLIENT_EP_setReportInterval(pClient->pFTOMC, xEPInfo.pEPID, pMsg->ulReportIntervalMS / 1000);
							}
						}
	
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, 0, "");
					}
				}
				break;

			case	FTOM_MSG_TYPE_TP_REQ_CONTROL_ACTUATOR:
				{
					FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR pMsg = (FTOM_MSG_TP_REQ_CONTROL_ACTUATOR_PTR)pBaseMsg;
					FTM_EP		xEP;
					FTM_EP_DATA	xData;

					xRet = FTOM_CLIENT_EP_get(pClient->pFTOMC, pMsg->pEPID, &xEP);
					if (xRet != FTM_RET_OK)
					{
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, xRet, "Invalid EPID");
						break;
					}

					switch(pMsg->xCtrl)
					{
					case	FTM_EP_CTRL_OFF:
						FTM_EP_DATA_initINT(&xData, 0);
						break;
				
					case	FTM_EP_CTRL_ON:
						FTM_EP_DATA_initINT(&xData, 1);
						break;

					case	FTM_EP_CTRL_BLINK:
						FTM_EP_DATA_initINT(&xData, 2);
						break;
					};

					xRet = FTOM_CLIENT_EP_remoteSet(pClient->pFTOMC, xEP.pEPID, &xData);
					if (xRet != FTM_RET_OK)
					{
					
						FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, xRet, "Remote set error!");
						break;
					}

					FTOM_TP_CLIENT_respose(pClient, pMsg->pReqID, 0, "");
				}
				break;

			default:
				{
					ERROR("Not supported msg[%08x]\n", pBaseMsg->xType);	
				}
			}
			FTOM_MSG_destroy(&pBaseMsg);
		}
	}

	FTOM_MQTT_CLIENT_stop(&pClient->xMQTT);
	TRACE("TPClient[%s] stopped.\n", pClient->xConfig.pGatewayID);

	FTOM_CLIENT_NET_destroy((FTOM_CLIENT_NET_PTR _PTR_)&pClient->pFTOMC);

	return 0;
}

FTM_RET	FTOM_TP_CLIENT_serverSyncStart
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createServerSync(bAutoRegister, (FTOM_MSG_SERVER_SYNC_PTR _PTR_)&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy(&pMsg);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_serverSync
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
)
{
	FTM_RET	xRet;
	FTOM_TP_GATEWAY_PTR pGateway = NULL;

	xRet = FTOM_TP_GATEWAY_create(&pGateway);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't creation gateway instance[%08x]!\n", xRet);
		return	xRet;	
	}
	xRet = FTOM_TP_RESTAPI_GW_getInfo(&pClient->xRESTApi, pGateway);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to get gateway information[%08x].\n", xRet);
		goto finish;
	}
	else
	{   
		FTM_INT     i;  
		FTM_ULONG   ulDeviceCount = 0;
		FTM_ULONG   ulSensorCount = 0;

		MESSAGE("%16s : %s\n",  "ID",   pGateway->pID);    
		MESSAGE("%16s : %s\n",  "Name", pGateway->pName);    
		MESSAGE("%16s : %lu\n", "Report Interval", pGateway->ulReportInterval);    
		MESSAGE("%16s : %llu\n","Installed Time", pGateway->ullCTime);    
		MESSAGE("%16s : %llu\n","Modified Time", pGateway->ullMTime);    
		MESSAGE("%16s : ", "Devices");
		FTM_LIST_count(pGateway->pDeviceList, &ulDeviceCount);
		for(i = 0 ; i < ulDeviceCount ; i++)
		{   
			FTM_CHAR_PTR    pDeviceID = NULL;
			xRet = FTM_LIST_getAt(pGateway->pDeviceList, i, (FTM_VOID_PTR _PTR_)&pDeviceID);
			if (xRet == FTM_RET_OK)
			{   
				MESSAGE("%16s ", pDeviceID);
				FTOM_CLIENT_NODE_setServerRegistered(pClient->pFTOMC, pDeviceID, FTM_TRUE);
			}   
		}   
		MESSAGE("\n");

		MESSAGE("%16s : ", "Sensors");

		FTM_LIST_count(pGateway->pSensorList, &ulSensorCount);
		for(i = 0 ; i < ulSensorCount ; i++)
		{   
			FTM_CHAR_PTR    pSensorID = NULL;
			xRet = FTM_LIST_getAt(pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensorID);
			if (xRet == FTM_RET_OK)
			{   
				MESSAGE("%16s ", pSensorID);
				FTOM_CLIENT_EP_setServerRegistered(pClient->pFTOMC, pSensorID, FTM_TRUE);
			}   
		}   
		MESSAGE("\n");

		if (bAutoRegister)
		{
			FTM_ULONG	ulNodeCount = 0;
			FTM_ULONG	ulEPCount = 0;

			xRet = FTOM_CLIENT_NODE_count(pClient->pFTOMC, &ulNodeCount);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}

			for(i = 0 ; i < ulNodeCount ; i++)
			{
				FTM_NODE	xNode;
				FTM_BOOL	bRegistered = FTM_FALSE;

				xRet = FTOM_CLIENT_NODE_getAt(pClient->pFTOMC, i, &xNode);
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't get EP info at %d\n", i);
					continue;	
				}

				xRet = FTOM_CLIENT_NODE_getServerRegistered(pClient->pFTOMC, xNode.pDID, &bRegistered);
				if ((xRet == FTM_RET_OK) && (!bRegistered))
				{
					FTOM_TP_CLIENT_NODE_register(pClient, &xNode);	
				}
			}

			xRet = FTOM_CLIENT_EP_count(pClient->pFTOMC, 0, &ulEPCount);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}

			for(i = 0 ; i < ulEPCount ; i++)
			{
				FTM_EP	xEP;
				FTM_BOOL	bRegistered = FTM_FALSE;

				xRet = FTOM_CLIENT_EP_getAt(pClient->pFTOMC, i, &xEP);
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't get EP info at %d\n", i);
					continue;	
				}
			
				xRet = FTOM_CLIENT_EP_getServerRegistered(pClient->pFTOMC, xEP.pEPID, &bRegistered);
				if ((xRet == FTM_RET_OK) && (!bRegistered))
				{
					FTOM_TP_CLIENT_EP_register(pClient, &xEP);	
				}
			}
		}
	}   

finish:

	if (pGateway != NULL)
	{
		FTOM_TP_GATEWAY_destroy(&pGateway);
	}

	return	FTM_RET_OK;
}
			
FTM_RET	FTOM_TP_CLIENT_NODE_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_NODE_PTR		pNode
)
{
	ASSERT(pClient != NULL);
	ASSERT(pNode != NULL);
	
	FTM_RET	xRet;

	TRACE("TPClient[%s] device[%s] creation!\n", pClient->xConfig.pGatewayID, pNode->pDID);

	xRet = FTOM_TP_RESTAPI_NODE_create(&pClient->xRESTApi, pNode);
	if (xRet != FTM_RET_OK)
	{
		TRACE("TPClient[%s] device[%s] creation failed!\n", pClient->xConfig.pGatewayID, pNode->pDID);	
	}
	else
	{
		TRACE("TPClient[%s] device[%s] creation successfully complete!\n", pClient->xConfig.pGatewayID, pNode->pDID);	
	}
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTM_EP_PTR			pEPInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPInfo != NULL);
	
	FTM_RET	xRet;

	TRACE("TPClient[%s] sensor[%s] creation!\n", pClient->xConfig.pGatewayID, pEPInfo->pEPID);

	xRet = FTOM_TP_RESTAPI_EP_create(&pClient->xRESTApi, pEPInfo);
	if (xRet != FTM_RET_OK)
	{
		TRACE("TPClient[%s] sensor[%s] creation failed!\n", pClient->xConfig.pGatewayID, pEPInfo->pEPID);	
	}
	else
	{
		TRACE("TPClient[%s] sensor[%s] creation successfully complete!\n", pClient->xConfig.pGatewayID, pEPInfo->pEPID);	
	}
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_reportGWStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);
	ASSERT(pGatewayID != NULL);

	return	FTOM_MQTT_CLIENT_reportGWStatus(&pClient->xMQTT, pGatewayID, bStatus, ulTimeout);
}

FTM_RET	FTOM_TP_CLIENT_sendEPStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	return	FTOM_MQTT_CLIENT_publishEPStatus(&pClient->xMQTT, pEPID, bStatus, ulTimeout);
}

FTM_RET	FTOM_TP_CLIENT_sendEPData
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pDatas, 
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pDatas != NULL);

	return	FTOM_MQTT_CLIENT_publishEPData(&pClient->xMQTT, pEPID, pDatas, ulCount);
}

FTM_RET	FTOM_TP_CLIENT_respose
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nErrorCode,
	FTM_CHAR_PTR		pMessage
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsgID != NULL);
	ASSERT(pMessage != NULL);

	return	FTOM_MQTT_CLIENT_response(&pClient->xMQTT, pMsgID, nErrorCode, pMessage);
}

FTM_RET	FTOM_TP_CLIENT_controlActuator
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_CTRL			xCtrl,
	FTM_ULONG			ulDuration
)
{
	ASSERT(pClient != NULL);


	return	FTM_RET_OK;
}


FTM_RET	FTOM_TP_CLIENT_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pMsg != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTOM_MSG_PTR		pNewMsg = NULL;

	xRet = FTOM_MSG_copy(pMsg, &pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to copy message!\n");
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to push message!\n");
		FTOM_MSG_destroy(&pNewMsg);	
	}

	return	xRet;
}
