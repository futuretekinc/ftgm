#include "ftm.h"
#include "ftm_trace.h"
#include "ftm_logger.h"

FTM_RET	FTM_LOGGER_init
(
	FTM_LOGGER_PTR	pLogger
)
{
	ASSERT(pLogger != NULL);

	FTM_LIST_init(&pLogger->xLogList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOGGER_final
(
	FTM_LOGGER_PTR	pLogger
)
{
	ASSERT(pLogger != NULL);

	FTM_LOG_PTR	pLog;

	FTM_LIST_iteratorStart(&pLogger->xLogList);
	while(FTM_LIST_iteratorNext(&pLogger->xLogList, (FTM_VOID_PTR _PTR_)&pLog) == FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);	
	}

	FTM_LIST_final(&pLogger->xLogList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOGGER_count
(
	FTM_LOGGER_PTR	pLogger,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pLogger->xLogList, pulCount);
}

FTM_RET	FTM_LOGGER_add
(
	FTM_LOGGER_PTR	pLogger,
	FTM_LOG_PTR		pLog
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pLog != NULL);

	return	FTM_LIST_append(&pLogger->xLogList, pLog);
}

FTM_RET	FTM_LOGGER_removeAt
(
	FTM_LOGGER_PTR	pLogger,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulRemovedCount
)
{
	ASSERT(pLogger != NULL);
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;
	FTM_ULONG	i;
	FTM_ULONG	ulRemovedCount = 0;

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_LIST_getAt(&pLogger->xLogList, ulIndex, (FTM_VOID_PTR _PTR_)&pLog);
		if (xRet != FTM_RET_OK)
		{
			break;
		}

		xRet = FTM_LIST_remove(&pLogger->xLogList, pLog);
		if (xRet != FTM_RET_OK)
		{
			break;
		}

		ulRemovedCount++;
	}

	if (pulRemovedCount != NULL)
	{
		*pulRemovedCount = ulRemovedCount;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOGGER_getAt
(
	FTM_LOGGER_PTR	pLogger,
	FTM_ULONG		ulIndex,
	FTM_LOG_PTR	_PTR_ ppLog
)
{
	ASSERT(pLogger != NULL);
	ASSERT(ppLog != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LIST_getAt(&pLogger->xLogList, ulIndex, (FTM_VOID_PTR _PTR_)&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*ppLog = pLog;

	return	FTM_RET_OK;
}

