#include <unistd.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_mem.h"
#include "ftm_msg_queue.h"
#include "ftdm_config.h"
#include "ftdm_trigger.h"
#include "ftdm_sqlite.h"

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
				FTDM_TRIGGER_PTR	pTrigger;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xTriggers, i, &xTriggerItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_TRIGGER		xInfo;
						FTM_ULONG		ulIndex = 0;
						FTM_CHAR		pTypeString[64];

						FTM_TRIGGER_setDefault(&xInfo);

						FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "id", xInfo.pID, FTM_ID_LEN);
						FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "name", xInfo.pName, FTM_NAME_LEN);
						FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "index", &ulIndex);
						
						xRet = FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "epid", xInfo.pEPID, FTM_EPID_LEN);
						if (xRet != FTM_RET_OK)
						{
							TRACE("Trigger epid get failed.\n");
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "type", pTypeString, sizeof(pTypeString) - 1);
						if (xRet != FTM_RET_OK)
						{
							TRACE("Trigger type get failed.\n");
							continue;
						}

						xRet = FTM_TRIGGER_strToType(pTypeString, &xInfo.xType);
						if (xRet != FTM_RET_OK)
						{
							TRACE("Trigger type[%s] is invalid.\n", pTypeString);
							continue;
						}

						switch(xInfo.xType)
						{
						case	FTM_TRIGGER_TYPE_ABOVE:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xAbove.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xAbove.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "value", &xInfo.xParams.xAbove.xValue);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_BELOW:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xBelow.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xBelow.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "value", &xInfo.xParams.xBelow.xValue);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_INCLUDE:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xInclude.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xInclude.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "upper", &xInfo.xParams.xInclude.xUpper);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "lower", &xInfo.xParams.xInclude.xLower);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_EXCEPT:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xExcept.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xExcept.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "upper", &xInfo.xParams.xExcept.xUpper);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "lower", &xInfo.xParams.xExcept.xLower);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_CHANGE:
						default:
							{
							}
							break;
						}

						xRet = FTDM_TRIGGER_create(&xInfo, &pTrigger);
						if (xRet != FTM_RET_OK)
						{
							ERROR("The new trigger can not creation.\n");
						}
						else if (ulIndex != 0)
						{
							pTrigger->ulIndex = ulIndex;	
						}
					}
				}
			}
		}
		else
		{
			TRACE("Triggers not found.\n");	
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
	FTM_RET		xRet;
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_TRIGGER_count(&nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_TRIGGER_PTR	pInfos;
		FTM_ULONG		nTriggerCount = 0;
		
		pInfos = (FTM_TRIGGER_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_TRIGGER));
		if (pInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_TRIGGER_getList(pInfos, nMaxCount, &nTriggerCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nTriggerCount ; i++)
			{
				FTDM_TRIGGER_PTR	pTrigger;

				pTrigger = (FTDM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_TRIGGER));
				if (pTrigger == NULL)
				{
					ERROR("Not enough memory!\n");
					break;	
				}

				memcpy(&pTrigger->xInfo, &pInfos[i], sizeof(FTM_TRIGGER));

				xRet = FTM_TRIGGER_append((FTM_TRIGGER_PTR)pTrigger);
				if (xRet != FTM_RET_OK)
				{
					FTM_MEM_free(pTrigger);	
				}
			}
		}

		FTM_MEM_free(pInfos);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_saveToDB
