#include <stdio.h>
#include <curl/curl.h>
#include <cJSON.h>
#include "ftom.h"
#include "ftom_tp_client.h"
#include "nxjson.h"

static
size_t FTOM_TP_CLIENT_CB_readString
(
 	FTM_VOID_PTR	pBuff, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
);

static 
size_t FTOM_TP_CLIENT_CB_response
(
 	FTM_VOID_PTR	pContents, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
);

static
FTM_RET	FTOM_TP_CLIENT_setGetURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_CLIENT_setPutURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_CLIENT_setPostURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_CLIENT_setDeleteURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
);

static
FTM_RET	FTOM_TP_CLIENT_putBody
(
	FTOM_TP_CLIENT_PTR	pClient,
	cJSON _PTR_			pBody
);

static
FTM_RET	FTOM_TP_CLIENT_postBody
(
	FTOM_TP_CLIENT_PTR	pClient,
	cJSON _PTR_			pBody
);

static
FTM_RET	FTOM_TP_CLIENT_perform
(
	FTOM_TP_CLIENT_PTR	pClient
);


typedef struct FTOM_TP_CLIENT_SENSOR_TYPE_INFO_STRUCT
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pName;
	FTM_CHAR_PTR	pDriver;
	FTM_CHAR_PTR	pModel;
	FTM_CHAR_PTR	pCategory;
} FTOM_TP_CLIENT_SENSOR_TYPE_INFO, _PTR_ FTOM_TP_CLIENT_SENSOR_TYPE_INFO_PTR; 

static
FTOM_TP_CLIENT_SENSOR_TYPE_INFO pSensorTypeInfo[] = 
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
FTM_BOOL	bGlobalInit = FTM_FALSE;
static 
FTM_BOOL	bVerbose = FTM_TRUE;
static 
FTM_BOOL	bDataDump = FTM_FALSE;

