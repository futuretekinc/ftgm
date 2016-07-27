#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"
#include "ftdm_sqlite.h"

#undef	__MODULE__
#define	__MODULE__	FTDM_TRACE_MODULE_SERVER

typedef struct
{
	FTDM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTDM_SERVICE_CALLBACK	fService;
}	FTDMS_CMD_SET, _PTR_ FTDMS_CMD_SET_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTDM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTDMS_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTDMS_service(FTM_VOID_PTR pData);
static FTM_BOOL		FTDMS_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTDMS_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTDM_CMD_NODE_ADD,				FTDMS_NODE_add ),
	MK_CMD_SET(FTDM_CMD_NODE_DEL,				FTDMS_NODE_del ),
	MK_CMD_SET(FTDM_CMD_NODE_COUNT,				FTDMS_NODE_count ),
	MK_CMD_SET(FTDM_CMD_NODE_GET,				FTDMS_NODE_get ),
	MK_CMD_SET(FTDM_CMD_NODE_GET_AT,			FTDMS_NODE_getAt ),
	MK_CMD_SET(FTDM_CMD_NODE_SET,				FTDMS_NODE_set ),
	MK_CMD_SET(FTDM_CMD_NODE_GET_DID_LIST,		FTDMS_NODE_getDIDList ),
	MK_CMD_SET(FTDM_CMD_EP_ADD,					FTDMS_EP_add ),
	MK_CMD_SET(FTDM_CMD_EP_DEL,					FTDMS_EP_del ),
	MK_CMD_SET(FTDM_CMD_EP_COUNT,				FTDMS_EP_count ),
	MK_CMD_SET(FTDM_CMD_EP_GET,					FTDMS_EP_get ),
	MK_CMD_SET(FTDM_CMD_EP_GET_AT,				FTDMS_EP_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_SET,					FTDMS_EP_set ),
	MK_CMD_SET(FTDM_CMD_EP_GET_EPID_LIST,		FTDMS_EP_getEPIDList ),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_ADD,			FTDMS_EP_CLASS_add),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_DEL,			FTDMS_EP_CLASS_del),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_COUNT,			FTDMS_EP_CLASS_count ),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_GET,			FTDMS_EP_CLASS_get),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_GET_AT,		FTDMS_EP_CLASS_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_DATA_ADD,			FTDMS_EP_DATA_add ),
	MK_CMD_SET(FTDM_CMD_EP_DATA_INFO,			FTDMS_EP_DATA_info),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL,			FTDMS_EP_DATA_del),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL_WITH_TIME,	FTDMS_EP_DATA_delWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET,			FTDMS_EP_DATA_get),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET_WITH_TIME,	FTDMS_EP_DATA_getWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT,			FTDMS_EP_DATA_count),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT_WITH_TIME,FTDMS_EP_DATA_countWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_SET_LIMIT,		FTDMS_EP_DATA_setLimit),
	MK_CMD_SET(FTDM_CMD_TRIGGER_ADD,			FTDMS_TRIGGER_add ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_DEL,			FTDMS_TRIGGER_del ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_COUNT,			FTDMS_TRIGGER_count ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET,			FTDMS_TRIGGER_get ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET_AT,			FTDMS_TRIGGER_getAt ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_SET,			FTDMS_TRIGGER_set ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET_ID_LIST,	FTDMS_TRIGGER_getIDList ),
	MK_CMD_SET(FTDM_CMD_ACTION_ADD,				FTDMS_ACTION_add ),
	MK_CMD_SET(FTDM_CMD_ACTION_DEL,				FTDMS_ACTION_del ),
	MK_CMD_SET(FTDM_CMD_ACTION_COUNT,			FTDMS_ACTION_count ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET,				FTDMS_ACTION_get ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_AT,			FTDMS_ACTION_getAt ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_ID_LIST,		FTDMS_ACTION_getIDList ),
	MK_CMD_SET(FTDM_CMD_RULE_ADD,				FTDMS_RULE_add ),
	MK_CMD_SET(FTDM_CMD_RULE_DEL,				FTDMS_RULE_del ),
	MK_CMD_SET(FTDM_CMD_RULE_COUNT,				FTDMS_RULE_count ),
	MK_CMD_SET(FTDM_CMD_RULE_GET,				FTDMS_RULE_get ),
	MK_CMD_SET(FTDM_CMD_RULE_GET_AT,			FTDMS_RULE_getAt ),
	MK_CMD_SET(FTDM_CMD_RULE_GET_ID_LIST,		FTDMS_RULE_getIDList ),
	MK_CMD_SET(FTDM_CMD_LOG_ADD,				FTDMS_LOG_add ),
	MK_CMD_SET(FTDM_CMD_LOG_DEL,				FTDMS_LOG_del ),
	MK_CMD_SET(FTDM_CMD_LOG_COUNT,				FTDMS_LOG_count ),
	MK_CMD_SET(FTDM_CMD_LOG_GET,				FTDMS_LOG_get ),
	MK_CMD_SET(FTDM_CMD_LOG_GET_AT,				FTDMS_LOG_getAt ),
	MK_CMD_SET(FTDM_CMD_UNKNOWN, 				NULL)
};


