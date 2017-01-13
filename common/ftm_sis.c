#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftm_sis.h"

#undef	__MODULE__
#define	__MODULE__ 100
#define	FTM_TRACE_IO	1
typedef	struct
{
	struct sockaddr_in	xAddr;
}	FTM_SUBSCRIBER_INFO, _PTR_ FTM_SUBSCRIBER_INFO_PTR;

static 
FTM_VOID_PTR FTM_SIS_processTCP
(
	FTM_VOID_PTR 	pData
);

static 
FTM_VOID_PTR FTM_SIS_processTCPSession
(
	FTM_VOID_PTR 	pData
);

static
FTM_VOID_PTR FTM_SIS_processUDP
(
	FTM_VOID_PTR 	pData
);

static 
FTM_RET	FTM_SIS_process
(
	FTM_SIS_PTR			pSIS,
	FTM_SIM_REQ_PTR		pReq,
	FTM_ULONG			ulReqLen,
	FTM_SIM_RESP_PTR	pResp,
	FTM_ULONG			ulRespLen
);

static
FTM_RET	FTM_SIS_registerSubscribe
(
	FTM_SIS_PTR			pSIS,
	FTM_SIM_REQ_REGISTER_SUBSCRIBE_PTR	pReq,
	FTM_ULONG			ulReqLen,
	FTM_SIM_RESP_REGISTER_SUBSCRIBE_PTR	pResp,
	FTM_ULONG			ulRespLen
);

static 
FTM_BOOL	FTM_SIS_CMD_comparator
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
);

static 
FTM_BOOL	FTM_SIS_CMD_seeker
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
);

static
FTM_RET	FTM_SIS_SUBSCRIBER_add
(
	FTM_SIS_PTR	pSIS,
	FTM_CHAR_PTR	pIP,
	FTM_USHORT		usPort
);

FTM_BOOL	FTM_SIS_SUBSCRIBER_seeker
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
);

static FTM_SIS_CMD	pCmdSet[] =
{
	FTM_SIS_MAKE_CMD(FTM_SIM_TYPE_REGISTER_SUBSCRIBE,		FTM_SIS_registerSubscribe),

	FTM_SIS_MAKE_CMD(FTM_SIM_TYPE_UNKNOWN, 		NULL)
};

FTM_RET	FTM_SIS_create
(
	FTM_VOID_PTR	pParent,
	FTM_SIS_PTR _PTR_ 	ppSIS
)
{
	ASSERT(ppSIS != NULL);

	FTM_RET	xRet;
	FTM_SIS_PTR	pSIS;

	pSIS = (FTM_SIS_PTR)FTM_MEM_malloc(sizeof(FTM_SIS));
	if (pSIS == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_LIST_create(&pSIS->pList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create command list!\n");
		goto error;	
	}

	FTM_LIST_setSeeker(pSIS->pList, FTM_SIS_CMD_seeker);
	FTM_LIST_setComparator(pSIS->pList, FTM_SIS_CMD_comparator);
	
	xRet = FTM_SIS_init(pSIS, pParent);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize service interface server!\n");
		goto error;
	}

	*ppSIS = pSIS;

	return	FTM_RET_OK;

error:

	if (pSIS != NULL)
	{
		if (pSIS->pList != NULL)
		{
			FTM_LIST_destroy(pSIS->pList);	
		}
		FTM_MEM_free(pSIS);
	}

	return	xRet;
}
	
FTM_RET	FTM_SIS_destroy
(
	FTM_SIS_PTR _PTR_ 	ppSIS
)
{
	ASSERT(ppSIS != NULL);

	if ((*ppSIS)->pList != NULL)
	{
		FTM_LIST_destroy((*ppSIS)->pList);
	}

	FTM_SIS_final(*ppSIS);

	FTM_MEM_free(*ppSIS);

	*ppSIS = NULL;

	return	FTM_RET_OK;
}
	
