#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftgm_type.h"
#include "debug.h"

typedef struct 
{
	FTGM_INT	hSock;
}	FTDM_CLIENT, * FTDM_CLIENT_PTR;

FTGM_RET FTDMC_connect(FTGM_STRING strAddress, FTGM_USHORT nPort, FTDM_CLIENT_PTR pClient);

FTGM_RET FTDMC_Request
(
	FTDM_CLIENT_PTR 	pClient, 
	FTGM_BYTE_PTR 		pData, 
	FTGM_INT 			nDataLen, 
	FTGM_BYTE_PTR 		pBuff, 
	FTGM_INT_PTR 		pBuffLen
);

FTGM_STRING	_strPrompt = "FTDMC> ";

int main(int argc , char *argv[])
{
	FTDM_CLIENT	xClient;
	FTGM_BYTE	pSendMessage[1000], pRecvBuff[2000];

	
	if (FTDMC_connect("127.0.0.1", 8888, &xClient) != FTGM_RET_OK)
	{
		perror("connect failed. Error");
		return	0;	
	}

	puts("Connected\n");

	while(1)
	{
		FTGM_INT	nBuffLen = 0;
		printf(_strPrompt);
		gets(pSendMessage);

		if (pSendMessage[0] == 'q')
		{
			break;	
		}

		nBuffLen = sizeof(pRecvBuff);
		FTDMC_Request(&xClient, pSendMessage, strlen((char*)pSendMessage), pRecvBuff, &nBuffLen);
	}

	close(xClient.hSock);

	return 0;
}

FTGM_RET FTDMC_connect(FTGM_STRING strAddress, FTGM_USHORT nPort, FTDM_CLIENT_PTR pClient)
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

FTGM_RET FTDMC_Request
(
	FTDM_CLIENT_PTR 	pClient, 
	FTGM_BYTE_PTR 		pData, 
	FTGM_INT 			nDataLen, 
	FTGM_BYTE_PTR 		pBuff, 
	FTGM_INT_PTR 		pBuffLen
)
{
	FTGM_ULONG	uTimeout = 5000;

	ASSERT((pClient != NULL) && (pData != NULL) &&  (pBuff != NULL));

	if( send(pClient->hSock, pData, nDataLen, 0) < 0)
	{
		return	FTGM_RET_ERROR;	
	}

	while(--uTimeout > 0)
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
