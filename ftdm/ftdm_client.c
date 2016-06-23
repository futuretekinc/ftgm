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
	FTDM_RESP_PARAMS_PTR	pResp,
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

	FTM_LOCK_init(&pSession->xLock);
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
	FTM_LOCK_final(&pSession->xLock);
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
FTM_RET FTDMC_NODE_append
(
 	FTDMC_SESSION_PTR		pSession,
	FTM_NODE_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_NODE_ADD_PARAMS	xReq;
	FTDM_RESP_NODE_ADD_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTDM_CMD_NODE_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE));

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
FTM_RET FTDMC_NODE_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
)
{
	FTM_RET								nRet;
	FTDM_REQ_NODE_DEL_PARAMS	xReq;
	FTDM_RESP_NODE_DEL_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_DEL;
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
FTM_RET FTDMC_NODE_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET								nRet;
	FTDM_REQ_NODE_COUNT_PARAMS		xReq;
	FTDM_RESP_NODE_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_COUNT;
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



FTM_RET FTDMC_NODE_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_NODE_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_NODE_GET_AT_PARAMS	xReq;
	FTDM_RESP_NODE_GET_AT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}


	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_GET_AT;
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
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}
	
	return	xResp.nRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_NODE_get
(
 	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_NODE_GET_PARAMS		xReq;
	FTDM_RESP_NODE_GET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_GET;
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
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_NODE_set
(
 	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR	pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_NODE_SET_PARAMS		xReq;
	FTDM_RESP_NODE_SET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_SET;
	xReq.nLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE));
	
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
FTM_RET FTDMC_NODE_getDIDList
(
 	FTDMC_SESSION_PTR	pSession,
	FTM_DID_PTR			pDIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET								nRet;
	FTM_ULONG							ulRespLen;
	FTDM_REQ_NODE_GET_DID_LIST_PARAMS	xReq;
	FTDM_RESP_NODE_GET_DID_LIST_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_NODE_GET_DID_LIST;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;
	xReq.ulCount=	ulMaxCount;

	ulRespLen = sizeof(FTDM_RESP_NODE_GET_DID_LIST_PARAMS) + sizeof(FTM_DID) * ulMaxCount;
	pResp = (FTDM_RESP_NODE_GET_DID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		ERROR("Not enough memory[size = %d]!\n", ulRespLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	nRet;	
	}

	nRet = pResp->nRet;

	if (nRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pDIDs, pResp->pDIDs, sizeof(FTM_DID) * pResp->ulCount);
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_append
(
	FTDMC_SESSION_PTR	pSession,
	FTM_EP_PTR		pInfo
)
{
	FTM_RET							nRet;
	FTDM_REQ_EP_ADD_PARAMS		xReq;
	FTDM_RESP_EP_ADD_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xReq.xCmd	=	FTDM_CMD_EP_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));

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
FTM_RET	FTDMC_EP_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID
)
{
	FTM_RET					nRet;
	FTDM_REQ_EP_DEL_PARAMS	xReq;
	FTDM_RESP_EP_DEL_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	xReq.xCmd	=	FTDM_CMD_EP_DEL;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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
FTM_RET	FTDMC_EP_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_COUNT_PARAMS	xReq;
	FTDM_RESP_EP_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_COUNT;
	xReq.xType	=	xType;
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
FTM_RET	FTDMC_EP_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_PTR		pInfo
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_GET_PARAMS		xReq;
	FTDM_RESP_EP_GET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_GET;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_EP_GET_AT_PARAMS	xReq;
	FTDM_RESP_EP_GET_AT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_GET_AT;
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
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_set
(
	FTDMC_SESSION_PTR	pSession,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_FIELD		xFields,
	FTM_EP_PTR			pInfo
)
{
	FTM_RET					nRet;
	FTDM_REQ_EP_SET_PARAMS	xReq;
	FTDM_RESP_EP_SET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_SET;
	xReq.nLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pEPID, pEPID, FTM_ID_LEN);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));

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
FTM_RET FTDMC_EP_getEPIDList
(
 	FTDMC_SESSION_PTR	pSession,
	FTM_EPID_PTR		pEPIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET	nRet;
	FTM_ULONG	ulRespLen;
	FTDM_REQ_EP_GET_EPID_LIST_PARAMS	xReq;
	FTDM_RESP_EP_GET_EPID_LIST_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_EP_GET_EPID_LIST;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;
	xReq.ulCount=	ulMaxCount;

	ulRespLen = sizeof(FTDM_RESP_EP_GET_EPID_LIST_PARAMS) + sizeof(FTM_EPID) * ulMaxCount;
	pResp = (FTDM_RESP_EP_GET_EPID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		ERROR("Not enough memory[size = %d]!\n", ulRespLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	nRet;	
	}

	nRet = pResp->nRet;

	if (nRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pEPIDs, pResp->pEPIDs, sizeof(FTM_EPID) * pResp->ulCount);
	}

	FTM_MEM_free(pResp);

	return	nRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_CLASS_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_CLASS_COUNT_PARAMS	xReq;
	FTDM_RESP_EP_CLASS_COUNT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pnCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_CLASS_COUNT;
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
FTM_RET	FTDMC_EP_CLASS_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_TYPE			xEPClass,
	FTM_EP_CLASS_PTR	pInfo
)
{
	FTM_RET						nRet;
	FTDM_REQ_EP_CLASS_GET_PARAMS		xReq;
	FTDM_RESP_EP_CLASS_GET_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd		=	FTDM_CMD_EP_CLASS_GET;
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
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_CLASS));
	}
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_CLASS_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_CLASS_PTR		pInfo
)
{
	FTM_RET								nRet;
	FTDM_REQ_EP_CLASS_GET_AT_PARAMS	xReq;
	FTDM_RESP_EP_CLASS_GET_AT_PARAMS	xResp;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xReq.xCmd	=	FTDM_CMD_EP_CLASS_GET_AT;
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
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP_CLASS));
	}
	
	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
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
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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
	FTM_CHAR_PTR			pEPID,
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
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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
	FTM_CHAR_PTR		pEPID,
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
	pResp = (FTDM_RESP_EP_DATA_GET_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_GET;
	xReq.nLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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
		TRACE("FTDMC_request error[%08x]\n", nRet);
		FTM_MEM_free(pResp);
		return	FTM_RET_ERROR;	
	}

	nRet = pResp->nRet;

	if (pResp->nRet == FTM_RET_OK)
	{
		FTM_INT	i;

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
FTM_RET	FTDMC_EP_DATA_getWithTime
(
	FTDMC_SESSION_PTR	pSession,
	FTM_CHAR_PTR		pEPID,
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
	pResp = (FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTDM_CMD_EP_DATA_GET_WITH_TIME;
	xReq.nLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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
		FTM_MEM_free(pResp);
		return	FTM_RET_ERROR;	
	}

	nRet = pResp->nRet;

	if (pResp->nRet == FTM_RET_OK)
	{
		FTM_INT	i;

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
FTM_RET	FTDMC_EP_DATA_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount,
	FTM_ULONG_PTR			pulCount
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
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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

	if (xResp.nRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
	}

	return	xResp.nRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTDMC_EP_DATA_removeWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_ULONG_PTR			pulCount
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
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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
		*pulCount = xResp.ulCount;	
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_EP_DATA_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
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
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

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
	FTM_CHAR_PTR			pEPID,
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
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
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

FTM_RET	FTDMC_TRIGGER_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_TRIGGER_PTR     		pTrigger
)
{
	ASSERT(pSession != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						nRet;
	FTDM_REQ_TRIGGER_ADD_PARAMS	xReq;
	FTDM_RESP_TRIGGER_ADD_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xTrigger, pTrigger, sizeof(FTM_TRIGGER));

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

FTM_RET	FTDMC_TRIGGER_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pTriggerID
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_TRIGGER_DEL_PARAMS	xReq;
	FTDM_RESP_TRIGGER_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_DEL;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pTriggerID, pTriggerID, FTM_ID_LEN);

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

FTM_RET	FTDMC_TRIGGER_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pSession != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_TRIGGER_COUNT_PARAMS		xReq;
	FTDM_RESP_TRIGGER_COUNT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_COUNT;
	xReq.nLen	=	sizeof(xReq);

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
		*pulCount = xResp.nCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_TRIGGER_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pTriggerID,
	FTM_TRIGGER_PTR			pTrigger
)
{
	ASSERT(pSession != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						nRet;
 	FTDM_REQ_TRIGGER_GET_PARAMS	xReq;
	FTDM_RESP_TRIGGER_GET_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_GET;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pTriggerID, pTriggerID, FTM_ID_LEN);

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
		memcpy(pTrigger, &xResp.xTrigger, sizeof(FTM_TRIGGER));
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_TRIGGER_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_TRIGGER_PTR			pTrigger

)
{
	ASSERT(pSession != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_TRIGGER_GET_AT_PARAMS	xReq;
	FTDM_RESP_TRIGGER_GET_AT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_GET_AT;
	xReq.nLen 	=	sizeof(xReq);
	xReq.nIndex	=	ulIndex;

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
		memcpy(pTrigger, &xResp.xTrigger, sizeof(FTM_TRIGGER));
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_TRIGGER_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pTriggerID,
	FTM_TRIGGER_FIELD		xFields,
	FTM_TRIGGER_PTR			pTrigger
)
{
	ASSERT(pSession != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_TRIGGER_SET_PARAMS		xReq;
	FTDM_RESP_TRIGGER_SET_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_GET;
	xReq.nLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pTriggerID, pTriggerID, FTM_ID_LEN);
	memcpy(&xReq.xTrigger, pTrigger, sizeof(FTM_TRIGGER));

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
FTM_RET FTDMC_TRIGGER_getIDList
(
 	FTDMC_SESSION_PTR	pSession,
	FTM_ID_PTR			pIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET	nRet;
	FTM_ULONG	ulRespLen;
	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS	xReq;
	FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_TRIGGER_GET_ID_LIST;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;
	xReq.ulCount=	ulMaxCount;

	ulRespLen = sizeof(FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		ERROR("Not enough memory[size = %d]!\n", ulRespLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	nRet;	
	}

	nRet = pResp->nRet;

	if (nRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pIDs, pResp->pIDs, sizeof(FTM_ID) * pResp->ulCount);
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

/////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////
FTM_RET	FTDMC_ACTION_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ACTION_PTR     		pAct
)
{
	ASSERT(pSession != NULL);
	ASSERT(pAct != NULL);

	FTM_RET						nRet;
	FTDM_REQ_ACTION_ADD_PARAMS	xReq;
	FTDM_RESP_ACTION_ADD_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xAction, pAct, sizeof(FTM_ACTION));

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

FTM_RET	FTDMC_ACTION_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pActionID
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_ACTION_DEL_PARAMS	xReq;
	FTDM_RESP_ACTION_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_DEL;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);

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

FTM_RET	FTDMC_ACTION_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pSession != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_ACTION_COUNT_PARAMS		xReq;
	FTDM_RESP_ACTION_COUNT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_COUNT;
	xReq.nLen	=	sizeof(xReq);

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
		*pulCount = xResp.nCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_ACTION_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pActionID,
	FTM_ACTION_PTR			pAct
)
{
	ASSERT(pSession != NULL);
	ASSERT(pAct != NULL);

	FTM_RET						nRet;
 	FTDM_REQ_ACTION_GET_PARAMS	xReq;
	FTDM_RESP_ACTION_GET_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_GET;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);

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
		memcpy(pAct, &xResp.xAction, sizeof(FTM_ACTION));
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_ACTION_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_ACTION_PTR			pAct

)
{
	ASSERT(pSession != NULL);
	ASSERT(pAct != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_ACTION_GET_AT_PARAMS	xReq;
	FTDM_RESP_ACTION_GET_AT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_GET_AT;
	xReq.nLen 	=	sizeof(xReq);
	xReq.nIndex	=	ulIndex;

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
		memcpy(pAct, &xResp.xAction, sizeof(FTM_ACTION));
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_ACTION_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pActionID,
	FTM_ACTION_FIELD		xFields,
	FTM_ACTION_PTR			pInfo
)
{
	ASSERT(pSession != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						nRet;
 	FTDM_REQ_ACTION_SET_PARAMS	xReq;
	FTDM_RESP_ACTION_SET_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_GET;
	xReq.nLen	=	sizeof(xReq);
	xReq.xFields= 	xFields;
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);
	memcpy(&xReq.xAction, pInfo, sizeof(FTM_ACTION));

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
FTM_RET FTDMC_ACTION_getIDList
(
 	FTDMC_SESSION_PTR	pSession,
	FTM_ID_PTR			pIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET	nRet;
	FTM_ULONG	ulRespLen;
	FTDM_REQ_ACTION_GET_ID_LIST_PARAMS	xReq;
	FTDM_RESP_ACTION_GET_ID_LIST_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_ACTION_GET_ID_LIST;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;
	xReq.ulCount=	ulMaxCount;

	ulRespLen = sizeof(FTDM_RESP_ACTION_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTDM_RESP_ACTION_GET_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		ERROR("Not enough memory[size = %d]!\n", ulRespLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	nRet;	
	}

	nRet = pResp->nRet;

	if (nRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pIDs, pResp->pIDs, sizeof(FTM_ID) * pResp->ulCount);
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

/////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////
FTM_RET	FTDMC_RULE_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_RULE_PTR     		pAct
)
{
	ASSERT(pSession != NULL);
	ASSERT(pAct != NULL);

	FTM_RET						nRet;
	FTDM_REQ_RULE_ADD_PARAMS	xReq;
	FTDM_RESP_RULE_ADD_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xRule, pAct, sizeof(FTM_RULE));

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

FTM_RET	FTDMC_RULE_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pRuleID
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_RULE_DEL_PARAMS	xReq;
	FTDM_RESP_RULE_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_DEL;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);

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

FTM_RET	FTDMC_RULE_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pSession != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_RULE_COUNT_PARAMS		xReq;
	FTDM_RESP_RULE_COUNT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_COUNT;
	xReq.nLen	=	sizeof(xReq);

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
		*pulCount = xResp.nCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_RULE_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pRuleID,
	FTM_RULE_PTR			pAct
)
{
	ASSERT(pSession != NULL);
	ASSERT(pAct != NULL);

	FTM_RET						nRet;
 	FTDM_REQ_RULE_GET_PARAMS	xReq;
	FTDM_RESP_RULE_GET_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_GET;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);

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
		memcpy(pAct, &xResp.xRule, sizeof(FTM_RULE));
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_RULE_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_RULE_PTR			pAct

)
{
	ASSERT(pSession != NULL);
	ASSERT(pAct != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_RULE_GET_AT_PARAMS	xReq;
	FTDM_RESP_RULE_GET_AT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_GET_AT;
	xReq.nLen 	=	sizeof(xReq);
	xReq.nIndex	=	ulIndex;

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
		memcpy(pAct, &xResp.xRule, sizeof(FTM_RULE));
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_RULE_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pRuleID,
	FTM_RULE_FIELD			xFields,
	FTM_RULE_PTR			pInfo
)
{
	ASSERT(pSession != NULL);
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						nRet;
 	FTDM_REQ_RULE_SET_PARAMS	xReq;
	FTDM_RESP_RULE_SET_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_GET;
	xReq.nLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);
	memcpy(&xReq.xRule, pInfo, sizeof(FTM_RULE));

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

/////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////
FTM_RET	FTDMC_LOG_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_LOG_PTR     		pLog
)
{
	ASSERT(pSession != NULL);
	ASSERT(pLog != NULL);

	FTM_RET						nRet;
	FTDM_REQ_LOG_ADD_PARAMS	xReq;
	FTDM_RESP_LOG_ADD_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_LOG_ADD;
	xReq.nLen	=	sizeof(xReq);
	memcpy(&xReq.xLog, pLog, sizeof(FTM_LOG));

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

FTM_RET	FTDMC_LOG_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulCount,
	FTM_ULONG_PTR			pulDeletedCount
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_LOG_DEL_PARAMS		xReq;
	FTDM_RESP_LOG_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_LOG_DEL;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;
	xReq.ulCount=	ulCount;

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

	if ((xResp.nRet == FTM_RET_OK) && (pulDeletedCount != NULL))
	{
		*pulDeletedCount = xResp.ulCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_LOG_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pSession != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET							nRet;
 	FTDM_REQ_LOG_COUNT_PARAMS		xReq;
	FTDM_RESP_LOG_COUNT_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_COUNT;
	xReq.nLen	=	sizeof(xReq);

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
		*pulCount = xResp.nCount;
	}

	return	xResp.nRet;
}

FTM_RET	FTDMC_LOG_get
(
	FTDMC_SESSION_PTR	pSession,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulCount,
	FTM_LOG_PTR			pLogs,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pSession != NULL);

	FTM_RET							nRet;
	FTDM_REQ_LOG_GET_PARAMS			xReq;
	FTDM_RESP_LOG_GET_PARAMS_PTR	pResp;
	FTM_ULONG						ulRespLen = 0;
	FTM_ULONG						ulReadCount = 0;

	while(ulCount > 0)
	{
		FTM_ULONG	ulRequestCount = 0;
		if (ulCount > 20)
		{
			ulRequestCount = 20;	
		}
		else
		{
			ulRequestCount = ulCount;	
		}
	
		xReq.xCmd	=	FTDM_CMD_LOG_GET;
		xReq.nLen	=	sizeof(xReq);
		xReq.ulIndex=	ulIndex;
		xReq.ulCount=	ulRequestCount;

		TRACE("%s[%d] : %lu, %lu\n", __func__, __LINE__, ulIndex, ulRequestCount);
		ulRespLen =  sizeof(FTDM_RESP_LOG_GET_PARAMS) + sizeof(FTM_LOG) * ulRequestCount;
		pResp = (FTDM_RESP_LOG_GET_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
		if (pResp == NULL)
		{
			nRet = FTM_RET_NOT_ENOUGH_MEMORY;	
			break;
		}

		nRet = FTDMC_request(pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen);
		if (nRet != FTM_RET_OK)
		{
			break;
		}

		nRet = pResp->nRet;
		if (nRet != FTM_RET_OK) 
		{
			break;
		}

		if (pResp->ulCount > ulCount)
		{
			nRet = FTM_RET_ERROR;
			ERROR("The requested quantity[%lu] has been exceeded[%lu]\n", ulRequestCount, pResp->ulCount);	
			break;
		}

		memcpy(&pLogs[ulReadCount], pResp->pLogs, sizeof(FTM_LOG) * pResp->ulCount);
		ulIndex += pResp->ulCount;
		ulReadCount += pResp->ulCount;
		ulCount -= pResp->ulCount;
		if (ulRequestCount > pResp->ulCount)
		{
			break;
		}
	}


	if (nRet == FTM_RET_OK)
	{
		*pulCount = ulReadCount;	
	}

	if (pResp != NULL)
	{
		FTM_MEM_free(pResp);	
	}

	return	nRet;
}

FTM_RET	FTDMC_LOG_getAt
(
	FTDMC_SESSION_PTR	pSession,
	FTM_ULONG			ulIndex,
	FTM_LOG_PTR			pLog
)
{
	ASSERT(pSession != NULL);

	FTM_RET							nRet;
	FTDM_REQ_LOG_GET_AT_PARAMS		xReq;
	FTDM_RESP_LOG_GET_AT_PARAMS		xResp;
	
	xReq.xCmd	=	FTDM_CMD_LOG_GET_AT;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;

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
		memcpy(pLog, &xResp.xLog, sizeof(FTM_LOG));
	}

	return	xResp.nRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTDMC_RULE_getIDList
(
 	FTDMC_SESSION_PTR	pSession,
	FTM_ID_PTR			pIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET	nRet;
	FTM_ULONG	ulRespLen;
	FTDM_REQ_RULE_GET_ID_LIST_PARAMS	xReq;
	FTDM_RESP_RULE_GET_ID_LIST_PARAMS_PTR	pResp = NULL;

	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTDM_CMD_RULE_GET_ID_LIST;
	xReq.nLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;
	xReq.ulCount=	ulMaxCount;

	ulRespLen = sizeof(FTDM_RESP_RULE_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTDM_RESP_RULE_GET_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		ERROR("Not enough memory[size = %d]!\n", ulRespLen);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nRet = FTDMC_request(
				pSession, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen);
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	nRet;	
	}

	nRet = pResp->nRet;

	if (nRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pIDs, pResp->pIDs, sizeof(FTM_ID) * pResp->ulCount);
	}

	FTM_MEM_free(pResp);

	return	nRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTDMC_request
(
	FTDMC_SESSION_PTR 	pSession, 
	FTDM_REQ_PARAMS_PTR	pReq,
	FTM_INT			nReqLen,
	FTDM_RESP_PARAMS_PTR	pResp,
	FTM_INT			nRespLen
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nTimeout;


	if ((pSession == NULL) || (pSession->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	FTM_LOCK_set(&pSession->xLock);

	//TRACE("send(%08lx, pReq, %d, 0)\n", pSession->hSock, nReqLen);

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

	FTM_LOCK_reset(&pSession->xLock);

	return	xRet;
}

