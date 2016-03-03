#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftm_timer.h"
#include "ftm_act.h"
#include "ftnm_ep.h"
#include "ftnm_trigger.h"
#include "ftnm_actor.h"
#include "ftnm_msg.h"
#include "libconfig.h"

#define	FTNM_TRIG_LOOP_INTERVAL	1000	// 1000 us

typedef enum
{
	FTNM_TRIG_MSG_TYPE_UPDATE_EP = 0
}	FTNM_TRIG_MSG_TYPE, _PTR_ FTNM_TRIG_MSG_TYPE_PTR;

typedef	struct
{
	FTNM_TRIG_MSG_TYPE	xType;
	FTNM_ACTOR_ID		xActID;
	union
	{
		struct
		{
			FTM_EPID	xEPID;
			FTM_EP_DATA	xLastData;
			FTM_EP_DATA	xNewData;
		}	xUpdateEP;
	}	xParams;
}	FTNM_TRIG_MSG, _PTR_ FTNM_TRIG_MSG_PTR;

static FTM_VOID_PTR FTNM_TRIG_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTNM_TRIG_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTNM_TRIG_MANAGER_PTR	pCTX = NULL;
static FTM_LIST_PTR				pTriggerList = NULL;

FTM_RET	FTNM_TRIG_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pCTX != NULL)
	{
		ERROR("The event manager is  not initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pCTX = (FTNM_TRIG_MANAGER_PTR)FTM_MEM_malloc(sizeof(FTNM_TRIG_MANAGER));
	if (pCTX == NULL)
	{
		ERROR("Can't allocated memory[%d].\n", sizeof(FTNM_TRIG_MANAGER));
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pCTX, 0, sizeof(FTNM_TRIG_MANAGER));

	xRet = FTM_MSGQ_create(&pCTX->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pCTX->pMsgQ);
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pTriggerList, FTNM_TRIG_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIG_final(FTM_VOID)
{
	FTM_RET			xRet;
	FTNM_TRIG_PTR	pTrigger;

	if (pCTX == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_MSGQ_destroy(pCTX->pMsgQ);
	pCTX->pMsgQ = NULL;


	FTM_LIST_iteratorStart(pTriggerList);
	while(FTM_LIST_iteratorNext(pTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pTriggerList, pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pTrigger);	
		}
	}

	FTM_MEM_free(pCTX);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIG_loadConfig(FTM_CHAR_PTR pFileName)
{
	ASSERT(pCTX != NULL);
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
			pCTX->xConfig.usPort =  config_setting_get_int(pField);
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
						FTNM_SNMPTRAPD_addTrapOID(pCTX, &xOID);
					}
				
				}
			}
		}

	}
	config_destroy(&xConfig);
#endif
	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIG_start(FTM_VOID)
{
	ASSERT(pCTX != NULL);
	
	FTM_INT	nRet;

	nRet = pthread_create(&pCTX->xPThread, NULL, FTNM_TRIG_process, pCTX);
	if (nRet < 0)
	{
		ERROR("Can't start Event Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TRIG_stop(FTM_VOID)
{
	ASSERT(pCTX != NULL);

	pCTX->bStop = FTM_TRUE;
	pthread_join(pCTX->xPThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_TRIG_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_TRIG_MANAGER_PTR	pCTX = (FTNM_TRIG_MANAGER_PTR)pData;
	FTM_RET					xRet;
	FTNM_TRIG_MSG_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	while(!pCTX->bStop)
	{
		FTM_TIMER_add(&xTimer, FTNM_TRIG_LOOP_INTERVAL);

		xRet = FTM_MSGQ_timedPop(pCTX->pMsgQ, FTNM_TRIG_LOOP_INTERVAL, (FTM_VOID_PTR _PTR_)&pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTNM_TRIG_MSG_TYPE_UPDATE_EP:
				{
					TRACE("EP[%08x] is updated.\n",	pMsg->xParams.xUpdateEP.xEPID);
					TRACE("Actor is %08x.\n",		pMsg->xActID);
				}
				break;

			default:
				{
					TRACE("Unknown message.\n");	
				}
			}
			FTM_MEM_free(pMsg);
		}
		
		if (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE)
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);
			usleep(ulRemain);
		}
	}
	return	0;
}

FTM_RET FTNM_TRIG_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(pTriggerList, pulCount);
}

