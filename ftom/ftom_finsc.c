#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftom.h"

static 
FTM_RET FTOM_FINS_request
(
	FTOM_FINS_SESSION_PTR	pSession, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_INT					nReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_INT					nRespLen
);

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_FINS_init
(
	FTOM_FINS_CFG_PTR pConfig
)
{
	return	FTM_RET_OK;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_FINS_final
(
	FTM_VOID
)
{
	FTM_finalEPTypeString();

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTOM_FINS_connect
(
	FTOM_FINS_SESSION_PTR 	pSession,
	FTM_IP_ADDR			xIP,
	FTM_USHORT 			usPort 
)
{
	int 	hSock;
	struct sockaddr_in 	xServer;
	FTM_RET	xRet;

	if ( pSession == NULL )
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	FTM_LOCK_init(&pSession->xLock);
	hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == -1)
	{
		xRet = FTM_RET_COMM_SOCK_ERROR;
		ERROR2(xRet, "Could not create socket.\n");	
		return	xRet;
	}

	xServer.sin_addr.s_addr	= xIP;
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(usPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		xRet = FTM_RET_COMM_ERROR;
		ERROR2(xRet, "Could not create socket.\n");	
		return	xRet;	
	}
	
	pSession->hSock = hSock;
	pSession->nTimeout = 5000;

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTOM_FINS_disconnect
(
 	FTOM_FINS_SESSION_PTR	pSession
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
FTM_RET FTOM_FINS_isConnected
(
	FTOM_FINS_SESSION_PTR	pSession,
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
 * Internal Functions
 *****************************************************************/
FTM_RET FTOM_FINS_request
(
	FTOM_FINS_SESSION_PTR 	pSession, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_INT					nReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_INT					nRespLen
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
		ERROR2(xRet, "Failed to send!\n");
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

	FTM_LOCK_reset(&pSession->xLock);

	return	xRet;
}

