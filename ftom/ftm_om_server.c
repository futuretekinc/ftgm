#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "libconfig.h"
#include "ftm_om.h"
#include "ftm_om_params.h"
#include "ftm_om_node.h"
#include "ftm_om_ep.h"
#include "ftm_om_server.h"
#include "ftm_om_server_cmd.h"


#ifndef	FTM_OM_TRACE_IO
#define	FTM_OM_TRACE_IO		0
#endif

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTM_OM_SERVER_CALLBACK)FUN }

static FTM_VOID_PTR FTM_OM_SERVER_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTM_OM_SERVER_serviceHandler(FTM_VOID_PTR pData);

static FTM_RET	FTM_OM_SERVER_serviceCall
(
	FTM_OM_SESSION_PTR		pSession,
	FTM_OM_REQ_PARAMS_PTR		pReq,
	FTM_OM_RESP_PARAMS_PTR	pResp
);

static FTM_RET	FTM_OM_SERVER_NODE_create
(
	FTM_OM_SESSION_PTR					pSession,
	FTM_OM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_CREATE_PARAMS_PTR	pResp
);

static FTM_RET	FTM_OM_SERVER_NODE_destroy
(
	FTM_OM_SESSION_PTR					pSession,
	FTM_OM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTM_OM_RESP_NODE_DESTROY_PARAMS_PTR	presp
);

static FTM_RET	FTM_OM_SERVER_NODE_count
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_COUNT_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_NODE_get
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_GET_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_NODE_getAt
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
);

static FTM_RET	FTM_OM_SERVER_EP_create
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_CREATE_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_CREATE_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_EP_destroy
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_DESTROY_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_DESTROY_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_EP_count
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_COUNT_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_COUNT_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_EP_getList
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
);

static FTM_RET	FTM_OM_SERVER_EP_get
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_GET_PARAMS_PTR			pReq,
	FTM_OM_RESP_EP_GET_PARAMS_PTR			pResp
);

static FTM_RET	FTM_OM_SERVER_EP_getAt
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_GET_AT_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_GET_AT_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_EP_registrationNotifyReceiver
(
	FTM_OM_SESSION_PTR							pSession,
 	FTM_OM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
 	FTM_OM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp
);

static FTM_RET	FTM_OM_SERVER_EP_DATA_info
(
	FTM_OM_SESSION_PTR					pSession,
	FTM_OM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTM_OM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp
);

static FTM_RET	FTM_OM_SERVER_EP_DATA_count
(
	FTM_OM_SESSION_PTR						pSession,
 	FTM_OM_REQ_EP_DATA_COUNT_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_DATA_COUNT_PARAMS_PTR		pResp
);

static FTM_RET	FTM_OM_SERVER_EP_DATA_getLast
(
	FTM_OM_SESSION_PTR						pSession,
 	FTM_OM_REQ_EP_DATA_GET_LAST_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_DATA_GET_LAST_PARAMS_PTR	pResp
);

static FTM_RET	FTM_OM_SERVER_EP_DATA_getList
(
	FTM_OM_SESSION_PTR						pSession,
 	FTM_OM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp
);

static FTM_OM_SERVER_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTM_OM_CMD_NODE_CREATE,			FTM_OM_SERVER_NODE_create),
	MK_CMD_SET(FTM_OM_CMD_NODE_DESTROY,			FTM_OM_SERVER_NODE_destroy),
	MK_CMD_SET(FTM_OM_CMD_NODE_COUNT,				FTM_OM_SERVER_NODE_count),
	MK_CMD_SET(FTM_OM_CMD_NODE_GET,				FTM_OM_SERVER_NODE_get),
	MK_CMD_SET(FTM_OM_CMD_NODE_GET_AT,			FTM_OM_SERVER_NODE_getAt),
	MK_CMD_SET(FTM_OM_CMD_EP_CREATE,				FTM_OM_SERVER_EP_create),
	MK_CMD_SET(FTM_OM_CMD_EP_DESTROY,				FTM_OM_SERVER_EP_destroy),
	MK_CMD_SET(FTM_OM_CMD_EP_COUNT,				FTM_OM_SERVER_EP_count),
	MK_CMD_SET(FTM_OM_CMD_EP_GET_LIST,			FTM_OM_SERVER_EP_getList),
	MK_CMD_SET(FTM_OM_CMD_EP_GET,					FTM_OM_SERVER_EP_get),
	MK_CMD_SET(FTM_OM_CMD_EP_GET_AT,				FTM_OM_SERVER_EP_getAt),
	MK_CMD_SET(FTM_OM_CMD_EP_REG_NOTIFY_RECEIVER, FTM_OM_SERVER_EP_registrationNotifyReceiver),
	MK_CMD_SET(FTM_OM_CMD_EP_DATA_INFO,			FTM_OM_SERVER_EP_DATA_info),
	MK_CMD_SET(FTM_OM_CMD_EP_DATA_GET_LAST,		FTM_OM_SERVER_EP_DATA_getLast),
	MK_CMD_SET(FTM_OM_CMD_EP_DATA_GET_LIST,		FTM_OM_SERVER_EP_DATA_getList),
	MK_CMD_SET(FTM_OM_CMD_EP_DATA_COUNT,			FTM_OM_SERVER_EP_DATA_count),
	MK_CMD_SET(FTM_OM_CMD_UNKNOWN, 		NULL)
};

