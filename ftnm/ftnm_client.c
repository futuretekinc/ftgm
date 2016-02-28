#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftnm_client.h"
#include "ftnm_params.h"
#include "ftm_msg_queue.h"

static FTM_RET FTNMC_request
(
	FTNMC_SESSION_PTR		pSession, 
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pRespBuff,
	FTM_ULONG				ulRespBuffLen,
	FTM_ULONG_PTR			pulRespLen
);

static FTM_RET FTNMC_TRANS_init
(
	FTNMC_TRANS_PTR			pTrans,
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

FTM_RET FTNMC_TRANS_final
(
	FTNMC_TRANS_PTR			pTrans
);

static FTM_VOID_PTR	FTNMC_process(FTM_VOID_PTR pData);
static FTM_BOOL 	FTNMC_transSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static pthread_t		xThread = 0;
static FTM_BOOL			bRun = FTM_FALSE;
static FTM_MSG_QUEUE	xMsgQ;
static FTM_LIST			xSessionList;
static FTNMC_NOTIFY_CALLBACK fEPNotifyCallback = NULL;
static FTNMC_NOTIFY_CALLBACK fNodeNotifyCallback = NULL;

FTM_RET	FTNMC_init
(
	FTNM_CFG_CLIENT_PTR	pConfig
)
{
	if (xThread != 0)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_MSGQ_init(&xMsgQ);
	FTM_LIST_init(&xSessionList);

	if (pthread_create(&xThread, NULL, FTNMC_process, NULL) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_final
(
	FTM_VOID
)
{
	FTM_VOID_PTR	pRet;
	
	if (xThread == 0)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	if (bRun)
	{
		bRun = FTM_FALSE;
		pthread_join(xThread, &pRet);
	}

	FTM_LIST_final(&xSessionList);
	FTM_MSGQ_final(&xMsgQ);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNMC_process(FTM_VOID_PTR pData)
{
	bRun = FTM_TRUE;
	
	FTM_BYTE				pBuff[2048];
	FTNM_RESP_PARAMS_PTR 	pResp = (FTNM_RESP_PARAMS_PTR)pBuff;

	while(bRun)
	{
		FTNMC_SESSION_PTR	pSession;

		FTM_LIST_iteratorStart(&xSessionList);
		while(FTM_LIST_iteratorNext(&xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
		{
			int	nLen = recv(pSession->hSock, pResp, 2048, MSG_DONTWAIT);
			if (nLen > 0)
			{
				FTNMC_TRANS_PTR	pTrans;

				if (FTM_LIST_get(&pSession->xTransList, &pResp->ulReqID, (FTM_VOID_PTR _PTR_)&pTrans) == FTM_RET_OK)
				{
					memcpy(pTrans->pResp, pResp, nLen);
					pTrans->ulRespLen = nLen;

					sem_post(&pTrans->xDone);
				}
				else if (pResp->ulReqID == 0)
				{
					FTNMC_MSG_PTR pMsg = (FTNMC_MSG_PTR)pResp;
					switch(pMsg->xType)
					{
					case	FTNMC_MSG_TYPE_EP_NOTIFY:
						{
							if (fEPNotifyCallback != NULL)
							{
								fEPNotifyCallback(NULL);	
							}
						}
						break;

					case	FTNMC_MSG_TYPE_NODE_NOTIFY:
						{
							if (fNodeNotifyCallback != NULL)
							{
								fNodeNotifyCallback(NULL);	
							}
						}
						break;
					}
				}
				else
				{
					ERROR("Invalid ReqID[%ul]\n", pResp->ulReqID);	
				}
			}
		}

		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNMC_createSession
(
	FTNMC_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(ppSession != NULL);

	FTNMC_SESSION_PTR	pSession;

	pSession = (FTNMC_SESSION_PTR)FTM_MEM_malloc(sizeof(FTNMC_SESSION));
	if (pSession == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}
	
	memset(pSession, 0, sizeof(FTNMC_SESSION));
	sem_init(&pSession->xReqLock, 0, 0);
	FTM_LIST_init(&pSession->xTransList);
	FTM_LIST_setSeeker(&pSession->xTransList, FTNMC_transSeeker);

	*ppSession = pSession;

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_destroySession
(
	FTNMC_SESSION_PTR pSession
)
{
	ASSERT(pSession != NULL);
	
	FTM_LIST_final(&pSession->xTransList);
	sem_destroy(&pSession->xReqLock);

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_connect
(
	FTNMC_SESSION_PTR		pSession,
	FTM_IP_ADDR				xIP,
	FTM_USHORT				usPort
)
{
	FTM_INT				hSock;
	struct sockaddr_in 	xServer;

	if ( pSession == NULL )
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == -1)
	{
		ERROR("Could not create socket.\n");	
		return	FTM_RET_ERROR;
	}

	xServer.sin_addr.s_addr	= xIP;
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(usPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		return	FTM_RET_ERROR;	
	}
	
	pSession->hSock = hSock;
	pSession->ulTimeout = 5000;

	return	FTM_RET_OK;
}

FTM_RET FTNMC_disconnect
(
	FTNMC_SESSION_PTR		pSession
)
{
	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	close(pSession->hSock);
	pSession->hSock = 0;
	
	return	FTM_RET_OK;
}

FTM_RET FTNMC_isConnected
(
	FTNMC_SESSION_PTR		pSession,
	FTM_BOOL_PTR			pbConnected
)
{
	if ((pSession != NULL) && (pSession->hSock != 0))
	{
		*pbConnected = FTM_TRUE;	
	}
	else
	{
		*pbConnected = FTM_FALSE;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNMC_NODE_create
(
	FTNMC_SESSION_PTR		pSession,
	FTM_NODE_INFO_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_CREATE_PARAMS		xReq;
	FTNM_RESP_NODE_CREATE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTNM_CMD_NODE_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE_INFO));

	nRet = FTNMC_request(
				pSession, 
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

FTM_RET FTNMC_NODE_destroy
(
	FTNMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_DESTROY_PARAMS	xReq;
	FTNM_RESP_NODE_DESTROY_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
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
	
	nRet = FTNMC_request(
				pSession, 
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

FTM_RET FTNMC_NODE_count
(
	FTNMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
)
{
	FTM_RET						nRet;
	FTNM_REQ_NODE_COUNT_PARAMS	xReq;
	FTNM_RESP_NODE_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
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
	
	nRet = FTNMC_request(
				pSession, 
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

FTM_RET FTNMC_NODE_getAt
(
	FTNMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_NODE_INFO_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_GET_AT_PARAMS		xReq;
	FTNM_RESP_NODE_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}


	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTNM_CMD_NODE_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	
	nRet = FTNMC_request(
				pSession, 
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
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE_INFO));
	}

	return	xResp.nRet;
}

FTM_RET FTNMC_NODE_get
(
	FTNMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTNM_REQ_NODE_GET_PARAMS		xReq;
	FTNM_RESP_NODE_GET_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
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
	
	nRet = FTNMC_request(
				pSession, 
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
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE_INFO));
	}
	
	return	xResp.nRet;
}

FTM_RET FTNMC_EP_create
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_INFO_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_CREATE_PARAMS	xReq;
	FTNM_RESP_EP_CREATE_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xReq.xCmd	=	FTNM_CMD_EP_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP_INFO));

	nRet = FTNMC_request(
				pSession, 
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

FTM_RET FTNMC_EP_destroy
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID
)
{
	FTM_RET					nRet;
	FTNM_REQ_EP_DESTROY_PARAMS	xReq;
	FTNM_RESP_EP_DESTROY_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTNM_CMD_EP_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTNMC_request(
				pSession, 
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

FTM_RET FTNMC_EP_count
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_COUNT_PARAMS	xReq;
	FTNM_RESP_EP_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTNM_CMD_EP_COUNT;
	xReq.xClass	=	xClass;
	xReq.ulLen	=	sizeof(xReq);

	nRet = FTNMC_request(
				pSession, 
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

FTM_RET	FTNMC_EP_getList
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_EPID_PTR			pEPIDList,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET	nRet;
	FTNM_REQ_EP_GET_LIST_PARAMS		xReq;
	FTM_ULONG						nRespSize = 0;
	FTNM_RESP_EP_GET_LIST_PARAMS_PTR	pResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTNM_RESP_EP_GET_LIST_PARAMS) + sizeof(FTM_EPID) * ulMaxCount;
	pResp = (FTNM_RESP_EP_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTNM_CMD_EP_GET_LIST;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xClass		=	xClass;
	xReq.ulMaxCount	=	ulMaxCount;

	nRet = FTNMC_request(
				pSession, 
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
		memcpy(pEPIDList, pResp->pEPIDList, sizeof(FTM_EPID) * pResp->ulCount);
		*pnCount = pResp->ulCount;
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

FTM_RET FTNMC_EP_get
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_GET_PARAMS		xReq;
	FTNM_RESP_EP_GET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
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

	nRet = FTNMC_request(
				pSession, 
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
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_INFO));
	}
	return	xResp.nRet;
}

FTM_RET FTNMC_EP_getAt
(
	FTNMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_EP_INFO_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_GET_AT_PARAMS	xReq;
	FTNM_RESP_EP_GET_AT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
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

	nRet = FTNMC_request(
				pSession, 
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
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_INFO));
	}
	
	return	xResp.nRet;
}


/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTNMC_EP_DATA_add
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pEPData
)
{
	FTM_RET							nRet;
	FTNM_REQ_EP_DATA_ADD_PARAMS		xReq;
	FTNM_RESP_EP_DATA_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTNM_CMD_EP_DATA_ADD;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	memcpy(&xReq.xData, pEPData, sizeof(FTM_EP_DATA));

	nRet = FTNMC_request(
				pSession, 
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
FTM_RET	FTNMC_EP_DATA_info
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET							nRet;
	FTNM_REQ_EP_DATA_INFO_PARAMS	xReq;
	FTNM_RESP_EP_DATA_INFO_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_INFO;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTNMC_request(
				pSession, 
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
FTM_RET FTNMC_EP_DATA_getLast
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pData
)
{
	FTM_RET								nRet;
	FTNM_REQ_EP_DATA_GET_LAST_PARAMS	xReq;
	FTNM_RESP_EP_DATA_GET_LAST_PARAMS	xResp;
	FTM_ULONG						ulRespLen;
	
	if ((pSession == NULL) || (pSession->hSock == 0))
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

	nRet = FTNMC_request(
				pSession, 
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
FTM_RET	FTNMC_EP_DATA_getList
(
	FTNMC_SESSION_PTR	pSession,
	FTM_EPID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	FTM_RET								nRet;
	FTNM_REQ_EP_DATA_GET_LIST_PARAMS			xReq;
	FTM_ULONG							nRespSize = 0;
	FTNM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp = NULL;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTNM_RESP_EP_DATA_GET_LIST_PARAMS) + sizeof(FTM_EP_DATA) * nMaxCount;
	pResp = (FTNM_RESP_EP_DATA_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_GET_LIST;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nStartIndex=	nStartIndex;
	xReq.nCount		=	nMaxCount;

	nRet = FTNMC_request(
				pSession, 
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
		for( i = 0 ; i < pResp->nCount && i < nMaxCount ; i++)
		{
			memcpy(&pData[i], &pResp->pData[i], sizeof(FTM_EP_DATA));
		}

		*pnCount = pResp->nCount;
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_del
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
)
{
	FTM_RET							nRet;
	FTNM_REQ_EP_DATA_DEL_PARAMS		xReq;
	FTNM_RESP_EP_DATA_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_DEL;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nIndex		=	nIndex;
	xReq.nCount		=	nCount;

	nRet = FTNMC_request(
				pSession, 
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

FTM_RET	FTNMC_EP_DATA_count
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_DATA_COUNT_PARAMS	xReq;
	FTNM_RESP_EP_DATA_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_COUNT;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTNMC_request(
				pSession, 
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
FTM_RET FTNMC_request
(
	FTNMC_SESSION_PTR 		pSession, 
	FTNM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTNM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	FTM_RET		xRet;
	FTNMC_TRANS	xTrans;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	pReq->ulReqID = ++pSession->ulReqID;

	TRACE("SEND[%08lx:%08x] : Len = %d\n", pSession->hSock, pReq->ulReqID, ulReqLen);
	
	FTNMC_TRANS_init(&xTrans, pReq, ulReqLen, pResp, ulRespLen);

	if( send(pSession->hSock, pReq, ulReqLen, 0) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	FTM_LIST_append(&pSession->xTransList, (FTM_VOID_PTR)&xTrans);

	struct timespec xTimeout;
	xTimeout.tv_sec  = pSession->ulTimeout / 1000000;
	xTimeout.tv_nsec = (pSession->ulTimeout % 1000000) * 1000;

	if (sem_timedwait(&xTrans.xDone, &xTimeout) < 0)
	{
		TRACE("RECV[%08lx:%08x] : Timeout\n", pSession->hSock, xTrans.pResp->ulReqID);
		xRet = FTM_RET_COMM_TIMEOUT;	
	}
	else
	{
		TRACE("RECV[%08lx:%08x] : Len = %d\n", pSession->hSock, xTrans.pResp->ulReqID, xTrans.ulRespLen);

		*pulRespLen = xTrans.ulRespLen;

		xRet = FTM_RET_OK;
	}

	FTM_LIST_remove(&pSession->xTransList, (FTM_VOID_PTR)&xTrans);
	FTNMC_TRANS_final(&xTrans);

	return	xRet;	
}


FTM_BOOL FTNMC_transSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTNMC_TRANS_PTR	pTrans = (FTNMC_TRANS_PTR)pElement;
	FTM_ULONG_PTR	pReqID = (FTM_ULONG_PTR)pIndicator;

	return	(pTrans->pReq->ulReqID == *pReqID);
}

FTM_RET FTNMC_TRANS_init
(
	FTNMC_TRANS_PTR			pTrans,
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

FTM_RET FTNMC_TRANS_final
(
	FTNMC_TRANS_PTR		pTrans
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
