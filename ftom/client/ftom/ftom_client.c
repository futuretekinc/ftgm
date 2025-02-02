#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_client.h"
#include "ftom_params.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_CLIENT

static
FTM_VOID_PTR	FTOM_CLIENT_threadMain
(
	FTM_VOID_PTR pData
);

FTM_RET	FTOM_CLIENT_destroy
(
	FTOM_CLIENT_PTR	_PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	if ((*ppClient)->fDestroy != NULL)
	{
		return	(*ppClient)->fDestroy(ppClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_init
(
	FTOM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->fInit != NULL)
	{
		return	pClient->fInit(pClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_internalInit
(
	FTOM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;

	pClient->fInit 		= FTOM_CLIENT_internalInit;
	pClient->fFinal		= FTOM_CLIENT_internalFinal;
	pClient->fSetNotifyCB = FTOM_CLIENT_setNotifyInternalCB;

	pClient->pNotifyData = NULL;

	xRet = FTOM_MSGQ_create(&pClient->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Can't create message queue.[%08lx]\n", xRet);	
		return	xRet;
	}

	pClient->xMain.bStop= FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_final
(
	FTOM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	if (!pClient->xMain.bStop)
	{
		FTOM_CLIENT_stop(pClient);	
	}

	if (pClient->fFinal != NULL)
	{
		return	pClient->fFinal(pClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_internalFinal
(
	FTOM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	if (pClient->pMsgQ != NULL)
	{
		xRet = FTOM_MSGQ_destroy(&pClient->pMsgQ);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to finaizlie message queue.\n");
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_start
(
	FTOM_CLIENT_PTR	pClient
)
{
	ASSERT(pClient != NULL);

	FTM_RET	xRet;

	TRACE("The client started!\n");

	if (!pClient->xMain.bStop)
	{
		TRACE("The client is already started!\n");
		return	FTM_RET_ALREADY_RUNNING;
	}

	if (pthread_create(&pClient->xMain.xThread, NULL, FTOM_CLIENT_threadMain, pClient) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The client main task creation failed!\n");
		return	xRet;
	}

	if (pClient->fStart != NULL)
	{
		xRet = pClient->fStart(pClient);
		if (xRet != FTM_RET_OK)
		{
			pClient->xMain.bStop = FTM_TRUE;
			goto error;
		}
	}

	TRACE("The client started!\n");

	return	FTM_RET_OK;

error:
	TRACE("The client start error!\n");

	pthread_join(pClient->xMain.xThread, NULL);
	pClient->xMain.xThread= 0;

	return	xRet;
}

FTM_RET	FTOM_CLIENT_stop
(
	FTOM_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;

	if (pClient->fStop != NULL)
	{
		xRet = pClient->fStop(pClient);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	pClient->xMain.bStop = FTM_TRUE;
	pthread_join(pClient->xMain.xThread, NULL);
	pClient->xMain.xThread = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_waitingForFinished
(
	FTOM_CLIENT_PTR pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->fWaitingForFinished != NULL)
	{
		return	pClient->fWaitingForFinished(pClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}


FTM_RET	FTOM_CLIENT_sendMessage
(
	FTOM_CLIENT_PTR	pClient,
	FTOM_MSG_PTR	pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(pClient->pMsgQ, pMsg);	
}

FTM_RET	FTOM_CLIENT_messageProcess
(
	FTOM_CLIENT_PTR pClient,
	FTOM_MSG_PTR	pMsg
)
{
	ASSERT(pClient != NULL);

	if (pClient->fMessageProcess != NULL)
	{
		return	pClient->fMessageProcess(pClient, pMsg);
	}
	else
	{
		TRACE("Message process not supported!\n");	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_loadConfigFromFile
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pConfigFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfigFileName != NULL);

	FTM_RET	xRet;
	FTM_CONFIG_PTR	pConfig;

	xRet = FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{   
		ERROR2(xRet, "Configuration loading failed!\n");
		return  xRet;
	}   

	xRet = FTOM_CLIENT_loadConfig(pClient, pConfig);
	if (xRet != FTM_RET_OK)
	{   
		ERROR2(xRet, "Faield to load configuration!\n");    
	}   

	FTM_CONFIG_destroy(&pConfig);

	return  xRet;

}

FTM_RET	FTOM_CLIENT_saveConfigToFile
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pConfigFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfigFileName != NULL);

	FTM_RET	xRet;
	FTM_CONFIG_PTR	pConfig;

	xRet = FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{   
		ERROR2(xRet, "Configuration loading failed!\n");
		return  xRet;
	}   

	xRet = FTOM_CLIENT_saveConfig(pClient, pConfig);
	if (xRet != FTM_RET_OK)
	{   
		ERROR2(xRet, "Faield to load configuration!\n");    
	}   

	FTM_CONFIG_destroy(&pConfig);

	return  xRet;

}

FTM_RET	FTOM_CLIENT_loadConfig
(
	FTOM_CLIENT_PTR			pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	if (pClient->fLoadConfig != NULL)
	{
		return	pClient->fLoadConfig(pClient, pConfig);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_saveConfig
(
	FTOM_CLIENT_PTR			pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	if (pClient->fSaveConfig != NULL)
	{
		return	pClient->fSaveConfig(pClient, pConfig);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_showConfig
(
	FTOM_CLIENT_PTR			pClient
)
{
	ASSERT(pClient != NULL);

	if (pClient->fShowConfig != NULL)
	{
		return	pClient->fShowConfig(pClient);
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_CLIENT_setNotifyCB
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_NOTIFY_CB	pCB,
	FTM_VOID_PTR			pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCB != NULL);

	if (pClient->fSetNotifyCB != NULL)
	{
		return	pClient->fSetNotifyCB(pClient, pCB, pData);
	}
	
	return	FTOM_CLIENT_setNotifyInternalCB(pClient, pCB, pData);
}

FTM_RET	FTOM_CLIENT_setNotifyInternalCB
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_NOTIFY_CB	pCB,
	FTM_VOID_PTR			pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCB != NULL);

	pClient->fNotifyCB	= pCB;
	pClient->pNotifyData= pData;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_setFunctionSet
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_FUNCTION_SET_PTR	pFunctionSet
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFunctionSet != NULL);

	
	if (pFunctionSet->fDestroy != NULL)
	{
		pClient->fDestroy = pFunctionSet->fDestroy;
	}

	if (pFunctionSet->fInit != NULL)
	{
		pClient->fInit = pFunctionSet->fInit;
	}

	if (pFunctionSet->fFinal != NULL)
	{
		pClient->fFinal = pFunctionSet->fFinal;
	}

	if (pFunctionSet->fStart != NULL)
	{
		pClient->fStart = pFunctionSet->fStart;
	}

	if (pFunctionSet->fStop != NULL)
	{
		pClient->fStop = pFunctionSet->fStop;
	}

	if (pFunctionSet->fWaitingForFinished != NULL)
	{
		pClient->fWaitingForFinished = pFunctionSet->fWaitingForFinished;
	}

	if (pFunctionSet->fLoadConfig != NULL)
	{
		pClient->fLoadConfig = pFunctionSet->fLoadConfig;
	}

	if (pFunctionSet->fSaveConfig != NULL)
	{
		pClient->fSaveConfig = pFunctionSet->fSaveConfig;
	}

	if (pFunctionSet->fShowConfig != NULL)
	{
		pClient->fShowConfig = pFunctionSet->fShowConfig;
	}

	if (pFunctionSet->fMessageProcess != NULL)
	{
		pClient->fMessageProcess = pFunctionSet->fMessageProcess;
	}

	if (pFunctionSet->fSetNotifyCB != NULL)
	{
		pClient->fSetNotifyCB = pFunctionSet->fSetNotifyCB;
	}

	if (pFunctionSet->fRequest != NULL)
	{
		pClient->fRequest = pFunctionSet->fRequest;
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_CLIENT_threadMain
(
	FTM_VOID_PTR pData
)
{
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_RET		xRet;
	FTM_TIMER	xLoopTimer;
	FTM_ULONG	ulLoopInterval;

	FTM_TIMER_initS(&xLoopTimer, 1);

	TRACE("The client main task was started!\n");

	pClient->xMain.bStop = FTM_FALSE;

	while(!pClient->xMain.bStop)
	{
		FTOM_MSG_PTR	pBaseMsg;

		FTM_TIMER_remainMS(&xLoopTimer, &ulLoopInterval);

		while (FTM_TRUE)
		{
			xRet = FTOM_MSGQ_timedPop(pClient->pMsgQ, ulLoopInterval, &pBaseMsg);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			FTOM_CLIENT_messageProcess(pClient, pBaseMsg);

			FTOM_MSG_destroy(&pBaseMsg);
		}

		FTM_TIMER_addMS(&xLoopTimer, 100);
	}

	TRACE("The client main task was finished !\n");

	return	0;
}

FTM_RET FTOM_CLIENT_NODE_create
(
	FTOM_CLIENT_PTR		pClient,
	FTM_NODE_PTR		pInfo,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG			ulDIDLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID	!= NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_CREATE_PARAMS		xReq;
	FTOM_RESP_NODE_CREATE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

#if 0
	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid node information!\n");
		return	xRet;	
	}
#endif
	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xNodeInfo, pInfo, sizeof(FTM_NODE));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	else
	{
		if (strlen(xResp.pDID) + 1 > ulDIDLen)
		{
			return	FTM_RET_BUFFER_TOO_SMALL;	
		}

		strcpy(pDID, xResp.pDID);
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_destroy
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR			pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_DESTROY_PARAMS	xReq;
	FTOM_RESP_NODE_DESTROY_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_count
(
	FTOM_CLIENT_PTR		pClient,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_COUNT_PARAMS	xReq;
	FTOM_RESP_NODE_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_COUNT;
	xReq.ulLen	=	sizeof(xReq);
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}	

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_getAt
(
	FTOM_CLIENT_PTR		pClient,
	FTM_ULONG				ulIndex,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_GET_AT_PARAMS		xReq;
	FTOM_RESP_NODE_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_get
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET							xRet;
	FTOM_REQ_NODE_GET_PARAMS		xReq;
	FTOM_RESP_NODE_GET_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd 	=	FTOM_CMD_NODE_GET;
	xReq.ulLen	=	sizeof(xReq);
	strcpy(xReq.pDID, pDID);
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xNodeInfo, sizeof(FTM_NODE));
	}
	
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_FIELD		xFields,
	FTM_NODE_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_SET_PARAMS	xReq;
	FTOM_RESP_NODE_SET_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_SET;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_NODE));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_run
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_RUN_PARAMS	xReq;
	FTOM_RESP_NODE_RUN_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_RUN;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_stop
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_STOP_PARAMS	xReq;
	FTOM_RESP_NODE_STOP_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_STOP;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_NODE_isRun
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pDID,
	FTM_BOOL_PTR		pbRun
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET						xRet;
	FTOM_REQ_NODE_IS_RUN_PARAMS	xReq;
	FTOM_RESP_NODE_IS_RUN_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_IS_RUN;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pbRun = xResp.bRun;
	}

	return	xResp.xRet;
}

FTM_RET	FTOm_CLIENT_NODE_registerAtServer
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET						xRet;
	FTM_ULONG					ulRespLen;
	FTOM_REQ_NODE_REGISTER_AT_SERVER_PARAMS 	xReq;
	FTOM_RESP_NODE_REGISTER_AT_SERVER_PARAMS 	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_REGISTER_AT_SERVER;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_NODE_setServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_BOOL		bRegistered
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET						xRet;
	FTM_ULONG					ulRespLen;
	FTOM_REQ_NODE_SET_SERVER_REGISTERED_PARAMS 	xReq;
	FTOM_RESP_NODE_SET_SERVER_REGISTERED_PARAMS 	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_SET_REGISTERED;
	xReq.ulLen	=	sizeof(xReq);
	xReq.bRegistered =	bRegistered;
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_NODE_getServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pbRegistered
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pbRegistered != NULL);

	FTM_RET						xRet;
	FTM_ULONG					ulRespLen;
	FTOM_REQ_NODE_GET_SERVER_REGISTERED_PARAMS 	xReq;
	FTOM_RESP_NODE_GET_SERVER_REGISTERED_PARAMS 	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_GET_REGISTERED;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pDID, pDID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}
	else
	{
		if (xResp.xRet == FTM_RET_OK)
		{
			*pbRegistered = xResp.bRegistered;	
		}
	}


	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_NODE_setReportInterval
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_ULONG		ulReportInterval
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET					xRet;
	FTM_ULONG				ulRespLen;
	FTOM_REQ_NODE_SET_REPORT_INTERVAL_PARAMS	xReq;
	FTOM_RESP_NODE_SET_REPORT_INTERVAL_PARAMS	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd = FTOM_CMD_NODE_SET_REPORT_INTERVAL;
	xReq.ulLen= sizeof(xReq);
	strncpy(xReq.pDID, pDID, FTM_DID_LEN);
	xReq.ulReportInterval = ulReportInterval;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/************************************************
 * EP
 ************************************************/
FTM_RET FTOM_CLIENT_EP_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo,
	FTM_EP_PTR			pNewInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(pNewInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_CREATE_PARAMS	xReq;
	FTOM_RESP_EP_CREATE_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

    FTM_EP_print(pInfo);
	xReq.xCmd	=	FTOM_CMD_EP_CREATE;
	xReq.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(pNewInfo, &xResp.xInfo, sizeof(FTM_EP));

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pClient != NULL);

	FTM_RET					xRet;
	FTOM_REQ_EP_DESTROY_PARAMS	xReq;
	FTOM_RESP_EP_DESTROY_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_DESTROY;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_count
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_COUNT_PARAMS	xReq;
	FTOM_RESP_EP_COUNT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_COUNT;
	xReq.xType	=	xType;
	xReq.ulLen	=	sizeof(xReq);

	if (pDID != NULL)
	{
		strncpy(xReq.pDID, pDID, FTM_DID_LEN);
	}

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.nCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_getList
(
	FTOM_CLIENT_PTR		pClient,
	FTM_EP_TYPE			xType,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG			ulIndex,
	FTM_CHAR			pEPIDList[][FTM_EPID_LEN+1],
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTOM_REQ_EP_GET_LIST_PARAMS		xReq;
	FTM_ULONG						nRespSize = 0;
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	nRespSize = sizeof(FTOM_RESP_EP_GET_LIST_PARAMS) + (FTM_EPID_LEN+1) * ulMaxCount;
	pResp = (FTOM_RESP_EP_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xReq.xCmd		=	FTOM_CMD_EP_GET_LIST;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xType		=	xType;
	if (pDID != NULL)
	{
		strncpy(xReq.pDID, pDID, FTM_DID_LEN);
	}
	xReq.ulMaxCount	=	ulMaxCount;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				nRespSize,
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pResp);
		return	FTM_RET_ERROR;	
	}

	xRet = pResp->xRet;

	if (xRet == FTM_RET_OK)
	{
		memcpy(pEPIDList, pResp->pEPIDList, (FTM_EPID_LEN+1) * pResp->ulCount);
		*pulCount = pResp->ulCount;
	}

	FTM_MEM_free(pResp);

	return	xRet;
}

FTM_RET FTOM_CLIENT_EP_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_GET_PARAMS		xReq;
	FTOM_RESP_EP_GET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_GET;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_getAt
(
	FTOM_CLIENT_PTR		pClient,
	FTM_ULONG			ulIndex,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_GET_AT_PARAMS	xReq;
	FTOM_RESP_EP_GET_AT_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_GET_AT;
	xReq.ulLen	=	sizeof(xReq);
	xReq.ulIndex=	ulIndex;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_EP));
	}
	
	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_FIELD		xFields,
	FTM_EP_PTR			pInfo
)
{
	ASSERT(pClient != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_SET_PARAMS		xReq;
	FTOM_RESP_EP_SET_PARAMS		xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_SET;
	xReq.ulLen	=	sizeof(xReq);
	xReq.xFields=	xFields;
	strncpy(xReq.pEPID, pEPID, FTM_ID_LEN);
	memcpy(&xReq.xInfo, pInfo, sizeof(FTM_EP));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET FTOM_CLIENT_EP_isRun
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL_PTR		pbRun
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_IS_RUN_PARAMS	xReq;
	FTOM_RESP_EP_IS_RUN_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_IS_RUN;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pbRun = xResp.bRun;
	}

	return	xResp.xRet;
}

FTM_RET	FTOm_CLIENT_EP_registerAtServer
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET						xRet;
	FTM_ULONG					ulRespLen;
	FTOM_REQ_EP_REGISTER_AT_SERVER_PARAMS 	xReq;
	FTOM_RESP_EP_REGISTER_AT_SERVER_PARAMS 	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_NODE_REGISTER_AT_SERVER;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_setServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL		bRegistered
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET						xRet;
	FTM_ULONG					ulRespLen;
	FTOM_REQ_EP_SET_SERVER_REGISTERED_PARAMS 	xReq;
	FTOM_RESP_EP_SET_SERVER_REGISTERED_PARAMS 	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_SET_REGISTERED;
	xReq.ulLen	=	sizeof(xReq);
	xReq.bRegistered =	bRegistered;
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_getServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL_PTR	pbRegistered
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pbRegistered != NULL);

	FTM_RET						xRet;
	FTM_ULONG					ulRespLen;
	FTOM_REQ_EP_GET_SERVER_REGISTERED_PARAMS 	xReq;
	FTOM_RESP_EP_GET_SERVER_REGISTERED_PARAMS 	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_GET_REGISTERED;
	xReq.ulLen	=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}
	else
	{
		if (xResp.xRet == FTM_RET_OK)
		{
			*pbRegistered = xResp.bRegistered;	
		}
	}


	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_setReportInterval
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulReportInterval
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET					xRet;
	FTM_ULONG				ulRespLen;
	FTOM_REQ_EP_SET_REPORT_INTERVAL_PARAMS	xReq;
	FTOM_RESP_EP_SET_REPORT_INTERVAL_PARAMS	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd = FTOM_CMD_EP_SET_REPORT_INTERVAL;
	xReq.ulLen= sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
	xReq.ulReportInterval = ulReportInterval;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_remoteSet
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);

	FTM_RET					xRet;
	FTM_ULONG				ulRespLen;
	FTOM_REQ_EP_REMOTE_SET_PARAMS	xReq;
	FTOM_RESP_EP_REMOTE_SET_PARAMS	xResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd = FTOM_CMD_EP_REMOTE_SET;
	xReq.ulLen= sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
	memcpy(&xReq.xData, pData, sizeof(FTM_EP_DATA));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pEPData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPData != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_ADD_PARAMS		xReq;
	FTOM_RESP_EP_DATA_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_DATA_ADD;
	xReq.nLen	=	sizeof(xReq);
	strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);
	memcpy(&xReq.xData, pEPData, sizeof(FTM_EP_DATA));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_info
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulBeginTime != NULL);
	ASSERT(pulEndTime != NULL);
	ASSERT(pCount != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_INFO_PARAMS	xReq;
	FTOM_RESP_EP_DATA_INFO_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_INFO;
	xReq.ulLen		=	sizeof(xReq);
	strncpy( xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulBeginTime 	= xResp.ulBeginTime;
		*pulEndTime 	= xResp.ulEndTime;
		*pCount 		= xResp.ulCount;
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTOM_CLIENT_EP_DATA_getLast
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET								xRet;
	FTOM_REQ_EP_DATA_GET_LAST_PARAMS	xReq;
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd	=	FTOM_CMD_EP_DATA_GET_LAST;
	xReq.ulLen	=	sizeof(xReq);
	strncpy( xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pData, &xResp.xData, sizeof(FTM_EP_DATA));
	}
	
	return	xResp.xRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pnCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);
	ASSERT(pnCount != NULL);

	FTM_RET									xRet;
	FTOM_REQ_EP_DATA_GET_LIST_PARAMS		xReq;
	FTM_ULONG								nRespSize = 0;
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp = NULL;
	FTM_ULONG								ulRespLen;
	FTM_ULONG								ulRespCount = 0;
	FTM_BOOL								bStop = FTM_FALSE;
	static	FTM_ULONG						ulLimitCount  = 0;

	if (ulLimitCount == 0)
	{
		ulLimitCount = (FTOM_DEFAULT_PACKET_SIZE - sizeof(FTOM_RESP_EP_DATA_GET_LIST_WITH_TIME_PARAMS)) / sizeof(FTM_EP_DATA) / 10 * 10; 
	}

	while(!bStop)
	{
		FTM_ULONG	ulReqCount;

		if (ulMaxCount > ulLimitCount) 
		{
			ulReqCount = ulLimitCount;	
		}
		else
		{
			ulReqCount = ulMaxCount;	
		}

		nRespSize = sizeof(FTOM_RESP_EP_DATA_GET_LIST_PARAMS) + sizeof(FTM_EP_DATA) * ulReqCount;
		pResp = (FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
		if (pResp == NULL)
		{
			ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %d]!\n", nRespSize);
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
	
		memset(&xReq, 0, sizeof(xReq));

		xReq.xCmd		=	FTOM_CMD_EP_DATA_GET_LIST;
		xReq.ulLen		=	sizeof(xReq);
		strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);
		xReq.nStartIndex=	nStartIndex;
		xReq.nCount		=	ulReqCount;
	
		xRet = pClient->fRequest(
					pClient, 
					(FTM_VOID_PTR)&xReq, 
					sizeof(xReq), 
					(FTM_VOID_PTR)pResp, 
					nRespSize,
					&ulRespLen);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Request error!\n");
			FTM_MEM_free(pResp);
			return	FTM_RET_ERROR;	
		}
	
		xRet = pResp->xRet;

		if (pResp->xRet == FTM_RET_OK)
		{
			FTM_INT	i;
	
			for( i = 0 ; i < pResp->nCount && i < ulReqCount ; i++)
			{
				memcpy(&pData[ulRespCount + i], &pResp->pData[i], sizeof(FTM_EP_DATA));
			}

			ulRespCount += pResp->nCount;
			nStartIndex += pResp->nCount;
			ulMaxCount -= pResp->nCount;

			if ((pResp->nCount == 0) || (!pResp->bRemain) || (ulMaxCount == 0))
			{
				bStop = FTM_TRUE;	
			}
		}
		else
		{
			ERROR2(pResp->xRet, "FTOM request error!");
			bStop = FTM_TRUE;
		}
	
		FTM_MEM_free(pResp);
	}

	*pnCount = ulRespCount;

	return	xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_getListWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pnCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);
	ASSERT(pnCount != NULL);

	FTM_RET				xRet;
	FTOM_REQ_EP_DATA_GET_LIST_WITH_TIME_PARAMS		xReq;
	FTM_ULONG			nRespSize = 0;
	FTOM_RESP_EP_DATA_GET_LIST_WITH_TIME_PARAMS_PTR	pResp = NULL;
	FTM_ULONG			ulRespLen;
	FTM_ULONG			ulRespCount = 0;
	FTM_BOOL			bStop = FTM_FALSE;
	FTM_BOOL			bAscending = FTM_TRUE;
	static	FTM_ULONG	ulLimitCount  = 0;

	if (ulLimitCount == 0)
	{
		ulLimitCount = (FTOM_DEFAULT_PACKET_SIZE - sizeof(FTOM_RESP_EP_DATA_GET_LIST_WITH_TIME_PARAMS)) / sizeof(FTM_EP_DATA) / 10 * 10; 
	}

	if (ulBegin > ulEnd)
	{
		FTM_ULONG	ulTemp;

		bAscending = FTM_FALSE;	
		ulTemp 	= ulBegin;
		ulBegin	= ulEnd;
		ulEnd 	= ulTemp;
	}

	while(!bStop)
	{
		FTM_ULONG	ulReqCount;

		if (ulMaxCount > ulLimitCount) 
		{
			ulReqCount = ulLimitCount;	
		}
		else
		{
			ulReqCount = ulMaxCount;	
		}

		nRespSize = sizeof(FTOM_RESP_EP_DATA_GET_LIST_WITH_TIME_PARAMS) + sizeof(FTM_EP_DATA) * ulReqCount;
		pResp = (FTOM_RESP_EP_DATA_GET_LIST_WITH_TIME_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
		if (pResp == NULL)
		{
			ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %d]!\n", nRespSize);
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
	
		memset(&xReq, 0, sizeof(xReq));

		xReq.xCmd		=	FTOM_CMD_EP_DATA_GET_LIST_WITH_TIME;
		xReq.ulLen		=	sizeof(xReq);
		strncpy(xReq.pEPID,	pEPID, FTM_EPID_LEN);
		xReq.ulBegin	=	ulBegin;
		xReq.ulEnd		=	ulEnd;
		xReq.bAscending	=	bAscending;
		xReq.nCount		=	ulReqCount;

		xRet = pClient->fRequest(
					pClient, 
					(FTM_VOID_PTR)&xReq, 
					sizeof(xReq), 
					(FTM_VOID_PTR)pResp, 
					nRespSize,
					&ulRespLen);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Request error!\n");
			FTM_MEM_free(pResp);
			return	FTM_RET_ERROR;	
		}
	
		xRet = pResp->xRet;

		if (pResp->xRet == FTM_RET_OK)
		{
			FTM_INT	i;
	
			for( i = 0 ; i < pResp->nCount && i < ulReqCount ; i++)
			{
				memcpy(&pData[ulRespCount + i], &pResp->pData[i], sizeof(FTM_EP_DATA));
			}

			if (pResp->nCount != 0)
			{
				ulRespCount += pResp->nCount;
				if (bAscending)
				{
					ulBegin	= pResp->pData[pResp->nCount - 1].ulTime + 1 ;
				}
				else
				{
					ulEnd	= pResp->pData[pResp->nCount - 1].ulTime - 1 ;
				}
				ulMaxCount  -= pResp->nCount;
			}

			if ((pResp->nCount == 0) || (!pResp->bRemain) || (ulMaxCount == 0))
			{
				bStop = FTM_TRUE;	
			}
		}
		else
		{
			ERROR2(pResp->xRet, "FTOM request error!\n");
			bStop = FTM_TRUE;
		}
	
		FTM_MEM_free(pResp);
	}

	*pnCount = ulRespCount;

	return	xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pClient != NULL);

	FTM_RET							xRet;
	FTOM_REQ_EP_DATA_DEL_PARAMS		xReq;
	FTOM_RESP_EP_DATA_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;
	
	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
	xReq.ulIndex	=	ulIndex;
	xReq.ulCount	=	ulCount;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulDeletedCount = xResp.ulCount;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTOM_CLIENT_EP_DATA_delWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);

	FTM_RET									xRet;
	FTOM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS	xReq;
	FTOM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS	xResp;
	FTM_ULONG						ulRespLen;
	
	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_DEL_WITH_TIME;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
	xReq.ulBegin	=	ulBegin;
	xReq.ulEnd		=	ulEnd;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulDeletedCount = xResp.ulCount;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_DATA_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_DATA_COUNT_PARAMS	xReq;
	FTOM_RESP_EP_DATA_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_COUNT;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_DATA_countWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS	xReq;
	FTOM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_COUNT_WITH_TIME;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
	xReq.ulStart	=	ulStart;
	xReq.ulEnd		=	ulEnd;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_EP_DATA_type
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_TYPE_PTR	pType
)
{
	ASSERT(pClient != NULL);
	ASSERT(pType != NULL);

	FTM_RET						xRet;
	FTOM_REQ_EP_DATA_TYPE_PARAMS	xReq;
	FTOM_RESP_EP_DATA_TYPE_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_TYPE;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pType = xResp.xType;
	}

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_EP_DATA_setServerTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulTime
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET		xRet;
	FTOM_REQ_EP_DATA_SET_SERVER_TIME_PARAMS		xReq;
	FTOM_RESP_EP_DATA_SET_SERVER_TIME_PARAMS	xResp;
	FTM_ULONG	ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_EP_DATA_SET_SERVER_TIME;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pEPID, pEPID, FTM_EPID_LEN);
	xReq.ulTime		=	ulTime;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_TRIGGER_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_ADD_PARAMS	xReq;
	FTOM_RESP_TRIGGER_ADD_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_ADD;
	xReq.ulLen		=	sizeof(xReq);
	memcpy(&xReq.xTrigger, pTrigger, sizeof(FTM_TRIGGER));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		strncpy(pTrigger->pID, xResp.pTriggerID, FTM_ID_LEN);
	}
	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pTriggerID
)
{
	ASSERT(pClient != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_DEL_PARAMS	xReq;
	FTOM_RESP_TRIGGER_DEL_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pTriggerID, pTriggerID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_COUNT_PARAMS	xReq;
	FTOM_RESP_TRIGGER_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_COUNT;
	xReq.ulLen		=	sizeof(xReq);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_GET_PARAMS	xReq;
	FTOM_RESP_TRIGGER_GET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_GET;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pTriggerID, pID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pTrigger, &xResp.xTrigger, sizeof(FTM_TRIGGER));
	}	

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_TRIGGER_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_GET_AT_PARAMS	xReq;
	FTOM_RESP_TRIGGER_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_GET_AT;
	xReq.ulLen		=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pTrigger, &xResp.xTrigger, sizeof(FTM_TRIGGER));
	}	

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_TRIGGER_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pTriggerID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pTrigger
)
{
	ASSERT(pClient != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET						xRet;
	FTOM_REQ_TRIGGER_SET_PARAMS	xReq;
	FTOM_RESP_TRIGGER_SET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_TRIG_SET;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xFields	=	xFields;
	strcpy(xReq.pTriggerID, pTriggerID);
	memcpy(&xReq.xTrigger, pTrigger, sizeof(FTM_TRIGGER));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_ACTION_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ACTION_PTR	pAction,
	FTM_CHAR_PTR	pActionID,
	FTM_ULONG		ulIDLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_ADD_PARAMS	xReq;
	FTOM_RESP_ACTION_ADD_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_ADD;
	xReq.ulLen		=	sizeof(xReq);
	memcpy(&xReq.xAction, pAction, sizeof(FTM_ACTION));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		strncpy(pActionID, xResp.pActionID, ulIDLen);
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pClient != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_DEL_PARAMS	xReq;
	FTOM_RESP_ACTION_DEL_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_COUNT_PARAMS	xReq;
	FTOM_RESP_ACTION_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_COUNT;
	xReq.ulLen		=	sizeof(xReq);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_GET_PARAMS	xReq;
	FTOM_RESP_ACTION_GET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_GET;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pAction, &xResp.xAction, sizeof(FTM_ACTION));
	}	

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_ACTION_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_GET_AT_PARAMS	xReq;
	FTOM_RESP_ACTION_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_GET_AT;
	xReq.ulLen		=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pAction, &xResp.xAction, sizeof(FTM_ACTION));
	}	

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_ACTION_set
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pAction
)
{
	ASSERT(pClient != NULL);
	ASSERT(pAction != NULL);

	FTM_RET						xRet;
	FTOM_REQ_ACTION_SET_PARAMS	xReq;
	FTOM_RESP_ACTION_SET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_ACTION_SET;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xFields	=	xFields;
	strncpy(xReq.pActionID, pActionID, FTM_ID_LEN);
	memcpy(&xReq.xAction, pAction, sizeof(FTM_ACTION));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_RULE_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_RULE_PTR	pRule,
	FTM_CHAR_PTR	pRuleID,
	FTM_ULONG		ulIDLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_ADD_PARAMS	xReq;
	FTOM_RESP_RULE_ADD_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_ADD;
	xReq.ulLen		=	sizeof(xReq);
	memcpy(&xReq.xRule, pRule, sizeof(FTM_RULE));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		strncpy(pRuleID, xResp.pRuleID, ulIDLen);
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pClient != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_DEL_PARAMS	xReq;
	FTOM_RESP_RULE_DEL_PARAMS	xResp;
	FTM_ULONG					ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_DEL;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_COUNT_PARAMS	xReq;
	FTOM_RESP_RULE_COUNT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_COUNT;
	xReq.ulLen		=	sizeof(xReq);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_GET_PARAMS	xReq;
	FTOM_RESP_RULE_GET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_GET;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pRule, &xResp.xRule, sizeof(FTM_RULE));
	}	

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_RULE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_GET_AT_PARAMS	xReq;
	FTOM_RESP_RULE_GET_AT_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_GET_AT;
	xReq.ulLen		=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	if (xResp.xRet == FTM_RET_OK)
	{
		memcpy(pRule, &xResp.xRule, sizeof(FTM_RULE));
	}	

	return	xResp.xRet;
}


