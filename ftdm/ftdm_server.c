#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
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
	MK_CMD_SET(FTDM_CMD_EP_ADD,					FTDMS_EP_add ),
	MK_CMD_SET(FTDM_CMD_EP_DEL,					FTDMS_EP_del ),
	MK_CMD_SET(FTDM_CMD_EP_COUNT,				FTDMS_EP_count ),
	MK_CMD_SET(FTDM_CMD_EP_GET,					FTDMS_EP_get ),
	MK_CMD_SET(FTDM_CMD_EP_GET_AT,				FTDMS_EP_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_SET,					FTDMS_EP_set ),
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
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT,			FTDMS_EP_DATA_count),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT_WITH_TIME,FTDMS_EP_DATA_countWithTime),
	MK_CMD_SET(FTDM_CMD_TRIGGER_ADD,			FTDMS_TRIGGER_add ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_DEL,			FTDMS_TRIGGER_del ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_COUNT,			FTDMS_TRIGGER_count ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET,			FTDMS_TRIGGER_get ),
	MK_CMD_SET(FTDM_CMD_TRIGGER_GET_AT,			FTDMS_TRIGGER_getAt ),
	MK_CMD_SET(FTDM_CMD_ACTION_ADD,				FTDMS_ACTION_add ),
	MK_CMD_SET(FTDM_CMD_ACTION_DEL,				FTDMS_ACTION_del ),
	MK_CMD_SET(FTDM_CMD_ACTION_COUNT,			FTDMS_ACTION_count ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET,				FTDMS_ACTION_get ),
	MK_CMD_SET(FTDM_CMD_ACTION_GET_AT,			FTDMS_ACTION_getAt ),
	MK_CMD_SET(FTDM_CMD_RULE_ADD,				FTDMS_RULE_add ),
	MK_CMD_SET(FTDM_CMD_RULE_DEL,				FTDMS_RULE_del ),
	MK_CMD_SET(FTDM_CMD_RULE_COUNT,				FTDMS_RULE_count ),
	MK_CMD_SET(FTDM_CMD_RULE_GET,				FTDMS_RULE_get ),
	MK_CMD_SET(FTDM_CMD_RULE_GET_AT,			FTDMS_RULE_getAt ),
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

FTM_RET	FTDMS_start(FTDM_SERVER_PTR pServer)
{
	ASSERT(pServer != NULL);

	if (pthread_create(&pServer->xThread, NULL, FTDMS_process, (void *)pServer) < 0)
	{
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

	ASSERT(pData != NULL);

	if (sem_init(&pServer->xSemaphore, 0, pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR("Can't alloc semaphore!\n");
		goto error;
	}

	pServer->hSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (pServer->hSocket == -1)
	{
		ERROR("Could not create socket\n");
		goto error;
	}

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( pServer->xConfig.usPort );

	nRet = bind( pServer->hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR("bind failed.[nRet = %d]\n", nRet);
		goto error;
	}

	listen(pServer->hSocket, 3);


	while(!pServer->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nSockAddrInLen = sizeof(struct sockaddr_in);	

		hClient = accept(pServer->hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient > 0)
		{

			TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClient.sin_addr), ntohs(xClient.sin_port));

			FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTDM_SESSION));
			if (pSession == NULL)
			{
				ERROR("System memory is not enough!\n");
				close(hClient);
				TRACE("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				FTM_INT	nRet;
				TRACE("The new session(%08x) has beed connected\n", hClient);

				pSession->pServer = pServer;
				pSession->hSocket = hClient;
				memcpy(&pSession->xPeer, &xClient, sizeof(xClient));
				nRet = pthread_create(&pSession->xThread, NULL, FTDMS_service, pSession);
				if (nRet != 0)
				{
					ERROR("Can't create a thread[%d]\n", nRet);
					FTM_MEM_free(pSession);			
				}
			}
		}
		usleep(10000);
	}

	FTDM_SESSION_PTR pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pSession->bStop = FTM_TRUE;
		shutdown(pSession->hSocket, SHUT_RD);
		pthread_join(pSession->xThread, 0);

		FTM_MEM_free(pSession);
	}

error:

	return	0;
}

