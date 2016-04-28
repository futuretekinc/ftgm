#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "libconfig.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
#include "ftom_action.h"
#include "ftom_server.h"
#include "ftm_shared_memory.h"
#include "ftom_server_cmd.h"


#ifndef	FTOM_TRACE_IO
#define	FTOM_TRACE_IO		1
#endif

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTOM_SERVER_CALLBACK)FUN }

static 
FTM_VOID_PTR FTOM_SERVER_process
(
	FTM_VOID_PTR 	pData
);

static 
FTM_VOID_PTR FTOM_SERVER_serviceHandler
(
	FTM_VOID_PTR 	pData
);

static
FTM_VOID_PTR	FTOM_SERVER_processPipe
(
	FTM_VOID_PTR	pData
);

static
FTM_VOID_PTR	FTOM_SERVER_processSM
(
	FTM_VOID_PTR	pData
);

static FTM_RET	FTOM_SERVER_serviceCall
(
	FTOM_SERVER_PTR			pServer,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_NODE_create
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_CREATE_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_NODE_destroy
(
	FTOM_SERVER_PTR					pServer,
	FTOM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_DESTROY_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_NODE_count
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_COUNT_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_COUNT_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_NODE_get
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_GET_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_GET_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_NODE_getAt
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_NODE_set
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_NODE_SET_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_NODE_SET_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_create
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_EP_CREATE_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_CREATE_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_destroy
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_EP_DESTROY_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_DESTROY_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_count
(
	FTOM_SERVER_PTR					pServer,
 	FTOM_REQ_EP_COUNT_PARAMS_PTR		pReq,
	FTM_ULONG							ulReqLen,
	FTOM_RESP_EP_COUNT_PARAMS_PTR		pResp,
	FTM_ULONG				ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_PARAMS_PTR			pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_PARAMS_PTR			pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_AT_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_AT_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_SET_PARAMS_PTR			pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_SET_PARAMS_PTR			pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_registrationNotifyReceiver
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_DATA_info
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_DATA_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_DATA_type
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_TYPE_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_TYPE_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
);

static FTM_RET	FTOM_SERVER_EP_DATA_getLast
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_EP_DATA_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_TRIGGER_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_ACTION_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static 
FTM_RET	FTOM_SERVER_RULE_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
);

static FTOM_SERVER_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTOM_CMD_NODE_CREATE,			FTOM_SERVER_NODE_create),
	MK_CMD_SET(FTOM_CMD_NODE_DESTROY,			FTOM_SERVER_NODE_destroy),
	MK_CMD_SET(FTOM_CMD_NODE_COUNT,				FTOM_SERVER_NODE_count),
	MK_CMD_SET(FTOM_CMD_NODE_GET,				FTOM_SERVER_NODE_get),
	MK_CMD_SET(FTOM_CMD_NODE_GET_AT,			FTOM_SERVER_NODE_getAt),
	MK_CMD_SET(FTOM_CMD_NODE_SET,				FTOM_SERVER_NODE_set),

	MK_CMD_SET(FTOM_CMD_EP_CREATE,				FTOM_SERVER_EP_create),
	MK_CMD_SET(FTOM_CMD_EP_DESTROY,				FTOM_SERVER_EP_destroy),
	MK_CMD_SET(FTOM_CMD_EP_COUNT,				FTOM_SERVER_EP_count),
	MK_CMD_SET(FTOM_CMD_EP_GET_LIST,			FTOM_SERVER_EP_getList),
	MK_CMD_SET(FTOM_CMD_EP_GET,					FTOM_SERVER_EP_get),
	MK_CMD_SET(FTOM_CMD_EP_GET_AT,				FTOM_SERVER_EP_getAt),
	MK_CMD_SET(FTOM_CMD_EP_SET,					FTOM_SERVER_EP_set),

	MK_CMD_SET(FTOM_CMD_EP_REG_NOTIFY_RECEIVER, FTOM_SERVER_EP_registrationNotifyReceiver),
	MK_CMD_SET(FTOM_CMD_EP_DATA_INFO,			FTOM_SERVER_EP_DATA_info),
	MK_CMD_SET(FTOM_CMD_EP_DATA_GET_LAST,		FTOM_SERVER_EP_DATA_getLast),
	MK_CMD_SET(FTOM_CMD_EP_DATA_GET_LIST,		FTOM_SERVER_EP_DATA_getList),
	MK_CMD_SET(FTOM_CMD_EP_DATA_COUNT,			FTOM_SERVER_EP_DATA_count),
	MK_CMD_SET(FTOM_CMD_EP_DATA_TYPE,			FTOM_SERVER_EP_DATA_type),

	MK_CMD_SET(FTOM_CMD_TRIG_ADD,				FTOM_SERVER_TRIGGER_add),
	MK_CMD_SET(FTOM_CMD_TRIG_DEL,				FTOM_SERVER_TRIGGER_del),
	MK_CMD_SET(FTOM_CMD_TRIG_COUNT,				FTOM_SERVER_TRIGGER_count),
	MK_CMD_SET(FTOM_CMD_TRIG_GET,				FTOM_SERVER_TRIGGER_get),
	MK_CMD_SET(FTOM_CMD_TRIG_GET_AT,			FTOM_SERVER_TRIGGER_getAt),
	MK_CMD_SET(FTOM_CMD_TRIG_SET,				FTOM_SERVER_TRIGGER_set),
	
	MK_CMD_SET(FTOM_CMD_ACTION_ADD,				FTOM_SERVER_ACTION_add),
	MK_CMD_SET(FTOM_CMD_ACTION_DEL,				FTOM_SERVER_ACTION_del),
	MK_CMD_SET(FTOM_CMD_ACTION_COUNT,			FTOM_SERVER_ACTION_count),
	MK_CMD_SET(FTOM_CMD_ACTION_GET,				FTOM_SERVER_ACTION_get),
	MK_CMD_SET(FTOM_CMD_ACTION_GET_AT,			FTOM_SERVER_ACTION_getAt),
	MK_CMD_SET(FTOM_CMD_ACTION_SET,				FTOM_SERVER_ACTION_set),
	
	MK_CMD_SET(FTOM_CMD_RULE_ADD,				FTOM_SERVER_RULE_add),
	MK_CMD_SET(FTOM_CMD_RULE_DEL,				FTOM_SERVER_RULE_del),
	MK_CMD_SET(FTOM_CMD_RULE_COUNT,				FTOM_SERVER_RULE_count),
	MK_CMD_SET(FTOM_CMD_RULE_GET,				FTOM_SERVER_RULE_get),
	MK_CMD_SET(FTOM_CMD_RULE_GET_AT,			FTOM_SERVER_RULE_getAt),
	MK_CMD_SET(FTOM_CMD_RULE_SET,				FTOM_SERVER_RULE_set),
	
	MK_CMD_SET(FTOM_CMD_UNKNOWN, 		NULL)
};

