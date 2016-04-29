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
	FTM_NODE_PTR		pInfo,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG			ulDIDLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID	!= NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_CREATE_PARAMS		xReq;
	FTOM_RESP_NODE_CREATE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

#if 0
	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid node information!\n");
		return	xRet;	
	}
#endif
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
	else
	{
		if (strlen(xResp.pDID) + 1 > ulDIDLen)
		{
			return	FTM_RET_BUFFER_TOO_SMALL;	
		}

		strcpy(pDID, xResp.pDID);
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

FTM_RET FTOM_CLIENT_NODE_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_FIELD		xFields,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_SET_PARAMS	xReq;
	FTOM_RESP_NODE_SET_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_SET;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_NODE));

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

FTM_RET FTOM_CLIENT_EP_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo,
	FTM_EP_PTR			pNewInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(pNewInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_CREATE_PARAMS	xReq;
	FTOM_RESP_EP_CREATE_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_CREATE;
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
		return	xRet;	
	}

	memcpy(pNewInfo, &xResp.xInfo, sizeof(FTM_EP));

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pClient != NULL);

	FTM_RET					xRet;
	FTOM_REQ_EP_DESTROY_PARAMS	xReq;
	FTOM_RESP_EP_DESTROY_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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

	memset(&xReq, 0, sizeof(xReq));

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
	FTM_CHAR			pEPIDList[][FTM_EPID_LEN+1],
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

	memset(&xReq, 0, sizeof(xReq));

	nRespSize = sizeof(FTOM_RESP_EP_GET_LIST_PARAMS) + (FTM_EPID_LEN+1) * ulMaxCount;
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
		memcpy(pEPIDList, pResp->pEPIDList, (FTM_EPID_LEN+1) * pResp->ulCount);
		*pulCount = pResp->ulCount;
	}

	FTM_MEM_free(pResp);

	return	xRet;
}

FTM_RET FTOM_CLIENT_EP_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_GET_PARAMS		xReq;
	FTOM_RESP_EP_GET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_GET;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);

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

	memset(&xReq, 0, sizeof(xReq));

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
	FTM_CHAR_PTR		pEPID,
	FTM_EP_FIELD		xFields,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_SET_PARAMS		xReq;
	FTOM_RESP_EP_SET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_SET;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pEPID, pEPID, FTM_ID_LEN);
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
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pEPData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPData != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_ADD_PARAMS		xReq;
	FTOM_RESP_EP_DATA_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_DATA_ADD;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);
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
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pCount
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

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_INFO;
	xReq.ulLen		=	sizeof(xReq);
	strncpy( xReq.pEPID, pEPID, FTM_EPID_LEN);

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
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET								xRet;
	FTOM_REQ_EP_DATA_GET_LAST_PARAMS	xReq;
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_DATA_GET_LAST;
	xReq.ulLen	=	sizeof(xReq);
	strncpy( xReq.pEPID, pEPID, FTM_EPID_LEN);

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
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pnCount
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

		if (ulMaxCount > 100)
		{
			ulReqCount = 100;	
		}
		else
		{
			ulReqCount = ulMaxCount;	
		}

		nRespSize = sizeof(FTOM_RESP_EP_DATA_GET_LIST_PARAMS) + sizeof(FTM_EP_DATA) * ulReqCount;
		pResp = (FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
	
		memset(&xReq, 0, sizeof(xReq));

		xReq.xCmd		=	FTOM_CMD_EP_DATA_GET_LIST;
		xReq.ulLen		=	sizeof(xReq);
		strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);
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
			nStartIndex += pResp->nCount;
			ulMaxCount -= pResp->nCount;

			if ((pResp->nCount != ulReqCount) || (ulMaxCount == 0))
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
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nIndex,
	FTM_ULONG		nCount
)
{
	ASSERT(pClient != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_DEL_PARAMS		xReq;
	FTOM_RESP_EP_DATA_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;
	
	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_DEL;
	xReq.nLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_DATA_COUNT_PARAMS	xReq;
	FTOM_RESP_EP_DATA_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_COUNT;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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

FTM_RET	FTOM_CLIENT_EP_DATA_type
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_TYPE_PTR	pType
)
{
	ASSERT(pClient != NULL);
	ASSERT(pType != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_DATA_TYPE_PARAMS	xReq;
	FTOM_RESP_EP_DATA_TYPE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_TYPE;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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
		*pType = xResp.xType;
	}

	return	xResp.xRet;
}


/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_TRIGGER_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_ADD_PARAMS	xReq;
	FTOM_RESP_TRIGGER_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_ADD;
	xReq.ulLen		=	sizeof(xReq);
	memcpy(&xReq.xTrigger, pTrigger, sizeof(FTM_TRIGGER));

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
		strncpy(pTrigger->pID, xResp.pTriggerID, FTM_ID_LEN);
	}
	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pTriggerID
)
{
	ASSERT(pClient != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_DEL_PARAMS	xReq;
	FTOM_RESP_TRIGGER_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pTriggerID, pTriggerID, FTM_ID_LEN);

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

FTM_RET	FTOM_CLIENT_TRIGGER_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_COUNT_PARAMS	xReq;
	FTOM_RESP_TRIGGER_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_COUNT;
	xReq.ulLen		=	sizeof(xReq);

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
		*pulCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_GET_PARAMS	xReq;
	FTOM_RESP_TRIGGER_GET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_GET;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pTriggerID, pID, FTM_ID_LEN);

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
		memcpy(pTrigger, &xResp.xTrigger, sizeof(FTM_TRIGGER));
	}	

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_GET_AT_PARAMS	xReq;
	FTOM_RESP_TRIGGER_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_GET_AT;
	xReq.ulLen		=	sizeof(xReq);
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
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pTrigger, &xResp.xTrigger, sizeof(FTM_TRIGGER));
	}	

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_TRIGGER_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pTriggerID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_SET_PARAMS	xReq;
	FTOM_RESP_TRIGGER_SET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_SET;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xFields	=	xFields;
	strcpy(xReq.pTriggerID, pTriggerID);
	memcpy(&xReq.xTrigger, pTrigger, sizeof(FTM_TRIGGER));

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