FTM_RET	FTOM_TP_CLIENT_init
(
	FTOM_TP_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (!bGlobalInit)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		bGlobalInit = FTM_TRUE;	
	}

	memset(pClient, 0, sizeof(FTOM_TP_CLIENT));

  	pClient->pCURL = curl_easy_init();
  	if(pClient->pCURL == NULL) 
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pClient->pBase, "https://api.thingplus.net/v1");
	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "username:00405cabcdef");
	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "apikey:tlLZy-8UeYAzNMubWvQWS19RUV4=");
	pClient->pHTTPHeader = curl_slist_append(pClient->pHTTPHeader, "Content-Type:application/json");

	curl_easy_setopt(pClient->pCURL, CURLOPT_HTTPHEADER, pClient->pHTTPHeader);
	if (bVerbose)
	{
 		curl_easy_setopt(pClient->pCURL, CURLOPT_VERBOSE, 1L);
	}
	curl_easy_setopt(pClient->pCURL, CURLOPT_WRITEFUNCTION, FTOM_TP_CLIENT_CB_response);
	curl_easy_setopt(pClient->pCURL, CURLOPT_WRITEDATA, (FTM_VOID_PTR)pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
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

/******************************************************
 * Gateway management
 ******************************************************/
FTM_RET	FTOM_TP_CLIENT_GW_getModel
(
	FTOM_TP_CLIENT_PTR 	pClient,
	FTM_ULONG			ulModel
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	xRet = FTOM_TP_CLIENT_setGetURL(pClient, "/gatewayModels/%lu", ulModel);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

	if ((bDataDump) && (xRet == FTM_RET_OK))
	{
		printf("%s", pClient->pResp);
	}
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_GW_setID
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pGatewayID != NULL);

	strncpy(pClient->pGatewayID, pGatewayID, FTM_DID_LEN);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_CLIENT_GW_getInfo
(
	FTOM_TP_CLIENT_PTR 	pClient,
	FTOM_TP_GATEWAY_PTR	pGateway
)
{
	ASSERT(pClient != NULL);
	ASSERT(pGateway!= NULL);

	FTM_RET	xRet;
	const nx_json _PTR_ pRoot = NULL;
	const nx_json _PTR_ pSensors = NULL;
	const nx_json _PTR_ pItem = NULL;

	xRet = FTOM_TP_CLIENT_setGetURL(pClient, "/gateways/%s", pClient->pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
		goto finish;
	}

	pRoot = nx_json_parse(pClient->pResp, nx_json_unicode_to_utf8);
	if (pRoot->type == NX_JSON_NULL)
	{
		xRet = FTM_RET_COMM_INVALID_VALUE;
		goto finish;	
	}

	pItem = nx_json_get(pRoot, "id") ;
	if (pItem->type == NX_JSON_NULL)
	{
		xRet = FTM_RET_COMM_INVALID_VALUE;	
		goto finish;
	}
	strncpy(pGateway->pID, pItem->text_value, FTM_DID_LEN);

	pItem = nx_json_get(pRoot, "name") ;
	if (pItem->type != NX_JSON_NULL)
	{
		strncpy(pGateway->pName, pItem->text_value, FTM_NAME_LEN);
	}

	pItem = nx_json_get(pRoot, "reportInterval") ;
	if (pItem->type != NX_JSON_NULL)
	{
		pGateway->ulReportInterval = pItem->int_value;
	}

	pItem = nx_json_get(pRoot, "ctime") ;
	if (pItem->type != NX_JSON_NULL)
	{
		pGateway->ulCTime = pItem->int_value;
	}

	pItem = nx_json_get(pRoot, "mtime") ;
	if (pItem->type != NX_JSON_NULL)
	{
		pGateway->ulMTime = pItem->int_value;
	}

	pSensors = nx_json_get(pRoot, "sensors");
	if (pSensors->type != NX_JSON_NULL)
	{
		FTM_INT	i;

		if (pSensors->type != NX_JSON_ARRAY)
		{
			xRet = FTM_RET_COMM_INVALID_VALUE;	
			goto finish;	
		}

		for(i = 0; ; i++)
		{
			pItem = nx_json_item(pRoot, i);
			if (pItem->type != NX_JSON_NULL)
			{
			
			}


		}
	}

finish:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);	
	}
	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_GW_setStatus
