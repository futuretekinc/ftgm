#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "ftm.h"
#include "ftom_net_client.h"
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
}	FTOM_NET_CLIENT_TRANS, _PTR_ FTOM_NET_CLIENT_TRANS_PTR;

static 
FTM_VOID_PTR FTOM_NET_CLIENT_threadMain
(
	FTM_VOID_PTR pData
);

static
FTM_RET FTOM_NET_CLIENT_notifyCB
(
	FTOM_MSG_PTR    pBaseMsg,
	FTM_VOID_PTR    pData
);

static
FTM_RET	FTOM_NET_CLIENT_TRANS_create
(
	FTOM_NET_CLIENT_PTR		pClient,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespMaxLen,
	FTOM_NET_CLIENT_TRANS_PTR _PTR_ ppTrans
);

static
FTM_RET	FTOM_NET_CLIENT_TRANS_destroy
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_NET_CLIENT_TRANS_PTR _PTR_ ppTrans
);

static
FTM_RET	FTOM_NET_CLIENT_TRANS_perform
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_NET_CLIENT_TRANS_PTR	pTrans
);

static
FTM_RET	FTOM_NET_CLIENT_TRANS_waitForCompleted
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_NET_CLIENT_TRANS_PTR	pTrans
);

static
FTM_INT FTOM_NET_CLIENT_TRANS_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pIndicator
);

static
FTM_RET	FTOM_NET_CLIENT_requestSubscriberRegistration
(
	FTOM_NET_CLIENT_PTR			pClient
);

static
FTOM_CLIENT_FUNCTION_SET	_xFunctionSet  =
{
	.fInit		= (FTOM_CLIENT_INIT)FTOM_NET_CLIENT_init,
	.fFinal		= (FTOM_CLIENT_FINAL)FTOM_NET_CLIENT_final,
	.fStart		= (FTOM_CLIENT_START)FTOM_NET_CLIENT_start,
	.fStop		= (FTOM_CLIENT_STOP)FTOM_NET_CLIENT_stop,

	.fMessageProcess=(FTOM_CLIENT_MESSAGE)FTOM_NET_CLIENT_MESSAGE_process,

	.fWaitingForFinished = (FTOM_CLIENT_WAITING_FOR_FINISHED)FTOM_NET_CLIENT_waitingForFinished,
	.fLoadConfig	= (FTOM_CLIENT_LOAD_CONFIG)FTOM_NET_CLIENT_CONFIG_load,
	.fSetNotifyCB	=(FTOM_CLIENT_SET_NOTIFY_CB)FTOM_NET_CLIENT_setNotifyCB,
	.fRequest		= (FTOM_CLIENT_REQUEST)FTOM_NET_CLIENT_request
};

FTM_RET	FTOM_NET_CLIENT_create
(
	FTM_CHAR_PTR	pName,
	FTOM_NET_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_NET_CLIENT_PTR	pClient;

	pClient = (FTOM_NET_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTOM_NET_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}
	
	memset(pClient, 0, sizeof(FTOM_NET_CLIENT));

	strncpy(pClient->xConfig.pName, pName, sizeof(pClient->xConfig.pName));

	FTOM_NET_CLIENT_init(pClient);

	*ppClient = pClient;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_destroy
(
	FTOM_NET_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	
	FTOM_NET_CLIENT_final(*ppClient);

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_init
(
	FTOM_NET_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;

	xRet = FTOM_CLIENT_internalInit((FTOM_CLIENT_PTR)pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialization!\n");
		return	xRet;	
	}

	FTOM_CLIENT_setFunctionSet((FTOM_CLIENT_PTR)pClient, &_xFunctionSet);

	xRet = FTOM_CLIENT_setNotifyInternalCB((FTOM_CLIENT_PTR)pClient, FTOM_NET_CLIENT_notifyCB, pClient);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pClient->xConfig.usPort 	= FTOM_DEFAULT_SERVER_PORT;

	pClient->bStop   	= FTM_TRUE;
	pClient->bConnected = FTM_FALSE;
	pClient->hSock 		= -1;
	pClient->ulTimeout 	= FTOM_DEFAULT_SERVER_TIMEOUT;

	xRet = FTM_LIST_init(&pClient->xTransList);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Faeild to initialize transaction list!\n");
		goto finish;	
	}

	FTM_LIST_setSeeker(&pClient->xTransList, FTOM_NET_CLIENT_TRANS_seeker);

	pClient->bInit = FTM_TRUE;

finish:
	return	xRet;
}

FTM_RET	FTOM_NET_CLIENT_final
(
	FTOM_NET_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	if (!pClient->bInit)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	xRet = FTM_LIST_final(&pClient->xTransList);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to finalize transaction list\n");
	}

	pClient->bInit = FTM_FALSE;

	return	FTOM_CLIENT_internalFinal((FTOM_CLIENT_PTR)pClient);
}

