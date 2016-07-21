#include <stdio.h>
#include <curl/curl.h>
#include <cJSON.h>
#include "ftom.h"
#include "ftom_tp_restapi.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static
size_t FTOM_TP_RESTAPI_CB_readString
(
 	FTM_VOID_PTR	pBuff, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
);

static 
size_t FTOM_TP_RESTAPI_CB_response
(
 	FTM_VOID_PTR	pContents, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
);

static
FTM_RET	FTOM_TP_RESTAPI_setGetURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_RESTAPI_setPutURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_RESTAPI_setPostURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_RESTAPI_setDeleteURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_RESTAPI_putBody
(
	FTOM_TP_RESTAPI_PTR	pClient,
	cJSON _PTR_			pBody
);

static
FTM_RET	FTOM_TP_RESTAPI_postBody
(
	FTOM_TP_RESTAPI_PTR	pClient,
	cJSON _PTR_			pBody
);

static
FTM_RET	FTOM_TP_RESTAPI_perform
(
	FTOM_TP_RESTAPI_PTR	pClient
);


typedef struct FTOM_TP_RESTAPI_SENSOR_TYPE_INFO_STRUCT
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pName;
	FTM_CHAR_PTR	pDriver;
	FTM_CHAR_PTR	pModel;
	FTM_CHAR_PTR	pCategory;
} FTOM_TP_RESTAPI_SENSOR_TYPE_INFO, _PTR_ FTOM_TP_RESTAPI_SENSOR_TYPE_INFO_PTR; 

static
FTOM_TP_RESTAPI_SENSOR_TYPE_INFO pSensorTypeInfo[] = 
{
	{
		.xType = FTM_EP_TYPE_TEMPERATURE,
		.pName = "temperature",
		.pDriver="futureSensor",
		.pModel= "futureTemp",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_HUMIDITY,
		.pName = "humidity",
		.pDriver="futureSensor",
		.pModel= "futureHumi",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_VOLTAGE,
		.pName = "voltage",
		.pDriver="futureSensor",
		.pModel= "futureVoltage",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_CURRENT,
		.pName = "current",
		.pDriver="futureSensor",
		.pModel= "futureCurrent",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_DI,
		.pName = "onoff",
		.pDriver="futureSensor",
		.pModel= "futureDi",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_DO,
		.pName = "powerSwitch",
		.pDriver="futureActuator",
		.pModel= "futureDo",
		.pCategory="actuator"
	},
	{
		.xType = FTM_EP_TYPE_GAS,
		.pName = "co2",
		.pDriver="futureSensor",
		.pModel= "futureGas",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_POWER,
		.pName = "power",
		.pDriver="futureSensor",
		.pModel= "futurePower",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_COUNT,
		.pName = "count",
		.pDriver="futureSensor",
		.pModel= "futureCount",
		.pCategory="sensor"
	},
	{
		.xType = FTM_EP_TYPE_PRESSURE,
		.pName = "pressure",
		.pDriver="futureSensor",
		.pModel= "futurePressure",
		.pCategory="sensor"
	}
};

static
FTM_CHAR	pRESTAPIDefaultBase[] = "https://api.thingplus.net/v1";

static 
FTM_BOOL	bGlobalInit = FTM_FALSE;
static 
FTM_BOOL	bVerbose = FTM_FALSE;
static 
FTM_BOOL	bDataDump = FTM_FALSE;

FTM_RET	FTOM_TP_RESTAPI_create
(
	FTOM_TP_RESTAPI_PTR _PTR_ ppClient,
	FTM_CHAR_PTR		pUserID,
	FTM_CHAR_PTR		pAPIKey
)
{
	ASSERT(ppClient != NULL);
	FTM_RET		xRet;
	FTOM_TP_RESTAPI_PTR	pClient = NULL;

	pClient = (FTOM_TP_RESTAPI_PTR)FTM_MEM_malloc(sizeof(FTOM_TP_RESTAPI));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet =  FTOM_TP_RESTAPI_init(pClient);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pClient);
		return	xRet;	
	}

	*ppClient = pClient;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_destroy
