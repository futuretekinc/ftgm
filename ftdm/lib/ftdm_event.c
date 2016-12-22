#include <unistd.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_mem.h"
#include "ftm_msg_queue.h"
#include "ftdm_config.h"
#include "ftdm_event.h"
#include "ftdm_dbif.h"
#include "ftdm_log.h"

FTM_RET	FTDM_EVENT_init
(
	FTM_VOID
)
{
	return	FTM_EVENT_init();
}

FTM_RET FTDM_EVENT_final
(
	FTM_VOID
)
{
	return	FTM_EVENT_final();
}


FTM_RET	FTDM_EVENT_count
(
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_EVENT_count(pulCount);
}

FTM_RET	FTDM_EVENT_get
(
	FTM_CHAR_PTR	pID,
	FTDM_EVENT_PTR	_PTR_ 	ppEvent
)
{
	return	FTM_EVENT_get(pID, (FTM_EVENT_PTR _PTR_)ppEvent);
}

FTM_RET	FTDM_EVENT_getAt
(
	FTM_ULONG				nIndex,
	FTDM_EVENT_PTR _PTR_ 	ppEvent
)
{
	return	FTM_EVENT_getAt(nIndex, (FTM_EVENT_PTR _PTR_)ppEvent);
}

FTM_RET	FTDM_EVENT_getByIndex
(
	FTM_ULONG				ulIndex,
	FTDM_EVENT_PTR _PTR_ 	ppEvent
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_INT		i;

	xRet = FTDM_EVENT_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i  < ulCount ; i++)
	{
		xRet = FTDM_EVENT_getAt(i, ppEvent);
		if (xRet == FTM_RET_OK)
		{
			return	xRet;	
		}

		if ((*ppEvent)->ulIndex == ulIndex)
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTDM_EVENT_set
(
	FTM_CHAR_PTR		pID,
	FTM_EVENT_FIELD	xFields,
	FTM_EVENT_PTR		pInfo
)
{
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_EVENT_PTR	pEvent;

	xRet = FTM_EVENT_get(pID, (FTM_EVENT_PTR _PTR_)&pEvent);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xFields & FTM_EVENT_FIELD_NAME)
	{
		strcpy(pEvent->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_EVENT_FIELD_EPID)
	{
		strcpy(pEvent->xInfo.pEPID, pInfo->pEPID);
	}

	if (xFields & FTM_EVENT_FIELD_DETECT_TIME)
	{
		pEvent->xInfo.xParams.xCommon.ulDetectionTime = pInfo->xParams.xCommon.ulDetectionTime;
	}

	if (xFields & FTM_EVENT_FIELD_HOLD_TIME)
	{
		pEvent->xInfo.xParams.xCommon.ulHoldingTime = pInfo->xParams.xCommon.ulHoldingTime;
	}

	if (xFields & FTM_EVENT_FIELD_VALUE)
	{
		memcpy(&pEvent->xInfo.xParams.xAbove.xValue, &pInfo->xParams.xAbove.xValue, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_EVENT_FIELD_LOWER)
	{
		memcpy(&pEvent->xInfo.xParams.xInclude.xLower, &pInfo->xParams.xInclude.xLower, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_EVENT_FIELD_UPPER)
	{
		memcpy(&pEvent->xInfo.xParams.xInclude.xUpper, &pInfo->xParams.xInclude.xUpper, sizeof(FTM_VALUE));
	}

	FTDM_DBIF_setEvent(pEvent->pDBIF, pID, &pEvent->xInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	i, ulCount;
	FTDM_EVENT_PTR	pEvent;

	xRet = FTDM_EVENT_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		xRet = FTDM_EVENT_getAt(ulIndex + i, &pEvent);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		strcpy(pIDs[(*pulCount)++], pEvent->xInfo.pID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_showList
(
	FTM_VOID
)
{
	FTDM_EVENT_PTR	pEvent;
	FTM_ULONG			i, ulCount;
	MESSAGE("\n# Event Information\n");
	MESSAGE("\t%16s %16s %8s %16s %16s %8s %s\n", 
		"ID", "NAME", "STATE", "EPID", "DETECT", "HOLD", "CONDITION");

	FTM_EVENT_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTDM_EVENT_getAt(i, &pEvent) == FTM_RET_OK)
		{
			MESSAGE("\t%16s %16s %8s %16s ", 
				pEvent->xInfo.pID, 
				pEvent->xInfo.pName, 
				"Running",
				pEvent->xInfo.pEPID);

			switch(pEvent->xInfo.xType)
			{
			case	FTM_EVENT_TYPE_ABOVE:
				{
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xAbove.ulDetectionTime) / 1000000.0);
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xAbove.ulHoldingTime) / 1000000.0); 
					MESSAGE("x >= %s ", FTM_VALUE_print(&pEvent->xInfo.xParams.xAbove.xValue));
				}
				break;

			case	FTM_EVENT_TYPE_BELOW:
				{
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xBelow.ulDetectionTime) / 1000000.0);
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xBelow.ulHoldingTime) / 1000000.0); 
					MESSAGE("x <= %s ", FTM_VALUE_print(&pEvent->xInfo.xParams.xBelow.xValue));
				}
				break;

			case	FTM_EVENT_TYPE_INCLUDE:
				{
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xInclude.ulDetectionTime) / 1000000.0);
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xInclude.ulHoldingTime) / 1000000.0); 
					MESSAGE("(%s <= x <= ", FTM_VALUE_print(&pEvent->xInfo.xParams.xInclude.xLower));
					MESSAGE("%s) ", FTM_VALUE_print(&pEvent->xInfo.xParams.xInclude.xUpper));
				}
				break;

			case	FTM_EVENT_TYPE_EXCEPT:
				{
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xExcept.ulDetectionTime) / 1000000.0);
					MESSAGE("%16.1f ", (pEvent->xInfo.xParams.xExcept.ulHoldingTime) / 1000000.0); 
					MESSAGE("(x < %s) && ", FTM_VALUE_print(&pEvent->xInfo.xParams.xExcept.xLower));
					MESSAGE("(%s < x) ", FTM_VALUE_print(&pEvent->xInfo.xParams.xExcept.xUpper));
				}
				break;

			case	FTM_EVENT_TYPE_CHANGE:
			default:
				{
				}
				break;
			}
			MESSAGE("\n");
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_EVENT_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_EVENT_PTR		pEvent = (FTM_EVENT_PTR)pElement;
	FTM_CHAR_PTR		pEventID = (FTM_CHAR_PTR)pIndicator;

	return	strcpy(pEvent->pID,pEventID) == 0;
}
