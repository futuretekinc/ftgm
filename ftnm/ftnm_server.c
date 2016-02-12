#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include "ftnm.h"
#include "ftnm_config.h"
#include "ftnm_params.h"
#include "ftnm_server.h"
#include "ftnm_server_cmd.h"

#define	FTNM_PACKET_LEN					2048

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTNM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTNM_PACKET_LEN];
}	FTNM_SESSION, _PTR_ FTNM_SESSION_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTNM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTNMS_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTNMS_serviceHandler(FTM_VOID_PTR pData);

static FTNMS_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTNM_CMD_NODE_CREATE,		FTNMS_NODE_create),
	MK_CMD_SET(FTNM_CMD_NODE_DESTROY,		FTNMS_NODE_destroy),
	MK_CMD_SET(FTNM_CMD_NODE_COUNT,			FTNMS_NODE_count),
	MK_CMD_SET(FTNM_CMD_NODE_GET,			FTNMS_NODE_get),
	MK_CMD_SET(FTNM_CMD_NODE_GET_AT,		FTNMS_NODE_getAt),
	MK_CMD_SET(FTNM_CMD_EP_CREATE,			FTNMS_EP_create),
	MK_CMD_SET(FTNM_CMD_EP_DESTROY,			FTNMS_EP_destroy),
	MK_CMD_SET(FTNM_CMD_EP_COUNT,			FTNMS_EP_count),
	MK_CMD_SET(FTNM_CMD_EP_GET_LIST,		FTNMS_EP_getList),
	MK_CMD_SET(FTNM_CMD_EP_GET,				FTNMS_EP_get),
	MK_CMD_SET(FTNM_CMD_EP_GET_AT,			FTNMS_EP_getAt),
	MK_CMD_SET(FTNM_CMD_EP_DATA_INFO,		FTNMS_EP_DATA_info),
	MK_CMD_SET(FTNM_CMD_EP_DATA_GET_LAST,	FTNMS_EP_DATA_getLast),
	MK_CMD_SET(FTNM_CMD_EP_DATA_COUNT,		FTNMS_EP_DATA_count),
	MK_CMD_SET(FTNM_CMD_UNKNOWN, 		NULL)
};

static sem_t		xSemaphore;
static pthread_t	xPThread;