FTM_RET	FTOM_SERVER_init
(
	FTOM_SERVER_PTR	pServer,
	FTOM_PTR	pOM
)
{
	ASSERT(pServer != NULL);

	memset(pServer, 0, sizeof(FTOM_SERVER));

	memset(&pServer->xConfig, 0, sizeof(FTOM_SERVER_CONFIG));

	pServer->xConfig.usPort			= FTOM_DEFAULT_SERVER_PORT;
	pServer->xConfig.ulMaxSession	= FTOM_DEFAULT_SERVER_SESSION_COUNT	;

	pServer->pOM = pOM;
	pServer->bStop = FTM_TRUE;
	FTM_LIST_init(&pServer->xSessionList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_final
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTOM_SERVER_stop(pServer);

	FTM_LIST_final(&pServer->xSessionList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_start
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTM_INT	nRet;

	if (!pServer->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	nRet = pthread_create(&pServer->xPThread, NULL, FTOM_SERVER_process, (FTM_VOID_PTR)pServer);
	if (nRet != 0)
	{
		ERROR("Can't create thread[%d]\n", nRet);
		return	FTM_RET_CANT_CREATE_THREAD;
	}
	TRACE("xThread = %08x\n", pServer->xPThread);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_stop
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	FTOM_SESSION_PTR	pSession = NULL;

	if (pServer->bStop)
	{
		return	FTM_RET_NOT_START;
	}

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pthread_cancel(pSession->xPThread);
		pthread_join(pSession->xPThread, NULL);

		FTM_MEM_free(pSession);		
	}

	pServer->bStop = FTM_TRUE;
	shutdown(pServer->hSocket, SHUT_RD);
	pthread_join(pServer->xPThread, NULL);

	TRACE("Server finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_notify
(
	FTOM_SERVER_PTR	pServer,
	FTOM_MSG_PTR 		pMsg
)
{
	ASSERT(pServer != NULL);
	ASSERT(pMsg != NULL);

	FTOM_SESSION_PTR	pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		FTOM_RESP_NOTIFY_PARAMS	xNotify;

		xNotify.ulReqID = 0;
		xNotify.xCmd	= 1;
		xNotify.ulLen	= sizeof(FTOM_RESP_NOTIFY_PARAMS);
		xNotify.xRet	= FTM_RET_OK;
		memcpy(&xNotify.xMsg, pMsg, sizeof(FTOM_MSG));

		//TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, xNotify.ulReqID, xNotify.ulLen);
		send(pSession->hSocket, &xNotify, sizeof(xNotify), MSG_DONTWAIT);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SERVER_setServiceCallback(FTOM_SERVER_PTR pServer, FTOM_SERVICE_ID xServiceID, FTOM_SERVICE_CALLBACK fServiceCB)
{
	ASSERT(pServer != NULL);
	ASSERT(fServiceCB != NULL);

	pServer->xServiceID = xServiceID;
	pServer->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_SERVER_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	FTM_INT				nRet;
	pthread_t			xProcessPipe;
	pthread_t			xProcessSM;
	struct sockaddr_in	xServerAddr, xClientAddr;


	if (sem_init(&pServer->xLock, 0,pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR("Can't alloc semaphore!\n");
		return	0;	
	}

	pServer->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pServer->hSocket == -1)
	{
		ERROR("Could not create socket\n");
		return	0;
	}

	xServerAddr.sin_family 		= AF_INET;
	xServerAddr.sin_addr.s_addr = INADDR_ANY;
	xServerAddr.sin_port 		= htons( pServer->xConfig.usPort );

	nRet = bind( pServer->hSocket, (struct sockaddr *)&xServerAddr, sizeof(xServerAddr));
	if (nRet < 0)
	{
		ERROR("bind failed.[%d]\n", nRet);
		return	0;
	}

	listen(pServer->hSocket, 3);

	pServer->bStop = FTM_FALSE;
	
	pthread_create(&xProcessPipe, NULL, FTOM_SERVER_processPipe, pServer);
	pthread_create(&xProcessSM, NULL, FTOM_SERVER_processSM, pServer);

	while(!pServer->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nValue;
		FTM_INT	nSockAddrIulLen = sizeof(struct sockaddr_in);	
		struct timespec			xTimeout ;

		clock_gettime(CLOCK_REALTIME, &xTimeout);

		xTimeout.tv_sec += 2;
		if (sem_timedwait(&pServer->xLock, &xTimeout) == 0)
		{
			sem_getvalue(&pServer->xLock, &nValue);
			MESSAGE("Waiting for connections ...[%d]\n", nValue);
			hClient = accept(pServer->hSocket, (struct sockaddr *)&xClientAddr, (socklen_t *)&nSockAddrIulLen);
			if (hClient > 0)
			{
				TRACE("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClientAddr.sin_addr), ntohs(xClientAddr.sin_port));

				FTOM_SESSION_PTR pSession = (FTOM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTOM_SESSION));
				if (pSession == NULL)
				{
					ERROR("System memory is not enough!\n");
					TRACE("The session(%08x) was closed.\n", hClient);
					close(hClient);
				}
				else
				{
					TRACE("The new session(%08x) has beed connected\n", hClient);

					pSession->hSocket = hClient;
					memcpy(&pSession->xPeer, &xClientAddr, sizeof(xClientAddr));
					pSession->pServer = pServer;
					if (pthread_create(&pSession->xPThread, NULL, FTOM_SERVER_serviceHandler, pSession) == 0)
					{
						FTM_LIST_append(&pServer->xSessionList, pSession);	
					}
					else
					{
						FTM_MEM_free(pSession);
					}
				}
			}
		}
	}

	pthread_join(xProcessPipe, NULL);
	pthread_join(xProcessSM, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_SERVER_serviceHandler(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTOM_SESSION_PTR		pSession= (FTOM_SESSION_PTR)pData;
	FTOM_REQ_PARAMS_PTR		pReq 	= (FTOM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTOM_RESP_PARAMS_PTR	pResp 	= (FTOM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timeval			xTimeval;

	xTimeval.tv_sec = 0;
	xTimeval.tv_usec = 100000;

	pSession->bStop = FTM_FALSE;

	if (setsockopt(pSession->hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&xTimeval, sizeof(xTimeval)) != 0)
	{
		pSession->bStop = FTM_TRUE;
		ERROR("Timeout set failed.\n");
   		return 0;
	}

	while(!pSession->bStop)
	{
		FTM_INT	ulReqLen;
		FTM_INT	ulSendLen;

		ulReqLen = recv(pSession->hSocket, pReq, sizeof(pSession->pReqBuff), 0);
		if (ulReqLen == 0)
		{
			TRACE("The connection is terminated.\n");
			pSession->bStop = FTM_TRUE;
		}
		else if (ulReqLen == -1)
		{
			WARN("Error packet received.\n");	
		}
		else if (ulReqLen > 0)
		{
			FTM_ULONG	ulRetry = 3;
#if	FTOM_TRACE_IO
			//TRACE("RECV[%08lx:%08x] : Len = %lu\n", pSession->hSocket, pReq->ulReqID, ulLen);
#endif
			pResp->ulReqID = pReq->ulReqID;

			if (FTM_RET_OK != FTOM_SERVER_serviceCall(pSession->pServer, pReq, ulReqLen, pResp, FTOM_DEFAULT_PACKET_SIZE))
			{
				pResp->xCmd = pReq->xCmd;
				pResp->xRet = FTM_RET_INTERNAL_ERROR;
				pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
			}

#if	FTOM_TRACE_IO
			//TRACE("send(%08x, %08x, %d, MSG_DONTWAIT)\n", pSession->hSocket, pResp->ulReqID, pResp->ulLen);
#endif
			do
			{
				ulSendLen = send(pSession->hSocket, pResp, pResp->ulLen, MSG_DONTWAIT);
			}
			while ((--ulRetry > 0) && (ulSendLen < 0));

			if (ulSendLen < 0)
			{
				ERROR("send failed[%d]\n", -ulSendLen);	
				pSession->bStop = FTM_TRUE;
			}
		}
		else if (ulReqLen < 0)
		{
			ERROR("recv failed.[%d]\n", -ulReqLen);
		}
	}

	close(pSession->hSocket);
	TRACE("The session(%08x) was closed\n", pSession->hSocket);

	FTM_LIST_remove(&pSession->pServer->xSessionList, pSession);	
	sem_post(&pSession->pServer->xLock);
	FTM_MEM_free(pSession);
	return	0;
}

FTM_VOID_PTR	FTOM_SERVER_processPipe
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_RET				xRet;
	FTM_INT				nReadFD, nWriteFD;
	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	fd_set				xFDSet;
	struct timeval		xTimeout;
	FTOM_REQ_PARAMS_PTR		pReq;
	FTOM_RESP_PARAMS_PTR	pResp;

	pReq	= (FTOM_REQ_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pReq == NULL)
	{
		ERROR("Not enough memory!\n");
		return	0;	
	}

	pResp 	= (FTOM_RESP_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pResp == NULL)
	{
		ERROR("Not enough memory!\n");
		FTM_MEM_free(pReq);
		return	0;	
	}

	mkfifo("/tmp/ftom_in", 0666);
	mkfifo("/tmp/ftom_out", 0666);

	nReadFD = open("/tmp/ftom_in", O_RDWR);
	nWriteFD= open("/tmp/ftom_out", O_RDWR);

	FD_ZERO(&xFDSet); 			/* clear the set */

	while(!pServer->bStop)
	{
		FTM_INT	nRet;
		FTM_INT	nReqLen;

    	xTimeout.tv_sec = 1;
		xTimeout.tv_usec = 00000;
		
  		FD_SET(nReadFD, &xFDSet); 	/* add our file descriptor to the set */
		nRet = select(nReadFD + 1, &xFDSet, NULL, NULL, &xTimeout);
		if (nRet > 0)
		{
			nReqLen = read(nReadFD, pReq, FTOM_DEFAULT_PACKET_SIZE);	
			if (nReqLen > 0)
			{

				xRet = FTOM_SERVER_serviceCall(pServer, pReq, nReqLen, pResp, FTOM_DEFAULT_PACKET_SIZE);
				if (xRet != FTM_RET_OK)
				{
					ERROR("PIPE Service call error[%08x:%08x]\n", pReq->xCmd, xRet);
					pResp->xCmd = pReq->xCmd;
					pResp->xRet = xRet;
					pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
				}

				write(nWriteFD, pResp, pResp->ulLen);
				//usleep(1);
#if 0
				FTM_INT	i;

				MESSAGE("WRITE : ");
				for(i = 0 ; i < pResp->ulLen ; i++)
				{
					MESSAGE("%02x ", ((FTM_UINT8_PTR)pResp)[i]);	
				}
				MESSAGE("\n");
#endif
			}
		}
	}

	close(nReadFD);
	close(nWriteFD);

	FTM_MEM_free(pReq);
	FTM_MEM_free(pResp);

	return	0;
}

FTM_VOID_PTR	FTOM_SERVER_processSM
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_RET				xRet;
	FTM_SMP_PTR			pSMP;
	FTOM_SERVER_PTR 	pServer = (FTOM_SERVER_PTR)pData;
	FTOM_REQ_PARAMS_PTR		pReq;
	FTOM_RESP_PARAMS_PTR	pResp;

	pReq	= (FTOM_REQ_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pReq == NULL)
	{
		ERROR("Not enough memory!\n");
		return	0;	
	}

	pResp 	= (FTOM_RESP_PARAMS_PTR)FTM_MEM_malloc(FTOM_DEFAULT_PACKET_SIZE);
	if (pResp == NULL)
	{
		ERROR("Not enough memory!\n");
		FTM_MEM_free(pReq);
		return	0;	
	}

	xRet = FTM_SMP_createServer(1234, &pSMP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("SMP creation failed.[%08x]\n", xRet);
		return	0;	
	}

	while(!pServer->bStop)
	{
		FTM_ULONG	ulReqLen = 0;

		xRet = FTM_SMP_receiveReq(pSMP, pReq, FTOM_DEFAULT_PACKET_SIZE, &ulReqLen, 1000000);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_SERVER_serviceCall(pServer, pReq, ulReqLen, pResp, FTOM_DEFAULT_PACKET_SIZE);
			if (xRet != FTM_RET_OK)
			{
				ERROR("PIPE Service call error[%08x:%08x]\n", pReq->xCmd, xRet);
				pResp->xCmd = pReq->xCmd;
				pResp->xRet = xRet;
				pResp->ulLen = sizeof(FTOM_RESP_PARAMS);
			}

			xRet = FTM_SMP_sendResp(pSMP, pResp, pResp->ulLen, 100000);
		}
	}

	FTM_SMP_destroy(&pSMP);

	FTM_MEM_free(pReq);
	FTM_MEM_free(pResp);

	return	0;
}

FTM_RET	FTOM_SERVER_serviceCall
(
	FTOM_SERVER_PTR			pServer,
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pResp,
	FTM_ULONG				ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_SERVER_CMD_SET_PTR	pSet = pCmdSet;

	while(pSet->xCmd != FTOM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
#if	FTOM_TRACE_IO
			FTM_RET	xRet;

			TRACE("CMD : %s\n", pSet->pCmdString);
			xRet = 
#endif
			pSet->fService(pServer, pReq, ulReqLen, pResp, ulRespLen);
#if	FTOM_TRACE_IO
			TRACE("RET : %08lx\n", xRet);
#endif
			return	FTM_RET_OK;
		}

		pSet++;
	}

	ERROR("FUNCTION NOT SUPPORTED\n");
	ERROR("CMD : %08lx\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_RET	FTOM_SERVER_createNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_NODE_PTR		pInfo,
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);

	return	FTOM_createNode(pServer->pOM, pInfo, ppNode);
}

FTM_RET	FTOM_SERVER_destroyNode
(
	FTOM_SERVER_PTR		pServer,
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);
	
	return	FTOM_destroyNode(pServer->pOM, ppNode);
}

FTM_RET	FTOM_SERVER_countNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pServer != NULL);
	ASSERT(pulCount != NULL);
	
	return	FTOM_countNode(pServer->pOM, pulCount);
}

FTM_RET	FTOM_SERVER_getNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_CHAR_PTR		pDID,
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);
	
	return	FTOM_getNode(pServer->pOM, pDID, ppNode);
}

