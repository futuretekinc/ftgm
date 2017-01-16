#include <unistd.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_mem.h"
#include "ftm_msg_queue.h"
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_trigger.h"
#include "ftdm_sqlite.h"
#include "ftdm_log.h"

typedef	struct FTDM_TRIGGER_STRUCT
{
	FTDM_PTR pFTDM;
	FTM_TRIGGER		xInfo;
	FTM_ULONG		ulIndex;
}	FTDM_TRIGGER, _PTR_ FTDM_TRIGGER_PTR;

FTM_RET	FTDM_TRIGGER_init
(
	FTDM_TRIGGER_PTR	pTrigger
)
{
	return	FTM_RET_OK;
}

FTM_RET FTDM_TRIGGER_final
(
	FTDM_TRIGGER_PTR	pTrigger
)
{
	return	FTM_RET_OK;
}
#if 0
FTM_RET	FTDM_TRIGGER_loadConfig
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_CONFIG_ITEM_PTR	pSection
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pSection != NULL);

	FTM_RET				xRet;
	FTM_TRIGGER		xInfo;
	FTM_ULONG		ulIndex = 0;
	FTM_CHAR		pTypeString[64];

	FTM_TRIGGER_setDefault(&xInfo);

	FTM_CONFIG_ITEM_getItemString(pSection,	"id", xInfo.pID, FTM_ID_LEN);
	FTM_CONFIG_ITEM_getItemString(pSection, "name", xInfo.pName, FTM_NAME_LEN);
	FTM_CONFIG_ITEM_getItemULONG(pSection, "index", &ulIndex);

	xRet = FTM_CONFIG_ITEM_getItemString(pSection, "epid", xInfo.pEPID, FTM_EPID_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Trigger epid get failed.\n");
		return	xRet;
	}

	xRet = FTM_CONFIG_ITEM_getItemString(pSection, "type", pTypeString, sizeof(pTypeString) - 1);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Trigger type get failed.\n");
		return	xRet;
	}

	xRet = FTM_TRIGGER_strToType(pTypeString, &xInfo.xType);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Trigger type[%s] is invalid.\n", pTypeString);
		return	xRet;
	}

	switch(xInfo.xType)
	{
	case	FTM_TRIGGER_TYPE_ABOVE:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "detect",&xInfo.xParams.xAbove.ulDetectionTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "hold",&xInfo.xParams.xAbove.ulHoldingTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemValue(pSection, "value", &xInfo.xParams.xAbove.xValue);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

		}
		break;

	case	FTM_TRIGGER_TYPE_BELOW:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "detect",&xInfo.xParams.xBelow.ulDetectionTime);
			if (xRet != FTM_RET_OK)
			{
				return xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "hold",&xInfo.xParams.xBelow.ulHoldingTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemValue(pSection, "value", &xInfo.xParams.xBelow.xValue);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

		}
		break;

	case	FTM_TRIGGER_TYPE_INCLUDE:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "detect",&xInfo.xParams.xInclude.ulDetectionTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "hold",&xInfo.xParams.xInclude.ulHoldingTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemValue(pSection, "upper", &xInfo.xParams.xInclude.xUpper);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemValue(pSection, "lower", &xInfo.xParams.xInclude.xLower);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

		}
		break;

	case	FTM_TRIGGER_TYPE_EXCEPT:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "detect",&xInfo.xParams.xExcept.ulDetectionTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemULONG(pSection, "hold",&xInfo.xParams.xExcept.ulHoldingTime);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemValue(pSection, "upper", &xInfo.xParams.xExcept.xUpper);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			xRet = FTM_CONFIG_ITEM_getItemValue(pSection, "lower", &xInfo.xParams.xExcept.xLower);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}

			}
			break;

	case	FTM_TRIGGER_TYPE_CHANGE:
	default:
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
	//FTDM_LOG_createTrigger(xInfo.pID, xRet);

	return	FTM_RET_OK;
}
#endif

FTM_RET	FTDM_TRIGGER_create
(
	FTDM_PTR	pFTDM,
	FTM_TRIGGER_PTR 	pInfo,
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger;
	FTDM_DBIF_PTR		pDBIF;

	if (FTDM_getTrigger(pFTDM, pInfo->pID, &pTrigger) == FTM_RET_OK)
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
		while (FTDM_getTrigger(pFTDM, pInfo->pID, &pTrigger) == FTM_RET_OK);
	}

	xRet = FTDM_getDBIF(pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTDM_DBIF_addTrigger(pDBIF, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed go create trigger.\n");
		return	xRet;
	}

	pTrigger = (FTDM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_TRIGGER));
	if (pTrigger == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Failed to create trigger!\n");
		FTDM_DBIF_deleteTrigger(pDBIF, pInfo->pID);
		return	xRet;	
	}

	memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));
	pTrigger->pFTDM = pFTDM;

	*ppTrigger = pTrigger;	
	
	return	xRet;
}

FTM_RET	FTDM_TRIGGER_destroy
(
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	FTM_RET				xRet;

	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF((*ppTrigger)->pFTDM, &pDBIF);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_deleteTrigger(pDBIF, (*ppTrigger)->xInfo.pID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to delete trigger from DB!\n");	
		}
	}

	FTM_MEM_free(*ppTrigger);

	*ppTrigger = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_get
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pInfo != NULL);

	memcpy(pInfo, &pTrigger->xInfo, sizeof(FTM_TRIGGER));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_TRIGGER_set
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF(pTrigger->pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
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

	return	FTDM_DBIF_setTrigger(pDBIF, pTrigger->xInfo.pID, &pTrigger->xInfo);
}

FTM_RET	FTDM_TRIGGER_getID
(
	FTDM_TRIGGER_PTR	pTrigger,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(pTrigger->xInfo.pID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strcpy(pBuff, pTrigger->xInfo.pID);

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_TRIGGER_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTDM_TRIGGER_PTR	pTrigger = (FTDM_TRIGGER_PTR)pElement;
	FTM_CHAR_PTR		pTriggerID = (FTM_CHAR_PTR)pIndicator;

	return	strcasecmp(pTrigger->xInfo.pID, pTriggerID) == 0;
}

FTM_BOOL	FTDM_TRIGGER_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTDM_TRIGGER_PTR		pTrigger1 = (FTDM_TRIGGER_PTR)pElement1;
	FTDM_TRIGGER_PTR		pTrigger2 = (FTDM_TRIGGER_PTR)pElement2;

	return	strcasecmp(pTrigger1->xInfo.pID, pTrigger2->xInfo.pID);
}

