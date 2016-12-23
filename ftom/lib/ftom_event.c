#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_db.h"
#include "ftom_ep.h"
#include "ftom_event.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_msg.h"
#include "libconfig.h"
#include "ftom_logger.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_EVENT

static 
FTM_VOID_PTR FTOM_EVENT_process
(
	FTM_VOID_PTR pData
);

static 
FTM_BOOL	FTOM_EVENT_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_LOCK				xLock;
static FTM_LIST_PTR			pEventList = NULL;
static FTM_MSG_QUEUE_PTR	pMsgQ = NULL;
static FTM_BOOL				bStop = FTM_TRUE;
static pthread_t			xEventThread;

FTM_RET	FTOM_EVENT_init
(
	FTM_VOID
)
{
	FTM_RET				xRet;

	if (pEventList != NULL)
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
		ERROR2(xRet, "Message Queue creation failed.\n");
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pEventList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Event list creation failed.\n");
		FTM_MSGQ_destroy(pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pEventList, FTOM_EVENT_seeker);

	return	xRet;
}

FTM_RET	FTOM_EVENT_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;
	FTOM_EVENT_PTR	pEvent;

	if (pEventList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_MSGQ_destroy(pMsgQ);
	pMsgQ = NULL;

	FTM_LIST_iteratorStart(pEventList);
	while(FTM_LIST_iteratorNext(pEventList, (FTM_VOID_PTR _PTR_)&pEvent) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pEventList, pEvent);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pEvent);	
		}
	}

	FTM_LIST_final(pEventList);
	pEventList = NULL;

	FTM_LOCK_final(&xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EVENT_create
(
	FTM_EVENT_PTR	pInfo,
	FTM_BOOL		bNew,
	FTOM_EVENT_PTR _PTR_ ppEvent
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppEvent != NULL);

	FTM_RET	xRet;
	FTOM_EVENT_PTR	pEvent;

	pEvent = (FTOM_EVENT_PTR)FTM_MEM_malloc(sizeof(FTOM_EVENT));
	if (pEvent == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Not enough memory\n");
		return	xRet;
	}

	memcpy(&pEvent->xInfo, pInfo, sizeof(FTM_EVENT));
	if (strlen(pEvent->xInfo.pID) == 0)
	{
		FTM_makeID(pEvent->xInfo.pID, 16);
	}

	xRet = FTM_LOCK_init(&pEvent->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initalize lock of trigger[%s].\n", pEvent->xInfo.pID);
		goto error1;
	}

	xRet = FTM_LIST_append(pEventList, pEvent);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to add trigger[%s] to list.\n", pEvent->xInfo.pID);
		goto error2;
	}

	if (bNew)
	{
		xRet = FTOM_DB_EVENT_add(&pEvent->xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add trigger[%s] to DB.\n", pEvent->xInfo.pID);
		}
		else
		{
			FTOM_addEventCreationLog(pEvent);
		}
	}

	*ppEvent = pEvent; 

	return	FTM_RET_OK;

error2:
	FTM_LOCK_final(&pEvent->xLock);

error1:
	FTM_MEM_free(pEvent);

	return	xRet;
}

FTM_RET	FTOM_EVENT_destroy
(
	FTOM_EVENT_PTR _PTR_ ppEvent,
	FTM_BOOL		bIncludeDB
)
{
	ASSERT(ppEvent != NULL);
	FTM_RET	xRet;

	if (bIncludeDB)
	{
		xRet = FTOM_DB_EVENT_remove((*ppEvent)->xInfo.pID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove trigger[%s] from DB!\n", (*ppEvent)->xInfo.pID);	
		}
		else
		{
			FTOM_addEventRemovalLog((*ppEvent)->xInfo.pID);
		}
	}

	xRet = FTM_LIST_remove(pEventList, (*ppEvent));
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove trigger[%s] from list!\n", (*ppEvent)->xInfo.pID);	
	}

	xRet = FTM_LOCK_final(&(*ppEvent)->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize lock of trigger[%s]!\n", (*ppEvent)->xInfo.pID);	
	}

	FTM_MEM_free(*ppEvent);
	*ppEvent = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EVENT_start
