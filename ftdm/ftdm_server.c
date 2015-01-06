#include <string.h>
#include "ftdm.h"
#include "ftdm_params.h"
#include "ftdm_server.h"

typedef	FTM_RET	(*FTDM_SERVICE_CALLBACK)(FTDM_REQ_PARAMS_PTR, FTDM_RESP_PARAMS_PTR);
typedef struct
{
	FTDM_CMD				xCmd;
	FTDM_SERVICE_CALLBACK	fService;
}	FTDMS_CMD_SET, _PTR_ FTDMS_CMD_SET_PTR;

static FTDMS_CMD_SET	pCmdSet[] =
{
	{	FTDM_CMD_CREATE_NODE,				(FTDM_SERVICE_CALLBACK)FTDMS_createNode },
	{	FTDM_CMD_DESTROY_NODE,				(FTDM_SERVICE_CALLBACK)FTDMS_destroyNode },
	{	FTDM_CMD_GET_NODE_COUNT,			(FTDM_SERVICE_CALLBACK)FTDMS_getNodeCount },
	{	FTDM_CMD_GET_NODE_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_getNodeInfo },
	{	FTDM_CMD_GET_NODE_INFO_BY_INDEX,	(FTDM_SERVICE_CALLBACK)FTDMS_getNodeInfoByIndex },
	{	FTDM_CMD_CREATE_EP,					(FTDM_SERVICE_CALLBACK)FTDMS_createEP },
	{	FTDM_CMD_DESTROY_EP,				(FTDM_SERVICE_CALLBACK)FTDMS_destroyEP },
	{	FTDM_CMD_GET_EP_COUNT,				(FTDM_SERVICE_CALLBACK)FTDMS_getEPCount },
	{	FTDM_CMD_GET_EP_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_getEPInfo },
	{	FTDM_CMD_GET_EP_INFO_BY_INDEX,		(FTDM_SERVICE_CALLBACK)FTDMS_getEPInfoByIndex },
	{	FTDM_CMD_APPEND_EP_DATA,			(FTDM_SERVICE_CALLBACK)FTDMS_appendEPData },
	{	FTDM_CMD_GET_EP_DATA,				(FTDM_SERVICE_CALLBACK)FTDMS_getEPData},
	{	FTDM_CMD_REMOVE_EP_DATA,			(FTDM_SERVICE_CALLBACK)FTDMS_removeEPData},
	{	
		.xCmd	=	FTDM_CMD_UNKNOWN, 
		.fService = 0
	}
};

FTM_RET	FTDMS_service
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
	ERROR("CMD : %08lx\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTDMS_createNode
(
	FTDM_REQ_CREATE_NODE_PARAMS_PTR	pReq,
	FTDM_RESP_CREATE_NODE_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_createNode(&pReq->xNodeInfo);

	return	pResp->nRet;
}


FTM_RET	FTDMS_destroyNode
(
 	FTDM_REQ_DESTROY_NODE_PARAMS_PTR	pReq,
	FTDM_RESP_DESTROY_NODE_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_destroyNode(pReq->pDID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeCount
(
 	FTDM_REQ_GET_NODE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeCount(&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeInfo
(
 	FTDM_REQ_GET_NODE_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_GET_NODE_INFO_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfo(pReq->pDID, &pNodeInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeInfoByIndex
(
 	FTDM_REQ_GET_NODE_INFO_BY_INDEX_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_INFO_BY_INDEX_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfoByIndex(pReq->nIndex, &pNodeInfo);

	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeType
(
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR		pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo = NULL;
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfo(pReq->pDID, &pNodeInfo);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->xType = pNodeInfo->xType;
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeURL
(
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo = NULL;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nURLLen = FTM_URL_LEN;
	pResp->nRet = FTDM_getNodeInfo(pReq->pDID, &pNodeInfo);

	if (pResp->nRet == FTM_RET_OK)
	{
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_setNodeURL
(
 	FTDM_REQ_SET_NODE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_NODE_URL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	//pResp->nRet = FTDM_setNodeURL(pReq->pDID, pReq->pURL);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeLocation
(
 	FTDM_REQ_GET_NODE_LOCATION_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_LOCATION_PARAMS_PTR pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nLocationLen = FTM_LOCATION_LEN;
	//pResp->nRet = FTDM_getNodeLocation(
	//				pReq->pDID, 
	//				pResp->pLocation, 
	//				pResp->nLocationLen);

	return	pResp->nRet;
}

FTM_RET	FTDMS_setNodeLocation
(
 	FTDM_REQ_SET_NODE_LOCATION_PARAMS_PTR		pReq,
 	FTDM_RESP_SET_NODE_LOCATION_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	//pResp->nRet = FTDM_setNodeLocation(
	//				pReq->pDID, 
	//				pReq->pLocation);

	return	pResp->nRet;
}

FTM_RET	FTDMS_createEP
(
 	FTDM_REQ_CREATE_EP_PARAMS_PTR	pReq,
	FTDM_RESP_CREATE_EP_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_createEP(&pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_destroyEP
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

FTM_RET	FTDMS_getEPCount
(
 	FTDM_REQ_GET_EP_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPCount(&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPInfo
(
 	FTDM_REQ_GET_EP_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_INFO_PARAMS_PTR	pResp
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfo(
					pReq->xEPID, 
					&pEPInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, pEPInfo, sizeof(FTM_EP_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPInfoByIndex
(
 	FTDM_REQ_GET_EP_INFO_BY_INDEX_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_INFO_BY_INDEX_PARAMS_PTR	pResp
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfoByIndex(
					pReq->nIndex, 
					&pEPInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, pEPInfo, sizeof(FTM_EP_INFO));
	}
	return	pResp->nRet;
}

FTM_RET	FTDMS_appendEPData
(
 	FTDM_REQ_APPEND_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_APPEND_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_appendEPData(&pReq->xData);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPData
(
 	FTDM_REQ_GET_EP_DATA_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_DATA_PARAMS_PTR	pResp
)
{
	TRACE("Request Frame\n");
	TRACE("%16s : %08lx\n", "Command", pReq->xCmd);
	TRACE("%16s : %08lx\n", "Length", pReq->nLen);
	TRACE("%16s : %08lx\n", "Begin Time", pReq->nBeginTime);
	TRACE("%16s : %08lx\n", "End Time", pReq->nEndTime);
	TRACE("%16s : %08lx\n", "Count", pReq->nCount);
	TRACE("%16s : %08lx\n", "EPID Count", pReq->nEPIDCount);
	if (pReq->nEPIDCount != 0)
	{
		FTM_INT	i;
		
		for(i = 0 ; i < pReq->nEPIDCount ; i++)
		{
			TRACE("%12s[%02d] : %08lx\n", "EPID", pReq->pEPID[i]);	
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_getEPData(
					pReq->pEPID, 
					pReq->nEPIDCount, 
					pReq->nBeginTime, 
					pReq->nEndTime, 
					pResp->pData, 
					pReq->nStartIndex,
					pReq->nCount, 
					&pResp->nCount);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_PARAMS);
	}
	return	pResp->nRet;
}

FTM_RET 	FTDMS_removeEPData
(
 	FTDM_REQ_REMOVE_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_REMOVE_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_removeEPData(
					pReq->pEPID, 
					pReq->nEPIDCount, 
					pReq->nBeginTime, 
					pReq->nEndTime, 
					0);

	return	pResp->nRet;
}

