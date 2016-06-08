#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "nxjson.h"
#include "ftom_tp_client.h"
#include "ftom_mqtt_client.h"
#include "ftom_mqtt_client_tpgw.h"

static 
FTM_VOID_PTR FTOM_TP_CLIENT_process
(
	FTM_VOID_PTR 		pData
);

static
FTM_VOID_PTR FTOM_TP_CLIENT_linkManager
(
	FTM_VOID_PTR pData
);

static
FTM_RET	FTOM_TP_CLIENT_serverSync
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
);

static
FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTOM_EP_PTR			pEP
);

static
FTM_RET	FTOM_TP_CLIENT_pushMsg
(
	FTOM_TP_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
);

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
		ERROR("MQTT Client initialization was failed.\n");	
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

	FTOM_getDID(pClient->xConfig.pGatewayID, FTM_GWID_LEN);
	strcpy(pClient->xConfig.pHost, FTOM_TP_CLIENT_DEFAULT_BROKER);
	pClient->xConfig.usPort 			= FTOM_TP_CLIENT_DEFAULT_PORT;
	pClient->xConfig.ulRetryInterval	= FTOM_TP_CLIENT_DEFAULT_RETRY_INTERVAL;
	pClient->xConfig.ulReportInterval 	= FTOM_TP_CLIENT_DEFAULT_REPORT_INTERVAL;
	pClient->bStop = FTM_TRUE;

	xRet = FTOM_TP_RESTAPI_init(&pClient->xRESTApi);
	if (xRet != FTM_RET_OK)
	{
		ERROR("TP REST API initialize failed!\n");
		goto error;	
	}

	xRet = FTOM_MQTT_CLIENT_init(&pClient->xMQTT);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client creation failed!\n");
		goto error;
	}

	xRet = FTOM_MSGQ_init(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		goto error;	
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
	FTOM_MSGQ_final(&pClient->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_TP_CLIENT_CONFIG_PTR 	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTOM_MQTT_CLIENT_CONFIG	xMQTTConfig;

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_TP_CLIENT_CONFIG));

	strncpy(xMQTTConfig.pHost, 		pClient->xConfig.pHost, 	FTM_HOST_LEN);
	strncpy(xMQTTConfig.pUserID, 	pClient->xConfig.pGatewayID,FTM_USER_ID_LEN);
	strncpy(xMQTTConfig.pPasswd, 	pClient->xConfig.pAPIKey, 	FTM_PASSWD_LEN);
	strncpy(xMQTTConfig.pCertFile, 	pClient->xConfig.pCertFile, FTM_FILE_NAME_LEN);
	xMQTTConfig.usPort 			= pClient->xConfig.usPort;
	xMQTTConfig.ulRetryInterval = pClient->xConfig.ulRetryInterval;
	xMQTTConfig.bTLS 			= FTM_TRUE;
	xMQTTConfig.ulCBSet 		= 1;

	xRet = FTOM_MQTT_CLIENT_loadConfig(&pClient->xMQTT, &xMQTTConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client configation loading failed!\n");
		return	0;	
	}

	return	FTM_RET_OK;
}