FTM_RET	FTOM_CLIENT_RULE_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRule
)
{
	ASSERT(pClient != NULL);
	ASSERT(pRule != NULL);

	FTM_RET						xRet;
	FTOM_REQ_RULE_SET_PARAMS	xReq;
	FTOM_RESP_RULE_SET_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_RULE_SET;
	xReq.ulLen		=	sizeof(xReq);
	xReq.xFields	=	xFields;
	strncpy(xReq.pRuleID, pRuleID, FTM_ID_LEN);
	memcpy(&xReq.xRule, pRule, sizeof(FTM_RULE));

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_LOG_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulStartIndex,
	FTM_ULONG		ulMaxCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pLogs != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET								xRet;
	FTOM_REQ_LOG_GET_LIST_PARAMS		xReq;
	FTM_ULONG							nRespSize = 0;
	FTOM_RESP_LOG_GET_LIST_PARAMS_PTR	pResp = NULL;
	FTM_ULONG							ulRespLen;
	FTM_ULONG							ulRespCount = 0;
	FTM_BOOL							bStop = FTM_FALSE;

	while(!bStop)
	{
		FTM_ULONG	ulReqCount;

		if (ulMaxCount > 20) 
		{
			ulReqCount = 20;	
		}
		else
		{
			ulReqCount = ulMaxCount;	
		}

		nRespSize = sizeof(FTOM_RESP_LOG_GET_LIST_PARAMS) + sizeof(FTM_LOG) * ulReqCount;
		pResp = (FTOM_RESP_LOG_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(nRespSize);
		if (pResp == NULL)
		{
			ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %d]!\n", nRespSize);
			return	FTM_RET_NOT_ENOUGH_MEMORY;
		}
	
		memset(&xReq, 0, sizeof(xReq));

		xReq.xCmd		=	FTOM_CMD_LOG_GET_LIST;
		xReq.ulLen		=	sizeof(xReq);
		xReq.ulIndex	=	ulStartIndex;
		xReq.ulCount	=	ulReqCount;
	
		xRet = pClient->fRequest(
					pClient, 
					(FTM_VOID_PTR)&xReq, 
					sizeof(xReq), 
					(FTM_VOID_PTR)pResp, 
					nRespSize,
					&ulRespLen);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Request error!\n");
			FTM_MEM_free(pResp);
			return	FTM_RET_ERROR;	
		}
	
		xRet = pResp->xRet;

		if (pResp->xRet == FTM_RET_OK)
		{
			FTM_INT	i;
	
			for( i = 0 ; i < pResp->ulCount && i < ulReqCount ; i++)
			{
				memcpy(&pLogs[ulRespCount + i], &pResp->pLogs[i], sizeof(FTM_LOG));
			}

			ulRespCount += pResp->ulCount;
			ulStartIndex += pResp->ulCount;
			ulMaxCount -= pResp->ulCount;

			if ((pResp->ulCount != ulReqCount) || (ulMaxCount == 0))
			{
				bStop = FTM_TRUE;	
			}
		}
		else
		{
			ERROR2(pResp->xRet, "FTOM request error!");
			bStop = FTM_TRUE;
		}
	
		FTM_MEM_free(pResp);
	}

	*pulCount = ulRespCount;

	return	xRet;
}

