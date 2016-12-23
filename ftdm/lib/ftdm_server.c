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
#include "ftdm_event.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"
#include "ftdm_dbif.h"

#undef	__MODULE__
#define	__MODULE__	FTDM_TRACE_MODULE_SERVER

typedef struct
{
	FTDM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTDM_SERVICE_CALLBACK	fService;
}	FTDM_SERVER_CMD_SET, _PTR_ FTDM_SERVER_CMD_SET_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTDM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTDM_SERVER_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTDM_SERVER_service(FTM_VOID_PTR pData);
static FTM_BOOL		FTDM_SERVER_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTDM_SERVER_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTDM_CMD_NODE_ADD,				FTDM_SERVER_NODE_add ),
	MK_CMD_SET(FTDM_CMD_NODE_DEL,				FTDM_SERVER_NODE_del ),
	MK_CMD_SET(FTDM_CMD_NODE_COUNT,				FTDM_SERVER_NODE_count ),
	MK_CMD_SET(FTDM_CMD_NODE_GET,				FTDM_SERVER_NODE_get ),
	MK_CMD_SET(FTDM_CMD_NODE_GET_AT,			FTDM_SERVER_NODE_getAt ),
	MK_CMD_SET(FTDM_CMD_NODE_SET,				FTDM_SERVER_NODE_set ),
	MK_CMD_SET(FTDM_CMD_NODE_GET_DID_LIST,		FTDM_SERVER_NODE_getDIDList ),
	MK_CMD_SET(FTDM_CMD_EP_ADD,					FTDM_SERVER_EP_add ),
	MK_CMD_SET(FTDM_CMD_EP_DEL,					FTDM_SERVER_EP_del ),
	MK_CMD_SET(FTDM_CMD_EP_COUNT,				FTDM_SERVER_EP_count ),
	MK_CMD_SET(FTDM_CMD_EP_GET,					FTDM_SERVER_EP_get ),
	MK_CMD_SET(FTDM_CMD_EP_GET_AT,				FTDM_SERVER_EP_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_SET,					FTDM_SERVER_EP_set ),
	MK_CMD_SET(FTDM_CMD_EP_GET_EPID_LIST,		FTDM_SERVER_EP_getEPIDList ),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_ADD,			FTDM_SERVER_EP_CLASS_add),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_DEL,			FTDM_SERVER_EP_CLASS_del),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_COUNT,			FTDM_SERVER_EP_CLASS_count ),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_GET,			FTDM_SERVER_EP_CLASS_get),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_GET_AT,		FTDM_SERVER_EP_CLASS_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_DATA_ADD,			FTDM_SERVER_EP_DATA_add ),
	MK_CMD_SET(FTDM_CMD_EP_DATA_INFO,			FTDM_SERVER_EP_DATA_info),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL,			FTDM_SERVER_EP_DATA_del),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL_WITH_TIME,	FTDM_SERVER_EP_DATA_delWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET,			FTDM_SERVER_EP_DATA_get),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET_WITH_TIME,	FTDM_SERVER_EP_DATA_getWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT,			FTDM_SERVER_EP_DATA_count),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT_WITH_TIME,FTDM_SERVER_EP_DATA_countWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_SET_LIMIT,		FTDM_SERVER_EP_DATA_setLimit),
	MK_CMD_SET(FTDM_CMD_EVENT_ADD,			FTDM_SERVER_EVENT_add ),
	MK_CMD_SET(FTDM_CMD_EVENT_DEL,			FTDM_SERVER_EVENT_del ),
	MK_CMD_SET(FTDM_CMD_EVENT_COUNT,			FTDM_SERVER_EVENT_count ),
	MK_CMD_SET(FTDM_CMD_EVENT_GET,			FTDM_SERVER_EVENT_get ),
	MK_CMD_SET(FTDM_CMD_EVENT_GET_AT,			FTDM_SERVER_EVENT_getAt ),
	MK_CMD_SET(FTDM_CMD_EVENT_SET,			FTDM_SERVER_EVENT_set ),
	MK_CMD_SET(FTDM_CMD_EVENT_GET_ID_LIST,	FTDM_SERVER_EVENT_getIDList ),
	MK_CMD_SET(FTDM_CMD_ACTION_ADD,				FTDM_SERVER_ACTION_add ),
	MK_CMD_SET(FTDM_CMD_ACTION_DEL,				FTDM_SERVER_ACTION_del ),
	MK_CMD_SET(FTDM_CMD_ACTION_COUNT,			FTDM_SERVER_ACTION_count ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET,				FTDM_SERVER_ACTION_get ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_AT,			FTDM_SERVER_ACTION_getAt ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_ID_LIST,		FTDM_SERVER_ACTION_getIDList ),
	MK_CMD_SET(FTDM_CMD_RULE_ADD,				FTDM_SERVER_RULE_add ),
	MK_CMD_SET(FTDM_CMD_RULE_DEL,				FTDM_SERVER_RULE_del ),
	MK_CMD_SET(FTDM_CMD_RULE_COUNT,				FTDM_SERVER_RULE_count ),
	MK_CMD_SET(FTDM_CMD_RULE_GET,				FTDM_SERVER_RULE_get ),
	MK_CMD_SET(FTDM_CMD_RULE_GET_AT,			FTDM_SERVER_RULE_getAt ),
	MK_CMD_SET(FTDM_CMD_RULE_GET_ID_LIST,		FTDM_SERVER_RULE_getIDList ),
	MK_CMD_SET(FTDM_CMD_LOG_ADD,				FTDM_SERVER_LOG_add ),
	MK_CMD_SET(FTDM_CMD_LOG_DEL,				FTDM_SERVER_LOG_del ),
	MK_CMD_SET(FTDM_CMD_LOG_COUNT,				FTDM_SERVER_LOG_count ),
	MK_CMD_SET(FTDM_CMD_LOG_GET,				FTDM_SERVER_LOG_get ),
	MK_CMD_SET(FTDM_CMD_LOG_GET_AT,				FTDM_SERVER_LOG_getAt ),
	MK_CMD_SET(FTDM_CMD_UNKNOWN, 				NULL)
};