FTM_RET	FTDMS_init
(
	FTDM_SERVER_PTR			pServer,
	FTDM_CONTEXT_PTR		pDM
)
{
	ASSERT(pServer != NULL);

	pServer->pDM = pDM;
	FTM_LIST_init(&pServer->xSessionList);
	FTM_LIST_setSeeker(&pServer->xSessionList, FTDMS_SESSION_LIST_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_final
(
	FTDM_SERVER_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_loadConfig
(
	FTDM_SERVER_PTR			pServer,
	FTDM_CFG_SERVER_PTR		pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pServer->xConfig, pConfig, sizeof(FTDM_CFG_SERVER));

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_loadFromFile
(
	FTDM_SERVER_PTR			pServer,
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pServer != NULL);
	ASSERT(pFileName != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_start
(
	FTDM_SERVER_PTR pServer
)
{
	ASSERT(pServer != NULL);

	if (pthread_create(&pServer->xThread, NULL, FTDMS_process, (void *)pServer) < 0)
	{
		ERROR2(FTM_RET_THREAD_CREATION_ERROR, "Failed to create pthread.");
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_stop
(
	FTDM_SERVER_PTR 		pServer
)
{
	ASSERT(pServer != NULL);

	pServer->bStop = FTM_TRUE;
	close(pServer->hSocket);

	return	FTDMS_waitingForFinished(pServer);
}

FTM_RET	FTDMS_waitingForFinished
(
	FTDM_SERVER_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	pthread_join(pServer->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDMS_process(FTM_VOID_PTR pData)
{
	FTM_INT				nRet;
	struct sockaddr_in	xServer, xClient;
	FTDM_SERVER_PTR		pServer =(FTDM_SERVER_PTR)pData;
	time_t				xPrevTime, xCurrentTime;

	ASSERT(pData != NULL);

	if (sem_init(&pServer->xSemaphore, 0, pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Can't alloc semaphore!\n");
		goto error;
	}

	pServer->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pServer->hSocket == -1)
	{
		ERROR2(FTM_RET_COMM_SOCK_ERROR, "Could not create socket\n");
		goto error;
	}

 	if( fcntl(pServer->hSocket, F_SETFL, O_NONBLOCK) == -1 )
	{
       ERROR2(FTM_RET_COMM_CTRL_ERROR, "Listen socket nonblocking\n");
       goto error;
	}
				 

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( pServer->xConfig.usPort );

	nRet = bind( pServer->hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR2(FTM_RET_COMM_SOCKET_BIND_FAILED, "bind failed.[nRet = %d]\n", nRet);
		goto error;
	}

	listen(pServer->hSocket, 3);


	xPrevTime = time(NULL) / 3600;
	while(!pServer->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nSockAddrInLen = sizeof(struct sockaddr_in);	

		hClient = accept(pServer->hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient > 0)
		{
			FTM_RET	xRet;

			TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClient.sin_addr), ntohs(xClient.sin_port));

			
			FTDM_SESSION_PTR pSession = NULL;
			
			xRet = FTDMS_createSession(pServer, hClient, (struct sockaddr *)&xClient, &pSession);
			if (xRet != FTM_RET_OK)
			{
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				FTM_INT	nRet;

				nRet = pthread_create(&pSession->xThread, NULL, FTDMS_service, pSession);
				if (nRet != 0)
				{
					ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create a thread[%d]\n", nRet);
					FTDMS_destroySession(pServer, &pSession);
				}
			}
		}
		xCurrentTime = time(NULL) / 3600; 
		if (xPrevTime != xCurrentTime)
		{
			FTDM_removeInvalidData(pServer->pDM);	
		}

		xPrevTime = xCurrentTime;
		usleep(10000);
	}

	FTDM_SESSION_PTR pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pSession->bStop = FTM_TRUE;
		shutdown(pSession->hSocket, SHUT_RD);
		pthread_join(pSession->xThread, 0);

		FTDMS_destroySession(pServer, &pSession);
	}

error:

	return	0;
}

FTM_VOID_PTR FTDMS_service(FTM_VOID_PTR pData)
{
	FTDM_SERVER_PTR			pServer;
	FTDM_SESSION_PTR		pSession= (FTDM_SESSION_PTR)pData;
	FTDM_REQ_PARAMS_PTR		pReq 	= (FTDM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTDM_RESP_PARAMS_PTR	pResp 	= (FTDM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timespec			xTimeout;

	pServer = pSession->pServer;

	clock_gettime(CLOCK_REALTIME, &xTimeout);
	xTimeout.tv_sec += 2;
	if (sem_timedwait(&pSession->pServer->xSemaphore, &xTimeout) < 0)
	{
		TRACE("The session(%08x) was closed\n", pSession->hSocket);
		shutdown(pSession->hSocket, SHUT_RD);
		return	0;	
	}

	while(!pSession->bStop)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, pSession->ulReqBufferLen, 0);
		if (nLen == 0)
		{
			TRACE("The connection is terminated.\n");
			break;	
		}
		else if (nLen < 0)
		{
			ERROR2(FTM_RET_COMM_RECV_ERROR, "recv failed[%d]\n", -nLen);
			break;	
		}

		FTM_TIME_getCurrent(&pSession->xLastTime);

		pResp->nLen = pSession->ulRespBufferLen;
		if (FTM_RET_OK != FTDMS_serviceCall(pSession->pServer, pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->xRet = FTM_RET_INTERNAL_ERROR;
			pResp->nLen = sizeof(FTDM_RESP_PARAMS);
		}

		nLen = send(pSession->hSocket, pResp, pResp->nLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			ERROR2(FTM_RET_COMM_SEND_ERROR, "send failed[%d]\n", -nLen);	
			break;
		}
	}

	FTDMS_destroySession(pServer, &pSession);

	sem_post(&pServer->xSemaphore);

	return	0;
}

FTM_RET	FTDMS_serviceCall
(
	FTDM_SERVER_PTR			pServer,
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
)
{
	FTM_RET				xRet;
	FTDMS_CMD_SET_PTR	pSet = pCmdSet;
	
	while(pSet->xCmd != FTDM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
			xRet = pSet->fService(pServer, pReq, pResp);

			return	xRet;
		}

		pSet++;
	}

	ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Function[%d] not supported.\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_BOOL	FTDMS_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTDM_SESSION_PTR	pSession = (FTDM_SESSION_PTR)pElement;
	pthread_t 			*pPThread = (pthread_t *)pIndicator;	

	return (pSession->xThread == *pPThread);
}

FTM_RET	FTDMS_createSession
(
	FTDM_SERVER_PTR pServer,
	FTM_INT			hClient,
	struct sockaddr *pSockAddr,
	FTDM_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(pServer != NULL);
	ASSERT(pSockAddr != NULL);
	ASSERT(ppSession != NULL);
	
	FTM_RET	xRet;

	FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTDM_SESSION));
	if (pSession == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		goto error;

	}

	pSession->ulReqBufferLen = pServer->xConfig.ulBufferLen;
	pSession->pReqBuff = (FTM_BYTE_PTR)FTM_MEM_malloc(pServer->xConfig.ulBufferLen);
	if (pSession->pReqBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		goto error;
	}

	pSession->ulRespBufferLen = pServer->xConfig.ulBufferLen;
	pSession->pRespBuff = (FTM_BYTE_PTR)FTM_MEM_malloc(pServer->xConfig.ulBufferLen);
	if (pSession->pRespBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		goto error;
	}

	pSession->pServer = pServer;
	pSession->hSocket = hClient;
	memcpy(&pSession->xPeer, pSockAddr, sizeof(struct sockaddr));

	if (sem_init(&pSession->xSemaphore, 0, 1) < 0)
	{
		FTM_MEM_free(pSession);
		ERROR2(FTM_RET_CANT_CREATE_SEMAPHORE, "Can't alloc semaphore!\n");

		return	FTM_RET_CANT_CREATE_SEMAPHORE;
	}

	xRet = FTM_LIST_append(&pServer->xSessionList, pSession);	
	if (xRet != FTM_RET_OK)
	{
		sem_destroy(&pSession->xSemaphore);	
		FTM_MEM_free(pSession);

		return	FTM_RET_LIST_NOT_INSERTABLE;
	}

	FTM_TIME_getCurrent(&pSession->xStartTime);

	*ppSession = pSession;
	
	return	FTM_RET_OK;

error:

	if (pSession != NULL)
	{
		if (pSession->pReqBuff != NULL)
		{
			FTM_MEM_free(pSession->pReqBuff);
			pSession->pReqBuff = NULL;
		}

		if (pSession->pRespBuff != NULL)
		{
			FTM_MEM_free(pSession->pRespBuff);
			pSession->pRespBuff = NULL;
		}
	}

	return	xRet;
}

FTM_RET	FTDMS_destroySession
(
	FTDM_SERVER_PTR pServer,
	FTDM_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppSession != NULL);

	if ((*ppSession)->hSocket != 0)
	{
		TRACE("The session(%08x) was closed\n", (*ppSession)->hSocket);
		close((*ppSession)->hSocket);
		(*ppSession)->hSocket = 0;
	}

	sem_destroy(&(*ppSession)->xSemaphore);

	FTM_LIST_remove(&pServer->xSessionList, (FTM_VOID_PTR)*ppSession);	

	if ((*ppSession)->pReqBuff != NULL)
	{
		FTM_MEM_free((*ppSession)->pReqBuff);
		(*ppSession)->pReqBuff = NULL;
	}

	if ((*ppSession)->pRespBuff != NULL)
	{
		FTM_MEM_free((*ppSession)->pRespBuff);
		(*ppSession)->pRespBuff = NULL;
	}

	FTM_MEM_free(*ppSession);

	*ppSession = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_getSessionCount(FTDM_SERVER_PTR pServer, FTM_ULONG_PTR pulCount)
{
	ASSERT(pServer != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pServer->xSessionList, pulCount);
}

FTM_RET	FTDMS_getSessionInfo(FTDM_SERVER_PTR pServer, FTM_ULONG ulIndex, FTDM_SESSION_PTR pSession)
{
	ASSERT(pServer != NULL);
	ASSERT(pSession != NULL);
	FTM_RET				xRet;
	FTDM_SESSION_PTR	pElement;

	xRet = FTM_LIST_getAt(&pServer->xSessionList, ulIndex, (FTM_VOID_PTR _PTR_)&pElement);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pSession, pElement, sizeof(FTDM_SESSION));
	}

	return	xRet;
}

FTM_RET	FTDMS_NODE_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_NODE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_NODE_create(&pReq->xNodeInfo, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODEM_append(pServer->pDM->pNodeM, pNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add node[%s] : Error Code - %08x\n", pReq->xNodeInfo.pDID, xRet);
			FTDM_NODE_destroy(&pNode);
		}
	}
	else
	{
		ERROR2(xRet, "Failed to add node[%s] : Error Code - %08x\n", pReq->xNodeInfo.pDID, xRet);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}


FTM_RET	FTDMS_NODE_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;


	TRACE("Try to remove node[%s].\n", pReq->pDID);
	xRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODEM_remove(pServer->pDM->pNodeM, pNode);
		if (xRet == FTM_RET_OK)
		{
			FTDM_NODE_destroy(&pNode);
		}
		else
		{
			ERROR2(xRet, "Failed to remove node[%s] from list : Error Code - %08x\n", pReq->pDID, xRet);
		}
	}
	else
	{
		ERROR2(xRet, "Failed to get node[%s] : Error Code - %08x\n", pReq->pDID, xRet);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_NODE_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_NODEM_count(pServer->pDM->pNodeM, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_NODE_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_GET_PARAMS_PTR	pResp
)
{
	FTDM_NODE_PTR	pNode;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_NODE_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_NODEM_getAt(pServer->pDM->pNodeM, pReq->nIndex, &pNode);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_NODE_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_SET_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_SET_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->xNodeInfo.pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODE_setInfo(pNode, &pReq->xNodeInfo);
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_NODE_getDIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_NODE_GET_DID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_DID_LIST_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_DID);

	if (pReq->ulCount < ulMaxCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_NODEM_getDIDList(pServer->pDM->pNodeM, pResp->pDIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_DID) * pResp->ulCount;
	}
	else
	{
		pResp->nLen = sizeof(*pResp);
	}
	
	return	pResp->xRet;
}

