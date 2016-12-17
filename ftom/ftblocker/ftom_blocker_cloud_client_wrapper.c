#include "ftom_blocker.h"
#include "ftom_blocker_cloud_client_wrapper.h"

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_create
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fCreate == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fCreate(pBlocker->xConfig.xCloudClient.pName, ppClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_destroy
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fDestroy == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fDestroy(ppClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_init
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fInit== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fInit(pClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_final
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fFinal== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fFinal(pClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_start
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fStart== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fStart(pClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_stop
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fStop== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fStop(pClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_isRunning
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_BOOL_PTR			pIsRunning
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fIsRunning == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fIsRunning(pClient, pIsRunning);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_waitingForFinished
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fWaitingForFinished == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fWaitingForFinished(pClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_load
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConfigLoad == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConfigLoad(pClient, pConfig);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_save
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConfigSave == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConfigSave(pClient, pConfig);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_show
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConfigShow == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConfigShow(pClient);
}


FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_setNotifyCB
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fSetNotifyCB == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fSetNotifyCB(pClient, fNotifyCB, pData);
}

FTM_RET FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_send
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fMessageSend == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fMessageSend(pClient, pBaseMsg);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_process
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fMessageProcess == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fMessageProcess(pClient, pMsg);
}

/**********************************************************
 *	Gateway management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_GATEWAY_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_BOOL				bStatus
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xGateway.fUpdateStatus == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xGateway.fUpdateStatus(pClient, bStatus);
}

/**********************************************************
 *	Node management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xNode.fGetCount == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xNode.fGetCount(pClient, pulCount);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_NODE_PTR			pNode
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xNode.fRegister == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xNode.fRegister(pClient, pNode);
}
	
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pID
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xNode.fIsExist == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xNode.fIsExist(pClient, pID);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fGetCount == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fGetCount(pClient, pulCount);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_getEPIDAt
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pBuff,
	FTM_ULONG				ulBuffLen
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fGetEPIDAt == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fGetEPIDAt(pClient, ulIndex, pBuff, ulBuffLen);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_EP_PTR			pEP
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fRegister == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fRegister(pClient, pEP);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fIsExist == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fIsExist(pClient, pEPID);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_BOOL				bStatus
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pEPID != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fUpdateStatus == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fUpdateStatus(pClient, pEPID, bStatus);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_getLastTime
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG_PTR			pulLastTime
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.xData.fGetLastTime == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.xData.fGetLastTime(pClient, pEPID, pulLastTime);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_send
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTOM_CLOUD_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pDatas,
	FTM_ULONG				ulCount
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.xData.fSend == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.xData.fSend(pClient, pEPID, pDatas, ulCount);
}

