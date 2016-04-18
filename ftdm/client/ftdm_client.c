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
	FTM_TRIGGER_ID     		xTriggerID
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_TRIGGER_DEL_PARAMS	xReq;
	FTDM_RESP_TRIGGER_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_TRIGGER_DEL;
	xReq.nLen	=	sizeof(xReq);
	xReq.xID	=	xTriggerID;

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
	FTM_TRIGGER_ID     		xTriggerID,
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
	xReq.xID	=	xTriggerID;

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
	FTM_ACTION_ID     		xActionionID
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_ACTION_DEL_PARAMS	xReq;
	FTDM_RESP_ACTION_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_ACTION_DEL;
	xReq.nLen	=	sizeof(xReq);
	xReq.xID	=	xActionionID;

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
	FTM_ACTION_ID     		xActionionID,
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
	xReq.xID	=	xActionionID;

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
	FTM_RULE_ID     		xRuleionID
)
{
	ASSERT(pSession != NULL);

	FTM_RET						nRet;
	FTDM_REQ_RULE_DEL_PARAMS	xReq;
	FTDM_RESP_RULE_DEL_PARAMS	xResp;

	xReq.xCmd	=	FTDM_CMD_RULE_DEL;
	xReq.nLen	=	sizeof(xReq);
	xReq.xID	=	xRuleionID;

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
	FTM_RULE_ID     		xRuleionID,
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
	xReq.xID	=	xRuleionID;

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
			//TRACE("recv(%08lx, pResp, %d, MSG_DONTWAIT)\n", pSession->hSock, nLen);
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

