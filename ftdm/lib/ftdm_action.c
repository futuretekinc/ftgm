#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_action.h"
#include "ftdm_sqlite.h"
#include "ftdm_log.h"

typedef	struct FTDM_ACTION_STRUCT
{
	FTDM_PTR 		pFTDM;
	FTM_ACTION		xInfo;
	FTM_ULONG		ulIndex;
}	FTDM_ACTION, _PTR_ FTDM_ACTION_PTR;

FTM_RET	FTDM_ACTION_create
(
	FTDM_PTR 		pFTDM,
	FTM_ACTION_PTR	pInfo,
	FTDM_ACTION_PTR	_PTR_ ppAction
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTDM_ACTION_PTR	pAction;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getAction(pFTDM, pInfo->pID, &pAction);
	if (xRet == FTM_RET_OK)
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
		while (FTDM_getAction(pFTDM, pInfo->pID, &pAction) == FTM_RET_OK);
	}

	xRet = FTDM_getDBIF(pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTDM_DBIF_addAction(pDBIF, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to add action!\n");
		return	xRet;
	}

	pAction = (FTDM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTDM_ACTION));
	if (pAction == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Failed to create action!\n");
		FTDM_DBIF_deleteAction(pDBIF, pInfo->pID);
		return	xRet;	
	}

	memset(pAction, 0, sizeof(FTDM_ACTION));
	memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));
	pAction->pFTDM = pFTDM;

	*ppAction = pAction;

	return	xRet;
}

FTM_RET	FTDM_ACTION_destroy
(
	FTDM_ACTION_PTR	_PTR_ ppAction
)
{
	ASSERT(ppAction != NULL);

	FTM_RET	xRet;
	FTDM_DBIF_PTR		pDBIF;

	
	xRet = FTDM_getDBIF((*ppAction)->pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTDM_DBIF_deleteAction(pDBIF, (*ppAction)->xInfo.pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove action from DB!\n");	
	}

	FTM_MEM_free((*ppAction));

	*ppAction = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_init
(
	FTDM_ACTION_PTR	pAction
)
{
	return	FTM_RET_OK;
}

FTM_RET FTDM_ACTION_final
(
	FTDM_ACTION_PTR	pAction
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_get
(
	FTDM_ACTION_PTR	pAction,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pAction != NULL);
	ASSERT(pInfo != NULL);

	memcpy(pInfo, &pAction->xInfo, sizeof(FTM_ACTION));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_set
(
	FTDM_ACTION_PTR		pAction,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pAction != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTDM_DBIF_PTR	pDBIF;

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

	xRet = FTDM_getDBIF(pAction->pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	FTDM_DBIF_setAction(pDBIF, pAction->xInfo.pID, &pAction->xInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_ACTION_getID
(
	FTDM_ACTION_PTR	pAction,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pAction != NULL);
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(pAction->xInfo.pID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strcpy(pBuff, pAction->xInfo.pID);

	return	FTM_RET_OK;
}


FTM_BOOL	FTDM_ACTION_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTDM_ACTION_PTR		pAction = (FTDM_ACTION_PTR)pElement;
	FTM_CHAR_PTR		pActionID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pAction->xInfo.pID, pActionID) == 0;
}

FTM_BOOL	FTDM_ACTION_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTDM_ACTION_PTR		pAction1 = (FTDM_ACTION_PTR)pElement1;
	FTDM_ACTION_PTR		pAction2 = (FTDM_ACTION_PTR)pElement2;

	return	strcasecmp(pAction1->xInfo.pID, pAction2->xInfo.pID);
}


