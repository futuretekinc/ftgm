#include <string.h>
#include "ftdm.h"
#include "ftdm_dbif.h"
#include "ftdm_logger.h"

FTM_RET	FTDM_LOGGER_create
(
	FTDM_DBIF_PTR		pDBIF,
	FTDM_LOGGER_PTR	_PTR_ ppLogger
)
{
	ASSERT(ppLogger != NULL);
	FTDM_LOGGER_PTR	pLogger;

	pLogger = (FTDM_LOGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_LOGGER));
	if (pLogger == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pLogger->xConfig.pName, "log");
	pLogger->xConfig.xLimit.xType = FTM_LIMIT_TYPE_COUNT;
	pLogger->xConfig.xLimit.xParams.ulCount=100;

	pLogger->ulCount = 0;
	pLogger->ulFirstTime = 0;
	pLogger->ulLastTime = 0;

	pLogger->pDBIF = pDBIF;

	*ppLogger = pLogger;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOGGER_destroy
(
	FTDM_LOGGER_PTR _PTR_ ppLogger
)
{
	ASSERT(ppLogger !=NULL);

	FTM_MEM_free((*ppLogger));

	*ppLogger = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOGGER_init
(
	FTDM_LOGGER_PTR	pLogger
)
{
	ASSERT(pLogger != NULL);
	
	FTM_RET		xRet;
	FTM_BOOL	bExist = FTM_FALSE;

	xRet = FTDM_DBIF_isLogTableExist(pLogger->pDBIF, pLogger->xConfig.pName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (!bExist)
	{
		xRet = FTDM_DBIF_initLogTable(pLogger->pDBIF, pLogger->xConfig.pName);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOGGER_add
(
	FTDM_LOGGER_PTR		pLogger,
	FTM_LOG_PTR	pLog
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pLog != NULL);

	FTM_RET		xRet;

	if (pLogger->xConfig.xLimit.xType == FTM_LIMIT_TYPE_COUNT)
	{
	
		if (pLogger->ulCount >= pLogger->xConfig.xLimit.xParams.ulCount)
		{
			FTM_ULONG	ulCount = pLogger->ulCount - pLogger->xConfig.xLimit.xParams.ulCount + 1;

			xRet = FTDM_DBIF_delLog(pLogger->pDBIF, pLogger->xConfig.pName, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				pLogger->ulCount -= ulCount;

				FTM_ULONG	ulBeginTime, ulEndTime;
				xRet = FTDM_DBIF_getLogInfo(pLogger->pDBIF, pLogger->xConfig.pName, &ulCount, &ulBeginTime, &ulEndTime);
				if (xRet == FTM_RET_OK)
				{
					pLogger->ulCount = ulCount;
					pLogger->ulFirstTime = ulBeginTime;
					pLogger->ulLastTime = ulEndTime;
				}
			}

		}
	}

	xRet = FTDM_DBIF_appendLog(pLogger->pDBIF, pLogger->xConfig.pName, pLog);
	if (xRet == FTM_RET_OK)
	{
		pLogger->ulCount++;	
		if ((pLogger->ulLastTime == 0) || (pLogger->ulLastTime < pLog->ulTime))
		{
			pLogger->ulLastTime = pLog->ulTime;
		}

		if((pLogger->ulFirstTime == 0) || (pLogger->ulFirstTime > pLog->ulTime))
		{
			pLogger->ulFirstTime = pLog->ulTime;
		}
	}

	return	xRet;
}

FTM_RET FTDM_LOGGER_info
(	
	FTDM_LOGGER_PTR	pLogger,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pulBeginTime != NULL);
	ASSERT(pulEndTime != NULL);
	ASSERT(pulCount != NULL);

	*pulBeginTime = pLogger->ulFirstTime;
	*pulEndTime	= pLogger->ulLastTime;
	*pulCount = pLogger->ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOGGER_get
(
	FTDM_LOGGER_PTR	pLogger,
	FTM_ULONG		nStartIndex,
	FTM_LOG_PTR 	pLoggerData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pCount 
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pLoggerData != NULL);
	ASSERT(pCount != NULL);

	return	FTDM_DBIF_getLogList(
				pLogger->pDBIF,
				pLogger->xConfig.pName, 
				nStartIndex,
				nMaxCount, 
				pLoggerData, 
				pCount);
}

FTM_RET	FTDM_LOGGER_getWithTime
(
	FTDM_LOGGER_PTR	pLogger,
	FTM_ULONG 		nBeginTime, 
	FTM_ULONG 		nEndTime, 
	FTM_LOG_PTR 	pLoggerData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pCount 
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pCount != NULL);

	return	FTDM_DBIF_getLogWithTime(
				pLogger->pDBIF,	
				pLogger->xConfig.pName, 
				nBeginTime, 
				nEndTime, 
				pLoggerData, 
				nMaxCount, 
				pCount);
}

FTM_RET	FTDM_LOGGER_del
(
	FTDM_LOGGER_PTR			pLogger,
	FTM_INT 			nIndex, 
	FTM_ULONG			nCount,
	FTM_ULONG_PTR		pulCount
) 
{
	FTM_RET		xRet;
	FTM_ULONG	ulFirstTime, ulLastTime;
	FTM_ULONG	ulCount1 = 0, ulCount2= 0;

	xRet = FTDM_DBIF_getLogInfo(pLogger->pDBIF, pLogger->xConfig.pName, &ulCount1, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to get log information.\n");
		return	xRet;	
	}

	xRet = FTDM_DBIF_delLog(pLogger->pDBIF, pLogger->xConfig.pName, nIndex, nCount, &nCount);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_getLogInfo(pLogger->pDBIF, pLogger->xConfig.pName, &ulCount2, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%s] information update failed.\n", pLogger->xConfig.pName);	
		}
		else
		{
			pLogger->ulFirstTime = ulFirstTime;
			pLogger->ulLastTime = ulLastTime;
			pLogger->ulCount = ulCount2;

			if (ulCount1 > ulCount2)
			{
				*pulCount = ulCount1 - ulCount2;
			}
			else
			{
				*pulCount = 0;
			}

			TRACE("Log deleted[%lu, %lu, %lu]\n", ulFirstTime, ulLastTime, *pulCount);
		}
	
	}

	return	xRet;
}

