#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "ftm.h"
#include "ftm_sim.h"
#include "ftm_sic.h"

#undef	__MODULE__
#define	__MODULE__ 100

typedef	struct
{
	sem_t				xLock;
	FTM_SIM_REQ_PTR		pReq;
	FTM_ULONG			ulReqLen;
	FTM_SIM_RESP_PTR	pResp;
	FTM_ULONG			ulRespMaxLen;
	FTM_ULONG			ulRespLen;
}	FTM_SIC_TRANS, _PTR_ FTM_SIC_TRANS_PTR;

static 
FTM_VOID_PTR FTM_SIC_threadMain
(
	FTM_VOID_PTR pData
);

static
FTM_RET FTM_SIC_notify
(
	FTM_SIC_PTR		pClient,
	FTM_SIM_PTR		pMsg
);

static
FTM_RET	FTM_SIC_TRANS_create
(
	FTM_SIC_PTR			pClient,
	FTM_SIM_REQ_PTR		pReq,
	FTM_ULONG			ulReqLen,
	FTM_SIM_RESP_PTR	pResp,
	FTM_ULONG			ulRespMaxLen,
	FTM_SIC_TRANS_PTR _PTR_ ppTrans
);

static
FTM_RET	FTM_SIC_TRANS_destroy
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_TRANS_PTR _PTR_ ppTrans
);

static
FTM_RET	FTM_SIC_TRANS_perform
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_TRANS_PTR	pTrans
);

static
FTM_RET	FTM_SIC_TRANS_waitForCompleted
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_TRANS_PTR	pTrans
);

static
FTM_INT FTM_SIC_TRANS_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pIndicator
);

static
FTM_RET	FTM_SIC_requestSubscriberRegistration
(
	FTM_SIC_PTR		pClient
);

FTM_RET	FTM_SIC_create
(
	FTM_SIC_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	FTM_RET	xRet;
	FTM_SIC_PTR	pClient;

	pClient = (FTM_SIC_PTR)FTM_MEM_malloc(sizeof(FTM_SIC));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create message queue\n");
		goto error;
	}

	xRet = FTM_SIC_init(pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize service interface client!\n");
		goto error;
	}
	
	*ppClient = pClient;
	

	return	FTM_RET_OK;

error:
	
	if (pClient != NULL)
	{
		if (pClient->pMsgQ != NULL)
		{
			FTM_MSGQ_destroy(pClient->pMsgQ);	
		}

		FTM_MEM_free(pClient);	
	}

	return	xRet;
}

FTM_RET	FTM_SIC_destroy
(
	FTM_SIC_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	
	FTM_SIC_final(*ppClient);

	if ((*ppClient)->pMsgQ != NULL)
	{
		FTM_MSGQ_destroy((*ppClient)->pMsgQ);
	}

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIC_init
(
	FTM_SIC_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;

	strcpy(pClient->xConfig.pHostName, "127.0.0.1");
	pClient->xConfig.usPort 	= FTM_SIC_DEFAULT_SERVER_PORT;

	pClient->bStop   	= FTM_TRUE;
	pClient->bConnected = FTM_FALSE;
	pClient->hSock 		= -1;
	pClient->ulTimeout 	= FTM_SIC_DEFAULT_SERVER_TIMEOUT;

	xRet = FTM_LIST_init(&pClient->xTransList);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Faeild to initialize transaction list!\n");
		goto finish;	
	}

	FTM_LIST_setSeeker(&pClient->xTransList, FTM_SIC_TRANS_seeker);

	pClient->bInit = FTM_TRUE;

finish:
	return	xRet;
}

FTM_RET	FTM_SIC_final
(
	FTM_SIC_PTR	pClient
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

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIC_CONFIG_load
(
	FTM_SIC_PTR	pClient,
	FTM_CONFIG_ITEM_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_CONFIG_ITEM_getItemString(pConfig,	"hostname",	pClient->xConfig.pHostName, FTM_URL_LEN);
	FTM_CONFIG_ITEM_getItemUSHORT(pConfig,	"port",		&pClient->xConfig.usPort);

	return  FTM_RET_OK;
}

FTM_RET	FTM_SIC_CONFIG_save
(
	FTM_SIC_PTR	pClient,
	FTM_CONFIG_ITEM_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_CONFIG_ITEM_setItemString(pConfig,	"hostname",	pClient->xConfig.pHostName);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to save configuration!\n");
		return	xRet;	
	}

	xRet = FTM_CONFIG_ITEM_setItemUSHORT(pConfig, "port",		pClient->xConfig.usPort);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to save configuration!\n");
	}


	return  xRet;
}