(
	FTOM_TP_RESTAPI_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_TP_RESTAPI_final(*ppClient);
	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_init
(
	FTOM_TP_RESTAPI_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (!bGlobalInit)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		bGlobalInit = FTM_TRUE;	
	}

	memset(pClient, 0, sizeof(FTOM_TP_RESTAPI));

  	pClient->pCURL = curl_easy_init();
  	if(pClient->pCURL == NULL) 
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pClient->xConfig.pBaseURL, pRESTAPIDefaultBase);

	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "Content-Type:application/json");

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPHEADER, pClient->pHTTPHeader);
	if (bVerbose)
	{
 		curl_easy_setopt(pClient->pCURL, CURLOPT_VERBOSE, 1L);
	}
	else
	{
 		curl_easy_setopt(pClient->pCURL, CURLOPT_VERBOSE, 0L);
	}
	//curl_easy_setopt(pClient->pCURL, CURLOPT_SSL_VERIFYPEER, 0L);
	//curl_easy_setopt(pClient->pCURL, CURLOPT_SSL_VERIFYHOST, 0L);
	//curl_easy_setopt(pClient->pCURL, CURLOPT_CAINFO, "/etc/ca-cert.pem");
	//curl_easy_setopt(pClient->pCURL, CURLOPT_SSL_ENABLE_ALPN, 0L);
	//curl_easy_setopt(pClient->pCURL, CURLOPT_SSL_ENABLE_NPN, 0L);
	curl_easy_setopt(pClient->pCURL, CURLOPT_WRITEFUNCTION, FTOM_TP_RESTAPI_CB_response);
	curl_easy_setopt(pClient->pCURL, CURLOPT_WRITEDATA, (FTM_VOID_PTR)pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_final
(
	FTOM_TP_RESTAPI_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->pCURL != NULL)
	{
    	curl_easy_cleanup(pClient->pCURL);
		pClient->pCURL = NULL;
	}

	if (pClient->pHTTPHeader != NULL)
	{
		curl_slist_free_all(pClient->pHTTPHeader);
		pClient->pHTTPHeader = NULL;		
	}

	if (pClient->pData != NULL)
	{
		free(pClient->pData);
		pClient->pData = NULL;
	}

	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
		pClient->pResp = NULL;
	}

	if (bGlobalInit)
	{
  		curl_global_cleanup();

		bGlobalInit = FTM_FALSE;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setConfig
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTOM_TP_RESTAPI_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_TP_RESTAPI_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setUserID
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pUserID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pUserID != NULL);
	FTM_CHAR	pBuff[1024];

	strncpy(pClient->xConfig.pUserID, pUserID, FTM_USER_ID_LEN);

	if (pClient->pHTTPHeader != NULL)
	{
		curl_slist_free_all(pClient->pHTTPHeader);
		pClient->pHTTPHeader = NULL;		
	}

	if (strlen(pClient->xConfig.pUserID) != 0)
	{
		sprintf(pBuff, "username:%s", pClient->xConfig.pUserID);
		pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, pBuff);
	}

	if (strlen(pClient->xConfig.pPasswd) != 0)
	{
		sprintf(pBuff, "apikey:%s", pClient->xConfig.pPasswd);
		pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, pBuff);
	}

	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "Content-Type:application/json");

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPHEADER, pClient->pHTTPHeader);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setPasswd
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pPasswd
)
{
	ASSERT(pClient != NULL);
	ASSERT(pPasswd != NULL);
	FTM_CHAR	pBuff[1024];

	strncpy(pClient->xConfig.pPasswd, pPasswd, FTM_PASSWD_LEN);

	if (pClient->pHTTPHeader != NULL)
	{
		curl_slist_free_all(pClient->pHTTPHeader);
		pClient->pHTTPHeader = NULL;		
	}

	if (strlen(pClient->xConfig.pUserID) != 0)
	{
		sprintf(pBuff, "username:%s", pClient->xConfig.pUserID);
		pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, pBuff);
	}

	if (strlen(pClient->xConfig.pPasswd) != 0)
	{
		sprintf(pBuff, "apikey:%s", pClient->xConfig.pPasswd);
		pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, pBuff);
	}

	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "Content-Type:application/json");

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPHEADER, pClient->pHTTPHeader);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setGatewayID
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pGatewayID != NULL);

	strncpy(pClient->xConfig.pGatewayID, pGatewayID, FTM_DID_LEN);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setBaseURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pBaseURL
)
{
	ASSERT(pClient != NULL);
	ASSERT(pBaseURL != NULL);

	strncpy(pClient->xConfig.pBaseURL, pBaseURL, FTM_DID_LEN);

	return	FTM_RET_OK;
}