FTM_RET	FTNMS_run(FTNM_CFG_SERVER_PTR pConfig, pthread_t *pPThread )
{
	int	nRet;

	nRet = pthread_create(&xPThread, NULL, FTNMS_process, (void *)pConfig);
	if (nRet != 0)
	{
		ERROR("Can't create thread[%d]\n", nRet);
		return	FTM_RET_CANT_CREATE_THREAD;
	}

	if (pPThread != NULL)
	{
		*pPThread = xPThread;	
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNMS_process(FTM_VOID_PTR pData)
{
	FTM_INT				nRet;
	FTM_INT				hSocket;
	struct sockaddr_in	xServer, xClient;
	FTNM_CFG_SERVER_PTR pServer = (FTNM_CFG_SERVER_PTR)pData;


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


	while(1)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrIulLen = sizeof(struct sockaddr_in);	

		sem_getvalue(&xSemaphore, &nValue);
		MESSAGE("Waiting for connections ...[%d]\n", nValue);
		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrIulLen);
		if (hClient != 0)
		{
			pthread_t xPthread;	

			TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClient.sin_addr), ntohs(xClient.sin_port));

			FTNM_SESSION_PTR pSession = (FTNM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTNM_SESSION));
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
				pthread_create(&xPthread, NULL, FTNMS_serviceHandler, pSession);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNMS_serviceHandler(FTM_VOID_PTR pData)
{
	FTNM_SESSION_PTR		pSession= (FTNM_SESSION_PTR)pData;
	FTNM_REQ_PARAMS_PTR		pReq 	= (FTNM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTNM_RESP_PARAMS_PTR	pResp 	= (FTNM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timespec			xTimeout = { .tv_sec = 2, .tv_nsec = 0};

	if (sem_timedwait(&xSemaphore, &xTimeout) < 0)
	{
		TRACE("The session(%08x) was closed\n", pSession->hSocket);
		shutdown(pSession->hSocket, SHUT_RD);
		return	0;	
	}

	while(1)
	{
		int	ulLen;

		ulLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		TRACE("recv(%08x, pReq, %lu, MSG_DONTWAIT)\n", pSession->hSocket, ulLen);
		if (ulLen == 0)
		{
			TRACE("The connection is terminated.\n");
			break;	
		}
		else if (ulLen < 0)
		{
			ERROR("recv failed[%d]\n", -ulLen);
			break;	
		}

		if (FTM_RET_OK != FTNMS_serviceCall(pReq, pResp))
		{
			pResp->xCmd = pReq->xCmd;
			pResp->nRet = FTM_RET_INTERNAL_ERROR;
			pResp->ulLen = sizeof(FTNM_RESP_PARAMS);
		}

		TRACE("send(%08x, pResp, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->ulLen);
		ulLen = send(pSession->hSocket, pResp, pResp->ulLen, MSG_DONTWAIT);
		if (ulLen < 0)
		{
			ERROR("send failed[%d]\n", -ulLen);	
			break;
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	sem_post(&xSemaphore);

	return	0;
}

FTM_RET	FTNMS_serviceCall
(
	FTNM_REQ_PARAMS_PTR		pReq,
	FTNM_RESP_PARAMS_PTR	pResp
)
{
	FTM_RET				nRet;
	FTNMS_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTNM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
			TRACE("CMD : %s\n", pSet->pCmdString);
			nRet = pSet->fService(pReq, pResp);
			TRACE("RET : %08lx\n", nRet);
			return	nRet;
		}

		pSet++;
	}

	ERROR("FUNCTION NOT SUPPORTED\n");
	ERROR("CMD : %08lx\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTNMS_NODE_create
(
	FTNM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_CREATE_PARAMS_PTR	pResp
)
{
	FTNM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_NODE_create(&pReq->xNodeInfo, &pNode);

	return	pResp->nRet;
}


FTM_RET	FTNMS_NODE_destroy
(
 	FTNM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTNM_RESP_NODE_DESTROY_PARAMS_PTR	pResp
)
{
	FTNM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_NODE_get(pReq->pDID, &pNode);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nRet = FTNM_NODE_destroy(pNode);
	}

	return	pResp->nRet;
}

FTM_RET	FTNMS_NODE_count
(
 	FTNM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_NODE_count(&pResp->ulCount);

	return	pResp->nRet;
}

FTM_RET	FTNMS_NODE_get
(
 	FTNM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTNM_RESP_NODE_GET_PARAMS_PTR	pResp
)
{
	FTNM_NODE_PTR	pNode;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_NODE_get(pReq->pDID, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTNMS_NODE_getAt
(
 	FTNM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTNM_RESP_NODE_GET_AT_PARAMS_PTR	pResp
)
{
	FTNM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_NODE_getAt(pReq->ulIndex, &pNode);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_create
(
 	FTNM_REQ_EP_CREATE_PARAMS_PTR	pReq,
	FTNM_RESP_EP_CREATE_PARAMS_PTR	pResp
)
{
	FTNM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_create(&pReq->xInfo, &pEP);

	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_destroy
(
 	FTNM_REQ_EP_DESTROY_PARAMS_PTR	pReq,
	FTNM_RESP_EP_DESTROY_PARAMS_PTR	pResp
)
{
	FTNM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_get(pReq->xEPID, &pEP);
	
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nRet = FTNM_EP_destroy(pEP);
	}

	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_count
(
 	FTNM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTNM_RESP_EP_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_count(pReq->xClass, &pResp->nCount);
	TRACE("EP COUNT : %d\n", pResp->nCount);
	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_get
(
 	FTNM_REQ_EP_GET_PARAMS_PTR		pReq,
	FTNM_RESP_EP_GET_PARAMS_PTR		pResp
)
{
	FTNM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_get(pReq->xEPID, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_getList
(
 	FTNM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTNM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nRet = FTNM_EP_getList(pReq->xClass, pResp->pEPIDList, pReq->ulMaxCount, &pResp->ulCount);
	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EPID) * pResp->ulCount;

	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_getAt
(
 	FTNM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTNM_RESP_EP_GET_AT_PARAMS_PTR	pResp
)
{
	FTNM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_getAt(pReq->ulIndex, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP_INFO));
	}
	
	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_DATA_info
(
	FTNM_REQ_EP_DATA_INFO_PARAMS_PTR pReq,
	FTNM_RESP_EP_DATA_INFO_PARAMS_PTR pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_DATA_info(pReq->xEPID, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);

	return	pResp->nRet;
}
FTM_RET	FTNMS_EP_DATA_getLast
(
	FTNM_REQ_EP_DATA_GET_LAST_PARAMS_PTR pReq,
	FTNM_RESP_EP_DATA_GET_LAST_PARAMS_PTR pResp
)
{
	FTNM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_get(pReq->xEPID, &pEP);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xData, &pEP->xData, sizeof(FTM_EP_DATA));
	}

	return	pResp->nRet;
}

FTM_RET	FTNMS_EP_DATA_count
(
	FTNM_REQ_EP_DATA_COUNT_PARAMS_PTR pReq,
	FTNM_RESP_EP_DATA_COUNT_PARAMS_PTR pResp
)
{
	FTNM_EP_PTR	pEP;
	FTM_ULONG	ulCount = 0;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_EP_DATA_count(pReq->xEPID, &ulCount);
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->ulCount = ulCount;
	}

	return	pResp->nRet;
}