FTM_RET	FTOM_NET_CLIENT_CONFIG_load
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

    FTM_RET             xRet;
	FTM_CONFIG_ITEM     xClient;

	xRet = FTM_CONFIG_getItem(pConfig, pClient->xConfig.pName, &xClient);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Client configuration not found!\n");
		goto finish;
	}    

	FTM_CONFIG_ITEM_getItemString(&xClient,	"hostname",	pClient->xConfig.pHostName, FTM_URL_LEN);
	FTM_CONFIG_ITEM_getItemUSHORT(&xClient, "port",	&pClient->xConfig.usPort);

finish:
	return  xRet;
}

FTM_RET	FTOM_NET_CLIENT_CONFIG_save
(
	FTOM_NET_CLIENT_PTR	pClient,
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

	FTM_CONFIG_ITEM_setItemString(&xClient,	"hostname",	pClient->xConfig.pHostName);
	FTM_CONFIG_ITEM_setItemUSHORT(&xClient, "port",	pClient->xConfig.usPort);

finish:
	return  xRet;
}

FTM_RET	FTOM_NET_CLIENT_CONFIG_show
(
	FTOM_NET_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("[ Net Client ]\n");
	MESSAGE("%16s : %s\n",	"Host Name",pClient->xConfig.pHostName);
	MESSAGE("%16s : %hu\n", "Port",		pClient->xConfig.usPort);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_start
(
	FTOM_NET_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	pClient->bStop = FTM_FALSE;

	if (pthread_create(&pClient->xThreadMain, NULL, FTOM_NET_CLIENT_threadMain, pClient) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The client net task creation failed!\n");

		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_stop
(
	FTOM_NET_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	pClient->bStop = FTM_TRUE;

	return FTOM_NET_CLIENT_waitingForFinished(pClient);
}

FTM_RET	FTOM_NET_CLIENT_isRunning
(
	FTOM_NET_CLIENT_PTR pClient,
	FTM_BOOL_PTR		pIsRunning
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIsRunning != NULL);

	*pIsRunning = !pClient->bStop;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_waitingForFinished
(
	FTOM_NET_CLIENT_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	pthread_join(pClient->xThreadMain, NULL);
	pClient->xThreadMain = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_MESSAGE_send
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTOM_MSG_PTR		pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);
	FTM_RET	xRet;

	xRet = FTOM_CLIENT_sendMessage((FTOM_CLIENT_PTR)pClient, pMsg);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Failed to push message!\n");
	}    

	return  xRet;
}

FTM_RET	FTOM_NET_CLIENT_MESSAGE_process
(
	FTOM_NET_CLIENT_PTR	pClient,
	FTOM_MSG_PTR		pBaseMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pBaseMsg != NULL);

	FTM_RET		xRet;

	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_NET_STAT:
		{
			FTOM_MSG_NET_STAT_PTR	pMsg = (FTOM_MSG_NET_STAT_PTR)pBaseMsg;
			if (pMsg->bConnected)
			{
				TRACE("Server connected!\n");
				xRet = FTOM_NET_CLIENT_requestSubscriberRegistration(pClient);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to request subscriber registration!\n");	
				}
			}

		}
		break;

	default:	
		{
			if (pClient->xCommon.fNotifyCB != NULL)
			{
				xRet = pClient->xCommon.fNotifyCB(pBaseMsg, pClient->xCommon.pNotifyData);	
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to notify message processing!\n");	
				}
			}
			else
			{
				ERROR2(FTM_RET_INVALID_MESSAGE_TYPE, "Invalid Message Type[%08x]\n", pBaseMsg->xType);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_NET_CLIENT_threadMain
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_NET_CLIENT_PTR		pClient = (FTOM_NET_CLIENT_PTR)pData;
	FTM_RET					xRet;
	struct timeval 			xTimeVal;
	FTOM_RESP_PARAMS_PTR	pRecvPkt;
	FTM_ULONG				ulRecvBuffLen = FTOM_DEFAULT_PACKET_SIZE;

	TRACE("The client net task was started!\n");
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

	xTimeVal.tv_sec = 1;
	xTimeVal.tv_usec= 0;

	if (setsockopt(pClient->hSock, SOL_SOCKET, SO_RCVTIMEO,&xTimeVal, sizeof(xTimeVal)) < 0) 
	{
   	 	ERROR2(FTM_RET_ERROR, "Failed to set socket timeout!\n");
	}

	pClient->xRemoteAddr.sin_family		= AF_INET;
	pClient->xRemoteAddr.sin_addr.s_addr= inet_addr(pClient->xConfig.pHostName);
	pClient->xRemoteAddr.sin_port 		= htons(pClient->xConfig.usPort);

	pClient->xLocalAddr.sin_family		= AF_INET;
	pClient->xLocalAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	pClient->xLocalAddr.sin_port 		= 0;

	TRACE("Client started.\n");

	while(!pClient->bStop)
	{
		if (!pClient->bConnected)
		{
			FTM_INT	nRet;
			socklen_t		xLocalAddrLen = sizeof(struct sockaddr_in);

			nRet = bind(pClient->hSock, (struct sockaddr *)&pClient->xLocalAddr, xLocalAddrLen);
			if (nRet < 0)
			{
				xRet = FTM_RET_COMM_ERRNO | errno;
				ERROR2(xRet, "Failed to bind socket!\n");
				sleep(10);
			}
			else
			{
				FTOM_MSG_PTR	pNewMsg;

				getsockname(pClient->hSock, (struct sockaddr *)&pClient->xLocalAddr, &xLocalAddrLen);
				TRACE("Client binds[%s:%d] succes.\n", inet_ntoa(pClient->xLocalAddr.sin_addr), ntohs(pClient->xLocalAddr.sin_port));
				pClient->bConnected = FTM_TRUE;	
			
				FTOM_NET_CLIENT_requestSubscriberRegistration(pClient);

				if (pClient->fNotifyCB != NULL)
				{
					xRet = FTOM_MSG_createNetConnected(pClient, &pNewMsg);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to create message!\n");	
					}
					else
					{
						xRet = pClient->fNotifyCB(pNewMsg, pClient->pNotifyData);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to send message!\n");
							FTOM_MSG_destroy(&pNewMsg);
						}
					}
				}
	
			}
		}

		while((!pClient->bStop) && (pClient->bConnected))
		{
			struct	sockaddr_in	xRecvAddr;
			socklen_t	xAddrLen = sizeof(xRecvAddr);
			FTM_INT		nRecvLen;

			nRecvLen  = recvfrom(pClient->hSock, pRecvPkt, ulRecvBuffLen, 0, (struct sockaddr *)&xRecvAddr, (socklen_t *)&xAddrLen);
			if (nRecvLen > 0)
			{
				if (((xRecvAddr.sin_addr.s_addr == 0x0100007F) || (pClient->xRemoteAddr.sin_addr.s_addr == xRecvAddr.sin_addr.s_addr)) && (pClient->xRemoteAddr.sin_port  == xRecvAddr.sin_port))
				{
					FTOM_NET_CLIENT_TRANS_PTR	pTrans;

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
						switch(pRecvPkt->xCmd)
						{
						case	FTOM_CMD_SERVER_NOTIFY:
							{
								FTOM_REQ_NOTIFY_PARAMS_PTR 	pReq = (FTOM_REQ_NOTIFY_PARAMS_PTR)pRecvPkt;
								FTOM_MSG_PTR				pMsg;


								if (pClient->fNotifyCB != NULL)
								{
									xRet = FTOM_MSG_copy(&pReq->xMsg, &pMsg);
									if (xRet == FTM_RET_OK)
									{
										xRet = pClient->fNotifyCB(pMsg, pClient->pNotifyData);						
										if (xRet != FTM_RET_OK)
										{
											ERROR2(xRet, "Failed to send message!\n");
											FTOM_MSG_destroy(&pMsg);
										}
									}
								}
							}
							break;

						default:
							{
								xRet = FTM_RET_INVALID_MESSAGE_TYPE;

								ERROR2(xRet, "Invalid message[%08x]\n", pRecvPkt->xCmd);
							}
						}
					}
				}
				break;
			}
			else if (nRecvLen == 0)
			{	
				if (errno != EAGAIN)
				{
					FTOM_MSG_PTR	pNewMsg;

					close(pClient->hSock);
					pClient->bConnected = FTM_FALSE;
		
					if (pClient->fNotifyCB != NULL)
					{
						xRet = FTOM_MSG_createNetDisconnected(pClient, &pNewMsg);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to create message!\n");	
						}
						else
						{
							xRet = pClient->fNotifyCB(pNewMsg, pClient->pNotifyData);	
							if (xRet != FTM_RET_OK)
							{
								ERROR2(xRet, "Failed to send message!\n");
								FTOM_MSG_destroy(&pNewMsg);
							}
						}
					}
	
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

	if (pRecvPkt != NULL)
	{
		FTM_MEM_free(pRecvPkt);
	}

	TRACE("The client net task was stopped!\n");

	return	0;
}