FTM_RET	FTDMS_getNodeType
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR		pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
)
{
	FTDM_NODE_PTR	pNode = NULL;
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);

	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->xType = pNode->xInfo.xType;
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_getNodeURL
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
)
{
	FTDM_NODE_PTR	pNode = NULL;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nURLLen = FTM_URL_LEN;
	pResp->xRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);

	if (pResp->xRet == FTM_RET_OK)
	{
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_setNodeURL
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_SET_NODE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_NODE_URL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	//pResp->xRet = FTDM_setNodeURL(pReq->pDID, pReq->pURL);

	return	pResp->xRet;
}

FTM_RET	FTDMS_getNodeLocation
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_GET_NODE_LOCATION_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_LOCATION_PARAMS_PTR pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nLocationLen = FTM_LOCATION_LEN;
	//pResp->xRet = FTDM_getNodeLocation(
	//				pReq->pDID, 
	//				pResp->pLocation, 
	//				pResp->nLocationLen);

	return	pResp->xRet;
}

FTM_RET	FTDMS_setNodeLocation
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_SET_NODE_LOCATION_PARAMS_PTR		pReq,
 	FTDM_RESP_SET_NODE_LOCATION_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	//pResp->xRet = FTDM_setNodeLocation(
	//				pReq->pDID, 
	//				pReq->pLocation);

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_add
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EP_create(&pReq->xInfo, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EPM_append(pServer->pDM->pEPM, pEP);
		if (xRet != FTM_RET_OK)
		{
			FTDM_EP_destroy(&pEP);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_del
(
	FTDM_SERVER_PTR				pServer,
 	FTDM_REQ_EP_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EPM_remove(pServer->pDM->pEPM, pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTDM_EP_destroy(&pEP);
			if (xRet != FTM_RET_OK)
			{
				TRACE("EP[%s] destroy falled!\n", pReq->pEPID);
			}
		}
		else
		{
			TRACE("EP[%s] remove falled!\n", pReq->pEPID);
		}
	}
	else
	{
		TRACE("EP[%s] not found!\n", pReq->pEPID);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EPM_count(pServer->pDM->pEPM, pReq->xType, &pResp->nCount);
	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_get
(
	FTDM_SERVER_PTR				pServer,
 	FTDM_REQ_EP_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_get(pEP, &pResp->xInfo);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_GET_AT_PARAMS_PTR		pReq,
	FTDM_RESP_EP_GET_AT_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_getAt(pServer->pDM->pEPM, pReq->nIndex, &pEP);
	if (xRet == FTM_RET_OK)
	{
		FTDM_EP_get(pEP, &pResp->xInfo);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_set
(
	FTDM_SERVER_PTR				pServer,
 	FTDM_REQ_EP_SET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_SET_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xInfo.pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_set(pEP, &pReq->xInfo);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_getEPIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_GET_EPID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_EPID_LIST_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen -  sizeof(*pResp)) / sizeof(FTM_EPID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;	
	}
	
	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_EPM_getEPIDList(pServer->pDM->pEPM, pResp->pEPIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_EPID) * pResp->ulCount;
	}
	else
	{
		pResp->nLen = sizeof(*pResp);
	}
	
	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_CLASS_add
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_CLASS_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_add(&pReq->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_CLASS_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_CLASS_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_del(pReq->xType);

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_CLASS_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_CLASS_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_count(&pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_CLASS_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_CLASS_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_get(pReq->xEPClass, &pResp->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_CLASS_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_CLASS_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_AT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_getAt(pReq->nIndex, &pResp->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_DATA_add
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_add(pEP, &pReq->xData);
	
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_DATA_info
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_INFO_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_info(pEP, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_DATA_setLimit
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EP_DATA_SET_LIMIT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_SET_LIMIT_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_setLimit(pEP, &pReq->xLimit);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_DATA_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_EP_DATA);
	if (ulMaxCount > pReq->nCount)
	{
		ulMaxCount = pReq->nCount;
	}

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulDataCount = 0;
		xRet = FTDM_EP_DATA_count(pEP, &ulDataCount);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTDM_EP_DATA_get( pEP, pReq->nStartIndex, pResp->pData, ulMaxCount, &pResp->nCount);
			if (xRet == FTM_RET_OK)
			{
				if (pReq->nStartIndex + pResp->nCount < ulDataCount)
				{
					pResp->bRemain = FTM_TRUE;	
				}
			}
		}
	}
	pResp->xCmd = pReq->xCmd;
	pResp->xRet = xRet;

	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_PARAMS);
	}
	return	pResp->xRet;
}

FTM_RET	FTDMS_EP_DATA_getWithTime
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_EP_DATA);
	if (ulMaxCount > pReq->nCount)
	{
		ulMaxCount = pReq->nCount;
	}

	pResp->nCount = 0;
	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulDataCount = 0;
		xRet = FTDM_EP_DATA_countWithTime(pEP,pReq->nBeginTime, pReq->nEndTime, &ulDataCount);
		if (xRet == FTM_RET_OK)
		{
			if (ulDataCount != 0)
			{
				xRet = FTDM_EP_DATA_getWithTime(pEP, pReq->nBeginTime, pReq->nEndTime, pReq->bAscending, pResp->pData, ulMaxCount, &pResp->nCount);
				if (xRet == FTM_RET_OK)
				{
					if (ulDataCount > ulMaxCount)
					{
						pResp->bRemain = FTM_TRUE;	
					}
				}
			}
			else
			{
				pResp->nCount  = 0;	
			}
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->xRet = xRet;

	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS);
	}
	return	pResp->xRet;
}

