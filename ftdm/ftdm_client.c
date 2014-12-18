#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftgm_type.h"
#include "ftdm_client.h"
#include "ftdm_if.h"
#include "debug.h"

FTGM_RET FTDMC_connect
(
 	FTGM_STRING 	strAddress, 
	FTGM_USHORT 	nPort, 
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
		return	FTGM_RET_ERROR;
	}

	xServer.sin_addr.s_addr	= inet_addr(strAddress);
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(nPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		return	FTGM_RET_ERROR;	
	}
	
	pClient->hSock = hSock;

	return	FTGM_RET_OK;
}

FTGM_RET FTDMC_disconnect
(
 	FTDM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	close(pClient->hSock);
	pClient->hSock = 0;
		
	return	FTGM_RET_OK;
}

FTGM_RET FTDMC_request
(
	FTDM_CLIENT_PTR 	pClient, 
	FTGM_BYTE_PTR 		pData, 
	FTGM_INT 			nDataLen, 
	FTGM_BYTE_PTR 		pBuff, 
	FTGM_INT_PTR 		pBuffLen
)
{
	FTGM_INT	nTimeout;

	ASSERT((pClient != NULL) && (pData != NULL) &&  (pBuff != NULL));

	if( send(pClient->hSock, pData, nDataLen, 0) < 0)
	{
		return	FTGM_RET_ERROR;	
	}

	nTimeout = pClient->nTimeout;
	while(--nTimeout > 0)
	{
		int	nLen = recv(pClient->hSock, pBuff, *pBuffLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			return	FTGM_RET_ERROR;	
		}

		usleep(1000);
	}

	return	FTGM_RET_OK;	
}

FTGM_RET FTDMC_devInsert
(
 	FTGM_DEVICE_ID			xDIV,
	FTGM_DEVICE_TYPE		xType,
	FTGM_DEVICE_URL			xURL,
	FTGM_DEVICE_LOC			xLoc
)
{

}

FTGM_RET FTDMC_devInfo
(
 	FTDM_CLIENT_PTR			pClient,
	FTGM_DEVICE_ID			xDID,
	FTGM_DEVICE_INFO_PTR	pInfo
)
{
	FTDM_REQ_DEVICE_INFO_PARAMS	xReq;
	FTDM_REP_DEVICE_INFO_PARAMS	xResp;
	FTGM_INT	nResp = sizeof(xResp);

	memcpy(xReq.xDID, xDID, sizeof(FTGM_DEVICE_ID));
	
	if (FTDMC_request(pClient, (FTGM_BYTE_PTR)&xReq, sizeof(xReq), (FTGM_BYTE_PTR)&xResp, &nResp) != FTGM_RET_OK)
	{
		return	FTGM_RET_ERROR;	
	}

	if (xResp.nRet == FTGM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTGM_DEVICE_INFO));
	}
	
	return	xResp.nRet;
}
	