FTM_RET	FTOM_SERVER_getNodeAt
(
	FTOM_SERVER_PTR		pServer,
	FTM_ULONG			ulIndex,
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppNode != NULL);
	
	return	FTOM_getNodeAt(pServer->pOM, ulIndex, ppNode);
}

FTM_RET	FTOM_SERVER_setNode
(
	FTOM_SERVER_PTR		pServer,
	FTM_CHAR_PTR		pDID,
	FTM_NODE_FIELD		xFields,
	FTM_NODE_PTR 		pInfo
)
{
	ASSERT(pServer != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);
	
	return	FTOM_setNode(pServer->pOM, pDID, xFields, pInfo);
}

/****************************************************************************************************
 *
 ****************************************************************************************************/
FTM_RET	FTOM_SERVER_NODE_create
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_NODE_CREATE_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_CREATE_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_createNode(pServer->pOM, &pReq->xNodeInfo, &pNode);

	return	pResp->xRet;
}


FTM_RET	FTOM_SERVER_NODE_destroy
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_DESTROY_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_DESTROY_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_getNode(pServer->pOM, pReq->pDID, &pNode);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->xRet = FTOM_NODE_destroy(&pNode);
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_countNode(pServer->pOM, &pResp->ulCount);

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;
 
	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_getNode(pServer->pOM, pReq->pDID, &pNode);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_getAt
