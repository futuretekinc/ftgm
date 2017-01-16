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
}	FTDM_SIS_CMD_SET, _PTR_ FTDM_SIS_CMD_SET_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTDM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTDM_SIS_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTDM_SIS_service(FTM_VOID_PTR pData);
static FTM_BOOL		FTDM_SIS_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTDM_SIS_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTDM_CMD_NODE_ADD,				FTDM_SIS_addNode),
	MK_CMD_SET(FTDM_CMD_NODE_DEL,				FTDM_SIS_deleteNode),
	MK_CMD_SET(FTDM_CMD_NODE_COUNT,				FTDM_SIS_getNodeCount),
	MK_CMD_SET(FTDM_CMD_NODE_GET,				FTDM_SIS_getNode),
	MK_CMD_SET(FTDM_CMD_NODE_GET_AT,			FTDM_SIS_getNodeAt),
	MK_CMD_SET(FTDM_CMD_NODE_SET,				FTDM_SIS_setNode),
	MK_CMD_SET(FTDM_CMD_NODE_GET_DID_LIST,		FTDM_SIS_getNodeIDList),
	MK_CMD_SET(FTDM_CMD_EP_ADD,					FTDM_SIS_addEP),
	MK_CMD_SET(FTDM_CMD_EP_DEL,					FTDM_SIS_deleteEP),
	MK_CMD_SET(FTDM_CMD_EP_COUNT,				FTDM_SIS_getEPCount),
	MK_CMD_SET(FTDM_CMD_EP_GET,					FTDM_SIS_getEP),
	MK_CMD_SET(FTDM_CMD_EP_GET_AT,				FTDM_SIS_getEPAt),
	MK_CMD_SET(FTDM_CMD_EP_SET,					FTDM_SIS_setEP),
	MK_CMD_SET(FTDM_CMD_EP_GET_EPID_LIST,		FTDM_SIS_getEPIDList),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_ADD,			FTDM_SIS_addEPClass),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_DEL,			FTDM_SIS_deleteEPClass),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_COUNT,			FTDM_SIS_getEPClassCount),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_GET,			FTDM_SIS_getEPClass),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_GET_AT,		FTDM_SIS_getEPClassAt),
	MK_CMD_SET(FTDM_CMD_EP_DATA_ADD,			FTDM_SIS_addEPData),
	MK_CMD_SET(FTDM_CMD_EP_DATA_INFO,			FTDM_SIS_getEPDataInfo),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL,			FTDM_SIS_deleteEPData),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL_WITH_TIME,	FTDM_SIS_deleteEPDataWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET,			FTDM_SIS_getEPData),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET_WITH_TIME,	FTDM_SIS_getEPDataWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT,			FTDM_SIS_getEPDataCount),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT_WITH_TIME,FTDM_SIS_getEPDataCountWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_SET_LIMIT,		FTDM_SIS_setEPDataLimit),
	MK_CMD_SET(FTDM_CMD_TRIGGER_ADD,			FTDM_SIS_addTrigger),
	MK_CMD_SET(FTDM_CMD_TRIGGER_DEL,			FTDM_SIS_deleteTrigger),
	MK_CMD_SET(FTDM_CMD_TRIGGER_COUNT,			FTDM_SIS_getTriggerCount),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET,			FTDM_SIS_getTrigger),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET_AT,			FTDM_SIS_getTriggerAt),
	MK_CMD_SET(FTDM_CMD_TRIGGER_SET,			FTDM_SIS_setTrigger),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET_ID_LIST,	FTDM_SIS_getTriggerIDList),
	MK_CMD_SET(FTDM_CMD_ACTION_ADD,				FTDM_SIS_addAction),
	MK_CMD_SET(FTDM_CMD_ACTION_DEL,				FTDM_SIS_deleteAction),
	MK_CMD_SET(FTDM_CMD_ACTION_COUNT,			FTDM_SIS_getActionCount),
	MK_CMD_SET(FTDM_CMD_ACTION_GET,				FTDM_SIS_getAction),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_AT,			FTDM_SIS_getActionAt),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_ID_LIST,		FTDM_SIS_getActionIDList),
	MK_CMD_SET(FTDM_CMD_RULE_ADD,				FTDM_SIS_addRule),
	MK_CMD_SET(FTDM_CMD_RULE_DEL,				FTDM_SIS_deleteRule),
	MK_CMD_SET(FTDM_CMD_RULE_COUNT,				FTDM_SIS_getRuleCount),
	MK_CMD_SET(FTDM_CMD_RULE_GET,				FTDM_SIS_getRule),
	MK_CMD_SET(FTDM_CMD_RULE_GET_AT,			FTDM_SIS_getRuleAt),
	MK_CMD_SET(FTDM_CMD_RULE_GET_ID_LIST,		FTDM_SIS_getRuleIDList),
	MK_CMD_SET(FTDM_CMD_LOG_ADD,				FTDM_SIS_addLog),
	MK_CMD_SET(FTDM_CMD_LOG_DEL,				FTDM_SIS_deleteLog),
	MK_CMD_SET(FTDM_CMD_LOG_COUNT,				FTDM_SIS_getLogCount),
	MK_CMD_SET(FTDM_CMD_LOG_GET,				FTDM_SIS_getLog),
	MK_CMD_SET(FTDM_CMD_LOG_GET_AT,				FTDM_SIS_getLogAt),
	MK_CMD_SET(FTDM_CMD_UNKNOWN, 				NULL)
};