FTM_RET	FTM_SIS_init
(
	FTM_SIS_PTR	pSIS,
	FTM_VOID_PTR	pParent
)
{
	ASSERT(pSIS != NULL);

	memset(&pSIS->xConfig, 0, sizeof(FTM_SIS_CONFIG));

	pSIS->pParent = pParent;

	pSIS->xConfig.xTCP.bEnabled		= FTM_TRUE;
	pSIS->xConfig.xTCP.usPort		= FTM_SIS_DEFAULT_PORT;
	pSIS->xConfig.xTCP.ulMaxSession	= FTM_SIS_DEFAULT_SESSION_COUNT	;
	FTM_LIST_init(&pSIS->xTCP.xSessionList);

	pSIS->xConfig.xUDP.bEnabled		= FTM_TRUE;
	pSIS->xConfig.xUDP.usPort		= FTM_SIS_DEFAULT_PORT;

	FTM_LIST_init(&pSIS->xUDP.xClientList);
	FTM_LIST_setSeeker(&pSIS->xUDP.xClientList, FTM_SIS_SUBSCRIBER_seeker);

	FTM_SIS_CMD_appendList(pSIS, pCmdSet, sizeof(pCmdSet) / sizeof(FTM_SIS_CMD));

	pSIS->bStop = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_final
(
	FTM_SIS_PTR	pSIS
)
{
	ASSERT(pSIS != NULL);

	FTM_SIS_stop(pSIS);

	FTM_LIST_final(&pSIS->xTCP.xSessionList);
	FTM_LIST_final(&pSIS->xUDP.xClientList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_start
(
	FTM_SIS_PTR	pSIS
)
{
	ASSERT(pSIS != NULL);

	FTM_INT	nRet;

	if (!pSIS->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	pSIS->bStop = FTM_FALSE;

	if (pSIS->xConfig.xTCP.bEnabled)
	{
		nRet = pthread_create(&pSIS->xTCP.xThread, NULL, FTM_SIS_processTCP, (FTM_VOID_PTR)pSIS);
		if (nRet != 0)
		{
			ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create Net interface[%d]\n", nRet);
		}
	}

	if (pSIS->xConfig.xUDP.bEnabled)
	{
		nRet = pthread_create(&pSIS->xUDP.xThread, NULL, FTM_SIS_processUDP, pSIS);
		if (nRet != 0)
		{
			ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create UDP interface[%d]\n", nRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_stop
(
	FTM_SIS_PTR	pSIS
)
{
	ASSERT(pSIS != NULL);

	FTM_SIS_SESSION_PTR	pSession = NULL;

	if (pSIS->bStop)
	{
		return	FTM_RET_NOT_START;
	}

	FTM_LIST_iteratorStart(&pSIS->xUDP.xClientList);
	while(FTM_LIST_iteratorNext(&pSIS->xUDP.xClientList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		FTM_MEM_free(pSession);		
	}

	FTM_LIST_iteratorStart(&pSIS->xTCP.xSessionList);
	while(FTM_LIST_iteratorNext(&pSIS->xTCP.xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, NULL);

		FTM_MEM_free(pSession);		
	}

	pSIS->bStop = FTM_TRUE;
	shutdown(pSIS->xTCP.hSocket, SHUT_RD);
	shutdown(pSIS->xUDP.hSocket, SHUT_RD);
	pthread_join(pSIS->xTCP.xThread, NULL);
	pthread_join(pSIS->xUDP.xThread, NULL);

	TRACE("Server finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_isRun
(
	FTM_SIS_PTR	pSIS,
	FTM_BOOL_PTR	pbRun
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pbRun != NULL);

	if ((pSIS != NULL) && (!pSIS->bStop))
	{
		*pbRun = FTM_TRUE;	
	}
	else
	{
		*pbRun = FTM_FALSE;	
	}

	return	FTM_RET_OK;
}

#if 0
FTM_RET	FTM_SIS_sendMessage
(
	FTM_SIS_PTR	pSIS,
	FTM_SIM_PTR 	pMsg
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pMsg != NULL);

	FTM_SUBSCRIBER_INFO_PTR	pSubscriber;
	FTM_REQ_NOTIFY_PTR	pPacket = NULL;
	FTM_ULONG					ulPacketLen;

	ulPacketLen = sizeof(FTM_REQ_NOTIFY) - sizeof(FTM_SIM) + pMsg->ulLen;
	pPacket = (FTM_REQ_NOTIFY_PTR)FTM_MEM_malloc(ulPacketLen);
	if (pPacket == NULL)
	{
		FTM_SIM_destroy(&pMsg);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pPacket->xCmd	= FTM_CMD_SERVER_NOTIFY;
	pPacket->ulLen	= ulPacketLen;
	memcpy(&pPacket->xMsg, pMsg, pMsg->ulLen);

	TRACE("Send Message[%08x]\n", pMsg->xType);
	FTM_LIST_iteratorStart(&pSIS->xUDP.xClientList);
	while(FTM_LIST_iteratorNext(&pSIS->xUDP.xClientList, (FTM_VOID_PTR _PTR_)&pSubscriber) == FTM_RET_OK)
	{
		FTM_INT		nSendLen = 0;

		pPacket->ulReqID = pSIS->ulReqID++;

		nSendLen = sendto(pSIS->xUDP.hSocket, pPacket, ulPacketLen, 0, (struct sockaddr *)&pSubscriber->xAddr, sizeof(pSubscriber->xAddr));
		if (nSendLen != ulPacketLen)
		{
			ERROR2(FTM_RET_ERROR, "Send failed!\n");	
		}
	}

	if (pPacket != NULL)
	{
		FTM_MEM_free(pPacket);	
	}

	FTM_SIM_destroy(&pMsg);

	return	FTM_RET_OK;
}
#endif
FTM_RET	FTM_SIS_setServiceCallback
(
	FTM_SIS_PTR		pSIS, 
	FTM_SIS_ID		xServiceID, 
	FTM_SIS_CALLBACK	fServiceCB
)
{
	ASSERT(pSIS != NULL);
	ASSERT(fServiceCB != NULL);

	pSIS->xServiceID = xServiceID;
	pSIS->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_SIS_processTCP
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_SIS_PTR 	pSIS = (FTM_SIS_PTR)pData;
	FTM_INT				nRet;
	struct sockaddr_in	xServerAddr, xClientAddr;

	
	if (sem_init(&pSIS->xTCP.xLock, 0,pSIS->xConfig.xTCP.ulMaxSession) < 0)
	{
		ERROR2(FTM_RET_CANT_CREATE_SEMAPHORE,"Can't alloc semaphore!\n");
		return	0;	
	}

	pSIS->xTCP.hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pSIS->xTCP.hSocket == -1)
	{
		ERROR2(FTM_RET_COMM_SOCK_ERROR, "Could not create socket\n");
		return	0;
	}

	xServerAddr.sin_family 		= AF_INET;
	xServerAddr.sin_addr.s_addr = INADDR_ANY;
	xServerAddr.sin_port 		= htons( pSIS->xConfig.xTCP.usPort );

	TRACE("Server[ %s:%d ]\n", inet_ntoa(xServerAddr.sin_addr), ntohs(xServerAddr.sin_port));
	nRet = bind( pSIS->xTCP.hSocket, (struct sockaddr *)&xServerAddr, sizeof(xServerAddr));
	if (nRet < 0)
	{
		ERROR2(FTM_RET_COMM_SOCKET_BIND_FAILED, "Failed to bind socket.[%s:%d]\n", inet_ntoa(xServerAddr.sin_addr), ntohs(xServerAddr.sin_port));
		return	0;
	}

	listen(pSIS->xTCP.hSocket, 3);

	while(!pSIS->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrIulLen = sizeof(struct sockaddr_in);	
		struct timespec			xTimeout ;

		clock_gettime(CLOCK_REALTIME, &xTimeout);

		xTimeout.tv_sec += 2;
		if (sem_timedwait(&pSIS->xTCP.xLock, &xTimeout) == 0)
		{
			sem_getvalue(&pSIS->xTCP.xLock, &nValue);
			TRACE("Waiting for connections ...[%d]\n", nValue);
			hClient = accept(pSIS->xTCP.hSocket, (struct sockaddr *)&xClientAddr, (socklen_t *)&nSockAddrIulLen);
			if (hClient > 0)
			{
				TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));

				FTM_SIS_SESSION_PTR pSession = (FTM_SIS_SESSION_PTR)FTM_MEM_malloc(sizeof(FTM_SIS_SESSION));
				if (pSession == NULL)
				{
					ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "System memory is not enough!\n");
					TRACE("The session[%08x] was closed.\n", hClient);
					close(hClient);
				}
				else
				{
					TRACE("The new session[%08x] has beed connected\n", hClient);

					pSession->hSocket = hClient;
					memcpy(&pSession->xPeer, &xClientAddr, sizeof(xClientAddr));
					pSession->pData = (FTM_VOID_PTR)pSIS;
					if (pthread_create(&pSession->xPThread, NULL, FTM_SIS_processTCPSession, pSession) == 0)
					{
						FTM_LIST_append(&pSIS->xTCP.xSessionList, pSession);	
					}
					else
					{
						FTM_MEM_free(pSession);
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_SIS_processUDP
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTM_INT				nRet;
	FTM_SIS_PTR 		pSIS = (FTM_SIS_PTR)pData;
	struct sockaddr_in	xServerAddr;
	FTM_BOOL			bStop = FTM_FALSE;
	FTM_SIM_REQ_PTR		pReq 	= NULL;
	FTM_SIM_RESP_PTR	pResp 	= NULL;
	FTM_ULONG			ulBuffLen = 4096;

	pReq = (FTM_SIM_REQ_PTR)FTM_MEM_malloc(ulBuffLen);
	if (pReq == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]\n", ulBuffLen);
		goto finish;
	}

	pResp = (FTM_SIM_RESP_PTR)FTM_MEM_malloc(ulBuffLen);
	if (pResp == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]\n", ulBuffLen);
		goto finish;
	}

	pSIS->xUDP.hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (pSIS->xUDP.hSocket == -1)
	{
		ERROR2(FTM_RET_COMM_SOCK_ERROR, "Could not create socket\n");
		goto finish;
	}

  	struct timeval tv = { .tv_sec = 0, .tv_usec = 100000};
	if (setsockopt(pSIS->xUDP.hSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
	{
		ERROR2(FTM_RET_ERROR, "Failed to set socket timeout!\n");
		goto finish;
	}

	xServerAddr.sin_family 		= AF_INET;
	xServerAddr.sin_addr.s_addr = INADDR_ANY;
	xServerAddr.sin_port 		= htons( pSIS->xConfig.xUDP.usPort );

	while(!pSIS->bStop)
	{
		nRet = bind( pSIS->xUDP.hSocket, (struct sockaddr *)&xServerAddr, sizeof(xServerAddr));
		if (nRet < 0)
		{
			xRet = FTM_RET_COMM_ERRNO | errno;
			ERROR2(xRet, "Failed to bind socket.[%s:%d]\n", inet_ntoa(xServerAddr.sin_addr), ntohs(xServerAddr.sin_port));
			goto finish;
		}

		bStop = FTM_FALSE;
		TRACE("UDP Server[%s:%d] started\n", inet_ntoa(xServerAddr.sin_addr), ntohs(xServerAddr.sin_port));

		while(!bStop)
		{
			struct sockaddr_in	xClientAddr;
	   		FTM_INT	nClientLen = sizeof(xClientAddr);
			FTM_INT	nReqLen, nSendLen;
	
			nReqLen =recvfrom(pSIS->xUDP.hSocket, pReq, ulBuffLen, 0, (struct sockaddr*)&xClientAddr, (socklen_t *)&nClientLen);
			if (nReqLen > 0)
			{
#if 0
				if ((pReq->xCmd & 0xFF000000) == 0)
				{
					switch(pReq->xCmd)
					{
					case	FTM_CMD_SET_NOTIFY:
						{
							FTM_REQ_SET_NOTIFY_PTR	pParams= (FTM_REQ_SET_NOTIFY_PTR)pReq;

							xRet = FTM_SIS_SUBSCRIBER_add(pSIS, pParams->pIP, pParams->usPort);
							if (xRet != FTM_RET_OK)
							{
								ERROR2(xRet, "Failed to add new subscriber[%s:%lu]!\n", pParams->pIP, pParams->usPort);	
							}
						}
						break;
					}
				}
				else
#endif
				{
					xRet = FTM_SIS_process(pSIS, pReq, nReqLen, pResp, ulBuffLen);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to call service!\n");
						pResp->xType= pReq->xType;
						pResp->xRet	= FTM_RET_INTERNAL_ERROR;
						pResp->ulLen= sizeof(FTM_SIM_RESP);
					}
	
					pResp->ulReqID = pReq->ulReqID;
	
					nSendLen = sendto(pSIS->xUDP.hSocket, pResp, pResp->ulLen, 0, (struct sockaddr*)&xClientAddr, sizeof(xClientAddr));
					if (nSendLen == 0)
					{
						bStop = FTM_TRUE;
					}
					else if (nSendLen < 0)
					{
						bStop = FTM_TRUE;
						xRet = FTM_RET_COMM_ERRNO | errno;
						ERROR2(xRet , "Failed to send packat!\n");
					}
					else
					{
						TRACE("send to [%s:%d]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));
					}
				}
			}
			else if (nReqLen == 0)
			{
				bStop = FTM_TRUE;
				xRet = FTM_RET_OK;
				TRACE("Socket closed!\n");
			}
			else if (nReqLen < -1)
			{
				bStop = FTM_TRUE;
				xRet = FTM_RET_COMM_ERRNO | errno;
				ERROR2(xRet , "Failed to receive packet[%d]!\n", nReqLen);
			}

			if (pSIS->bStop)
			{
				bStop = FTM_TRUE;	
			}
		}


		close(pSIS->xUDP.hSocket);
		pSIS->xUDP.hSocket = 0;
	}

	TRACE("UDP Server[%s:%d] stopped!\n", inet_ntoa(xServerAddr.sin_addr), ntohs(xServerAddr.sin_port));

finish:
	if (pReq != NULL)
	{
		FTM_MEM_free(pReq);	
		pReq = NULL;
	}

	if (pResp != NULL)
	{
		FTM_MEM_free(pResp);	
		pResp = NULL;
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_SIS_processTCPSession
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_SIS_SESSION_PTR		pSession= (FTM_SIS_SESSION_PTR)pData;
	FTM_SIM_REQ_PTR		pReq 	= (FTM_SIM_REQ_PTR)pSession->pReqBuff;
	FTM_SIM_RESP_PTR	pResp 	= (FTM_SIM_RESP_PTR)pSession->pRespBuff;

	pSession->bStop = FTM_FALSE;
	while(!pSession->bStop)
	{
		FTM_INT	ulReqLen;
		FTM_INT	ulSendLen;

		ulReqLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		if (ulReqLen == 0)
		{
			TRACE("The connection is terminated.\n");
			pSession->bStop = FTM_TRUE;
		}
		else if (ulReqLen == -1)
		{
			WARN("Error packet received.\n");	
		}
		else if (ulReqLen > 0)
		{
			FTM_ULONG	ulRetry = 3;
#if	FTM_TRACE_IO
			TRACE("RECV[%08lx:%08x] : Len = %lu\n", pSession->hSocket, pReq->ulReqID, ulReqLen);
#endif
			pResp->ulReqID = pReq->ulReqID;

			if (FTM_RET_OK != FTM_SIS_process((FTM_SIS_PTR)pSession->pData, pReq, ulReqLen, pResp, FTM_SIS_DEFAULT_PACKET_SIZE))
			{
				pResp->xType= pReq->xType;
				pResp->xRet	= FTM_RET_INTERNAL_ERROR;
				pResp->ulLen= sizeof(FTM_SIM_RESP);
			}

#if	FTM_TRACE_IO
			TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->ulReqID, pResp->ulLen);
#endif
			do
			{
				ulSendLen = send(pSession->hSocket, pResp, pResp->ulLen, MSG_DONTWAIT);
			}
			while ((--ulRetry > 0) && (ulSendLen < 0));

			if (ulSendLen < 0)
			{
				ERROR2(FTM_RET_ERROR, "send failed[%d]\n", -ulSendLen);	
				pSession->bStop = FTM_TRUE;
			}
		}
		else if (ulReqLen < 0)
		{
			ERROR2(FTM_RET_ERROR, "recv failed.[%d]\n", -ulReqLen);
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	FTM_LIST_remove(&((FTM_SIS_PTR)pSession->pData)->xTCP.xSessionList, pSession);	
	sem_post(&((FTM_SIS_PTR)pSession->pData)->xTCP.xLock);
	FTM_MEM_free(pSession);
	return	0;
}

FTM_RET	FTM_SIS_process
(
	FTM_SIS_PTR	pSIS,
	FTM_SIM_REQ_PTR		pReq,
	FTM_ULONG			ulReqLen,
	FTM_SIM_RESP_PTR		pResp,
	FTM_ULONG			ulRespLen
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET	xRet;
	FTM_SIS_CMD_PTR	pCmd;

	xRet = FTM_LIST_get(pSIS->pList, (FTM_VOID_PTR)pReq->xType, (FTM_VOID_PTR _PTR_)&pCmd);
	if (xRet == FTM_RET_OK)
	{
#if	FTM_TRACE_IO
		FTM_RET	xRet;

		TRACE("CMD : %s\n", pCmd->pName);
		xRet = 
#endif
		pCmd->fService(pSIS, pReq, ulReqLen, pResp, ulRespLen);
#if	FTM_TRACE_IO
		TRACE("RET : %08lx\n", xRet);
#endif
		return	FTM_RET_OK;
	}

	ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "FUNCTION NOT SUPPORTED\n");
	ERROR2(FTM_RET_INVALID_COMMAND,  "CMD : %08lx\n", pReq->xType);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

/****************************************************************************************************
 *
 ****************************************************************************************************/
FTM_RET	FTM_SIS_registerSubscribe
(
	FTM_SIS_PTR	pSIS,
	FTM_SIM_REQ_REGISTER_SUBSCRIBE_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTM_SIM_RESP_REGISTER_SUBSCRIBE_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET			xRet;

	TRACE("Received subscribe registration.\n");
	xRet = FTM_SIS_SUBSCRIBER_add(pSIS, pReq->pHost, pReq->usPort);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to register Subscriber.\n");
	}

	pResp->xType= pReq->xType;
	pResp->ulLen= sizeof(*pResp);
	pResp->xRet	= xRet;

	return	xRet;
}

FTM_RET FTM_SIS_CONFIG_load
(
	FTM_SIS_PTR	pSIS,
	FTM_CONFIG_ITEM_PTR	pConfig
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pConfig != NULL);

	FTM_CONFIG_ITEM_getItemULONG(pConfig,	"max_session", 	&pSIS->xConfig.xTCP.ulMaxSession);
	FTM_CONFIG_ITEM_getItemUSHORT(pConfig, 	"port", 		&pSIS->xConfig.xTCP.usPort);

	FTM_CONFIG_ITEM_getItemUSHORT(pConfig, 	"port", 		&pSIS->xConfig.xUDP.usPort);

	return	FTM_RET_OK;
}

FTM_RET FTM_SIS_CONFIG_save
(
	FTM_SIS_PTR	pSIS,
	FTM_CONFIG_ITEM_PTR	pConfig
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pConfig != NULL);

	FTM_CONFIG_ITEM_setItemULONG(pConfig,	"max_session", 	pSIS->xConfig.xTCP.ulMaxSession);
	FTM_CONFIG_ITEM_setItemUSHORT(pConfig, 	"port", 		pSIS->xConfig.xTCP.usPort);

	return	FTM_RET_OK;
}

FTM_RET FTM_SIS_CONFIG_show
(
	FTM_SIS_PTR	pSIS
)
{
	ASSERT(pSIS != NULL);

	MESSAGE("%16s : %s\n", "TCP", pSIS->xConfig.xTCP.bEnabled?"Active":"Inactive");
	MESSAGE("%16s : %d\n", "Port", pSIS->xConfig.xTCP.usPort);
	MESSAGE("%16s : %lu\n", "Max Session", pSIS->xConfig.xTCP.ulMaxSession);

	MESSAGE("%16s : %s\n", "UDP", pSIS->xConfig.xUDP.bEnabled?"Active":"Inactive");
	MESSAGE("%16s : %d\n", "Port", pSIS->xConfig.xUDP.usPort);
	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_CMD_append
(
	FTM_SIS_PTR		pSIS,
	FTM_SIS_CMD_PTR	pCmd
)
{
	ASSERT(pSIS != NULL);
	
	FTM_RET	xRet;
	FTM_SIS_CMD_PTR	pTempCmd = NULL;

	xRet = FTM_LIST_get(pSIS->pList, pCmd->pName, (FTM_VOID_PTR _PTR_)&pTempCmd);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXISTS;	
	}

	FTM_LIST_insert(pSIS->pList, pCmd, FTM_LIST_POS_ASSENDING);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_CMD_appendList
(
	FTM_SIS_PTR		pSIS,
	FTM_SIS_CMD_PTR	pCmdSet,
	FTM_ULONG		ulCount
)
{
	ASSERT(pSIS != NULL);
	FTM_RET		xRet;
	FTM_ULONG	i;

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_SIS_CMD_append(pSIS, &pCmdSet[i]	);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Already exist command!\n");	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIS_CMD_count
(
	FTM_SIS_PTR		pSIS,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pSIS->pList, pulCount);
}

FTM_RET	FTM_SIS_CMD_getAt
(
	FTM_SIS_PTR		pSIS,
	FTM_ULONG		ulIndex,
	FTM_SIS_CMD_PTR _PTR_ ppCmd
)
{
	ASSERT(pSIS != NULL);
	ASSERT(ppCmd != NULL);
	FTM_RET	xRet;
	FTM_SIS_CMD_PTR	pCmd;

	xRet = FTM_LIST_getAt(pSIS->pList, ulIndex, (FTM_VOID_PTR _PTR_)&pCmd);
	if (xRet == FTM_RET_OK)
	{
		*ppCmd = pCmd;	
	}

	return	xRet;
}

FTM_RET	FTM_SIS_CMD_print
(
	FTM_SIS_PTR		pSIS,
	FTM_SIM_TYPE	xType,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pSIS != NULL);
	FTM_RET	xRet;	
	FTM_SIS_CMD_PTR	pCmd;

	xRet = FTM_LIST_get(pSIS->pList, (FTM_VOID_PTR)xType, (FTM_VOID_PTR _PTR_)&pCmd);
	if (xRet == FTM_RET_OK)
	{
		snprintf(pBuff, ulBuffLen, "%s", pCmd->pName);
	}
	else
	{
		pBuff[0] = '\0';
	}

	return	xRet;
}

FTM_RET	FTM_SIS_SUBSCRIBER_add
(
	FTM_SIS_PTR	pSIS,
	FTM_CHAR_PTR	pIP,
	FTM_USHORT		usPort
)
{
	ASSERT(pSIS != NULL);
	ASSERT(pIP != NULL);

	FTM_RET	xRet;
	FTM_SUBSCRIBER_INFO_PTR	pInfo;
	struct sockaddr_in			xAddr;

	xAddr.sin_addr.s_addr  	= inet_addr(pIP);
	xAddr.sin_port			= htons(usPort);

	FTM_LIST_iteratorStart(&pSIS->xUDP.xClientList);
	while(FTM_LIST_iteratorNext(&pSIS->xUDP.xClientList, (FTM_VOID_PTR _PTR_)&pInfo) == FTM_RET_OK)
	{
		if ((pInfo->xAddr.sin_addr.s_addr == xAddr.sin_addr.s_addr) && (pInfo->xAddr.sin_port == xAddr.sin_port))
		{
			TRACE("The subscriber[%s:%lu] is exists.\n", pIP, usPort);
			return	FTM_RET_OK;	
		}
	}

	pInfo = (FTM_SUBSCRIBER_INFO_PTR)FTM_MEM_malloc(sizeof(FTM_SUBSCRIBER_INFO));
	if (pInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pInfo->xAddr.sin_addr.s_addr  	= inet_addr(pIP);
	pInfo->xAddr.sin_port			= htons(usPort);

	xRet = FTM_LIST_append(&pSIS->xUDP.xClientList, pInfo);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to append subscriber info!\n");	
		FTM_MEM_free(pInfo);
	}
	else
	{
		TRACE("New subscriber[%s:%lu]\n", pIP, usPort);
	}

	return	xRet;
}

FTM_BOOL	FTM_SIS_CMD_comparator
(
	const FTM_VOID_PTR	pElement1,
	const FTM_VOID_PTR	pElement2
)
{
	FTM_SIS_CMD_PTR	pCmd1 = (FTM_SIS_CMD_PTR)pElement1;
	FTM_SIS_CMD_PTR	pCmd2 = (FTM_SIS_CMD_PTR)pElement2;

	return	strcmp(pCmd1->pName, pCmd2->pName);
}

FTM_BOOL	FTM_SIS_CMD_seeker
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
)
{
	FTM_SIS_CMD_PTR	pCmd = (FTM_SIS_CMD_PTR)pElement;
	FTM_SIM_TYPE	xType = (FTM_SIM_TYPE)pKey;

	return	pCmd->xType == xType;
}

FTM_BOOL	FTM_SIS_SUBSCRIBER_seeker
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
)
{
	FTM_SUBSCRIBER_INFO_PTR	pInfo1 = (FTM_SUBSCRIBER_INFO_PTR)pElement;
	FTM_SUBSCRIBER_INFO_PTR	pInfo2 = (FTM_SUBSCRIBER_INFO_PTR)pKey;

	if ((pInfo1->xAddr.sin_addr.s_addr == pInfo2->xAddr.sin_addr.s_addr) && (pInfo1->xAddr.sin_port == pInfo2->xAddr.sin_port))
	{
		return	FTM_TRUE;	
	}

	return	FTM_RET_FALSE;
}
