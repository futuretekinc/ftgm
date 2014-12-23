#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm.h"
#include "ftdm_client.h"

static FTDM_RET FTDMC_request
(
	FTDM_CLIENT_PTR 	pClient, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTDM_INT			nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTDM_INT			nRespLen
);

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_connect
(
	FTDM_IP_ADDR			xIP,
	FTDM_USHORT 			nPort, 
	FTDM_CLIENT_HANDLE_PTR	phClient
)
{
	int 	hSock;
	struct sockaddr_in 	xServer;
	FTDM_CLIENT_PTR		pClient;

	ASSERT( phClient != NULL );

	pClient = (FTDM_CLIENT_PTR)malloc(sizeof(FTDM_CLIENT));
	if (pClient == NULL)
	{
		return	FTDM_RET_NOT_ENOUGH_MEMORY;	
	}

	hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == -1)
	{
		printf("Could not create socket.\n");	
		free(pClient);
		return	FTDM_RET_ERROR;
	}

	xServer.sin_addr.s_addr	= xIP;
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(nPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		free(pClient);
		return	FTDM_RET_ERROR;	
	}
	
	pClient->hSock = hSock;
	pClient->nTimeout = 5000;

	*phClient = pClient;

	return	FTDM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_disconnect
(
 	FTDM_CLIENT_HANDLE	hClient
)
{
	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	close(hClient->hSock);
	hClient->hSock = 0;
	
	free(hClient);

	return	FTDM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_isConnected
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_BOOL_PTR			pbConnected
)
{
	if ((hClient != NULL) && (hClient->hSock != 0))
	{
		*pbConnected = FTDM_BOOL_TRUE;	
	}
	else
	{
		*pbConnected = FTDM_BOOL_FALSE;	
	}

	return	FTDM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_createDevice
(
 	FTDM_CLIENT_HANDLE		hClient,
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_TYPE		xType,
	FTDM_CHAR_PTR			pURL,
	FTDM_CHAR_PTR			pLocation
)
{
	FTDM_RET						nRet;
	FTDM_REQ_CREATE_DEVICE_PARAMS	xReq;
	FTDM_RESP_CREATE_DEVICE_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((strlen(pDID) > FTDM_DEVICE_ID_LEN) ||
		(strlen(pURL) > FTDM_DEVICE_URL_LEN) || 
		(strlen(pLocation) > FTDM_DEVICE_LOCATION_LEN))
	{
		return	FTDM_RET_DATA_LEN_RANGE;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTDM_CMD_CREATE_DEVICE;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.xInfo.pDID, pDID);
	xReq.xInfo.xType = xType;
	strcpy(xReq.xInfo.pURL, pURL);
	strcpy(xReq.xInfo.pLocation, pLocation);

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_destroyDevice
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_CHAR_PTR			pDID
)
{
	FTDM_RET						nRet;
	FTDM_REQ_DESTROY_DEVICE_PARAMS	xReq;
	FTDM_RESP_DESTROY_DEVICE_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}


	if (strlen(pDID) > FTDM_DEVICE_ID_LEN)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_DESTROY_DEVICE;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_getDeviceCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG_PTR			pnCount
)
{
	FTDM_RET							nRet;
	FTDM_REQ_GET_DEVICE_COUNT_PARAMS	xReq;
	FTDM_RESP_GET_DEVICE_COUNT_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_DEVICE_COUNT;
	xReq.nLen	=	sizeof(xReq);
	
	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	*pnCount = xResp.nCount;

	return	xResp.nRet;
}



FTDM_RET FTDMC_getDeviceInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG				nIndex,
	FTDM_DEVICE_INFO_PTR	pInfo
)
{
	FTDM_RET							nRet;
	FTDM_REQ_GET_DEVICE_INFO_BY_INDEX_PARAMS		xReq;
	FTDM_RESP_GET_DEVICE_INFO_BY_INDEX_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}


	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_DEVICE_INFO_BY_INDEX;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;
	
	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	if (xResp.nRet == FTDM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTDM_DEVICE_INFO));
	}
	
	return	xResp.nRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_getDeviceInfo
