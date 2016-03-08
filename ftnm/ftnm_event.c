#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftnm_ep.h"
#include "ftnm_event.h"
#include "ftnm_actor.h"
#include "ftnm_msg.h"
#include "libconfig.h"

#define	FTNM_EVENT_LOOP_INTERVAL	1000	// 1000 us

typedef enum
{
	FTNM_EVENT_MSG_TYPE_OCCURRENCE = 0,
	FTNM_EVENT_MSG_TYPE_RELEASE
}	FTNM_EVENT_MSG_TYPE, _PTR_ FTNM_EVENT_MSG_TYPE_PTR;

typedef	struct
{
	FTNM_EVENT_MSG_TYPE	xType;
	FTNM_EVENT_ID		xEventID;
	FTNM_ACTOR_ID		xActID;
	union
	{
		struct
		{
			FTM_EP_ID	xEPID;
			FTM_EP_DATA	xData;
		}	xEP;
	}	xParams;
}	FTNM_EVENT_MSG, _PTR_ FTNM_EVENT_MSG_PTR;

static FTM_VOID_PTR FTNM_EVENTM_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTNM_EVENTM_delayedEventProcess(FTM_VOID_PTR pData);
static FTM_BOOL		FTNM_EVENTM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTNM_EVENTM_init(FTNM_EVENTM_PTR pCTX)
{
	ASSERT(pCTX);

	FTM_RET	xRet;

	memset(pCTX, 0, sizeof(FTNM_EVENTM));

	xRet = FTM_MSGQ_create(&pCTX->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_init(&pCTX->xEventList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Event list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pCTX->pMsgQ);
		return	xRet;	
	}

	FTM_LIST_setSeeker(&pCTX->xEventList, FTNM_EVENTM_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_final(FTNM_EVENTM_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_EVENT_PTR	pEvent;

	FTM_MSGQ_destroy(pCTX->pMsgQ);
	pCTX->pMsgQ = NULL;


	FTM_LIST_iteratorStart(&pCTX->xEventList);
	while(FTM_LIST_iteratorNext(&pCTX->xEventList, (FTM_VOID_PTR _PTR_)&pEvent) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(&pCTX->xEventList, pEvent);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pEvent);	
		}
	}

	FTM_LIST_final(&pCTX->xEventList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENT_loadConfig(FTNM_EVENTM_PTR pCTX, FTM_CHAR_PTR pFileName)
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

FTM_RET	FTNM_EVENTM_start(FTNM_EVENTM_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	
	FTM_INT	nRet;

	nRet = pthread_create(&pCTX->xEventThread, NULL, FTNM_EVENTM_process, pCTX);
	if (nRet < 0)
	{
		ERROR("Can't start Event Manager!\n");
		return	FTM_RET_ERROR;
	}

	nRet = pthread_create(&pCTX->xDelayedEventThread, NULL, FTNM_EVENTM_delayedEventProcess, pCTX);
	if (nRet < 0)
	{
		ERROR("Can't start Event Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_stop(FTNM_EVENTM_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	pCTX->bStop = FTM_TRUE;
	pthread_join(pCTX->xDelayedEventThread, NULL);
	pthread_join(pCTX->xEventThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_EVENTM_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_EVENTM_PTR	pCTX = (FTNM_EVENTM_PTR)pData;
	FTM_RET					xRet;
	FTNM_EVENT_MSG_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	while(!pCTX->bStop)
	{
		FTM_TIMER_add(&xTimer, FTNM_EVENT_LOOP_INTERVAL);

		xRet = FTM_MSGQ_timedPop(pCTX->pMsgQ, FTNM_EVENT_LOOP_INTERVAL, (FTM_VOID_PTR _PTR_)&pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTNM_EVENT_MSG_TYPE_OCCURRENCE:
				{
					TRACE("EP[%08x] event occurrence.\n",	pMsg->xParams.xEP.xEPID);

					TRACE("Actor is %08x.\n",		pMsg->xActID);
				}
				break;

			case	FTNM_EVENT_MSG_TYPE_RELEASE:
				{
					TRACE("EP[%08x] event release.\n",	pMsg->xParams.xEP.xEPID);
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

FTM_VOID_PTR FTNM_EVENTM_delayedEventProcess(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_EVENTM_PTR	pCTX = (FTNM_EVENTM_PTR)pData;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	while(!pCTX->bStop)
	{
		FTM_TIMER_add(&xTimer, FTNM_EVENT_LOOP_INTERVAL);

		if (FTM_TIMER_isExpired(&xTimer) != FTM_TRUE)
		{
			FTM_ULONG	ulRemain = 0;	

			FTM_TIMER_remain(&xTimer, &ulRemain);
			usleep(ulRemain);
		}
	}

	return	0;
}

FTM_RET FTNM_EVENTM_count(FTNM_EVENTM_PTR pCTX, FTM_ULONG_PTR pulCount)
{
	ASSERT(pCTX != NULL);

	return	FTM_LIST_count(&pCTX->xEventList, pulCount);
}

FTM_RET	FTNM_EVENTM_create(FTNM_EVENTM_PTR pCTX, FTM_EVENT_PTR pInfo)
{
	ASSERT(pCTX != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTNM_EVENT_PTR	pEvent;

	pEvent = (FTNM_EVENT_PTR)FTM_MEM_malloc(sizeof(FTNM_EVENT));
	if (pEvent == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pEvent, 0, sizeof(FTNM_EVENT));
	memcpy(&pEvent->xInfo, pInfo, sizeof(FTM_EVENT));

	xRet = FTM_LIST_append(&pCTX->xEventList, pEvent);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEvent);	
	}

	return	xRet;
}

FTM_RET	FTNM_EVENTM_del(FTNM_EVENTM_PTR pCTX, FTNM_EVENT_ID  xEventID)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_EVENT_PTR	pEvent;

	xRet = FTM_LIST_get(&pCTX->xEventList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)&pEvent);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(&pCTX->xEventList, pEvent);
		FTM_MEM_free(pEvent);
	}

	return	xRet;	
}

FTM_RET	FTNM_EVENTM_get(FTNM_EVENTM_PTR pCTX, FTNM_EVENT_ID xEventID, FTNM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pCTX != NULL);

	return	FTM_LIST_get(&pCTX->xEventList, (FTM_VOID_PTR)&xEventID, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTNM_EVENTM_getAt(FTNM_EVENTM_PTR pCTX, FTM_ULONG ulIndex, FTNM_EVENT_PTR _PTR_ ppEvent)
{
	ASSERT(pCTX != NULL);

	return	FTM_LIST_getAt(&pCTX->xEventList, ulIndex, (FTM_VOID_PTR _PTR_)ppEvent);
}

FTM_RET	FTNM_EVENTM_updateEP(FTNM_EVENTM_PTR pCTX, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pCTX != NULL);

	FTM_RET				xRet;
	FTNM_EVENT_MSG_PTR	pMsg;
	FTNM_EVENT_PTR		pEvent;
	FTM_ULONG			i, ulCount;

	TRACE("EP : %08x\n", xEPID);
	FTM_LIST_count(&pCTX->xEventList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTNM_EVENTM_getAt(pCTX, i, &pEvent);
		if (xRet != FTM_RET_OK)
		{
			return	FTM_RET_OK;	
		}
		
		if (pEvent->xInfo.xEPID == xEPID)
		{
			FTM_BOOL	bOccurrence = FTM_FALSE;

			xRet = FTM_EVENT_occurred(&pEvent->xInfo, pData, &bOccurrence);
			if (xRet == FTM_RET_OK)
			{
				if(bOccurrence && !pEvent->bOccurrence)
				{
					pMsg = (FTNM_EVENT_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_EVENT_MSG));
					if (pMsg == NULL)
					{
						ERROR("Not enough memory.\n");	
					}
					else
					{
						pMsg->xType = FTNM_EVENT_MSG_TYPE_OCCURRENCE;
						pMsg->xEventID = pEvent->xInfo.xID;
						pMsg->xActID = pEvent->xInfo.xActID;
						pMsg->xParams.xEP.xEPID = xEPID;
						memcpy(&pMsg->xParams.xEP.xData, pData, sizeof(FTM_EP_DATA));

						if (FTM_TIME_isZero(&pEvent->xInfo.xDetectionTime))
						{
							xRet = FTM_MSGQ_push(pCTX->pMsgQ, (FTM_VOID_PTR)pMsg);
							if (xRet != FTM_RET_OK)
							{
									FTM_MEM_free(pMsg);
							}
						}
						else
						{
							TRACE("EVENT delayed\n");	
						}
					}
				}
				if(!bOccurrence && pEvent->bOccurrence)
				{
					pMsg = (FTNM_EVENT_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_EVENT_MSG));
					if (pMsg == NULL)
					{
						ERROR("Not enough memory.\n");	
					}
					else
					{
						pMsg->xType = FTNM_EVENT_MSG_TYPE_RELEASE;
						pMsg->xEventID = pEvent->xInfo.xID;
						pMsg->xParams.xEP.xEPID = xEPID;
						memcpy(&pMsg->xParams.xEP.xData, pData, sizeof(FTM_EP_DATA));

						if (FTM_TIME_isZero(&pEvent->xInfo.xHoldingTime))
						{
							xRet = FTM_MSGQ_push(pCTX->pMsgQ, (FTM_VOID_PTR)pMsg);
							if (xRet != FTM_RET_OK)
							{
								FTM_MEM_free(pMsg);
							}
						}
						else
						{
							TRACE("EVENT hold.\n");	
						}
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTNM_EVENTM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTNM_EVENT_PTR)pElement)->xInfo.xID == *((FTM_EVENT_ID_PTR)pIndicator);
}

