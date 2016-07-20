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

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_CLIENT

typedef	struct
{
	sem_t					xLock;
	FTOM_REQ_PARAMS_PTR		pReq;
	FTM_ULONG				ulReqLen;
	FTOM_RESP_PARAMS_PTR	pResp;
	FTM_ULONG				ulRespMaxLen;
	FTM_ULONG				ulRespLen;
}	FTOM_CLIENT_NET_TRANS, _PTR_ FTOM_CLIENT_NET_TRANS_PTR;

static 
FTM_VOID_PTR FTOM_CLIENT_NET_threadMain
(
	FTM_VOID_PTR pData
);

static
FTM_RET	FTOM_CLIENT_NET_TRANS_create
(
	FTOM_CLIENT_NET_PTR		pClient,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespMaxLen,
	FTOM_CLIENT_NET_TRANS_PTR _PTR_ ppTrans
);

static
FTM_RET	FTOM_CLIENT_NET_TRANS_destroy
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_TRANS_PTR _PTR_ ppTrans
);

static
FTM_RET	FTOM_CLIENT_NET_TRANS_perform
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_TRANS_PTR	pTrans
);

static
FTM_RET	FTOM_CLIENT_NET_TRANS_waitForCompleted
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_TRANS_PTR	pTrans
);

static
FTM_INT FTOM_CLIENT_NET_TRANS_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pIndicator
);

FTM_RET	FTOM_CLIENT_NET_init
(
	FTOM_CLIENT_NET_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;

	memset(pClient, 0, sizeof(FTOM_CLIENT_NET));

	pClient->xCommon.fStart 			= (FTOM_CLIENT_START)FTOM_CLIENT_NET_start;
	pClient->xCommon.fStop 				= (FTOM_CLIENT_STOP)FTOM_CLIENT_NET_stop;
	pClient->xCommon.fLoadConfig		= (FTOM_CLIENT_LOAD_CONFIG)FTOM_CLIENT_NET_loadConfig;
	pClient->xCommon.fSetConfig 		= (FTOM_CLIENT_SET_CONFIG)FTOM_CLIENT_NET_setConfig;
	pClient->xCommon.fSetNotifyCB 		= NULL;
	pClient->xCommon.fRequest 			= (FTOM_CLIENT_REQUEST)FTOM_CLIENT_NET_request;	

	strcpy(pClient->xConfig.xServer.pHost, FTOM_DEFAULT_SERVER_IP);
	pClient->xConfig.xServer.usPort 	= FTOM_DEFAULT_SERVER_PORT;

	pClient->bStop 		= FTM_TRUE;
	pClient->bConnected = FTM_FALSE;
	pClient->hSock 		= -1;
	pClient->ulTimeout 	= FTOM_DEFAULT_SERVER_TIMEOUT;

	xRet = FTM_LIST_init(&pClient->xTransList);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Faeild to initialize transaction list!\n");
		goto finish;	
	}

	FTM_LIST_setSeeker(&pClient->xTransList , FTOM_CLIENT_NET_TRANS_seeker);

	xRet = FTM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Can't create message queue.[%08lx]\n", xRet);	
	}
	else
	{
		pClient->bInit = FTM_TRUE;
	}

finish:
	return	xRet;
}

FTM_RET	FTOM_CLIENT_NET_final
(
	FTOM_CLIENT_NET_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	if (!pClient->bInit)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	if (!pClient->bStop)
	{
		FTOM_CLIENT_NET_stop(pClient);	
	}

	xRet = FTM_LIST_final(&pClient->xTransList);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to finalize transaction list\n");
	}

	xRet = FTM_MSGQ_destroy(pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to finaizlie message queue.\n");
	}

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

FTM_RET	FTOM_CLIENT_NET_loadConfig
(
	FTOM_CLIENT_NET_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

    FTM_RET             xRet;
	FTM_CONFIG_ITEM     xClient;

	xRet = FTM_CONFIG_getItem(pConfig, "client", &xClient);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Client configuration not found!\n");
		goto finish;
	}    

	FTM_CONFIG_ITEM_getItemString(&xClient,	"host",	pClient->xConfig.xServer.pHost, FTM_URL_LEN);
	FTM_CONFIG_ITEM_getItemUSHORT(&xClient, "port",	&pClient->xConfig.xServer.usPort);

