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
	{	FTDM_CMD_ADD_NODE_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_addNodeInfo },
	{	FTDM_CMD_DEL_NODE_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_delNodeInfo },
	{	FTDM_CMD_GET_NODE_INFO_COUNT,		(FTDM_SERVICE_CALLBACK)FTDMS_getNodeInfoCount },
	{	FTDM_CMD_GET_NODE_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_getNodeInfo },
	{	FTDM_CMD_GET_NODE_INFO_BY_INDEX,	(FTDM_SERVICE_CALLBACK)FTDMS_getNodeInfoByIndex },
	{	FTDM_CMD_ADD_EP_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_addEPInfo },
	{	FTDM_CMD_DEL_EP_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_delEPInfo },
	{	FTDM_CMD_GET_EP_INFO_COUNT,			(FTDM_SERVICE_CALLBACK)FTDMS_getEPInfoCount },
	{	FTDM_CMD_GET_EP_INFO,				(FTDM_SERVICE_CALLBACK)FTDMS_getEPInfo },
	{	FTDM_CMD_GET_EP_INFO_BY_INDEX,		(FTDM_SERVICE_CALLBACK)FTDMS_getEPInfoByIndex },
	{	FTDM_CMD_ADD_EP_CLASS_INFO,			(FTDM_SERVICE_CALLBACK)FTDMS_addEPClassInfo },
	{	FTDM_CMD_DEL_EP_CLASS_INFO,			(FTDM_SERVICE_CALLBACK)FTDMS_delEPClassInfo },
	{	FTDM_CMD_GET_EP_CLASS_INFO,			(FTDM_SERVICE_CALLBACK)FTDMS_getEPClassInfo },
	{	FTDM_CMD_ADD_EP_DATA,				(FTDM_SERVICE_CALLBACK)FTDMS_addEPData },
	{	FTDM_CMD_DEL_EP_DATA,				(FTDM_SERVICE_CALLBACK)FTDMS_delEPData},
	{	FTDM_CMD_DEL_EP_DATA_WITH_TIME,		(FTDM_SERVICE_CALLBACK)FTDMS_delEPDataWithTime},
	{	FTDM_CMD_GET_EP_DATA,				(FTDM_SERVICE_CALLBACK)FTDMS_getEPData},
	{	FTDM_CMD_GET_EP_DATA_COUNT,			(FTDM_SERVICE_CALLBACK)FTDMS_getEPDataCount},
	{	FTDM_CMD_GET_EP_DATA_COUNT_WITH_TIME,(FTDM_SERVICE_CALLBACK)FTDMS_getEPDataCountWithTime},
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

FTM_RET	FTDMS_addNodeInfo
(
	FTDM_REQ_ADD_NODE_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_NODE_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addNodeInfo(&pReq->xNodeInfo);

	return	pResp->nRet;
}


FTM_RET	FTDMS_delNodeInfo
(
 	FTDM_REQ_DEL_NODE_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_NODE_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delNodeInfo(pReq->pDID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeInfoCount
(
 	FTDM_REQ_GET_NODE_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfoCount(&pResp->nCount);

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

FTM_RET	FTDMS_addEPInfo
(
 	FTDM_REQ_ADD_EP_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_EP_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPInfo(&pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_delEPInfo
(
 	FTDM_REQ_DEL_EP_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPInfo(pReq->xEPID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPInfoCount
(
 	FTDM_REQ_GET_EP_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfoCount(&pResp->nCount);

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

FTM_RET	FTDMS_addEPClassInfo
(
 	FTDM_REQ_ADD_EP_CLASS_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_EP_CLASS_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPClassInfo(pReq->xClass, &pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_delEPClassInfo
(
 	FTDM_REQ_DEL_EP_CLASS_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_CLASS_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPClassInfo(pReq->xClass);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPClassInfo
(
 	FTDM_REQ_GET_EP_CLASS_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_CLASS_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPClassInfo(pReq->xClass, &pResp->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_addEPData
(
 	FTDM_REQ_ADD_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPData(pReq->xEPID, &pReq->xData);

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
	TRACE("%16s : %08lx\n", "EPID", pReq->xEPID);
	TRACE("%16s : %08lx\n", "Start Index", pReq->nStartIndex);

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_getEPData(
					pReq->xEPID, 
					pReq->nStartIndex,
					pResp->pData, 
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

FTM_RET	FTDMS_getEPDataWithTime
(
 	FTDM_REQ_GET_EP_DATA_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS_PTR	pResp
)
{
	TRACE("Request Frame\n");
	TRACE("%16s : %08lx\n", "Command", pReq->xCmd);
	TRACE("%16s : %08lx\n", "Length", pReq->nLen);
	TRACE("%16s : %08lx\n", "EPID", pReq->xEPID);
	TRACE("%16s : %08lx\n", "Begin Time", pReq->nBeginTime);
	TRACE("%16s : %08lx\n", "End Time", pReq->nEndTime);

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_getEPDataWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime, 
					pResp->pData, 
					pReq->nCount, 
					&pResp->nCount);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS);
	}
	return	pResp->nRet;
}

FTM_RET 	FTDMS_delEPData
(
 	FTDM_REQ_DEL_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPData(
					pReq->xEPID, 
					pReq->nIndex, 
					pReq->nCount);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_delEPDataWithTime
(
 	FTDM_REQ_DEL_EP_DATA_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_DATA_WITH_TIME_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPDataWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_getEPDataCount
(
 	FTDM_REQ_GET_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_DATA_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPDataCount(
					pReq->xEPID, 
					&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_getEPDataCountWithTime
(
 	FTDM_REQ_GET_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPDataCountWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime,
					&pResp->nCount);

	return	pResp->nRet;
}

