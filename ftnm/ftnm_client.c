#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftnm_client.h"
#include "ftnm_params.h"

static FTM_RET FTNMC_request
(
	FTNMC_SESSION_PTR	pSession, 
	FTNM_REQ_PARAMS_PTR	pReq,
	FTM_ULONG			ulReqLen,
	FTNM_REQ_PARAMS_PTR	pResp,
	FTM_ULONG			ulRespLen
);

FTM_RET	FTNMC_init
(
	FTNM_CFG_CLIENT_PTR	pConfig
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNMC_final
(
	FTM_VOID
)
{
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
		*pbConnected = FTM_BOOL_TRUE;	
	}
	else
	{
		*pbConnected = FTM_BOOL_FALSE;	
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
				sizeof(xResp));
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
				sizeof(xResp));
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
				sizeof(xResp));
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

FTM_RET FTNMC_EP_create
(
	FTNMC_SESSION_PTR		pSession,
	FTM_EP_INFO_PTR			pInfo
)
{
	FTM_RET						nRet;
	FTNM_REQ_EP_CREATE_PARAMS	xReq;
	FTNM_RESP_EP_CREATE_PARAMS	xResp;

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
				sizeof(xResp));
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
				sizeof(xResp));
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
				nRespSize);
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
	FTNM_RESP_EP_GET_PARAMS	xResp;

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
				sizeof(xResp));
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
				nRespSize);
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
				sizeof(xResp));
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

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd		=	FTNM_CMD_EP_DATA_COUNT;
	xReq.nLen		=	sizeof(xReq);
	xReq.xEPID		=	xEPID;

	nRet = FTNMC_request(
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
FTM_RET FTNMC_request
(
	FTNMC_SESSION_PTR 	pSession, 
	FTNM_REQ_PARAMS_PTR	pReq,
	FTM_ULONG			ulReqLen,
	FTNM_REQ_PARAMS_PTR	pResp,
	FTM_ULONG			ulRespLen
)
{
	FTM_ULONG	ulTimeout;


	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

//	TRACE("send(%08lx, pReq, %d, 0)\n", pSession->hSock, ulReqLen);

	if( send(pSession->hSock, pReq, ulReqLen, 0) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	ulTimeout = pSession->ulTimeout;
	while(--ulTimeout > 0)
	{
		int	nLen = recv(pSession->hSock, pResp, ulRespLen, MSG_DONTWAIT);
		if (nLen > 0)
		{
//			TRACE("recv(%08lx, pResp, %d, MSG_DONTWAIT)\n", pSession->hSock, nLen);
			return	FTM_RET_OK;	
		}

		usleep(1000);
	}

	return	FTM_RET_COMM_TIMEOUT;	
}