/******************************************************
 * Gateway management
 ******************************************************/
FTM_RET	FTOM_TP_RESTAPI_GW_getModel
(
	FTOM_TP_RESTAPI_PTR 	pClient,
	FTM_ULONG			ulModel
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	xRet = FTOM_TP_RESTAPI_setGetURL(pClient, "/gatewayModels/%lu", ulModel);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

	if ((bDataDump) && (xRet == FTM_RET_OK))
	{
		printf("%s", pClient->pResp);
	}
	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_GW_getInfo
(
	FTOM_TP_RESTAPI_PTR 	pClient,
	FTOM_TP_GATEWAY_PTR	pGateway
)
{
	ASSERT(pClient != NULL);
	ASSERT(pGateway!= NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pRoot = NULL;
	cJSON _PTR_ pDevices = NULL;
	cJSON _PTR_ pSensors = NULL;
	cJSON _PTR_ pItem = NULL;

	xRet = FTOM_TP_RESTAPI_setGetURL(pClient, "/gateways/%s", pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
		goto finish;
	}

	pRoot = cJSON_Parse(pClient->pResp);
	if (pRoot == NULL)
	{
		xRet = FTM_RET_COMM_INVALID_VALUE;
		ERROR2(xRet, "Invalid json format!\n");
		goto finish;	
	}

	pItem = cJSON_GetObjectItem(pRoot, "id") ;
	if ((pItem == NULL) || (pItem->type != cJSON_String))
	{
		xRet = FTM_RET_COMM_INVALID_VALUE;	
		ERROR2(xRet, "Can't found id!\n");
		goto finish;
	}
	strncpy(pGateway->pID, pItem->valuestring, FTM_DID_LEN);

	pItem = cJSON_GetObjectItem(pRoot, "name") ;
	if ((pItem != NULL) && (pItem->type == cJSON_String))
	{
		strncpy(pGateway->pName, pItem->valuestring, FTM_NAME_LEN);
	}

	pItem = cJSON_GetObjectItem(pRoot, "reportInterval") ;
	if ((pItem != NULL) && (pItem->type == cJSON_Number))
	{
		pGateway->ulReportInterval = pItem->valueint;
	}

	pItem = cJSON_GetObjectItem(pRoot, "ctime") ;
	if ((pItem != NULL) && (pItem->type == cJSON_String))
	{
		pGateway->ullCTime = strtoull(pItem->valuestring, 0, 10);
	}

	pItem = cJSON_GetObjectItem(pRoot, "mtime") ;
	if ((pItem != NULL) && (pItem->type == cJSON_String))
	{
		pGateway->ullMTime = strtoull(pItem->valuestring, 0, 10);
	}

	pDevices = cJSON_GetObjectItem(pRoot, "devices");
	if (pDevices != NULL)
	{
		FTM_INT	i;

		if (pDevices->type != cJSON_Array)
		{
			xRet = FTM_RET_COMM_INVALID_VALUE;	
			ERROR2(xRet, "Devices information is not array!\n");
			goto finish;	
		}

		for(i = 0; ; i++)
		{
			pItem = cJSON_GetArrayItem(pDevices, i);
			if ((pItem == NULL) || (pItem->type != cJSON_String))
			{
				break;	
			}
		
			FTM_ULONG		ulLen = strlen(pItem->valuestring);
			FTM_CHAR_PTR	pDeviceID = FTM_MEM_malloc(ulLen+1);
			if (pDeviceID == NULL)
			{
				xRet = FTM_RET_NOT_ENOUGH_MEMORY;
				ERROR2(xRet, "Not enough memory!\n");	
				break;
			}

			strcpy(pDeviceID, pItem->valuestring);

			FTM_LIST_append(pGateway->pDeviceList, pDeviceID);
		}
	}

	pSensors = cJSON_GetObjectItem(pRoot, "sensors");
	if (pSensors != NULL)
	{
		FTM_INT	i;

		if (pSensors->type != cJSON_Array)
		{
			xRet = FTM_RET_COMM_INVALID_VALUE;	
			ERROR2(xRet, "Sensor information is not array!\n");
			goto finish;	
		}

		for(i = 0; ; i++)
		{
			pItem = cJSON_GetArrayItem(pSensors, i);
			if ((pItem == NULL) || (pItem->type != cJSON_String))
			{
				break;	
			}
		
			FTM_ULONG		ulLen = strlen(pItem->valuestring);
			FTM_CHAR_PTR	pSensorID = FTM_MEM_malloc(ulLen+1);
			if (pSensorID == NULL)
			{
				xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
				ERROR2(xRet, "Not enough memory!\n");	
				break;
			}

			strcpy(pSensorID, pItem->valuestring);

			FTM_LIST_append(pGateway->pSensorList, pSensorID);
		}
	}

finish:

	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}
	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_GW_setStatus
(
	FTOM_TP_RESTAPI_PTR 	pClient,
	FTM_BOOL			bStatus,	
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;
	cJSON _PTR_ 	pRoot = NULL;

	xRet = FTOM_TP_RESTAPI_setPutURL(pClient, "/gateways/%s/status", pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	pRoot = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pRoot, "value", (bStatus)?"on":"off");
	cJSON_AddNumberToObject(pRoot, "timeout", ulTimeout);

	xRet = FTOM_TP_RESTAPI_putBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

/******************************************************
 * Device management
 ******************************************************/
FTM_RET	FTOM_TP_RESTAPI_DEVICE_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pDeviceID,
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pModel
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDeviceID != NULL);
	ASSERT(pName != NULL);
	ASSERT(pModel!= NULL);
	FTM_RET	xRet;
	cJSON _PTR_ 	pRoot = NULL;

	pRoot = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pRoot, "reqId", pDeviceID);
	cJSON_AddStringToObject(pRoot, "name", 	pName);
	cJSON_AddStringToObject(pRoot, "model", pModel);

	xRet = FTOM_TP_RESTAPI_setPostURL(pClient, "/gateways/%s/devices", pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_postBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_DEVICE_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pDeviceID
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_RESTAPI_setDeleteURL(pClient, "/gateways/%s/devices/%s", pClient->xConfig.pGatewayID, pDeviceID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

	return	xRet;
}

/*********************************************************
 * Sensor management 
 *********************************************************/
FTM_RET	FTOM_TP_RESTAPI_SENSOR_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_CHAR_PTR		pSensorID,
	FTM_CHAR_PTR		pType,
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pDeviceID,
	FTM_CHAR_PTR		pAddress,
	FTM_CHAR_PTR		pSequence
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;
	FTM_INT			i;
	cJSON _PTR_ 	pRoot = NULL;

	FTOM_TP_RESTAPI_SENSOR_TYPE_INFO_PTR pSensorInfo = NULL;

	if ((pSensorID == NULL) || (pName == NULL) || (pType == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	for(i = 0 ; i < sizeof(pSensorTypeInfo) / sizeof(FTOM_TP_RESTAPI_SENSOR_TYPE_INFO); i++)
	{
		if (strcasecmp(pType, pSensorTypeInfo[i].pName) == 0)
		{
			pSensorInfo = &pSensorTypeInfo[i];	
			break;
		}
	}

	if (pSensorInfo == NULL)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	pRoot = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pRoot, "reqId", pSensorID);
	cJSON_AddStringToObject(pRoot, "name", pName);
	if (pDeviceID != NULL)
	{
		cJSON_AddStringToObject(pRoot, "deviceId", pDeviceID);
	}
	else
	{
		cJSON_AddStringToObject(pRoot, "deviceId", pClient->xConfig.pGatewayID);
	}
	cJSON_AddStringToObject(pRoot, "type", pType);
	cJSON_AddStringToObject(pRoot, "network", "future");
	if (pAddress != NULL)
	{
		cJSON_AddStringToObject(pRoot, "address", pAddress);
	}

	cJSON_AddStringToObject(pRoot, "driverName", pSensorInfo->pDriver);
	cJSON_AddStringToObject(pRoot, "model", pSensorInfo->pModel);
	if (pSequence != NULL)
	{
		cJSON_AddStringToObject(pRoot, "sequence", pSequence);
	}

	cJSON_AddStringToObject(pRoot, "category", pSensorInfo->pCategory);


	xRet = FTOM_TP_RESTAPI_setPostURL(pClient, "/gateways/%s/sensors", pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_postBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_SENSOR_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_RESTAPI_setDeleteURL(pClient, "/gateways/%s/sensors/%s", pClient->xConfig.pGatewayID, pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_SENSOR_getStatus
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL_PTR		pbStatus
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_RESTAPI_setGetURL(pClient, 
				"/gateways/%s/sensors/%s/status", 
				pClient->xConfig.pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_SENSOR_setStatus
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;
	cJSON _PTR_ 	pRoot = NULL;

	xRet = FTOM_TP_RESTAPI_setPutURL(pClient, 
				"/gateways/%s/sensors/%s/status", 
				pClient->xConfig.pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	pRoot = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pRoot, "value", (bStatus)?"on":"off");
	cJSON_AddNumberToObject(pRoot, "timeout", ulTimeout);

	xRet = FTOM_TP_RESTAPI_putBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_SENSOR_getValue
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValue
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_RESTAPI_setGetURL(pClient, 
				"/gateways/%s/sensors/%s/series", 
				pClient->xConfig.pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_SENSOR_setValues
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValues,
	FTM_ULONG			ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;
	FTM_INT			i;
	cJSON _PTR_ 	pRoot = NULL;

	xRet = FTOM_TP_RESTAPI_setPutURL(pClient, 
				"/gateways/%s/sensors/%s/series", 
				pClient->xConfig.pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	pRoot = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pItem = NULL;

		cJSON_AddItemToArray(pRoot, pItem = cJSON_CreateObject());
		cJSON_AddNumberToObject(pItem, "time", (double)pValues[i].ulTime * 1000);
		cJSON_AddStringToObject(pItem, "value",FTM_VALUE_print(&pValues[i].xValue));
	}

	xRet = FTOM_TP_RESTAPI_putBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client put data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_SENSOR_getList
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTOM_TP_RESTAPI_SENSOR_PTR	pSensors,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;
	cJSON _PTR_ pRoot;
	FTM_ULONG		i, ulCount = 0;

	xRet = FTOM_TP_RESTAPI_setGetURL(pClient, "/gateways/%s/sensors", pClient->xConfig.pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_RESTAPI_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "An error occurred while performing the client.\n");
	}

	pRoot = cJSON_Parse(pClient->pResp);
	if ((pRoot == NULL) || (pRoot->type != cJSON_Array))
	{
		xRet = FTM_RET_COMM_INVALID_VALUE;	
		goto finish;
	}

	
	for( i = 0 ; ulCount < ulMaxCount ; i++)
	{
		cJSON _PTR_ pSensorInfo;
		cJSON _PTR_ pItem;
		FTOM_TP_RESTAPI_SENSOR		xSensor;

		pSensorInfo = cJSON_GetArrayItem(pRoot, i);
		if (pSensorInfo == NULL)
		{
			break;	
		}

		memset(&xSensor, 0, sizeof(xSensor));

		pItem = cJSON_GetObjectItem(pSensorInfo, "id"); // get object's property by key
		if ((pItem == NULL) || (pItem->type != cJSON_String))
		{
			continue;	
		}
		strncpy(xSensor.pID, pItem->valuestring, FTM_EPID_LEN);

		pItem = cJSON_GetObjectItem(pSensorInfo, "type");
		if ((pItem == NULL) || (pItem->type != cJSON_String))
		{
			continue;	
		}

		xRet = FTM_EP_strToType((FTM_CHAR_PTR)pItem->valuestring, &xSensor.xType);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "name");
		if ((pItem != NULL) || (pItem->type != cJSON_String))
		{
			strncpy(xSensor.pName, pItem->valuestring, FTM_NAME_LEN);	
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "owner");
		if ((pItem != NULL) || (pItem->type != cJSON_String))
		{
			strncpy(xSensor.pOwnerID, pItem->valuestring, FTM_DID_LEN);	
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "deviceId");
		if ((pItem != NULL) || (pItem->type != cJSON_String))
		{
			strncpy(xSensor.pDeviceID, pItem->valuestring, FTM_DID_LEN);	
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "address");
		if ((pItem != NULL) || (pItem->type != cJSON_Number))
		{
			xSensor.ulAddress = pItem->valueint;
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "sequence");
		if ((pItem != NULL) || (pItem->type != cJSON_Number))
		{
			xSensor.ulSequence = pItem->valueint;
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "ctime");
		if ((pItem != NULL) || (pItem->type != cJSON_String))
		{
			xSensor.ullCTime = strtoull(pItem->valuestring, 0, 10);
		}

		pItem = cJSON_GetObjectItem(pSensorInfo, "mtime");
		if ((pItem != NULL) || (pItem->type != cJSON_Number))
		{
			xSensor.ullMTime = pItem->valueint;
		}

		memcpy(&pSensors[ulCount++], &xSensor, sizeof(xSensor));
	}

	*pulCount = ulCount;

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	xRet;
}

/*********************************************************
 * Node management 
 *********************************************************/
FTM_RET	FTOM_TP_RESTAPI_NODE_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_NODE_PTR		pNodeInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pNodeInfo != NULL);

	FTM_RET	xRet;

	xRet = FTOM_TP_RESTAPI_DEVICE_create(
				pClient, 
				pNodeInfo->pDID,
				pNodeInfo->pName,
				pNodeInfo->pModel);

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_NODE_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	return	FTOM_TP_RESTAPI_DEVICE_delete(pClient, pDID);
}

FTM_RET	FTOM_TP_RESTAPI_NODE_getList
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_NODE_PTR			pNodeInfos,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	return	FTM_RET_OK;
}

/*********************************************************
 * End Point management 
 *********************************************************/
FTM_RET	FTOM_TP_RESTAPI_EP_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_EP_PTR			pEPInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPInfo != NULL);

	FTM_RET	xRet;
	FTM_INT	i;
	FTOM_TP_RESTAPI_SENSOR_TYPE_INFO_PTR	pTypeInfo = NULL;

	for(i = 0 ; i < sizeof(pSensorTypeInfo) / sizeof(FTOM_TP_RESTAPI_SENSOR_TYPE_INFO) ; i++)
	{
		if (pEPInfo->xType == pSensorTypeInfo[i].xType)
		{
			pTypeInfo = &pSensorTypeInfo[i];	
			break;
		}
	}

	if (pTypeInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTOM_TP_RESTAPI_SENSOR_create(
				pClient, 
				pClient->xConfig.pGatewayID, 
				pEPInfo->pEPID,
				pTypeInfo->pName,
				pEPInfo->pName,
				pEPInfo->pDID,
				"1",
				pEPInfo->pEPID);

	return	xRet;
}

