#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm_client.h"
#include "debug.h"

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
	FTDM_IP_ADDR	xIP,
	FTDM_USHORT 	nPort, 
	FTDM_CLIENT_PTR pClient
)
{
	int 	hSock;
	struct sockaddr_in xServer;

	ASSERT( (strAddress != NULL) && (pClient != NULL) );

	hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == -1)
	{
		printf("Could not create socket.\n");	
		return	FTDM_RET_ERROR;
	}

	xServer.sin_addr.s_addr	= xIP;
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(nPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		return	FTDM_RET_ERROR;	
	}
	
	pClient->hSock = hSock;

	return	FTDM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_disconnect
(
 	FTDM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	close(pClient->hSock);
	pClient->hSock = 0;
		
	return	FTDM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET FTDMC_createDevice
(
 	FTDM_CLIENT_PTR			pClient,
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_TYPE		xType,
	FTDM_BYTE_PTR			pURL,
	FTDM_INT				nURLLen,
	FTDM_BYTE_PTR			pLocation,
	FTDM_INT				nLocationLen
)
{
	FTDM_RET						nRet;
	FTDM_REQ_CREATE_DEVICE_PARAMS	xReq;
	FTDM_RESP_CREATE_DEVICE_PARAMS	xResp;

	if ((nURLLen > FTDM_DEVICE_URL_LEN) || 
		(nLocationLen > FTDM_DEVICE_LOCATION_LEN))
	{
		return	FTDM_RET_DATA_LEN_RANGE;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTDM_CMD_CREATE_DEVICE;
	xReq.nLen	=	sizeof(xReq);
	memcpy(xReq.xInfo.pDID, pDID, FTDM_DEVICE_ID_LEN) ;
	xReq.xInfo.xType = xType;
	memcpy(xReq.xInfo.pURL, pURL, nURLLen);
	memcpy(xReq.xInfo.pLocation, pLocation, nLocationLen);

	nRet = FTDMC_request(
				pClient, 
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
	FTDM_CLIENT_PTR			pClient,
	FTDM_BYTE_PTR			pDID
)
{
	FTDM_RET						nRet;
	FTDM_REQ_DESTROY_DEVICE_PARAMS	xReq;
	FTDM_RESP_DESTROY_DEVICE_PARAMS	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_DESTROY_DEVICE;
	xReq.nLen	=	sizeof(xReq);
	memcpy(xReq.pDID, pDID, FTDM_DEVICE_ID_LEN);
	
	nRet = FTDMC_request(
				pClient, 
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
FTDM_RET FTDMC_getDeviceInfo
(
 	FTDM_CLIENT_PTR			pClient,
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_INFO_PTR	pInfo
)
{
	FTDM_RET							nRet;
	FTDM_REQ_GET_DEVICE_INFO_PARAMS		xReq;
	FTDM_RESP_GET_DEVICE_INFO_PARAMS	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_GET_DEVICE_INFO;
	xReq.nLen	=	sizeof(xReq);
	memcpy(xReq.pDID, pDID, FTDM_DEVICE_ID_LEN);
	
	nRet = FTDMC_request(
				pClient, 
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
	FTDM_CLIENT_PTR			pClient,
	FTDM_EP_INFO_PTR		pInfo
)
{
	FTDM_RET					nRet;
	FTDM_REQ_CREATE_EP_PARAMS	xReq;
	FTDM_RESP_CREATE_EP_PARAMS	xResp;

	ASSERT((pClient!= NULL) && (pInfo != NULL));

	xReq.xCmd	=	FTDM_CMD_CREATE_EP;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTDM_EP_INFO));

	nRet = FTDMC_request(
				pClient, 
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
	FTDM_CLIENT_PTR			pClient,
	FTDM_EP_ID				xEPID
)
{
	FTDM_RET					nRet;
	FTDM_REQ_DESTROY_EP_PARAMS	xReq;
	FTDM_RESP_DESTROY_EP_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_DESTROY_EP;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTDMC_request(
				pClient, 
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
FTDM_RET	FTDMC_getEPInfo
(
	FTDM_CLIENT_PTR			pClient,
	FTDM_EP_ID				xEPID,
	FTDM_EP_INFO_PTR		pInfo
)
{
	FTDM_RET						nRet;
	FTDM_REQ_GET_EP_INFO_PARAMS		xReq;
	FTDM_RESP_GET_EP_INFO_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_GET_EP_INFO;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;


	nRet = FTDMC_request(
				pClient, 
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
FTDM_RET	FTDMC_appendEPData
(
	FTDM_CLIENT_PTR			pClient,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nTime,
	FTDM_ULONG				nValue
)
{
	FTDM_RET						nRet;
	FTDM_REQ_APPEND_EP_DATA_PARAMS	xReq;
	FTDM_RESP_APPEND_EP_DATA_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_APPEND_EP_DATA;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	xReq.nTime	=	nTime;
	xReq.nValue	=	nValue;

	nRet = FTDMC_request(
				pClient, 
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
	FTDM_CLIENT_PTR			pClient,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nBeginTime,
	FTDM_ULONG				nEndTime,
	FTDM_ULONG				nCount,
	FTDM_EP_DATA_PTR		pData
)
{
	FTDM_RET						nRet;
	FTDM_REQ_GET_EP_DATA_PARAMS		xReq;
	FTDM_RESP_GET_EP_DATA_PARAMS	xResp;

	xReq.xCmd		=	FTDM_CMD_GET_EP_DATA;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime	=	nBeginTime;
	xReq.nEndTime	=	nEndTime;
	xReq.nCount		=	nCount;

	nRet = FTDMC_request(
				pClient, 
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
		FTDM_INT	i;

		for( i = 0 ; i < nCount && i < xResp.nCount ; i++)
		{
			memcpy(&pData[i], &xResp.pData[i], sizeof(FTDM_EP_DATA));
		}
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTDM_RET	FTDMC_removeEPData
(
	FTDM_CLIENT_PTR			pClient,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nBeginTime,
	FTDM_ULONG				nEndTime,
	FTDM_ULONG				nCount
)
{
	FTDM_RET						nRet;
	FTDM_REQ_REMOVE_EP_DATA_PARAMS	xReq;
	FTDM_RESP_REMOVE_EP_DATA_PARAMS	xResp;

	xReq.xCmd		=	FTDM_CMD_REMOVE_EP_DATA;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime	=	nBeginTime;
	xReq.nEndTime	=	nEndTime;
	xReq.nCount		=	nCount;

	nRet = FTDMC_request(
				pClient, 
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
	FTDM_CLIENT_PTR 	pClient, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTDM_INT			nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTDM_INT			nRespLen
)
{
	FTDM_INT	nTimeout;

	ASSERT((pClient != NULL) && (pReq != NULL) &&  (pResp != NULL));

	if( send(pClient->hSock, pReq, nReqLen, 0) < 0)
	{
		return	FTDM_RET_ERROR;	
	}

	nTimeout = pClient->nTimeout;
	while(--nTimeout > 0)
	{
		int	nLen = recv(pClient->hSock, pResp, nRespLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			return	FTDM_RET_ERROR;	
		}

		usleep(1000);
	}

	return	FTDM_RET_OK;	
}

