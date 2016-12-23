#include "ftdm.h"
#include "ftm_log.h"
#include "ftdm_log.h"

extern
FTDM_CONTEXT	xFTDM;

FTM_RET	FTDM_LOG_createNode
(
	FTM_CHAR_PTR	pDID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pDID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_NODE;
		xRet = FTDM_LOGGER_add(xFTDM.pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOG_createEP
(
	FTM_CHAR_PTR	pEPID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pEPID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_EP;
		xRet = FTDM_LOGGER_add(xFTDM.pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOG_createEvent
(
	FTM_CHAR_PTR	pID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_EVENT;
		xRet = FTDM_LOGGER_add(xFTDM.pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOG_createAction
(
	FTM_CHAR_PTR	pID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_ACTION;
		xRet = FTDM_LOGGER_add(xFTDM.pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LOG_createRule
(
	FTM_CHAR_PTR	pID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_RULE;
		xRet = FTDM_LOGGER_add(xFTDM.pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

