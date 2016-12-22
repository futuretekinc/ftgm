#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_config.h"
#include "ftdm_action.h"
#include "ftdm_dbif.h"
#include "ftdm_log.h"

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
	FTM_CHAR_PTR	pActionID,
	FTDM_ACTION_PTR	_PTR_ 	ppAction
)
{
	return	FTM_ACTION_get(pActionID, (FTM_ACTION_PTR _PTR_)ppAction);
}

FTM_RET	FTDM_ACTION_getAt
(
	FTM_ULONG				nIndex,
	FTDM_ACTION_PTR	_PTR_ 	ppAction
)
{
	return	FTM_ACTION_getAt(nIndex, (FTM_ACTION_PTR _PTR_)ppAction);
}

FTM_RET	FTDM_ACTION_getByIndex
(
	FTM_ULONG				ulIndex,
	FTDM_ACTION_PTR _PTR_ 	ppAction
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_INT		i;

	xRet = FTDM_ACTION_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i  < ulCount ; i++)
	{
		xRet = FTDM_ACTION_getAt(i, ppAction);
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

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_getIDList
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
	FTDM_ACTION_PTR	pAction;

	xRet = FTDM_ACTION_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		xRet = FTDM_ACTION_getAt(ulIndex + i, &pAction);
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
	FTM_VOID
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
