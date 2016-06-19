#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "ftm.h"
#include "ftom_client_net.h"
#include "ftom_params.h"
#include "ftom_session.h"

static 
FTM_VOID_PTR FTOM_CLIENT_NET_masterProcess
(
	FTM_VOID_PTR pData
);

static 
FTM_VOID_PTR FTOM_CLIENT_NET_subscribeProcess
(
	FTM_VOID_PTR pData
);

FTM_RET	FTOM_CLIENT_NET_init
(
	FTOM_CLIENT_NET_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;

	memset(pClient, 0, sizeof(FTOM_CLIENT_NET));

	pClient->xCommon.fStart = (FTOM_CLIENT_START)FTOM_CLIENT_NET_start;
	pClient->xCommon.fStop = (FTOM_CLIENT_STOP)FTOM_CLIENT_NET_stop;
	pClient->xCommon.fSetConfig = (FTOM_CLIENT_SET_CONFIG)FTOM_CLIENT_NET_setConfig;
	pClient->xCommon.fLoadConfigFromFile = (FTOM_CLIENT_LOAD_CONFIG_FROM_FILE)FTOM_CLIENT_NET_loadConfigFromFile;
	pClient->xCommon.fSetNotifyCB = NULL;
	pClient->xCommon.fRequest = (FTOM_CLIENT_REQUEST)FTOM_CLIENT_NET_request;	

	strcpy(pClient->xConfig.xServer.pHost, "127.0.0.1");
	pClient->xConfig.xServer.usPort 		= 8889;
	strcpy(pClient->xConfig.xPublishServer.pHost, "127.0.0.1");
	pClient->xConfig.xPublishServer.usPort 	= 9000;

	pClient->bStop 		= FTM_TRUE;
	pClient->bConnected = FTM_FALSE;
	pClient->hSock 		= -1;
	pClient->ulTimeout 	= 5000000;

	sem_init(&pClient->xReqLock, 0, 0);

	xRet = FTM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Can't create message queue.[%08lx]\n", xRet);	
	}
	else
	{
		pClient->bInit = FTM_TRUE;
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_NET_final
(
	FTOM_CLIENT_NET_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (!pClient->bInit)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	if (!pClient->bStop)
	{
		FTOM_CLIENT_NET_stop(pClient);	
	}

	sem_destroy(&pClient->xReqLock);
	FTM_MSGQ_destroy(pClient->pMsgQ);

	pClient->pMsgQ = NULL;
	pClient->bInit = FTM_FALSE;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_create
(
	FTOM_CLIENT_NET_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_CLIENT_NET_PTR	pClient;

	pClient = (FTOM_CLIENT_NET_PTR)FTM_MEM_malloc(sizeof(FTOM_CLIENT_NET));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}
	
	memset(pClient, 0, sizeof(FTOM_CLIENT_NET));

	FTOM_CLIENT_NET_init(pClient);

	*ppClient = pClient;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_destroy
(
	FTOM_CLIENT_NET_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	
	FTOM_CLIENT_NET_final(*ppClient);

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_setConfig
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTOM_CLIENT_NET_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_CLIENT_NET_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_getConfig
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTOM_CLIENT_NET_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &pClient->xConfig, sizeof(FTOM_CLIENT_NET_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_start
(
	FTOM_CLIENT_NET_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (!pClient->bStop)
	{
		return	FTM_RET_ALREADY_RUNNING;
	}

	if (pthread_create(&pClient->xThread, NULL, FTOM_CLIENT_NET_masterProcess, pClient) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	if (pthread_create(&pClient->xSubscriber.xThread, NULL, FTOM_CLIENT_NET_subscribeProcess, pClient) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_stop
(
	FTOM_CLIENT_NET_PTR pClient
)
{
	ASSERT(pClient != NULL);

	pClient->bStop = FTM_TRUE;
	pthread_join(pClient->xSubscriber.xThread, NULL);
	pthread_join(pClient->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_CLIENT_NET_masterProcess
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	
	FTOM_CLIENT_NET_PTR		pClient = (FTOM_CLIENT_NET_PTR)pData;

	TRACE("Client started.\n");

	pClient->hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (pClient->hSock == -1)
	{
		ERROR("Faile to create socket.\n");	
		goto finish;
	}

	struct timeval tv = { .tv_sec = 0, .tv_usec = 2000000};
	if (setsockopt(pClient->hSock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
	{
   	 	ERROR("Failed to set socket timeout!\n");
	}
		
	pClient->bStop = FTM_FALSE;
	while(!pClient->bStop)
	{
		if (!pClient->bConnected)
		{
			ASSERT(pClient != NULL);
		
			struct sockaddr_in 	xServer;
		
			xServer.sin_family 		= AF_INET;
			xServer.sin_addr.s_addr	= inet_addr(pClient->xConfig.xServer.pHost);
			xServer.sin_port 		= htons(pClient->xConfig.xServer.usPort);
		
			if (connect(pClient->hSock, (struct sockaddr *)&xServer, sizeof(xServer)) == 0)
			{
				if (pClient->xCommon.fNotifyCB != NULL)
				{
					FTM_RET	xRet;
					FTOM_MSG_PTR	pMsg;
			
					xRet = FTOM_MSG_createConnectionStatus((FTM_ULONG)pClient, FTM_TRUE, &pMsg);
					if (xRet == FTM_RET_OK)
					{
						xRet =pClient->xCommon.fNotifyCB(pMsg, pClient->xCommon.pNotifyData);	
						if (xRet != FTM_RET_OK)
						{
							WARN("Failed to notify!\n");	
						}
			
						FTOM_MSG_destroy(&pMsg);
					}
				}	
			}
		
			pClient->bConnected = FTM_TRUE;
		}
		else
		{
			sleep(1);
		}
	}

	if (pClient->bConnected)
	{

		if (pClient->xCommon.fNotifyCB != NULL)
		{
			FTM_RET	xRet;
			FTOM_MSG_PTR	pMsg;
		
			xRet = FTOM_MSG_createConnectionStatus((FTM_ULONG)pClient, FTM_FALSE, &pMsg);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to create message!\n");	
			}
			else
			{
				xRet =pClient->xCommon.fNotifyCB(pMsg, pClient->xCommon.pNotifyData);	
				if (xRet != FTM_RET_OK)
				{
					WARN("Failed to notify!\n");	
				}
	
				FTOM_MSG_destroy(&pMsg);
			}
		}	

		pClient->bConnected = FTM_FALSE;
	}
	
finish:
	TRACE("Client stopped.\n");

	return	0;
}

FTM_VOID_PTR	FTOM_CLIENT_NET_subscribeProcess
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	
	FTOM_CLIENT_NET_PTR		pClient = (FTOM_CLIENT_NET_PTR)pData;
	FTM_ULONG				ulRespLen = 4096;
	FTOM_RESP_PARAMS_PTR 	pResp = NULL;

	pClient->xSubscriber.hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (pClient->xSubscriber.hSock == -1)
	{
		ERROR("Failed to create subscribe socket.\n");	
		return	0;	
	}

	pResp = (FTOM_RESP_PARAMS_PTR)FTM_MEM_malloc(4096);
	TRACE("Subscriber started.\n");

	struct timeval tv = { .tv_sec = 0, .tv_usec = 100000};
	if (setsockopt(pClient->xSubscriber.hSock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
	{
   	 	ERROR("Failed to set socket timeout!\n");
	}

	pClient->bStop = FTM_FALSE;
	while(!pClient->bStop)
	{
		if (!pClient->xSubscriber.bConnected)
		{
			struct sockaddr_in 	xServer;
		
			xServer.sin_family 		= AF_INET;
			xServer.sin_addr.s_addr	= inet_addr(pClient->xConfig.xPublishServer.pHost);
			xServer.sin_port 		= htons(pClient->xConfig.xPublishServer.usPort);
			
			TRACE("Subscriber connect to host[%s:%d]\n", pClient->xConfig.xPublishServer.pHost, pClient->xConfig.xPublishServer.usPort);
			if (connect(pClient->xSubscriber.hSock, (struct sockaddr *)&xServer, sizeof(xServer)) == 0)
			{
				pClient->xSubscriber.bConnected = FTM_TRUE;
			}	
			else
			{
				TRACE("Failed to subscriber connection!\n");
				sleep(1);	
			}
		}
		else
		{
			int	nLen = recv(pClient->xSubscriber.hSock, pResp, ulRespLen, 0);
			if (nLen > 0)
			{
				FTM_RET	xRet;
				FTOM_REQ_NOTIFY_PARAMS_PTR 	pReq = (FTOM_REQ_NOTIFY_PARAMS_PTR)pResp;
				FTOM_RESP_NOTIFY_PARAMS		xResp;

				if (pClient->xCommon.fNotifyCB != NULL)
				{
					xRet = pClient->xCommon.fNotifyCB(&pReq->xMsg, pClient->xCommon.pNotifyData);	
				}
				else
				{
					WARN("Notify CB not assigned!\n");
					xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
				}
			
				xResp.ulReqID 	= pReq->ulReqID;
				xResp.xCmd 		= pReq->xCmd;	
				xResp.ulLen		= sizeof(FTOM_RESP_NOTIFY_PARAMS);
				xResp.xRet 		= xRet;

				send(pClient->xSubscriber.hSock, &xResp, xResp.ulLen, 0);
			}
			else if ((nLen == 0) || (errno != EAGAIN))
			{
				close(pClient->xSubscriber.hSock);
				pClient->xSubscriber.bConnected = FTM_FALSE;
			}
		}
	}

	if (pResp != NULL)
	{
		FTM_MEM_free(pResp);
		pResp = NULL;
	}

	TRACE("Subscriber stopped.\n");

	return	0;
}

FTM_RET	FTOM_CLIENT_NET_loadConfigFromFile
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_CHAR_PTR 	pFileName
)
{
	return	FTM_RET_OK;
}

FTM_RET FTOM_CLIENT_NET_disconnect
(
	FTOM_CLIENT_NET_PTR		pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->bConnected)
	{
		if (pClient->hSock != -1)
		{
			TRACE("Socket close.\n");
			close(pClient->hSock);
			pClient->hSock = -1;
		}

		pClient->bConnected = FTM_FALSE;

		if (pClient->xCommon.fNotifyCB != NULL)
		{
			FTM_RET	xRet;
			FTOM_MSG_PTR	pMsg;
		
			xRet = FTOM_MSG_createConnectionStatus((FTM_ULONG)pClient, FTM_FALSE, &pMsg);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to create message!\n");	
			}
			else
			{
				xRet =pClient->xCommon.fNotifyCB(pMsg, pClient->xCommon.pNotifyData);	
				if (xRet != FTM_RET_OK)
				{
					WARN("Failed to notify!\n");	
				}
	
				FTOM_MSG_destroy(&pMsg);
			}
		}	
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CLIENT_NET_isConnected
(
	FTOM_CLIENT_NET_PTR		pClient,
	FTM_BOOL_PTR		pbConnected
)
{
	ASSERT(pClient != NULL);
	ASSERT(pbConnected != NULL);

	*pbConnected = pClient->bConnected;	

	return	FTM_RET_OK;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/

FTM_RET FTOM_CLIENT_NET_request
(
	FTOM_CLIENT_NET_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	ASSERT(pulRespLen != NULL);

	FTM_RET		xRet;
	FTM_INT		nRecvLen;

	pReq->ulReqID = ++pClient->ulReqID;

	if( send(pClient->hSock, pReq, ulReqLen, 0) < 0)
	{
		return	FTM_RET_COMM_FAILED_TO_TRANSMIT_PACKETS;
	}

	nRecvLen  = recv(pClient->hSock, pResp, ulRespLen, 0);
	if (nRecvLen > 0)
	{
		*pulRespLen = nRecvLen;

		xRet = FTM_RET_OK;
	}
	else if ((nRecvLen == 0) && (errno != EAGAIN))
	{
		close(pClient->hSock);
		pClient->bConnected = FTM_FALSE;
	
	}

	return	xRet;	
}