FTM_RET	FTOM_CLIENT_ACTION_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_ADD_PARAMS	xReq;
	FTOM_RESP_ACTION_ADD_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_ADD;
	xReq.ulLen		=	sizeof(xReq);
	memcpy(&xReq.xAction, pAction, sizeof(FTM_ACTION));

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
		strcpy(pAction->pID, xResp.pActionID);
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pClient != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_DEL_PARAMS	xReq;
	FTOM_RESP_ACTION_DEL_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);

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

FTM_RET	FTOM_CLIENT_ACTION_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_COUNT_PARAMS	xReq;
	FTOM_RESP_ACTION_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_COUNT;
	xReq.ulLen		=	sizeof(xReq);

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
		*pulCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_GET_PARAMS	xReq;
	FTOM_RESP_ACTION_GET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_GET;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);

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
		memcpy(pAction, &xResp.xAction, sizeof(FTM_ACTION));
	}	

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_GET_AT_PARAMS	xReq;
	FTOM_RESP_ACTION_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_GET_AT;
	xReq.ulLen		=	sizeof(xReq);
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
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pAction, &xResp.xAction, sizeof(FTM_ACTION));
	}	

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_ACTION_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_SET_PARAMS	xReq;
	FTOM_RESP_ACTION_SET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_SET;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xFields	=	xFields;
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);
	memcpy(&xReq.xAction, pAction, sizeof(FTM_ACTION));

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

FTM_RET	FTOM_CLIENT_RULE_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_ADD_PARAMS	xReq;
	FTOM_RESP_RULE_ADD_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_ADD;
	xReq.ulLen		=	sizeof(xReq);
	memcpy(&xReq.xRule, pRule, sizeof(FTM_RULE));

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
		strcpy(pRule->pID, xResp.pRuleID);
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pClient != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_DEL_PARAMS	xReq;
	FTOM_RESP_RULE_DEL_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);

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

FTM_RET	FTOM_CLIENT_RULE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_COUNT_PARAMS	xReq;
	FTOM_RESP_RULE_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_COUNT;
	xReq.ulLen		=	sizeof(xReq);

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
		*pulCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_GET_PARAMS	xReq;
	FTOM_RESP_RULE_GET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_GET;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);

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
		memcpy(pRule, &xResp.xRule, sizeof(FTM_RULE));
	}	

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_GET_AT_PARAMS	xReq;
	FTOM_RESP_RULE_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_GET_AT;
	xReq.ulLen		=	sizeof(xReq);
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
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pRule, &xResp.xRule, sizeof(FTM_RULE));
	}	

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_RULE_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_SET_PARAMS	xReq;
	FTOM_RESP_RULE_SET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_SET;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xFields	=	xFields;
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);
	memcpy(&xReq.xRule, pRule, sizeof(FTM_RULE));

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

