#include <unistd.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_mem.h"
#include "ftm_msg_queue.h"
#include "ftdm_config.h"
#include "ftdm_trigger.h"

FTM_RET	FTDM_TRIGGER_init
(
	FTM_VOID
)
{
	return	FTM_TRIGGER_init();
}

FTM_RET FTDM_TRIGGER_final
(
	FTM_VOID
)
{
	return	FTM_TRIGGER_final();
}

FTM_RET	FTDM_TRIGGER_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xTrigger;
	FTM_CONFIG_ITEM		xTriggers;
	FTM_CONFIG_ITEM		xTriggerItem;
	FTM_CONFIG_ITEM		xRules;
	FTM_CONFIG_ITEM		xRuleItem;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger configuration load failed.\n");

		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "event", &xTrigger);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xTrigger, "triggers", &xTriggers);
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
						FTM_TRIGGER		xTrigger;
						FTM_TRIGGER_PTR	pTrigger;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "id", (FTM_INT_PTR)&xTrigger.xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "epid", (FTM_INT_PTR)&xTrigger.xEPID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "type", (FTM_INT_PTR)&xTrigger.xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "detect",&xTrigger.xDetectionTime);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "hold",&xTrigger.xHoldingTime);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xTrigger.xType)
						{
						case	FTM_TRIGGER_TYPE_ABOVE:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "value", &xTrigger.xParams.xAbove.xValue);
							}
							break;

						case	FTM_TRIGGER_TYPE_BELOW:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "value", &xTrigger.xParams.xBelow.xValue);
							}
							break;

						case	FTM_TRIGGER_TYPE_INCLUDE:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "upper", &xTrigger.xParams.xInclude.xUpper);
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "lower", &xTrigger.xParams.xInclude.xLower);
							}
							break;

						case	FTM_TRIGGER_TYPE_EXCEPT:
							{
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "upper", &xTrigger.xParams.xExcept.xUpper);
								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "lower", &xTrigger.xParams.xExcept.xLower);
							}
							break;

						case	FTM_TRIGGER_TYPE_CHANGE:
							{
							}
							break;
						}

						xRet = FTM_TRIGGER_createCopy(&xTrigger, &pTrigger);
						if (xRet != FTM_RET_OK)
						{
							ERROR("The new event can not creation.\n");
						}
					
					}
				}
			}
		}

		xRet = FTM_CONFIG_ITEM_getChildItem(&xTrigger, "rules", &xRules);
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

FTM_RET	FTDM_TRIGGER_loadFromDB
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_add
(
	FTM_TRIGGER_PTR 	pTrigger
)
{
	return	FTM_TRIGGER_createCopy(pTrigger, NULL);
}

FTM_RET	FTDM_TRIGGER_del
(
	FTM_TRIGGER_ID	xID
)
{
	FTM_RET			xRet;
	FTM_TRIGGER_PTR	pTrigger = NULL;

	xRet = FTM_TRIGGER_get(xID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_TRIGGER_destroy(pTrigger);
}

FTM_RET	FTDM_TRIGGER_count
(
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_TRIGGER_count(pulCount);
}

FTM_RET	FTDM_TRIGGER_get
(
	FTM_TRIGGER_ID	xID,
	FTM_TRIGGER_PTR	_PTR_ 	ppTrigger
)
{
	return	FTM_TRIGGER_get(xID, ppTrigger);
}

FTM_RET	FTDM_TRIGGER_getAt
(
	FTM_ULONG				nIndex,
	FTM_TRIGGER_PTR	_PTR_ 	ppTrigger
)
{
	return	FTM_TRIGGER_getAt(nIndex, ppTrigger);
}

FTM_RET	FTDM_TRIGGER_showList
(
	FTM_VOID
)
{
	FTM_TRIGGER_PTR	pTrigger;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# Trigger Information\n");
	MESSAGE("\t%4s %8s %8s %8s %s\n", "ID", "EPID", "DETECT", "HOLD", "CONDITION");

	FTM_TRIGGER_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_TRIGGER_getAt(i, &pTrigger) == FTM_RET_OK)
		{
			FTM_CHAR	pBuff[1024];

			MESSAGE("\t%4d %08x ", pTrigger->xID, pTrigger->xEPID);
			MESSAGE("%8.1f ", (pTrigger->xDetectionTime.xTimeval.tv_sec * 1000000 +  pTrigger->xDetectionTime.xTimeval.tv_usec) / 1000000.0);
			MESSAGE("%8.1f ", (pTrigger->xHoldingTime.xTimeval.tv_sec * 1000000 +  pTrigger->xHoldingTime.xTimeval.tv_usec) / 1000000.0); 

			switch(pTrigger->xType)
			{
			case	FTM_TRIGGER_TYPE_ABOVE:
				{
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xParams.xAbove.xValue);
					MESSAGE("x >= %s ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_BELOW:
				{
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xParams.xBelow.xValue);
					MESSAGE("x <= %s ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_INCLUDE:
				{
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xParams.xInclude.xLower);
					MESSAGE("(%s <= x <= ", pBuff);

					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xParams.xInclude.xUpper);
					MESSAGE("%s) ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_EXCEPT:
				{
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xParams.xExcept.xLower);
					MESSAGE("(x < %s) && ", pBuff);

					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xParams.xExcept.xUpper);
					MESSAGE("(%s < x) ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_CHANGE:
				{
				}
				break;
			}
			MESSAGE("\n");
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_TRIGGER_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_TRIGGER_PTR		pTrigger = (FTM_TRIGGER_PTR)pElement;
	FTM_TRIGGER_ID_PTR	pTriggerID = (FTM_TRIGGER_ID_PTR)pIndicator;

	return	(pTrigger->xID == *pTriggerID);
}