(
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTM_TRIGGER_PTR	pTrigger;
	
	xRet = FTM_TRIGGER_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER	xInfo;

		xRet = FTM_TRIGGER_getAt(i, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Failed to get trigger information[%08x]\n", xRet);
			continue;
		}
		

		xRet = FTDM_DBIF_TRIGGER_get(pTrigger->pID, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			xRet = FTDM_DBIF_TRIGGER_create(pTrigger);	
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to save the new trigger.[%08x]\n", xRet);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_create
(
	FTM_TRIGGER_PTR 	pInfo,
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	if (FTDM_TRIGGER_get(pInfo->pID, &pTrigger) == FTM_RET_OK)
	{
		ERROR("Object already exist!\n");
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
		while (FTDM_TRIGGER_get(pInfo->pID, &pTrigger) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_TRIGGER_create(pInfo);
	if (xRet == FTM_RET_OK)
	{
		pTrigger = (FTDM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_TRIGGER));
		if (pTrigger == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_TRIGGER_destroy(pInfo->pID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pTrigger, 0, sizeof(FTDM_TRIGGER));
		memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));

		xRet = FTM_TRIGGER_append((FTM_TRIGGER_PTR)pTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Trigger[%s] append failed\n", pTrigger->xInfo.pID);
			FTDM_DBIF_TRIGGER_destroy(pInfo->pID);
			FTM_MEM_free(pTrigger);

		}
	}
	else
	{
		ERROR("Trigger[%s] DB creation failed.\n", pInfo->pID);
	}

	if (xRet == FTM_RET_OK)
	{
		*ppTrigger = pTrigger;	
	}
	
	return	xRet;
}

FTM_RET	FTDM_TRIGGER_destroy
(
	FTM_CHAR_PTR	pID
)
{
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger = NULL;

	xRet = FTDM_TRIGGER_get(pID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_TRIGGER_destroy(pID);
	FTM_TRIGGER_remove((FTM_TRIGGER_PTR)pTrigger);

	FTM_MEM_free(pTrigger);

	return	FTM_RET_OK;
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
	FTM_CHAR_PTR	pID,
	FTDM_TRIGGER_PTR	_PTR_ 	ppTrigger
)
{
	return	FTM_TRIGGER_get(pID, (FTM_TRIGGER_PTR _PTR_)ppTrigger);
}

FTM_RET	FTDM_TRIGGER_getAt
(
	FTM_ULONG				nIndex,
	FTDM_TRIGGER_PTR _PTR_ 	ppTrigger
)
{
	return	FTM_TRIGGER_getAt(nIndex, (FTM_TRIGGER_PTR _PTR_)ppTrigger);
}

FTM_RET	FTDM_TRIGGER_getByIndex
(
	FTM_ULONG				ulIndex,
	FTDM_TRIGGER_PTR _PTR_ 	ppTrigger
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_INT		i;

	xRet = FTDM_TRIGGER_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i  < ulCount ; i++)
	{
		xRet = FTDM_TRIGGER_getAt(i, ppTrigger);
		if (xRet == FTM_RET_OK)
		{
			return	xRet;	
		}

		if ((*ppTrigger)->ulIndex == ulIndex)
		{
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTDM_TRIGGER_set
(
	FTM_CHAR_PTR		pID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	xRet = FTM_TRIGGER_get(pID, (FTM_TRIGGER_PTR _PTR_)&pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xFields & FTM_TRIGGER_FIELD_NAME)
	{
		strcpy(pTrigger->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_TRIGGER_FIELD_EPID)
	{
		strcpy(pTrigger->xInfo.pEPID, pInfo->pEPID);
	}

	if (xFields & FTM_TRIGGER_FIELD_DETECT_TIME)
	{
		pTrigger->xInfo.xParams.xCommon.ulDetectionTime = pInfo->xParams.xCommon.ulDetectionTime;
	}

	if (xFields & FTM_TRIGGER_FIELD_HOLD_TIME)
	{
		pTrigger->xInfo.xParams.xCommon.ulHoldingTime = pInfo->xParams.xCommon.ulHoldingTime;
	}

	if (xFields & FTM_TRIGGER_FIELD_VALUE)
	{
		memcpy(&pTrigger->xInfo.xParams.xAbove.xValue, &pInfo->xParams.xAbove.xValue, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_TRIGGER_FIELD_LOWER)
	{
		memcpy(&pTrigger->xInfo.xParams.xInclude.xLower, &pInfo->xParams.xInclude.xLower, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_TRIGGER_FIELD_UPPER)
	{
		memcpy(&pTrigger->xInfo.xParams.xInclude.xUpper, &pInfo->xParams.xInclude.xUpper, sizeof(FTM_VALUE));
	}

	FTDM_DBIF_TRIGGER_set(pID, &pTrigger->xInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_getIDList
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
	FTDM_TRIGGER_PTR	pTrigger;

	xRet = FTDM_TRIGGER_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		xRet = FTDM_TRIGGER_getAt(ulIndex + i, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		strcpy(pIDs[(*pulCount)++], pTrigger->xInfo.pID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_showList
(
	FTM_VOID
)
{
	FTDM_TRIGGER_PTR	pTrigger;
	FTM_ULONG			i, ulCount;
	MESSAGE("\n# Trigger Information\n");
	MESSAGE("\t%16s %16s %8s %8s %s\n", "ID", "EPID", "DETECT", "HOLD", "CONDITION");

	FTM_TRIGGER_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTDM_TRIGGER_getAt(i, &pTrigger) == FTM_RET_OK)
		{
			MESSAGE("\t%16s %16s ", pTrigger->xInfo.pID, pTrigger->xInfo.pEPID);

			switch(pTrigger->xInfo.xType)
			{
			case	FTM_TRIGGER_TYPE_ABOVE:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xAbove.ulDetectionTime) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xAbove.ulHoldingTime) / 1000000.0); 
					MESSAGE("x >= %s ", FTM_VALUE_print(&pTrigger->xInfo.xParams.xAbove.xValue));
				}
				break;

			case	FTM_TRIGGER_TYPE_BELOW:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xBelow.ulDetectionTime) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xBelow.ulHoldingTime) / 1000000.0); 
					MESSAGE("x <= %s ", FTM_VALUE_print(&pTrigger->xInfo.xParams.xBelow.xValue));
				}
				break;

			case	FTM_TRIGGER_TYPE_INCLUDE:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xInclude.ulDetectionTime) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xInclude.ulHoldingTime) / 1000000.0); 
					MESSAGE("(%s <= x <= ", FTM_VALUE_print(&pTrigger->xInfo.xParams.xInclude.xLower));
					MESSAGE("%s) ", FTM_VALUE_print(&pTrigger->xInfo.xParams.xInclude.xUpper));
				}
				break;

			case	FTM_TRIGGER_TYPE_EXCEPT:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xExcept.ulDetectionTime) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xExcept.ulHoldingTime) / 1000000.0); 
					MESSAGE("(x < %s) && ", FTM_VALUE_print(&pTrigger->xInfo.xParams.xExcept.xLower));
					MESSAGE("(%s < x) ", FTM_VALUE_print(&pTrigger->xInfo.xParams.xExcept.xUpper));
				}
				break;

			case	FTM_TRIGGER_TYPE_CHANGE:
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

FTM_BOOL	FTDM_TRIGGER_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_TRIGGER_PTR		pTrigger = (FTM_TRIGGER_PTR)pElement;
	FTM_CHAR_PTR		pTriggerID = (FTM_CHAR_PTR)pIndicator;

	return	strcpy(pTrigger->pID,pTriggerID) == 0;
}