(
 	FTDM_CLIENT_HANDLE		hClient,
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_INFO_PTR	pInfo
)
{
	FTDM_RET							nRet;
	FTDM_REQ_GET_DEVICE_INFO_PARAMS		xReq;
	FTDM_RESP_GET_DEVICE_INFO_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}


	if (strlen(pDID) > FTDM_DEVICE_ID_LEN)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_DEVICE_INFO;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	if (xResp.nRet == FTDM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTDM_DEVICE_INFO));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_createEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_INFO_PTR		pInfo
)
{
	FTDM_RET					nRet;
	FTDM_REQ_CREATE_EP_PARAMS	xReq;
	FTDM_RESP_CREATE_EP_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}


	xReq.xCmd	=	FTDM_CMD_CREATE_EP;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTDM_EP_INFO));

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_destroyEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID				xEPID
)
{
	FTDM_RET					nRet;
	FTDM_REQ_DESTROY_EP_PARAMS	xReq;
	FTDM_RESP_DESTROY_EP_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_DESTROY_EP;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_getEPCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG_PTR			pnCount
)
{
	FTDM_RET						nRet;
	FTDM_REQ_GET_EP_COUNT_PARAMS	xReq;
	FTDM_RESP_GET_EP_COUNT_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_GET_EP_COUNT;
	xReq.nLen	=	sizeof(xReq);

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	*pnCount = xResp.nCount;

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_getEPInfo
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID				xEPID,
	FTDM_EP_INFO_PTR		pInfo
)
{
	FTDM_RET						nRet;
	FTDM_REQ_GET_EP_INFO_PARAMS		xReq;
	FTDM_RESP_GET_EP_INFO_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_GET_EP_INFO;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;


	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	memcpy(pInfo, &xResp.xInfo, sizeof(FTDM_EP_INFO));
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_getEPInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG				nIndex,
	FTDM_EP_INFO_PTR		pInfo
)
{
	FTDM_RET								nRet;
	FTDM_REQ_GET_EP_INFO_BY_INDEX_PARAMS	xReq;
	FTDM_RESP_GET_EP_INFO_BY_INDEX_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_GET_EP_INFO_BY_INDEX;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;


	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	memcpy(pInfo, &xResp.xInfo, sizeof(FTDM_EP_INFO));
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_appendEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nTime,
	FTDM_ULONG				nValue
)
{
	FTDM_RET						nRet;
	FTDM_REQ_APPEND_EP_DATA_PARAMS	xReq;
	FTDM_RESP_APPEND_EP_DATA_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_APPEND_EP_DATA;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	xReq.nTime	=	nTime;
	xReq.nValue	=	nValue;

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_getEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				nBeginTime,
	FTDM_ULONG				nEndTime,
	FTDM_EP_DATA_PTR		pData,
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pnCount
)
{
	FTDM_RET							nRet;
	FTDM_ULONG							nReqSize = 0;
	FTDM_REQ_GET_EP_DATA_PARAMS_PTR		pReq = NULL;
	FTDM_ULONG							nRespSize = 0;
	FTDM_RESP_GET_EP_DATA_PARAMS_PTR	pResp = NULL;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	nReqSize = sizeof(FTDM_REQ_GET_EP_DATA_PARAMS) + sizeof(FTDM_EP_ID) * nEPIDCount;

	pReq = (FTDM_REQ_GET_EP_DATA_PARAMS_PTR)malloc(nReqSize);
	if (pReq == NULL)
	{
		CALL_TRACE();
		return	FTDM_RET_NOT_ENOUGH_MEMORY;
	}

	nRespSize = sizeof(FTDM_RESP_GET_EP_DATA_PARAMS) + sizeof(FTDM_EP_DATA) * nMaxCount;
	pResp = (FTDM_RESP_GET_EP_DATA_PARAMS_PTR)malloc(nRespSize);
	if (pResp == NULL)
	{
		CALL_TRACE();
		free(pReq);
		return	FTDM_RET_NOT_ENOUGH_MEMORY;
	}

	pReq->xCmd		=	FTDM_CMD_GET_EP_DATA;
	pReq->nLen		=	nReqSize;
	pReq->nBeginTime=	nBeginTime;
	pReq->nEndTime	=	nEndTime;
	pReq->nCount	=	nMaxCount;
	pReq->nEPIDCount=	nEPIDCount;

	memcpy(pReq->pEPID,	pEPID, sizeof(FTDM_EP_ID) * nEPIDCount) ;

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)pReq, 
				nReqSize, 
				(FTDM_VOID_PTR)pResp, 
				nRespSize);
	if (nRet != FTDM_RET_OK)
	{
		CALL_TRACE();
		free(pReq);
		free(pResp);
		return	FTDM_RET_ERROR;	
	}

	nRet = pResp->nRet;

	if (pResp->nRet == FTDM_RET_OK)
	{
		FTDM_INT	i;

		for( i = 0 ; i < pResp->nCount ; i++)
		{
			memcpy(&pData[i], &pResp->pData[i], sizeof(FTDM_EP_DATA));
		}

		*pnCount = pResp->nCount;
	}

	free(pReq);
	free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_removeEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				nBeginTime,
	FTDM_ULONG				nEndTime,
	FTDM_ULONG				nCount
)
{
	FTDM_RET						nRet;
	FTDM_REQ_REMOVE_EP_DATA_PARAMS	xReq;
	FTDM_RESP_REMOVE_EP_DATA_PARAMS	xResp;

	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_REMOVE_EP_DATA;
	xReq.nLen		=	sizeof(xReq) + sizeof(FTDM_EP_ID) * nEPIDCount;
	xReq.nBeginTime	=	nBeginTime;
	xReq.nEndTime	=	nEndTime;
	xReq.nCount		=	nCount;
	xReq.nEPIDCount	=	nEPIDCount;
	memcpy(xReq.pEPID, pEPID, sizeof(FTDM_EP_ID) * nEPIDCount);

	nRet = FTDMC_request(
				hClient, 
				(FTDM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTDM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTDM_RET_OK)
	{
		return	FTDM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTDM_RET FTDMC_request
(
	FTDM_CLIENT_HANDLE 	hClient, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTDM_INT			nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTDM_INT			nRespLen
)
{
	FTDM_INT	nTimeout;


	if ((hClient == NULL) || (hClient->hSock == 0))
	{
		return	FTDM_RET_CLIENT_HANDLE_INVALID;	
	}

	if( send(hClient->hSock, pReq, nReqLen, 0) < 0)
	{
		return	FTDM_RET_ERROR;	
	}

	nTimeout = hClient->nTimeout;
	while(--nTimeout > 0)
	{
		int	nLen = recv(hClient->hSock, pResp, nRespLen, MSG_DONTWAIT);
		if (nLen > 0)
		{
			return	FTDM_RET_OK;	
		}

		usleep(1000);
	}

	return	FTDM_RET_COMM_TIMEOUT;	
}

