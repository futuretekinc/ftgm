#include "ftom_azure_blocker.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static
FTM_VOID_PTR	FTOM_AZURE_BLOCKER_threadMain
(
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTOM_AZURE_BLOCKER_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

FTM_RET	FTOM_AZURE_BLOCKER_create
(
	FTOM_AZURE_BLOCKER_PTR _PTR_ ppBlocker
)
{
	ASSERT(ppBlocker != NULL);
	FTM_RET	xRet;	
	FTOM_AZURE_BLOCKER_PTR	pBlocker = NULL;

	pBlocker = (FTOM_AZURE_BLOCKER_PTR)FTM_MEM_malloc(sizeof(FTOM_AZURE_BLOCKER));
	if (pBlocker == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Failed to create azure blocker!\n");
		return	xRet;	
	}

	pBlocker->bStop = FTM_TRUE;

	xRet = FTOM_NET_CLIENT_create(&pBlocker->pNetClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create azure client!\n");
		goto error;
	}
	
	xRet = FTOM_AZURE_CLIENT_create(&pBlocker->pAzureClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create azure client!\n");
		goto error;
	}

	xRet = FTOM_MSGQ_create(&pBlocker->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create message queue!\n");
		goto error;
	}

	*ppBlocker = pBlocker;

	return	FTM_RET_OK;

error:
	if (pBlocker != NULL)
	{
		if (pBlocker->pMsgQ != NULL)
		{
			FTOM_MSGQ_destroy(&pBlocker->pMsgQ);	
		}

		if (pBlocker->pNetClient != NULL)
		{
			FTOM_NET_CLIENT_destroy(&pBlocker->pNetClient);	
		}

		if (pBlocker->pAzureClient != NULL)
		{
			FTOM_AZURE_CLIENT_destroy(&pBlocker->pAzureClient);	
		}

		FTM_MEM_free(pBlocker);
	}

	return	xRet;
}

FTM_RET	FTOM_AZURE_BLOCKER_destroy
(
	FTOM_AZURE_BLOCKER_PTR _PTR_ ppBlocker
)
{
	ASSERT(ppBlocker != NULL);
	FTM_RET	xRet;

	if ((*ppBlocker)->pNetClient != NULL)
	{
		xRet = FTOM_NET_CLIENT_destroy(&(*ppBlocker)->pNetClient);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy ftom client!\n");	
		}
	}

	if ((*ppBlocker)->pAzureClient != NULL)
	{
		xRet = FTOM_AZURE_CLIENT_destroy(&(*ppBlocker)->pAzureClient);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy azure client!\n");	
		}
	}

	FTM_MEM_free(*ppBlocker);

	*ppBlocker = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_BLOCKER_start
(
	FTOM_AZURE_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBlocker->pNetClient != NULL);
	ASSERT(pBlocker->pAzureClient != NULL);
	FTM_RET	xRet;

	TRACE_ENTRY();

    if (pthread_create(&pBlocker->xThreadMain, NULL, FTOM_AZURE_BLOCKER_threadMain, pBlocker) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The azure blocker thread creation failed!\n");
		return  xRet;
    }

	TRACE_EXIT();

	return	FTM_RET_OK;	
}

FTM_RET	FTOM_AZURE_BLOCKER_stop
(
	FTOM_AZURE_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBlocker->pNetClient != NULL);
	ASSERT(pBlocker->pAzureClient != NULL);

	pBlocker->bStop = FTM_TRUE;
	
	pthread_join(pBlocker->xThreadMain, NULL);
	pBlocker->xThreadMain = 0;
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_AZURE_BLOCKER_waitingForFinished
(	
	FTOM_AZURE_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	FTM_RET	xRet;

	xRet = FTOM_NET_CLIENT_waitingForFinished(pBlocker->pNetClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Exit wait was processed abnormally.\n");
	}

	xRet = FTOM_AZURE_CLIENT_waitingForFinished(pBlocker->pAzureClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Exit wait was processed abnormally.\n");
	}

	pthread_join(pBlocker->xThreadMain, NULL);
	pBlocker->xThreadMain = 0;

	return	xRet;
}

