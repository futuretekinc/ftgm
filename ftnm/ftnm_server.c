#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftnm.h"
#include "ftnm_params.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_server.h"
#include "ftnm_server_cmd.h"

#define	FTNM_PACKET_LEN					2048

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTM_BYTE			pReqBuff[FTNM_PACKET_LEN];
	FTM_BYTE			pRespBuff[FTNM_PACKET_LEN];
}	FTNM_SESSION, _PTR_ FTNM_SESSION_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTNM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTNM_SRV_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTNM_SRV_serviceHandler(FTM_VOID_PTR pData);

static FTNM_SRV_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTNM_CMD_NODE_CREATE,		FTNM_SRV_NODE_create),
	MK_CMD_SET(FTNM_CMD_NODE_DESTROY,		FTNM_SRV_NODE_destroy),
	MK_CMD_SET(FTNM_CMD_NODE_COUNT,			FTNM_SRV_NODE_count),
	MK_CMD_SET(FTNM_CMD_NODE_GET,			FTNM_SRV_NODE_get),
	MK_CMD_SET(FTNM_CMD_NODE_GET_AT,		FTNM_SRV_NODE_getAt),
	MK_CMD_SET(FTNM_CMD_EP_CREATE,			FTNM_SRV_EP_create),
	MK_CMD_SET(FTNM_CMD_EP_DESTROY,			FTNM_SRV_EP_destroy),
	MK_CMD_SET(FTNM_CMD_EP_COUNT,			FTNM_SRV_EP_count),
	MK_CMD_SET(FTNM_CMD_EP_GET_LIST,		FTNM_SRV_EP_getList),
	MK_CMD_SET(FTNM_CMD_EP_GET,				FTNM_SRV_EP_get),
	MK_CMD_SET(FTNM_CMD_EP_GET_AT,			FTNM_SRV_EP_getAt),
	MK_CMD_SET(FTNM_CMD_EP_DATA_INFO,		FTNM_SRV_EP_DATA_info),
	MK_CMD_SET(FTNM_CMD_EP_DATA_GET_LAST,	FTNM_SRV_EP_DATA_getLast),
	MK_CMD_SET(FTNM_CMD_EP_DATA_COUNT,		FTNM_SRV_EP_DATA_count),
	MK_CMD_SET(FTNM_CMD_UNKNOWN, 		NULL)
};

