#include <sys/time.h>
#include <string.h>
#include "ftm_trigger.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_mem.h"

static	
FTM_RET	FTM_TRIGGER_create1
(
	FTM_TRIGGER_TYPE 	xType, 
	FTM_TRIGGER_ID 		xTriggerID, 
	FTM_CHAR_PTR		pEPID,
	FTM_VALUE_PTR		pValue1, 
	FTM_VALUE_PTR		pValue2, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
);

static 	
FTM_BOOL FTM_TRIGGER_seeker
(
	const FTM_VOID_PTR 	pItem, 
	const FTM_VOID_PTR 	pIndicator
);

static	FTM_LIST_PTR	pTriggerList = NULL;

FTM_RET	FTM_TRIGGER_init
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	if (pTriggerList != NULL)
	{
		ERROR("Trigger list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pTriggerList);	
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list is not allocated.\n");
		return	xRet;
	}

	FTM_LIST_setSeeker(pTriggerList, FTM_TRIGGER_seeker);

	TRACE("Trigger list is initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_TRIGGER_final
(
	FTM_VOID
)
{
	FTM_TRIGGER_PTR	pTrigger;

	if (pTriggerList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

 	FTM_LIST_iteratorStart(pTriggerList);
  	while(FTM_LIST_iteratorNext(pTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
   	{   
    	FTM_TRIGGER_destroy(pTrigger);
	}

	FTM_LIST_destroy(pTriggerList);
	pTriggerList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRIGGER_setDefault
(
	FTM_TRIGGER_PTR pTrigger
)
{
	ASSERT(pTrigger != NULL);

	memset(pTrigger, 0, sizeof(FTM_TRIGGER));

	pTrigger->xType = FTM_TRIGGER_TYPE_NONE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRIGGER_create
(
	FTM_TRIGGER_PTR pTrigger
)
{
	ASSERT(pTrigger != NULL);

	FTM_RET			xRet;
	FTM_TRIGGER_PTR	pNew;
	struct timeval	xTime;

	if (pTriggerList == NULL)
	{
		ERROR("Trigger list is not initialized.\n");
		FTM_TRIGGER_init();
	}

	pNew = (FTM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTM_TRIGGER));
	if (pNew == NULL)
	{
		ERROR("Can't not allocation Trigger.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pNew, pTrigger, sizeof(FTM_TRIGGER));

	xRet = FTM_LIST_append(pTriggerList, pNew);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNew);
		return	xRet;
	}

	if (pNew->xID == 0)
	{
		gettimeofday(&xTime, NULL);
		pNew->xID = xTime.tv_sec * 1000000 + xTime.tv_usec;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRIGGER_createAbove
(
	FTM_TRIGGER_ID 	xTriggerID, 
	FTM_CHAR_PTR 	pEPID, 
	FTM_VALUE_PTR	pValue,
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	return	FTM_TRIGGER_create1(FTM_TRIGGER_TYPE_ABOVE, 
				xTriggerID, pEPID, pValue, NULL, ppTrigger);
}

FTM_RET	FTM_TRIGGER_createBelow
(
	FTM_TRIGGER_ID 	xTriggerID, 
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue,
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	return	FTM_TRIGGER_create1(FTM_TRIGGER_TYPE_BELOW, 
				xTriggerID, pEPID, pValue, NULL, ppTrigger);
}

FTM_RET	FTM_TRIGGER_createInclude
(
	FTM_TRIGGER_ID 	xTriggerID, 
	FTM_CHAR_PTR	pEPID, 
	FTM_VALUE_PTR	pUpper, 
	FTM_VALUE_PTR	pLower, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	return	FTM_TRIGGER_create1(FTM_TRIGGER_TYPE_INCLUDE, 
					xTriggerID, pEPID, pUpper, pLower, ppTrigger);
}

FTM_RET	FTM_TRIGGER_createExcept
(
	FTM_TRIGGER_ID 	xTriggerID, 
	FTM_CHAR_PTR	pEPID, 
	FTM_VALUE_PTR	pUpper, 
	FTM_VALUE_PTR	pLower, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	return	FTM_TRIGGER_create1(FTM_TRIGGER_TYPE_EXCEPT, 
					xTriggerID, pEPID, pUpper, pLower, ppTrigger);
}

FTM_RET	FTM_TRIGGER_createChange
(
	FTM_TRIGGER_ID 	xTriggerID, 
	FTM_CHAR_PTR	pEPID, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	return	FTM_TRIGGER_create1(FTM_TRIGGER_TYPE_CHANGE, 
				xTriggerID, pEPID, NULL, NULL, ppTrigger);
}

FTM_RET	FTM_TRIGGER_create1
(
	FTM_TRIGGER_TYPE 	xType, 
	FTM_TRIGGER_ID 		xTriggerID, 
	FTM_CHAR_PTR		pEPID,  
	FTM_VALUE_PTR		pValue1, 
	FTM_VALUE_PTR		pValue2, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pTriggerList != NULL);

	FTM_RET			xRet;
	FTM_TRIGGER		xTrigger;
	FTM_TRIGGER_PTR	pTrigger;
	struct timeval	xTime;

	if (pTriggerList == NULL)
	{
		ERROR("Trigger list is not initialized.\n");
		FTM_TRIGGER_init();
	}

	if (xTriggerID == 0)
	{
		gettimeofday(&xTime, NULL);
	
		xTrigger.xID = xTime.tv_sec * 1000000 + xTime.tv_usec;
	}
	else
	{
		xTrigger.xID = xTriggerID;	
	}
	xTrigger.xType = xType;
	
	switch(xType)
	{
	case	FTM_TRIGGER_TYPE_ABOVE:
	case	FTM_TRIGGER_TYPE_BELOW:
		{
			memcpy(&xTrigger.xParams.xAbove.xValue, pValue1, sizeof(FTM_VALUE));
		}
		break;

	case	FTM_TRIGGER_TYPE_INCLUDE:
	case	FTM_TRIGGER_TYPE_EXCEPT:
		{
			memcpy(&xTrigger.xParams.xInclude.xUpper, pValue1, sizeof(FTM_VALUE));
			memcpy(&xTrigger.xParams.xInclude.xLower, pValue2, sizeof(FTM_VALUE));
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	pTrigger = (FTM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTM_TRIGGER));
	if (pTrigger == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pTrigger, &xTrigger, sizeof(xTrigger));

	xRet = FTM_LIST_append(pTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);
		return	xRet;	
	}

	if (ppTrigger != NULL)
	{
		*ppTrigger = pTrigger;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRIGGER_count
(
	FTM_ULONG_PTR pulCount
)
{
	return	FTM_LIST_count(pTriggerList, pulCount);
}

FTM_RET	FTM_TRIGGER_get
(
	FTM_ULONG 	ulTriggerID, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerList != NULL);
	ASSERT(ppTrigger != NULL);

	return	FTM_LIST_get(pTriggerList, &ulTriggerID, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTM_TRIGGER_getAt
(
	FTM_ULONG 	ulIndex, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerList != NULL);
	ASSERT(ppTrigger != NULL);

	return	FTM_LIST_getAt(pTriggerList, ulIndex, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTM_TRIGGER_destroy
(
	FTM_TRIGGER_PTR 	pTrigger
)
{
	ASSERT(pTriggerList != NULL);
	ASSERT(pTrigger != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pTriggerList, pTrigger);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);	
	}

	return	xRet;
}

FTM_RET	FTM_TRIGGER_append
(
	FTM_TRIGGER_PTR 	pTrigger
)
{
	ASSERT(pTrigger != NULL);

	FTM_RET			xRet;

	if (pTriggerList == NULL)
	{
		ERROR("Trigger list is not initialized.\n");
		FTM_TRIGGER_init();
	}

	xRet = FTM_LIST_append(pTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRIGGER_remove
(
	FTM_TRIGGER_PTR 	pTrigger
)
{
	ASSERT(pTriggerList != NULL);
	ASSERT(pTrigger != NULL);

	return	FTM_LIST_remove(pTriggerList, pTrigger);
}

FTM_RET	FTM_TRIGGER_occurred
(
	FTM_TRIGGER_PTR 	pTrigger, 
	FTM_VALUE_PTR 		pValue, 
	FTM_BOOL_PTR 		pResult
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pValue != NULL);

	FTM_RET	xRet;
	FTM_INT	nResult;

	if (pTriggerList == NULL)
	{
		ERROR("Trigger list is not initialized.\n");
		FTM_TRIGGER_init();
	}

	switch(pTrigger->xType)
	{
	case	FTM_TRIGGER_TYPE_NONE:
		{
		}
		break;

	case	FTM_TRIGGER_TYPE_ABOVE:
		{
			xRet = FTM_VALUE_compare(&pTrigger->xParams.xAbove.xValue, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult <= 0);
			}
		}
		break;

	case	FTM_TRIGGER_TYPE_BELOW:
		{
			xRet = FTM_VALUE_compare(&pTrigger->xParams.xBelow.xValue, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult >= 0);
			}
		}
		break;

	case	FTM_TRIGGER_TYPE_INCLUDE:
		{
			xRet = FTM_VALUE_compare(&pTrigger->xParams.xInclude.xUpper, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult >= 0);
				if (*pResult != FTM_FALSE)
				{
					xRet = FTM_VALUE_compare(&pTrigger->xParams.xInclude.xLower, pValue, &nResult);
					if (xRet == FTM_RET_OK)
					{
						*pResult = (nResult <= 0);
					}
				}
			}
		}
		break;

	case 	FTM_TRIGGER_TYPE_EXCEPT:
		{
			xRet = FTM_VALUE_compare(&pTrigger->xParams.xExcept.xUpper, pValue, &nResult);
			if (xRet == FTM_RET_OK)
			{
				*pResult = (nResult < 0);
				if (*pResult != FTM_TRUE)
				{
					xRet = FTM_VALUE_compare(&pTrigger->xParams.xExcept.xLower, pValue, &nResult);
					if (xRet == FTM_RET_OK)
					{
						*pResult = (nResult > 0);
					}
				}
			}
		}
		break;

	case	FTM_TRIGGER_TYPE_CHANGE:
		{
			xRet = FTM_VALUE_compare(&pTrigger->xParams.xAbove.xValue, pValue, &nResult);
			*pResult = (nResult == 0);
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
	"CHANGE"
};

FTM_CHAR_PTR	FTM_TRIGGER_typeString
(
	FTM_TRIGGER_TYPE 	xType
)
{
	if (xType < sizeof(pTypeString) / sizeof(FTM_CHAR_PTR))
	{
		return	pTypeString[xType];
	}

	return	"UNKNOWN";
}

FTM_RET	FTM_TRIGGER_conditionToString
(
	FTM_TRIGGER_PTR 	pTrigger, 
	FTM_CHAR_PTR 		pBuff, 
	FTM_ULONG 			ulBuffLen
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pBuff != NULL);

	FTM_ULONG	ulLen = 0;
	FTM_CHAR	pTemp[256];

	switch(pTrigger->xType)
	{   
	case    FTM_TRIGGER_TYPE_ABOVE:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pTrigger->xParams.xAbove.xValue);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "x >= %s", pTemp);
		}   
		break;

	case    FTM_TRIGGER_TYPE_BELOW:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pTrigger->xParams.xBelow.xValue);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "x <= %s", pTemp);
		}   
		break;

	case    FTM_TRIGGER_TYPE_INCLUDE:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pTrigger->xParams.xInclude.xLower);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%s <= x <= ", pTemp);

			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pTrigger->xParams.xInclude.xUpper);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "%s)", pTemp);
		}   
		break;

	case    FTM_TRIGGER_TYPE_EXCEPT:
		{   
			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pTrigger->xParams.xExcept.xLower);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "(x < %s) && ", pTemp);

			FTM_VALUE_snprint(pTemp, sizeof(pTemp) - 1, &pTrigger->xParams.xExcept.xUpper);
			ulLen += snprintf(&pBuff[ulLen], ulBuffLen - ulLen, "(%s < x)", pTemp);
		}   
		break;

	case    FTM_TRIGGER_TYPE_CHANGE:
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

FTM_BOOL	FTM_TRIGGER_seeker
(
	const FTM_VOID_PTR 	pItem, 
	const FTM_VOID_PTR 	pIndicator
)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_TRIGGER_PTR		pTrigger = (FTM_TRIGGER_PTR)pItem;
	FTM_TRIGGER_ID_PTR	pTriggerID = (FTM_TRIGGER_ID_PTR)pIndicator;

	return	(pTrigger->xID == *pTriggerID);
}
