#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "debug.h"


#define	FTDM_SERVICE_PORT	8888

static FTDM_VOID_PTR FTDMS_serviceHandler(FTDM_VOID_PTR pData);
static FTDM_RET FTDMS_startDaemon(FTDM_USHORT nPort);

int main(int argc, char *argv[])
{
	FTDM_INT	nOpt;
	FTDM_USHORT	nPort = FTDM_SERVICE_PORT;

	while((nOpt = getopt(argc, argv, "P:")) != -1)
	{
		switch(nOpt)
		{
		case	'P':
			nPort = atoi(optarg);
			break;

		default:
			break;
		}
	}

	FTDMS_startDaemon(nPort);

	return	0;
}

FTDM_RET FTDMS_startDaemon(FTDM_USHORT nPort)
{
	int					nRet;
	int					hSocket;
	struct sockaddr_in	xServer, xClient;

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	FTDM_RET_ERROR;
	}

	xServer.sin_family = AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port = htons( nPort );

	nRet = bind( hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR("bin failed.\n");
		return	FTDM_RET_ERROR;
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
			pthread_create(&xPthread, NULL, FTDMS_serviceHandler, (void *)hClient);
		}
	}

	return	FTDM_RET_OK;
}

FTDM_VOID_PTR FTDMS_serviceHandler(FTDM_VOID_PTR pData)
{
	int		hSock = (int)pData;
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