FTM_RET	FTOM_CLIENT_LOG_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pulDeletedCount != NULL);

	FTM_RET							xRet;
	FTOM_REQ_LOG_DEL_PARAMS			xReq;
	FTOM_RESP_LOG_DEL_PARAMS		xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_LOG_DEL;
	xReq.ulLen		=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	xReq.ulCount	=	ulCount;
	
	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Request error!\n");
		return	FTM_RET_ERROR;	
	}
	
	xRet = xResp.xRet;
	if (xRet == FTM_RET_OK)
	{
		*pulDeletedCount = xResp.ulCount;
	}

	return	xRet;
}

/*****************************************************************
 * Discovery Functions
 *****************************************************************/
FTM_RET	FTOM_CLIENT_DISCOVERY_start
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);

	FTM_RET						xRet;
	FTOM_REQ_DISCOVERY_START_PARAMS	xReq;
	FTOM_RESP_DISCOVERY_START_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_DISCOVERY_START;
	xReq.ulLen		=	sizeof(xReq);
	strncpy(xReq.pIP, pIP, sizeof(xReq.pIP) - 1);
	xReq.usPort = usPort;
	xReq.ulRetryCount = ulRetryCount;

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;	
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_DISCOVERY_getInfo
(
	FTOM_CLIENT_PTR	pClient,
	FTM_BOOL_PTR	pbFinished,
	FTM_ULONG_PTR	pulNodeCount,
	FTM_ULONG_PTR	pulEPCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pbFinished != NULL);
	ASSERT(pulNodeCount != NULL);
	ASSERT(pulEPCount != NULL);

	FTM_RET						xRet;
	FTOM_REQ_DISCOVERY_GET_INFO_PARAMS	xReq;
	FTOM_RESP_DISCOVERY_GET_INFO_PARAMS	xResp;
	FTM_ULONG						ulRespLen;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_DISCOVERY_GET_INFO;
	xReq.ulLen		=	sizeof(xReq);

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)&xResp, 
				sizeof(xResp),
				&ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		*pbFinished = xResp.bFinished;
		if (xResp.bFinished == FTM_TRUE)
		{
			*pulNodeCount = xResp.ulNodeCount;	
			*pulEPCount = xResp.ulEPCount;	
		}
	}

	return	xResp.xRet;
}