FTM_RET	FTM_SIC_CONFIG_show
(
	FTM_SIC_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	MESSAGE("%16s : %s\n",	"Host Name",pClient->xConfig.pHostName);
	MESSAGE("%16s : %hu\n", "Port",		pClient->xConfig.usPort);
	return	FTM_RET_OK;
}

FTM_RET	FTM_SIC_start
(
	FTM_SIC_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	pClient->bStop = FTM_FALSE;

	if (pthread_create(&pClient->xThreadMain, NULL, FTM_SIC_threadMain, pClient) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The client net task creation failed!\n");

		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIC_stop
(
	FTM_SIC_PTR pClient
)
{
	ASSERT(pClient != NULL);

	pClient->bStop = FTM_TRUE;

	return FTM_SIC_waitingForFinished(pClient);
}

FTM_RET	FTM_SIC_isRunning
(
	FTM_SIC_PTR pClient,
	FTM_BOOL_PTR		pIsRunning
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIsRunning != NULL);

	*pIsRunning = !pClient->bStop;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIC_waitingForFinished
(
	FTM_SIC_PTR 	pClient
)
{
	ASSERT(pClient != NULL);

	pthread_join(pClient->xThreadMain, NULL);
	pClient->xThreadMain = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIC_MESSAGE_send
(
	FTM_SIC_PTR	pClient,
	FTM_SIM_PTR	pSIM
)
{
	ASSERT(pClient != NULL);
	ASSERT(pSIM != NULL);
	FTM_RET	xRet;

	xRet = FTM_MSGQ_push(pClient->pMsgQ, pSIM);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Failed to push message!\n");
	}    

	return  xRet;
}

FTM_RET	FTM_SIC_MESSAGE_process
(
	FTM_SIC_PTR	pClient,
	FTM_SIM_PTR	pBaseMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pBaseMsg != NULL);


	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_SIC_threadMain
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_SIC_PTR			pClient = (FTM_SIC_PTR)pData;
	FTM_RET				xRet;
	struct timeval 		xTimeVal;
	FTM_SIM_RESP_PTR	pRecvPkt;
	FTM_ULONG			ulRecvBuffLen = FTM_SIC_DEFAULT_PACKET_SIZE;

	TRACE("The client net task was started!\n");
	pRecvPkt = (FTM_SIM_RESP_PTR)FTM_MEM_malloc(ulRecvBuffLen);
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
				FTM_SIM_PTR	pNewMsg;

				getsockname(pClient->hSock, (struct sockaddr *)&pClient->xLocalAddr, &xLocalAddrLen);
				TRACE("Client binds[%s:%d] succes.\n", inet_ntoa(pClient->xLocalAddr.sin_addr), ntohs(pClient->xLocalAddr.sin_port));
				pClient->bConnected = FTM_TRUE;	
		
		#if 0
				xRet = FTM_SIC_requestSubscriberRegistration(pClient);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to send request subscriber registration!\n");	
				}
#endif
				xRet = FTM_SIM_createNetStat((FTM_ULONG)pClient, FTM_TRUE, &pNewMsg);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to create message!\n");	
				}
				else
				{
					xRet = FTM_SIC_notify(pClient, pNewMsg);
					if (xRet != FTM_RET_OK)
					{
						FTM_SIM_destroy(&pNewMsg);
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
					FTM_SIC_TRANS_PTR	pTrans;

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
						switch(pRecvPkt->xType)
						{
#if 0
						case	FTOM_CMD_SERVER_NOTIFY:
							{
								FTM_SIM_REQ_NOTIFY_PTR 	pReq = (FTM_SIM_REQ_NOTIFY_PTR)pRecvPkt;
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
#endif
						default:
							{
								xRet = FTM_RET_INVALID_MESSAGE_TYPE;

								ERROR2(xRet, "Invalid message[%08x]\n", pRecvPkt->xType);
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
					FTM_SIM_PTR	pNewMsg;

					close(pClient->hSock);
					pClient->bConnected = FTM_FALSE;

					xRet = FTM_SIM_createNetStat((FTM_ULONG)pClient, FTM_FALSE, &pNewMsg);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to create message!\n");	
					}
					else
					{
						xRet = FTM_SIC_notify(pClient, pNewMsg);	
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to send message!\n");
							FTM_SIM_destroy(&pNewMsg);
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

FTM_RET FTM_SIC_disconnect
(
	FTM_SIC_PTR		pClient
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

		if (pClient->fNotifyCB != NULL)
		{
			FTM_RET		xRet;
			FTM_SIM_PTR	pSIM;
		
			xRet = FTM_SIM_createConnectionStatus((FTM_ULONG)pClient, FTM_FALSE, &pSIM);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to create message!\n");	
			}
			else
			{
				xRet =pClient->fNotifyCB(pSIM, pClient->pNotifyData);	
				if (xRet != FTM_RET_OK)
				{
					WARN2(xRet, "Failed to notify!\n");	
					FTM_SIM_destroy(&pSIM);
				}
			}
		}	
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTM_SIC_isConnected
(
	FTM_SIC_PTR		pClient,
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

FTM_RET FTM_SIC_request
(
	FTM_SIC_PTR		pClient, 
	FTM_SIM_REQ_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTM_SIM_RESP_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	ASSERT(pulRespLen != NULL);

	FTM_RET		xRet;
	FTM_SIC_TRANS_PTR	pTrans = NULL;

	xRet = FTM_SIC_TRANS_create(pClient, pReq, ulReqLen, pResp, ulRespLen, &pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create transaction!\n");
		goto finish;	
	}

	xRet = FTM_SIC_TRANS_perform(pClient, pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to perform transaction!\n");
		goto finish;	
	}


	xRet = FTM_SIC_TRANS_waitForCompleted(pClient, pTrans);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to complete transaction!\n");
		goto finish;
	}

	*pulRespLen = pTrans->ulRespLen;

finish:
	FTM_SIC_TRANS_destroy(pClient, &pTrans);

	return	xRet;	
}

FTM_RET	FTM_SIC_TRANS_create
(
	FTM_SIC_PTR		pClient,
	FTM_SIM_REQ_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTM_SIM_RESP_PTR	pResp,
	FTM_ULONG				ulRespMaxLen,
	FTM_SIC_TRANS_PTR _PTR_ ppTrans
)
{
	ASSERT(ppTrans != NULL);
	FTM_RET	xRet;

	FTM_SIC_TRANS_PTR pTrans = (FTM_SIC_TRANS_PTR)FTM_MEM_malloc(sizeof(FTM_SIC_TRANS));
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

	return	xRet;
}

FTM_RET	FTM_SIC_TRANS_perform
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_TRANS_PTR	pTrans
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

FTM_RET	FTM_SIC_TRANS_waitForCompleted
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_TRANS_PTR	pTrans
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

FTM_RET	FTM_SIC_TRANS_destroy
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_TRANS_PTR _PTR_ ppTrans
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

FTM_INT FTM_SIC_TRANS_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pIndicator
)
{
	FTM_SIC_TRANS_PTR	pTrans = (FTM_SIC_TRANS_PTR)pElement;
	FTM_ULONG_PTR				pulReqID = (FTM_ULONG_PTR)pIndicator;

	return	((pTrans->pReq != NULL) && (pTrans->pReq->ulReqID == *pulReqID));
}

FTM_RET	FTM_SIC_requestSubscriberRegistration
(
	FTM_SIC_PTR			pClient
)
{
	ASSERT(pClient != NULL);

	if(pClient->bConnected)
	{
		FTM_SIM_REQ_REGISTER_SUBSCRIBE	xParams;
		
		xParams.ulReqID = 0;
		xParams.xType	= FTM_SIM_TYPE_REGISTER_SUBSCRIBE;
		xParams.ulLen 	= sizeof(FTM_SIM_REQ_REGISTER_SUBSCRIBE);
		strcpy(xParams.pHost, inet_ntoa(pClient->xLocalAddr.sin_addr));
		xParams.usPort 	= ntohs(pClient->xLocalAddr.sin_port);

		sendto(pClient->hSock, &xParams, sizeof(xParams), 0, (struct sockaddr *)&pClient->xRemoteAddr, sizeof(pClient->xRemoteAddr) );
	}

	return	FTM_RET_OK;
}


FTM_RET	FTM_SIC_setNotifyCB
(
	FTM_SIC_PTR			pClient,
	FTM_SIC_CALLBACK	fNotifyCB,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pClient != NULL);

	pClient->fNotifyCB = fNotifyCB;
	pClient->pNotifyData = pData;

	return	FTM_RET_OK;
}

FTM_RET FTM_SIC_notify
(
	FTM_SIC_PTR		pClient,
	FTM_SIM_PTR		pBaseMsg
)
{
	ASSERT(pBaseMsg != NULL);

	FTM_RET             xRet;
	FTM_SIM_PTR	pNewMsg = NULL;

	if (pClient->fNotifyCB == NULL)
	{
		return	FTM_RET_NOTIFY_IS_NOT_SET;
	}

	xRet = FTM_SIM_copy(pBaseMsg, &pNewMsg);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Failed to copy message!\n");
		return  xRet;   
	}    

	xRet = pClient->fNotifyCB(pClient->pNotifyData, pNewMsg);
	if (xRet != FTM_RET_OK)
	{    
		ERROR2(xRet, "Failed to push message!\n");
		FTM_SIM_destroy(&pNewMsg); 
	}    

	return  xRet;
}