(	
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_GET_AT_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_NODE_PTR	pNode;

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_getNodeAt(pServer->pOM, pReq->ulIndex, &pNode);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xNodeInfo, &pNode->xInfo, sizeof(FTM_NODE));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_NODE_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_NODE_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_NODE_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd	= pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	pResp->xRet = FTOM_setNode(pServer->pOM, pReq->pDID, pReq->xFields, &pReq->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_create
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_CREATE_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_CREATE_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EPM_getEP(pServer->pOM->pEPM, pReq->xInfo.pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_RET_ALREADY_EXISTS;
	}
	else
	{
		xRet = FTOM_EPM_createEP(pServer->pOM->pEPM, &pReq->xInfo, &pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_createEP(pServer->pOM, &pReq->xInfo);
		}
	}

	pResp->xCmd 	= pReq->xCmd;
	pResp->ulLen 	= sizeof(*pResp);
	pResp->xRet 	= xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_destroy
(		
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DESTROY_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DESTROY_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EPM_getEP(pServer->pOM->pEPM, pReq->pEPID, &pEP);
	
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_EPM_destroyEP(pServer->pOM->pEPM, pEP);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_destroyEP(pServer->pOM, pReq->pEPID);
		}
	}

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EPM_count(pServer->pOM->pEPM, pReq->xType, &pResp->nCount);

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EPM_getEP(pServer->pOM->pEPM, pReq->pEPID, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_getList
(	
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->xRet = FTOM_EPM_getIDList(pServer->pOM->pEPM, pReq->xType, pResp->pEPIDList, pReq->ulMaxCount, &pResp->ulCount);
	pResp->ulLen = sizeof(*pResp) + (FTM_EPID_LEN + 1) * pResp->ulCount;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EPM_getEPAt(pServer->pOM->pEPM, pReq->ulIndex, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		memcpy(&pResp->xInfo, &pEP->xInfo, sizeof(FTM_EP));
	}
	
	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_SET_PARAMS_PTR		pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_SET_PARAMS_PTR		pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);


	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_setEPInfo(pServer->pOM, 
					pReq->pEPID, 
					pReq->xFields, 
					&pReq->xInfo);

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_registrationNotifyReceiver
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
 	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	FTM_RET_OK;
}



