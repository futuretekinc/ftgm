#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftnm_ep.h"
#include "ftnm_trigger.h"
#include "ftnm_action.h"
#include "ftnm_msg.h"
#include "libconfig.h"

#define	FTNM_TRIGGER_LOOP_INTERVAL	100000	// 1000 us

#if 0
#define	TRACE_CALL()	TRACE("%s[%d]\n", __func__, __LINE__)
#else
#define	TRACE_CALL()
#endif

static FTM_VOID_PTR FTNM_TRIGGERM_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTNM_TRIGGERM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTM_BOOL	bInit = FTM_FALSE;
static FTM_LIST	xList;
static sem_t	xLock;

FTM_RET	FTNM_TRIGGERM_create(FTNM_TRIGGERM_PTR _PTR_ ppTriggerM)
{
	ASSERT(ppTriggerM != NULL);

	FTM_RET				xRet;
	FTNM_TRIGGERM_PTR	pTriggerM;

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


	pTriggerM = (FTNM_TRIGGERM_PTR)FTM_MEM_malloc(sizeof(FTNM_TRIGGERM));
	if (pTriggerM == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTNM_TRIGGERM_init(pTriggerM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTriggerM);	
	}
	else
	{
		FTM_LIST_append(&xList, pTriggerM);

		*ppTriggerM = pTriggerM;	
	}

	return	xRet;
}

