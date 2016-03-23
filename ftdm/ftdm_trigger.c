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
						FTM_TRIGGER		xInfo;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "id", (FTM_INT_PTR)&xInfo.xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "epid", (FTM_INT_PTR)&xInfo.xEPID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemINT(&xTriggerItem, "type", (FTM_INT_PTR)&xInfo.xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xInfo.xType)
						{
						case	FTM_TRIGGER_TYPE_ABOVE:
							{
								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "detect",&xInfo.xParams.xAbove.xDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "hold",&xInfo.xParams.xAbove.xHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "value", &xInfo.xParams.xAbove.xValue);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_BELOW:
							{
								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "detect",&xInfo.xParams.xBelow.xDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "hold",&xInfo.xParams.xBelow.xHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "value", &xInfo.xParams.xBelow.xValue);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_INCLUDE:
							{
								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "detect",&xInfo.xParams.xInclude.xDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "hold",&xInfo.xParams.xInclude.xHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "upper", &xInfo.xParams.xInclude.xUpper);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "lower", &xInfo.xParams.xInclude.xLower);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_EXCEPT:
							{
								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "detect",&xInfo.xParams.xExcept.xDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemTime(&xTriggerItem, "hold",&xInfo.xParams.xExcept.xHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "upper", &xInfo.xParams.xExcept.xUpper);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemEPData(&xTriggerItem, "lower", &xInfo.xParams.xExcept.xLower);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_CHANGE:
							{
							}
							break;
						}

						xRet = FTDM_TRIGGER_create(&xInfo);
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
		xRet = FTM_TRIGGER_getAt(i, &pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_TRIGGER	xInfo;

			xRet = FTDM_DBIF_TRIGGER_get(pTrigger->xID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_TRIGGER_create(&xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Failed to save the new trigger.[%08x]\n", xRet);
				}
				else
				{
					ERROR("The trigger[%d] has been saved.\n", pTrigger->xID);
				
				}
			}
		}
		else
		{
			ERROR("Failed to get trigger information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_create
(
	FTM_TRIGGER_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	if (FTDM_TRIGGER_get(pInfo->xID, &pTrigger) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	xRet = FTDM_DBIF_TRIGGER_create(pInfo);
	if (xRet == FTM_RET_OK)
	{
		pTrigger = (FTDM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_TRIGGER));
		if (pTrigger == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_TRIGGER_destroy(pInfo->xID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pTrigger, 0, sizeof(FTDM_TRIGGER));
		memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));
		xRet = FTM_TRIGGER_append((FTM_TRIGGER_PTR)pTrigger);
		if (xRet != FTM_RET_OK)
		{
			FTDM_DBIF_TRIGGER_destroy(pInfo->xID);
			FTM_MEM_free(pTrigger);
		}
	}

	return	xRet;
}

FTM_RET	FTDM_TRIGGER_destroy
(
	FTM_TRIGGER_ID	xID
)
{
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger = NULL;

	xRet = FTDM_TRIGGER_get(xID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_TRIGGER_destroy(xID);
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
	FTM_TRIGGER_ID	xID,
	FTDM_TRIGGER_PTR	_PTR_ 	ppTrigger
)
{
	return	FTM_TRIGGER_get(xID, (FTM_TRIGGER_PTR _PTR_)ppTrigger);
}

FTM_RET	FTDM_TRIGGER_getAt
(
	FTM_ULONG				nIndex,
	FTDM_TRIGGER_PTR _PTR_ 	ppTrigger
)
{
	return	FTM_TRIGGER_getAt(nIndex, (FTM_TRIGGER_PTR _PTR_)ppTrigger);
}

FTM_RET	FTDM_TRIGGER_showList
(
	FTM_VOID
)
{
	FTDM_TRIGGER_PTR	pTrigger;
	FTM_ULONG			i, ulCount;
	MESSAGE("\n# Trigger Information\n");
	MESSAGE("\t%4s %8s %8s %8s %s\n", "ID", "EPID", "DETECT", "HOLD", "CONDITION");

	FTM_TRIGGER_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTDM_TRIGGER_getAt(i, &pTrigger) == FTM_RET_OK)
		{
			FTM_CHAR	pBuff[1024];

			MESSAGE("\t%4d %08x ", pTrigger->xInfo.xID, pTrigger->xInfo.xEPID);

			switch(pTrigger->xInfo.xType)
			{
			case	FTM_TRIGGER_TYPE_ABOVE:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xAbove.xDetectionTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xAbove.xDetectionTime.xTimeval.tv_usec) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xAbove.xHoldingTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xAbove.xHoldingTime.xTimeval.tv_usec) / 1000000.0); 
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xInfo.xParams.xAbove.xValue);
					MESSAGE("x >= %s ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_BELOW:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xBelow.xDetectionTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xBelow.xDetectionTime.xTimeval.tv_usec) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xBelow.xHoldingTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xBelow.xHoldingTime.xTimeval.tv_usec) / 1000000.0); 
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xInfo.xParams.xBelow.xValue);
					MESSAGE("x <= %s ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_INCLUDE:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xInclude.xDetectionTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xInclude.xDetectionTime.xTimeval.tv_usec) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xInclude.xHoldingTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xInclude.xHoldingTime.xTimeval.tv_usec) / 1000000.0); 
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xInfo.xParams.xInclude.xLower);
					MESSAGE("(%s <= x <= ", pBuff);

					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xInfo.xParams.xInclude.xUpper);
					MESSAGE("%s) ", pBuff);
				}
				break;

			case	FTM_TRIGGER_TYPE_EXCEPT:
				{
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xExcept.xDetectionTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xExcept.xDetectionTime.xTimeval.tv_usec) / 1000000.0);
					MESSAGE("%8.1f ", (pTrigger->xInfo.xParams.xExcept.xHoldingTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xExcept.xHoldingTime.xTimeval.tv_usec) / 1000000.0); 
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xInfo.xParams.xExcept.xLower);
					MESSAGE("(x < %s) && ", pBuff);

					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pTrigger->xInfo.xParams.xExcept.xUpper);
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
