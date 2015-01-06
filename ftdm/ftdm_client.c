#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm.h"
#include "ftdm_client.h"

static FTM_RET FTDMC_request
(
	FTDM_CLIENT_PTR 	pClient, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTM_INT			nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTM_INT			nRespLen
);

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_connect
(
	FTM_IP_ADDR			xIP,
	FTM_USHORT 			nPort, 
	FTDM_CLIENT_HANDLE_PTR	phClient
)
{
	int 	hSock;
	struct sockaddr_in 	xServer;
	FTDM_CLIENT_PTR		pClient;

	if ( phClient == NULL )
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pClient = (FTDM_CLIENT_PTR)malloc(sizeof(FTDM_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == -1)
	{
		ERROR("Could not create socket.\n");	
		free(pClient);
		return	FTM_RET_ERROR;
	}

	xServer.sin_addr.s_addr	= xIP;
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(nPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		free(pClient);
		return	FTM_RET_ERROR;	
	}
	
	pClient->hSock = hSock;
	pClient->nTimeout = 5000;

	*phClient = pClient;

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_disconnect
(
 	FTDM_CLIENT_HANDLE	hClient
)
{
	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	close(hClient->hSock);
	hClient->hSock = 0;
	
	free(hClient);

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_isConnected
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_BOOL_PTR			pbConnected
)
{
	if ((hClient != NULL) && (hClient->hSock != 0))
	{
		*pbConnected = FTM_BOOL_TRUE;	
	}
	else
	{
		*pbConnected = FTM_BOOL_FALSE;	
	}

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_createNode
(
 	FTDM_CLIENT_HANDLE		hClient,
	FTM_NODE_INFO_PTR			pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_CREATE_NODE_PARAMS		xReq;
	FTDM_RESP_CREATE_NODE_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTDM_CMD_CREATE_NODE;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE_INFO));

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_destroyNode
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_CHAR_PTR			pDID
)
{
	FTM_RET						nRet;
	FTDM_REQ_DESTROY_NODE_PARAMS	xReq;
	FTDM_RESP_DESTROY_NODE_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_DESTROY_NODE;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_getNodeCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET							nRet;
	FTDM_REQ_GET_NODE_COUNT_PARAMS	xReq;
	FTDM_RESP_GET_NODE_COUNT_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_NODE_COUNT;
	xReq.nLen	=	sizeof(xReq);
	
	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pnCount = xResp.nCount;
	}	

	return	xResp.nRet;
}



FTM_RET FTDMC_getNodeInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_GET_NODE_INFO_BY_INDEX_PARAMS	xReq;
	FTDM_RESP_GET_NODE_INFO_BY_INDEX_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}


	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_NODE_INFO_BY_INDEX;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;
	
	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE_INFO));
	}
	
	return	xResp.nRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_getNodeInfo
