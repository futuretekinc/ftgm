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

#define	FTDM_PACKET_LEN					2048

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	FTM_BYTE			pReqBuff[FTDM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTDM_PACKET_LEN];
}	FTDM_SESSION, _PTR_ FTDM_SESSION_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTDM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTDMS_startDaemon(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTDMS_serviceHandler(FTM_VOID_PTR pData);

static FTDMS_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTDM_CMD_ADD_NODE_INFO,				FTDMS_addNodeInfo ),
	MK_CMD_SET(FTDM_CMD_DEL_NODE_INFO,				FTDMS_delNodeInfo ),
	MK_CMD_SET(FTDM_CMD_GET_NODE_INFO_COUNT,		FTDMS_getNodeInfoCount ),
	MK_CMD_SET(FTDM_CMD_GET_NODE_INFO,				FTDMS_getNodeInfo ),
	MK_CMD_SET(FTDM_CMD_GET_NODE_INFO_BY_INDEX,		FTDMS_getNodeInfoByIndex ),
	MK_CMD_SET(FTDM_CMD_ADD_EP_INFO,				FTDMS_addEPInfo ),
	MK_CMD_SET(FTDM_CMD_DEL_EP_INFO,				FTDMS_delEPInfo ),
	MK_CMD_SET(FTDM_CMD_GET_EP_INFO_COUNT,			FTDMS_getEPInfoCount ),
	MK_CMD_SET(FTDM_CMD_GET_EP_INFO,				FTDMS_getEPInfo ),
	MK_CMD_SET(FTDM_CMD_GET_EP_INFO_BY_INDEX,		FTDMS_getEPInfoByIndex ),
	MK_CMD_SET(FTDM_CMD_ADD_EP_CLASS_INFO,			FTDMS_addEPClassInfo ),
	MK_CMD_SET(FTDM_CMD_DEL_EP_CLASS_INFO,			FTDMS_delEPClassInfo ),
	MK_CMD_SET(FTDM_CMD_GET_EP_CLASS_INFO,			FTDMS_getEPClassInfo ),
	MK_CMD_SET(FTDM_CMD_ADD_EP_DATA,				FTDMS_addEPData ),
	MK_CMD_SET(FTDM_CMD_DEL_EP_DATA,				FTDMS_delEPData),
	MK_CMD_SET(FTDM_CMD_DEL_EP_DATA_WITH_TIME,		FTDMS_delEPDataWithTime),
	MK_CMD_SET(FTDM_CMD_GET_EP_DATA,				FTDMS_getEPData),
	MK_CMD_SET(FTDM_CMD_GET_EP_DATA_COUNT,			FTDMS_getEPDataCount),
	MK_CMD_SET(FTDM_CMD_GET_EP_DATA_COUNT_WITH_TIME,FTDMS_getEPDataCountWithTime),
	MK_CMD_SET(FTDM_CMD_UNKNOWN, 					NULL)
};

static sem_t		xSemaphore;
static pthread_t	xPThread;

