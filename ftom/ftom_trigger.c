#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom_ep.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_msg.h"
#include "libconfig.h"
#include "ftom_logger.h"

static 
FTM_VOID_PTR FTOM_TRIGGER_process
(
	FTM_VOID_PTR pData
);

static 
FTM_BOOL	FTOM_TRIGGER_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_LOCK				xLock;
static FTM_LIST_PTR			pTriggerList = NULL;
static FTM_MSG_QUEUE_PTR	pMsgQ = NULL;
static FTM_BOOL				bStop = FTM_TRUE;
static pthread_t			xTriggerThread;

FTM_RET	FTOM_TRIGGER_init
(
	FTM_VOID
)
{
	FTM_RET				xRet;

	if (pTriggerList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}


	xRet = FTM_LOCK_init(&xLock);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_MSGQ_create(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pTriggerList, FTOM_TRIGGER_seeker);

	return	xRet;
}

FTM_RET	FTOM_TRIGGER_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	if (pTriggerList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_MSGQ_destroy(pMsgQ);
	pMsgQ = NULL;

	FTM_LIST_iteratorStart(pTriggerList);
	while(FTM_LIST_iteratorNext(pTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pTriggerList, pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pTrigger);	
		}
	}

	FTM_LIST_final(pTriggerList);
	pTriggerList = NULL;

	FTM_LOCK_final(&xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_create
(
	FTM_TRIGGER_PTR	pInfo,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppTrigger != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	pTrigger = (FTOM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTOM_TRIGGER));
	if (pTrigger == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_LOCK_init(&pTrigger->xLock);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);	
		return	xRet;
	}

	memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));
	if (strlen(pTrigger->xInfo.pID) == 0)
	{
		FTM_makeID(pTrigger->xInfo.pID, 16);
	}

	xRet = FTM_LIST_append(pTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);
		return	xRet;
	}

	*ppTrigger = pTrigger; 

	return	xRet;
}

FTM_RET	FTOM_TRIGGER_createFromDB
(
	FTM_CHAR_PTR	pID,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pID != NULL);
	ASSERT(ppTrigger != NULL);

	FTM_RET	xRet;
	FTM_TRIGGER			xInfo;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_DB_TRIGGER_getInfo(pID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pTrigger = (FTOM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTOM_TRIGGER));
	if (pTrigger == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_LOCK_init(&pTrigger->xLock);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);	
		return	xRet;
	}

	memcpy(&pTrigger->xInfo, &xInfo, sizeof(FTM_TRIGGER));
	if (strlen(pTrigger->xInfo.pID) == 0)
	{
		FTM_makeID(pTrigger->xInfo.pID, 16);
	}

	FTM_LIST_append(pTriggerList, pTrigger);

	*ppTrigger = pTrigger; 

	return	xRet;
}