FTM_RET	FTOM_AZURE_BLOCKER_CONFIG_load
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;

	xRet = FTOM_NET_CLIENT_CONFIG_load(pBlocker->pNetClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load ftom client configuration.\n");
		return	xRet;
	}

	xRet = FTOM_AZURE_CLIENT_CONFIG_load(pBlocker->pAzureClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load azure client configuration.\n");
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTOM_AZURE_BLOCKER_CONFIG_save
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;

	xRet = FTOM_AZURE_CLIENT_CONFIG_save(pBlocker->pAzureClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load ftom client configuration.\n");
		return	xRet;
	}

	xRet = FTOM_AZURE_CLIENT_CONFIG_save(pBlocker->pAzureClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load azure client configuration.\n");
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTOM_AZURE_BLOCKER_CONFIG_show
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker
)
{
	ASSERT(pBlocker != NULL);

	FTOM_NET_CLIENT_CONFIG_show(pBlocker->pNetClient);
	FTOM_AZURE_CLIENT_CONFIG_show(pBlocker->pAzureClient);

	return	FTM_RET_OK;
}


FTM_VOID_PTR	FTOM_AZURE_BLOCKER_threadMain
(
	FTM_VOID_PTR	pData
)
{
	FTOM_AZURE_BLOCKER_PTR	pBlocker = (FTOM_AZURE_BLOCKER_PTR)pData;
	FTM_RET     xRet;
	FTM_TIMER   xLoopTimer;
	FTM_ULONG   ulLoopInterval;

	FTM_TIMER_initMS(&xLoopTimer, 1000);

	TRACE("The azure blocker was started!\n");

	FTOM_NET_CLIENT_setNotifyCB(pBlocker->pNetClient, FTOM_AZURE_BLOCKER_notifyCB, pBlocker);

	xRet = FTOM_NET_CLIENT_start(pBlocker->pNetClient);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start ftom client!\n");
	}

	FTOM_AZURE_CLIENT_setNotifyCB(pBlocker->pAzureClient, FTOM_AZURE_BLOCKER_notifyCB, pBlocker);

	xRet = FTOM_AZURE_CLIENT_start(pBlocker->pAzureClient);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start azure client!\n");
	}

	pBlocker->bStop = FTM_FALSE;

	while(!pBlocker->bStop)
	{    
		FTOM_MSG_PTR    pBaseMsg;

		FTM_TIMER_remainMS(&xLoopTimer, &ulLoopInterval);

		while (!pBlocker->bStop)
		{    
			xRet = FTOM_MSGQ_timedPop(pBlocker->pMsgQ, ulLoopInterval, &pBaseMsg);
			if (xRet != FTM_RET_OK)
			{    
				break;  
			}    

			xRet = FTOM_AZURE_BLOCKER_MESSAGE_process(pBlocker, pBaseMsg);
			if (xRet != FTM_RET_OK)
			{
				FTOM_MSG_destroy(&pBaseMsg);
			}
		}    

		FTM_TIMER_addMS(&xLoopTimer, 100);
	}    

	xRet = FTOM_AZURE_CLIENT_stop(pBlocker->pAzureClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop azure client!\n");	
	}

	FTOM_AZURE_CLIENT_setNotifyCB(pBlocker->pAzureClient, NULL, NULL);

	xRet = FTOM_NET_CLIENT_stop(pBlocker->pNetClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop ftom client!\n");	
	}

	FTOM_NET_CLIENT_setNotifyCB(pBlocker->pNetClient, NULL, NULL);

	TRACE("The azure blocker was finished !\n");

	return  0;
}

FTM_RET	FTOM_AZURE_BLOCKER_MESSAGE_process
(
	FTOM_AZURE_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBaseMsg != NULL);

	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_EP_DATA:
		{
			return	FTOM_AZURE_CLIENT_MESSAGE_send(pBlocker->pAzureClient, pBaseMsg);
		}
		break;

	case	FTOM_MSG_TYPE_EP_STATUS:
		{
		}
		break;
		
	default:
		TRACE("Unknown message received[%s]\n", FTOM_MSG_printType(pBaseMsg->xType));
		return	FTM_RET_INVALID_MESSAGE_TYPE;
	}

	FTOM_MSG_destroy(&pBaseMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_BLOCKER_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pMsg != NULL);
	ASSERT(pData != NULL);

	FTOM_AZURE_BLOCKER_PTR	pBlocker = (FTOM_AZURE_BLOCKER_PTR)pData;

	return	FTOM_MSGQ_push(pBlocker->pMsgQ, pMsg);

}
