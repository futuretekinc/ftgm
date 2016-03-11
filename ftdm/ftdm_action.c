#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_config.h"
#include "ftdm_action.h"

FTM_RET	FTDM_ACTION_init
(
	FTM_VOID
)
{
	return	FTM_ACTION_init();
}

FTM_RET FTDM_ACTION_final
(
	FTM_VOID
)
{
	return	FTM_ACTION_final();
}

FTM_RET	FTDM_ACTION_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xAction;
	FTM_CONFIG_ITEM		xActions;
	FTM_CONFIG_ITEM		xActionItem;
	FTM_CONFIG_ITEM		xParamsItem;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "event", &xAction);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xAction, "actions", &xActions);
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
						FTM_ACTION_ID	xID;
						FTM_ACTION_TYPE	xType;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xActionItem, "id", (FTM_INT_PTR)&xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_ITEM_getItemINT(&xActionItem, "type", (FTM_INT_PTR)&xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xType)
						{
						case	FTM_ACTION_TYPE_SET:
							{
								FTM_EP_ID	xEPID;
								FTM_EP_DATA	xValue;

								xRet = FTM_CONFIG_ITEM_getChildItem(&xActionItem, "params", &xParamsItem);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

	
								xRet = FTM_CONFIG_ITEM_getItemINT(&xParamsItem, "epid", (FTM_INT_PTR)&xEPID);
								if (xRet != FTM_RET_OK)
								{
										continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xParamsItem, "value", &xValue);
								if (xRet != FTM_RET_OK)
								{
										continue;
								}

								xRet = FTM_ACTION_createSet(xID, xType, xEPID, &xValue, NULL);
								if (xRet != FTM_RET_OK)
								{
										ERROR("The new event can not creation.\n");
								}
							}
							break;

						default:
							{
								ERROR("ACtion type[%08x] is not supported.\n", xType);
								continue;
							}
						}
					
					}
				}
			}
		}
	}
	else
	{
		TRACE("Section [action] not found.\n");
	}


	FTM_CONFIG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_loadFromDB
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_add
(
	FTM_ACTION_PTR 	pAction
)
{
	return	FTM_ACTION_createCopy(pAction, NULL);
}

FTM_RET	FTDM_ACTION_del
(
	FTM_ACTION_ID	xID
)
{
	FTM_RET			xRet;
	FTM_ACTION_PTR	pAct = NULL;

	xRet = FTM_ACTION_get(xID, &pAct);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_ACTION_destroy(pAct);
}

FTM_RET	FTDM_ACTION_count
(
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_ACTION_count(pulCount);
}

FTM_RET	FTDM_ACTION_get
(
	FTM_ACTION_ID	xID,
	FTM_ACTION_PTR	_PTR_ 	ppAct
)
{
	return	FTM_ACTION_get(xID, ppAct);
}

FTM_RET	FTDM_ACTION_getAt
(
	FTM_ULONG				nIndex,
	FTM_ACTION_PTR	_PTR_ 	ppAct
)
{
	return	FTM_ACTION_getAt(nIndex, ppAct);
}

FTM_RET	FTDM_ACTION_showList
(
	FTM_VOID
)
{
	FTM_ACTION_PTR	pAction;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# Action Information\n");
	MESSAGE("\t%4s %8s %8s %8s\n", "ID", "TYPE", "TARGET", "VALUE");

	FTM_ACTION_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_ACTION_getAt(i, &pAction) == FTM_RET_OK)
		{
			MESSAGE("\t%4d ", pAction->xID);
			switch(pAction->xType)
			{
			case	FTM_ACTION_TYPE_SET:
				{
					FTM_CHAR	pBuff[1024];

					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pAction->xParams.xSet.xValue);
					MESSAGE("%8s %08x %8s\n", "Set", pAction->xParams.xSet.xEPID, pBuff);
				}
				break;

			default:
				{
					MESSAGE("UNKNOWN\n");
				}
				break;
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_ACTION_PTR		pAction = (FTM_ACTION_PTR)pElement;
	FTM_ACTION_ID_PTR	pActionID = (FTM_ACTION_ID_PTR)pIndicator;

	return	(pAction->xID == *pActionID);
}
