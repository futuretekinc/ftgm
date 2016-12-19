#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "ftm_log.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_LOG

FTM_RET	FTM_LOG_create
(
	FTM_LOG_PTR _PTR_ ppLog
)
{
	ASSERT(ppLog != NULL);

	FTM_LOG_PTR	pLog;
	struct timeval	xTime;
	
	pLog = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG));
	if (pLog == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	gettimeofday(&xTime, NULL);

	pLog->ullID = ((FTM_UINT64)xTime.tv_sec) * 1000000 + (FTM_UINT64)xTime.tv_usec;
	pLog->xType = FTM_LOG_TYPE_UNKNOWN;
	pLog->ulTime= xTime.tv_sec;

	*ppLog = pLog;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOG_destroy
(
	FTM_LOG_PTR _PTR_ ppLog
)
{
	ASSERT(ppLog != NULL);

	FTM_MEM_free((*ppLog));

	*ppLog = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOG_duplicate
(
	FTM_LOG_PTR	pLog,
	FTM_LOG_PTR _PTR_ ppNewLog
)
{
	ASSERT(pLog != NULL);
	ASSERT(ppNewLog != NULL);
	FTM_RET		xRet;
	FTM_LOG_PTR	pNewLog;

	xRet = FTM_LOG_create(&pNewLog);
	if(xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(pNewLog, pLog, sizeof(FTM_LOG));

	*ppNewLog = pNewLog;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOG_createObject
(
	FTM_CHAR_PTR	pObjectID,
	FTM_RET			xResult,
	FTM_LOG_PTR	_PTR_ ppLog
)
{
	ASSERT(pObjectID != NULL);
	ASSERT(ppLog != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_OBJECT;
	pLog->xParams.xCreateObject.xResult=xRet;
	strncpy(pLog->xParams.xCreateObject.pObjectID, pObjectID, FTM_ID_LEN);
	
	*ppLog = pLog;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOG_destroyObject
(
	FTM_CHAR_PTR	pObjectID,
	FTM_RET			xResult,
	FTM_LOG_PTR	_PTR_ ppLog
)
{
	ASSERT(pObjectID != NULL);
	ASSERT(ppLog != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_OBJECT;
	pLog->xParams.xDestroyObject.xResult=xRet;
	strncpy(pLog->xParams.xCreateObject.pObjectID, pObjectID, FTM_ID_LEN);
	
	*ppLog = pLog;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOG_createEvent
(
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL		bOccurred,
	FTM_LOG_PTR	_PTR_ ppLog
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(ppLog != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_EVENT;
	pLog->xParams.xEvent.bOccurred = bOccurred;
	strncpy(pLog->xParams.xEvent.pRuleID, pRuleID, FTM_ID_LEN);
	
	*ppLog = pLog;

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_LOG_print
(
	FTM_LOG_PTR	pLog
)
{
	static FTM_CHAR	pBuff[256];

	switch(pLog->xType)
	{
	case FTM_LOG_TYPE_CREATE_OBJECT:
		sprintf(pBuff,"Object[%s] created.", pLog->xParams.xCreateObject.pObjectID);
		break;

	case FTM_LOG_TYPE_CREATE_NODE:
		sprintf(pBuff,"Node[%s] created.", pLog->xParams.xCreateObject.pObjectID);
		break;

	case FTM_LOG_TYPE_CREATE_EP:
		sprintf(pBuff,"EP[%s] created.", pLog->xParams.xCreateObject.pObjectID);
		break;

	case FTM_LOG_TYPE_CREATE_TRIGGER:
		sprintf(pBuff,"Trigger[%s] created.", pLog->xParams.xCreateObject.pObjectID);
		break;

	case FTM_LOG_TYPE_CREATE_ACTION:
		sprintf(pBuff,"Action[%s] created.", pLog->xParams.xCreateObject.pObjectID);
		break;

	case FTM_LOG_TYPE_CREATE_RULE:
		sprintf(pBuff,"Rule[%s] created.", pLog->xParams.xCreateObject.pObjectID);
		break;

	default:
		sprintf(pBuff,"Unknown type[%08lx]", (FTM_ULONG)pLog->xType);
	}

	return	pBuff;
}
