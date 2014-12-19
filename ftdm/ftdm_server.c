#include "ftdm.h"
#include "ftdm_params.h"
#include "ftdm_server.h"

typedef	FTDM_RET	(*FTDM_SERVICE_CALLBACK)(FTDM_REQ_PARAMS_PTR, FTDM_RESP_PARAMS_PTR);
typedef struct
{
	FTDM_CMD				xCmd;
	FTDM_SERVICE_CALLBACK	fService;
}	FTDMS_CMD_SET, _PTR_ FTDMS_CMD_SET_PTR;

static FTDMS_CMD_SET	pCmdSet[] =
{
	{	.xCmd	=	FTDM_CMD_CREATE_DEVICE,	.fService = (FTDM_SERVICE_CALLBACK)FTDMS_createDevice },
	{	.xCmd	=	FTDM_CMD_UNKNOWN, .fService = 0}
};

FTDM_RET	FTDMS_service
(
	FTDM_CMD				xCmd,
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
)
{
	FTDMS_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTDM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == xCmd)
		{
			return	pSet->fService(pReq, pResp);
		}

		pSet++;
	}

	return	FTDM_RET_OK;
}

FTDM_RET	FTDMS_createDevice
(
	FTDM_REQ_CREATE_DEVICE_PARAMS_PTR	pReq,
	FTDM_RESP_CREATE_DEVICE_PARAMS_PTR	pResp
)
{
	pResp->nRet = FTDM_createDevice(
					pReq->xInfo.pDID,
					pReq->xInfo.xType, 
					pReq->xInfo.pURL,
					strlen(pReq->xInfo.pURL),
					pReq->xInfo.pLocation,
					strlen(pReq->xInfo.pLocation));

	return	pResp->nRet;
}
