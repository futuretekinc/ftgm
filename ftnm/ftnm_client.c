#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "ftm.h"
#include "ftnm_client.h"
#include "ftnm_params.h"

static FTM_RET FTNM_CLIENT_request
(
	FTNM_CLIENT_PTR		pClient, 
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pRespBuff,
	FTM_ULONG				ulRespBuffLen,
	FTM_ULONG_PTR			pulRespLen
);

static FTM_RET FTNM_CLIENT_TRANS_init
(
	FTNM_CLIENT_TRANS_PTR			pTrans,
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

FTM_RET FTNM_CLIENT_TRANS_final
(
	FTNM_CLIENT_TRANS_PTR			pTrans
);

static FTM_VOID_PTR	FTNM_CLIENT_process(FTM_VOID_PTR pData);
static FTM_BOOL 	FTNM_CLIENT_transSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTNM_CLIENT_init
(
	FTNM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET	xRet;

	memset(pClient, 0, sizeof(FTNM_CLIENT));

	strcpy(pClient->xConfig.xServer.pHost, "127.0.0.1");
	pClient->xConfig.xServer.usPort = 8889;

	pClient->bStop 		= FTM_TRUE;
	pClient->bConnected = FTM_FALSE;
	pClient->hSock 		= -1;
	pClient->ulTimeout 	= 5000000;

	sem_init(&pClient->xReqLock, 0, 0);
	FTM_LIST_init(&pClient->xTransList);
	FTM_LIST_setSeeker(&pClient->xTransList, FTNM_CLIENT_transSeeker);

	xRet = FTM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Can't create message queue.[%08x]\n", xRet);	
	}
	else
	{
		pClient->bInit = FTM_TRUE;
	}

	return	xRet;
}

FTM_RET	FTNM_CLIENT_final
(
	FTNM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (!pClient->bInit)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	if (!pClient->bStop)
	{
		FTNM_CLIENT_stop(pClient);	
	}

	FTM_LIST_final(&pClient->xTransList);
	sem_destroy(&pClient->xReqLock);
	FTM_MSGQ_destroy(pClient->pMsgQ);

	pClient->pMsgQ = NULL;
	pClient->bInit = FTM_FALSE;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_create
(
	FTNM_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTNM_CLIENT_PTR	pClient;

	pClient = (FTNM_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTNM_CLIENT));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}
	
	memset(pClient, 0, sizeof(FTNM_CLIENT));

	FTNM_CLIENT_init(pClient);

	*ppClient = pClient;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_destroy
