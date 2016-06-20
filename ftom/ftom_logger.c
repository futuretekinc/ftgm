#include "ftom.h"
#include "ftom_logger.h"

FTM_RET	FTOM_LOGGER_create
(
	FTOM_LOGGER_PTR	_PTR_ ppLogger
)
{
	ASSERT(ppLogger != NULL);
	FTM_RET	xRet;	
	FTOM_LOGGER_PTR	pLogger ;

	pLogger = (FTOM_LOGGER_PTR)FTM_MEM_malloc(sizeof(FTOM_LOGGER));
	if (pLogger == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_LOGGER_init(&pLogger->xCommon);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pLogger);	
	}
	else
	{
		*ppLogger = pLogger;	
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_destroy
(
	FTOM_LOGGER_PTR	_PTR_ ppLogger
)
{
	ASSERT(ppLogger != NULL);
	FTM_RET	xRet;	

	xRet = FTM_LOGGER_final(&(*ppLogger)->xCommon);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(*ppLogger);
		*ppLogger = NULL;
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_createNode
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pDID != NULL);
	
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_NODE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pDID, FTM_DID_LEN);

	xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_LOGGER_destroyNode
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pDID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_NODE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pDID, FTM_DID_LEN);

	xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_LOGGER_createEP
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_EP;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pEPID, FTM_EPID_LEN);

	xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_LOGGER_createTrigger
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pTriggerID
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pTriggerID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_TRIGGER;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pTriggerID, FTM_ID_LEN);

	xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_LOGGER_createAction
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pActionID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_ACTION;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pActionID, FTM_ID_LEN);

	xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_LOGGER_createRule
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pRuleID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_RULE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pRuleID, FTM_ID_LEN);

	xRet  = FTM_LOGGER_add(&pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_LOGGER_count
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LOGGER_count(&pLogger->xCommon, pulCount);
}

FTM_RET	FTOM_LOGGER_remove
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_ULONG		ulCount
)
{
	ASSERT(pLogger != NULL);
	
	FTM_RET	xRet;
	FTM_INT	i;

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_LOGGER_removeAt(&pLogger->xCommon, 0);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}
	}

	return	FTM_RET_OK;
}


FTM_RET	FTOM_LOGGER_event
(
	FTOM_LOGGER_PTR	pLogger,
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL		bOccurred
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pRuleID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType 	= FTM_LOG_TYPE_EVENT;
	pLog->ulTime 	= time(NULL);
	strncpy(pLog->xParams.xEvent.pRuleID, pRuleID, FTM_ID_LEN);
	pLog->xParams.xEvent.bOccurred = bOccurred;

	xRet  = FTM_LOGGER_add(pLogger->xCommon, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
		return	xRet;	
	}

	return	FTM_RET_OK;
}


