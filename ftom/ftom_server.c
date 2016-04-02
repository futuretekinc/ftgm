#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "libconfig.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
#include "ftom_server.h"
#include "ftom_server_cmd.h"


#ifndef	FTOM_TRACE_IO
#define	FTOM_TRACE_IO		0
#endif

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTOM_SERVER_CALLBACK)FUN }

static FTM_VOID_PTR FTOM_SERVER_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTOM_SERVER_serviceHandler(FTM_VOID_PTR pData);

static FTM_RET	FTOM_SERVER_serviceCall
(
	FTOM_SESSION_PTR		pSession,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTOM_RESP_PARAMS_PTR	pResp
);

static FTM_RET	FTOM_SERVER_NODE_create
(
	FTOM_SESSION_PTR					pSession,
	FTOM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_CREATE_PARAMS_PTR	pResp
);

static FTM_RET	FTOM_SERVER_NODE_destroy
(
	FTOM_SESSION_PTR					pSession,
	FTOM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTOM_RESP_NODE_DESTROY_PARAMS_PTR	presp
);

static FTM_RET	FTOM_SERVER_NODE_count
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_COUNT_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_NODE_get
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_GET_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_NODE_getAt
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
);

static FTM_RET	FTOM_SERVER_EP_create
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_CREATE_PARAMS_PTR		pReq,
	FTOM_RESP_EP_CREATE_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_EP_destroy
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_DESTROY_PARAMS_PTR		pReq,
	FTOM_RESP_EP_DESTROY_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_EP_count
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_COUNT_PARAMS_PTR		pReq,
	FTOM_RESP_EP_COUNT_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_EP_getList
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
);

static FTM_RET	FTOM_SERVER_EP_get
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_GET_PARAMS_PTR			pReq,
	FTOM_RESP_EP_GET_PARAMS_PTR			pResp
);

static FTM_RET	FTOM_SERVER_EP_getAt
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_GET_AT_PARAMS_PTR		pReq,
	FTOM_RESP_EP_GET_AT_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_EP_registrationNotifyReceiver
(
	FTOM_SESSION_PTR							pSession,
 	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
 	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp
);

static FTM_RET	FTOM_SERVER_EP_DATA_info
(
	FTOM_SESSION_PTR					pSession,
	FTOM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp
);

static FTM_RET	FTOM_SERVER_EP_DATA_count
(
	FTOM_SESSION_PTR						pSession,
 	FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR		pReq,
	FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR		pResp
);

static FTM_RET	FTOM_SERVER_EP_DATA_getLast
(
	FTOM_SESSION_PTR						pSession,
 	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR	pReq,
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR	pResp
);

static FTM_RET	FTOM_SERVER_EP_DATA_getList
(
	FTOM_SESSION_PTR						pSession,
 	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp
);

static FTOM_SERVER_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTOM_CMD_NODE_CREATE,			FTOM_SERVER_NODE_create),
	MK_CMD_SET(FTOM_CMD_NODE_DESTROY,			FTOM_SERVER_NODE_destroy),
	MK_CMD_SET(FTOM_CMD_NODE_COUNT,				FTOM_SERVER_NODE_count),
	MK_CMD_SET(FTOM_CMD_NODE_GET,				FTOM_SERVER_NODE_get),
	MK_CMD_SET(FTOM_CMD_NODE_GET_AT,			FTOM_SERVER_NODE_getAt),
	MK_CMD_SET(FTOM_CMD_EP_CREATE,				FTOM_SERVER_EP_create),
	MK_CMD_SET(FTOM_CMD_EP_DESTROY,				FTOM_SERVER_EP_destroy),
	MK_CMD_SET(FTOM_CMD_EP_COUNT,				FTOM_SERVER_EP_count),
	MK_CMD_SET(FTOM_CMD_EP_GET_LIST,			FTOM_SERVER_EP_getList),
	MK_CMD_SET(FTOM_CMD_EP_GET,					FTOM_SERVER_EP_get),
	MK_CMD_SET(FTOM_CMD_EP_GET_AT,				FTOM_SERVER_EP_getAt),
	MK_CMD_SET(FTOM_CMD_EP_REG_NOTIFY_RECEIVER, FTOM_SERVER_EP_registrationNotifyReceiver),
	MK_CMD_SET(FTOM_CMD_EP_DATA_INFO,			FTOM_SERVER_EP_DATA_info),
	MK_CMD_SET(FTOM_CMD_EP_DATA_GET_LAST,		FTOM_SERVER_EP_DATA_getLast),
	MK_CMD_SET(FTOM_CMD_EP_DATA_GET_LIST,		FTOM_SERVER_EP_DATA_getList),
	MK_CMD_SET(FTOM_CMD_EP_DATA_COUNT,			FTOM_SERVER_EP_DATA_count),
	MK_CMD_SET(FTOM_CMD_UNKNOWN, 		NULL)
};

