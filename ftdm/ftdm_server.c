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
#include "ftdm_event.h"

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
	MK_CMD_SET(FTDM_CMD_NODE_INFO_ADD,				FTDMS_NODE_INFO_add ),
	MK_CMD_SET(FTDM_CMD_NODE_INFO_DEL,				FTDMS_NODE_INFO_del ),
	MK_CMD_SET(FTDM_CMD_NODE_INFO_COUNT,			FTDMS_NODE_INFO_count ),
	MK_CMD_SET(FTDM_CMD_NODE_INFO_GET,				FTDMS_NODE_INFO_get ),
	MK_CMD_SET(FTDM_CMD_NODE_INFO_GET_AT,			FTDMS_NODE_INFO_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_INFO_ADD,				FTDMS_EP_INFO_add ),
	MK_CMD_SET(FTDM_CMD_EP_INFO_DEL,				FTDMS_EP_INFO_del ),
	MK_CMD_SET(FTDM_CMD_EP_INFO_COUNT,				FTDMS_EP_INFO_count ),
	MK_CMD_SET(FTDM_CMD_EP_INFO_GET,				FTDMS_EP_INFO_get ),
	MK_CMD_SET(FTDM_CMD_EP_INFO_GET_AT,				FTDMS_EP_INFO_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_INFO_ADD,			FTDMS_EP_CLASS_INFO_add),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_INFO_DEL,			FTDMS_EP_CLASS_INFO_del),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_INFO_COUNT,		FTDMS_EP_CLASS_INFO_count ),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_INFO_GET,			FTDMS_EP_CLASS_INFO_get),
	MK_CMD_SET(FTDM_CMD_EP_CLASS_INFO_GET_AT,		FTDMS_EP_CLASS_INFO_getAt ),
	MK_CMD_SET(FTDM_CMD_EP_DATA_ADD,				FTDMS_EP_DATA_add ),
	MK_CMD_SET(FTDM_CMD_EP_DATA_INFO,				FTDMS_EP_DATA_info),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL,				FTDMS_EP_DATA_del),
	MK_CMD_SET(FTDM_CMD_EP_DATA_DEL_WITH_TIME,		FTDMS_EP_DATA_delWithTime),
	MK_CMD_SET(FTDM_CMD_EP_DATA_GET,				FTDMS_EP_DATA_get),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT,				FTDMS_EP_DATA_count),
	MK_CMD_SET(FTDM_CMD_EP_DATA_COUNT_WITH_TIME,	FTDMS_EP_DATA_countWithTime),
	MK_CMD_SET(FTDM_CMD_EVENT_ADD,					FTDMS_EVENT_add ),
	MK_CMD_SET(FTDM_CMD_EVENT_DEL,					FTDMS_EVENT_del ),
	MK_CMD_SET(FTDM_CMD_EVENT_COUNT,				FTDMS_EVENT_count ),
	MK_CMD_SET(FTDM_CMD_EVENT_GET,					FTDMS_EVENT_get ),
	MK_CMD_SET(FTDM_CMD_EVENT_GET_AT,				FTDMS_EVENT_getAt ),
	MK_CMD_SET(FTDM_CMD_UNKNOWN, 					NULL)
};

static sem_t		xSemaphore;
static pthread_t	xPThread;
static FTM_LIST		xSessionList;

