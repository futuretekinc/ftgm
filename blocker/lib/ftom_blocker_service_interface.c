#include "ftm_sis.h"
#include "ftom_blocker_service_interface.h"

FTM_RET	FTOM_BLOCKER_SIS_CMD_getConfig
(
	struct FTM_SIS_STRUCT _PTR_ pSIS, 
	FTOM_BLOCKER_SIM_REQ_GET_CONFIG_PTR		pReq,
	FTM_ULONG 		ulReqLen, 
	FTOM_BLOCKER_SIM_RESP_GET_CONFIG_PTR	pResp,
	FTM_ULONG 		ulRespLen
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	FTOM_BLOCKER_PTR	pBlocker = (FTOM_BLOCKER_PTR)pSIS->pParent;

	pResp->xCommon.ulReqID 	= pReq->xCommon.ulReqID;
	pResp->xCommon.xType 	= pReq->xCommon.xType;
	pResp->xCommon.ulLen 	= sizeof(FTOM_BLOCKER_SIM_RESP_GET_CONFIG);
	pResp->xCommon.xRet  	= FTM_RET_OK;
	memcpy(&pResp->xConfig, &pBlocker->xConfig, sizeof(FTOM_BLOCKER_CONFIG));
	
	return	FTM_RET_OK;
}


FTM_SIS_CMD	pSISCmdSet[] = 
{
	FTM_SIS_MAKE_CMD(FTM_SIM_TYPE_BLOCKER_GET_CONFIG,FTOM_BLOCKER_SIS_CMD_getConfig)
};

FTM_ULONG	ulSISCmdSetCount = sizeof(pSISCmdSet) / sizeof(FTM_SIS_CMD);

