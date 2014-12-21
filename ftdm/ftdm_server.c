#include <string.h>
#include "ftdm.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "debug.h"

typedef	FTDM_RET	(*FTDM_SERVICE_CALLBACK)(FTDM_REQ_PARAMS_PTR, FTDM_RESP_PARAMS_PTR);
typedef struct
{
	FTDM_CMD				xCmd;
	FTDM_SERVICE_CALLBACK	fService;
}	FTDMS_CMD_SET, _PTR_ FTDMS_CMD_SET_PTR;

static FTDMS_CMD_SET	pCmdSet[] =
{
	{	FTDM_CMD_CREATE_DEVICE,		(FTDM_SERVICE_CALLBACK)FTDMS_createDevice },
	{	FTDM_CMD_DESTROY_DEVICE,	(FTDM_SERVICE_CALLBACK)FTDMS_destroyDevice },
	{	FTDM_CMD_GET_DEVICE_INFO,	(FTDM_SERVICE_CALLBACK)FTDMS_getDeviceInfo },
	{	FTDM_CMD_CREATE_EP,			(FTDM_SERVICE_CALLBACK)FTDMS_createEP },
	{	FTDM_CMD_DESTROY_EP,		(FTDM_SERVICE_CALLBACK)FTDMS_destroyEP },
	{	FTDM_CMD_GET_EP_INFO,		(FTDM_SERVICE_CALLBACK)FTDMS_getEPInfo },
	{	FTDM_CMD_APPEND_EP_DATA,	(FTDM_SERVICE_CALLBACK)FTDMS_appendEPData },
	{	FTDM_CMD_GET_EP_DATA,		(FTDM_SERVICE_CALLBACK)FTDMS_getEPData},
	{	FTDM_CMD_REMOVE_EP_DATA,	(FTDM_SERVICE_CALLBACK)FTDMS_removeEPData},
	{	
		.xCmd	=	FTDM_CMD_UNKNOWN, 
		.fService = 0
	}
};

FTDM_RET	FTDMS_service
(
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
)
{
	FTDMS_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTDM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
			return	pSet->fService(pReq, pResp);
		}

		pSet++;
	}

	ERROR("FUNCTION NOT SUPPORTED\n");
	ERROR("CMD : %08x\n", (FTDM_INT)pReq->xCmd);
	return	FTDM_RET_FUNCTION_NOT_SUPPORTED;
}

FTDM_RET	FTDMS_createDevice
(
	FTDM_REQ_CREATE_DEVICE_PARAMS_PTR	pReq,
	FTDM_RESP_CREATE_DEVICE_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_createDevice(
					pReq->xInfo.pDID,
					pReq->xInfo.xType, 
					pReq->xInfo.pURL,
					strlen(pReq->xInfo.pURL),
					pReq->xInfo.pLocation,
					strlen(pReq->xInfo.pLocation));

	return	pResp->nRet;
}


FTDM_RET	FTDMS_destroyDevice
(
 	FTDM_REQ_DESTROY_DEVICE_PARAMS_PTR	pReq,
	FTDM_RESP_DESTROY_DEVICE_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_destroyDevice(pReq->pDID);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_getDeviceInfo
(
 	FTDM_REQ_GET_DEVICE_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_GET_DEVICE_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getDeviceInfo(pReq->pDID, &pResp->xInfo);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_getDeviceType
(
 	FTDM_REQ_GET_DEVICE_TYPE_PARAMS_PTR		pReq,
	FTDM_RESP_GET_DEVICE_TYPE_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getDeviceType(pReq->pDID, &pResp->xType);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_getDeviceURL
(
	FTDM_REQ_GET_DEVICE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_DEVICE_URL_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nURLLen = FTDM_DEVICE_URL_LEN;
	pResp->nRet = FTDM_getDeviceURL(pReq->pDID, pResp->pURL, &pResp->nURLLen);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_setDeviceURL
(
 	FTDM_REQ_SET_DEVICE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_DEVICE_URL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_setDeviceURL(pReq->pDID, pReq->pURL, pReq->nURLLen);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_getDeviceLocation
(
 	FTDM_REQ_GET_DEVICE_LOCATION_PARAMS_PTR	pReq,
	FTDM_RESP_GET_DEVICE_LOCATION_PARAMS_PTR pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nLocationLen = FTDM_DEVICE_LOCATION_LEN;
	pResp->nRet = FTDM_getDeviceLocation(pReq->pDID, pResp->pLocation, &pResp->nLocationLen);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_setDeviceLocation
(
 	FTDM_REQ_SET_DEVICE_LOCATION_PARAMS_PTR		pReq,
 	FTDM_RESP_SET_DEVICE_LOCATION_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_setDeviceLocation(pReq->pDID, pReq->pLocation, pReq->nLocationLen);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_createEP
(
 	FTDM_REQ_CREATE_EP_PARAMS_PTR	pReq,
	FTDM_RESP_CREATE_EP_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_createEP(pReq->xInfo.xEPID, &pReq->xInfo);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_destroyEP
(
 	FTDM_REQ_DESTROY_EP_PARAMS_PTR	pReq,
	FTDM_RESP_DESTROY_EP_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_destroyEP(pReq->xEPID);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_getEPInfo
(
 	FTDM_REQ_GET_EP_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfo(pReq->xEPID, &pResp->xInfo);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_appendEPData
(
 	FTDM_REQ_APPEND_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_APPEND_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_appendEPData(pReq->xEPID, pReq->nTime, pReq->nValue);

	return	pResp->nRet;
}

FTDM_RET	FTDMS_getEPData
(
 	FTDM_REQ_GET_EP_DATA_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_getEPData(&pReq->xEPID, 1, pReq->nBeginTime, pReq->nEndTime,
			 pResp->pData, &pResp->nCount);

	return	pResp->nRet;
}

FTDM_RET 	FTDMS_removeEPData
(
 	FTDM_REQ_REMOVE_EP_DATA_PARAMS_PTR		pReq,
	FTDM_RESP_REMOVE_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_removeEPData(&pReq->xEPID, 1, pReq->nBeginTime, pReq->nEndTime, 0);

	return	pResp->nRet;
}