finish:
	return  xRet;
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

	if (pthread_create(&pClient->xThread, NULL, FTOM_CLIENT_NET_threadMain, pClient) < 0)
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
	pthread_join(pClient->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_CLIENT_NET_threadMain
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_CLIENT_NET_PTR		pClient = (FTOM_CLIENT_NET_PTR)pData;
	FTM_RET					xRet;
	struct timeval 			xTimeVal;
	FTOM_RESP_PARAMS_PTR	pRecvPkt;
	FTM_ULONG				ulRecvBuffLen = FTOM_DEFAULT_PACKET_SIZE;

	pRecvPkt = (FTOM_RESP_PARAMS_PTR)FTM_MEM_malloc(ulRecvBuffLen);
	if (pRecvPkt == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		goto finish;	
	}

	pClient->hSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (pClient->hSock == -1)
	{
		ERROR2(FTM_RET_ERROR, "Faile to create socket.\n");	
		goto finish;
	}

	xTimeVal.tv_sec = pClient->ulTimeout / 1000;
	xTimeVal.tv_usec= pClient->ulTimeout % 1000 * 1000;

	if (setsockopt(pClient->hSock, SOL_SOCKET, SO_RCVTIMEO,&xTimeVal, sizeof(xTimeVal)) < 0) 
	{
   	 	ERROR2(FTM_RET_ERROR, "Failed to set socket timeout!\n");
	}

	pClient->xServerAddr.sin_family		=AF_INET;
	pClient->xServerAddr.sin_addr.s_addr= inet_addr(pClient->xConfig.xServer.pHost);
	pClient->xServerAddr.sin_port 		= htons(pClient->xConfig.xServer.usPort);

	TRACE("Client started.\n");

	pClient->bStop = FTM_FALSE;
	while(!pClient->bStop)
	{
		if (!pClient->bConnected)
		{
			FTM_INT	nRet;
			struct sockaddr_in		xClientAddr;
			socklen_t				xClientAddrLen = sizeof(struct sockaddr_in);

			xClientAddr.sin_family 		= AF_INET;
			xClientAddr.sin_addr.s_addr	= INADDR_ANY;
			xClientAddr.sin_port 		= 0;

			nRet = bind(pClient->hSock, (struct sockaddr *)&xClientAddr, xClientAddrLen);
			if (nRet < 0)
			{
				xRet = FTM_RET_COMM_ERRNO | errno;
				ERROR2(xRet, "Failed to bind socket!\n");
				sleep(10);
			}
			else
			{
				getsockname(pClient->hSock, (struct sockaddr *)&xClientAddr, &xClientAddrLen);
				TRACE("Client bind success.[%s:%d]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));
				pClient->bConnected = FTM_TRUE;	
			}
		}

		while(pClient->bConnected)
		{
			struct	sockaddr_in	xRecvAddr;
			socklen_t	xAddrLen = sizeof(xRecvAddr);
			FTM_INT		nRecvLen;

			nRecvLen  = recvfrom(pClient->hSock, pRecvPkt, ulRecvBuffLen, 0, (struct sockaddr *)&xRecvAddr, (socklen_t *)&xAddrLen);
			if (nRecvLen > 0)
			{
				if ((pClient->xServerAddr.sin_addr.s_addr == xRecvAddr.sin_addr.s_addr) && (pClient->xServerAddr.sin_port  == xRecvAddr.sin_port))
				{
					FTOM_CLIENT_NET_TRANS_PTR	pTrans;

					xRet = FTM_LIST_get(&pClient->xTransList, &pRecvPkt->ulReqID, (FTM_VOID_PTR _PTR_)&pTrans);
					if (xRet == FTM_RET_OK)
					{
						if (pTrans->ulRespMaxLen >= nRecvLen)
						{
							memcpy(pTrans->pResp, pRecvPkt, nRecvLen);
							pTrans->ulRespLen = nRecvLen;
						}
						else
						{
							xRet= FTM_RET_BUFFER_TOO_SMALL;
							ERROR2(xRet, "Buffer too small[%lu:%lu]!\n", pTrans->ulRespMaxLen, nRecvLen);
						}

						sem_post(&pTrans->xLock);
					}
					else
					{
						FTOM_REQ_NOTIFY_PARAMS_PTR 	pReq = (FTOM_REQ_NOTIFY_PARAMS_PTR)pRecvPkt;
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

						send(pClient->hSock, &xResp, xResp.ulLen, 0);
					}
				}
				break;
			}
			else if (nRecvLen == 0)
			{	
				if (errno != EAGAIN)
				{
					close(pClient->hSock);
					pClient->bConnected = FTM_FALSE;
		
					xRet = FTM_RET_COMM_SOCKET_CLOSED;
					ERROR2(xRet, "Failed to send packet[errno = %d]!\n", errno);
					break;
				}	
				else
				{
					xRet = FTM_RET_TIMEOUT;
					ERROR2(xRet, "Response timeout!\n");
					break;
				}
			}
		}
	}

finish:
	TRACE("Client stopped.\n");

	return	0;
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
	FTOM_CLIENT_NET_TRANS_PTR	pTrans = NULL;

	xRet = FTOM_CLIENT_NET_TRANS_create(pClient, pReq, ulReqLen, pResp, ulRespLen, &pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create transaction!\n");
		goto finish;	
	}

	xRet = FTOM_CLIENT_NET_TRANS_perform(pClient, pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to perform transaction!\n");
		goto finish;	
	}


	xRet = FTOM_CLIENT_NET_TRANS_waitForCompleted(pClient, pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to complete transaction!\n");
		goto finish;
	}

	*pulRespLen = pTrans->ulRespLen;

finish:
	FTOM_CLIENT_NET_TRANS_destroy(pClient, &pTrans);

	return	xRet;	
}

FTM_RET	FTOM_CLIENT_NET_TRANS_create
(
	FTOM_CLIENT_NET_PTR		pClient,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespMaxLen,
	FTOM_CLIENT_NET_TRANS_PTR _PTR_ ppTrans
)
{
	ASSERT(ppTrans != NULL);
	FTM_RET	xRet;

	FTOM_CLIENT_NET_TRANS_PTR pTrans = (FTOM_CLIENT_NET_TRANS_PTR)FTM_MEM_malloc(sizeof(FTOM_CLIENT_NET_TRANS));
	if (pTrans == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");

		return	xRet;
	}

	if (sem_init(&pTrans->xLock, 0, 0) < 0)
	{
		xRet = FTM_RET_CANT_CREATE_SEMAPHORE;
		ERROR2(xRet,"Can't alloc semaphore!\n");

		return	xRet;
	}

	pReq->ulReqID = ++pClient->ulReqID;

	pTrans->pReq 		= pReq;
	pTrans->ulReqLen 	= ulReqLen;
	pTrans->pResp 		= pResp;
	pTrans->ulRespMaxLen= ulRespMaxLen;

	xRet = FTM_LIST_append(&pClient->xTransList, pTrans);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrans);
	}
	else
	{
		*ppTrans = pTrans;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_TRANS_perform
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_TRANS_PTR	pTrans
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrans != NULL);
	FTM_RET		xRet;
	FTM_INT		nSendLen;

	nSendLen = sendto(pClient->hSock, pTrans->pReq, pTrans->ulReqLen, 0, (struct sockaddr *)&pClient->xServerAddr, sizeof(pClient->xServerAddr) );
	if( nSendLen < 0)
	{
		xRet = FTM_RET_COMM_SEND_ERROR;
		ERROR2(xRet, "Failed to send packet[errno = %d]!\n", errno);
	}
	else if (nSendLen == 0)
	{
		xRet = FTM_RET_COMM_SOCKET_CLOSED;
		ERROR2(xRet, "Socket closed!\n");
	}
	else
	{
		xRet = FTM_RET_OK;	
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_NET_TRANS_waitForCompleted
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_TRANS_PTR	pTrans
)
{
	ASSERT(pTrans != NULL);

	struct timespec		xTimeout ;
	clock_gettime(CLOCK_REALTIME, &xTimeout);

	xTimeout.tv_sec += pClient->ulTimeout / 1000;
	if (sem_timedwait(&pTrans->xLock, &xTimeout) < 0)
	{
		return	FTM_RET_TIMEOUT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_NET_TRANS_destroy
(
	FTOM_CLIENT_NET_PTR			pClient,
	FTOM_CLIENT_NET_TRANS_PTR _PTR_ ppTrans
)
{
	ASSERT(pClient != NULL);
	ASSERT(ppTrans != NULL);

	FTM_RET	xRet;

	if (*ppTrans == NULL)
	{
		WARN("Transaction is NULL!\n");	
		return	FTM_RET_OK;
	}

	xRet = FTM_LIST_remove(&pClient->xTransList, (*ppTrans));
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove transaction from list!\n");
	}
	
	sem_destroy(&(*ppTrans)->xLock);
	FTM_MEM_free(*ppTrans);

	*ppTrans = NULL;

	return	FTM_RET_OK;
}

FTM_INT FTOM_CLIENT_NET_TRANS_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pIndicator
)
{
	FTOM_CLIENT_NET_TRANS_PTR	pTrans = (FTOM_CLIENT_NET_TRANS_PTR)pElement;
	FTM_ULONG_PTR				pulReqID = (FTM_ULONG_PTR)pIndicator;

	return	((pTrans->pReq != NULL) && (pTrans->pReq->ulReqID == *pulReqID));
}


