#include <sys/time.h>
#include <string.h>
#include "ftm_event.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"
#include "ftm_mem.h"

static	FTM_RET			FTM_EVENT_create(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EP_DATA_PTR pData1, FTM_EP_DATA_PTR pData2, FTM_EVENT_PTR _PTR_ ppEvent);
static	FTM_LIST_PTR	pEventList = NULL;

FTM_RET	FTM_EVENT_init(FTM_VOID)
{
	if (pEventList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pEventList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(pEventList));
	if (pEventList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_LIST_init(pEventList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_final(FTM_VOID)
{
	if (pEventList != NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_LIST_final(pEventList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_createAbove(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create(xType, xEPID, pData, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createBelow(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create(xType, xEPID, pData, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createInclude(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create(xType, xEPID, pUpper, pLower, ppEvent);
}

FTM_RET	FTM_EVENT_createExcept(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create(xType, xEPID, pUpper, pLower, ppEvent);
}

FTM_RET	FTM_EVENT_createChange(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create(xType, xEPID, NULL, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_create(FTM_EVENT_TYPE xType, FTM_EPID xEPID, FTM_EP_DATA_PTR pData1, FTM_EP_DATA_PTR pData2, FTM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pEventList != NULL);
	ASSERT(ppEvent != NULL);

	FTM_RET			xRet;
	FTM_EVENT		xEvent;
	FTM_EVENT_PTR	pEvent;
	struct timeval	xTime;

	gettimeofday(&xTime, NULL);
	
	xEvent.ulID = xTime.tv_sec * 1000000 + xTime.tv_usec;
	xEvent.xType = xType;
	xEvent.xEPID = xEPID;
	
	switch(xType)
	{
	case	FTM_EVENT_TYPE_ABOVE:
	case	FTM_EVENT_TYPE_BELOW:
		{
			memcpy(&xEvent.xParams.xAbove.xValue, pData1, sizeof(FTM_EP_DATA));
		}
		break;

	case	FTM_EVENT_TYPE_INCLUDE:
	case	FTM_EVENT_TYPE_EXCEPT:
		{
			memcpy(&xEvent.xParams.xInclude.xUpper, pData1, sizeof(FTM_EP_DATA));
			memcpy(&xEvent.xParams.xInclude.xLower, pData2, sizeof(FTM_EP_DATA));
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	pEvent = (FTM_EVENT_PTR)FTM_MEM_malloc(sizeof(FTM_EVENT));
	if (pEvent == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pEvent, &xEvent, sizeof(xEvent));

	xRet = FTM_LIST_append(pEventList, pEvent);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEvent);
		return	xRet;	
	}

	*ppEvent = pEvent;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_destroy(FTM_EVENT_PTR pEvent)
{
	ASSERT(pEventList != NULL);
	ASSERT(pEvent != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pEventList, pEvent);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEvent);	
	}

	return	xRet;
}

FTM_BOOL	FTM_EVENT_occurred(FTM_EVENT_PTR pEvent, FTM_EP_DATA_PTR pPrevData, FTM_EP_DATA_PTR pCurrData)
{
	ASSERT(pEvent != NULL);
	ASSERT(pPrevData != NULL);
	ASSERT(pCurrData != NULL);

	switch(pEvent->xType)
	{
	case	FTM_EVENT_TYPE_ABOVE:
		{
		}
		break;

	case	FTM_EVENT_TYPE_BELOW:
		{
		}
		break;

	case	FTM_EVENT_TYPE_INCLUDE:
		{
		}
		break;

	case 	FTM_EVENT_TYPE_EXCEPT:
		{
		}
		break;

	case	FTM_EVENT_TYPE_CHANGE:
		{
		}
		break;
	}
	return	FTM_TRUE;
}