FTM_RET	FTDMS_run(FTDM_CFG_SERVER_PTR pConfig, pthread_t *pPThread )
{
	pthread_create(&xPThread, NULL, FTDMS_startDaemon, (void *)pConfig);

	if (pPThread != NULL)
	{
		*pPThread = xPThread;	
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDMS_startDaemon(FTM_VOID_PTR pData)
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


	while(1)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrInLen = sizeof(struct sockaddr_in);	

		sem_getvalue(&xSemaphore, &nValue);
		MESSAGE("Waiting for connections ...[%d]\n", nValue);
		hClient = accept(hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient != 0)
		{
			pthread_t xPthread;	

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
				pthread_create(&xPthread, NULL, FTDMS_serviceHandler, pSession);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTDMS_serviceHandler(FTM_VOID_PTR pData)
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

	while(1)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		TRACE("recv(%08x, pReq, %lu, MSG_DONTWAIT)\n", pSession->hSocket, nLen);
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

		TRACE("send(%08x, pResp, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->nLen);
		nLen = send(pSession->hSocket, pResp, pResp->nLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			ERROR("send failed[%d]\n", -nLen);	
			break;
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

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
			TRACE("CMD : %s\n", pSet->pCmdString);
			return	pSet->fService(pReq, pResp);
		}

		pSet++;
	}

	ERROR("FUNCTION NOT SUPPORTED\n");
	ERROR("CMD : %08lx\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTDMS_addNodeInfo
(
	FTDM_REQ_ADD_NODE_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_NODE_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addNodeInfo(&pReq->xNodeInfo);

	return	pResp->nRet;
}


FTM_RET	FTDMS_delNodeInfo
(
 	FTDM_REQ_DEL_NODE_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_NODE_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delNodeInfo(pReq->pDID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeInfoCount
(
 	FTDM_REQ_GET_NODE_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfoCount(&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeInfo
(
 	FTDM_REQ_GET_NODE_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_GET_NODE_INFO_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfo(pReq->pDID, &pNodeInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, pNodeInfo, sizeof(FTM_NODE_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getNodeInfoByIndex
(
 	FTDM_REQ_GET_NODE_INFO_BY_INDEX_PARAMS_PTR	pReq,
	FTDM_RESP_GET_NODE_INFO_BY_INDEX_PARAMS_PTR	pResp
)
{
	FTM_NODE_INFO_PTR	pNodeInfo;

	pResp->xCmd	= pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getNodeInfoByIndex(pReq->nIndex, &pNodeInfo);

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
	pResp->nRet = FTDM_getNodeInfo(pReq->pDID, &pNodeInfo);

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
	pResp->nRet = FTDM_getNodeInfo(pReq->pDID, &pNodeInfo);

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

FTM_RET	FTDMS_addEPInfo
(
 	FTDM_REQ_ADD_EP_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_EP_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPInfo(&pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_delEPInfo
(
 	FTDM_REQ_DEL_EP_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPInfo(pReq->xEPID);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPInfoCount
(
 	FTDM_REQ_GET_EP_INFO_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_INFO_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfoCount(pReq->xClass, &pResp->nCount);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPInfo
(
 	FTDM_REQ_GET_EP_INFO_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_INFO_PARAMS_PTR	pResp
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfo(
					pReq->xEPID, 
					&pEPInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, pEPInfo, sizeof(FTM_EP_INFO));
	}

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPInfoByIndex
(
 	FTDM_REQ_GET_EP_INFO_BY_INDEX_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_INFO_BY_INDEX_PARAMS_PTR	pResp
)
{
	FTM_EP_INFO_PTR	pEPInfo;

	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPInfoByIndex(
					pReq->nIndex, 
					&pEPInfo);
	if (pResp->nRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, pEPInfo, sizeof(FTM_EP_INFO));
	}
	return	pResp->nRet;
}

FTM_RET	FTDMS_addEPClassInfo
(
 	FTDM_REQ_ADD_EP_CLASS_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_EP_CLASS_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPClassInfo(&pReq->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_delEPClassInfo
(
 	FTDM_REQ_DEL_EP_CLASS_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_CLASS_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPClassInfo(pReq->xClass);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPClassInfo
(
 	FTDM_REQ_GET_EP_CLASS_INFO_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_CLASS_INFO_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPClassInfo(pReq->xClass, &pResp->xInfo);

	return	pResp->nRet;
}

FTM_RET	FTDMS_addEPData
(
 	FTDM_REQ_ADD_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_ADD_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_addEPData(pReq->xEPID, &pReq->xData);

	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPData
(
 	FTDM_REQ_GET_EP_DATA_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_DATA_PARAMS_PTR	pResp
)
{
	TRACE("Request Frame\n");
	TRACE("%16s : %08lx\n", "Command", pReq->xCmd);
	TRACE("%16s : %08lx\n", "Length", pReq->nLen);
	TRACE("%16s : %08lx\n", "EPID", pReq->xEPID);
	TRACE("%16s : %08lx\n", "Start Index", pReq->nStartIndex);

	pResp->xCmd = pReq->xCmd;
	pResp->nCount = pReq->nCount;
	pResp->nRet = FTDM_getEPData(
					pReq->xEPID, 
					pReq->nStartIndex,
					pResp->pData, 
					pReq->nCount, 
					&pResp->nCount);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_PARAMS);
	}
	return	pResp->nRet;
}

FTM_RET	FTDMS_getEPDataWithTime
(
 	FTDM_REQ_GET_EP_DATA_WITH_TIME_PARAMS_PTR		pReq,
	FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS_PTR	pResp
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
	pResp->nRet = FTDM_getEPDataWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime, 
					pResp->pData, 
					pReq->nCount, 
					&pResp->nCount);

	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS) + pResp->nCount * sizeof(FTM_EP_DATA);
	}
	else
	{
		pResp->nLen = sizeof(FTDM_RESP_GET_EP_DATA_WITH_TIME_PARAMS);
	}
	return	pResp->nRet;
}

FTM_RET 	FTDMS_delEPData
(
 	FTDM_REQ_DEL_EP_DATA_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_DATA_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPData(
					pReq->xEPID, 
					pReq->nIndex, 
					pReq->nCount);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_delEPDataWithTime
(
 	FTDM_REQ_DEL_EP_DATA_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_DEL_EP_DATA_WITH_TIME_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_delEPDataWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_getEPDataCount
(
 	FTDM_REQ_GET_EP_DATA_COUNT_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_DATA_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPDataCount(
					pReq->xEPID, 
					&pResp->nCount);

	return	pResp->nRet;
}

FTM_RET 	FTDMS_getEPDataCountWithTime
(
 	FTDM_REQ_GET_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pReq,
	FTDM_RESP_GET_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nLen = sizeof(*pResp);
	pResp->nRet = FTDM_getEPDataCountWithTime(
					pReq->xEPID, 
					pReq->nBeginTime, 
					pReq->nEndTime,
					&pResp->nCount);

	return	pResp->nRet;
}

