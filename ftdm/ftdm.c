#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "debug.h"

#define	FTDM_SERVICE_PORT	8888

void *FTDM_serviceHandler(void *pData);

int FTDM_main(void)
{
	int					nRet;
	int					hSocket;
	struct sockaddr_in	xServer, xClient;

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	1;
	}

	xServer.sin_family = AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port = htons( FTDM_SERVICE_PORT );

	nRet = bind( hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR("bin failed.\n");
		return	1;	
	}

	listen(hSocket, 3);

	puts("Waiting for incomint connections ...\n");

	while(1)
	{
		int hClient;
		int	nSockAddrInLen = sizeof(struct sockaddr_in);	

		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient != 0)
		{
			pthread_t xPthread;	

			puts("Accept ...\n");
			pthread_create(&xPthread, NULL, FTDM_serviceHandler, (void *)hClient);
		}
		
	
	}
}

void *FTDM_serviceHandler(void *pData)
{
	int		hSock = (int)pData;
	int		nLen;
	char	pBuff[2048];

	while(1)
	{
		int	nLen;

		nLen = recv(hSock, pBuff, sizeof(pBuff), 0);
		if (nLen == 0)
		{
			break;	
		}
		else if (nLen < 0)
		{
			break;	
		}
	}

	close(hSock);
	return	0;
}