FTM_RET	FTNM_SRV_init
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNM_SRV_final
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNM_SRV_create
(
	FTNM_SERVER_PTR _PTR_	ppServer 
)
{
	ASSERT(ppServer != NULL);
	FTNM_SERVER_PTR	pServer;

	pServer = (FTNM_SERVER_PTR)FTM_MEM_malloc(sizeof(FTNM_SERVER));
	if (pServer == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pServer, 0, sizeof(FTNM_SERVER));

	memset(&pSRV->xConfig, 0, sizeof(FTNM_SRV_CONFIG));

	pSRV->xConfig.usPort		= FTNM_DEFAULT_SERVER_PORT;
	pSRV->xConfig.ulMaxSession	= FTNM_DEFAULT_SERVER_SESSION_COUNT	;

	FTM_LIST_init(&pServer->xSessionList);

	*ppServer = pServer;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SRV_destroy
(
	FTNM_SERVER_PTR 		pServer 
)
{
	ASSERT(pServer != NULL);

	FTNM_SRV_stop(pServer);

	FTM_LIST_final(&pServer->xSessionList);

	FTM_MEM_free(pServer);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SRV_start(FTNM_SERVER_PTR pServer)
{
	ASSERT(pServer != NULL);

	int	nRet;

	nRet = pthread_create(&pServer->xPThread, NULL, FTNM_SRV_process, (FTM_VOID_PTR)pServer);
	if (nRet != 0)
	{
		ERROR("Can't create thread[%d]\n", nRet);
		return	FTM_RET_CANT_CREATE_THREAD;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SRV_stop(FTNM_SERVER_PTR pServer)
{

	ASSERT(pServer != NULL);
	FTM_VOID_PTR	pRes;

	FTNM_SESSION_PTR	pSession = NULL;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, &pRes);

		FTM_MEM_free(pSession);		
	}

	pthread_cancel(pServer->xPThread);
	pthread_join(pServer->xPThread, &pRes);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_SRV_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_INT				nRet;
	FTM_INT				hSocket;
	struct sockaddr_in	xServerAddr, xClientAddr;
	FTNM_SERVER_PTR 	pServer = (FTNM_SERVER_PTR)pData;


	if (sem_init(&pServer->xSemaphore, 0,pServer->xConfig.ulMaxSession) < 0)
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

		if (sem_timedwait(&pServer->xSemaphore, &xTimeout) == 0)
		{
			sem_getvalue(&pServer->xSemaphore, &nValue);
			MESSAGE("Waiting for connections ...[%d]\n", nValue);
			hClient = accept(hSocket, (struct sockaddr *)&xClientAddr, (socklen_t *)&nSockAddrIulLen);
			if (hClient != 0)
			{
				TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));

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
					memcpy(&pSession->xPeer, &xClientAddr, sizeof(xClientAddr));
					pthread_create(&pSession->xPThread, NULL, FTNM_SRV_serviceHandler, pSession);
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

FTM_VOID_PTR FTNM_SRV_serviceHandler(FTM_VOID_PTR pData)
{
	FTNM_SESSION_PTR		pSession= (FTNM_SESSION_PTR)pData;
	FTNM_REQ_PARAMS_PTR		pReq 	= (FTNM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTNM_RESP_PARAMS_PTR	pResp 	= (FTNM_RESP_PARAMS_PTR)pSession->pRespBuff;


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

		if (FTM_RET_OK != FTNM_SRV_serviceCall(pReq, pResp))
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

	return	0;
}

FTM_RET	FTNM_SRV_serviceCall
(
	FTNM_REQ_PARAMS_PTR		pReq,
	FTNM_RESP_PARAMS_PTR	pResp
)
{
	FTM_RET				nRet;
	FTNM_SRV_CMD_SET_PTR	pSet = pCmdSet;

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

FTM_RET	FTNM_SRV_NODE_create
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


FTM_RET	FTNM_SRV_NODE_destroy
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

FTM_RET	FTNM_SRV_NODE_count
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

FTM_RET	FTNM_SRV_NODE_get
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

FTM_RET	FTNM_SRV_NODE_getAt
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

FTM_RET	FTNM_SRV_EP_create
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

FTM_RET	FTNM_SRV_EP_destroy
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

FTM_RET	FTNM_SRV_EP_count
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

FTM_RET	FTNM_SRV_EP_get
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

FTM_RET	FTNM_SRV_EP_getList
(
 	FTNM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTNM_RESP_EP_GET_LIST_PARAMS_PTR	pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->nRet = FTNM_EP_getIDList(pReq->xClass, pResp->pEPIDList, pReq->ulMaxCount, &pResp->ulCount);
	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EPID) * pResp->ulCount;

	return	pResp->nRet;
}

FTM_RET	FTNM_SRV_EP_getAt
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

FTM_RET	FTNM_SRV_EP_DATA_info
(
	FTNM_REQ_EP_DATA_INFO_PARAMS_PTR pReq,
	FTNM_RESP_EP_DATA_INFO_PARAMS_PTR pResp
)
{
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_getEPDataInfo(pReq->xEPID, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);

	return	pResp->nRet;
}
FTM_RET	FTNM_SRV_EP_DATA_getLast
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

FTM_RET	FTNM_SRV_EP_DATA_count
(
	FTNM_REQ_EP_DATA_COUNT_PARAMS_PTR pReq,
	FTNM_RESP_EP_DATA_COUNT_PARAMS_PTR pResp
)
{
	FTM_ULONG	ulCount = 0;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->nRet = FTNM_getEPDataCount(pReq->xEPID, &ulCount);
	if (pResp->nRet == FTM_RET_OK)
	{
		pResp->ulCount = ulCount;
	}

	return	pResp->nRet;
}

FTM_RET FTNM_SRV_loadConfig(FTNM_SERVER_PTR pSRV, FTM_CHAR_PTR pFileName)
{
	ASSERT(pSRV != NULL);
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
			pSRV->xConfig.ulMaxSession = (FTM_ULONG)config_setting_get_int(pField);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pSRV->xConfig.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SRV_showConfig(FTNM_SERVER_PTR pSRV)
{
	ASSERT(pSRV != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "PORT", pSRV->xConfig.usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pSRV->xConfig.ulMaxSession);

	return	FTM_RET_OK;
}