FTM_RET	FTOM_SERVER_init
(
	FTOM_SERVER_PTR	pServer,
	FTOM_PTR	pOM
)
{
	ASSERT(pServer != NULL);

	memset(pServer, 0, sizeof(FTOM_SERVER));

	memset(&pServer->xConfig, 0, sizeof(FTOM_SERVER_CONFIG));

	pServer->xConfig.usPort			= FTOM_DEFAULT_SERVER_PORT;
	pServer->xConfig.ulMaxSession	= FTOM_DEFAULT_SERVER_SESSION_COUNT	;

	pServer->pOM = pOM;
	pServer->bStop = FTM_TRUE;
	FTM_LIST_init(&pServer->xSessionList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_final
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTOM_SERVER_stop(pServer);

	FTM_LIST_final(&pServer->xSessionList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_start
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTM_INT	nRet;

	if (!pServer->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	nRet = pthread_create(&pServer->xPThread, NULL, FTOM_SERVER_process, (FTM_VOID_PTR)pServer);
	if (nRet != 0)
	{
		ERROR("Can't create thread[%d]\n", nRet);
		return	FTM_RET_CANT_CREATE_THREAD;
	}
	TRACE("xThread = %08x\n", pServer->xPThread);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_stop
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTOM_SESSION_PTR	pSession = NULL;

	if (pServer->bStop)
	{
		return	FTM_RET_NOT_START;
	}

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, NULL);

		FTM_MEM_free(pSession);		
	}

	pServer->bStop = FTM_TRUE;
	shutdown(pServer->hSocket, SHUT_RD);
	//pthread_cancel(pServer->xPThread);
	pthread_join(pServer->xPThread, NULL);

	TRACE("Server finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_notify
(
	FTOM_SERVER_PTR	pServer,
	FTOM_MSG_PTR 		pMsg
)
{
	ASSERT(pServer != NULL);
	ASSERT(pMsg != NULL);

	FTOM_SESSION_PTR	pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		FTOM_RESP_NOTIFY_PARAMS	xNotify;

		xNotify.ulReqID = 0;
		xNotify.xCmd	= 1;
		xNotify.ulLen	= sizeof(FTOM_RESP_NOTIFY_PARAMS);
		xNotify.nRet	= FTM_RET_OK;
		memcpy(&xNotify.xMsg, pMsg, sizeof(FTOM_MSG));

		//TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, xNotify.ulReqID, xNotify.ulLen);
		send(pSession->hSocket, &xNotify, sizeof(xNotify), MSG_DONTWAIT);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_setServiceCallback(FTOM_SERVER_PTR pServer, FTOM_SERVICE_ID xServiceID, FTOM_SERVICE_CALLBACK fServiceCB)
{
	ASSERT(pServer != NULL);
	ASSERT(fServiceCB != NULL);

	pServer->xServiceID = xServiceID;
	pServer->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_SERVER_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	FTM_INT				nRet;
	struct sockaddr_in	xServerAddr, xClientAddr;


	if (sem_init(&pServer->xLock, 0,pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR("Can't alloc semaphore!\n");
		return	0;	
	}

	pServer->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pServer->hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	0;
	}

	xServerAddr.sin_family 		= AF_INET;
	xServerAddr.sin_addr.s_addr = INADDR_ANY;
	xServerAddr.sin_port 		= htons( pServer->xConfig.usPort );

	nRet = bind( pServer->hSocket, (struct sockaddr *)&xServerAddr, sizeof(xServerAddr));
	if (nRet < 0)
	{
		ERROR("bind failed.[nRet = %d]\n", nRet);
		return	0;
	}

	listen(pServer->hSocket, 3);

	pServer->bStop = FTM_FALSE;
	
	while(!pServer->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrIulLen = sizeof(struct sockaddr_in);	
		struct timespec			xTimeout = { .tv_sec = 2, .tv_nsec = 0};

		if (sem_timedwait(&pServer->xLock, &xTimeout) == 0)
		{
			sem_getvalue(&pServer->xLock, &nValue);
			MESSAGE("Waiting for connections ...[%d]\n", nValue);
			hClient = accept(pServer->hSocket, (struct sockaddr *)&xClientAddr, (socklen_t *)&nSockAddrIulLen);
			if (hClient > 0)
			{
				TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));

				FTOM_SESSION_PTR pSession = (FTOM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTOM_SESSION));
				if (pSession == NULL)
				{
					ERROR("System memory is not enough!\n");
					TRACE("The session(%08x) was closed.\n", hClient);
					close(hClient);
				}
				else
				{
					TRACE("The new session(%08x) has beed connected\n", hClient);

					pSession->hSocket = hClient;
					memcpy(&pSession->xPeer, &xClientAddr, sizeof(xClientAddr));
					pSession->pServer = pServer;
					if (pthread_create(&pSession->xPThread, NULL, FTOM_SERVER_serviceHandler, pSession) == 0)
					{
						FTM_LIST_append(&pServer->xSessionList, pSession);	
					}
					else
					{
						FTM_MEM_free(pSession);
					}
				}
			}
		}
		else
		{
			TRACE("It has exceeded the allowed number of sessions.\n");
			shutdown(pServer->hSocket, SHUT_RD);
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_SERVER_serviceHandler(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTOM_SESSION_PTR		pSession= (FTOM_SESSION_PTR)pData;
	FTOM_REQ_PARAMS_PTR		pReq 	= (FTOM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTOM_RESP_PARAMS_PTR	pResp 	= (FTOM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timeval			xTimeval;

	xTimeval.tv_sec = 1;
	xTimeval.tv_usec = 0;

	pSession->bStop = FTM_FALSE;

	if (setsockopt(pSession->hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&xTimeval, sizeof(xTimeval)) != 0)
	{
		pSession->bStop = FTM_TRUE;
		ERROR("Timeout set failed.\n");
   		return 0;
	}

	while(!pSession->bStop)
	{
		int	ulLen;

		ulLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		if (ulLen == 0)
		{
			TRACE("The connection is terminated.\n");
			pSession->bStop = FTM_TRUE;
		}
		else if (ulLen == -1)
		{
				
		}
		else if (ulLen > 0)
		{
			FTM_ULONG	ulRetry = 3;
#if	FTOM_TRACE_IO
			//TRACE("RECV[%08lx:%08x] : Len = %lu\n", pSession->hSocket, pReq->ulReqID, ulLen);
#endif
			pResp->ulReqID = pReq->ulReqID;

			if (FTM_RET_OK != FTOM_SERVER_serviceCall(pSession, pReq, pResp))
			{
				pResp->xCmd = pReq->xCmd;
				pResp->nRet = FTM_RET_INTERNAL_ERROR;
				pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
			}

#if	FTOM_TRACE_IO
			//TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->ulReqID, pResp->ulLen);
#endif
			do
			{
				ulLen = send(pSession->hSocket, pResp, pResp->ulLen, MSG_DONTWAIT);
			}
			while ((--ulRetry > 0) && (ulLen < 0));

			if (ulLen < 0)
			{
				ERROR("send failed[%d]\n", -ulLen);	
				pSession->bStop = FTM_TRUE;
			}
		}
		else if (ulLen < 0)
		{
			ERROR("recv failed.[%d]\n", -ulLen);
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	FTM_LIST_remove(&pSession->pServer->xSessionList, pSession);	
	sem_post(&pSession->pServer->xLock);
	FTM_MEM_free(pSession);
	return	0;
}

FTM_RET	FTOM_SERVER_serviceCall
(
	FTOM_SESSION_PTR		pSession,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTOM_RESP_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET				nRet;
	FTOM_SERVER_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTOM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
#if	FTOM_TRACE_IO
			TRACE("CMD : %s\n", pSet->pCmdString);
#endif
			nRet = pSet->fService(pSession, pReq, pResp);
#if	FTOM_TRACE_IO
			TRACE("RET : %08lx\n", nRet);
#endif
			return	nRet;
		}

		pSet++;
	}

	ERROR("FUNCTION NOT SUPPORTED\n");
	ERROR("CMD : %08lx\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_SERVER_createNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_NODE_PTR		pInfo,
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);

	return	FTOM_createNode(pServer->pOM, pInfo, ppNode);
}

FTM_RET	FTOM_SERVER_destroyNode
(
	FTOM_SERVER_PTR		pServer,
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);
	
	return	FTOM_destroyNode(pServer->pOM, ppNode);
}

FTM_RET	FTOM_SERVER_countNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pServer != NULL);
	ASSERT(pulCount != NULL);
	
	return	FTOM_countNode(pServer->pOM, pulCount);
}

FTM_RET	FTOM_SERVER_getNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_CHAR			pDID[FTM_DID_LEN + 1],
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);
	
	return	FTOM_getNode(pServer->pOM, pDID, ppNode);
}

FTM_RET	FTOM_SERVER_getNodeAt
(
	FTOM_SERVER_PTR		pServer,
	FTM_ULONG			ulIndex,
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);
	
	return	FTOM_getNodeAt(pServer->pOM, ulIndex, ppNode);
}