FTM_RET	FTOM_TP_RESTAPI_EP_getList
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_EP_PTR			pEPInfos,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	return	FTM_RET_OK;
}

/*********************************************************
 * Library configuration
 *********************************************************/
 FTM_RET	FTOM_TP_RESTAPI_setVerbose
 (
 	FTOM_TP_RESTAPI_PTR	pClient,
 	FTM_BOOL	bOn
 )
 {
 	ASSERT(pClient != NULL);

	bVerbose = bOn;

	if (bVerbose)
	{
 		curl_easy_setopt(pClient->pCURL, CURLOPT_VERBOSE, 1L);
	}
	else
	{
 		curl_easy_setopt(pClient->pCURL, CURLOPT_VERBOSE, 0L);
	}
	return	FTM_RET_OK;
 }

 FTM_RET	FTOM_TP_RESTAPI_setDataDump
 (
 	FTM_BOOL	bOn
 )
 {
	bDataDump = bOn;

	return	FTM_RET_OK;
 }

/*********************************************************
 * Internal functions
 *********************************************************/
FTM_RET	FTOM_TP_RESTAPI_setGetURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->xConfig.pBaseURL);

	va_start(pArgs, pFormat);
	vsprintf(&pClient->pURL[strlen(pClient->pURL)], pFormat, pArgs);
	va_end(pArgs);

	curl_easy_setopt(pClient->pCURL, CURLOPT_URL, pClient->pURL);
	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
	}

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPGET, 1L);

	pClient->pResp = FTM_MEM_malloc(1);
	pClient->pResp[0] = '\0';
	pClient->ulRespLen = 0;
	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setPutURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->xConfig.pBaseURL);

	va_start(pArgs, pFormat);
	vsprintf(&pClient->pURL[strlen(pClient->pURL)], pFormat, pArgs);
	va_end(pArgs);

	curl_easy_setopt(pClient->pCURL, CURLOPT_URL, pClient->pURL);
	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
	}

	curl_easy_setopt(pClient->pCURL, CURLOPT_PUT, 1L);

	pClient->pResp = FTM_MEM_malloc(1);
	pClient->pResp[0] = '\0';
	pClient->ulRespLen = 0;
	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setPostURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->xConfig.pBaseURL);

	va_start(pArgs, pFormat);
	vsprintf(&pClient->pURL[strlen(pClient->pURL)], pFormat, pArgs);
	va_end(pArgs);

	curl_easy_setopt(pClient->pCURL, CURLOPT_URL, pClient->pURL);
	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
	}

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPPOST, 1L);

	pClient->pResp = FTM_MEM_malloc(1);
	pClient->pResp[0] = '\0';
	pClient->ulRespLen = 0;
	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_setDeleteURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->xConfig.pBaseURL);

	va_start(pArgs, pFormat);
	vsprintf(&pClient->pURL[strlen(pClient->pURL)], pFormat, pArgs);
	va_end(pArgs);

	curl_easy_setopt(pClient->pCURL, CURLOPT_URL, pClient->pURL);
	if (pClient->pResp != NULL)
	{
		FTM_MEM_free(pClient->pResp);
	}

	curl_easy_setopt(pClient->pCURL,CURLOPT_CUSTOMREQUEST,"DELETE"); 

	pClient->pResp = FTM_MEM_malloc(1);
	pClient->pResp[0] = '\0';
	pClient->ulRespLen = 0;
	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_putBody
