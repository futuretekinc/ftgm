#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "debug.h"


#define	FTDM_SERVICE_PORT	8888
#define	FTDM_PACKET_LEN		2048

typedef	struct
{
	FTDM_INT			hSocket;
	struct sockaddr_in	xPeer;
	FTDM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTDM_BYTE			pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

static FTDM_VOID_PTR 	FTDMS_serviceHandler(FTDM_VOID_PTR pData);
static FTDM_RET 		FTDMS_startDaemon(FTDM_USHORT nPort);

int main(int argc, char *argv[])
{
	FTDM_INT	nOpt;
	FTDM_USHORT	nPort = FTDM_SERVICE_PORT;

	while((nOpt = getopt(argc, argv, "P:SV")) != -1)
	{
		switch(nOpt)
		{
		case	'P':
			nPort = atoi(optarg);
			break;

		case	'S':
			setPrintMode(0);
			break;

		case	'V':
			setPrintMode(2);
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

	FTDM_init();

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	FTDM_RET_ERROR;
	}

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( nPort );

	nRet = bind( hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR("bind failed.\n");
		return	FTDM_RET_ERROR;
	}

	listen(hSocket, 3);

	MESSAGE("Waiting for incoming connections ...\n");

	while(1)
	{
		int hClient;
		int	nSockAddrInLen = sizeof(struct sockaddr_in);	

		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient != 0)
		{
			pthread_t xPthread;	
			FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)malloc(sizeof(FTDM_SESSION));
			if (pSession == NULL)
			{
				ERROR("System memory is not enough!\n");
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				TRACE("The new session has beed connected\n"\
					  "HANDLE : %08lx\n"\
					  "  PEER : %s:%d\n", 
					  hClient, 
					  inet_ntoa(xClient.sin_addr), 
					  ntohs(xClient.sin_port));

				pSession->hSocket = hClient;
				memcpy(&pSession->xPeer, &xClient, sizeof(xClient));
				pthread_create(&xPthread, NULL, FTDMS_serviceHandler, pSession);
			}
		}
	}

	return	FTDM_RET_OK;
}

FTDM_VOID_PTR FTDMS_serviceHandler(FTDM_VOID_PTR pData)
{
	FTDM_SESSION_PTR		pSession= (FTDM_SESSION_PTR)pData;
	FTDM_REQ_PARAMS_PTR		pReq 	= (FTDM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTDM_RESP_PARAMS_PTR	pResp 	= (FTDM_RESP_PARAMS_PTR)pSession->pRespBuff;

	while(1)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		TRACE("recv(%08x, pReq, %lu, MSG_DONTWAIT)\n", pSession->hSocket, nLen);
		if (nLen == 0)
		{
			TRACE("The connection is terminated.\n");
			break;	
		}
		else if (nLen < 0)
		{
			ERROR("recv failed[%d]\n", -nLen);
			break;	
		}

		if (FTDM_RET_OK != FTDMS_service(pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->nRet = FTDM_RET_INTERNAL_ERROR;
			pResp->nLen = sizeof(FTDM_RESP_PARAMS);
		}

		TRACE("send(%08x, pResp, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->nLen);
		nLen = send(pSession->hSocket, pResp, pResp->nLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			ERROR("send failed[%d]\n", -nLen);	
			break;
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	return	0;
}

