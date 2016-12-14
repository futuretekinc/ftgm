#include "ftom_tp_blocker.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static
FTM_VOID_PTR	FTOM_TP_BLOCKER_threadMain
(
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTOM_TP_BLOCKER_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

FTM_RET	FTOM_TP_BLOCKER_create
(
	FTOM_TP_BLOCKER_PTR _PTR_ ppBlocker
)
{
	ASSERT(ppBlocker != NULL);
	FTM_RET	xRet;	
	FTOM_TP_BLOCKER_PTR	pBlocker = NULL;

	pBlocker = (FTOM_TP_BLOCKER_PTR)FTM_MEM_malloc(sizeof(FTOM_TP_BLOCKER));
	if (pBlocker == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Failed to create tp blocker!\n");
		return	xRet;	
	}

	pBlocker->bStop = FTM_TRUE;

	xRet = FTOM_NET_CLIENT_create(&pBlocker->pNetClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create tp client!\n");
		goto error;
	}
	
	xRet = FTOM_TP_CLIENT_create(&pBlocker->pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create tp client!\n");
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

		if (pBlocker->pTPClient != NULL)
		{
			FTOM_TP_CLIENT_destroy(&pBlocker->pTPClient);	
		}

		FTM_MEM_free(pBlocker);
	}

	return	xRet;
}

FTM_RET	FTOM_TP_BLOCKER_destroy
(
	FTOM_TP_BLOCKER_PTR _PTR_ ppBlocker
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

	if ((*ppBlocker)->pTPClient != NULL)
	{
		xRet = FTOM_TP_CLIENT_destroy(&(*ppBlocker)->pTPClient);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy tp client!\n");	
		}
	}

	FTM_MEM_free(*ppBlocker);

	*ppBlocker = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TP_BLOCKER_start
(
	FTOM_TP_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBlocker->pNetClient != NULL);
	ASSERT(pBlocker->pTPClient != NULL);
	FTM_RET	xRet;

	TRACE_ENTRY();

    if (pthread_create(&pBlocker->xThreadMain, NULL, FTOM_TP_BLOCKER_threadMain, pBlocker) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The tp blocker thread creation failed!\n");
		return  xRet;
    }

	TRACE_EXIT();

	return	FTM_RET_OK;	
}

FTM_RET	FTOM_TP_BLOCKER_stop
(
	FTOM_TP_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBlocker->pNetClient != NULL);
	ASSERT(pBlocker->pTPClient != NULL);

	pBlocker->bStop = FTM_TRUE;
	
	pthread_join(pBlocker->xThreadMain, NULL);
	pBlocker->xThreadMain = 0;
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_TP_BLOCKER_waitingForFinished
(	
	FTOM_TP_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	FTM_RET	xRet;

	xRet = FTOM_NET_CLIENT_waitingForFinished(pBlocker->pNetClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Exit wait was processed abnormally.\n");
	}

	xRet = FTOM_TP_CLIENT_waitingForFinished(pBlocker->pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Exit wait was processed abnormally.\n");
	}

	pthread_join(pBlocker->xThreadMain, NULL);
	pBlocker->xThreadMain = 0;

	return	xRet;
}

FTM_RET	FTOM_TP_BLOCKER_CONFIG_load
(
	FTOM_TP_BLOCKER_PTR	pBlocker,
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

	xRet = FTOM_TP_CLIENT_CONFIG_load(pBlocker->pTPClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load tp client configuration.\n");
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTOM_TP_BLOCKER_CONFIG_save
(
	FTOM_TP_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;

	xRet = FTOM_TP_CLIENT_CONFIG_save(pBlocker->pTPClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load ftom client configuration.\n");
		return	xRet;
	}

	xRet = FTOM_TP_CLIENT_CONFIG_save(pBlocker->pTPClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load tp client configuration.\n");
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTOM_TP_BLOCKER_CONFIG_show
(
	FTOM_TP_BLOCKER_PTR	pBlocker
)
{
	ASSERT(pBlocker != NULL);

	FTOM_NET_CLIENT_CONFIG_show(pBlocker->pNetClient);
	FTOM_TP_CLIENT_CONFIG_show(pBlocker->pTPClient);

	return	FTM_RET_OK;
}


FTM_VOID_PTR	FTOM_TP_BLOCKER_threadMain
(
	FTM_VOID_PTR	pData
)
{
	FTOM_TP_BLOCKER_PTR	pBlocker = (FTOM_TP_BLOCKER_PTR)pData;
	FTM_RET     xRet;
	FTM_TIMER   xLoopTimer;
	FTM_ULONG   ulLoopInterval;

	FTM_TIMER_initMS(&xLoopTimer, 1000);

	TRACE("The tp blocker was started!\n");

	FTOM_NET_CLIENT_setNotifyCB(pBlocker->pNetClient, FTOM_TP_BLOCKER_notifyCB, pBlocker);

	xRet = FTOM_NET_CLIENT_start(pBlocker->pNetClient);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start ftom client!\n");
	}

	FTOM_TP_CLIENT_setNotifyCB(pBlocker->pTPClient, FTOM_TP_BLOCKER_notifyCB, pBlocker);

	xRet = FTOM_TP_CLIENT_start(pBlocker->pTPClient);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start tp client!\n");
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

			xRet = FTOM_TP_BLOCKER_MESSAGE_process(pBlocker, pBaseMsg);
			if (xRet != FTM_RET_OK)
			{
				FTOM_MSG_destroy(&pBaseMsg);
			}
		}    

		FTM_TIMER_addMS(&xLoopTimer, 100);
	}    

	xRet = FTOM_TP_CLIENT_stop(pBlocker->pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop tp client!\n");	
	}

	FTOM_TP_CLIENT_setNotifyCB(pBlocker->pTPClient, NULL, NULL);

	xRet = FTOM_NET_CLIENT_stop(pBlocker->pNetClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop ftom client!\n");	
	}

	FTOM_NET_CLIENT_setNotifyCB(pBlocker->pNetClient, NULL, NULL);

	TRACE("The tp blocker was finished !\n");

	return  0;
}

FTM_RET	FTOM_TP_BLOCKER_MESSAGE_process
(
	FTOM_TP_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBaseMsg != NULL);

	return	FTOM_TP_CLIENT_MESSAGE_send(pBlocker->pTPClient, pBaseMsg);
}

FTM_RET	FTOM_TP_BLOCKER_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pMsg != NULL);
	ASSERT(pData != NULL);

	FTOM_TP_BLOCKER_PTR	pBlocker = (FTOM_TP_BLOCKER_PTR)pData;

	return	FTOM_MSGQ_push(pBlocker->pMsgQ, pMsg);

}