FTM_RET	FTNM_TRIGGERM_destroy(FTNM_TRIGGERM_PTR _PTR_ ppTriggerM)
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

	FTNM_TRIGGERM_final(*ppTriggerM);
	FTM_MEM_free(*ppTriggerM);

	*ppTriggerM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIGGERM_init(FTNM_TRIGGERM_PTR pTriggerM)
{
	ASSERT(pTriggerM);

	FTM_RET	xRet;
	
	TRACE_CALL();

	FTM_TRIGGER_init();

	memset(pTriggerM, 0, sizeof(FTNM_TRIGGERM));
	xRet = FTM_MSGQ_create(&pTriggerM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_init(&pTriggerM->xEventList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Event list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pTriggerM->pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(&pTriggerM->xEventList, FTNM_TRIGGERM_seeker);


	TRACE("Trigger management initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIGGERM_final(FTNM_TRIGGERM_PTR pTriggerM)
{
	ASSERT(pTriggerM != NULL);

	FTM_RET			xRet;
	FTNM_TRIGGER_PTR	pTrigger;

	TRACE_CALL();

	FTM_MSGQ_destroy(pTriggerM->pMsgQ);
	pTriggerM->pMsgQ = NULL;


	FTM_LIST_iteratorStart(&pTriggerM->xEventList);
	while(FTM_LIST_iteratorNext(&pTriggerM->xEventList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(&pTriggerM->xEventList, pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pTrigger);	
		}
	}

	FTM_LIST_final(&pTriggerM->xEventList);

	FTM_TRIGGER_final();

	TRACE("Trigger management finalized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIGGER_loadConfig(FTNM_TRIGGERM_PTR pTriggerM, FTM_CHAR_PTR pFileName)
{
	ASSERT(pTriggerM != NULL);
	ASSERT(pFileName != NULL);

	TRACE_CALL();

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
					FTNM_SNMP_OID	xOID;
					
					xOID.ulOIDLen = FTNM_SNMP_OID_LENGTH;

					MESSAGE("pObID = %s\n", config_setting_get_string(pField));
					if (read_objid(config_setting_get_string(pField), xOID.pOID, (size_t *)&xOID.ulOIDLen) == 1)
					{
						MESSAGE("SNMP_PARSE_OID success!\n");
						FTNM_SNMPTRAPD_addTrapOID(pTriggerM, &xOID);
					}
				
				}
			}
		}

	}
	config_destroy(&xConfig);
#endif
	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIGGERM_start(FTNM_TRIGGERM_PTR pTriggerM)
{
	ASSERT(pTriggerM != NULL);
	
	FTM_INT	nRet;

	TRACE_CALL();

	nRet = pthread_create(&pTriggerM->xEventThread, NULL, FTNM_TRIGGERM_process, pTriggerM);
	if (nRet < 0)
	{
		ERROR("Can't start Event Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIGGERM_stop(FTNM_TRIGGERM_PTR pTriggerM)
{
	ASSERT(pTriggerM != NULL);

	TRACE_CALL();

	pTriggerM->bStop = FTM_TRUE;
	pthread_join(pTriggerM->xEventThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_TRIGGERM_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_TRIGGERM_PTR	pTriggerM = (FTNM_TRIGGERM_PTR)pData;
	FTM_TIMER				xTimer;
	
	TRACE_CALL();

	FTM_TIMER_init(&xTimer, 0);

	TRACE("Trigger management process started.\n");
	while(!pTriggerM->bStop)
	{
		FTM_RET				xRet;
		FTNM_TRIGGER_PTR		pTrigger;
		FTM_ULONG			i, ulCount;

		FTM_TIMER_add(&xTimer, FTNM_TRIGGER_LOOP_INTERVAL);
	
		FTM_LIST_count(&pTriggerM->xEventList, &ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTNM_TRIGGERM_getAt(pTriggerM, i, &pTrigger);
			if (xRet == FTM_RET_OK)
			{
				FTM_LOCK_set(&pTrigger->xLock);

				if (pTrigger->xState == FTNM_TRIGGER_STATE_PRESET)
				{
					if (FTM_TIMER_isExpired(&pTrigger->xDetectionTimer))
					{
						FTM_LOG(FTM_LOG_TYPE_TRIGGER, "Trigger[%d] occurred!\n", pTrigger->xInfo.xID);
						pTrigger->xState = FTNM_TRIGGER_STATE_SET;
						FTM_TIME_getCurrent(&pTrigger->xOccurrenceTime);
						FTM_TIMER_initTime(&pTrigger->xHoldingTimer, &pTrigger->xInfo.xParams.xCommon.xHoldingTime);

						FTNM_RULEM_notifyChanged(pTrigger->xInfo.xID);
					}
				}
				else if (pTrigger->xState == FTNM_TRIGGER_STATE_PRERESET)
				{
					if (FTM_TIMER_isExpired(&pTrigger->xHoldingTimer))
					{
						FTM_LOG(FTM_LOG_TYPE_TRIGGER, "Trigger[%d] clrean!\n", pTrigger->xInfo.xID);
						pTrigger->xState = FTNM_TRIGGER_STATE_RESET;
						FTM_TIME_getCurrent(&pTrigger->xReleaseTime);

						FTNM_RULEM_notifyChanged(pTrigger->xInfo.xID);
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

FTM_RET FTNM_TRIGGERM_count(FTNM_TRIGGERM_PTR pTriggerM, FTM_ULONG_PTR pulCount)
{
	ASSERT(pTriggerM != NULL);

	TRACE_CALL();

	return	FTM_LIST_count(&pTriggerM->xEventList, pulCount);
}

FTM_RET	FTNM_TRIGGERM_add(FTNM_TRIGGERM_PTR pTriggerM, FTM_TRIGGER_PTR pInfo)
{
	ASSERT(pTriggerM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTNM_TRIGGER_PTR	pTrigger;

	TRACE_CALL();

	pTrigger = (FTNM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTNM_TRIGGER));
	if (pTrigger == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pTrigger, 0, sizeof(FTNM_TRIGGER));
	memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));
	
	FTM_LOCK_init(&pTrigger->xLock);

	xRet = FTM_LIST_append(&pTriggerM->xEventList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);	
	}

	return	xRet;
}

FTM_RET	FTNM_TRIGGERM_del(FTNM_TRIGGERM_PTR pTriggerM, FTNM_TRIGGER_ID  xEventID)
{
	ASSERT(pTriggerM != NULL);

	FTM_RET			xRet;
	FTNM_TRIGGER_PTR	pTrigger;

	TRACE_CALL();

	xRet = FTM_LIST_get(&pTriggerM->xEventList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)&pTrigger);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(&pTriggerM->xEventList, pTrigger);

		FTM_LOCK_final(&pTrigger->xLock);
		FTM_MEM_free(pTrigger);
	}

	return	xRet;	
}

FTM_RET	FTNM_TRIGGERM_get(FTNM_TRIGGERM_PTR pTriggerM, FTNM_TRIGGER_ID xEventID, FTNM_TRIGGER_PTR _PTR_ ppTrigger)
{
	ASSERT(pTriggerM != NULL);

	TRACE_CALL();

	return	FTM_LIST_get(&pTriggerM->xEventList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTNM_TRIGGERM_getAt(FTNM_TRIGGERM_PTR pTriggerM, FTM_ULONG ulIndex, FTNM_TRIGGER_PTR _PTR_ ppTrigger)
{
	ASSERT(pTriggerM != NULL);

	TRACE_CALL();

	return	FTM_LIST_getAt(&pTriggerM->xEventList, ulIndex, (FTM_VOID_PTR _PTR_)ppTrigger);
}


FTM_RET	FTNM_TRIGGERM_updateEP(FTNM_TRIGGERM_PTR pTriggerM, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pTriggerM != NULL);

	FTM_RET				xRet;
	FTNM_TRIGGER_PTR		pTrigger;
	FTM_ULONG			i, ulCount;

	TRACE_CALL();

	FTM_LIST_count(&pTriggerM->xEventList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTNM_TRIGGERM_getAt(pTriggerM, i, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
		
		if (pTrigger->xInfo.xEPID == xEPID)
		{
			FTM_BOOL	bOccurrence = FTM_FALSE;

			xRet = FTM_TRIGGER_occurred(&pTrigger->xInfo, pData, &bOccurrence);
			if (xRet == FTM_RET_OK)
			{
				FTM_LOCK_set(&pTrigger->xLock);
		
				switch(pTrigger->xState)
				{
				case	FTNM_TRIGGER_STATE_RESET:
					{
						if (bOccurrence)
						{
							FTM_TIMER_initTime(&pTrigger->xDetectionTimer, &pTrigger->xInfo.xParams.xCommon.xDetectionTime);
							pTrigger->xState = FTNM_TRIGGER_STATE_PRESET;
						}
					}
					break;

				case	FTNM_TRIGGER_STATE_PRESET:
					{
						if (!bOccurrence)
						{
							FTM_TIMER_init(&pTrigger->xDetectionTimer, 0);
							pTrigger->xState = FTNM_TRIGGER_STATE_RESET;
						}
					}
					break;

				case	FTNM_TRIGGER_STATE_SET:
					{
						if (!bOccurrence)
						{
							pTrigger->xState = FTNM_TRIGGER_STATE_PRERESET;
						}
					}
					break;

				case	FTNM_TRIGGER_STATE_PRERESET:
					{
						if (bOccurrence)
						{
							pTrigger->xState = FTNM_TRIGGER_STATE_SET;
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

FTM_BOOL	FTNM_TRIGGERM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	TRACE_CALL();

	return	((FTNM_TRIGGER_PTR)pElement)->xInfo.xID == *((FTM_TRIGGER_ID_PTR)pIndicator);
}