FTM_RET	FTNM_TRIG_create(FTM_EVENT_PTR pEvent)
{
	ASSERT(pEvent != NULL);

	FTM_RET			xRet;
	FTNM_TRIG_PTR	pTrigger;

	pTrigger = (FTNM_TRIG_PTR)FTM_MEM_malloc(sizeof(FTNM_TRIG));
	if (pTrigger == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pTrigger->xEvent, pEvent, sizeof(FTM_EVENT));

	xRet = FTM_LIST_append(pTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTrigger);	
	}

	return	xRet;
}

FTM_RET	FTNM_TRIG_del(FTNM_TRIG_ID  xEventID)
{
	FTM_RET			xRet;
	FTNM_TRIG_PTR	pTrigger;

	xRet = FTM_LIST_get(pTriggerList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)&pTrigger);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pTriggerList, pTrigger);
		FTM_MEM_free(pTrigger);
	}

	return	xRet;	
}

FTM_RET	FTNM_TRIG_get(FTNM_TRIG_ID xEventID, FTNM_TRIG_PTR _PTR_ ppTrigger)
{
	return	FTM_LIST_get(pTriggerList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTNM_TRIG_getAt(FTM_ULONG ulIndex, FTNM_TRIG_PTR _PTR_ ppTrigger)
{
	return	FTM_LIST_getAt(pTriggerList, ulIndex, (FTM_VOID_PTR _PTR_)ppTrigger);
}

FTM_RET	FTNM_TRIG_updateEP(FTM_EPID xEPID, FTM_EP_DATA_PTR pLastData, FTM_EP_DATA_PTR pNewData)
{
	FTM_RET				xRet;
	FTNM_TRIG_MSG_PTR	pMsg;
	FTNM_TRIG_PTR		pTrigger;
	FTM_ULONG			i, ulCount;

	TRACE("EP : %08x\n", xEPID);
	FTM_LIST_count(pTriggerList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTNM_TRIG_getAt(i, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			return	FTM_RET_OK;	
		}
		
		TRACE("EP[%08x] <-> TRIG[%08x]\n", xEPID, pTrigger->xEvent.xEPID);
		if (pTrigger->xEvent.xEPID == xEPID)
		{
			FTM_BOOL	bTriggered = FTM_FALSE;

			xRet = FTM_EVENT_occurred(&pTrigger->xEvent, pLastData, pNewData, &bTriggered);
			TRACE("Trigger : %d\n", bTriggered);
			if ((xRet == FTM_RET_OK) && (bTriggered))
			{
				pMsg = (FTNM_TRIG_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_TRIG_MSG));
				if (pMsg == NULL)
				{
					ERROR("Not enough memory.\n");	
					return	FTM_RET_NOT_ENOUGH_MEMORY;
				}

				pMsg->xType = FTNM_TRIG_MSG_TYPE_UPDATE_EP;
				pMsg->xActID = pTrigger->xEvent.xActID;
				pMsg->xParams.xUpdateEP.xEPID = xEPID;
				memcpy(&pMsg->xParams.xUpdateEP.xLastData, pLastData, sizeof(FTM_EP_DATA));
				memcpy(&pMsg->xParams.xUpdateEP.xNewData, pNewData, sizeof(FTM_EP_DATA));

				xRet = FTM_MSGQ_push(pCTX->pMsgQ, (FTM_VOID_PTR)pMsg);
				if (xRet != FTM_RET_OK)
				{
					FTM_MEM_free(pMsg);
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTNM_TRIG_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTNM_TRIG_PTR)pElement)->xEvent.xID == *((FTM_EVENT_ID_PTR)pIndicator);
}