(
	FTOM_TP_RESTAPI_PTR	pClient,
	cJSON _PTR_			pBody
)
{
	ASSERT(pClient != NULL);

	if (pClient->pData != NULL)
	{
		free(pClient->pData);	
		pClient->pData = NULL;
	}
	pClient->pData = cJSON_Print(pBody);

	curl_easy_setopt(pClient->pCURL, CURLOPT_READDATA, pClient->pData);
	curl_easy_setopt(pClient->pCURL, CURLOPT_READFUNCTION, FTOM_TP_RESTAPI_CB_readString);
	curl_easy_setopt(pClient->pCURL, CURLOPT_INFILESIZE, strlen(pClient->pData));

	if (bDataDump)
	{
		fprintf(stdout, "\n### PUT ###\n");
		fprintf(stdout, "%s", pClient->pData);
		fprintf(stdout, "\n################\n\n");
		fflush(stdout);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_postBody
(
	FTOM_TP_RESTAPI_PTR	pClient,
	cJSON _PTR_			pBody
)
{
	ASSERT(pClient != NULL);

	if (pClient->pData != NULL)
	{
		free(pClient->pData);	
		pClient->pData = NULL;
	}

	pClient->pData = cJSON_Print(pBody);

	curl_easy_setopt(pClient->pCURL, CURLOPT_POSTFIELDS, pClient->pData);

	if (bDataDump)
	{
		fprintf(stdout, "\n### POST ###\n");
		fprintf(stdout, "%s", pClient->pData);
		fprintf(stdout, "\n################\n\n");
		fflush(stdout);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_RESTAPI_perform
(
	FTOM_TP_RESTAPI_PTR	pClient
)
{
	CURLcode res;

    res = curl_easy_perform(pClient->pCURL);
    if(res != CURLE_OK)
	{
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	  return	FTM_RET_ERROR;
	}

	if (bDataDump)
	{
		fprintf(stdout, "\n### RESPONSE ###\n");
		fprintf(stdout, "%s", pClient->pResp);
		fprintf(stdout, "\n################\n\n");
		fflush(stdout);
	}

	return	FTM_RET_OK;
}

size_t FTOM_TP_RESTAPI_CB_readString
(
 	FTM_VOID_PTR	pBuff, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
)
{
	ASSERT(pBuff != NULL);
	ASSERT(pUser != NULL);

	strcpy(pBuff, pUser);

	return	strlen((FTM_CHAR_PTR)pUser);
}

size_t FTOM_TP_RESTAPI_CB_response
(
 	FTM_VOID_PTR	pContents, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
)
{
	ASSERT(pContents != NULL);
	ASSERT(pUser != NULL);

	FTM_RET	xRet;
	FTOM_TP_RESTAPI_PTR	pClient = (FTOM_TP_RESTAPI_PTR)pUser;
	FTM_INT				nRealSize = nSize * nMemB;
	FTM_CHAR_PTR		pMem = NULL;

	if (nRealSize == 0)
	{
		return	0;	
	}

	pMem = (FTM_VOID_PTR)FTM_MEM_malloc(pClient->ulRespLen + nRealSize + 1);
	if (pMem == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		return	0;	
	}

	strcpy(pMem, pClient->pResp);
	memcpy(&pMem[pClient->ulRespLen], pContents, nRealSize);
	FTM_MEM_free(pClient->pResp);
	pClient->pResp = pMem;

	pClient->ulRespLen += nRealSize;
	pClient->pResp[pClient->ulRespLen] = 0;

	return nRealSize;
}

