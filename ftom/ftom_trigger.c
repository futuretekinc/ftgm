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

#define	FTOM_TRIGGER_LOOP_INTERVAL	100000	// 1s

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

static sem_t				xLock;
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


	if (sem_init(&xLock, 0, 1) < 0)
	{
		return	FTM_RET_ERROR;	
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
	FTM_VOID
)
{
	FTM_INT	nRet;

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

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_stop
(
	FTM_VOID
)
{
	if (!bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	bStop = FTM_TRUE;
	pthread_join(xTriggerThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_TRIGGER_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	FTM_TIMER xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	TRACE("Trigger management process started.\n");
	while(!bStop)
	{
		FTM_RET				xRet;
		FTOM_TRIGGER_PTR	pTrigger;
		FTM_ULONG			i, ulCount;

		FTM_TIMER_add(&xTimer, FTOM_TRIGGER_LOOP_INTERVAL);
	
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
						INFO("Trigger[%s] occurred!\n", pTrigger->xInfo.pID);
						pTrigger->xState = FTOM_TRIGGER_STATE_SET;
						FTM_TIME_getCurrent(&pTrigger->xOccurrenceTime);
						FTM_TIMER_init(&pTrigger->xHoldingTimer, pTrigger->xInfo.xParams.xCommon.ulHoldingTime);

						FTOM_RULE_notifyChanged(pTrigger->xInfo.pID);
					}
				}
				else if (pTrigger->xState == FTOM_TRIGGER_STATE_PRERESET)
				{
					if (FTM_TIMER_isExpired(&pTrigger->xHoldingTimer))
					{
						INFO("Trigger[%s] clrean!\n", pTrigger->xInfo.pID);
						pTrigger->xState = FTOM_TRIGGER_STATE_RESET;
						FTM_TIME_getCurrent(&pTrigger->xReleaseTime);

						FTOM_RULE_notifyChanged(pTrigger->xInfo.pID);
					}
				}

				FTM_LOCK_reset(&pTrigger->xLock);
			}
		}
		
		if (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE)
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);
			usleep(ulRemain);
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
							FTM_TIMER_init(&pTrigger->xDetectionTimer, pTrigger->xInfo.xParams.xCommon.ulDetectionTime);
							pTrigger->xState = FTOM_TRIGGER_STATE_PRESET;
						}
					}
					break;

				case	FTOM_TRIGGER_STATE_PRESET:
					{
						if (!bOccurrence)
						{
							FTM_TIMER_init(&pTrigger->xDetectionTimer, 0);
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

