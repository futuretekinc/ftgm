#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "libconfig.h"
#include "simclist.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "ftdm_config.h"
#include "debug.h"

#define	FTDM_PACKET_LEN					2048

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

static FTM_VOID_PTR 	_serviceHandler(FTM_VOID_PTR pData);
static FTM_VOID_PTR		_startDaemon(FTM_VOID_PTR pData);
static FTM_VOID			_showUsage(FTM_CHAR_PTR pAppName);

extern char *program_invocation_short_name;

static FTDM_CFG	xConfig;
static sem_t	xSemaphore;

int main(int nArgc, char *pArgv[])
{
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_BOOL_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	pthread_t xPthread;	
		

	
	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgv, "c:d?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strcpy(pConfigFileName, optarg);
			}
			break;

		case	'd':
			{
				bDaemon = FTM_BOOL_TRUE;
			}
			break;

		case	'?':
		default:
			_showUsage(pArgv[0]);
			return	0;
		}
	}


	setPrintMode(2);

	/* load configuration  */
	FTDM_CFG_init(&xConfig);
	FTDM_CFG_load(&xConfig, pConfigFileName);
	FTDM_CFG_show(&xConfig);

	/* apply configuration */
	setPrintMode(2);

	if (sem_init(&xSemaphore, 0,xConfig.xServer.ulMaxSession) < 0)
	{
		ERROR("Can't alloc semaphore!\n");
		return	0;	
	}

	if (!bDaemon || (fork() == 0))
	{
		pthread_create(&xPthread, NULL, _startDaemon, (void *)&xConfig);
	}

	FTDM_CFG_final(&xConfig);

	pthread_join(xPthread, NULL);

	return	0;
}

FTM_VOID_PTR _startDaemon(FTM_VOID_PTR pData)
{
	FTM_INT				nRet;
	FTM_INT				hSocket;
	struct sockaddr_in	xServer, xClient;
	FTDM_CFG_PTR pConfig = (FTDM_CFG_PTR)pData;

	FTDM_init(pConfig);

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	0;
	}

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( pConfig->xServer.usPort );

	nRet = bind( hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR("bind failed.[nRet = %d]\n", nRet);
		return	0;
	}

	listen(hSocket, 3);


	while(1)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrInLen = sizeof(struct sockaddr_in);	

		sem_getvalue(&xSemaphore, &nValue);
		MESSAGE("Waiting for connections ...[%d]\n", nValue);
		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient != 0)
		{
			pthread_t xPthread;	

			TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClient.sin_addr), ntohs(xClient.sin_port));

			FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)malloc(sizeof(FTDM_SESSION));
			if (pSession == NULL)
			{
				ERROR("System memory is not enough!\n");
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				TRACE("The new session(%08x) has beed connected\n", hClient);

				pSession->hSocket = hClient;
				memcpy(&pSession->xPeer, &xClient, sizeof(xClient));
				pthread_create(&xPthread, NULL, _serviceHandler, pSession);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR _serviceHandler(FTM_VOID_PTR pData)
{
	FTDM_SESSION_PTR		pSession= (FTDM_SESSION_PTR)pData;
	FTDM_REQ_PARAMS_PTR		pReq 	= (FTDM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTDM_RESP_PARAMS_PTR	pResp 	= (FTDM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timespec			xTimeout = { .tv_sec = 2, .tv_nsec = 0};

	if (sem_timedwait(&xSemaphore, &xTimeout) < 0)
	{
		TRACE("The session(%08x) was closed\n", pSession->hSocket);
		shutdown(pSession->hSocket, SHUT_RD);
		return	0;	
	}

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

		if (FTM_RET_OK != FTDMS_service(pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->nRet = FTM_RET_INTERNAL_ERROR;
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

	sem_post(&xSemaphore);

	return	0;
}

FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName)
{
	MESSAGE("Usage : %s [-d] [-m 0|1|2]\n", pAppName);
	MESSAGE("\tFutureTek Data Manger for M2M gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("    -d      Run as a daemon\n");
	MESSAGE("    -m <n>  Set message output mode.\n");
	MESSAGE("            0 - Don't output any messages.\n");
	MESSAGE("            1 - Only general message\n");
	MESSAGE("            2 - All message(include debugging message).\n");
}