(
	FTOM_EVENT_PTR	pEvent
)
{
	FTM_RET	xRet;
	FTM_INT	nRet;

	if (pEvent == NULL)
	{
		if (bStop)
		{
			return	FTM_RET_ALREADY_STARTED;	
		}
	
		nRet = pthread_create(&xEventThread, NULL, FTOM_EVENT_process, NULL);
		if (nRet < 0)
		{
			xRet = FTM_RET_THREAD_CREATION_ERROR;
			ERROR2(xRet, "Can't start Event Manager!\n");
			return	xRet;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EVENT_stop
(
	FTOM_EVENT_PTR	pEvent
)
{
	if (pEvent == NULL)
	{
		if (!bStop)
		{
			return	FTM_RET_NOT_START;	
		}
	
		bStop = FTM_TRUE;
		pthread_join(xEventThread, NULL);
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_EVENT_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	FTM_TIMER xTimer;
	
	FTM_TIMER_initS(&xTimer, 0);

	TRACE("Event management process started.\n");
	while(!bStop)
	{
		FTM_RET				xRet;
		FTOM_EVENT_PTR	pEvent;
		FTM_ULONG			i, ulCount;

		FTM_TIMER_addMS(&xTimer, FTOM_EVENT_LOOP_INTERVAL);
	
		FTM_LIST_count(pEventList, &ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTOM_EVENT_getAt(i, &pEvent);
			if (xRet == FTM_RET_OK)
			{
				FTM_LOCK_set(&pEvent->xLock);

				if (pEvent->xState == FTOM_EVENT_STATE_PRESET)
				{
					if (FTM_TIMER_isExpired(&pEvent->xDetectionTimer))
					{
						FTOM_MSG_PTR	pMsg;

						INFO("Event[%s] occurred!\n", pEvent->xInfo.pID);

						pEvent->xState = FTOM_EVENT_STATE_SET;
						FTM_TIME_getCurrent(&pEvent->xOccurrenceTime);
						FTM_TIMER_initS(&pEvent->xHoldingTimer, pEvent->xInfo.xParams.xCommon.ulHoldingTime);

						FTOM_MSG_createEvent(pEvent, pEvent->xInfo.pID, FTM_TRUE, &pMsg);
						FTOM_RULE_sendMessage(pMsg);
					}
				}
				else if (pEvent->xState == FTOM_EVENT_STATE_PRERESET)
				{
					if (FTM_TIMER_isExpired(&pEvent->xHoldingTimer))
					{
						FTOM_MSG_PTR	pMsg;

						INFO("Event[%s] clrean!\n", pEvent->xInfo.pID);

						pEvent->xState = FTOM_EVENT_STATE_RESET;
						FTM_TIME_getCurrent(&pEvent->xReleaseTime);

						FTOM_MSG_createEvent(pEvent, pEvent->xInfo.pID, FTM_FALSE, &pMsg);
						FTOM_RULE_sendMessage(pMsg);
					}
				}

				FTM_LOCK_reset(&pEvent->xLock);
			}
		}
		
		if (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE)
		{
			FTM_UINT64	ulRemainUS = 0;	

			FTM_TIMER_remainUS(&xTimer, &ulRemainUS);
			usleep(ulRemainUS);
		}
	}

	TRACE("Event management process finished.\n");

	return	0;
}

FTM_RET FTOM_EVENT_count
(
	FTM_ULONG_PTR 		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pEventList, pulCount);
}

FTM_RET	FTOM_EVENT_get
(
	FTM_CHAR_PTR		pEventID,
	FTOM_EVENT_PTR _PTR_ ppEvent
)
{
	ASSERT(pEventID != NULL);
	ASSERT(ppEvent != NULL);

	return	FTM_LIST_get(pEventList, pEventID, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTOM_EVENT_getAt
(
	FTM_ULONG 			ulIndex, 
	FTOM_EVENT_PTR _PTR_ ppEvent
)
{
	ASSERT(ppEvent != NULL);

	return	FTM_LIST_getAt(pEventList, ulIndex, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTOM_EVENT_setInfo
(
	FTOM_EVENT_PTR	pEvent,
	FTM_EVENT_FIELD	xFields,
	FTM_EVENT_PTR		pInfo
)
{
	ASSERT(pEvent != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;

	xRet = FTOM_DB_EVENT_setInfo(pEvent->xInfo.pID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set trigger[%s] on DB.\n", pEvent->xInfo.pID);	
	}

	if (xFields & FTM_EVENT_FIELD_NAME)
	{
		strcpy(pEvent->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_EVENT_FIELD_EPID)
	{
		strcpy(pEvent->xInfo.pEPID, pInfo->pEPID);
	}

	if (xFields & FTM_EVENT_FIELD_DETECT_TIME)
	{
		pEvent->xInfo.xParams.xCommon.ulDetectionTime = pInfo->xParams.xCommon.ulDetectionTime;
	}

	if (xFields & FTM_EVENT_FIELD_HOLD_TIME)
	{
		pEvent->xInfo.xParams.xCommon.ulHoldingTime = pInfo->xParams.xCommon.ulHoldingTime;
	}

	if (xFields & FTM_EVENT_FIELD_VALUE)
	{
		memcpy(&pEvent->xInfo.xParams.xAbove.xValue, &pInfo->xParams.xAbove.xValue, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_EVENT_FIELD_LOWER)
	{
		memcpy(&pEvent->xInfo.xParams.xInclude.xLower, &pInfo->xParams.xInclude.xLower, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_EVENT_FIELD_UPPER)
	{
		memcpy(&pEvent->xInfo.xParams.xInclude.xUpper, &pInfo->xParams.xInclude.xUpper, sizeof(FTM_VALUE));
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EVENT_updateEP
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_EVENT_PTR	pEvent;
	FTM_ULONG			i, ulCount;
	FTM_VALUE			xValue;

	FTM_EP_DATA_toValue(pData, &xValue);

	FTM_LIST_count(pEventList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTOM_EVENT_getAt(i, &pEvent);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
		
		if (strcmp(pEvent->xInfo.pEPID, pEPID) == 0)
		{
			FTM_BOOL	bOccurrence = FTM_FALSE;
			
			xRet = FTM_EVENT_occurred(&pEvent->xInfo, &xValue, &bOccurrence);
			if (xRet == FTM_RET_OK)
			{
				FTM_LOCK_set(&pEvent->xLock);
		
				switch(pEvent->xState)
				{
				case	FTOM_EVENT_STATE_RESET:
					{
						if (bOccurrence)
						{
							FTM_TIMER_initS(&pEvent->xDetectionTimer, pEvent->xInfo.xParams.xCommon.ulDetectionTime);
							pEvent->xState = FTOM_EVENT_STATE_PRESET;
						}
					}
					break;

				case	FTOM_EVENT_STATE_PRESET:
					{
						if (!bOccurrence)
						{
							FTM_TIMER_initS(&pEvent->xDetectionTimer, 0);
							pEvent->xState = FTOM_EVENT_STATE_RESET;
						}
					}
					break;

				case	FTOM_EVENT_STATE_SET:
					{
						if (!bOccurrence)
						{
							pEvent->xState = FTOM_EVENT_STATE_PRERESET;
						}
					}
					break;

				case	FTOM_EVENT_STATE_PRERESET:
					{
						if (bOccurrence)
						{
							pEvent->xState = FTOM_EVENT_STATE_SET;
						}
					}
					break;
				}

				FTM_LOCK_reset(&pEvent->xLock);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTOM_EVENT_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_EVENT_PTR	pEvent = (FTOM_EVENT_PTR)pElement;
	FTM_CHAR_PTR		pEventID = (FTM_CHAR_PTR)pIndicator;
	
	return	strcasecmp(pEvent->xInfo.pID, pEventID) == 0;
}

FTM_RET	FTOM_EVENT_printList
(
	FTM_VOID
)
{
	FTM_RET				xRet;
	FTM_INT				i;
	FTM_ULONG			ulCount;

	MESSAGE("\n# Event Information\n");
	FTOM_EVENT_count(&ulCount);
	MESSAGE("%16s %16s %16s %8s %8s %32s %16s\n", "ID", "NAME", "TYPE", "DETECT", "HOLD", "CONDITION", "EPID");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_EVENT_PTR	pEvent;

		xRet = FTOM_EVENT_getAt(i, &pEvent);
		if (xRet == FTM_RET_OK)
		{
			FTM_CHAR	pCondition[1024];

			FTM_EVENT_conditionToString(&pEvent->xInfo, pCondition, sizeof(pCondition));

			MESSAGE("%16s %16s %16s %8.3f %8.3f %32s %16s\n", 
				pEvent->xInfo.pID, 
				pEvent->xInfo.pName, 
				FTM_EVENT_typeString(pEvent->xInfo.xType),
            	pEvent->xInfo.xParams.xCommon.ulDetectionTime / 1000000.0,
				pEvent->xInfo.xParams.xCommon.ulHoldingTime / 1000000.0,
				pCondition,
				pEvent->xInfo.pEPID);
		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EVENT_print
(
	FTOM_EVENT_PTR	pEvent
)
{
	FTM_CHAR	pCondition[1024];

	MESSAGE("\n# Event Information\n");
	FTM_EVENT_conditionToString(&pEvent->xInfo, pCondition, sizeof(pCondition));

	MESSAGE("%16s : %s\n", 	"ID", 		pEvent->xInfo.pID); 
	MESSAGE("%16s : %s\n", 	"Name",		pEvent->xInfo.pName); 
	MESSAGE("%16s : %s\n", 	"Type",		FTM_EVENT_typeString(pEvent->xInfo.xType));
	MESSAGE("%16s : %8.3f\n","Detect",	pEvent->xInfo.xParams.xCommon.ulDetectionTime / 1000000.0);
	MESSAGE("%16s : %8.3f\n","Hold", 	pEvent->xInfo.xParams.xCommon.ulHoldingTime / 1000000.0);
	MESSAGE("%16s : %s\n", 	"Condition", pCondition);
	MESSAGE("%16s : %s\n", 	"EPID", 	pEvent->xInfo.pEPID);

	return	FTM_RET_OK;
}
