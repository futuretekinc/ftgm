#include "ftom_blocker.h"
#include "ftom_blocker_cloud_client_wrapper.h"

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_create
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fCreate == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fCreate(pBlocker->xConfig.xCloudClient.pName, &pBlocker->pCloudClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_destroy
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fDestroy == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fDestroy(&pBlocker->pCloudClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_init
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fInit== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fInit(pBlocker->pCloudClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_final
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fFinal== NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fFinal(pBlocker->pCloudClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_load
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConfigLoad == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConfigLoad(pBlocker->pCloudClient, pConfig);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_save
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConfigSave == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConfigSave(pBlocker->pCloudClient, pConfig);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_show
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConfigShow == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConfigShow(pBlocker->pCloudClient);
}


FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_setNotifyCB
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR			pData
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fSetNotifyCB == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fSetNotifyCB(pBlocker->pCloudClient, fNotifyCB, pData);
}

FTM_RET FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_send
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fMessageSend == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fMessageSend(pBlocker->pCloudClient, pBaseMsg);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_process
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTOM_MSG_PTR			pMsg
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fMessageProcess == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fMessageProcess(pBlocker->pCloudClient, pMsg);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_connect
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fConnect == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fConnect(pBlocker->pCloudClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_disconnect
(
	FTOM_BLOCKER_PTR		pBlocker
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fDisconnect == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fDisconnect(pBlocker->pCloudClient);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_isConnected
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_BOOL_PTR			pConnected
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->fIsConnected == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->fIsConnected(pBlocker->pCloudClient, pConnected);
}

/**********************************************************
 *	Gateway management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_GATEWAY_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_BOOL				bStatus
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xGateway.fUpdateStatus == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xGateway.fUpdateStatus(pBlocker->pCloudClient, bStatus);
}

/**********************************************************
 *	Node management
 **********************************************************/
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xNode.fGetCount == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xNode.fGetCount(pBlocker->pCloudClient, pulCount);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_NODE_PTR			pNode
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xNode.fRegister == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xNode.fRegister(pBlocker->pCloudClient, pNode);
}
	
FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_NODE_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CHAR_PTR			pID
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xNode.fIsExist == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xNode.fIsExist(pBlocker->pCloudClient, pID);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_getCount
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fGetCount == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fGetCount(pBlocker->pCloudClient, pulCount);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_getEPIDAt
(
	FTOM_BLOCKER_PTR		pBlocker,
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

	return	pBlocker->pCloudClientModule->xEP.fGetEPIDAt(pBlocker->pCloudClient, ulIndex, pBuff, ulBuffLen);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_register
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_EP_PTR			pEP
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fRegister == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fRegister(pBlocker->pCloudClient, pEP);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_isExist
(
	FTOM_BLOCKER_PTR		pBlocker,
	FTM_CHAR_PTR			pEPID
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.fIsExist == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.fIsExist(pBlocker->pCloudClient, pEPID);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_updateStatus
(
	FTOM_BLOCKER_PTR		pBlocker, 
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

	return	pBlocker->pCloudClientModule->xEP.fUpdateStatus(pBlocker->pCloudClient, pEPID, bStatus);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_getLastTime
(
	FTOM_BLOCKER_PTR		pBlocker, 
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG_PTR			pulLastTime
)
{
	ASSERT(pBlocker != NULL);

	if ((pBlocker->pCloudClientModule == NULL) || (pBlocker->pCloudClientModule->xEP.xData.fGetLastTime == NULL))
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pBlocker->pCloudClientModule->xEP.xData.fGetLastTime(pBlocker->pCloudClient, pEPID, pulLastTime);
}

FTM_RET	FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_send
(
	FTOM_BLOCKER_PTR		pBlocker, 
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

	return	pBlocker->pCloudClientModule->xEP.xData.fSend(pBlocker->pCloudClient, pEPID, pDatas, ulCount);
}