(
	FTNM_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	FTNM_CLIENT_final(pClient);

	FTM_MEM_free(pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_start
(
	FTNM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (!pClient->bStop)
	{
		return	FTM_RET_ALREADY_RUNNING;
	}

	if (pthread_create(&pClient->xThread, NULL, FTNM_CLIENT_process, pClient) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_stop
(
	FTNM_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	pClient->bStop = FTM_TRUE;
	pthread_join(pClient->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_CLIENT_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	
	FTM_RET			xRet;
	FTNM_CLIENT_PTR	pClient = (FTNM_CLIENT_PTR)pData;

	pClient->bStop = FTM_FALSE;

	TRACE("Client started.\n");

	while(!pClient->bStop)
	{
		if (!pClient->bConnected)
		{
			xRet = FTNM_CLIENT_connect(pClient);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Server connection failed!\n");	
				sleep(5);
			}
			else
			{
				MESSAGE("Connected to server.\n");	
			}
		}
		else
		{
			FTM_BYTE				pBuff[2048];
			FTNM_RESP_PARAMS_PTR 	pResp = (FTNM_RESP_PARAMS_PTR)pBuff;
	
			int	nLen = recv(pClient->hSock, pResp, 2048, MSG_DONTWAIT);
			if (nLen > 0)
			{
				FTNM_CLIENT_TRANS_PTR	pTrans;
				FTM_ULONG		ulCount = 0;
	
				FTM_LIST_count(&pClient->xTransList, &ulCount);
				if (FTM_LIST_get(&pClient->xTransList, &pResp->ulReqID, (FTM_VOID_PTR _PTR_)&pTrans) == FTM_RET_OK)
				{
					memcpy(pTrans->pResp, pResp, nLen);
					pTrans->ulRespLen = nLen;
	
					sem_post(&pTrans->xDone);
				}
				else if (pResp->ulReqID == 0)
				{
					FTNM_RESP_NOTIFY_PARAMS_PTR pNotify = (FTNM_RESP_NOTIFY_PARAMS_PTR)pResp;
					ERROR("Received Notify [%08x]\n",pNotify->xCmd);	
					if (pClient->fNotifyCallback != NULL)
					{
						pClient->fNotifyCallback(&pNotify->xMsg);	
					}
				}
				else
				{
					ERROR("Invalid ReqID[%lu]\n", pResp->ulReqID);	
				}
			}	
		}

		usleep(1000);
	}

	FTNM_CLIENT_disconnect(pClient);

	TRACE("Client stopped.\n");

	return	0;
}

FTM_RET	FTNM_CLIENT_loadConfig
(
	FTNM_CLIENT_PTR			pClient,
	FTNM_CLIENT_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTNM_CLIENT_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_loadConfigFromFile
(
	FTNM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR 	pFileName
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_connect
(
	FTNM_CLIENT_PTR		pClient
)
{
	ASSERT(pClient != NULL);

	struct sockaddr_in 	xServer;

	if (pClient->bConnected)
	{
		ERROR("Already connected.\n");
		return	FTM_RET_COMM_ALREADY_CONNECTED;	
	}

	ERROR("Try connect to server.\n");
	if (pClient->hSock == -1)
	{
		pClient->hSock = socket(AF_INET, SOCK_STREAM, 0);
		if (pClient->hSock == -1)
		{
			ERROR("Could not create socket.\n");	
			return	FTM_RET_COMM_ERROR;
		}
	}

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr	= inet_addr(pClient->xConfig.xServer.pHost);
	xServer.sin_port 		= htons(pClient->xConfig.xServer.usPort);

	if (connect(pClient->hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		
		return	FTM_RET_ERROR;	
	}

	pClient->bConnected = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET FTNM_CLIENT_disconnect
(
	FTNM_CLIENT_PTR		pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->bConnected)
	{
		if (pClient->hSock != -1)
		{
			TRACE("Socket close.\n");
			close(pClient->hSock);
			pClient->hSock = -1;
		}

		pClient->bConnected = FTM_FALSE;
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTNM_CLIENT_isConnected
(
	FTNM_CLIENT_PTR		pClient,
	FTM_BOOL_PTR		pbConnected
)
{
	ASSERT(pClient != NULL);
	ASSERT(pbConnected != NULL);

	*pbConnected = pClient->bConnected;	

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CLIENT_setNotifyCallback
(
	FTNM_CLIENT_PTR		pClient,
	FTNM_CLIENT_NOTIFY_CALLBACK	pCB
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCB != NULL);

	pClient->fNotifyCallback = pCB;

	return	FTM_RET_OK;
}

FTM_RET FTNM_CLIENT_NODE_create
(
	FTNM_CLIENT_PTR		pClient,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);

	FTM_RET							nRet;
	FTNM_REQ_NODE_CREATE_PARAMS		xReq;
	FTNM_RESP_NODE_CREATE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTNM_CMD_NODE_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE));

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}
	
	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_NODE_destroy
(
	FTNM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR			pDID
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_DESTROY_PARAMS	xReq;
	FTNM_RESP_NODE_DESTROY_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTNM_CMD_NODE_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_NODE_count
(
	FTNM_CLIENT_PTR		pClient,
	FTM_ULONG_PTR			pulCount
)
{
	FTM_RET						nRet;
	FTNM_REQ_NODE_COUNT_PARAMS	xReq;
	FTNM_RESP_NODE_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pulCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTNM_CMD_NODE_COUNT;
	xReq.ulLen	=	sizeof(xReq);
	
	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}	

	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_NODE_getAt
(
	FTNM_CLIENT_PTR		pClient,
	FTM_ULONG				ulIndex,
	FTM_NODE_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_GET_AT_PARAMS		xReq;
	FTNM_RESP_NODE_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}


	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTNM_CMD_NODE_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	
	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}

	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_NODE_get
(
	FTNM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_GET_PARAMS		xReq;
	FTNM_RESP_NODE_GET_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTNM_CMD_NODE_GET;
	xReq.ulLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}
	
	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_EP_create
(
	FTNM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_CREATE_PARAMS	xReq;
	FTNM_RESP_EP_CREATE_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xReq.xCmd	=	FTNM_CMD_EP_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_EP_destroy
(
	FTNM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID
)
{
	FTM_RET					nRet;
	FTNM_REQ_EP_DESTROY_PARAMS	xReq;
	FTNM_RESP_EP_DESTROY_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTNM_CMD_EP_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_EP_count
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_COUNT_PARAMS	xReq;
	FTNM_RESP_EP_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTNM_CMD_EP_COUNT;
	xReq.xType	=	xType;
	xReq.ulLen	=	sizeof(xReq);

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pnCount = xResp.nCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTNM_CLIENT_EP_getList
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_EP_ID_PTR			pEPIDList,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET	nRet;
	FTNM_REQ_EP_GET_LIST_PARAMS		xReq;
	FTM_ULONG						nRespSize = 0;
	FTNM_RESP_EP_GET_LIST_PARAMS_PTR	pResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTNM_RESP_EP_GET_LIST_PARAMS) + sizeof(FTM_EP_ID) * ulMaxCount;
	pResp = (FTNM_RESP_EP_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTNM_CMD_EP_GET_LIST;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xType		=	xType;
	xReq.ulMaxCount	=	ulMaxCount;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize,
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	FTM_RET_ERROR;	
	}

	nRet = pResp->nRet;

	if (nRet == FTM_RET_OK)
	{
		memcpy(pEPIDList, pResp->pEPIDList, sizeof(FTM_EP_ID) * pResp->ulCount);
		*pnCount = pResp->ulCount;
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

FTM_RET FTNM_CLIENT_EP_get
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_EP_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_GET_PARAMS		xReq;
	FTNM_RESP_EP_GET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTNM_CMD_EP_GET;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	return	xResp.nRet;
}

FTM_RET FTNM_CLIENT_EP_getAt
(
	FTNM_CLIENT_PTR		pClient,
	FTM_ULONG				ulIndex,
	FTM_EP_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_GET_AT_PARAMS	xReq;
	FTNM_RESP_EP_GET_AT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTNM_CMD_EP_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	
	return	xResp.nRet;
}


/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTNM_CLIENT_EP_DATA_add
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pEPData
)
{
	FTM_RET							nRet;
	FTNM_REQ_EP_DATA_ADD_PARAMS		xReq;
	FTNM_RESP_EP_DATA_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTNM_CMD_EP_DATA_ADD;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	memcpy(&xReq.xData, pEPData, sizeof(FTM_EP_DATA));

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTNM_CLIENT_EP_DATA_info
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET							nRet;
	FTNM_REQ_EP_DATA_INFO_PARAMS	xReq;
	FTNM_RESP_EP_DATA_INFO_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_INFO;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pulBeginTime 	= xResp.ulBeginTime;
		*pulEndTime 	= xResp.ulEndTime;
		*pCount 		= xResp.ulCount;
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTNM_CLIENT_EP_DATA_getLast
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pData
)
{
	FTM_RET								nRet;
	FTNM_REQ_EP_DATA_GET_LAST_PARAMS	xReq;
	FTNM_RESP_EP_DATA_GET_LAST_PARAMS	xResp;
	FTM_ULONG						ulRespLen;
	
	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pData == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTNM_CMD_EP_DATA_GET_LAST;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pData, &xResp.xData, sizeof(FTM_EP_DATA));
	}
	
	return	xResp.nRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTNM_CLIENT_EP_DATA_getList
(
	FTNM_CLIENT_PTR	pClient,
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	FTM_RET									nRet;
	FTNM_REQ_EP_DATA_GET_LIST_PARAMS		xReq;
	FTM_ULONG								nRespSize = 0;
	FTNM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp = NULL;
	FTM_ULONG								ulRespLen;
	FTM_ULONG								ulRespCount = 0;
	FTM_BOOL								bStop = FTM_FALSE;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	while(!bStop)
	{
		FTM_ULONG	ulReqCount;

		if (nMaxCount > 100)
		{
			ulReqCount = 100;	
		}
		else
		{
			ulReqCount = nMaxCount;	
		}

		nRespSize = sizeof(FTNM_RESP_EP_DATA_GET_LIST_PARAMS) + sizeof(FTM_EP_DATA) * ulReqCount;
		pResp = (FTNM_RESP_EP_DATA_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
	
		xReq.xCmd		=	FTNM_CMD_EP_DATA_GET_LIST;
		xReq.ulLen		=	sizeof(xReq);
		xReq.xEPID		=	xEPID;
		xReq.nStartIndex=	nStartIndex;
		xReq.nCount		=	ulReqCount;
	
		nRet = FTNM_CLIENT_request(
					pClient, 
					(FTM_VOID_PTR)&xReq, 
					sizeof(xReq), 
					(FTM_VOID_PTR)pResp, 
					nRespSize,
					&ulRespLen);
		if (nRet != FTM_RET_OK)
		{
			FTM_MEM_free(pResp);
			return	FTM_RET_ERROR;	
		}
	
		nRet = pResp->nRet;
	
		if (pResp->nRet == FTM_RET_OK)
		{
			FTM_INT	i;
	
			TRACE("pResp->nCount = %d\n", pResp->nCount);
			for( i = 0 ; i < pResp->nCount && i < ulReqCount ; i++)
			{
				memcpy(&pData[ulRespCount + i], &pResp->pData[i], sizeof(FTM_EP_DATA));
			}

			ulRespCount += pResp->nCount;
			nMaxCount -= pResp->nCount;
			nStartIndex += pResp->nCount;

			if ((pResp->nCount != ulReqCount) || (nMaxCount == 0))
			{
				bStop = FTM_TRUE;	
			}
		}
	
		FTM_MEM_free(pResp);
	}

	*pnCount = ulRespCount;

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_del
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
)
{
	FTM_RET							nRet;
	FTNM_REQ_EP_DATA_DEL_PARAMS		xReq;
	FTNM_RESP_EP_DATA_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_DEL;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nIndex		=	nIndex;
	xReq.nCount		=	nCount;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

FTM_RET	FTNM_CLIENT_EP_DATA_count
(
	FTNM_CLIENT_PTR		pClient,
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_DATA_COUNT_PARAMS	xReq;
	FTNM_RESP_EP_DATA_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_COUNT;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTNM_CLIENT_request(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pCount = xResp.ulCount;
	}

	return	xResp.nRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTNM_CLIENT_request
(
	FTNM_CLIENT_PTR 		pClient, 
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	FTM_RET		xRet;
	FTNM_CLIENT_TRANS	xTrans;
	FTM_INT		nRet;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	pReq->ulReqID = ++pClient->ulReqID;

	FTNM_CLIENT_TRANS_init(&xTrans, pReq, ulReqLen, pResp, ulRespLen);

	if( send(pClient->hSock, pReq, ulReqLen, 0) < 0)
	{
		TRACE("SEND[%08lx:%08x] - send failed\n", pClient->hSock, xTrans.pReq->ulReqID);
		return	FTM_RET_ERROR;	
	}

	xRet =FTM_LIST_append(&pClient->xTransList, (FTM_VOID_PTR)&xTrans);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	struct timespec xTimeout;

	clock_gettime(CLOCK_REALTIME, &xTimeout);

	xTimeout.tv_sec  += pClient->ulTimeout / 1000000 + (xTimeout.tv_nsec + (pClient->ulTimeout % 1000000) * 1000) / 1000000000;
	xTimeout.tv_nsec = (xTimeout.tv_nsec + (pClient->ulTimeout % 1000000) * 1000) % 1000000;

	nRet = sem_timedwait(&xTrans.xDone, &xTimeout);
	if (nRet != 0)
	{
		xRet = FTM_RET_COMM_TIMEOUT;	
		switch(nRet)
		{
   		case	EINTR:  
			{
				ERROR("The call was interrupted by a signal handler\n");
			}
			break;

        case	 EINVAL: 
			{
				ERROR("sem is not a valid semaphore.\n");
		        ERROR("The following additional error can occur for sem_trywait():\n");
				ERROR("The value of abs_timeout.tv_nsecs is less than 0, or greater than or equal to 1000 million.\n");
			}
			break;

		case	EAGAIN:
			{
				ERROR("The operation could not be performed without blocking (i.e., the semaphore currently has the value zero).\n");
				ERROR("The following additional errors can occur for sem_timedwait():\n");
			}
			break;

		case	ETIMEDOUT:
			{
				ERROR("The call timed out before the semaphore could be locked.\n");
			}
			break;

		default:
			ERROR("Unknown error[%08x]\n", nRet);
		}	
	}
	else
	{
		*pulRespLen = xTrans.ulRespLen;

		xRet = FTM_RET_OK;
	}

	FTM_LIST_remove(&pClient->xTransList, (FTM_VOID_PTR)&xTrans);
	FTNM_CLIENT_TRANS_final(&xTrans);

	return	xRet;	
}


FTM_BOOL FTNM_CLIENT_transSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTNM_CLIENT_TRANS_PTR	pTrans = (FTNM_CLIENT_TRANS_PTR)pElement;
	FTM_ULONG_PTR	pReqID = (FTM_ULONG_PTR)pIndicator;

	return	(pTrans->pReq->ulReqID == *pReqID);
}

FTM_RET FTNM_CLIENT_TRANS_init
(
	FTNM_CLIENT_TRANS_PTR			pTrans,
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pTrans != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pTrans->pReq 		= pReq;
	pTrans->ulReqLen 	= ulReqLen;
	pTrans->pResp 		= pResp;
	pTrans->ulRespLen 	= ulRespLen;
	sem_init(&pTrans->xDone, 0, 0);

	return	FTM_RET_OK;
}

FTM_RET FTNM_CLIENT_TRANS_final
(
	FTNM_CLIENT_TRANS_PTR		pTrans
)
{
	ASSERT(pTrans != NULL);

	pTrans->pReq 		= NULL;
	pTrans->ulReqLen 	= 0;
	pTrans->pResp 		= NULL;
	pTrans->ulRespLen 	= 0;
	sem_destroy(&pTrans->xDone);

	return	FTM_RET_OK;
}
