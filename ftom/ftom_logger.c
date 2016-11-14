#include "ftom.h"
#include "ftm_logger.h"
#include "ftom_logger.h"
#include "ftom_server.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_LOGGER

FTM_RET	FTOM_LOGGER_init
(
	FTOM_LOG_MANAGER_PTR	pManager
)
{
	ASSERT(pManager != NULL);

	FTM_RET	xRet;	

	xRet = FTM_LOGGER_init(&pManager->xLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Logger initialize failed!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_final
(
	FTOM_LOG_MANAGER_PTR	pManager
)
{
	ASSERT(pManager != NULL);

	FTM_RET	xRet;	

	xRet = FTM_LOGGER_final(&pManager->xLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Logger finalize failed!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_add
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_LOG_PTR		pLog
)
{
	ASSERT(pManager != NULL);
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
		else
		{
			FTM_MEM_free(pLog);
		}
	}
	else
	{
		xRet  = FTM_LOGGER_add(&pManager->xLogger, pLog);
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
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulRemovedCount
)
{
	ASSERT(pManager != NULL);

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
		xRet = FTM_LOGGER_removeAt(&pManager->xLogger, ulIndex, ulCount, &ulRemovedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, NULL);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_count
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pManager != NULL);
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
		xRet = FTM_LOGGER_count(&pManager->xLogger, pulCount);
	}

	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get log count!\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_LOGGER_getAt
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pManager != NULL);
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

			xRet = FTM_LOGGER_getAt(&pManager->xLogger, ulIndex + i, &pLog);
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


FTM_RET	FTOM_LOGGER_DMC_get
(
	FTOM_LOG_MANAGER_PTR	pManager,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLog,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pManager != NULL);

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