FTM_RET	FTDM_LOGGER_delWithTime
(
	FTDM_LOGGER_PTR			pLogger,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pulCount
) 
{
	FTM_RET		xRet;
	FTM_ULONG	ulFirstTime, ulLastTime;
	FTM_ULONG	ulCount, ulCount1 = 0, ulCount2 = 0;

	xRet = FTDM_DBIF_getLogInfo(pLogger->pDBIF, pLogger->xConfig.pName, &ulCount1, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTDM_DBIF_delLogWithTime(pLogger->pDBIF, pLogger->xConfig.pName, nBeginTime, nEndTime, &ulCount);
	if (xRet == FTM_RET_OK)
	{

		xRet = FTDM_DBIF_getLogInfo(pLogger->pDBIF, pLogger->xConfig.pName, &ulCount2, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%s] information update failed.\n", pLogger->xConfig.pName);	
		}
		else
		{
			pLogger->ulFirstTime = ulFirstTime;
			pLogger->ulLastTime = ulLastTime;
			pLogger->ulCount = ulCount2;

			if (ulCount1 > ulCount2)
			{
				*pulCount = ulCount1 - ulCount2;
			}
			else
			{
				*pulCount = 0;
			}
		}
	
	}

	return	xRet;
}

FTM_RET	FTDM_LOGGER_count
(
	FTDM_LOGGER_PTR			pLogger,
	FTM_ULONG_PTR		pulCount
) 
{
	ASSERT(pulCount != NULL);

	*pulCount = pLogger->ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOGGER_countWithTime
(
	FTDM_LOGGER_PTR			pLogger,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pulCount
) 
{
	ASSERT(pulCount != NULL);

	return	FTDM_DBIF_getLogCountWithTime(pLogger->pDBIF, pLogger->xConfig.pName, nBeginTime, nEndTime, pulCount);
}