FTM_RET	FTDM_SIS_create
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTDM_SIS_PTR _PTR_ ppSIS
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(ppSIS != NULL);

	FTM_RET	xRet;
	FTDM_SIS_PTR	pSIS;

	pSIS = (FTDM_SIS_PTR)FTM_MEM_malloc(sizeof(FTDM_SIS));
	if (pSIS == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Failed to create service interface server!\n");
		return	xRet;
	}
	
	pSIS->pFTDM = pFTDM;
	FTM_LIST_create(&pSIS->pSessionList);
	FTM_LIST_setSeeker(pSIS->pSessionList, FTDM_SIS_SESSION_LIST_seeker);

	xRet = FTDM_SIS_init(pSIS);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pSIS);	
		ERROR2(xRet, "Failed to create servcice interface server!\n");
		return	xRet;
	}

	*ppSIS = pSIS;

	return	xRet;
}

FTM_RET	FTDM_SIS_destroy
(
	FTDM_SIS_PTR _PTR_ ppSIS
)
{
	ASSERT(ppSIS != NULL);

	FTM_RET	xRet;

	xRet = FTDM_SIS_final((*ppSIS));
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy service interface server!\n");	
	}

	FTM_LIST_destroy((*ppSIS)->pSessionList);

	FTM_MEM_free(*ppSIS);

	*ppSIS = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SIS_init