FTM_RET FTOM_NET_CLIENT_disconnect
(
	FTOM_NET_CLIENT_PTR		pClient
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
		
			xRet = FTOM_MSG_createConnectionStatus(pClient, (FTM_ULONG)pClient, FTM_FALSE, &pMsg);
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

FTM_RET FTOM_NET_CLIENT_isConnected
(
	FTOM_NET_CLIENT_PTR		pClient,
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

FTM_RET FTOM_NET_CLIENT_request
(
	FTOM_NET_CLIENT_PTR		pClient, 
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
	FTOM_NET_CLIENT_TRANS_PTR	pTrans = NULL;

	xRet = FTOM_NET_CLIENT_TRANS_create(pClient, pReq, ulReqLen, pResp, ulRespLen, &pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create transaction!\n");
		goto finish;	
	}

	xRet = FTOM_NET_CLIENT_TRANS_perform(pClient, pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to perform transaction!\n");
		goto finish;	
	}


	xRet = FTOM_NET_CLIENT_TRANS_waitForCompleted(pClient, pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to complete transaction!\n");
		goto finish;
	}

	*pulRespLen = pTrans->ulRespLen;

finish:
	FTOM_NET_CLIENT_TRANS_destroy(pClient, &pTrans);

	return	xRet;	
}

FTM_RET	FTOM_NET_CLIENT_TRANS_create
(
	FTOM_NET_CLIENT_PTR		pClient,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespMaxLen,
	FTOM_NET_CLIENT_TRANS_PTR _PTR_ ppTrans
)
{
	ASSERT(ppTrans != NULL);
	FTM_RET	xRet;

	FTOM_NET_CLIENT_TRANS_PTR pTrans = (FTOM_NET_CLIENT_TRANS_PTR)FTM_MEM_malloc(sizeof(FTOM_NET_CLIENT_TRANS));
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

FTM_RET	FTOM_NET_CLIENT_TRANS_perform
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_NET_CLIENT_TRANS_PTR	pTrans
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrans != NULL);
	FTM_RET		xRet;
	FTM_INT		nSendLen;

	nSendLen = sendto(pClient->hSock, pTrans->pReq, pTrans->ulReqLen, 0, (struct sockaddr *)&pClient->xRemoteAddr, sizeof(pClient->xRemoteAddr) );
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

FTM_RET	FTOM_NET_CLIENT_TRANS_waitForCompleted
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_NET_CLIENT_TRANS_PTR	pTrans
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

FTM_RET	FTOM_NET_CLIENT_TRANS_destroy
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_NET_CLIENT_TRANS_PTR _PTR_ ppTrans
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

FTM_INT FTOM_NET_CLIENT_TRANS_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pIndicator
)
{
	FTOM_NET_CLIENT_TRANS_PTR	pTrans = (FTOM_NET_CLIENT_TRANS_PTR)pElement;
	FTM_ULONG_PTR				pulReqID = (FTM_ULONG_PTR)pIndicator;

	return	((pTrans->pReq != NULL) && (pTrans->pReq->ulReqID == *pulReqID));
}

FTM_RET	FTOM_NET_CLIENT_requestSubscriberRegistration
(
	FTOM_NET_CLIENT_PTR			pClient
)
{
	ASSERT(pClient != NULL);

	if(pClient->bConnected)
	{
		FTOM_REQ_REGISTER_SUBSCRIBE_PARAMS	xParams;
		
		xParams.ulReqID = 0;
		xParams.xCmd 	= FTOM_CMD_REGISTER_SUBSCRIBE;
		xParams.ulLen 	= sizeof(FTOM_REQ_REGISTER_SUBSCRIBE_PARAMS);
		strcpy(xParams.pHost, inet_ntoa(pClient->xLocalAddr.sin_addr));
		xParams.usPort 	= ntohs(pClient->xLocalAddr.sin_port);

		sendto(pClient->hSock, &xParams, sizeof(xParams), 0, (struct sockaddr *)&pClient->xRemoteAddr, sizeof(pClient->xRemoteAddr) );
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NET_CLIENT_setNotifyCB
(
	FTOM_NET_CLIENT_PTR			pClient,
	FTOM_CLIENT_NOTIFY_CB		fNotifyCB,
	FTM_VOID_PTR				pData
)
{
	ASSERT(pClient != NULL);

	pClient->fNotifyCB = fNotifyCB;
	pClient->pNotifyData = pData;

	return	FTM_RET_OK;
}

FTM_RET FTOM_NET_CLIENT_notifyCB
(
	FTOM_MSG_PTR    pBaseMsg,
	FTM_VOID_PTR    pData
)
{
	ASSERT(pBaseMsg != NULL);
	ASSERT(pData != NULL);

	FTM_RET             xRet;
	FTOM_NET_CLIENT_PTR  pClient = (FTOM_NET_CLIENT_PTR)pData;
	FTOM_MSG_PTR        pNewMsg = NULL;

	xRet = FTOM_MSG_copy(pBaseMsg, &pNewMsg);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Failed to copy message!\n");
		return  xRet;   
	}    

	xRet = FTOM_CLIENT_sendMessage((FTOM_CLIENT_PTR)pClient, pNewMsg);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Failed to push message!\n");
		FTOM_MSG_destroy(&pNewMsg); 
	}    

	return  xRet;
}

