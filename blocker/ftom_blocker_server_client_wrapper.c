#include "ftom_blocker.h"
#include "ftom_server_client.h"
#include "ftom_blocker_server_client_wrapper.h"

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_create
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fCreate == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fCreate(pBlocker->xConfig.xServerClient.pName, ppClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_destroy
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fDestroy == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fDestroy(ppClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_init
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fInit== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fInit(pClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_final
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fFinal== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fFinal(pClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_start
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fStart== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fStart(pClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_stop
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fStop== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fStop(pClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_isRunning
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fIsRunning == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fIsRunning(pClient, pIsRunning);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_waitingForFinished
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fWaitingForFinished == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fWaitingForFinished(pClient);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_load
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pClient != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fConfigLoad == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fConfigLoad(pClient, pConfig);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_save
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fConfigSave == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fConfigSave(pClient, pConfig);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_show
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fConfigShow == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fConfigShow(pClient);
}


FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_setNotifyCB
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fSetNotifyCB == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fSetNotifyCB(pClient, fNotifyCB, pData);
}

FTM_RET FTOM_BLOCKER_SERVER_CLIENT_MESSAGE_send
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fMessageSend == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fMessageSend(pClient, pBaseMsg);
}

FTM_RET	FTOM_BLOCKER_SERVER_CLIENT_MESSAGE_process
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_SERVER_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pServerClientModule == NULL) || (pBlocker->pServerClientModule->fMessageProcess == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pServerClientModule->fMessageProcess(pClient, pMsg);
}

