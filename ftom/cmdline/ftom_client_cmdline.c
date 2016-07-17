#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include "ftm.h"
#include "ftm_shared_memory.h"
#include "ftom_config.h"
#include "ftom_client.h"
#include "ftom_client_cmdline.h"
#include "ftom_params.h"

static
FTM_RET	FTOM_CLIENT_CL_getSMKey
(
	key_t _PTR_		pxKey,
	FTM_INT_PTR		pnShmID
);

FTM_RET	FTOM_CLIENT_CL_init
(
	FTOM_CLIENT_CL_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	memset(pClient, 0, sizeof(FTOM_CLIENT_CL));

	pClient->xConfig.ulDefaultDataCount = FTOM_DATA_READ_COUNT_DEFAULT;
	pClient->xConfig.ulMaxDataCount = FTOM_DATA_READ_COUNT_MAX;

	pClient->xCommon.fStart = (FTOM_CLIENT_START)FTOM_CLIENT_CL_start;
	pClient->xCommon.fStop = (FTOM_CLIENT_STOP)FTOM_CLIENT_CL_stop;
	pClient->xCommon.fSetConfig = (FTOM_CLIENT_SET_CONFIG)FTOM_CLIENT_CL_setConfig;
	pClient->xCommon.fLoadConfigFromFile = (FTOM_CLIENT_LOAD_CONFIG_FROM_FILE)FTOM_CLIENT_CL_loadConfigFromFile;
	//pClient->xCommon.fSetNotifyCallback = (FTOM_CLIENT_SET_NOTIFY_CALLBACK)FTOM_CLIENT_CL_setNotifyCallback;
	pClient->xCommon.fRequest = (FTOM_CLIENT_REQUEST)FTOM_CLIENT_CL_requestSM;	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_final
(
	FTOM_CLIENT_CL_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_create
(
	FTOM_CLIENT_CL_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTOM_CLIENT_CL_PTR	pClient;

	pClient = (FTOM_CLIENT_CL_PTR)FTM_MEM_malloc(sizeof(FTOM_CLIENT_CL));
	if (pClient == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}
	
	memset(pClient, 0, sizeof(FTOM_CLIENT_CL));

	FTOM_CLIENT_CL_init(pClient);

	*ppClient = pClient;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_destroy
(
	FTOM_CLIENT_CL_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	
	FTOM_CLIENT_CL_final(*ppClient);

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_start
(
	FTOM_CLIENT_CL_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_stop
(
	FTOM_CLIENT_CL_PTR pClient
)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_setConfig
(
	FTOM_CLIENT_CL_PTR			pClient,
	FTOM_CLIENT_CL_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTOM_CLIENT_CL_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CL_loadConfigFromFile
(
	FTOM_CLIENT_CL_PTR	pClient,
	FTM_CHAR_PTR 	pFileName
)
{
	return	FTM_RET_OK;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTOM_CLIENT_CL_requestPipe
(
	FTOM_CLIENT_CL_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulMaxRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	FTM_RET	xRet;
	FTM_INT	nRet;
	FTM_INT	nReadFD, nWriteFD;
	fd_set	xFDSet;
	FTM_INT	nRespLen;
	struct timeval			xTimeout;
	
	nReadFD = open("/tmp/ftom_out", O_RDWR);
	if (nReadFD < 0)
	{
		ERROR("Pipe not exist.\n");	
		return	FTM_RET_ERROR;
	}

	nWriteFD= open("/tmp/ftom_in", O_RDWR);
	if (nWriteFD < 0)
	{
		close(nReadFD);
		ERROR("Pipe not exist.\n");	
		return	FTM_RET_ERROR;
	}

	FD_ZERO(&xFDSet); 			/* clear the set */
  	FD_SET(nReadFD, &xFDSet); 	/* add our file descriptor to the set */

	xTimeout.tv_sec = 1;
	xTimeout.tv_usec = 0;

	write(nWriteFD, pReq, ulReqLen);
	usleep(1);
#if 0
	FTM_INT	i;
	MESSAGE("REQ : ");
	for(i = 0 ; i < ulReqLen ; i++)
	{
		MESSAGE("%02x ", ((FTM_UINT8_PTR)pReq)[i]);
	}
	MESSAGE("\n");
#endif

	nRet = select(nReadFD + 1, &xFDSet, NULL, NULL, &xTimeout);
	if (nRet > 0)
	{	
		nRespLen = read(nReadFD, pResp, ulMaxRespLen);	
		if (nRespLen > 0)
		{
#if 0
			MESSAGE("RESP : ");
			for(i = 0 ; i < nRespLen ; i++)
			{
				MESSAGE("%02x ", ((FTM_UINT8_PTR)pResp)[i]);
			}
			MESSAGE("\n");
#endif
			*pulRespLen = nRespLen;	
			xRet = FTM_RET_OK;
		}
		else
		{
			xRet = FTM_RET_ERROR;	
		}
	}
	else if (nRet == 0)
	{
		xRet = FTM_RET_TIMEOUT;	
	}
	else
	{
		xRet = FTM_RET_ERROR;	
	}

	close(nReadFD);
	close(nWriteFD);

	return	xRet;
}


/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTOM_CLIENT_CL_requestSM
(
	FTOM_CLIENT_CL_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulMaxRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	FTM_RET	xRet;
	FTM_SMP_PTR	pSMP;
	key_t		xKey;
	FTM_INT		nShmID;

	FTOM_CLIENT_CL_getSMKey(&xKey, &nShmID);

	xRet = FTM_SMP_createClient(xKey, nShmID, &pSMP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_SMP_call(pSMP, pReq, ulReqLen, pResp, ulMaxRespLen, pulRespLen, 5000000);
		if (xRet != FTM_RET_OK)
		{
			TRACE("FTM_SMP_call error[%08lx]!\n", xRet);
		}
		FTM_SMP_destroy(&pSMP);
	}
	else
	{
		TRACE("Can't create SMP!\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CL_getSMKey
(
	key_t	_PTR_	pxKey,
	FTM_INT_PTR		pnShmID
)
{
	FILE *pFile;

	*pxKey	= 1234;
	*pnShmID= 0;

	pFile = fopen("/var/run/ftom.smkey", "r");
	if (pFile != NULL)
	{
		FTM_RET	nCount;
		FTM_INT	nKey = 0;
		FTM_INT	nShmID = 0;

		nCount = fscanf(pFile, "%d %d", &nKey, &nShmID);	
		if (nCount == 2)
		{
			*pxKey = nKey;
			*pnShmID = nShmID;
		}

		fclose(pFile);
	}

	return	FTM_RET_OK;

}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTOM_CLIENT_CL_requestNet
(
	FTOM_CLIENT_CL_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulMaxRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	FTM_RET	xRet;
	FTM_INT	hSock;

	hSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (hSock == -1)
	{
		xRet = FTM_RET_COMM_SOCK_ERROR;	
		ERROR2(xRet, "Failed to create subscribe socket.\n");	
		return	xRet;	
	}

	struct timeval tv = { .tv_sec = 5, .tv_usec = 0};
	if (setsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) 
	{
   	 	ERROR2(FTM_RET_ERROR, "Failed to set socket timeout!\n");
	}

	struct sockaddr_in 	xServer;
		
	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr	= inet_addr(pClient->xConfig.xPublishServer.pHost);
	xServer.sin_port 		= htons(pClient->xConfig.xPublishServer.usPort);
			
	TRACE("Subscriber connect to host[%s:%d]\n", pClient->xConfig.xPublishServer.pHost, pClient->xConfig.xPublishServer.usPort);
	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) != 0)
	{
		TRACE("Failed to subscriber connection!\n");
		xRet = FTM_RET_COMM_CONNECTION_ERROR;
		return	xRet;
	}

	FTM_INT	nLen;

	nLen = send(hSock, pReq, ulReqLen, 0);
	if (nLen != ulReqLen)
	{
		xRet = FTM_RET_COMM_SEND_ERROR;
		ERROR2(xRet, "Failed to send request!\n");
	}

	nLen = recv(hSock, pResp, ulRespLen, 0);
	if (nLen > 0)
	{
		xRet = FTM_RET_OK;
	}
	else if ((nLen == 0) || (errno != EAGAIN))
	{
		xRet = FTM_RET_COMM_RECV_ERROR;
		ERROR2(xRet, "Failed to receive response!\n");
	}


	return	FTM_RET_OK;
}