/****************************************************************************************************
 *
 ****************************************************************************************************/
FTM_RET	FTOM_SERVER_NODE_create
(
	FTOM_SESSION_PTR					pSession,
	FTOM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_CREATE_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_SERVER_createNode(pSession->pServer, &pReq->xNodeInfo, &pNode);

	return	pResp->nRet;
}


FTM_RET	FTOM_SERVER_NODE_destroy
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTOM_RESP_NODE_DESTROY_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_SERVER_getNode(pSession->pServer, pReq->pDID, &pNode);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nRet = FTOM_NODE_destroy(&pNode);
	}

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_NODE_count
(
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_COUNT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_SERVER_countNode(pSession->pServer, &pResp->ulCount);

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_NODE_get
(
	FTOM_SESSION_PTR				pSession,
 	FTOM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTOM_RESP_NODE_GET_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_SERVER_getNode(pSession->pServer, pReq->pDID, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_NODE_getAt
(	
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTOM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_SERVER_getNodeAt(pSession->pServer, pReq->ulIndex, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_create
(
	FTOM_SESSION_PTR				pSession,
 	FTOM_REQ_EP_CREATE_PARAMS_PTR	pReq,
	FTOM_RESP_EP_CREATE_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EPM_getEP(pSession->pServer->pOM->pEPM, pReq->xInfo.xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_RET_ALREADY_EXISTS;
	}
	else
	{
		xRet = FTOM_EPM_createEP(pSession->pServer->pOM->pEPM, &pReq->xInfo, &pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_createEP(pSession->pServer->pOM, &pReq->xInfo);
		}
	}

	pResp->xCmd 	= pReq->xCmd;
	pResp->ulLen 	= sizeof(*pResp);
	pResp->nRet 	= xRet;

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_destroy
(		
	FTOM_SESSION_PTR				pSession,
 	FTOM_REQ_EP_DESTROY_PARAMS_PTR	pReq,
	FTOM_RESP_EP_DESTROY_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EPM_getEP(pSession->pServer->pOM->pEPM, pReq->xEPID, &pEP);
	
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EPM_destroyEP(pSession->pServer->pOM->pEPM, pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_destroyEP(pSession->pServer->pOM, pReq->xEPID);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_count
(
	FTOM_SESSION_PTR				pSession,
 	FTOM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTOM_RESP_EP_COUNT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_EPM_count(pSession->pServer->pOM->pEPM, pReq->xType, &pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_get
(
	FTOM_SESSION_PTR				pSession,
 	FTOM_REQ_EP_GET_PARAMS_PTR		pReq,
	FTOM_RESP_EP_GET_PARAMS_PTR		pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_EPM_getEP(pSession->pServer->pOM->pEPM, pReq->xEPID, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_getList
(	
	FTOM_SESSION_PTR					pSession,
 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->nRet = FTOM_EPM_getIDList(pSession->pServer->pOM->pEPM, pReq->xType, pResp->pEPIDList, pReq->ulMaxCount, &pResp->ulCount);
	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP_ID) * pResp->ulCount;

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_getAt
(
	FTOM_SESSION_PTR				pSession,
 	FTOM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTOM_RESP_EP_GET_AT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_EPM_getEPAt(pSession->pServer->pOM->pEPM, pReq->ulIndex, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}
	
	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_registrationNotifyReceiver
(
	FTOM_SESSION_PTR							pSession,
 	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
 	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	FTM_RET_OK;
}



FTM_RET	FTOM_SERVER_EP_DATA_info
(
	FTOM_SESSION_PTR					pSession,
	FTOM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_getEPDataInfo(pSession->pServer->pOM, pReq->xEPID, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);

	return	pResp->nRet;
}
FTM_RET	FTOM_SERVER_EP_DATA_getLast
(	
	FTOM_SESSION_PTR						pSession,
	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR 	pReq,
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR 	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_EPM_getEP(pSession->pServer->pOM->pEPM, pReq->xEPID, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		FTM_EP_DATA_PTR	pData;

		xRet = FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
		if (xRet != FTM_RET_OK)
		{	
			return	xRet;
		}
		memcpy(&pResp->xData, pData, sizeof(FTM_EP_DATA));
	}

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_getList
(
	FTOM_SESSION_PTR						pSession,
 	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;

	xRet = FTOM_getEPDataList(pSession->pServer->pOM, pReq->xEPID, pReq->nStartIndex, pResp->pData, pReq->nCount, &pResp->nCount);
	if (xRet != FTM_RET_OK)
	{
		pResp->nCount = 0;
	}

	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP_DATA) * pResp->nCount;
	pResp->xCmd = pReq->xCmd;
	pResp->nRet = xRet;

	return	pResp->nRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_count
(
	FTOM_SESSION_PTR					pSession,
	FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR 	pReq,
	FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR 	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_ULONG	ulCount = 0;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTOM_getEPDataCount(pSession->pServer->pOM, pReq->xEPID, &ulCount);
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->ulCount = ulCount;
	}

	return	pResp->nRet;
}

FTM_RET FTOM_SERVER_loadFromFile
(
	FTOM_SERVER_PTR	pServer,
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pServer != NULL);
	ASSERT(pFileName != NULL);

	config_t			xConfig;
	config_setting_t	*pSection;
	

	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "server");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "max_session");
		if (pField != NULL)
		{
			pServer->xConfig.ulMaxSession = (FTM_ULONG)config_setting_get_int(pField);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pServer->xConfig.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SERVER_showConfig
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "PORT", pServer->xConfig.usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pServer->xConfig.ulMaxSession);

	return	FTM_RET_OK;
}