FTM_RET 	FTDMS_EP_DATA_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_del(pEP, pReq->nIndex, pReq->nCount, &pResp->ulCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET 	FTDMS_EP_DATA_delWithTime
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_delWithTime( pEP, pReq->nBeginTime, pReq->nEndTime, &pResp->ulCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET 	FTDMS_EP_DATA_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_count( pEP, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET 	FTDMS_EP_DATA_countWithTime
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_countWithTime(pEP, pReq->nBeginTime, pReq->nEndTime, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDMS_TRIGGER_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger = NULL;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_create(&pReq->xTrigger, &pTrigger);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		strncpy(pResp->pTriggerID, pTrigger->xInfo.pID, FTM_ID_LEN);
	}
	return	pResp->xRet;
}


FTM_RET	FTDMS_TRIGGER_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_destroy(pReq->pTriggerID);

	return	pResp->xRet;
}

FTM_RET	FTDMS_TRIGGER_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_count(&pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_TRIGGER_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_GET_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_get(pReq->pTriggerID, &pTrigger);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_TRIGGER_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_getAt(pReq->nIndex, &pTrigger);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_TRIGGER_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_SET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_SET_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_set(pReq->pTriggerID, pReq->xFields, &pReq->xTrigger);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->xRet = FTDM_TRIGGER_get(pReq->pTriggerID, &pTrigger);
		if (pResp->xRet == FTM_RET_OK)
		{
			memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
		}
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_TRIGGER_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_ID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_TRIGGER_getIDList(pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_ID) * pResp->ulCount;
	}
	else
	{
		pResp->nLen = sizeof(*pResp);
	}
	
	return	pResp->xRet;
}