FTM_RET	FTM_OM_SERVER_init
(
	FTM_OM_SERVER_PTR	pServer,
	FTM_OM_PTR	pOM
)
{
	ASSERT(pServer != NULL);

	memset(pServer, 0, sizeof(FTM_OM_SERVER));

	memset(&pServer->xConfig, 0, sizeof(FTM_OM_SERVER_CONFIG));

	pServer->xConfig.usPort			= FTM_OM_DEFAULT_SERVER_PORT;
	pServer->xConfig.ulMaxSession	= FTM_OM_DEFAULT_SERVER_SESSION_COUNT	;

	pServer->pOM = pOM;
	FTM_LIST_init(&pServer->xSessionList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SERVER_final
(
	FTM_OM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTM_OM_SERVER_stop(pServer);

	FTM_LIST_final(&pServer->xSessionList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SERVER_start
(
	FTM_OM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTM_INT	nRet;

	nRet = pthread_create(&pServer->xPThread, NULL, FTM_OM_SERVER_process, (FTM_VOID_PTR)pServer);
	if (nRet != 0)
	{
		ERROR("Can't create thread[%d]\n", nRet);
		return	FTM_RET_CANT_CREATE_THREAD;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SERVER_stop
(
	FTM_OM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTM_VOID_PTR	pRes;

	FTM_OM_SESSION_PTR	pSession = NULL;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, &pRes);

		FTM_MEM_free(pSession);		
	}

	pthread_cancel(pServer->xPThread);
	pthread_join(pServer->xPThread, &pRes);

	TRACE("Server finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SERVER_notify
(
	FTM_OM_SERVER_PTR	pServer,
	FTM_OM_MSG_PTR 		pMsg
)
{
	ASSERT(pServer != NULL);
	ASSERT(pMsg != NULL);

	FTM_OM_SESSION_PTR	pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		FTM_OM_RESP_NOTIFY_PARAMS	xNotify;

		xNotify.ulReqID = 0;
		xNotify.xCmd	= 1;
		xNotify.ulLen	= sizeof(FTM_OM_RESP_NOTIFY_PARAMS);
		xNotify.nRet	= FTM_RET_OK;
		memcpy(&xNotify.xMsg, pMsg, sizeof(FTM_OM_MSG));

		//TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, xNotify.ulReqID, xNotify.ulLen);
		send(pSession->hSocket, &xNotify, sizeof(xNotify), MSG_DONTWAIT);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SERVER_setServiceCallback(FTM_OM_SERVER_PTR pServer, FTM_OM_SERVICE_ID xServiceID, FTM_OM_SERVICE_CALLBACK fServiceCB)
{
	ASSERT(pServer != NULL);
	ASSERT(fServiceCB != NULL);

	pServer->xServiceID = xServiceID;
	pServer->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_OM_SERVER_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_INT				nRet;
	FTM_INT				hSocket;
	struct sockaddr_in	xServerAddr, xClientAddr;
	FTM_OM_SERVER_PTR 	pServer = (FTM_OM_SERVER_PTR)pData;


	if (sem_init(&pServer->xLock, 0,pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR("Can't alloc semaphore!\n");
		return	0;	
	}

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	0;
	}

	xServerAddr.sin_family 		= AF_INET;
	xServerAddr.sin_addr.s_addr = INADDR_ANY;
	xServerAddr.sin_port 		= htons( pServer->xConfig.usPort );

	nRet = bind( hSocket, (struct sockaddr *)&xServerAddr, sizeof(xServerAddr));
	if (nRet < 0)
	{
		ERROR("bind failed.[nRet = %d]\n", nRet);
		return	0;
	}

	listen(hSocket, 3);


	while(FTM_TRUE)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrIulLen = sizeof(struct sockaddr_in);	
		struct timespec			xTimeout = { .tv_sec = 2, .tv_nsec = 0};

		if (sem_timedwait(&pServer->xLock, &xTimeout) == 0)
		{
			sem_getvalue(&pServer->xLock, &nValue);
			MESSAGE("Waiting for connections ...[%d]\n", nValue);
			hClient = accept(hSocket, (struct sockaddr *)&xClientAddr, (socklen_t *)&nSockAddrIulLen);
			if (hClient != 0)
			{
				TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));

				FTM_OM_SESSION_PTR pSession = (FTM_OM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTM_OM_SESSION));
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
					memcpy(&pSession->xPeer, &xClientAddr, sizeof(xClientAddr));
					pSession->pServer = pServer;
					if (pthread_create(&pSession->xPThread, NULL, FTM_OM_SERVER_serviceHandler, pSession) == 0)
					{
						FTM_LIST_append(&pServer->xSessionList, pSession);	
					}
				}
			
			}
		}
		else
		{
			TRACE("It has exceeded the allowed number of sessions.\n");
			shutdown(hSocket, SHUT_RD);
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_OM_SERVER_serviceHandler(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_OM_SESSION_PTR		pSession= (FTM_OM_SESSION_PTR)pData;
	FTM_OM_REQ_PARAMS_PTR		pReq 	= (FTM_OM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTM_OM_RESP_PARAMS_PTR	pResp 	= (FTM_OM_RESP_PARAMS_PTR)pSession->pRespBuff;
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
#if	FTM_OM_TRACE_IO
			//TRACE("RECV[%08lx:%08x] : Len = %lu\n", pSession->hSocket, pReq->ulReqID, ulLen);
#endif
			pResp->ulReqID = pReq->ulReqID;

			if (FTM_RET_OK != FTM_OM_SERVER_serviceCall(pSession, pReq, pResp))
			{
				pResp->xCmd = pReq->xCmd;
				pResp->nRet = FTM_RET_INTERNAL_ERROR;
				pResp->ulLen = sizeof(FTM_OM_RESP_PARAMS);
			}

#if	FTM_OM_TRACE_IO
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

FTM_RET	FTM_OM_SERVER_serviceCall
(
	FTM_OM_SESSION_PTR		pSession,
	FTM_OM_REQ_PARAMS_PTR		pReq,
	FTM_OM_RESP_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET				nRet;
	FTM_OM_SERVER_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTM_OM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
#if	FTM_OM_TRACE_IO
			TRACE("CMD : %s\n", pSet->pCmdString);
#endif
			nRet = pSet->fService(pSession, pReq, pResp);
#if	FTM_OM_TRACE_IO
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

FTM_RET	FTM_OM_SERVER_NODE_create
(
	FTM_OM_SESSION_PTR					pSession,
	FTM_OM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_CREATE_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_OM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_NODE_create(&pReq->xNodeInfo, &pNode);

	return	pResp->nRet;
}


FTM_RET	FTM_OM_SERVER_NODE_destroy
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTM_OM_RESP_NODE_DESTROY_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_OM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_NODE_get(pReq->pDID, &pNode);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nRet = FTM_OM_NODE_destroy(pNode);
	}

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_NODE_count
(
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_COUNT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_NODE_count(&pResp->ulCount);

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_NODE_get
(
	FTM_OM_SESSION_PTR				pSession,
 	FTM_OM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTM_OM_RESP_NODE_GET_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_OM_NODE_PTR	pNode;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_NODE_get(pReq->pDID, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_NODE_getAt
(	
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTM_OM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_OM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_NODE_getAt(pReq->ulIndex, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_create
(
	FTM_OM_SESSION_PTR				pSession,
 	FTM_OM_REQ_EP_CREATE_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_CREATE_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTM_OM_EP_PTR	pEP;

	xRet = FTM_OM_EPM_get(pSession->pServer->pOM->pEPM, pReq->xInfo.xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_RET_ALREADY_EXISTS;
	}
	else
	{
		xRet = FTM_OM_EPM_createEP(pSession->pServer->pOM->pEPM, &pReq->xInfo, &pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_OM_createEP(pSession->pServer->pOM, &pReq->xInfo);
		}
	}

	pResp->xCmd 	= pReq->xCmd;
	pResp->ulLen 	= sizeof(*pResp);
	pResp->nRet 	= xRet;

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_destroy
(		
	FTM_OM_SESSION_PTR				pSession,
 	FTM_OM_REQ_EP_DESTROY_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_DESTROY_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTM_OM_EP_PTR	pEP;

	xRet = FTM_OM_EPM_get(pSession->pServer->pOM->pEPM, pReq->xEPID, &pEP);
	
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_OM_EPM_destroyEP(pSession->pServer->pOM->pEPM, pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_OM_destroyEP(pSession->pServer->pOM, pReq->xEPID);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_count
(
	FTM_OM_SESSION_PTR				pSession,
 	FTM_OM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_COUNT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_EPM_count(pSession->pServer->pOM->pEPM, pReq->xType, &pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_get
(
	FTM_OM_SESSION_PTR				pSession,
 	FTM_OM_REQ_EP_GET_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_GET_PARAMS_PTR		pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_OM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_EPM_get(pSession->pServer->pOM->pEPM, pReq->xEPID, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_getList
(	
	FTM_OM_SESSION_PTR					pSession,
 	FTM_OM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTM_OM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->nRet = FTM_OM_EPM_getIDList(pSession->pServer->pOM->pEPM, pReq->xType, pResp->pEPIDList, pReq->ulMaxCount, &pResp->ulCount);
	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP_ID) * pResp->ulCount;

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_getAt
(
	FTM_OM_SESSION_PTR				pSession,
 	FTM_OM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_GET_AT_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_OM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_EPM_getAt(pSession->pServer->pOM->pEPM, pReq->ulIndex, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}
	
	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_registrationNotifyReceiver
(
	FTM_OM_SESSION_PTR							pSession,
 	FTM_OM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
 	FTM_OM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	FTM_RET_OK;
}



FTM_RET	FTM_OM_SERVER_EP_DATA_info
(
	FTM_OM_SESSION_PTR					pSession,
	FTM_OM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTM_OM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_getEPDataInfo(pSession->pServer->pOM, pReq->xEPID, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);

	return	pResp->nRet;
}
FTM_RET	FTM_OM_SERVER_EP_DATA_getLast
(	
	FTM_OM_SESSION_PTR						pSession,
	FTM_OM_REQ_EP_DATA_GET_LAST_PARAMS_PTR 	pReq,
	FTM_OM_RESP_EP_DATA_GET_LAST_PARAMS_PTR 	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTM_OM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_EPM_get(pSession->pServer->pOM->pEPM, pReq->xEPID, &pEP);
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

FTM_RET	FTM_OM_SERVER_EP_DATA_getList
(
	FTM_OM_SESSION_PTR						pSession,
 	FTM_OM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTM_OM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;

	xRet = FTM_OM_getEPDataList(pSession->pServer->pOM, pReq->xEPID, pReq->nStartIndex, pResp->pData, pReq->nCount, &pResp->nCount);
	if (xRet != FTM_RET_OK)
	{
		pResp->nCount = 0;
	}

	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP_DATA) * pResp->nCount;
	pResp->xCmd = pReq->xCmd;
	pResp->nRet = xRet;

	return	pResp->nRet;
}

FTM_RET	FTM_OM_SERVER_EP_DATA_count
(
	FTM_OM_SESSION_PTR					pSession,
	FTM_OM_REQ_EP_DATA_COUNT_PARAMS_PTR 	pReq,
	FTM_OM_RESP_EP_DATA_COUNT_PARAMS_PTR 	pResp
)
{
	ASSERT(pSession != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_ULONG	ulCount = 0;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTM_OM_getEPDataCount(pSession->pServer->pOM, pReq->xEPID, &ulCount);
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->ulCount = ulCount;
	}

	return	pResp->nRet;
}

FTM_RET FTM_OM_SERVER_loadFromFile
(
	FTM_OM_SERVER_PTR	pServer,
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

FTM_RET FTM_OM_SERVER_showConfig
(
	FTM_OM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "PORT", pServer->xConfig.usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pServer->xConfig.ulMaxSession);

	return	FTM_RET_OK;
}