(
 	FTDM_CLIENT_HANDLE		hClient,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_GET_NODE_INFO_PARAMS		xReq;
	FTDM_RESP_GET_NODE_INFO_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_NODE_INFO;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE_INFO));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_createEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EP_INFO_PTR		pInfo
)
{
	FTM_RET					nRet;
	FTDM_REQ_CREATE_EP_PARAMS	xReq;
	FTDM_RESP_CREATE_EP_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xReq.xCmd	=	FTDM_CMD_CREATE_EP;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP_INFO));

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_destroyEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID
)
{
	FTM_RET					nRet;
	FTDM_REQ_DESTROY_EP_PARAMS	xReq;
	FTDM_RESP_DESTROY_EP_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_DESTROY_EP;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_getEPCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_GET_EP_COUNT_PARAMS	xReq;
	FTDM_RESP_GET_EP_COUNT_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_GET_EP_COUNT;
	xReq.nLen	=	sizeof(xReq);

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pnCount = xResp.nCount;
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_getEPInfo
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR		pInfo
)
{
	FTM_RET						nRet;
	FTDM_REQ_GET_EP_INFO_PARAMS		xReq;
	FTDM_RESP_GET_EP_INFO_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_GET_EP_INFO;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_INFO));
	}
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_getEPInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_GET_EP_INFO_BY_INDEX_PARAMS	xReq;
	FTDM_RESP_GET_EP_INFO_BY_INDEX_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_GET_EP_INFO_BY_INDEX;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_INFO));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_appendEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pEPData
)
{
	FTM_RET						nRet;
	FTDM_REQ_APPEND_EP_DATA_PARAMS	xReq;
	FTDM_RESP_APPEND_EP_DATA_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_APPEND_EP_DATA;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	memcpy(&xReq.xData, pEPData, sizeof(FTM_EP_DATA));

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_getEPData
(
	FTDM_CLIENT_HANDLE	hClient,
	FTM_EPID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	FTM_RET								nRet;
	FTDM_REQ_GET_EP_DATA_PARAMS			xReq;
	FTM_ULONG							nRespSize = 0;
	FTDM_RESP_GET_EP_DATA_PARAMS_PTR	pResp = NULL;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTDM_RESP_GET_EP_DATA_PARAMS) + sizeof(FTM_EP_DATA) * nMaxCount;
	pResp = (FTDM_RESP_GET_EP_DATA_PARAMS_PTR)malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTDM_CMD_GET_EP_DATA;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nStartIndex=	nStartIndex;
	xReq.nCount		=	nMaxCount;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize);
	if (nRet != FTM_RET_OK)
	{
		free(pResp);
		return	FTM_RET_ERROR;	
	}

	nRet = pResp->nRet;

	if (pResp->nRet == FTM_RET_OK)
	{
		FTM_INT	i;

		TRACE("pResp->nCount = %d\n", pResp->nCount);
		for( i = 0 ; i < pResp->nCount && i < nMaxCount ; i++)
		{
			memcpy(&pData[i], &pResp->pData[i], sizeof(FTM_EP_DATA));
		}

		*pnCount = pResp->nCount;
	}

	free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_getEPDataWithTime
(
	FTDM_CLIENT_HANDLE	hClient,
	FTM_EPID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	FTM_RET										nRet;
	FTDM_REQ_GET_EP_DATA_WITH_TIME_PARAMS		xReq;
	FTM_ULONG									nRespSize = 0;
	FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS_PTR	pResp = NULL;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS) + sizeof(FTM_EP_DATA) * nMaxCount;
	pResp = (FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS_PTR)malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTDM_CMD_GET_EP_DATA_WITH_TIME;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime=	nBeginTime;
	xReq.nEndTime	=	nEndTime;
	xReq.nCount		=	nMaxCount;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize);
	if (nRet != FTM_RET_OK)
	{
		free(pResp);
		return	FTM_RET_ERROR;	
	}

	nRet = pResp->nRet;

	if (pResp->nRet == FTM_RET_OK)
	{
		FTM_INT	i;

		TRACE("pResp->nCount = %d\n", pResp->nCount);
		for( i = 0 ; i < pResp->nCount && i < nMaxCount ; i++)
		{
			memcpy(&pData[i], &pResp->pData[i], sizeof(FTM_EP_DATA));
		}

		*pnCount = pResp->nCount;
	}

	free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_removeEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_REMOVE_EP_DATA_PARAMS	xReq;
	FTDM_RESP_REMOVE_EP_DATA_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_REMOVE_EP_DATA;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nIndex		=	nIndex;
	xReq.nCount		=	nCount;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_removeEPDataWithTime
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime
)
{
	FTM_RET						nRet;
	FTDM_REQ_REMOVE_EP_DATA_WITH_TIME_PARAMS	xReq;
	FTDM_RESP_REMOVE_EP_DATA_WITH_TIME_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_REMOVE_EP_DATA_WITH_TIME;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime	=	nBeginTime;
	xReq.nEndTime	=	nEndTime;

	nRet = FTDMC_request(
				hClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTDMC_request
(
	FTDM_CLIENT_HANDLE 	hClient, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTM_INT			nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTM_INT			nRespLen
)
{
	FTM_INT	nTimeout;


	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	TRACE("send(%08lx, pReq, %d, 0)\n", hClient->hSock, nReqLen);


	if( send(hClient->hSock, pReq, nReqLen, 0) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	nTimeout = hClient->nTimeout;
	while(--nTimeout > 0)
	{
		int	nLen = recv(hClient->hSock, pResp, nRespLen, MSG_DONTWAIT);
		if (nLen > 0)
		{
			TRACE("recv(%08lx, pResp, %d, MSG_DONTWAIT)\n", hClient->hSock, nLen);
			return	FTM_RET_OK;	
		}

		usleep(1000);
	}

	return	FTM_RET_COMM_TIMEOUT;	
}

