#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_action.h"
#include "ftdm_sqlite.h"
#include "ftdm_log.h"

FTM_RET	FTDM_ACTION_init
(
	FTDM_CONTEXT_PTR	pFTDM
)
{
	return	FTM_ACTION_init();
}

FTM_RET FTDM_ACTION_final
(
	FTDM_CONTEXT_PTR	pFTDM
)
{
	return	FTM_ACTION_final();
}

FTM_RET	FTDM_ACTION_loadConfig
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xAction;
	FTM_CONFIG_ITEM		xActions;
	FTM_CONFIG_ITEM		xActionItem;
	FTM_CONFIG_ITEM		xParamsItem;

	xRet = FTM_CONFIG_getItem(pConfig, "event", &xAction);
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
						FTDM_ACTION_PTR	pAction = NULL;
						FTM_ULONG		ulIndex = 0;

						FTM_ACTION_setDefault(&xInfo);

						FTM_CONFIG_ITEM_getItemString(&xActionItem, "id", xInfo.pID, FTM_ID_LEN);
						FTM_CONFIG_ITEM_getItemString(&xActionItem, "name", xInfo.pName, FTM_NAME_LEN);
						FTM_CONFIG_ITEM_getItemULONG(&xActionItem, "index", &ulIndex);
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

								xRet = FTDM_ACTION_create(pFTDM, &xInfo, &pAction);
								if (xRet != FTM_RET_OK)
								{
										ERROR("The new event can not creation.\n");
								}
								else if (ulIndex != 0)
								{
									pAction->ulIndex = ulIndex;	
								}
								FTDM_LOG_createAction(xInfo.pID, xRet);
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

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_loadFromFile
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTDM_ACTION_loadConfig(pFTDM, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTDM_ACTION_loadFromDB
(
	FTDM_CONTEXT_PTR	pFTDM
)
{
	ASSERT(pFTDM != NULL);
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_ACTION_count(pFTDM->pDBIF, &nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_ACTION_PTR	pActions;
		FTM_ULONG		nActionCount = 0;
		
		pActions = (FTM_ACTION_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_ACTION));
		if (pActions == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_ACTION_getList(pFTDM->pDBIF, pActions, nMaxCount, &nActionCount) == FTM_RET_OK)
		{
			FTM_INT	i;
			FTDM_ACTION_PTR	pAction;

			for(i = 0 ; i < nActionCount ; i++)
			{
				FTDM_ACTION_create(pFTDM, &pActions[i], &pAction);
			}
		}

		FTM_MEM_free(pActions);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_saveToDB
(
	FTDM_CONTEXT_PTR	pFTDM
)
{
	ASSERT(pFTDM != NULL);

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

			xRet = FTDM_DBIF_ACTION_get(pFTDM->pDBIF, pAction->pID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_ACTION_create(pFTDM->pDBIF, &xInfo);	
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
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ACTION_PTR 	pInfo,
	FTDM_ACTION_PTR	_PTR_ ppAction
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_ACTION_PTR	pAction;

	if (FTDM_ACTION_get(pFTDM, pInfo->pID, &pAction) == FTM_RET_OK)
	{
			ERROR("Action[%s] already exist.\n", pInfo->pID);
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	if (strlen(pInfo->pID) == 0)
	{
		do 
		{	
			struct timeval	xTime;

			gettimeofday(&xTime, NULL);
			sprintf(pInfo->pID, "%08lx%08lx", (FTM_ULONG)xTime.tv_sec, (FTM_ULONG)xTime.tv_usec);
			usleep(10);
		}
		while (FTDM_ACTION_get(pFTDM, pInfo->pID, &pAction) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_ACTION_create(pFTDM->pDBIF, pInfo);
	if (xRet == FTM_RET_OK)
	{
		pAction = (FTDM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTDM_ACTION));
		if (pAction == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_ACTION_destroy(pFTDM->pDBIF, pInfo->pID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pAction, 0, sizeof(FTDM_ACTION));
		memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));
		xRet = FTM_ACTION_append((FTM_ACTION_PTR)pAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Action[%s] append failed.\n", pAction->xInfo.pID);
			FTDM_DBIF_ACTION_destroy(pFTDM->pDBIF, pInfo->pID);
			FTM_MEM_free(pAction);
		}
		else
		{
			if (ppAction != NULL)
			{
				*ppAction = pAction;
			}
		}
	}

	return	xRet;
}

FTM_RET	FTDM_ACTION_destroy
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pActionID
)
{
	FTM_RET				xRet;
	FTDM_ACTION_PTR	pAction = NULL;

	xRet = FTDM_ACTION_get(pFTDM, pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_ACTION_destroy(pFTDM->pDBIF, pActionID);
	FTM_ACTION_remove((FTM_ACTION_PTR)pAction);

	FTM_MEM_free(pAction);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_count
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_ACTION_count(pulCount);
}

FTM_RET	FTDM_ACTION_get
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pActionID,
	FTDM_ACTION_PTR	_PTR_ 	ppAction
)
{
	return	FTM_ACTION_get(pActionID, (FTM_ACTION_PTR _PTR_)ppAction);
}

FTM_RET	FTDM_ACTION_getAt
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG				nIndex,
	FTDM_ACTION_PTR	_PTR_ 	ppAction
)
{
	return	FTM_ACTION_getAt(nIndex, (FTM_ACTION_PTR _PTR_)ppAction);
}

FTM_RET	FTDM_ACTION_getByIndex
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG				ulIndex,
	FTDM_ACTION_PTR _PTR_ 	ppAction
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_INT		i;

	xRet = FTDM_ACTION_count(pFTDM, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i  < ulCount ; i++)
	{
		xRet = FTDM_ACTION_getAt(pFTDM, i, ppAction);
		if (xRet == FTM_RET_OK)
		{
			return	xRet;	
		}

		if ((*ppAction)->ulIndex == ulIndex)
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTDM_ACTION_set
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_ACTION_PTR	pAction;

	xRet = FTM_ACTION_get(pID, (FTM_ACTION_PTR _PTR_)&pAction);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	switch(pAction->xInfo.xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			if (xFields & FTM_ACTION_FIELD_NAME)
			{
				strcpy(pAction->xInfo.pName, pInfo->pName);
			}

			if (xFields & FTM_ACTION_FIELD_EPID)
			{
				strcpy(pAction->xInfo.xParams.xSet.pEPID, pInfo->xParams.xSet.pEPID);
			}
			
			if (xFields & FTM_ACTION_FIELD_VALUE)
			{
				memcpy(&pAction->xInfo.xParams.xSet.xValue, &pInfo->xParams.xSet.xValue, sizeof(FTM_VALUE));
			}
		}
		break;

	default:
		{
			return	FTM_RET_ERROR;	
		}
	}

	FTDM_DBIF_ACTION_set(pFTDM->pDBIF, pID, &pAction->xInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_getIDList
(
	FTDM_CONTEXT_PTR	pFTDM,
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
	FTDM_ACTION_PTR	pAction;

	xRet = FTDM_ACTION_count(pFTDM, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		xRet = FTDM_ACTION_getAt(pFTDM, ulIndex + i, &pAction);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		strcpy(pIDs[(*pulCount)++], pAction->xInfo.pID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_showList
(
	FTDM_CONTEXT_PTR	pFTDM
)
{
	FTM_ACTION_PTR	pAction;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# Action Information\n");
	MESSAGE("\t%16s %16s %8s %16s %16s %8s\n", 
		"ID", "NAME", "STATE", "TARGET", "TYPE", "VALUE");

	FTM_ACTION_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_ACTION_getAt(i, &pAction) == FTM_RET_OK)
		{
			MESSAGE("\t%16s ", pAction->pID);
			MESSAGE("%16s ", pAction->pName);
			MESSAGE("%8s ", "Running");
			switch(pAction->xType)
			{
			case	FTM_ACTION_TYPE_SET:
				{
					MESSAGE("%16s %16s %8s\n", 
						pAction->xParams.xSet.pEPID,
						"Set",
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
	FTM_CHAR_PTR		pActionID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pAction->pID, pActionID) == 0;
}