FTM_RET	FTOM_TRIGGER_destroy
(
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(ppTrigger != NULL);

	FTM_LIST_remove(pTriggerList, (*ppTrigger));

	FTM_LOCK_final(&(*ppTrigger)->xLock);

	FTM_MEM_free(*ppTrigger);
	*ppTrigger = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_start
(
	FTOM_TRIGGER_PTR	pTrigger
)
{
	FTM_INT	nRet;

	if (pTrigger == NULL)
	{
		if (bStop)
		{
			return	FTM_RET_ALREADY_STARTED;	
		}
	
		nRet = pthread_create(&xTriggerThread, NULL, FTOM_TRIGGER_process, NULL);
		if (nRet < 0)
		{
			ERROR("Can't start Trigger Manager!\n");
			return	FTM_RET_ERROR;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_stop
(
	FTOM_TRIGGER_PTR	pTrigger
)
{
	if (pTrigger == NULL)
	{
		if (!bStop)
		{
			return	FTM_RET_NOT_START;	
		}
	
		bStop = FTM_TRUE;
		pthread_join(xTriggerThread, NULL);
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_TRIGGER_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	FTM_TIMER xTimer;
	
	FTM_TIMER_initS(&xTimer, 0);

	TRACE("Trigger management process started.\n");
	while(!bStop)
	{
		FTM_RET				xRet;
		FTOM_TRIGGER_PTR	pTrigger;
		FTM_ULONG			i, ulCount;

		FTM_TIMER_addMS(&xTimer, FTOM_TRIGGER_LOOP_INTERVAL);
	
		FTM_LIST_count(pTriggerList, &ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTOM_TRIGGER_getAt(i, &pTrigger);
			if (xRet == FTM_RET_OK)
			{
				FTM_LOCK_set(&pTrigger->xLock);

				if (pTrigger->xState == FTOM_TRIGGER_STATE_PRESET)
				{
					if (FTM_TIMER_isExpired(&pTrigger->xDetectionTimer))
					{
						FTOM_MSG_PTR	pMsg;

						INFO("Trigger[%s] occurred!\n", pTrigger->xInfo.pID);

						pTrigger->xState = FTOM_TRIGGER_STATE_SET;
						FTM_TIME_getCurrent(&pTrigger->xOccurrenceTime);
						FTM_TIMER_initS(&pTrigger->xHoldingTimer, pTrigger->xInfo.xParams.xCommon.ulHoldingTime);

						FTOM_MSG_createEvent(pTrigger->xInfo.pID, FTM_TRUE, &pMsg);
						FTOM_RULE_sendMessage(pMsg);
					}
				}
				else if (pTrigger->xState == FTOM_TRIGGER_STATE_PRERESET)
				{
					if (FTM_TIMER_isExpired(&pTrigger->xHoldingTimer))
					{
						FTOM_MSG_PTR	pMsg;

						INFO("Trigger[%s] clrean!\n", pTrigger->xInfo.pID);

						pTrigger->xState = FTOM_TRIGGER_STATE_RESET;
						FTM_TIME_getCurrent(&pTrigger->xReleaseTime);

						FTOM_MSG_createEvent(pTrigger->xInfo.pID, FTM_FALSE, &pMsg);
						FTOM_RULE_sendMessage(pMsg);
					}
				}

				FTM_LOCK_reset(&pTrigger->xLock);
			}
		}
		
		if (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE)
		{
			FTM_UINT64	ulRemainUS = 0;	

			FTM_TIMER_remainUS(&xTimer, &ulRemainUS);
			usleep(ulRemainUS);
		}
	}

	TRACE("Trigger management process finished.\n");

	return	0;
}

FTM_RET FTOM_TRIGGER_count
(
	FTM_ULONG_PTR 		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pTriggerList, pulCount);
}

FTM_RET	FTOM_TRIGGER_get
(
	FTM_CHAR_PTR		pTriggerID,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerID != NULL);
	ASSERT(ppTrigger != NULL);

	return	FTM_LIST_get(pTriggerList, pTriggerID, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTOM_TRIGGER_getAt
(
	FTM_ULONG 			ulIndex, 
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(ppTrigger != NULL);

	return	FTM_LIST_getAt(pTriggerList, ulIndex, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTOM_TRIGGER_setInfo
(
	FTOM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pTrigger != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;

	TRACE("Trigger[%s] set[%08x]\n", pTrigger->xInfo.pID, xFields);
	xRet = FTOM_DB_TRIGGER_setInfo(pTrigger->xInfo.pID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger[%s] DB update failed.\n", pTrigger->xInfo.pID);	
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

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_updateEP
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_TRIGGER_PTR	pTrigger;
	FTM_ULONG			i, ulCount;
	FTM_VALUE			xValue;

	FTM_EP_DATA_toValue(pData, &xValue);

	FTM_LIST_count(pTriggerList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTOM_TRIGGER_getAt(i, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
		
		if (strcmp(pTrigger->xInfo.pEPID, pEPID) == 0)
		{
			FTM_BOOL	bOccurrence = FTM_FALSE;
			
			xRet = FTM_TRIGGER_occurred(&pTrigger->xInfo, &xValue, &bOccurrence);
			if (xRet == FTM_RET_OK)
			{
				FTM_LOCK_set(&pTrigger->xLock);
		
				switch(pTrigger->xState)
				{
				case	FTOM_TRIGGER_STATE_RESET:
					{
						if (bOccurrence)
						{
							FTM_TIMER_initS(&pTrigger->xDetectionTimer, pTrigger->xInfo.xParams.xCommon.ulDetectionTime);
							pTrigger->xState = FTOM_TRIGGER_STATE_PRESET;
						}
					}
					break;

				case	FTOM_TRIGGER_STATE_PRESET:
					{
						if (!bOccurrence)
						{
							FTM_TIMER_initS(&pTrigger->xDetectionTimer, 0);
							pTrigger->xState = FTOM_TRIGGER_STATE_RESET;
						}
					}
					break;

				case	FTOM_TRIGGER_STATE_SET:
					{
						if (!bOccurrence)
						{
							pTrigger->xState = FTOM_TRIGGER_STATE_PRERESET;
						}
					}
					break;

				case	FTOM_TRIGGER_STATE_PRERESET:
					{
						if (bOccurrence)
						{
							pTrigger->xState = FTOM_TRIGGER_STATE_SET;
						}
					}
					break;
				}

				FTM_LOCK_reset(&pTrigger->xLock);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTOM_TRIGGER_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_TRIGGER_PTR	pTrigger = (FTOM_TRIGGER_PTR)pElement;
	FTM_CHAR_PTR		pTriggerID = (FTM_CHAR_PTR)pIndicator;
	
	return	strcasecmp(pTrigger->xInfo.pID, pTriggerID) == 0;
}

FTM_RET	FTOM_TRIGGER_printList
(
	FTM_VOID
)
{
	FTM_RET				xRet;
	FTM_INT				i;
	FTM_ULONG			ulCount;

	MESSAGE("\n# Trigger Information\n");
	FTOM_TRIGGER_count(&ulCount);
	MESSAGE("%16s %16s %16s %8s %8s %32s %16s\n", "ID", "NAME", "TYPE", "DETECT", "HOLD", "CONDITION", "EPID");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_TRIGGER_PTR	pTrigger;

		xRet = FTOM_TRIGGER_getAt(i, &pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_CHAR	pCondition[1024];

			FTM_TRIGGER_conditionToString(&pTrigger->xInfo, pCondition, sizeof(pCondition));

			MESSAGE("%16s %16s %16s %8.3f %8.3f %32s %16s\n", 
				pTrigger->xInfo.pID, 
				pTrigger->xInfo.pName, 
				FTM_TRIGGER_typeString(pTrigger->xInfo.xType),
            	pTrigger->xInfo.xParams.xCommon.ulDetectionTime / 1000000.0,
				pTrigger->xInfo.xParams.xCommon.ulHoldingTime / 1000000.0,
				pCondition,
				pTrigger->xInfo.pEPID);
		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_print
(
	FTOM_TRIGGER_PTR	pTrigger
)
{
	FTM_CHAR	pCondition[1024];

	MESSAGE("\n# Trigger Information\n");
	FTM_TRIGGER_conditionToString(&pTrigger->xInfo, pCondition, sizeof(pCondition));

	MESSAGE("%16s : %s\n", 	"ID", 		pTrigger->xInfo.pID); 
	MESSAGE("%16s : %s\n", 	"Name",		pTrigger->xInfo.pName); 
	MESSAGE("%16s : %s\n", 	"Type",		FTM_TRIGGER_typeString(pTrigger->xInfo.xType));
	MESSAGE("%16s : %8.3f\n","Detect",	pTrigger->xInfo.xParams.xCommon.ulDetectionTime / 1000000.0);
	MESSAGE("%16s : %8.3f\n","Hold", 	pTrigger->xInfo.xParams.xCommon.ulHoldingTime / 1000000.0);
	MESSAGE("%16s : %s\n", 	"Condition", pCondition);
	MESSAGE("%16s : %s\n", 	"EPID", 	pTrigger->xInfo.pEPID);

	return	FTM_RET_OK;
}