FTM_RET	FTOM_SERVER_EP_DATA_info
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_INFO_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_getEPDataInfo(pServer->pOM, pReq->pEPID, &pResp->ulBeginTime, &pResp->ulEndTime, &pResp->ulCount);

	return	pResp->xRet;
}
FTM_RET	FTOM_SERVER_EP_DATA_getLast
(	
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer!= NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EPM_getEP(pServer->pOM->pEPM, pReq->pEPID, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		FTM_EP_DATA_PTR	pData;

		xRet = FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
		if (xRet != FTM_RET_OK)
		{	
			return	xRet;
		}
		memcpy(&pResp->xData, pData, sizeof(FTM_EP_DATA));
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_getList
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_RET		xRet;

	xRet = FTOM_getEPDataList(pServer->pOM, pReq->pEPID, pReq->nStartIndex, pResp->pData, pReq->nCount, &pResp->nCount);
	if (xRet != FTM_RET_OK)
	{
		TRACE("EP[%s] get data list error[%08x]!\n", pReq->pEPID, xRet);
		pResp->nCount = 0;
	}

	pResp->ulLen = sizeof(*pResp) + sizeof(FTM_EP_DATA) * pResp->nCount;
	pResp->xCmd = pReq->xCmd;
	pResp->xRet = xRet;

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_count
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_ULONG	ulCount = 0;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_getEPDataCount(pServer->pOM, pReq->pEPID, &ulCount);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->ulCount = ulCount;
	}

	return	pResp->xRet;
}