FTM_RET	FTOM_CLIENT_DISCOVERY_getNodeList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_NODE_PTR	pNodeList,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pNodeList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulRespLen;
	FTOM_REQ_DISCOVERY_GET_NODE_LIST_PARAMS	xReq;
	FTOM_RESP_DISCOVERY_GET_NODE_LIST_PARAMS_PTR	pResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_DISCOVERY_GET_NODE;
	xReq.ulLen		=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	xReq.ulCount	=	ulMaxCount;

	ulRespLen = sizeof(FTOM_RESP_DISCOVERY_GET_NODE_LIST_PARAMS) + sizeof(FTM_NODE) * ulMaxCount;
	pResp = (FTOM_RESP_DISCOVERY_GET_NODE_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen,
				&ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pNodeList, pResp->pNodeList, sizeof(FTM_NODE)*pResp->ulCount);
		*pulCount = pResp->ulCount;

		xRet = pResp->xRet;
	}

	FTM_MEM_free(pResp);

	return	xRet;
}

FTM_RET	FTOM_CLIENT_DISCOVERY_getEPList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_EP_PTR		pEPList,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulRespLen;
	FTOM_REQ_DISCOVERY_GET_EP_LIST_PARAMS	xReq;
	FTOM_RESP_DISCOVERY_GET_EP_LIST_PARAMS_PTR	pResp;

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCmd		=	FTOM_CMD_DISCOVERY_GET_EP;
	xReq.ulLen		=	sizeof(xReq);
	xReq.ulIndex	=	ulIndex;
	xReq.ulCount	=	ulMaxCount;

	ulRespLen = sizeof(FTOM_RESP_DISCOVERY_GET_EP_LIST_PARAMS) + sizeof(FTM_EP) * ulMaxCount;
	pResp = (FTOM_RESP_DISCOVERY_GET_EP_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = pClient->fRequest(
				pClient, 
				(FTM_VOID_PTR)&xReq, 
				sizeof(xReq), 
				(FTM_VOID_PTR)pResp, 
				ulRespLen,
				&ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pEPList, pResp->pEPList, sizeof(FTM_EP)*pResp->ulCount);
		*pulCount = pResp->ulCount;

		xRet = pResp->xRet;
	}

	FTM_MEM_free(pResp);

	return	xRet;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTOM_CLIENT_request
(
	FTOM_CLIENT_PTR		pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen,
	FTM_ULONG_PTR			pulRespLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	ASSERT(pulRespLen != NULL);

	if (pClient->fRequest != NULL)
	{
		return	pClient->fRequest(pClient, pReq, ulReqLen, pResp, ulRespLen, pulRespLen);	
	}

	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