(
	FTOM_TP_CLIENT_PTR 	pClient,
	FTM_BOOL			bStatus,	
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;
	cJSON _PTR_ 	pRoot = NULL;

	xRet = FTOM_TP_CLIENT_setPutURL(pClient, "/gateways/%s/status", pClient->pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	pRoot = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pRoot, "value", (bStatus)?"on":"off");
	cJSON_AddNumberToObject(pRoot, "timeout", ulTimeout);

	xRet = FTOM_TP_CLIENT_putBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
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
FTM_RET	FTOM_TP_CLIENT_DEVICE_create
(
	FTOM_TP_CLIENT_PTR	pClient,
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

	xRet = FTOM_TP_CLIENT_setPostURL(pClient, "/devices");
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_postBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

/*********************************************************
 * Sensor management 
 *********************************************************/
FTM_RET	FTOM_TP_CLIENT_SENSOR_create
(
	FTOM_TP_CLIENT_PTR	pClient,
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

	FTOM_TP_CLIENT_SENSOR_TYPE_INFO_PTR pSensorInfo = NULL;

	if ((pSensorID == NULL) || (pName == NULL) || (pType == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	for(i = 0 ; i < sizeof(pSensorTypeInfo) / sizeof(FTOM_TP_CLIENT_SENSOR_TYPE_INFO); i++)
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
		cJSON_AddStringToObject(pRoot, "deviceId", pClient->pGatewayID);
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


	xRet = FTOM_TP_CLIENT_setPostURL(pClient, "/gateways/%s/sensors", pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}


	xRet = FTOM_TP_CLIENT_postBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_SENSOR_delete
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_CLIENT_setDeleteURL(pClient, "/gateways/%s/sensors/%s", pClient->pGatewayID, pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_SENSOR_getStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL_PTR		pbStatus
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_CLIENT_setGetURL(pClient, 
				"/gateways/%s/sensors/%s/status", 
				pClient->pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_SENSOR_setStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;
	cJSON _PTR_ 	pRoot = NULL;

	xRet = FTOM_TP_CLIENT_setPutURL(pClient, 
				"/gateways/%s/sensors/%s/status", 
				pClient->pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	pRoot = cJSON_CreateObject();
	
	cJSON_AddStringToObject(pRoot, "value", (bStatus)?"on":"off");
	cJSON_AddNumberToObject(pRoot, "timeout", ulTimeout);

	xRet = FTOM_TP_CLIENT_putBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client post data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_SENSOR_getValue
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValue
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSensorID != NULL);

	FTM_RET			xRet;

	xRet = FTOM_TP_CLIENT_setGetURL(pClient, 
				"/gateways/%s/sensors/%s/series", 
				pClient->pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_SENSOR_setValues
(
	FTOM_TP_CLIENT_PTR	pClient,
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

	xRet = FTOM_TP_CLIENT_setPutURL(pClient, 
				"/gateways/%s/sensors/%s/series", 
				pClient->pGatewayID, 
				pSensorID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
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

	xRet = FTOM_TP_CLIENT_putBody(pClient, pRoot);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client put data setting.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_SENSOR_getList
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTOM_TP_SENSOR_PTR	pSensors,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pClient != NULL);

	FTM_RET			xRet;
	const nx_json _PTR_ pRoot;
	FTM_ULONG		i, ulCount = 0;

	xRet = FTOM_TP_CLIENT_setGetURL(pClient, "/gateways/%s/sensors", pClient->pGatewayID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error has occurred in the client settings of the URL.\n");
		return	xRet;
	}

	xRet = FTOM_TP_CLIENT_perform(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("An error occurred while performing the client.\n");
	}

	pRoot = nx_json_parse(pClient->pResp, nx_json_unicode_to_utf8);
	if (pRoot->type == NX_JSON_NULL)
	{
		xRet = FTM_RET_COMM_INVALID_VALUE;	
		goto finish;
	}

	
	for( i = 0 ; ulCount < ulMaxCount ; i++)
	{
		const nx_json _PTR_ pSensorInfo;
		const nx_json _PTR_ pItem;
		FTOM_TP_SENSOR		xSensor;

		pSensorInfo = nx_json_item(pRoot, i);
		if (pSensorInfo->type == NX_JSON_NULL)
		{
			break;	
		}

		memset(&xSensor, 0, sizeof(xSensor));

		pItem = nx_json_get(pSensorInfo, "id"); // get object's property by key
		if (pItem->type == NX_JSON_NULL)
		{
			continue;	
		}
		strncpy(xSensor.pID, pItem->text_value, FTM_EPID_LEN);

		pItem = nx_json_get(pSensorInfo, "type");
		if (pItem->type == NX_JSON_NULL)
		{
			continue;	
		}

		xRet = FTM_EP_strToType((FTM_CHAR_PTR)pItem->text_value, &xSensor.xType);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		pItem = nx_json_get(pSensorInfo, "name");
		if (pItem->type != NX_JSON_NULL)
		{
			strncpy(xSensor.pName, pItem->text_value, FTM_NAME_LEN);	
		}

		pItem = nx_json_get(pSensorInfo, "owner");
		if (pItem->type != NX_JSON_NULL)
		{
			strncpy(xSensor.pOwnerID, pItem->text_value, FTM_DID_LEN);	
		}

		pItem = nx_json_get(pSensorInfo, "deviceId");
		if (pItem->type != NX_JSON_NULL)
		{
			strncpy(xSensor.pDeviceID, pItem->text_value, FTM_DID_LEN);	
		}

		pItem = nx_json_get(pSensorInfo, "address");
		if (pItem->type != NX_JSON_NULL)
		{
			xSensor.ulAddress = pItem->int_value;
		}

		pItem = nx_json_get(pSensorInfo, "sequence");
		if (pItem->type != NX_JSON_NULL)
		{
			xSensor.ulSequence = pItem->int_value;
		}

		pItem = nx_json_get(pSensorInfo, "ctime");
		if (pItem->type != NX_JSON_NULL)
		{
			xSensor.ulCTime = pItem->int_value;
		}

		pItem = nx_json_get(pSensorInfo, "mtime");
		if (pItem->type != NX_JSON_NULL)
		{
			xSensor.ulMTime = pItem->int_value;
		}

		memcpy(&pSensors[ulCount++], &xSensor, sizeof(xSensor));
	}

	*pulCount = ulCount;

finish:
	if (pRoot != NULL)
	{
		nx_json_free(pRoot);
	}

	return	xRet;
}
/*********************************************************
 * End Point management 
 *********************************************************/
FTM_RET	FTOM_TP_CLIENT_EP_create
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_EP_PTR			pEPInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPInfo != NULL);

	FTM_RET	xRet;
	FTM_INT	i;
	FTOM_TP_CLIENT_SENSOR_TYPE_INFO_PTR	pTypeInfo = NULL;

	for(i = 0 ; i < sizeof(pSensorTypeInfo) / sizeof(FTOM_TP_CLIENT_SENSOR_TYPE_INFO) ; i++)
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

	xRet = FTOM_TP_CLIENT_SENSOR_create(
				pClient, 
				pClient->pGatewayID, 
				pEPInfo->pEPID,
				pTypeInfo->pName,
				pEPInfo->pEPID,
				NULL,
				"1",
				"1");

	return	xRet;
}

FTM_RET	FTOM_TP_CLIENT_EP_getList
(
	FTOM_TP_CLIENT_PTR	pClient,
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
 FTM_RET	FTOM_TP_CLIENT_setVerbose
 (
 	FTM_BOOL	bOn
 )
 {
	bVerbose = bOn;

	return	FTM_RET_OK;
 }

 FTM_RET	FTOM_TP_CLIENT_setDataDump
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
FTM_RET	FTOM_TP_CLIENT_setGetURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->pBase);

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

FTM_RET	FTOM_TP_CLIENT_setPutURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->pBase);

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

FTM_RET	FTOM_TP_CLIENT_setPostURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->pBase);

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

FTM_RET	FTOM_TP_CLIENT_setDeleteURL
(
	FTOM_TP_CLIENT_PTR	pClient,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	ASSERT(pClient != NULL);
	va_list pArgs;

	strcpy(pClient->pURL, pClient->pBase);

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

FTM_RET	FTOM_TP_CLIENT_putBody
(
	FTOM_TP_CLIENT_PTR	pClient,
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
	curl_easy_setopt(pClient->pCURL, CURLOPT_READFUNCTION, FTOM_TP_CLIENT_CB_readString);
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

FTM_RET	FTOM_TP_CLIENT_postBody
(
	FTOM_TP_CLIENT_PTR	pClient,
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

FTM_RET	FTOM_TP_CLIENT_perform
(
	FTOM_TP_CLIENT_PTR	pClient
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

size_t FTOM_TP_CLIENT_CB_readString
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

size_t FTOM_TP_CLIENT_CB_response
(
 	FTM_VOID_PTR	pContents, 
	size_t 			nSize, 
	size_t 			nMemB, 
	FTM_VOID_PTR	pUser
)
{
	ASSERT(pContents != NULL);
	ASSERT(pUser != NULL);

	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pUser;
	FTM_INT				nRealSize = nSize * nMemB;
	FTM_CHAR_PTR		pMem = NULL;

	if (nRealSize == 0)
	{
		return	0;	
	}

	pMem = (FTM_VOID_PTR)FTM_MEM_malloc(pClient->ulRespLen + nRealSize + 1);
	if (pMem == NULL)
	{
		ERROR("Not enough memory!\n");
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

