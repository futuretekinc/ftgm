#include <unistd.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_mem.h"
#include "ftm_msg_queue.h"
#include "ftdm_config.h"
#include "ftdm_event.h"

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

FTM_RET	FTDM_EVENT_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xEvent;
	FTM_CONFIG_ITEM		xTriggers;
	FTM_CONFIG_ITEM		xTriggerItem;
	FTM_CONFIG_ITEM		xActions;
	FTM_CONFIG_ITEM		xActionItem;
	FTM_CONFIG_ITEM		xRules;
	FTM_CONFIG_ITEM		xRuleItem;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "event", &xEvent);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xEvent, "triggers", &xTriggers);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xTriggers, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xTriggers, i, &xTriggerItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_EVENT		xEvent;
						FTM_EVENT_PTR	pEvent;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "id", (FTM_INT_PTR)&xEvent.xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "epid", (FTM_INT_PTR)&xEvent.xEPID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "type", (FTM_INT_PTR)&xEvent.xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "detect",&xEvent.xDetectionTime);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "hold",&xEvent.xHoldingTime);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xEvent.xType)
						{
						case	FTM_EVENT_TYPE_ABOVE:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "value", &xEvent.xParams.xAbove.xValue);
							}
							break;

						case	FTM_EVENT_TYPE_BELOW:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "value", &xEvent.xParams.xBelow.xValue);
							}
							break;

						case	FTM_EVENT_TYPE_INCLUDE:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "upper", &xEvent.xParams.xInclude.xUpper);
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "lower", &xEvent.xParams.xInclude.xLower);
							}
							break;

						case	FTM_EVENT_TYPE_EXCEPT:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "upper", &xEvent.xParams.xExcept.xUpper);
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "lower", &xEvent.xParams.xExcept.xLower);
							}
							break;

						case	FTM_EVENT_TYPE_CHANGE:
							{
							}
							break;
						}

						xRet = FTM_EVENT_createCopy(&xEvent, &pEvent);
						if (xRet != FTM_RET_OK)
						{
							ERROR("The new event can not creation.\n");
						}
					
					}
				}
			}
		}

		xRet = FTM_CONFIG_ITEM_getChildItem(&xEvent, "actions", &xActions);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xActions, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xActions, i, &xActionItem);	
					if (xRet == FTM_RET_OK)
					{
					
					}
				}
			}
		}

		xRet = FTM_CONFIG_ITEM_getChildItem(&xEvent, "rules", &xRules);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xRules, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xRules, i, &xRuleItem);	
					if (xRet == FTM_RET_OK)
					{
					
					}
				}
			}
		}
	}
	else
	{
		TRACE("Section [trigger] not found.\n");
	}


	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_loadFromDB
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_add
(
	FTM_EVENT_PTR 	pEvent
)
{
	return	FTM_EVENT_createCopy(pEvent, NULL);
}

FTM_RET	FTDM_EVENT_del
(
	FTM_EVENT_ID	xID
)
{
	FTM_RET			xRet;
	FTM_EVENT_PTR	pEvent = NULL;

	xRet = FTM_EVENT_get(xID, &pEvent);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_EVENT_destroy(pEvent);
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
	FTM_EVENT_ID	xID,
	FTM_EVENT_PTR	_PTR_ 	ppEvent
)
{
	return	FTM_EVENT_get(xID, ppEvent);
}

FTM_RET	FTDM_EVENT_getAt
(
	FTM_ULONG				nIndex,
	FTM_EVENT_PTR	_PTR_ 	ppEvent
)
{
	return	FTM_EVENT_getAt(nIndex, ppEvent);
}

FTM_RET	FTDM_EVENT_showList
(
	FTM_VOID
)
{
	FTM_EVENT_PTR	pEvent;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# TRIGGER INFORMATION\n");
	MESSAGE("\t%-8s %-8s %-8s %-8s %s\n", "ID", "EPID", "DETECT", "HOLD", "CONDITION");

	FTM_EVENT_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_EVENT_getAt(i, &pEvent) == FTM_RET_OK)
		{
			FTM_CHAR	pBuff[1024];

			MESSAGE("\t%08x %08x ", pEvent->xID, pEvent->xEPID);
			MESSAGE("%8.3f ", (pEvent->xDetectionTime.xTimeval.tv_sec * 1000000 +  pEvent->xDetectionTime.xTimeval.tv_usec) / 1000000.0);
			MESSAGE("%8.3f ", (pEvent->xHoldingTime.xTimeval.tv_sec * 1000000 +  pEvent->xHoldingTime.xTimeval.tv_usec) / 1000000.0); 

			switch(pEvent->xType)
			{
			case	FTM_EVENT_TYPE_ABOVE:
				{
					FTM_EP_DATA_snprint(pBuff, 1023, &pEvent->xParams.xAbove.xValue);
					MESSAGE("x >= %s ", pBuff);
				}
				break;

			case	FTM_EVENT_TYPE_BELOW:
				{
					FTM_EP_DATA_snprint(pBuff, 1023, &pEvent->xParams.xBelow.xValue);
					MESSAGE("x <= %s ", pBuff);
				}
				break;

			case	FTM_EVENT_TYPE_INCLUDE:
				{
					FTM_EP_DATA_snprint(pBuff, 1023, &pEvent->xParams.xInclude.xLower);
					MESSAGE("(%s <= x <= ", pBuff);

					FTM_EP_DATA_snprint(pBuff, 1023, &pEvent->xParams.xInclude.xUpper);
					MESSAGE("%s) ", pBuff);
				}
				break;

			case	FTM_EVENT_TYPE_EXCEPT:
				{
					FTM_EP_DATA_snprint(pBuff, 1023, &pEvent->xParams.xExcept.xLower);
					MESSAGE("(x < %s) && ", pBuff);

					FTM_EP_DATA_snprint(pBuff, 1023, &pEvent->xParams.xExcept.xUpper);
					MESSAGE("(%s < x) ", pBuff);
				}
				break;

			case	FTM_EVENT_TYPE_CHANGE:
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
	FTM_EVENT_ID_PTR	pEventID = (FTM_EVENT_ID_PTR)pIndicator;

	return	(pEvent->xID == *pEventID);
}
