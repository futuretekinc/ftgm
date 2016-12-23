#include <sys/time.h>
#include <string.h>
#include "ftm_event.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_OBJECT

static	
FTM_RET	FTM_EVENT_create1
(
	FTM_EVENT_TYPE 	xType, 
	FTM_CHAR_PTR		pID,
	FTM_CHAR_PTR		pEPID,
	FTM_VALUE_PTR		pValue1, 
	FTM_VALUE_PTR		pValue2, 
	FTM_EVENT_PTR _PTR_ ppEvent
);

static 	
FTM_BOOL FTM_EVENT_seeker
(
	const FTM_VOID_PTR 	pItem, 
	const FTM_VOID_PTR 	pIndicator
);

static	FTM_LIST_PTR	pEventList = NULL;

FTM_RET	FTM_EVENT_init
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	if (pEventList != NULL)
	{
		ERROR("Event list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pEventList);	
	if (xRet != FTM_RET_OK)
	{
		ERROR("Event list is not allocated.\n");
		return	xRet;
	}

	FTM_LIST_setSeeker(pEventList, FTM_EVENT_seeker);

	TRACE("Event list is initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_final
(
	FTM_VOID
)
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

	FTM_LIST_destroy(pEventList);
	pEventList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_setDefault
(
	FTM_EVENT_PTR pEvent
)
{
	ASSERT(pEvent != NULL);

	memset(pEvent, 0, sizeof(FTM_EVENT));

	pEvent->xType = FTM_EVENT_TYPE_NONE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_create
(
	FTM_EVENT_PTR pEvent
)
{
	ASSERT(pEvent != NULL);

	FTM_RET			xRet;
	FTM_EVENT_PTR	pNew;
	struct timeval	xTime;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	pNew = (FTM_EVENT_PTR)FTM_MEM_malloc(sizeof(FTM_EVENT));
	if (pNew == NULL)
	{
		ERROR("Can't not allocation Event.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pNew, pEvent, sizeof(FTM_EVENT));

	xRet = FTM_LIST_append(pEventList, pNew);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNew);
		return	xRet;
	}

	if (strlen(pNew->pID) == 0)
	{
		gettimeofday(&xTime, NULL);
		sprintf(pNew->pID, "%08lx%08lx", (FTM_ULONG)xTime.tv_sec, (FTM_ULONG)xTime.tv_usec);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_createAbove
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR 	pEPID, 
	FTM_VALUE_PTR	pValue,
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_ABOVE, pID, pEPID, pValue, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createBelow
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue,
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_BELOW, pID, pEPID, pValue, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_createInclude
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID, 
	FTM_VALUE_PTR	pUpper, 
	FTM_VALUE_PTR	pLower, 
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_INCLUDE, pID, pEPID, pUpper, pLower, ppEvent);
}

FTM_RET	FTM_EVENT_createExcept
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID, 
	FTM_VALUE_PTR	pUpper, 
	FTM_VALUE_PTR	pLower, 
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_EXCEPT, pID, pEPID, pUpper, pLower, ppEvent);
}

FTM_RET	FTM_EVENT_createChange
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID, 
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	return	FTM_EVENT_create1(FTM_EVENT_TYPE_CHANGE, pID, pEPID, NULL, NULL, ppEvent);
}

FTM_RET	FTM_EVENT_create1
(
	FTM_EVENT_TYPE 	xType, 
	FTM_CHAR_PTR		pID,
	FTM_CHAR_PTR		pEPID,  
	FTM_VALUE_PTR		pValue1, 
	FTM_VALUE_PTR		pValue2, 
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	ASSERT(pEPID != NULL);
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

	if ((pID != NULL) && (strlen(pID) > FTM_ID_LEN))
	{
		return	FTM_RET_INVALID_ID;	
	}

	if ((pID == NULL) || (strlen(pID) == 0))
	{
		gettimeofday(&xTime, NULL);

		sprintf(xEvent.pID, "%08lx%08lx", xTime.tv_sec, xTime.tv_usec);
	}
	else
	{
		strcpy(xEvent.pID, pID);	
	}

	xEvent.xType = xType;
	
	switch(xType)
	{
	case	FTM_EVENT_TYPE_ABOVE:
	case	FTM_EVENT_TYPE_BELOW:
		{
			memcpy(&xEvent.xParams.xAbove.xValue, pValue1, sizeof(FTM_VALUE));
		}
		break;

	case	FTM_EVENT_TYPE_INCLUDE:
	case	FTM_EVENT_TYPE_EXCEPT:
		{
			memcpy(&xEvent.xParams.xInclude.xUpper, pValue1, sizeof(FTM_VALUE));
			memcpy(&xEvent.xParams.xInclude.xLower, pValue2, sizeof(FTM_VALUE));
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

FTM_RET	FTM_EVENT_count
(
	FTM_ULONG_PTR pulCount
)
{
	return	FTM_LIST_count(pEventList, pulCount);
}

FTM_RET	FTM_EVENT_get
(
	FTM_CHAR_PTR	pID,
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	ASSERT(pEventList != NULL);
	ASSERT(ppEvent != NULL);

	return	FTM_LIST_get(pEventList, pID, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTM_EVENT_getAt
(
	FTM_ULONG 	ulIndex, 
	FTM_EVENT_PTR _PTR_ ppEvent
)
{
	ASSERT(pEventList != NULL);
	ASSERT(ppEvent != NULL);

	return	FTM_LIST_getAt(pEventList, ulIndex, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTM_EVENT_destroy
(
	FTM_EVENT_PTR 	pEvent
)
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

FTM_RET	FTM_EVENT_append
(
	FTM_EVENT_PTR 	pEvent
)
{
	ASSERT(pEvent != NULL);

	FTM_RET			xRet;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	xRet = FTM_LIST_append(pEventList, pEvent);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EVENT_remove
(
	FTM_EVENT_PTR 	pEvent
)
{
	ASSERT(pEventList != NULL);
	ASSERT(pEvent != NULL);

	return	FTM_LIST_remove(pEventList, pEvent);
}

FTM_RET	FTM_EVENT_occurred
(
	FTM_EVENT_PTR 	pEvent, 
	FTM_VALUE_PTR 		pValue, 
	FTM_BOOL_PTR 		pResult
)
{
	ASSERT(pEvent != NULL);
	ASSERT(pValue != NULL);

	FTM_RET	xRet;
	FTM_INT	nResult;

	if (pEventList == NULL)
	{
		ERROR("Event list is not initialized.\n");
		FTM_EVENT_init();
	}

	switch(pEvent->xType)
	{
	case	FTM_EVENT_TYPE_NONE:
	case	FTM_EVENT_TYPE_UNKNOWN:
		{
		}
		break;

	case	FTM_EVENT_TYPE_ABOVE:
		{
			xRet = FTM_VALUE_compare(&pEvent->xParams.xAbove.xValue, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult <= 0);
			}
		}
		break;

	case	FTM_EVENT_TYPE_BELOW:
		{
			xRet = FTM_VALUE_compare(&pEvent->xParams.xBelow.xValue, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult >= 0);
			}
		}
		break;

	case	FTM_EVENT_TYPE_INCLUDE:
		{
			xRet = FTM_VALUE_compare(&pEvent->xParams.xInclude.xUpper, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult >= 0);
				if (*pResult != FTM_FALSE)
				{
					xRet = FTM_VALUE_compare(&pEvent->xParams.xInclude.xLower, pValue, &nResult);
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
			xRet = FTM_VALUE_compare(&pEvent->xParams.xExcept.xUpper, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult < 0);
				if (*pResult != FTM_TRUE)
				{
					xRet = FTM_VALUE_compare(&pEvent->xParams.xExcept.xLower, pValue, &nResult);
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
			xRet = FTM_VALUE_compare(&pEvent->xParams.xAbove.xValue, pValue, &nResult);
			*pResult = (nResult == 0);
		}
		break;

	}

	return	xRet;
}

struct	
{
	FTM_EVENT_TYPE	xType;
	FTM_CHAR_PTR		pName;
} pTypeString[] =
{
	{	.xType = FTM_EVENT_TYPE_NONE, 	.pName = "NONE"		},
	{	.xType = FTM_EVENT_TYPE_ABOVE, 	.pName = "ABOVE"	},
	{	.xType = FTM_EVENT_TYPE_BELOW, 	.pName = "BELOW"	},
	{	.xType = FTM_EVENT_TYPE_INCLUDE, 	.pName = "INCLUDE"	},
	{	.xType = FTM_EVENT_TYPE_EXCEPT, 	.pName = "EXCEPT"	},
	{	.xType = FTM_EVENT_TYPE_CHANGE,	.pName = "CHANGE"	},
	{	.xType = FTM_EVENT_TYPE_UNKNOWN, 	.pName = "UNKNOWN"	}
};

FTM_RET	FTM_EVENT_strToType
(
	FTM_CHAR_PTR	pString,
	FTM_EVENT_TYPE_PTR pType
)
{
	FTM_INT	i;

	for(i = 0; pTypeString[i].xType != FTM_EVENT_TYPE_UNKNOWN ; i++)
	{
		if (strcasecmp(pString, pTypeString[i].pName) == 0)
		{
			*pType = pTypeString[i].xType;	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_INVALID_TYPE;
}

FTM_CHAR_PTR	FTM_EVENT_typeString
(
	FTM_EVENT_TYPE 	xType
)
{
	FTM_INT	i;

	for(i = 0; pTypeString[i].xType != FTM_EVENT_TYPE_UNKNOWN ; i++)
	{
		if (pTypeString[i].xType == xType)
		{
			return	pTypeString[i].pName;	
		}
	}

	return	"UNKNOWN";
}

FTM_RET	FTM_EVENT_conditionToString
(
	FTM_EVENT_PTR 	pEvent, 
	FTM_CHAR_PTR 		pBuff, 
	FTM_ULONG 			ulBuffLen
)
{
	ASSERT(pEvent != NULL);
	ASSERT(pBuff != NULL);

	FTM_ULONG	ulLen = 0;
	FTM_CHAR	pTemp[256];

	switch(pEvent->xType)
	{   
	case    FTM_EVENT_TYPE_ABOVE:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xAbove.xValue);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "x >= %s", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_BELOW:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xBelow.xValue);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "x <= %s", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_INCLUDE:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xInclude.xLower);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%s <= x <= ", pTemp);

			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xInclude.xUpper);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%s)", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_EXCEPT:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xExcept.xLower);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "(x < %s) && ", pTemp);

			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pEvent->xParams.xExcept.xUpper);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "(%s < x)", pTemp);
		}   
		break;

	case    FTM_EVENT_TYPE_CHANGE:
		{   
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "CHANGE");
		}   
		break;

	default:
		{   
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "UNKNOWN");
		}
	}

	return	FTM_RET_OK;

}

FTM_BOOL	FTM_EVENT_seeker
(
	const FTM_VOID_PTR 	pItem, 
	const FTM_VOID_PTR 	pIndicator
)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_EVENT_PTR		pEvent = (FTM_EVENT_PTR)pItem;
	FTM_CHAR_PTR		pID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pEvent->pID, pID) == 0;
}
