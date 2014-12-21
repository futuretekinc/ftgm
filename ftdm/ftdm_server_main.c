#include <stdio.h>
#include <stdlib.h>
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
	FTDM_INT	hSocket;
	FTDM_BYTE	pReqBuff[FTDM_PACKET_LEN];
	FTDM_BYTE	pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

static FTDM_VOID_PTR 	FTDMS_serviceHandler(FTDM_VOID_PTR pData);
static FTDM_RET 		FTDMS_startDaemon(FTDM_USHORT nPort);

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
			FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)malloc(sizeof(FTDM_SESSION));
			if (pSession == NULL)
			{
				ERROR("System memory is not enough!\n");
			}
			else
			{
				TRACE("New session connected[hSession : %08x]\n", hClient);
				pSession->hSocket = hClient;
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

		nLen = recv(pSession->hSocket, pSession->pReqBuff, sizeof(pSession->pReqBuff), 0);
		if (nLen == 0)
		{
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

		nLen = send(pSession->hSocket, pSession->pRespBuff, sizeof(pSession->pRespBuff), 0);
		if (nLen < 0)
		{
			ERROR("send failed[%d]\n", -nLen);	
			break;
		}
	}

	TRACE("Session Closed[hSession : %08x]\n", pSession->hSocket);
	close(pSession->hSocket);

	return	0;
}

