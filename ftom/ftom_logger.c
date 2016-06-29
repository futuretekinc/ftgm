#include "ftom.h"
#include "ftm_logger.h"
#include "ftom_logger.h"
#include "ftom_server.h"

typedef	struct	FTOM_LOGGER_STRUCT 
{
	FTM_LOGGER	xCommon;
}	FTOM_LOGGER, _PTR_ FTOM_LOGGER_PTR;

static
FTOM_LOGGER_PTR	pLogger = NULL;

FTM_RET	FTOM_LOGGER_init
(
	FTM_VOID
)
{
	FTM_RET	xRet;	

	if (pLogger != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pLogger = (FTOM_LOGGER_PTR)FTM_MEM_malloc(sizeof(FTOM_LOGGER));
	if (pLogger == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_LOGGER_init(&pLogger->xCommon);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pLogger);	
		pLogger = NULL;
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;	

	if (pLogger == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, NULL);
		return	FTM_RET_NOT_INITIALIZED;
	}

	xRet = FTM_LOGGER_final(&pLogger->xCommon);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pLogger);
		pLogger = NULL;
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_add
(
	FTM_LOG_PTR		pLog
)
{
	ASSERT(pLog != NULL);

	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if ((xRet == FTM_RET_OK) && (pService != NULL))
	{
		xRet = FTOM_DMC_LOG_add(pService->pData, pLog);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, NULL);
			return	xRet;
		}
	}
	else
	{
		xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
		if (xRet != FTM_RET_OK)
		{
			FTM_LOG_destroy(&pLog);
			return	xRet;	
		}
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_remove
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulRemovedCount
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulRemovedCount = 0;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if ((xRet == FTM_RET_OK) && (pService != NULL))
	{
		xRet = FTOM_DMC_LOG_del(pService->pData, ulIndex, ulCount, &ulRemovedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, NULL);
		}
		else
		{
			*pulRemovedCount = ulRemovedCount;	
		}
	}
	else
	{
		xRet = FTM_LOGGER_removeAt(&pLogger->xCommon, ulIndex, ulCount, &ulRemovedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, NULL);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_count
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;

	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if ((xRet == FTM_RET_OK) && (pService != NULL))
	{
		xRet = FTOM_DMC_LOG_count(pService->pData, pulCount);
	}
	else
	{
		xRet = FTM_LOGGER_count(&pLogger->xCommon, pulCount);
	}

	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get log count!\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_getAt
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if ((xRet == FTM_RET_OK) && (pService != NULL))
	{
		xRet = FTOM_DMC_LOG_get(pService->pData, ulIndex, ulCount, pLogs, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, NULL);
		}
	}
	else
	{
		FTM_INT	i;

		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_LOG_PTR	pLog;

			xRet = FTM_LOGGER_getAt(&pLogger->xCommon, ulIndex + i, &pLog);
			if (xRet != FTM_RET_OK)
			{
				break;
			}

			memcpy(&pLogs[i], pLog, sizeof(FTM_LOG));
		}

		*pulCount = i;
		xRet = FTM_RET_OK;
	}

	return	xRet;
}

/*********************************************************************
 * Log messages
 *********************************************************************/
FTM_RET	FTOM_LOG_createNode
(
	FTM_NODE_PTR	pNodeInfo
)
{
	ASSERT(pNodeInfo != NULL);
	
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_NODE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pNodeInfo->pDID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_destroyNode
(
	FTM_NODE_PTR	pNodeInfo
)
{
	ASSERT(pNodeInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_NODE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pNodeInfo->pDID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_createEP
(
	FTM_EP_PTR		pEPInfo
)
{
	ASSERT(pEPInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_EP;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pEPInfo->pEPID, FTM_EPID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_destroyEP
(
	FTM_EP_PTR	pEPInfo
)
{
	ASSERT(pEPInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_EP;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pEPInfo->pEPID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_createTrigger
(
	FTM_TRIGGER_PTR	pTriggerInfo
)
{
	ASSERT(pTriggerInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_TRIGGER;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pTriggerInfo->pID, FTM_ID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_destroyTrigger
(
	FTM_TRIGGER_PTR	pTriggerInfo
)
{
	ASSERT(pTriggerInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_TRIGGER;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pTriggerInfo->pID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_createAction
(
	FTM_ACTION_PTR	pActionInfo
)
{
	ASSERT(pActionInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_ACTION;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pActionInfo->pID, FTM_ID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_destroyAction
(
	FTM_ACTION_PTR	pActionInfo
)
{
	ASSERT(pActionInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_ACTION;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pActionInfo->pID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_createRule
(
	FTM_RULE_PTR	pRuleInfo
)
{
	ASSERT(pRuleInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_RULE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pRuleInfo->pID, FTM_ID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_count
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_LOGGER_count(&pLogger->xCommon, pulCount);
}

FTM_RET	FTOM_LOG_destroyRule
(
	FTM_RULE_PTR	pRuleInfo
)
{
	ASSERT(pRuleInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_RULE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pRuleInfo->pID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_LOG_event
(
	FTM_RULE_PTR	pRuleInfo,
	FTM_BOOL		bOccurred
)
{
	ASSERT(pRuleInfo != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType 	= FTM_LOG_TYPE_EVENT;
	pLog->ulTime 	= time(NULL);
	strncpy(pLog->xParams.xEvent.pRuleID, pRuleInfo->pID, FTM_ID_LEN);
	pLog->xParams.xEvent.bOccurred = bOccurred;

	xRet = FTOM_LOGGER_add(pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}


FTM_RET	FTOM_LOGGER_DMC_get
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLog,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if ((xRet == FTM_RET_OK) && (pService != NULL))
	{
		TRACE("Log add!\n");
		xRet = FTOM_DMC_LOG_get(pService->pData, ulIndex, ulCount, pLog, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, NULL);
		}
	}

	return	xRet;
}