FTM_RET	FTDMS_ACTION_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_ADD_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_ACTION_create(&pReq->xAction, &pAction);

	return	pResp->xRet;
}

FTM_RET	FTDMS_ACTION_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_ACTION_destroy(pReq->pActionID);

	return	pResp->xRet;
}

FTM_RET	FTDMS_ACTION_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_ACTION_count(&pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_ACTION_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_GET_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_GET_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_ACTION_get(pReq->pActionID, &pAction);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_ACTION_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_ACTION_getAt(pReq->nIndex, &pAction);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_ACTION_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_ID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_ACTION_getIDList(pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_ID) * pResp->ulCount;
	}
	else
	{
		pResp->nLen = sizeof(*pResp);
	}
	
	return	pResp->xRet;
}

FTM_RET	FTDMS_RULE_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_RULE_create(&pReq->xRule);

	return	pResp->xRet;
}


FTM_RET	FTDMS_RULE_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_RULE_destroy(pReq->pRuleID);

	return	pResp->xRet;
}

FTM_RET	FTDMS_RULE_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_RULE_count(&pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_RULE_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_GET_PARAMS_PTR		pReq,
	FTDM_RESP_RULE_GET_PARAMS_PTR	pResp
)
{
	FTDM_RULE_PTR	pRule;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_RULE_get(pReq->pRuleID, &pRule);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_RULE));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_RULE_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_RULE_PTR	pRule;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_RULE_getAt(pReq->nIndex, &pRule);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_RULE));
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_RULE_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_EP_DATA);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_RULE_getIDList(pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_ID) * pResp->ulCount;
	}
	else
	{
		pResp->nLen = sizeof(*pResp);
	}
	
	return	pResp->xRet;
}