FTM_VOID_PTR FTDMS_service(FTM_VOID_PTR pData)
{
	FTDM_SESSION_PTR		pSession= (FTDM_SESSION_PTR)pData;
	FTDM_REQ_PARAMS_PTR		pReq 	= (FTDM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTDM_RESP_PARAMS_PTR	pResp 	= (FTDM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timespec			xTimeout = { .tv_sec = 2, .tv_nsec = 0};

	if (sem_timedwait(&pSession->pServer->xSemaphore, &xTimeout) < 0)
	{
		TRACE("The session(%08x) was closed\n", pSession->hSocket);
		shutdown(pSession->hSocket, SHUT_RD);
		return	0;	
	}

	if (sem_init(&pSession->xSemaphore, 0, 1) < 0)
	{
		ERROR("Can't alloc semaphore!\n");
		return	0;	
	}

	FTM_LIST_append(&pSession->pServer->xSessionList, pSession);	

	while(!pSession->bStop)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		//TRACE("recv(%08x, pReq, %lu, MSG_DONTWAIT)\n", pSession->hSocket, nLen);
		if (nLen == 0)
		{
			TRACE("The connection is terminated.\n");
			break;	
		}
		else if (nLen < 0)
		{
			ERROR("recv failed[%d]\n", -nLen);
			break;	
		}

		if (FTM_RET_OK != FTDMS_serviceCall(pSession->pServer, pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->nRet = FTM_RET_INTERNAL_ERROR;
			pResp->nLen = sizeof(FTDM_RESP_PARAMS);
		}

		//TRACE("send(%08x, pResp, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->nLen);
		nLen = send(pSession->hSocket, pResp, pResp->nLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			ERROR("send failed[%d]\n", -nLen);	
			break;
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	sem_destroy(&pSession->xSemaphore);

	FTM_LIST_remove(&pSession->pServer->xSessionList, (FTM_VOID_PTR)&pSession->xThread);	

	sem_post(&pSession->pServer->xSemaphore);

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

	ERROR("FUNCTION NOT SUPPORTED\n");
	ERROR("CMD : %08lx\n", pReq->xCmd);
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
			FTDM_NODE_destroy(&pNode);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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


	xRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODEM_remove(pServer->pDM->pNodeM, pNode);
		if (xRet == FTM_RET_OK)
		{
			FTDM_NODE_destroy(&pNode);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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
	pResp->nRet = FTDM_NODEM_count(pServer->pDM->pNodeM, &pResp->nCount);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->nRet;
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
	pResp->nRet = FTDM_NODEM_getAt(pServer->pDM->pNodeM, pReq->nIndex, &pNode);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->nRet;
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
	pResp->nRet = xRet;

	return	pResp->nRet;
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
	pResp->nRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->xType = pNode->xInfo.xType;
	}

	return	pResp->nRet;
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
	pResp->nRet = FTDM_NODEM_get(pServer->pDM->pNodeM, pReq->pDID, &pNode);

	if (pResp->nRet == FTM_RET_OK)
	{
	}

	return	pResp->nRet;
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
	//pResp->nRet = FTDM_setNodeURL(pReq->pDID, pReq->pURL);

	return	pResp->nRet;
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
	//pResp->nRet = FTDM_getNodeLocation(
	//				pReq->pDID, 
	//				pResp->pLocation, 
	//				pResp->nLocationLen);

	return	pResp->nRet;
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
	//pResp->nRet = FTDM_setNodeLocation(
	//				pReq->pDID, 
	//				pReq->pLocation);

	return	pResp->nRet;
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
	pResp->nRet = xRet;

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_del
(
	FTDM_SERVER_PTR					pServer,
 	FTDM_REQ_EP_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET		xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EPM_remove(pServer->pDM->pEPM, pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTDM_EP_destroy(&pEP);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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
	pResp->nRet = FTDM_EPM_count(pServer->pDM->pEPM, pReq->xType, &pResp->nCount);
	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_get(pEP, &pResp->xInfo);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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
	pResp->nRet = xRet;

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xInfo.xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_set(pEP, &pReq->xInfo);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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
	pResp->nRet = FTDM_EP_CLASS_add(&pReq->xInfo);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_EP_CLASS_del(pReq->xType);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_EP_CLASS_count(&pResp->nCount);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_EP_CLASS_get(pReq->xEPClass, &pResp->xInfo);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_EP_CLASS_getAt(pReq->nIndex, &pResp->xInfo);

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_add(pEP, &pReq->xData);
	
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_info(pEP, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_get( pEP, pReq->nStartIndex, pResp->pData, pReq->nCount, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = xRet;

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_PARAMS);
	}
	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_getWithTime(
					pEP, 
					pReq->nBeginTime, 
					pReq->nEndTime, 
					pResp->pData, 
					pReq->nCount, 
					&pResp->nCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = xRet;

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS);
	}
	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_del(pEP, pReq->nIndex, pReq->nCount);
	}

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_delWithTime( pEP, pReq->nBeginTime, pReq->nEndTime);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_count( pEP, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
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

	xRet = FTDM_EPM_get(pServer->pDM->pEPM, pReq->xEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_DATA_countWithTime(pEP, pReq->nBeginTime, pReq->nEndTime, &pResp->nCount);
	}
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = xRet;

	return	pResp->nRet;
}

FTM_RET	FTDMS_TRIGGER_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_TRIGGER_create(&pReq->xTrigger);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_TRIGGER_destroy(pReq->xID);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_TRIGGER_count(&pResp->nCount);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_TRIGGER_get(pReq->xID, &pTrigger);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	pResp->nRet;
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
	pResp->nRet = FTDM_TRIGGER_getAt(pReq->nIndex, &pTrigger);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xTrigger, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_ACTION_add
(
	FTDM_SERVER_PTR					pServer,
	FTDM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_ACTION_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_ACTION_create(&pReq->xAction);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_ACTION_destroy(pReq->xID);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_ACTION_count(&pResp->nCount);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_ACTION_get(pReq->xID, &pAction);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));
	}

	return	pResp->nRet;
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
	pResp->nRet = FTDM_ACTION_getAt(pReq->nIndex, &pAction);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xAction, &pAction->xInfo, sizeof(FTM_ACTION));
	}

	return	pResp->nRet;
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
	pResp->nRet = FTDM_RULE_create(&pReq->xRule);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_RULE_destroy(pReq->xID);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_RULE_count(&pResp->nCount);

	return	pResp->nRet;
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
	pResp->nRet = FTDM_RULE_get(pReq->xID, &pRule);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_RULE));
	}

	return	pResp->nRet;
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
	pResp->nRet = FTDM_RULE_getAt(pReq->nIndex, &pRule);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xRule, &pRule->xInfo, sizeof(FTM_RULE));
	}

	return	pResp->nRet;
}
