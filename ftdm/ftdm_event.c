#include <unistd.h>
#include "ftm_error.h"
#include "ftm_config.h"
#include "ftdm_config.h"
#include "ftdm_event.h"

static FTM_LIST_PTR	pEventList = NULL;

FTM_RET	FTDM_EVENT_init
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	if (pEventList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pEventList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_EVENT_final
(
	FTM_VOID
)
{
	FTM_RET			xRet;
	FTM_VOID_PTR	pEvent;

	if (pEventList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_LIST_iteratorStart(pEventList);
	while(FTM_LIST_iteratorNext(pEventList, &pEvent) == FTM_RET_OK)
	{
		FTM_EVENT_destroy(pEvent);			
	}
	
	xRet = FTM_LIST_destroy(pEventList);
	if (xRet == FTM_RET_OK)
	{
		pEventList = NULL;
	}

	return	xRet;
}

FTM_RET	FTDM_EVENT_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xTrigger;
	FTM_CONFIG_ITEM		xEvents;
	FTM_CONFIG_ITEM		xEventItem;
	FTM_CONFIG_ITEM		xIDItem;
	FTM_CONFIG_ITEM		xTypeItem;
	FTM_CONFIG_ITEM		xValueItem;

	if (pEventList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "trigger", &xTrigger);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xTrigger, "events", &xEvents);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xEvents, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xEvents, i, &xEventItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_EVENT		xEvent;
						FTM_EVENT_PTR	pEvent;

						xRet = FTM_CONFIG_getInt(&xEventItem, "id", &xEvent.xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_getInt(&xEventItem, "type", &xEvent.xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xEvent.xType)
						{
						case	FTM_EVENT_TYPE_ABOVE:
							{
								xRet = FTM_CONFIG_getData(&xEventItem, "value", &xEvent.xParams.xAbove.xValue);
							}
							break;

						case	FTM_EVENT_TYPE_BELOW:
							{
								xRet = FTM_CONFIG_getData(&xEventItem, "value", &xEvent.xParams.xBelow.xValue);
							}
							break;

						case	FTM_EVENT_TYPE_INCLUDE:
							{
								xRet = FTM_CONFIG_getData(&xEventItem, "upper", &xEvent.xParams.xInclude.xUpper.xValue);
								xRet = FTM_CONFIG_getData(&xEventItem, "lower", &xEvent.xParams.xInclude.xLower.xValue);
							}
							break;

						case	FTM_EVENT_TYPE_EXCEPT:
							{
								xRet = FTM_CONFIG_getData(&xEventItem, "upper", &xEvent.xParams.xInclude.xExcept.xValue);
								xRet = FTM_CONFIG_getData(&xEventItem, "lower", &xEvent.xParams.xInclude.xExcept.xValue);
							}
							break;
						}

						
						xRet = FTM_EVENT_createCopy(&xEvent, &pEvent);
						if (xRet == FTM_RET_OK)
						{
							FTM_LIST_append(pEventList, pEvent);
						}
					
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
	FTM_EVENT_PTR 	pInfo
)
{
	
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_del
(
	FTM_EVENT_ID	xID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_count
(
	FTM_ULONG_PTR		pnCount
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_get
(
	FTM_EVENT_ID	xID,
	FTM_EVENT_PTR	_PTR_ 	ppEvent
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_getAt
(
	FTM_ULONG				nIndex,
	FTM_EVENT_PTR	_PTR_ 	ppEvent
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_EVENT_showList
(
	FTM_VOID
)
{
	FTM_EVENT_PTR	pEvent;

	if (pEventList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}


	MESSAGE("# TRIGGER INFORMATION\n");
	MESSAGE("%8s %s ", "ID", "CONDITION");
	FTM_LIST_iteratorStart(pEventList);
	while(FTM_LIST_iteratorNext(pEventList, (FTM_VOID_PTR _PTR_)&pEvent) == FTM_RET_OK)
	{
		FTM_CHAR	pBuff[1024];

		MESSAGE("%08x ", pEvent->xID);
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

		case	FTM_EVENT_TYPE_AND:
			{
				MESSAGE("%d and %d", 
					pEvent->xParams.xAnd.xID1,
					pEvent->xParams.xAnd.xID2);
			}
			break;

		case	FTM_EVENT_TYPE_OR:
			{
				MESSAGE("%d or %d", 
					pEvent->xParams.xAnd.xID1,
					pEvent->xParams.xAnd.xID2);
			}
			break;
		}

		MESSAGE("\n");
	}
	MESSAGE("\n");

	return	FTM_RET_OK;
}