FTM_RET	FTDMS_LOG_add
(
	FTDM_SERVER_PTR				pServer,
	FTDM_REQ_LOG_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_LOGGER_add(pServer->pDM->pLogger, &pReq->xLog);

	return	pResp->xRet;
}


FTM_RET	FTDMS_LOG_del
(
	FTDM_SERVER_PTR				pServer,
 	FTDM_REQ_LOG_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_LOGGER_del(pServer->pDM->pLogger, pReq->ulIndex, pReq->ulCount, &pResp->ulCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_LOG_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_LOG_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_LOGGER_count(pServer->pDM->pLogger, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDMS_LOG_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_LOG_GET_PARAMS_PTR		pReq,
	FTDM_RESP_LOG_GET_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_LOG);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_LOGGER_get(pServer->pDM->pLogger, pReq->ulIndex, pResp->pLogs, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_LOG) * pResp->ulCount;
	}

	return	pResp->xRet;
}

FTM_RET	FTDMS_LOG_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_LOG_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_GET_AT_PARAMS_PTR	pResp
)
{
	//FTDM_LOG_PTR	pRule;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
#if 0
	pResp->xRet = FTDM_LOG_getAt(pReq->nIndex, &pRule);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_LOG));
	}
#endif
	return	pResp->xRet;
}