FTM_RET	FTOM_SERVER_EP_DATA_type
(
	FTOM_SERVER_PTR	pServer,
	FTOM_REQ_EP_DATA_TYPE_PARAMS_PTR 	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_EP_DATA_TYPE_PARAMS_PTR 	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTOM_EP_PTR		pEP;

	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	pResp->xRet = FTOM_EPM_getEP(pServer->pOM->pEPM, pReq->pEPID, &pEP);
	if (pResp->xRet == FTM_RET_OK)
	{
		pResp->xRet = FTOM_EP_getDataType(pEP, &pResp->xType);
	}

	return	pResp->xRet;
}

/********************************************************************
 *
 ********************************************************************/
FTM_RET	FTOM_SERVER_TRIGGER_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);
	
	pResp->xRet = FTOM_addTrigger(pServer->pOM, &pReq->xTrigger, pResp->pTriggerID);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_delTrigger(pServer->pOM, pReq->pTriggerID);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getTriggerCount(pServer->pOM, &pResp->ulCount);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getTriggerInfo(pServer->pOM, pReq->pTriggerID, &pResp->xTrigger);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getTriggerInfoAt(pServer->pOM, pReq->ulIndex, &pResp->xTrigger);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_TRIGGER_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_TRIGGER_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_TRIGGER_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_setTriggerInfo(pServer->pOM, 
					pReq->pTriggerID, 
					pReq->xFields, 
					&pReq->xTrigger);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_CHAR	pActionID[FTM_ID_LEN+1];
	
	pResp->xRet = FTOM_addAction(pServer->pOM, &pReq->xAction, pActionID);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);
	
	if (pResp->xRet == FTM_RET_OK)
	{
		strcpy(pResp->pActionID, pActionID);
	}

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_delAction(pServer->pOM, pReq->pActionID);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getActionCount(pServer->pOM, &pResp->ulCount);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getActionInfo(pServer->pOM, pReq->pActionID, &pResp->xAction);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getActionInfoAt(pServer->pOM, pReq->ulIndex, &pResp->xAction);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_ACTION_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_ACTION_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_ACTION_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getActionInfo(pServer->pOM, pReq->pActionID, &pReq->xAction);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_add
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_ADD_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_ADD_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	FTM_CHAR	pRuleID[FTM_ID_LEN+1];

	pResp->xRet = FTOM_addRule(pServer->pOM, &pReq->xRule, pRuleID);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	if (pResp->xRet == FTM_RET_OK)
	{
		strcpy(pResp->pRuleID, pRuleID);	
	}

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_del
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_DEL_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_DEL_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_delRule(pServer->pOM, pReq->pRuleID);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_count
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_COUNT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_COUNT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getRuleCount(pServer->pOM, &pResp->ulCount);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_get
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getRuleInfo(pServer->pOM, pReq->pRuleID, &pResp->xRule);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_getAt
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_GET_AT_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_GET_AT_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getRuleInfoAt(pServer->pOM, pReq->ulIndex, &pResp->xRule);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

static 
FTM_RET	FTOM_SERVER_RULE_set
(
	FTOM_SERVER_PTR	pServer,
 	FTOM_REQ_RULE_SET_PARAMS_PTR	pReq,
	FTM_ULONG		ulReqLen,
	FTOM_RESP_RULE_SET_PARAMS_PTR	pResp,
	FTM_ULONG		ulRespLen
)
{
	ASSERT(pServer != NULL);
	ASSERT(pReq != NULL);
	ASSERT(pResp != NULL);

	pResp->xRet = FTOM_getRuleInfo(pServer->pOM, pReq->pRuleID, &pReq->xRule);
	pResp->xCmd = pReq->xCmd;
	pResp->ulLen = sizeof(*pResp);

	return	pResp->xRet;
}

FTM_RET FTOM_SERVER_loadFromFile
(
	FTOM_SERVER_PTR	pServer,
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pServer != NULL);
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
			pServer->xConfig.ulMaxSession = (FTM_ULONG)config_setting_get_int(pField);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pServer->xConfig.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SERVER_showConfig
(
	FTOM_SERVER_PTR	pServer
)
{
	ASSERT(pServer != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "PORT", pServer->xConfig.usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pServer->xConfig.ulMaxSession);

	return	FTM_RET_OK;
}
