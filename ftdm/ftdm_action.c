#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_config.h"
#include "ftdm_action.h"
#include "ftdm_sqlite.h"

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
						FTM_ACTION		xInfo;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xActionItem, "id", (FTM_INT_PTR)&xInfo.xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_ITEM_getItemINT(&xActionItem, "type", (FTM_INT_PTR)&xInfo.xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xInfo.xType)
						{
						case	FTM_ACTION_TYPE_SET:
							{
								xRet = FTM_CONFIG_ITEM_getChildItem(&xActionItem, "params", &xParamsItem);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}
	
								xRet = FTM_CONFIG_ITEM_getItemString(&xParamsItem, "epid", xInfo.xParams.xSet.pEPID, FTM_EPID_LEN);
								if (xRet != FTM_RET_OK)
								{
									ERROR("EPID invalid!\n");
										continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xParamsItem, "value", &xInfo.xParams.xSet.xValue);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Action value invalid!\n");
										continue;
								}

								xRet = FTDM_ACTION_create(&xInfo);
								if (xRet != FTM_RET_OK)
								{
										ERROR("The new event can not creation.\n");
								}
							}
							break;

						default:
							{
								ERROR("ACtion type[%08x] is not supported.\n", xInfo.xType);
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
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_ACTION_count(&nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_ACTION_PTR	pActions;
		FTM_ULONG		nActionCount = 0;
		
		pActions = (FTM_ACTION_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_ACTION));
		if (pActions == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_ACTION_getList(pActions, nMaxCount, &nActionCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nActionCount ; i++)
			{
				FTM_ACTION_create(&pActions[i]);
			}
		}

		FTM_MEM_free(pActions);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_saveToDB
(
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTM_ACTION_PTR	pAction;
	
	xRet = FTM_ACTION_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_ACTION_getAt(i, &pAction);
		if (xRet == FTM_RET_OK)
		{
			FTM_ACTION	xInfo;

			xRet = FTDM_DBIF_ACTION_get(pAction->xID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_ACTION_create(&xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Failed to save the new action.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR("Failed to get action information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_create
(
	FTM_ACTION_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_ACTION_PTR	pAction;

	if (FTDM_ACTION_get(pInfo->xID, &pAction) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	xRet = FTDM_DBIF_ACTION_create(pInfo);
	if (xRet == FTM_RET_OK)
	{
		pAction = (FTDM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTDM_ACTION));
		if (pAction == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_ACTION_destroy(pInfo->xID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pAction, 0, sizeof(FTDM_ACTION));
		memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));
		xRet = FTM_ACTION_append((FTM_ACTION_PTR)pAction);
		if (xRet != FTM_RET_OK)
		{
			FTDM_DBIF_ACTION_destroy(pInfo->xID);
			FTM_MEM_free(pAction);
		}
	}

	return	xRet;
}

FTM_RET	FTDM_ACTION_destroy
(
	FTM_ACTION_ID	xID
)
{
	FTM_RET				xRet;
	FTDM_ACTION_PTR	pAction = NULL;

	xRet = FTDM_ACTION_get(xID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_ACTION_destroy(xID);
	FTM_ACTION_remove((FTM_ACTION_PTR)pAction);

	FTM_MEM_free(pAction);

	return	FTM_RET_OK;
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
	FTDM_ACTION_PTR	_PTR_ 	ppAction
)
{
	return	FTM_ACTION_get(xID, (FTM_ACTION_PTR _PTR_)ppAction);
}

FTM_RET	FTDM_ACTION_getAt
(
	FTM_ULONG				nIndex,
	FTDM_ACTION_PTR	_PTR_ 	ppAction
)
{
	return	FTM_ACTION_getAt(nIndex, (FTM_ACTION_PTR _PTR_)ppAction);
}

FTM_RET	FTDM_ACTION_showList
(
	FTM_VOID
)
{
	FTM_ACTION_PTR	pAction;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# Action Information\n");
	MESSAGE("\t%4s %8s %16s %8s\n", "ID", "TYPE", "TARGET", "VALUE");

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
					MESSAGE("%8s %16s %8s\n", "Set", 
						pAction->xParams.xSet.pEPID,
						FTM_VALUE_print(&pAction->xParams.xSet.xValue));
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
