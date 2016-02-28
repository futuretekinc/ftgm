#include <sys/time.h>
#include <string.h>
#include "ftm_event.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"
#include "ftm_mem.h"

static	FTM_RET			FTM_EVENT_create1(FTM_EVENT_TYPE xType, FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData1, FTM_EP_DATA_PTR pData2, FTM_EVENT_PTR _PTR_ ppEvent);
static  FTM_RET			FTM_EVENT_create2(FTM_EVENT_TYPE xType, FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_ULONG ulEvent1, FTM_ULONG ulEvent2, FTM_EVENT_PTR _PTR_ ppEvent);
static 	FTM_BOOL		FTM_EVENT_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator);

static	FTM_LIST_PTR	pEventList = NULL;

FTM_RET	FTM_EVENT_init(FTM_VOID)
{
	if (pEventList != NULL)
	{
		ERROR("Event list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pEventList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pEventList == NULL)
	{
		ERROR("Event list is not allocated.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_LIST_init(pEventList);
	FTM_LIST_setSeeker(pEventList, FTM_EVENT_seeker);

	TRACE("Event list is initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_final(FTM_VOID)
{
	FTM_EVENT_PTR	pEvent;

	if (pEventList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

 	FTM_LIST_iteratorStart(pEventList);
  	while(FTM_LIST_iteratorNext(pEventList, (FTM_VOID_PTR _PTR_)&pEvent) == FTM_RET_OK)
   	{   
    	FTM_EVENT_destroy(pEvent);
	}

	FTM_LIST_final(pEventList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_createCopy(FTM_EVENT_PTR pSrc, FTM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pSrc != NULL);

	FTM_RET			xRet;
	FTM_EVENT_PTR	pEvent;
	struct timeval	xTime;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	pEvent = (FTM_EVENT_PTR)FTM_MEM_malloc(sizeof(FTM_EVENT));
	if (pEvent == NULL)
	{
		ERROR("Can't not allocation Event.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pEvent, pSrc, sizeof(FTM_EVENT));

	xRet = FTM_LIST_append(pEventList, pEvent);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEvent);
		return	xRet;
	}

	if (pEvent->xID == 0)
	{
		gettimeofday(&xTime, NULL);
		pEvent->xID = xTime.tv_sec * 1000000 + xTime.tv_usec;
	}

	if (ppEvent != NULL)
	{
		*ppEvent = pEvent;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_createAbove(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_ABOVE, xEventID, xEPID, pData, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createBelow(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_BELOW, xEventID, xEPID, pData, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createInclude(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_INCLUDE, xEventID, xEPID, pUpper, pLower, ppEvent);
}

FTM_RET	FTM_EVENT_createExcept(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_EXCEPT, xEventID, xEPID, pUpper, pLower, ppEvent);
}

FTM_RET	FTM_EVENT_createChange(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_CHANGE, xEventID, xEPID, NULL, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createAnd(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EVENT_ID xEvent1, FTM_EVENT_ID xEvent2, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create2(FTM_EVENT_TYPE_AND, xEventID, xEPID, xEvent1, xEvent2, ppEvent);
}

FTM_RET	FTM_EVENT_createOr(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EVENT_ID xEvent1, FTM_EVENT_ID xEvent2, FTM_EVENT_PTR _PTR_ ppEvent)
{
	return	FTM_EVENT_create2(FTM_EVENT_TYPE_OR, xEventID, xEPID, xEvent1, xEvent2, ppEvent);
}

FTM_RET	FTM_EVENT_create1(FTM_EVENT_TYPE xType, FTM_EVENT_ID xEventID, FTM_EP_ID xEPID,  FTM_EP_DATA_PTR pData1, FTM_EP_DATA_PTR pData2, FTM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pEventList != NULL);

	FTM_RET			xRet;
	FTM_EVENT		xEvent;
	FTM_EVENT_PTR	pEvent;
	struct timeval	xTime;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	if (xEventID == 0)
	{
		gettimeofday(&xTime, NULL);
	
		xEvent.xID = xTime.tv_sec * 1000000 + xTime.tv_usec;
	}
	else
	{
		xEvent.xID = xEventID;	
	}
	xEvent.xType = xType;
	
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

	if (ppEvent != NULL)
	{
		*ppEvent = pEvent;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_create2(FTM_EVENT_TYPE xType, FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EVENT_ID xEvent1, FTM_EVENT_ID xEvent2, FTM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pEventList != NULL);

	FTM_RET			xRet;
	FTM_EVENT		xEvent;
	FTM_EVENT_PTR	pEvent;
	struct timeval	xTime;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	if (xEventID == 0)
	{
		gettimeofday(&xTime, NULL);

		xEvent.xID = xTime.tv_sec * 1000000 + xTime.tv_usec;
	}
	else
	{
		xEvent.xID = xEventID;
	}

	xEvent.xType = xType;
	xEvent.xParams.xAnd.xID1 = xEvent1;
	xEvent.xParams.xAnd.xID2 = xEvent2;

	pEvent = (FTM_EVENT_PTR)FTM_MEM_malloc(sizeof(FTM_EVENT));
	if (pEvent == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pEvent, &xEvent, sizeof(FTM_EVENT));

	xRet = FTM_LIST_append(pEventList, pEvent);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEvent);
		return	xRet;	
	}

	if (ppEvent != NULL)
	{
		*ppEvent = pEvent;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(pEventList, pulCount);
}

FTM_RET	FTM_EVENT_get(FTM_ULONG ulEventID, FTM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pEventList != NULL);
	ASSERT(ppEvent != NULL);

	return	FTM_LIST_get(pEventList, &ulEventID, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTM_EVENT_getAt(FTM_ULONG ulIndex, FTM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pEventList != NULL);
	ASSERT(ppEvent != NULL);

	return	FTM_LIST_getAt(pEventList, ulIndex, (FTM_VOID_PTR _PTR_)ppEvent);
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

FTM_RET	FTM_EVENT_occurred(FTM_EVENT_PTR pEvent, FTM_EP_DATA_PTR pPrevData, FTM_EP_DATA_PTR pCurrData, FTM_BOOL_PTR pResult)
{
	ASSERT(pEvent != NULL);
	ASSERT(pPrevData != NULL);
	ASSERT(pCurrData != NULL);

	FTM_RET	xRet;
	FTM_INT	nResult;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	switch(pEvent->xType)
	{
	case	FTM_EVENT_TYPE_ABOVE:
		{
			xRet = FTM_EP_DATA_compare(&pEvent->xParams.xAbove.xValue, pCurrData, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult <= 0);
			}
		}
		break;

	case	FTM_EVENT_TYPE_BELOW:
		{
			xRet = FTM_EP_DATA_compare(&pEvent->xParams.xBelow.xValue, pCurrData, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult >= 0);
			}
		}
		break;

	case	FTM_EVENT_TYPE_INCLUDE:
		{
			xRet = FTM_EP_DATA_compare(&pEvent->xParams.xInclude.xUpper, pCurrData, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult >= 0);
				if (*pResult != FTM_FALSE)
				{
					xRet = FTM_EP_DATA_compare(&pEvent->xParams.xInclude.xLower, pCurrData, &nResult);
					if (xRet == FTM_RET_OK)
					{
						*pResult = (nResult <= 0);
					}
				}
			}
		}
		break;

	case 	FTM_EVENT_TYPE_EXCEPT:
		{
			xRet = FTM_EP_DATA_compare(&pEvent->xParams.xExcept.xUpper, pCurrData, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult < 0);
				if (*pResult != FTM_TRUE)
				{
					xRet = FTM_EP_DATA_compare(&pEvent->xParams.xExcept.xLower, pCurrData, &nResult);
					if (xRet == FTM_RET_OK)
					{
						*pResult = (nResult > 0);
					}
				}
			}
		}
		break;

	case	FTM_EVENT_TYPE_CHANGE:
		{
			xRet = FTM_EP_DATA_compare(&pEvent->xParams.xAbove.xValue, pCurrData, &nResult);
			*pResult = (nResult == 0);
		}
		break;
	
	case	FTM_EVENT_TYPE_AND:
		{
			FTM_EVENT_PTR	pEvent1, pEvent2;
			FTM_BOOL		bEvent1, bEvent2;

			xRet = FTM_LIST_get(pEventList, (FTM_VOID_PTR)&pEvent->xParams.xAnd.xID1, (FTM_VOID_PTR _PTR_)&pEvent1);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_EVENT_occurred(pEvent1, pPrevData, pCurrData, &bEvent1);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;	
			}
		
			if (bEvent1 != FTM_TRUE)
			{
				*pResult = FTM_FALSE;
				break;
			}

			xRet = FTM_LIST_get(pEventList, (FTM_VOID_PTR)&pEvent->xParams.xAnd.xID2, (FTM_VOID_PTR _PTR_)&pEvent2);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_EVENT_occurred(pEvent2, pPrevData, pCurrData, &bEvent2);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;	
			}
			*pResult = bEvent2;
		}

	case	FTM_EVENT_TYPE_OR:
		{
			FTM_EVENT_PTR	pEvent1, pEvent2;
			FTM_BOOL		bEvent1, bEvent2;

			xRet = FTM_LIST_get(pEventList, (FTM_VOID_PTR)&pEvent->xParams.xAnd.xID1, (FTM_VOID_PTR _PTR_)&pEvent1);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_EVENT_occurred(pEvent1, pPrevData, pCurrData, &bEvent1);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;	
			}

			if (bEvent1 == FTM_TRUE)
			{
				*pResult = FTM_TRUE;
				break;
			}

			xRet = FTM_LIST_get(pEventList, (FTM_VOID_PTR)&pEvent->xParams.xAnd.xID2, (FTM_VOID_PTR _PTR_)&pEvent2);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_EVENT_occurred(pEvent2, pPrevData, pCurrData, &bEvent2);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;	
			}
			*pResult = bEvent2;
		}
		break;
	}

	return	xRet;
}

static FTM_CHAR_PTR	pTypeString[] =
{
	"ABOVE",
	"BELOW",
	"INCLUDE",
	"EXCEPT",
	"CHANGE",
	"AND",
	"OR"
};

FTM_CHAR_PTR	FTM_EVENT_typeString(FTM_EVENT_TYPE xType)
{
	if (xType < sizeof(pTypeString) / sizeof(FTM_CHAR_PTR))
	{
		return	pTypeString[xType];
	}

	return	"UNKNOWN";
}

FTM_RET	FTM_EVENT_conditionToString(FTM_EVENT_PTR pEvent, FTM_CHAR_PTR pBuff, FTM_ULONG ulBuffLen)
{
	ASSERT(pEvent != NULL);
	ASSERT(pBuff != NULL);

	FTM_ULONG	ulLen = 0;
	FTM_CHAR	pTemp[256];

	switch(pEvent->xType)
	{   
	case    FTM_EVENT_TYPE_ABOVE:
		{   
			FTM_EP_DATA_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xAbove.xValue);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "x >= %s", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_BELOW:
		{   
			FTM_EP_DATA_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xBelow.xValue);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "x <= %s", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_INCLUDE:
		{   
			FTM_EP_DATA_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xInclude.xLower);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%s <= x <= ", pTemp);

			FTM_EP_DATA_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xInclude.xUpper);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%s)", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_EXCEPT:
		{   
			FTM_EP_DATA_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xExcept.xLower);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "(x < %s) && ", pTemp);

			FTM_EP_DATA_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xExcept.xUpper);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "(%s < x)", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_CHANGE:
		{   
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "CHANGE");
		}   
		break;

	case    FTM_EVENT_TYPE_AND:
		{   
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%lu and %lu", pEvent->xParams.xAnd.xID1, pEvent->xParams.xAnd.xID2);
		}   
		break;

	case    FTM_EVENT_TYPE_OR:
		{   
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%lu or %lu", pEvent->xParams.xAnd.xID1, pEvent->xParams.xAnd.xID2);
		}
		break;

	default:
		{   
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "UNKNOWN");
		}
	}

	return	FTM_RET_OK;

}

FTM_BOOL		FTM_EVENT_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_EVENT_PTR		pEvent = (FTM_EVENT_PTR)pItem;
	FTM_EVENT_ID_PTR	pEventID = (FTM_EVENT_ID_PTR)pIndicator;

	return	(pEvent->xID == *pEventID);
}