FTM_RET	FTDM_SERVER_init
(
	FTDM_SERVER_PTR			pServer,
	FTDM_CONTEXT_PTR		pDM
)
{
	ASSERT(pServer != NULL);

	pServer->pDM = pDM;
	pServer->xConfig.usPort 		= FTDM_SERVER_DEFAULT_PORT;
	pServer->xConfig.ulMaxSession 	= FTDM_SERVER_DEFAULT_MAX_SESSION;
	pServer->xConfig.ulBufferLen 	= FTDM_SERVER_DEFAULT_BUFFER_LEN;

	FTM_LIST_init(&pServer->xSessionList);
	FTM_LIST_setSeeker(&pServer->xSessionList, FTDM_SERVER_SESSION_LIST_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SERVER_final
(
	FTDM_SERVER_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SERVER_loadConfig
(
	FTDM_SERVER_PTR			pServer,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "server", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_USHORT	usPort;
		FTM_ULONG	ulSession;
		FTM_ULONG	ulBufferLen;

		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &usPort);
		if (xRet == FTM_RET_OK)
		{
			pServer->xConfig.usPort = usPort;
		}

		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "session_count", &ulSession);
		if (xRet == FTM_RET_OK)
		{
			pServer->xConfig.ulMaxSession = ulSession;
		}

		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "buffer_len", &ulBufferLen);
		if (xRet == FTM_RET_OK)
		{
			pServer->xConfig.ulBufferLen = ulBufferLen;
		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SERVER_start
(
	FTDM_SERVER_PTR pServer
)
{
	ASSERT(pServer != NULL);

	if (pthread_create(&pServer->xThread, NULL, FTDM_SERVER_process, (void *)pServer) < 0)
	{
		ERROR2(FTM_RET_THREAD_CREATION_ERROR, "Failed to create pthread.");
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SERVER_stop
(
	FTDM_SERVER_PTR 		pServer
)
{
	ASSERT(pServer != NULL);

	pServer->bStop = FTM_TRUE;
	close(pServer->hSocket);

	return	FTDM_SERVER_waitingForFinished(pServer);
}

FTM_RET	FTDM_SERVER_waitingForFinished
(
	FTDM_SERVER_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	pthread_join(pServer->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDM_SERVER_process(FTM_VOID_PTR pData)
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
			
			xRet = FTDM_SERVER_createSession(pServer, hClient, (struct sockaddr *)&xClient, &pSession);
			if (xRet != FTM_RET_OK)
			{
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				FTM_INT	nRet;

				nRet = pthread_create(&pSession->xThread, NULL, FTDM_SERVER_service, pSession);
				if (nRet != 0)
				{
					ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create a thread[%d]\n", nRet);
					FTDM_SERVER_destroySession(pServer, &pSession);
				}
			}
		}
		xCurrentTime = time(NULL) / 3600; 
		if (xPrevTime != xCurrentTime)
		{
			FTDM_removeInvalidData();	
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

		FTDM_SERVER_destroySession(pServer, &pSession);
	}

error:

	return	0;
}

FTM_VOID_PTR FTDM_SERVER_service(FTM_VOID_PTR pData)
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
		if (FTM_RET_OK != FTDM_SERVER_serviceCall(pSession->pServer, pReq, pResp))
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

	FTDM_SERVER_destroySession(pServer, &pSession);

	sem_post(&pServer->xSemaphore);

	return	0;
}

FTM_RET	FTDM_SERVER_serviceCall
(
	FTDM_SERVER_PTR			pServer,
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
)
{
	FTM_RET				xRet;
	FTDM_SERVER_CMD_SET_PTR	pSet = pCmdSet;
	
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

FTM_BOOL	FTDM_SERVER_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTDM_SESSION_PTR	pSession = (FTDM_SESSION_PTR)pElement;
	pthread_t 			*pPThread = (pthread_t *)pIndicator;	

	return (pSession->xThread == *pPThread);
}

FTM_RET	FTDM_SERVER_createSession
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

FTM_RET	FTDM_SERVER_destroySession
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

FTM_RET	FTDM_SERVER_getSessionCount(FTDM_SERVER_PTR pServer, FTM_ULONG_PTR pulCount)
{
	ASSERT(pServer != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pServer->xSessionList, pulCount);
}

FTM_RET	FTDM_SERVER_getSessionInfo(FTDM_SERVER_PTR pServer, FTM_ULONG ulIndex, FTDM_SESSION_PTR pSession)
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

FTM_RET	FTDM_SERVER_NODE_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_NODE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_NODE_create(pServer->pDM->pDBIF, &pReq->xNodeInfo, &pNode);
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


FTM_RET	FTDM_SERVER_NODE_del
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

FTM_RET	FTDM_SERVER_NODE_count
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

FTM_RET	FTDM_SERVER_NODE_get
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

FTM_RET	FTDM_SERVER_NODE_getAt
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

FTM_RET	FTDM_SERVER_NODE_set
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

FTM_RET	FTDM_SERVER_NODE_getDIDList
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

FTM_RET	FTDM_SERVER_getNodeType
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

FTM_RET	FTDM_SERVER_getNodeURL
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

FTM_RET	FTDM_SERVER_setNodeURL
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

FTM_RET	FTDM_SERVER_getNodeLocation
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

FTM_RET	FTDM_SERVER_setNodeLocation
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

FTM_RET	FTDM_SERVER_EP_add
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EP_create(pServer->pDM->pDBIF, &pReq->xInfo, &pEP);
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

FTM_RET	FTDM_SERVER_EP_del
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

FTM_RET	FTDM_SERVER_EP_count
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

FTM_RET	FTDM_SERVER_EP_get
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
		xRet = FTDM_EP_getInfo(pEP, &pResp->xInfo);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EP_getAt
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
		FTDM_EP_getInfo(pEP, &pResp->xInfo);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EP_set
(
	FTDM_SERVER_PTR				pServer,
 	FTDM_REQ_EP_SET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_SET_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_setFields(pEP, pReq->xFields, &pReq->xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "EP[%s] set fields failed.\n", pReq->pEPID);	
		}

	}
	else
	{
		ERROR2(xRet, "EP[%s] get failed.\n", pReq->pEPID);	
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EP_getEPIDList
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

FTM_RET	FTDM_SERVER_EP_CLASS_add
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

FTM_RET	FTDM_SERVER_EP_CLASS_del
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

FTM_RET	FTDM_SERVER_EP_CLASS_count
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

FTM_RET	FTDM_SERVER_EP_CLASS_get
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

FTM_RET	FTDM_SERVER_EP_CLASS_getAt
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

FTM_RET	FTDM_SERVER_EP_DATA_add
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

FTM_RET	FTDM_SERVER_EP_DATA_info
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

FTM_RET	FTDM_SERVER_EP_DATA_setLimit
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

FTM_RET	FTDM_SERVER_EP_DATA_get
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

FTM_RET	FTDM_SERVER_EP_DATA_getWithTime
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

FTM_RET 	FTDM_SERVER_EP_DATA_del
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

FTM_RET 	FTDM_SERVER_EP_DATA_delWithTime
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

FTM_RET 	FTDM_SERVER_EP_DATA_count
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

FTM_RET 	FTDM_SERVER_EP_DATA_countWithTime
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

FTM_RET	FTDM_SERVER_EVENT_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_EVENT_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_ADD_PARAMS_PTR	pResp
)
{
	FTDM_EVENT_PTR	pEvent = NULL;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_createEvent(&pReq->xEvent, &pEvent);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		strncpy(pResp->pEventID, pEvent->xInfo.pID, FTM_ID_LEN);
	}
	return	pResp->xRet;
}


FTM_RET	FTDM_SERVER_EVENT_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EVENT_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_destroyEvent(pReq->pEventID);

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EVENT_count
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EVENT_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EVENT_count(&pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EVENT_get
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EVENT_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EVENT_GET_PARAMS_PTR	pResp
)
{
	FTDM_EVENT_PTR	pEvent;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EVENT_get(pReq->pEventID, &pEvent);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xEvent, &pEvent->xInfo, sizeof(FTM_EVENT));
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EVENT_getAt
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EVENT_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_EVENT_PTR	pEvent;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EVENT_getAt(pReq->nIndex, &pEvent);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xEvent, &pEvent->xInfo, sizeof(FTM_EVENT));
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EVENT_set
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EVENT_SET_PARAMS_PTR		pReq,
	FTDM_RESP_EVENT_SET_PARAMS_PTR	pResp
)
{
	FTDM_EVENT_PTR	pEvent;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EVENT_set(pReq->pEventID, pReq->xFields, &pReq->xEvent);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->xRet = FTDM_EVENT_get(pReq->pEventID, &pEvent);
		if (pResp->xRet == FTM_RET_OK)
		{
			memcpy(&pResp->xEvent, &pEvent->xInfo, sizeof(FTM_EVENT));
		}
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_EVENT_getIDList
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EVENT_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_ID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = FTDM_EVENT_getIDList(pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
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

FTM_RET	FTDM_SERVER_ACTION_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_ADD_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_createAction(&pReq->xAction, &pAction);

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_ACTION_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_destroyAction(pReq->pActionID);

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_ACTION_count
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

FTM_RET	FTDM_SERVER_ACTION_get
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

FTM_RET	FTDM_SERVER_ACTION_getAt
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

FTM_RET	FTDM_SERVER_ACTION_getIDList
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

FTM_RET	FTDM_SERVER_RULE_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_createRule(&pReq->xRule, NULL);

	return	pResp->xRet;
}


FTM_RET	FTDM_SERVER_RULE_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_destroyRule(pReq->pRuleID);

	return	pResp->xRet;
}

FTM_RET	FTDM_SERVER_RULE_count
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

FTM_RET	FTDM_SERVER_RULE_get
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

FTM_RET	FTDM_SERVER_RULE_getAt
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

FTM_RET	FTDM_SERVER_RULE_getIDList
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

FTM_RET	FTDM_SERVER_LOG_add
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


FTM_RET	FTDM_SERVER_LOG_del
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

FTM_RET	FTDM_SERVER_LOG_count
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

FTM_RET	FTDM_SERVER_LOG_get
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

FTM_RET	FTDM_SERVER_LOG_getAt
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
