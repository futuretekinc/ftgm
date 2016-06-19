#ifndef	__FTOM_TP_RESTAPI_H__
#define	__FTOM_TP_RESTAPI_H__

#include "ftom.h"
#include <curl/curl.h>
#include "ftom_tp_restapi_gateway.h"

typedef	struct
{
	FTM_CHAR			pGatewayID[FTM_DID_LEN+1];
	FTM_CHAR			pUserID[FTM_USER_ID_LEN+1];
	FTM_CHAR			pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR			pBaseURL[FTM_URL_LEN+1];
	FTM_BOOL			bSecure;
}	FTOM_TP_RESTAPI_CONFIG, _PTR_ FTOM_TP_RESTAPI_CONFIG_PTR;

typedef	struct
{
	FTOM_TP_RESTAPI_CONFIG	xConfig;

	FTM_CHAR			pURL[1024];
	FTM_CHAR_PTR		pData;
	FTM_CHAR_PTR		pResp;
	FTM_ULONG			ulRespLen;

	struct curl_slist	_PTR_ pHTTPHeader;
	CURL 				_PTR_ pCURL;
}	FTOM_TP_RESTAPI, _PTR_ FTOM_TP_RESTAPI_PTR;

typedef	struct
{
	FTM_CHAR		pID[FTM_EPID_LEN+1];
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_CHAR		pOwnerID[FTM_DID_LEN+1];
	FTM_CHAR		pDeviceID[FTM_DID_LEN+1];
	FTM_EP_TYPE		xType;
	FTM_ULONG		ulAddress;
	FTM_ULONG		ulSequence;
	FTM_UINT64		ullCTime;
	FTM_UINT64		ullMTime;
}	FTOM_TP_RESTAPI_SENSOR, _PTR_ FTOM_TP_RESTAPI_SENSOR_PTR;

FTM_RET	FTOM_TP_RESTAPI_create
(
	FTOM_TP_RESTAPI_PTR _PTR_ ppClient,
	FTM_CHAR_PTR		pUserID,
	FTM_CHAR_PTR		pAPIKey
);

FTM_RET	FTOM_TP_RESTAPI_destroy
(
	FTOM_TP_RESTAPI_PTR _PTR_ ppClient
);

FTM_RET	FTOM_TP_RESTAPI_init
(
	FTOM_TP_RESTAPI_PTR pClient
);


FTM_RET	FTOM_TP_RESTAPI_final
(
	FTOM_TP_RESTAPI_PTR pClient
);

FTM_RET	FTOM_TP_RESTAPI_setConfig
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTOM_TP_RESTAPI_CONFIG_PTR	pConfig
);

FTM_RET	FTOM_TP_RESTAPI_setUserID
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pUserID
);

FTM_RET	FTOM_TP_RESTAPI_setPasswd
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pPasswd
);

FTM_RET	FTOM_TP_RESTAPI_setGatewayID
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID
);

FTM_RET	FTOM_TP_RESTAPI_setBaseURL
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pBaseURL
);

/********************************************************************
 * Gateway mangement
 ********************************************************************/
FTM_RET	FTOM_TP_RESTAPI_GW_getModel
(
	FTOM_TP_RESTAPI_PTR pClient,
	FTM_ULONG			ulModel
);

FTM_RET	FTOM_TP_RESTAPI_GW_getInfo
(
	FTOM_TP_RESTAPI_PTR pClient,
	FTOM_TP_GATEWAY_PTR	pGateway
);

FTM_RET	FTOM_TP_RESTAPI_GW_setStatus
(
	FTOM_TP_RESTAPI_PTR pClient,
	FTM_BOOL			bStatus,	
	FTM_ULONG			ulTimeout
);

/********************************************************************
 * Device mangement
 ********************************************************************/
FTM_RET	FTOM_TP_RESTAPI_DEVICE_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pDeviceID,
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pModel
);

FTM_RET	FTOM_TP_RESTAPI_DEVICE_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pDeviceID
);
/********************************************************************
 * Sensor management
 ********************************************************************/
FTM_RET	FTOM_TP_RESTAPI_SENSOR_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_CHAR_PTR		pSensorID,
	FTM_CHAR_PTR		pType,
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pDeviceID,
	FTM_CHAR_PTR		pAddress,
	FTM_CHAR_PTR		pSqeuence
);

FTM_RET	FTOM_TP_RESTAPI_SENSOR_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID
);

FTM_RET	FTOM_TP_RESTAPI_SENSOR_getList
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTOM_TP_RESTAPI_SENSOR_PTR	pSensors,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_TP_RESTAPI_SENSOR_getStatus
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL_PTR		pbStatus
);

FTM_RET	FTOM_TP_RESTAPI_SENSOR_setStatus
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
);

FTM_RET	FTOM_TP_RESTAPI_SENSOR_getValue
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValue
);

FTM_RET	FTOM_TP_RESTAPI_SENSOR_setValues
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValues,
	FTM_ULONG			ulCount
);


/********************************************************************
 * Node management
 ********************************************************************/
FTM_RET	FTOM_TP_RESTAPI_NODE_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_NODE_PTR			pNodeInfo
);

FTM_RET	FTOM_TP_RESTAPI_NODE_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pDID
);

/********************************************************************
 * End Point management
 ********************************************************************/
FTM_RET	FTOM_TP_RESTAPI_EP_create
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_EP_PTR			pEPInfo
);

FTM_RET	FTOM_TP_RESTAPI_EP_delete
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_CHAR_PTR		pEPID
);

FTM_RET	FTOM_TP_RESTAPI_EP_getList
(
	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_EP_PTR			pEPInfos,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_TP_RESTAPI_setVerbose
(
 	FTOM_TP_RESTAPI_PTR	pClient,
	FTM_BOOL	bOn
);

FTM_RET	FTOM_TP_RESTAPI_setDataDump
(
	FTM_BOOL	bOn
);
#endif
