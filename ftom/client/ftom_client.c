#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "ftm.h"
#include "ftom_client.h"
#include "ftom_params.h"

FTM_RET	FTOM_CLIENT_start
(
	FTOM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->fStart != NULL)
	{
		return	pClient->fStart(pClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_stop
(
	FTOM_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->fStop != NULL)
	{
		return	pClient->fStop(pClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_loadConfig
(
	FTOM_CLIENT_PTR			pClient,
	FTM_VOID_PTR			pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	if (pClient->fLoadConfig != NULL)
	{
		pClient->fLoadConfig(pClient, pConfig);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_loadConfigFromFile
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	if (pClient->fLoadConfigFromFile != NULL)
	{
		return	pClient->fLoadConfigFromFile(pClient, pFileName);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}


FTM_RET	FTOM_CLIENT_setNotifyCallback
(
	FTOM_CLIENT_PTR		pClient,
	FTOM_CLIENT_NOTIFY_CALLBACK	pCB
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCB != NULL);

	if (pClient->fSetNotifyCallback != NULL)
	{
		return	pClient->fSetNotifyCallback(pClient, pCB);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET FTOM_CLIENT_NODE_create
(
	FTOM_CLIENT_PTR		pClient,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_CREATE_PARAMS		xReq;
	FTOM_RESP_NODE_CREATE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid node information!\n");
		return	xRet;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_destroy
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR			pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_DESTROY_PARAMS	xReq;
	FTOM_RESP_NODE_DESTROY_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_count
(
	FTOM_CLIENT_PTR		pClient,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_COUNT_PARAMS	xReq;
	FTOM_RESP_NODE_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_COUNT;
	xReq.ulLen	=	sizeof(xReq);
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}	

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_getAt
(
	FTOM_CLIENT_PTR		pClient,
	FTM_ULONG				ulIndex,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_GET_AT_PARAMS		xReq;
	FTOM_RESP_NODE_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_get
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_GET_PARAMS		xReq;
	FTOM_RESP_NODE_GET_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_GET;
	xReq.ulLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}
	
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_CREATE_PARAMS	xReq;
	FTOM_RESP_EP_CREATE_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));
	
	TRACE("%s[%d]\n", __func__, __LINE__);
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID
)
{
	ASSERT(pClient != NULL);

	FTM_RET					xRet;
	FTOM_REQ_EP_DESTROY_PARAMS	xReq;
	FTOM_RESP_EP_DESTROY_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_count
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_COUNT_PARAMS	xReq;
	FTOM_RESP_EP_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_COUNT;
	xReq.xType	=	xType;
	xReq.ulLen	=	sizeof(xReq);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.nCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_getList
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_EP_ID_PTR		pEPIDList,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTOM_REQ_EP_GET_LIST_PARAMS		xReq;
	FTM_ULONG						nRespSize = 0;
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp;
	FTM_ULONG					ulRespLen;

	nRespSize = sizeof(FTOM_RESP_EP_GET_LIST_PARAMS) + sizeof(FTM_EP_ID) * ulMaxCount;
	pResp = (FTOM_RESP_EP_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTOM_CMD_EP_GET_LIST;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xType		=	xType;
	xReq.ulMaxCount	=	ulMaxCount;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize,
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	FTM_RET_ERROR;	
	}

	xRet = pResp->xRet;

	if (xRet == FTM_RET_OK)
	{
		memcpy(pEPIDList, pResp->pEPIDList, sizeof(FTM_EP_ID) * pResp->ulCount);
		*pulCount = pResp->ulCount;
	}

	FTM_MEM_free(pResp);

	return	xRet;
}

FTM_RET FTOM_CLIENT_EP_get
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_GET_PARAMS		xReq;
	FTOM_RESP_EP_GET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_GET;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_getAt
(
	FTOM_CLIENT_PTR		pClient,
	FTM_ULONG			ulIndex,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_GET_AT_PARAMS	xReq;
	FTOM_RESP_EP_GET_AT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_SET_PARAMS		xReq;
	FTOM_RESP_EP_SET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_SET;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_add
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pEPData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPData != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_ADD_PARAMS		xReq;
	FTOM_RESP_EP_DATA_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_DATA_ADD;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	memcpy(&xReq.xData, pEPData, sizeof(FTM_EP_DATA));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_info
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime,
	FTM_ULONG_PTR			pCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulBeginTime != NULL);
	ASSERT(pulEndTime != NULL);
	ASSERT(pCount != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_INFO_PARAMS	xReq;
	FTOM_RESP_EP_DATA_INFO_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	xReq.xCmd		=	FTOM_CMD_EP_DATA_INFO;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulBeginTime 	= xResp.ulBeginTime;
		*pulEndTime 	= xResp.ulEndTime;
		*pCount 		= xResp.ulCount;
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTOM_CLIENT_EP_DATA_getLast
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET								xRet;
	FTOM_REQ_EP_DATA_GET_LAST_PARAMS	xReq;
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	xReq.xCmd	=	FTOM_CMD_EP_DATA_GET_LAST;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pData, &xResp.xData, sizeof(FTM_EP_DATA));
	}
	
	return	xResp.xRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);
	ASSERT(pnCount != NULL);

	FTM_RET									xRet;
	FTOM_REQ_EP_DATA_GET_LIST_PARAMS		xReq;
	FTM_ULONG								nRespSize = 0;
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp = NULL;
	FTM_ULONG								ulRespLen;
	FTM_ULONG								ulRespCount = 0;
	FTM_BOOL								bStop = FTM_FALSE;

	while(!bStop)
	{
		FTM_ULONG	ulReqCount;

		if (nMaxCount > 100)
		{
			ulReqCount = 100;	
		}
		else
		{
			ulReqCount = nMaxCount;	
		}

		nRespSize = sizeof(FTOM_RESP_EP_DATA_GET_LIST_PARAMS) + sizeof(FTM_EP_DATA) * ulReqCount;
		pResp = (FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
	
		xReq.xCmd		=	FTOM_CMD_EP_DATA_GET_LIST;
		xReq.ulLen		=	sizeof(xReq);
		xReq.xEPID		=	xEPID;
		xReq.nStartIndex=	nStartIndex;
		xReq.nCount		=	ulReqCount;
	
		xRet = pClient->fRequest(
					pClient, 
					(FTM_VOID_PTR)&xReq, 
					sizeof(xReq), 
					(FTM_VOID_PTR)pResp, 
					nRespSize,
					&ulRespLen);
		if (xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pResp);
			return	FTM_RET_ERROR;	
		}
	
		xRet = pResp->xRet;
	
		if (pResp->xRet == FTM_RET_OK)
		{
			FTM_INT	i;
	
			for( i = 0 ; i < pResp->nCount && i < ulReqCount ; i++)
			{
				memcpy(&pData[ulRespCount + i], &pResp->pData[i], sizeof(FTM_EP_DATA));
			}

			ulRespCount += pResp->nCount;
			nMaxCount -= pResp->nCount;
			nStartIndex += pResp->nCount;

			if ((pResp->nCount != ulReqCount) || (nMaxCount == 0))
			{
				bStop = FTM_TRUE;	
			}
		}
		else
		{
			bStop = FTM_TRUE;
		}
	
		FTM_MEM_free(pResp);
	}

	*pnCount = ulRespCount;

	return	xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_del
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nIndex,
	FTM_ULONG			nCount
)
{
	ASSERT(pClient != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_DEL_PARAMS		xReq;
	FTOM_RESP_EP_DATA_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	xReq.xCmd		=	FTOM_CMD_EP_DATA_DEL;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nIndex		=	nIndex;
	xReq.nCount		=	nCount;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_DATA_count
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_DATA_COUNT_PARAMS	xReq;
	FTOM_RESP_EP_DATA_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	xReq.xCmd		=	FTOM_CMD_EP_DATA_COUNT;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTOM_CLIENT_request
(
	FTOM_CLIENT_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	ASSERT(pulRespLen != NULL);

	if (pClient->fRequest != NULL)
	{
		return	pClient->fRequest(pClient, pReq, ulReqLen, pResp, ulRespLen, pulRespLen);	
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

