#include "ftom_blocker_service_interface.h"

FTM_RET	FTOM_BLOCKER_SIC_getConfig
(
	FTM_SIC_PTR	pSIC,
	FTOM_BLOCKER_CONFIG_PTR	pConfig
)
{
	ASSERT(pSIC != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTOM_BLOCKER_SIM_REQ_GET_CONFIG		xReq;
	FTOM_BLOCKER_SIM_RESP_GET_CONFIG	xResp;
	FTM_ULONG	ulRespLen;

	xReq.xCommon.ulReqID= 0;
	xReq.xCommon.xType = FTM_SIM_TYPE_BLOCKER_GET_CONFIG;
	xReq.xCommon.ulLen = sizeof(xReq);

	xRet = FTM_SIC_request(pSIC, (FTM_SIM_REQ_PTR)&xReq, sizeof(xReq), (FTM_SIM_RESP_PTR)&xResp, sizeof(xResp), &ulRespLen);
	if (xRet != FTM_RET_OK)
	{	
		ERROR2(xRet, "Failed to send request to blocker!\n");
		return	xRet;	
	}

	memcpy(pConfig, &xResp.xConfig, sizeof(FTOM_BLOCKER_CONFIG));

	return	xResp.xCommon.xRet;
}
