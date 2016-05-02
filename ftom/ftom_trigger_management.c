#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom_ep.h"
#include "ftom_trigger_management.h"
#include "ftom_action_management.h"
#include "ftom_rule_management.h"
#include "ftom_msg.h"
#include "libconfig.h"

#define	FTOM_TRIGGER_LOOP_INTERVAL	100000	// 1s

static 
FTM_VOID_PTR FTOM_TRIGGERM_process
(
	FTM_VOID_PTR pData
);

static 
FTM_BOOL	FTOM_TRIGGERM_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTOM_TRIGGERM_create
(
	FTOM_PTR	pOM,
	FTOM_TRIGGERM_PTR _PTR_ ppTriggerM
)
{
	ASSERT(ppTriggerM != NULL);

	FTM_RET				xRet;
	FTOM_TRIGGERM_PTR	pTriggerM;

	if (!bInit)
	{
		xRet = FTM_LIST_init(&xList);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
		bInit = FTM_TRUE;	

		if (sem_init(&xLock, 0, 1) < 0)
		{
			return	FTM_RET_ERROR;	
		}
	}

	pTriggerM = (FTOM_TRIGGERM_PTR)FTM_MEM_malloc(sizeof(FTOM_TRIGGERM));
	if (pTriggerM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_TRIGGERM_init(pTriggerM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create trigger manager!\n");
		FTM_MEM_free(pTriggerM);	
	}
	else
	{
		pTriggerM->pOM = pOM;
		FTM_LIST_append(&xList, pTriggerM);

		*ppTriggerM = pTriggerM;	
	}

	return	xRet;
}

FTM_RET	FTOM_TRIGGERM_destroy
(
	FTOM_TRIGGERM_PTR _PTR_ ppTriggerM
)
{
	ASSERT(ppTriggerM != NULL);
	
	FTM_RET	xRet;
	
	if (*ppTriggerM == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTM_LIST_remove(&xList, *ppTriggerM);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTOM_TRIGGERM_final(*ppTriggerM);
	FTM_MEM_free(*ppTriggerM);

	*ppTriggerM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGERM_init
(
	FTOM_TRIGGERM_PTR pTriggerM
)
{
	ASSERT(pTriggerM);

	FTM_RET	xRet;
	
	FTM_TRIGGER_init();

	memset(pTriggerM, 0, sizeof(FTOM_TRIGGERM));
	xRet = FTM_MSGQ_create(&pTriggerM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_init(&pTriggerM->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pTriggerM->pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(&pTriggerM->xTriggerList, FTOM_TRIGGERM_seeker);


	TRACE("Trigger management initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGERM_final
(
	FTOM_TRIGGERM_PTR pTriggerM
)
{
	ASSERT(pTriggerM != NULL);

	FTM_RET			xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	FTM_MSGQ_destroy(pTriggerM->pMsgQ);
	pTriggerM->pMsgQ = NULL;


	FTM_LIST_iteratorStart(&pTriggerM->xTriggerList);
	while(FTM_LIST_iteratorNext(&pTriggerM->xTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(&pTriggerM->xTriggerList, pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pTrigger);	
		}
	}

	FTM_LIST_final(&pTriggerM->xTriggerList);

	FTM_TRIGGER_final();

	TRACE("Trigger management finalized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGER_loadConfig
(
	FTOM_TRIGGERM_PTR 	pTriggerM, 
	FTM_CHAR_PTR 		pFileName
)
{
	ASSERT(pTriggerM != NULL);
	ASSERT(pFileName != NULL);

#if 0
	config_t			xConfig;
	config_setting_t	*pSection;
	
	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "event");
	if (pSection != NULL)
	{
		config_setting_t	*pField;
		config_setting_t	*pList;

		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pTriggerM->xConfig.usPort =  config_setting_get_int(pField);
		}

		pList = config_setting_get_member(pSection, "traps");
		if (pList != NULL)
        {
			FTM_ULONG	i, ulCount;
			
			ulCount = config_setting_length(pList);
			for(i = 0 ; i < ulCount ; i++)
			{
				pField = config_setting_get_elem(pList, i);
				if (pField != NULL)
				{
					FTOM_SNMP_OID	xOID;
					
					xOID.ulOIDLen = FTOM_SNMP_OID_LENGTH;

					MESSAGE("pObID = %s\n", config_setting_get_string(pField));
					if (read_objid(config_setting_get_string(pField), xOID.pOID, (size_t *)&xOID.ulOIDLen) == 1)
					{
						MESSAGE("SNMP_PARSE_OID success!\n");
						FTOM_SNMPTRAPD_addTrapOID(pTriggerM, &xOID);
					}
				
				}
			}
		}

	}
	config_destroy(&xConfig);
#endif
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGERM_start
(
	FTOM_TRIGGERM_PTR pTriggerM
)
{
	ASSERT(pTriggerM != NULL);
	
	FTM_INT	nRet;

	if (pTriggerM->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	nRet = pthread_create(&pTriggerM->xTriggerThread, NULL, FTOM_TRIGGERM_process, pTriggerM);
	if (nRet < 0)
	{
		ERROR("Can't start Trigger Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TRIGGERM_stop
(
	FTOM_TRIGGERM_PTR pTriggerM
)
{
	ASSERT(pTriggerM != NULL);


	if (!pTriggerM->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pTriggerM->bStop = FTM_TRUE;
	pthread_join(pTriggerM->xTriggerThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_TRIGGERM_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);
	FTOM_TRIGGERM_PTR	pTriggerM = (FTOM_TRIGGERM_PTR)pData;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	TRACE("Trigger management process started.\n");
	while(!pTriggerM->bStop)
	{
		FTM_RET				xRet;
		FTOM_TRIGGER_PTR		pTrigger;
		FTM_ULONG			i, ulCount;

		FTM_TIMER_add(&xTimer, FTOM_TRIGGER_LOOP_INTERVAL);
	
		FTM_LIST_count(&pTriggerM->xTriggerList, &ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTOM_TRIGGERM_getAt(pTriggerM, i, &pTrigger);
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

						FTOM_RULEM_notifyChanged(pTrigger->xInfo.pID);
					}
				}
				else if (pTrigger->xState == FTOM_TRIGGER_STATE_PRERESET)
				{
					if (FTM_TIMER_isExpired(&pTrigger->xHoldingTimer))
					{
						INFO("Trigger[%s] clrean!\n", pTrigger->xInfo.pID);
						pTrigger->xState = FTOM_TRIGGER_STATE_RESET;
						FTM_TIME_getCurrent(&pTrigger->xReleaseTime);

						FTOM_RULEM_notifyChanged(pTrigger->xInfo.pID);
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

FTM_RET FTOM_TRIGGERM_count
(
	FTOM_TRIGGERM_PTR 	pTriggerM, 
	FTM_ULONG_PTR 		pulCount
)
{
	ASSERT(pTriggerM != NULL);

	return	FTM_LIST_count(&pTriggerM->xTriggerList, pulCount);
}

FTM_RET	FTOM_TRIGGERM_createTrigger
(
	FTOM_TRIGGERM_PTR 	pTriggerM, 
	FTM_TRIGGER_PTR 	pInfo,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerM != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(ppTrigger != NULL);

	FTM_RET			xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_create(pInfo, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_LIST_append(&pTriggerM->xTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		FTOM_TRIGGER_destroy(&pTrigger);
		return	xRet;
	}
	
	*ppTrigger = pTrigger;

	return	xRet;
}

FTM_RET	FTOM_TRIGGERM_destroyTrigger
(
	FTOM_TRIGGERM_PTR 	pTriggerM, 
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerM != NULL);
	ASSERT(ppTrigger != NULL);

	FTM_RET			xRet;

	xRet = FTM_LIST_remove(&pTriggerM->xTriggerList, *ppTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTOM_TRIGGER_destroy(ppTrigger);
}

FTM_RET	FTOM_TRIGGERM_get
(
	FTOM_TRIGGERM_PTR 	pTriggerM, 
	FTM_CHAR_PTR		pTriggerID,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerM != NULL);

	return	FTM_LIST_get(&pTriggerM->xTriggerList, pTriggerID, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTOM_TRIGGERM_getAt
(
	FTOM_TRIGGERM_PTR 	pTriggerM, 
	FTM_ULONG 			ulIndex, 
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pTriggerM != NULL);

	return	FTM_LIST_getAt(&pTriggerM->xTriggerList, ulIndex, (FTM_VOID_PTR _PTR_)ppTrigger);
}


FTM_RET	FTOM_TRIGGERM_updateEP
(
	FTOM_TRIGGERM_PTR	pTriggerM, 
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pTriggerM != NULL);

	FTM_RET				xRet;
	FTOM_TRIGGER_PTR	pTrigger;
	FTM_ULONG			i, ulCount;
	FTM_VALUE			xValue;

	FTM_EP_DATA_toValue(pData, &xValue);

	FTM_LIST_count(&pTriggerM->xTriggerList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTOM_TRIGGERM_getAt(pTriggerM, i, &pTrigger);
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

FTM_BOOL	FTOM_TRIGGERM_seeker
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

