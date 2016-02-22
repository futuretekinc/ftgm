#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm.h"
#include "ftdm_client.h"

static FTM_RET FTDMC_request
(
	FTDMC_SESSION_PTR	pSession, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTM_INT				nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTM_INT				nRespLen
);

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_init(FTDMC_CFG_PTR pConfig)
{
	FTM_initEPTypeString();

	return	FTM_RET_OK;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_final(void)
{
	FTM_finalEPTypeString();

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_connect
(
	FTDMC_SESSION_PTR 	pSession,
	FTM_IP_ADDR			xIP,
	FTM_USHORT 			usPort 
)
{
	int 	hSock;
	struct sockaddr_in 	xServer;

	if ( pSession == NULL )
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	sem_init(&pSession->xLock, 0, 1);
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
	pSession->nTimeout = 5000;

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_disconnect
(
 	FTDMC_SESSION_PTR	pSession
)
{
	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	close(pSession->hSock);
	sem_destroy(&pSession->xLock);
	pSession->hSock = 0;
	
	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_isConnected
(
	FTDMC_SESSION_PTR		pSession,
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

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_NODE_INFO_append
(
 	FTDMC_SESSION_PTR		pSession,
	FTM_NODE_INFO_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_NODE_INFO_ADD_PARAMS	xReq;
	FTDM_RESP_NODE_INFO_ADD_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTDM_CMD_NODE_INFO_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE_INFO));

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_NODE_INFO_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
)
{
	FTM_RET								nRet;
	FTDM_REQ_NODE_INFO_DEL_PARAMS	xReq;
	FTDM_RESP_NODE_INFO_DEL_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_INFO_DEL;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_NODE_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET								nRet;
	FTDM_REQ_NODE_INFO_COUNT_PARAMS		xReq;
	FTDM_RESP_NODE_INFO_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_INFO_COUNT;
	xReq.nLen	=	sizeof(xReq);
	
	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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



FTM_RET FTDMC_NODE_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_NODE_INFO_GET_AT_PARAMS	xReq;
	FTDM_RESP_NODE_INFO_GET_AT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}


	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_INFO_GET_AT;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;
	
	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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
/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_NODE_INFO_get
(
 	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_NODE_INFO_GET_PARAMS		xReq;
	FTDM_RESP_NODE_INFO_GET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_INFO_GET;
	xReq.nLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_INFO_append
(
	FTDMC_SESSION_PTR	pSession,
	FTM_EP_INFO_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_EP_INFO_ADD_PARAMS		xReq;
	FTDM_RESP_EP_INFO_ADD_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xReq.xCmd	=	FTDM_CMD_EP_INFO_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP_INFO));

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_INFO_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID
)
{
	FTM_RET					nRet;
	FTDM_REQ_EP_INFO_DEL_PARAMS	xReq;
	FTDM_RESP_EP_INFO_DEL_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_EP_INFO_DEL;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_INFO_COUNT_PARAMS	xReq;
	FTDM_RESP_EP_INFO_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_INFO_COUNT;
	xReq.xClass	=	xClass;
	xReq.nLen	=	sizeof(xReq);

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_INFO_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR		pInfo
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_INFO_GET_PARAMS		xReq;
	FTDM_RESP_EP_INFO_GET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_INFO_GET;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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
FTM_RET	FTDMC_EP_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_EP_INFO_GET_AT_PARAMS	xReq;
	FTDM_RESP_EP_INFO_GET_AT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_INFO_GET_AT;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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
FTM_RET	FTDMC_EP_CLASS_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_CLASS_INFO_COUNT_PARAMS	xReq;
	FTDM_RESP_EP_CLASS_INFO_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_CLASS_INFO_COUNT;
	xReq.nLen	=	sizeof(xReq);

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
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

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_CLASS_INFO_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xEPClass,
	FTM_EP_CLASS_INFO_PTR	pInfo
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_CLASS_INFO_GET_PARAMS		xReq;
	FTDM_RESP_EP_CLASS_INFO_GET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd		=	FTDM_CMD_EP_CLASS_INFO_GET;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPClass	=	xEPClass;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_CLASS_INFO));
	}
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_CLASS_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_CLASS_INFO_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_EP_CLASS_INFO_GET_AT_PARAMS	xReq;
	FTDM_RESP_EP_CLASS_INFO_GET_AT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_CLASS_INFO_GET_AT;
	xReq.nLen	=	sizeof(xReq);
	xReq.nIndex	=	nIndex;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_CLASS_INFO));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pEPData
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_DATA_ADD_PARAMS	xReq;
	FTDM_RESP_EP_DATA_ADD_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_EP_DATA_ADD;
	xReq.nLen	=	sizeof(xReq);
	xReq.xEPID	=	xEPID;
	memcpy(&xReq.xData, pEPData, sizeof(FTM_EP_DATA));

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_EP_DATA_info
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime,
	FTM_ULONG_PTR			pulCount
)
{
	FTM_RET								nRet;
	FTDM_REQ_EP_DATA_INFO_PARAMS		xReq;
	FTDM_RESP_EP_DATA_INFO_PARAMS		xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_INFO;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	*pulBeginTime 	= xResp.ulBeginTime;
	*pulEndTime 	= xResp.ulEndTime;
	*pulCount		= xResp.ulCount;

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_get
(
	FTDMC_SESSION_PTR	pSession,
	FTM_EPID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	FTM_RET								nRet;
	FTDM_REQ_EP_DATA_GET_PARAMS			xReq;
	FTM_ULONG							nRespSize = 0;
	FTDM_RESP_EP_DATA_GET_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTDM_RESP_EP_DATA_GET_PARAMS) + sizeof(FTM_EP_DATA) * nMaxCount;
	pResp = (FTDM_RESP_EP_DATA_GET_PARAMS_PTR)malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_GET;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nStartIndex=	nStartIndex;
	xReq.nCount		=	nMaxCount;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize);
	if (nRet != FTM_RET_OK)
	{
		free(pResp);
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

	free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_getWithTime
(
	FTDMC_SESSION_PTR	pSession,
	FTM_EPID			xEPID,
	FTM_ULONG			nBeginTime,
	FTM_ULONG			nEndTime,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount
)
{
	FTM_RET										nRet;
	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS		xReq;
	FTM_ULONG									nRespSize = 0;
	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	nRespSize = sizeof(FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS) + sizeof(FTM_EP_DATA) * nMaxCount;
	pResp = (FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR)malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_GET_WITH_TIME;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime=	nBeginTime;
	xReq.nEndTime	=	nEndTime;
	xReq.nCount		=	nMaxCount;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize);
	if (nRet != FTM_RET_OK)
	{
		free(pResp);
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

	free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_DATA_DEL_PARAMS	xReq;
	FTDM_RESP_EP_DATA_DEL_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_DEL;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nIndex		=	nIndex;
	xReq.nCount		=	nCount;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_removeWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS	xReq;
	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_DEL_WITH_TIME;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime	=	nBeginTime;
	xReq.nEndTime	=	nEndTime;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_EP_DATA_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_DATA_COUNT_PARAMS	xReq;
	FTDM_RESP_EP_DATA_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_COUNT;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pCount = xResp.nCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_EP_DATA_countWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS	xReq;
	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_COUNT_WITH_TIME;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;
	xReq.nBeginTime	=	nBeginTime;
	xReq.nEndTime	=	nEndTime;

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp));
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.nRet == FTM_RET_OK)
	{
		*pCount = xResp.nCount;
	}

	return	xResp.nRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTDMC_request
(
	FTDMC_SESSION_PTR 	pSession, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTM_INT			nReqLen,
	FTDM_REQ_PARAMS_PTR	pResp,
	FTM_INT			nRespLen
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nTimeout;


	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	sem_wait(&pSession->xLock);

//	TRACE("send(%08lx, pReq, %d, 0)\n", pSession->hSock, nReqLen);

	if( send(pSession->hSock, pReq, nReqLen, 0) < 0)
	{
		xRet = FTM_RET_ERROR;	
	}
	else
	{
		nTimeout = pSession->nTimeout;
		while(--nTimeout > 0)
		{
			int	nLen = recv(pSession->hSock, pResp, nRespLen, MSG_DONTWAIT);
//			TRACE("recv(%08lx, pResp, %d, MSG_DONTWAIT)\n", pSession->hSock, nLen);
			if (nLen > 0)
			{
				break;
			}

			usleep(1000);
		}

		if (nTimeout == 0)
		{
			xRet = FTM_RET_COMM_TIMEOUT;	
		}

	}

	sem_post(&pSession->xLock);

	return	xRet;
}