FTM_RET	FTDMS_run(FTDM_CFG_SERVER_PTR pConfig, pthread_t *pPThread )
{
	FTM_LIST_init(&xSessionList);
	FTM_LIST_setSeeker(&xSessionList, FTDMS_SESSION_LIST_seeker);


	pthread_create(&xPThread, NULL, FTDMS_process, (void *)pConfig);

	if (pPThread != NULL)
	{
		*pPThread = xPThread;	
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDMS_process(FTM_VOID_PTR pData)
{
	FTM_INT				nRet;
	FTM_INT				hSocket;
	struct sockaddr_in	xServer, xClient;
	FTDM_CFG_SERVER_PTR pServer = (FTDM_CFG_SERVER_PTR)pData;


	if (sem_init(&xSemaphore, 0,pServer->ulMaxSession) < 0)
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

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( pServer->usPort );

	nRet = bind( hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
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
		FTM_INT	nSockAddrInLen = sizeof(struct sockaddr_in);	

		sem_getvalue(&xSemaphore, &nValue);
		TRACE("Waiting for connections ...[%d]\n", nValue);
		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient != 0)
		{

			TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClient.sin_addr), ntohs(xClient.sin_port));

			FTDM_SESSION_PTR pSession = (FTDM_SESSION_PTR)malloc(sizeof(FTDM_SESSION));
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
				memcpy(&pSession->xPeer, &xClient, sizeof(xClient));
				pthread_create(&pSession->xPthread, NULL, FTDMS_service, pSession);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDMS_service(FTM_VOID_PTR pData)
{
	FTDM_SESSION_PTR		pSession= (FTDM_SESSION_PTR)pData;
	FTDM_REQ_PARAMS_PTR		pReq 	= (FTDM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTDM_RESP_PARAMS_PTR	pResp 	= (FTDM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timespec			xTimeout = { .tv_sec = 2, .tv_nsec = 0};

	if (sem_timedwait(&xSemaphore, &xTimeout) < 0)
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

	FTM_LIST_append(&xSessionList, pSession);	

	while(FTM_TRUE)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
//		TRACE("recv(%08x, pReq, %lu, MSG_DONTWAIT)\n", pSession->hSocket, nLen);
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

		if (FTM_RET_OK != FTDMS_serviceCall(pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->nRet = FTM_RET_INTERNAL_ERROR;
			pResp->nLen = sizeof(FTDM_RESP_PARAMS);
		}

//		TRACE("send(%08x, pResp, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->nLen);
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

	FTM_LIST_remove(&xSessionList, (FTM_VOID_PTR)&pSession->xPthread);	

	sem_post(&xSemaphore);

	return	0;
}

FTM_RET	FTDMS_serviceCall
(
	FTDM_REQ_PARAMS_PTR		pReq,
	FTDM_RESP_PARAMS_PTR	pResp
)
{
	FTDMS_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTDM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
	//		TRACE("CMD : %s\n", pSet->pCmdString);
			return	pSet->fService(pReq, pResp);
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

	return (pSession->xPthread == *pPThread);
}

FTM_RET	FTDMS_getSessionCount(FTM_ULONG_PTR pulCount)
{
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&xSessionList, pulCount);
}

FTM_RET	FTDMS_getSessionInfo(FTM_ULONG ulIndex, FTDM_SESSION_PTR pSession)
{
	ASSERT(pSession != NULL);
	FTM_RET				xRet;
	FTDM_SESSION_PTR	pElement;

	xRet = FTM_LIST_getAt(&xSessionList, ulIndex, (FTM_VOID_PTR _PTR_)&pElement);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pSession, pElement, sizeof(FTDM_SESSION));
	}

	return	xRet;
}

FTM_RET	FTDMS_NODE_INFO_add
(
	FTDM_REQ_NODE_INFO_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_NODE_INFO_add(&pReq->xNodeInfo);

	return	pResp->nRet;
}


FTM_RET	FTDMS_NODE_INFO_del
(
 	FTDM_REQ_NODE_INFO_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_NODE_INFO_del(pReq->pDID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_NODE_INFO_count
(
 	FTDM_REQ_NODE_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_NODE_INFO_count(&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_NODE_INFO_get
(
 	FTDM_REQ_NODE_INFO_GET_PARAMS_PTR		pReq,
	FTDM_RESP_NODE_INFO_GET_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_NODE_INFO_get(pReq->pDID, &pNodeInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_NODE_INFO_getAt
(
 	FTDM_REQ_NODE_INFO_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_NODE_INFO_GET_AT_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_NODE_INFO_getAt(pReq->nIndex, &pNodeInfo);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeType
(
 	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR		pReq,
	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo = NULL;
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_NODE_INFO_get(pReq->pDID, &pNodeInfo);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->xType = pNodeInfo->xType;
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeURL
(
	FTDM_REQ_GET_NODE_URL_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_URL_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo = NULL;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nURLLen = FTM_URL_LEN;
	pResp->nRet = FTDM_NODE_INFO_get(pReq->pDID, &pNodeInfo);

	if (pResp->nRet == FTM_RET_OK)
	{
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_setNodeURL
(
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

FTM_RET	FTDMS_EP_INFO_add
(
 	FTDM_REQ_EP_INFO_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_INFO_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_INFO_add(&pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_INFO_del
(
 	FTDM_REQ_EP_INFO_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_INFO_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_INFO_del(pReq->xEPID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_INFO_count
(
 	FTDM_REQ_EP_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_INFO_count(pReq->xClass, &pResp->nCount);
	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_INFO_get
(
 	FTDM_REQ_EP_INFO_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_INFO_GET_PARAMS_PTR	pResp
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_INFO_get(
					pReq->xEPID, 
					&pEPInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, pEPInfo, sizeof(FTM_EP_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_INFO_getAt
(
 	FTDM_REQ_EP_INFO_GET_AT_PARAMS_PTR		pReq,
	FTDM_RESP_EP_INFO_GET_AT_PARAMS_PTR	pResp
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_INFO_getAt(
					pReq->nIndex, 
					&pEPInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, pEPInfo, sizeof(FTM_EP_INFO));
	}
	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_CLASS_INFO_add
(
 	FTDM_REQ_EP_CLASS_INFO_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_CLASS_INFO_add(&pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_CLASS_INFO_del
(
 	FTDM_REQ_EP_CLASS_INFO_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_CLASS_INFO_del(pReq->xClass);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_CLASS_INFO_count
(
 	FTDM_REQ_EP_CLASS_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_CLASS_INFO_count(&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_CLASS_INFO_get
(
 	FTDM_REQ_EP_CLASS_INFO_GET_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_GET_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_CLASS_INFO_get(pReq->xEPClass, &pResp->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_CLASS_INFO_getAt
(
 	FTDM_REQ_EP_CLASS_INFO_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_CLASS_INFO_GET_AT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_CLASS_INFO_getAt(pReq->nIndex, &pResp->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_DATA_add
(
 	FTDM_REQ_EP_DATA_ADD_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_DATA_add(pReq->xEPID, &pReq->xData);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_DATA_info
(
 	FTDM_REQ_EP_DATA_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_DATA_info(pReq->xEPID, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EP_DATA_get
(
 	FTDM_REQ_EP_DATA_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_PARAMS_PTR	pResp
)
{
	TRACE("Request Frame\n");
	TRACE("%16s : %08lx\n", "Command", pReq->xCmd);
	TRACE("%16s : %08lx\n", "Length", pReq->nLen);
	TRACE("%16s : %08lx\n", "EPID", pReq->xEPID);
	TRACE("%16s : %08lx\n", "Start Index", pReq->nStartIndex);

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_EP_DATA_get(
					pReq->xEPID, 
					pReq->nStartIndex,
					pResp->pData, 
					pReq->nCount, 
					&pResp->nCount);

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
 	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pResp
)
{
	TRACE("Request Frame\n");
	TRACE("%16s : %08lx\n", "Command", pReq->xCmd);
	TRACE("%16s : %08lx\n", "Length", pReq->nLen);
	TRACE("%16s : %08lx\n", "EPID", pReq->xEPID);
	TRACE("%16s : %08lx\n", "Begin Time", pReq->nBeginTime);
	TRACE("%16s : %08lx\n", "End Time", pReq->nEndTime);

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_EP_DATA_getWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime, 
					pResp->pData, 
					pReq->nCount, 
					&pResp->nCount);

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
 	FTDM_REQ_EP_DATA_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_DATA_del(
					pReq->xEPID, 
					pReq->nIndex, 
					pReq->nCount);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_EP_DATA_delWithTime
(
 	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_DATA_delWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_EP_DATA_count
(
 	FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_DATA_count(
					pReq->xEPID, 
					&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_EP_DATA_countWithTime
(
 	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EP_DATA_countWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime,
					&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EVENT_add
(
	FTDM_REQ_EVENT_ADD_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_ADD_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EVENT_add(&pReq->xEvent);

	return	pResp->nRet;
}


FTM_RET	FTDMS_EVENT_del
(
 	FTDM_REQ_EVENT_DEL_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_DEL_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EVENT_del(pReq->xID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EVENT_count
(
 	FTDM_REQ_EVENT_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EVENT_count(&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_EVENT_get
(
 	FTDM_REQ_EVENT_GET_PARAMS_PTR		pReq,
	FTDM_RESP_EVENT_GET_PARAMS_PTR	pResp
)
{
	FTM_EVENT_PTR	pNodeInfo;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EVENT_get(pReq->xID, &pNodeInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xEvent, pNodeInfo, sizeof(FTM_EVENT));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_EVENT_getAt
(
 	FTDM_REQ_EVENT_GET_AT_PARAMS_PTR	pReq,
	FTDM_RESP_EVENT_GET_AT_PARAMS_PTR	pResp
)
{
	FTM_EVENT_PTR	pNodeInfo;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_EVENT_getAt(pReq->nIndex, &pNodeInfo);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xEvent, pNodeInfo, sizeof(FTM_EVENT));
	}

	return	pResp->nRet;
}