FTM_RET	FTOM_TP_CLIENT_loadFromFile
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet !=  FTM_RET_OK)
	{
		ERROR("Configration loading failed!\n");
		return	xRet;	
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "tpclient", &xSection);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "id", pClient->xConfig.pGatewayID, FTM_GWID_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can not find the gateway id for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "cert", pClient->xConfig.pCertFile, FTM_FILE_NAME_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can not find the certificate information for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "apikey", pClient->xConfig.pAPIKey, FTM_PASSWD_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can not find a APIKEY information for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "host", pClient->xConfig.pHost, FTM_HOST_LEN);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can not find a host for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &pClient->xConfig.usPort);
		if (xRet != FTM_RET_OK)
		{
			INFO("Can not find a port for the TPClient!\n");
		}
	
		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "report_interval", &pClient->xConfig.ulReportInterval);
		if (xRet != FTM_RET_OK)
		{
			INFO("Can not find a report interval for the TPClient!\n");
		}
	
	}

	FTM_CONFIG_final(&xConfig);

	FTOM_MQTT_CLIENT_CONFIG	xMQTTConfig;

	strncpy(xMQTTConfig.pHost, 		pClient->xConfig.pHost, 	FTM_HOST_LEN);
	strncpy(xMQTTConfig.pUserID, 	pClient->xConfig.pGatewayID,FTM_USER_ID_LEN);
	strncpy(xMQTTConfig.pPasswd, 	pClient->xConfig.pAPIKey, 	FTM_PASSWD_LEN);
	strncpy(xMQTTConfig.pCertFile, 	pClient->xConfig.pCertFile, FTM_FILE_NAME_LEN);
	xMQTTConfig.usPort 			= pClient->xConfig.usPort;
	xMQTTConfig.ulRetryInterval = pClient->xConfig.ulRetryInterval;
	xMQTTConfig.bTLS 			= FTM_TRUE;
	xMQTTConfig.ulCBSet 		= 1;

	xRet = FTOM_MQTT_CLIENT_loadConfig(&pClient->xMQTT, &xMQTTConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MQTT Client configation loading failed!\n");
		return	0;	
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
	MESSAGE("%16s : %s\n", "Host", 		pClient->xConfig.pHost);
	MESSAGE("%16s : %d\n", "Port", 		pClient->xConfig.usPort);
	MESSAGE("%16s : %s\n", "API Key", 	pClient->xConfig.pAPIKey);
	MESSAGE("%16s : %s\n", "Cert File", pClient->xConfig.pCertFile);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_setCallback
(
	FTOM_TP_CLIENT_PTR 		pClient, 
	FTOM_SERVICE_ID 		xServiceID, 
	FTOM_SERVICE_CALLBACK 	fServiceCB
)
{
	ASSERT(pClient != NULL);

	pClient->xServiceID	= xServiceID;
	pClient->fServiceCB	= fServiceCB;

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

	FTM_RET			xRet;
	FTOM_MSG_PTR	pNewMsg;

	xRet = FTOM_MSG_copy(pMsg, &pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTOM_TP_CLIENT_pushMsg(pClient, pNewMsg);
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

FTM_VOID_PTR FTOM_TP_CLIENT_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET					xRet;
	FTOM_TP_CLIENT_PTR		pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTM_CHAR				pTopic[FTM_MQTT_TOPIC_LEN + 1];

	pClient->bConnected = FTM_FALSE;

	FTOM_TP_RESTAPI_setUserID(&pClient->xRESTApi, pClient->xConfig.pGatewayID);
	FTOM_TP_RESTAPI_setPasswd(&pClient->xRESTApi, pClient->xConfig.pAPIKey);
	FTOM_TP_RESTAPI_GW_setID(&pClient->xRESTApi, pClient->xConfig.pGatewayID);
	FTOM_TP_RESTAPI_setVerbose(&pClient->xRESTApi, FTM_TRUE);


	sprintf(pTopic, "v/a/g/%s/res", pClient->xConfig.pGatewayID);
	FTOM_MQTT_CLIENT_subscribe(&pClient->xMQTT, pTopic);
	sprintf(pTopic, "v/a/g/%s/req", pClient->xConfig.pGatewayID);
	FTOM_MQTT_CLIENT_subscribe(&pClient->xMQTT, pTopic);

	pClient->bStop 		= FTM_FALSE;
	TRACE("TPClient[%s] started.\n", pClient->xConfig.pGatewayID);
	pthread_create(&pClient->xLinkManager, NULL, FTOM_TP_CLIENT_linkManager, pClient);

	FTOM_MQTT_CLIENT_start(&pClient->xMQTT);

	while(!pClient->bStop)
	{
		FTOM_MSG_PTR	pBaseMsg;

		xRet = FTOM_MSGQ_timedPop(&pClient->xMsgQ, 1000000, &pBaseMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pBaseMsg->xType)
			{
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
					FTOM_TP_CLIENT_serverSync(pClient, FTM_TRUE);
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
	FTOM_MQTT_CLIENT_final(&pClient->xMQTT);
	TRACE("TPClient[%s] stopped.\n", pClient->xConfig.pGatewayID);

	return 0;
}

FTM_VOID_PTR FTOM_TP_CLIENT_linkManager
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;
	FTM_TIME			xTime, xCurrentTime, xDiffTime, xTimeInterval;
	FTM_TIMER			xTimer;
	FTM_TIMER			xReportTimer;
	FTM_BOOL			bConnected = FTM_FALSE;
	FTM_UINT64			ullDiffTime;

	TRACE("TPClient[%s] link manager was started.\n", pClient->xConfig.pGatewayID);

	FTM_TIME_getCurrent(&xTime);
	FTM_TIME_set(&xTimeInterval, 1000);
	FTM_TIMER_init(&xTimer, 		pClient->xConfig.ulRetryInterval * 1000000);
	FTM_TIMER_init(&xReportTimer, 	pClient->xConfig.ulReportInterval * 1000000);

	FTM_TIME_add(&xTime, &xTimeInterval, &xTime);

	while(!pClient->bStop)
	{
		FTOM_MQTT_CLIENT_isConnected(&pClient->xMQTT, &pClient->bConnected);

		if(!pClient->bConnected)
		{
			if (bConnected)
			{
				FTM_TIMER_init(&xTimer, pClient->xConfig.ulRetryInterval * 1000000);
				bConnected = pClient->bConnected;
			}
			else
			{
				if (FTM_TIMER_isExpired(&xTimer))
				{
					FTM_TIMER_addSeconds(&xTimer, pClient->xConfig.ulRetryInterval);
				}
			}
		}
		else
		{
			if (!bConnected)
			{
				FTOM_TP_CLIENT_reportGWStatus(pClient, pClient->xConfig.pGatewayID, FTM_TRUE, pClient->xConfig.ulReportInterval);
				FTM_TIMER_init(&xReportTimer, 	pClient->xConfig.ulReportInterval * 1000000);
				bConnected = pClient->bConnected;
			}
			else
			{
				if (FTM_TIMER_isExpired(&xReportTimer))
				{
					FTOM_TP_CLIENT_reportGWStatus(pClient, pClient->xConfig.pGatewayID, FTM_TRUE, pClient->xConfig.ulReportInterval);
					FTM_TIMER_addSeconds(&xReportTimer, pClient->xConfig.ulReportInterval);
				}
			}
		}

		FTM_TIME_getCurrent(&xCurrentTime);
		FTM_TIME_sub(&xTime, &xCurrentTime, &xDiffTime);

		ullDiffTime = 0;
		FTM_TIME_toMS(&xDiffTime, &ullDiffTime);
		if (ullDiffTime > 0)
		{
			FTM_TIME_add(&xTime, &xTimeInterval, &xTime);
			usleep(ullDiffTime * 1000);
		}
		else
		{
			FTM_TIME_add(&xCurrentTime, &xTimeInterval, &xTime);
			usleep(1000);
		}
	}

	TRACE("TPClient[%s] link manager was stopped.\n", pClient->xConfig.pGatewayID);

	return 0;
}

FTM_RET	FTOM_TP_CLIENT_pushMsg
(
	FTOM_TP_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);
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

	xRet = FTOM_TP_CLIENT_pushMsg(pClient, pMsg);
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
		FTM_ULONG   ulSensorCount = 0;
		FTM_ULONG	ulEPCount = 0;

		MESSAGE("%16s : %s\n",  "ID",   pGateway->pID);    
		MESSAGE("%16s : %s\n",  "Name", pGateway->pName);    
		MESSAGE("%16s : %lu\n", "Report Interval", pGateway->ulReportInterval);    
		MESSAGE("%16s : %llu\n","Installed Time", pGateway->ullCTime);    
		MESSAGE("%16s : %llu\n","Modified Time", pGateway->ullMTime);    
		MESSAGE("%16s : ", "Sensors");

		FTM_LIST_count(pGateway->pSensorList, &ulSensorCount);
		for(i = 0 ; i < ulSensorCount ; i++)
		{   
			FTM_CHAR_PTR    pSensorID = NULL;
			xRet = FTM_LIST_getAt(pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensorID);
			if (xRet == FTM_RET_OK)
			{   
				MESSAGE("%16s ", pSensorID);
			}   
		}   
		MESSAGE("\n");

		for(i = 0 ; i < ulSensorCount ; i++)
		{   
			FTM_CHAR_PTR    pSensorID = NULL;
			xRet = FTM_LIST_getAt(pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensorID);
			if (xRet == FTM_RET_OK)
			{   
				FTOM_EP_PTR	pEP;

				xRet = FTOM_EP_get(pSensorID, &pEP);
				if (xRet == FTM_RET_OK)
				{
					pEP->xServer.bRegistered = FTM_TRUE;
				}
			}   
		}   

		if (bAutoRegister)
		{
			xRet = FTOM_EP_count(&ulEPCount);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}

			for(i = 0 ; i < ulEPCount ; i++)
			{
				FTOM_EP_PTR	pEP = NULL;

				xRet = FTOM_EP_getAt(i, &pEP);
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't get EP info at %d\n", i);
					continue;	
				}

				if (!pEP->xServer.bRegistered)
				{
					FTOM_TP_CLIENT_EP_register(pClient, pEP);	
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
			
FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient, 
	FTOM_EP_PTR			pEP
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEP != NULL);
	
	FTM_RET	xRet;

	TRACE("TPClient[%s] sensor[%s] creation!\n", pClient->xConfig.pGatewayID, pEP->xInfo.pEPID);

	xRet = FTOM_TP_RESTAPI_EP_create(&pClient->xRESTApi, &pEP->xInfo);
	if (xRet != FTM_RET_OK)
	{
		TRACE("TPClient[%s] sensor[%s] creation failed!\n", pClient->xConfig.pGatewayID, pEP->xInfo.pEPID);	
	}
	else
	{
		TRACE("TPClient[%s] sensor[%s] creation successfully complete!\n", pClient->xConfig.pGatewayID, pEP->xInfo.pEPID);	
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

	TRACE("Report Gateway[%s] status!\n", pGatewayID);
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

	TRACE("EP[%s] send status!\n", pEPID);
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

	TRACE("EP[%s] send data!\n", pEPID);
	return	FTOM_MQTT_CLIENT_publishEPData(&pClient->xMQTT, pEPID, pDatas, ulCount);
}