(
	FTDM_SIS_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SIS_final
(
	FTDM_SIS_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SIS_loadConfig
(
	FTDM_SIS_PTR		pServer,
	FTM_CONFIG_ITEM_PTR	pSection
)
{
	ASSERT(pServer != NULL);
	ASSERT(pSection != NULL);
	FTM_RET		xRet;
	FTM_USHORT	usPort;
	FTM_ULONG	ulSession;
	FTM_ULONG	ulBufferLen;

	xRet = FTM_CONFIG_ITEM_getItemUSHORT(pSection, "port", &usPort);
	if (xRet == FTM_RET_OK)
	{
		pServer->xConfig.usPort = usPort;
	}

	xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "session_count", &ulSession);
	if (xRet == FTM_RET_OK)
	{
		pServer->xConfig.ulMaxSession = ulSession;
	}

	xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "buffer_len", &ulBufferLen);
	if (xRet == FTM_RET_OK)
	{
		pServer->xConfig.ulBufferLen = ulBufferLen;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SIS_start
(
	FTDM_SIS_PTR pServer
)
{
	ASSERT(pServer != NULL);

	if (pthread_create(&pServer->xThread, NULL, FTDM_SIS_process, (void *)pServer) < 0)
	{
		ERROR2(FTM_RET_THREAD_CREATION_ERROR, "Failed to create pthread.");
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SIS_stop
(
	FTDM_SIS_PTR 		pServer
)
{
	ASSERT(pServer != NULL);

	pServer->bStop = FTM_TRUE;
	close(pServer->hSocket);

	return	FTDM_SIS_waitingForFinished(pServer);
}

FTM_RET	FTDM_SIS_waitingForFinished
(
	FTDM_SIS_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	pthread_join(pServer->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDM_SIS_process(FTM_VOID_PTR pData)
{
	FTM_INT				nRet;
	struct sockaddr_in	xServer, xClient;
	FTDM_SIS_PTR		pServer =(FTDM_SIS_PTR)pData;
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
			
			xRet = FTDM_SIS_createSession(pServer, hClient, (struct sockaddr *)&xClient, &pSession);
			if (xRet != FTM_RET_OK)
			{
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				FTM_INT	nRet;

				nRet = pthread_create(&pSession->xThread, NULL, FTDM_SIS_service, pSession);
				if (nRet != 0)
				{
					ERROR2(FTM_RET_CANT_CREATE_THREAD, "Can't create a thread[%d]\n", nRet);
					FTDM_SIS_destroySession(pServer, &pSession);
				}
			}
		}
		xCurrentTime = time(NULL) / 3600; 
		if (xPrevTime != xCurrentTime)
		{
			FTDM_removeInvalidData(pServer->pFTDM);	
		}

		xPrevTime = xCurrentTime;
		usleep(10000);
	}

	FTDM_SESSION_PTR pSession;

	FTM_LIST_iteratorStart(pServer->pSessionList);
	while(FTM_LIST_iteratorNext(pServer->pSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pSession->bStop = FTM_TRUE;
		shutdown(pSession->hSocket, SHUT_RD);
		pthread_join(pSession->xThread, 0);

		FTDM_SIS_destroySession(pServer, &pSession);
	}

error:

	return	0;
}

FTM_VOID_PTR FTDM_SIS_service(FTM_VOID_PTR pData)
{
	FTDM_SIS_PTR			pServer;
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
		if (FTM_RET_OK != FTDM_SIS_serviceCall(pSession->pServer, pReq, pResp))
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

	FTDM_SIS_destroySession(pServer, &pSession);

	sem_post(&pServer->xSemaphore);

	return	0;
}

FTM_RET	FTDM_SIS_serviceCall
(
	FTDM_SIS_PTR			pServer,
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
)
{
	FTM_RET				xRet;
	FTDM_SIS_CMD_SET_PTR	pSet = pCmdSet;
	
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

FTM_BOOL	FTDM_SIS_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTDM_SESSION_PTR	pSession = (FTDM_SESSION_PTR)pElement;
	pthread_t 			*pPThread = (pthread_t *)pIndicator;	

	return (pSession->xThread == *pPThread);
}

FTM_RET	FTDM_SIS_createSession
(
	FTDM_SIS_PTR pServer,
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

	xRet = FTM_LIST_append(pServer->pSessionList, pSession);	
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

FTM_RET	FTDM_SIS_destroySession
(
	FTDM_SIS_PTR pServer,
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

	FTM_LIST_remove(pServer->pSessionList, (FTM_VOID_PTR)*ppSession);	

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

FTM_RET	FTDM_SIS_getSessionCount(FTDM_SIS_PTR pServer, FTM_ULONG_PTR pulCount)
{
	ASSERT(pServer != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pServer->pSessionList, pulCount);
}

FTM_RET	FTDM_SIS_getSessionInfo(FTDM_SIS_PTR pServer, FTM_ULONG ulIndex, FTDM_SESSION_PTR pSession)
{
	ASSERT(pServer != NULL);
	ASSERT(pSession != NULL);
	FTM_RET				xRet;
	FTDM_SESSION_PTR	pElement;

	xRet = FTM_LIST_getAt(pServer->pSessionList, ulIndex, (FTM_VOID_PTR _PTR_)&pElement);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pSession, pElement, sizeof(FTDM_SESSION));
	}

	return	xRet;
}

FTM_RET	FTDM_SIS_addNode
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_NODE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_createNode(pServer->pFTDM, &pReq->xNodeInfo, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to add node[%s] : Error Code - %08x\n", pReq->xNodeInfo.pDID, xRet);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}


FTM_RET	FTDM_SIS_deleteNode
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;

	TRACE("Try to remove node[%s].\n", pReq->pDID);
	xRet = FTDM_deleteNode(pServer->pFTDM, pReq->pDID);
	if (xRet == FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove node[%s] from list : Error Code - %08x\n", pReq->pDID, xRet);
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

FTM_RET	FTDM_SIS_getNodeCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getNodeCount(pServer->pFTDM, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getNode
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_GET_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;
 
	xRet = FTDM_getNode(pServer->pFTDM, pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODE_getInfo(pNode, &pResp->xNodeInfo);
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getNodeAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_getNodeAt(pServer->pFTDM, pReq->nIndex, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODE_getInfo(pNode, &pResp->xNodeInfo);
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_setNode
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_NODE_SET_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_SET_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_getNode(pServer->pFTDM, pReq->xNodeInfo.pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODE_setInfo(pNode, &pReq->xNodeInfo);
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getNodeIDList
(
	FTDM_SIS_PTR					pServer,
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
	pResp->xRet = FTDM_getNodeIDList(pServer->pFTDM, pResp->pDIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
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

FTM_RET	FTDM_SIS_getNodeType
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR		pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode = NULL;

	xRet = FTDM_getNode(pServer->pFTDM, pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODE_getType(pNode, &pResp->xType);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;
	

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getNodeURL
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
)
{
	FTDM_NODE_PTR	pNode = NULL;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nURLLen = FTM_URL_LEN;
	pResp->xRet = FTDM_getNode(pServer->pFTDM, pReq->pDID, &pNode);

	if (pResp->xRet == FTM_RET_OK)
	{
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_setNodeURL
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_SET_NODE_URL_PARAMS_PTR 	pReq,
	FTDM_RESP_SET_NODE_URL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	//pResp->xRet = FTDM_setNodeURL(pReq->pDID, pReq->pURL);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getNodeLocation
(
	FTDM_SIS_PTR					pServer,
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

FTM_RET	FTDM_SIS_setNodeLocation
(
	FTDM_SIS_PTR					pServer,
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

FTM_RET	FTDM_SIS_addEP
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_createEP(pServer->pFTDM, &pReq->xInfo, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create EP!\n");
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_deleteEP
(
	FTDM_SIS_PTR				pServer,
 	FTDM_REQ_EP_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;

	xRet = FTDM_deleteEP(pServer->pFTDM, pReq->pEPID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete EP!\n");
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getEPCount(pServer->pFTDM, pReq->xType, &pResp->nCount);
	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEP
(
	FTDM_SIS_PTR				pServer,
 	FTDM_REQ_EP_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_GET_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_getInfo(pEP, &pResp->xInfo);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_GET_AT_PARAMS_PTR		pReq,
	FTDM_RESP_EP_GET_AT_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEPAt(pServer->pFTDM, pReq->nIndex, &pEP);
	if (xRet == FTM_RET_OK)
	{
		FTDM_EP_getInfo(pEP, &pResp->xInfo);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_setEP
(
	FTDM_SIS_PTR				pServer,
 	FTDM_REQ_EP_SET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_SET_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
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

FTM_RET	FTDM_SIS_getEPIDList
(
	FTDM_SIS_PTR					pServer,
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
	pResp->xRet = FTDM_getEPIDList(pServer->pFTDM, pResp->pEPIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
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

FTM_RET	FTDM_SIS_addEPClass
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_CLASS_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_add(&pReq->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_deleteEPClass
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_CLASS_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_del(pReq->xType);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPClassCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_CLASS_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_count(&pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPClass
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_CLASS_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_get(pReq->xEPClass, &pResp->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPClassAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_CLASS_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_GET_AT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_EP_CLASS_getAt(pReq->nIndex, &pResp->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_addEPData
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_addData(pEP, &pReq->xData);
	
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPDataInfo
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_INFO_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_getDataInfo(pEP, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_setEPDataLimit
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_EP_DATA_SET_LIMIT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_SET_LIMIT_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_setDataLimit(pEP, &pReq->xLimit);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getEPData
(
	FTDM_SIS_PTR					pServer,
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

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulDataCount = 0;
		xRet = FTDM_EP_getDataCount(pEP, &ulDataCount);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTDM_EP_getData( pEP, pReq->nStartIndex, pResp->pData, ulMaxCount, &pResp->nCount);
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

FTM_RET	FTDM_SIS_getEPDataWithTime
(
	FTDM_SIS_PTR					pServer,
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
	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulDataCount = 0;
		xRet = FTDM_EP_getDataCountWithTime(pEP,pReq->nBeginTime, pReq->nEndTime, &ulDataCount);
		if (xRet == FTM_RET_OK)
		{
			if (ulDataCount != 0)
			{
				xRet = FTDM_EP_getDataWithTime(pEP, pReq->nBeginTime, pReq->nEndTime, pReq->bAscending, pResp->pData, ulMaxCount, &pResp->nCount);
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

FTM_RET 	FTDM_SIS_deleteEPData
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_deleteData(pEP, pReq->nIndex, pReq->nCount, &pResp->ulCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET 	FTDM_SIS_deleteEPDataWithTime
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_deleteDataWithTime( pEP, pReq->nBeginTime, pReq->nEndTime, &pResp->ulCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET 	FTDM_SIS_getEPDataCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_getDataCount( pEP, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET 	FTDM_SIS_getEPDataCountWithTime
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_getEP(pServer->pFTDM, pReq->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_getDataCountWithTime(pEP, pReq->nBeginTime, pReq->nEndTime, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_addTrigger
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger = NULL;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_TRIGGER_create(pServer->pFTDM, &pReq->xTrigger, &pTrigger);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		strncpy(pResp->pTriggerID, pTrigger->xInfo.pID, FTM_ID_LEN);
	}
	return	pResp->xRet;
}


FTM_RET	FTDM_SIS_deleteTrigger
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTDM_deleteTrigger(pServer->pFTDM, pReq->pTriggerID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete trigger!\n");
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getTriggerCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getTriggerCount(pServer->pFTDM, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getTrigger
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_GET_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getTrigger(pServer->pFTDM, pReq->pTriggerID, &pTrigger);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getTriggerAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_TRIGGER_PTR	pTrigger;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getTriggerAt(pServer->pFTDM, pReq->nIndex, &pTrigger);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_setTrigger
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_SET_PARAMS_PTR		pReq,
	FTDM_RESP_TRIGGER_SET_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	xRet = FTDM_getTrigger(pServer->pFTDM, pReq->pTriggerID, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_TRIGGER_set(pTrigger, pReq->xFields, &pReq->xTrigger);
		if (xRet == FTM_RET_OK)
		{
			memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
		}
	
	}

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getTriggerIDList
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulMaxCount;

	ulMaxCount = (pResp->nLen - sizeof(*pResp)) / sizeof(FTM_ID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	xRet = FTDM_getTriggerIDList(pServer->pFTDM, pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_ID) * pResp->ulCount;
	}
	else
	{
		pResp->nLen = sizeof(*pResp);
	}
	
	pResp->xCmd	= pReq->xCmd;
	pResp->xRet = xRet;
	
	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_addAction
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_ADD_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_createAction(pServer->pFTDM, &pReq->xAction, &pAction);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_deleteAction
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTDM_deleteAction(pServer->pFTDM, pReq->pActionID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete trigger!\n");
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getActionCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getActionCount(pServer->pFTDM, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getAction
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_GET_PARAMS_PTR		pReq,
	FTDM_RESP_ACTION_GET_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getAction(pServer->pFTDM, pReq->pActionID, &pAction);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getActionAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_ACTION_PTR	pAction;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getActionAt(pServer->pFTDM, pReq->nIndex, &pAction);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getActionIDList
(
	FTDM_SIS_PTR					pServer,
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
	pResp->xRet = FTDM_getActionIDList(pServer->pFTDM, pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
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

FTM_RET	FTDM_SIS_addRule
(
	FTDM_SIS_PTR					pServer,
	FTDM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_ADD_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTDM_RULE_PTR	pRule;

	xRet = FTDM_createRule(pServer->pFTDM, &pReq->xRule, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create rule!\n");
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}


FTM_RET	FTDM_SIS_deleteRule
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTDM_deleteRule(pServer->pFTDM, pReq->pRuleID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete rule!\n");	
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getRuleCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getRuleCount(pServer->pFTDM, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getRule
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_GET_PARAMS_PTR		pReq,
	FTDM_RESP_RULE_GET_PARAMS_PTR	pResp
)
{
	FTDM_RULE_PTR	pRule;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getRule(pServer->pFTDM, pReq->pRuleID, &pRule);
	if (pResp->xRet == FTM_RET_OK)
	{
		FTDM_RULE_get(pRule, &pResp->xRule);
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getRuleAt
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_RULE_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_RULE_GET_AT_PARAMS_PTR	pResp
)
{
	FTDM_RULE_PTR	pRule;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getRuleAt(pServer->pFTDM, pReq->nIndex, &pRule);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		FTDM_RULE_get(pRule, &pResp->xRule);
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getRuleIDList
(
	FTDM_SIS_PTR					pServer,
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
	pResp->xRet = FTDM_getRuleIDList(pServer->pFTDM, pResp->pIDs, pReq->ulIndex, ulMaxCount, &pResp->ulCount);
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

FTM_RET	FTDM_SIS_addLog
(
	FTDM_SIS_PTR				pServer,
	FTDM_REQ_LOG_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_ADD_PARAMS_PTR	pResp
)
{

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_addLog(pServer->pFTDM, &pReq->xLog);

	return	pResp->xRet;
}


FTM_RET	FTDM_SIS_deleteLog
(
	FTDM_SIS_PTR				pServer,
 	FTDM_REQ_LOG_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_deleteLog(pServer->pFTDM, pReq->ulIndex, pReq->ulCount, &pResp->ulCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getLogCount
(
	FTDM_SIS_PTR					pServer,
 	FTDM_REQ_LOG_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_LOG_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->xRet = FTDM_getLogCount(pServer->pFTDM, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getLog
(
	FTDM_SIS_PTR					pServer,
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
	pResp->xRet = FTDM_getLog(pServer->pFTDM, pReq->ulIndex, pResp->pLogs, ulMaxCount, &pResp->ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(*pResp) + sizeof(FTM_LOG) * pResp->ulCount;
	}

	return	pResp->xRet;
}

FTM_RET	FTDM_SIS_getLogAt
(
	FTDM_SIS_PTR					pServer,
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
