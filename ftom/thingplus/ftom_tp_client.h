#ifndef	__FTOM_TP_CLIENT_H__
#define	__FTOM_TP_CLIENT_H__

#include "ftom.h"
#include <curl/curl.h>

typedef	struct
{
	CURL 				_PTR_ pCURL;
	struct curl_slist	_PTR_ pHTTPHeader;

	FTM_CHAR			pGatewayID[FTM_DID_LEN+1];
	FTM_CHAR			pBase[1024];
	FTM_CHAR			pURL[1024];
	FTM_CHAR_PTR		pData;
	FTM_CHAR_PTR		pResp;
	FTM_ULONG			ulRespLen;
}	FTOM_TP_CLIENT, _PTR_ FTOM_TP_CLIENT_PTR;

FTM_RET	FTOM_TP_CLIENT_init
(
	FTOM_TP_CLIENT_PTR 	pClient
);


FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR 	pClient
);

/********************************************************************
 * Gateway mangement
 ********************************************************************/
FTM_RET	FTOM_TP_CLIENT_GW_getModel
(
	FTOM_TP_CLIENT_PTR 	pClient,
	FTM_ULONG			ulModel
);

FTM_RET	FTOM_TP_CLIENT_GW_setID
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID
);

FTM_RET	FTOM_TP_CLIENT_GW_getInfo
(
	FTOM_TP_CLIENT_PTR 	pClient
);

FTM_RET	FTOM_TP_CLIENT_GW_setStatus
(
	FTOM_TP_CLIENT_PTR 	pClient,
	FTM_BOOL			bStatus,	
	FTM_ULONG			ulTimeout
);

/********************************************************************
 * Device mangement
 ********************************************************************/
FTM_RET	FTOM_TP_CLIENT_DEVICE_create
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pDeviceID,
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pModel
);

/********************************************************************
 * Sensor management
 ********************************************************************/
FTM_RET	FTOM_TP_CLIENT_SENSOR_create
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_CHAR_PTR		pSensorID,
	FTM_CHAR_PTR		pType,
	FTM_CHAR_PTR		pName,
	FTM_CHAR_PTR		pDeviceID,
	FTM_CHAR_PTR		pAddress,
	FTM_CHAR_PTR		pSqeuence
);

FTM_RET	FTOM_TP_CLIENT_SENSOR_delete
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID
);

FTM_RET	FTOM_TP_CLIENT_SENSOR_getList
(
	FTOM_TP_CLIENT_PTR	pClient
);

FTM_RET	FTOM_TP_CLIENT_SENSOR_getStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL_PTR		pbStatus
);

FTM_RET	FTOM_TP_CLIENT_SENSOR_setStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
);

FTM_RET	FTOM_TP_CLIENT_SENSOR_getValue
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValue
);

FTM_RET	FTOM_TP_CLIENT_SENSOR_setValues
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pSensorID,
	FTM_EP_DATA_PTR		pValues,
	FTM_ULONG			ulCount
);
/********************************************************************
 * End Point management
 ********************************************************************/
FTM_RET	FTOM_TP_CLIENT_EP_create
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_EP_PTR			pEPInfo
);

FTM_RET	FTOM_TP_CLIENT_setVerbose
(
	FTM_BOOL	bOn
);

FTM_RET	FTOM_TP_CLIENT_setDataDump
(
	FTM_BOOL	bOn
);
#endif
