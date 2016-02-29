#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_config.h"
#include "ftdm_act.h"

FTM_RET	FTDM_ACT_init
(
	FTM_VOID
)
{
	return	FTM_ACT_init();
}

FTM_RET FTDM_ACT_final
(
	FTM_VOID
)
{
	return	FTM_ACT_final();
}

FTM_RET	FTDM_ACT_loadFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG			xConfig;
	FTM_CONFIG_ITEM		xTrigger;
	FTM_CONFIG_ITEM		xActs;
	FTM_CONFIG_ITEM		xActItem;

	xRet = FTM_CONFIG_init(&xConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(&xConfig, "action", &xTrigger);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xTrigger, "actors", &xActs);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xActs, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xActs, i, &xActItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_ACT_ID		xID;
						FTM_EP_ID		xTargetID;
						FTM_ACT_TYPE	xType;

						xRet = FTM_CONFIG_ITEM_getItemINT(&xActItem, "id", (FTM_INT_PTR)&xID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						
						xRet = FTM_CONFIG_ITEM_getItemINT(&xActItem, "target", (FTM_INT_PTR)&xTargetID);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemINT(&xActItem, "type", (FTM_INT_PTR)&xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTM_ACT_create(xID, xType, xTargetID, NULL);
						if (xRet != FTM_RET_OK)
						{
							ERROR("The new actor can not creation.\n");
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

FTM_RET	FTDM_ACT_loadFromDB
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACT_add
(
	FTM_ACT_PTR 	pAct
)
{
	return	FTM_ACT_create(pAct->xID, pAct->xType, pAct->xTargetID, NULL);
}

FTM_RET	FTDM_ACT_del
(
	FTM_ACT_ID	xID
)
{
	FTM_RET			xRet;
	FTM_ACT_PTR	pAct = NULL;

	xRet = FTM_ACT_get(xID, &pAct);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_ACT_destroy(pAct);
}

FTM_RET	FTDM_ACT_count
(
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_ACT_count(pulCount);
}

FTM_RET	FTDM_ACT_get
(
	FTM_ACT_ID	xID,
	FTM_ACT_PTR	_PTR_ 	ppAct
)
{
	return	FTM_ACT_get(xID, ppAct);
}

FTM_RET	FTDM_ACT_getAt
(
	FTM_ULONG				nIndex,
	FTM_ACT_PTR	_PTR_ 	ppAct
)
{
	return	FTM_ACT_getAt(nIndex, ppAct);
}

FTM_RET	FTDM_ACT_showList
(
	FTM_VOID
)
{
	FTM_ACT_PTR	pAct;
	FTM_ULONG		i, ulCount;
	MESSAGE("\n# ACTION INFORMATION\n");
	MESSAGE("\t%-8s %-8s %-8s\n", "ID", "TYPE", "TARGET");

	FTM_ACT_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_ACT_getAt(i, &pAct) == FTM_RET_OK)
		{
			MESSAGE("\t%08x ", pAct->xID);
			switch(pAct->xType)
			{
			case	FTM_ACT_TYPE_ON:
				{
					MESSAGE("%-8s ", "ON");
				}
				break;

			case	FTM_ACT_TYPE_OFF:
				{
					MESSAGE("%-8s ", "OFF");
				}
				break;

			case	FTM_ACT_TYPE_BLINK:
				{
					MESSAGE("%-8s ", "BLINK");
				}
				break;
			}
			MESSAGE("%08x\n", pAct->xTargetID);
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_ACT_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	FTM_ACT_PTR		pAct = (FTM_ACT_PTR)pElement;
	FTM_ACT_ID_PTR	pActID = (FTM_ACT_ID_PTR)pIndicator;

	return	(pAct->xID == *pActID);
}
