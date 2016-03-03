#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftm_timer.h"
#include "ftnm_ep.h"
#include "ftnm_actor.h"
#include "ftnm_msg.h"
#include "libconfig.h"

#define	FTNM_ACTOR_LOOP_INTERVAL	1000	// 1000 us

typedef enum
{
	FTNM_ACTOR_MSG_TYPE_RUN = 0
}	FTNM_ACTOR_MSG_TYPE, _PTR_ FTNM_ACTOR_MSG_TYPE_PTR;

typedef	struct
{
	FTNM_ACTOR_MSG_TYPE	xType;
	FTM_ACT_ID			xActID;
}	FTNM_ACTOR_MSG, _PTR_ FTNM_ACTOR_MSG_PTR;

static FTM_VOID_PTR FTNM_ACTOR_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTNM_ACTOR_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTNM_ACTOR_MANAGER_PTR	pCTX = NULL;
static FTM_LIST_PTR				pActorList = NULL;

FTM_RET	FTNM_ACTOR_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pCTX != NULL)
	{
		ERROR("The event manager is  not initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pCTX = (FTNM_ACTOR_MANAGER_PTR)FTM_MEM_malloc(sizeof(FTNM_ACTOR_MANAGER));
	if (pCTX == NULL)
	{
		ERROR("Can't allocated memory[%d].\n", sizeof(FTNM_ACTOR_MANAGER));
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pCTX, 0, sizeof(FTNM_ACTOR_MANAGER));

	xRet = FTM_MSGQ_create(&pCTX->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pActorList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pCTX->pMsgQ);
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pActorList, FTNM_ACTOR_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_ACTOR_final(FTM_VOID)
{
	FTM_RET			xRet;
	FTNM_ACTOR_PTR	pActor;

	if (pCTX == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_MSGQ_destroy(pCTX->pMsgQ);
	pCTX->pMsgQ = NULL;


	FTM_LIST_iteratorStart(pActorList);
	while(FTM_LIST_iteratorNext(pActorList, (FTM_VOID_PTR _PTR_)&pActor) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pActorList, pActor);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pActor);	
		}
	}

	FTM_MEM_free(pCTX);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_ACTOR_loadConfig(FTM_CHAR_PTR pFileName)
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

FTM_RET	FTNM_ACTOR_start(FTM_VOID)
{
	ASSERT(pCTX != NULL);
	
	FTM_INT	nRet;

	nRet = pthread_create(&pCTX->xPThread, NULL, FTNM_ACTOR_process, pCTX);
	if (nRet < 0)
	{
		ERROR("Can't start Act Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_ACTOR_stop(FTM_VOID)
{
	ASSERT(pCTX != NULL);

	pCTX->bStop = FTM_TRUE;
	pthread_join(pCTX->xPThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_ACTOR_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_ACTOR_MANAGER_PTR	pCTX = (FTNM_ACTOR_MANAGER_PTR)pData;
	FTM_RET					xRet;
	FTNM_ACTOR_MSG_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	while(!pCTX->bStop)
	{
		FTM_TIMER_add(&xTimer, FTNM_ACTOR_LOOP_INTERVAL);

		xRet = FTM_MSGQ_timedPop(pCTX->pMsgQ, FTNM_ACTOR_LOOP_INTERVAL, (FTM_VOID_PTR _PTR_)&pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTNM_ACTOR_MSG_TYPE_RUN:
				{
					TRACE("Actor[%08x] is updated.\n",	pMsg->xActID);
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

FTM_RET FTNM_ACTOR_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(pActorList, pulCount);
}

FTM_RET	FTNM_ACTOR_create(FTM_EVENT_PTR pAct)
{
	ASSERT(pAct != NULL);

	FTM_RET			xRet;
	FTNM_ACTOR_PTR	pActor;

	pActor = (FTNM_ACTOR_PTR)FTM_MEM_malloc(sizeof(FTNM_ACTOR));
	if (pActor == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pActor->xAct, pAct, sizeof(FTM_EVENT));

	xRet = FTM_LIST_append(pActorList, pActor);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pActor);	
	}

	return	xRet;
}

FTM_RET	FTNM_ACTOR_del(FTNM_ACTOR_ID  xActID)
{
	FTM_RET			xRet;
	FTNM_ACTOR_PTR	pActor;

	xRet = FTM_LIST_get(pActorList, (FTM_VOID_PTR)&xActID, (FTM_VOID_PTR _PTR_)&pActor);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pActorList, pActor);
		FTM_MEM_free(pActor);
	}

	return	xRet;	
}

FTM_RET	FTNM_ACTOR_get(FTNM_ACTOR_ID xActID, FTNM_ACTOR_PTR _PTR_ ppActor)
{
	return	FTM_LIST_get(pActorList, (FTM_VOID_PTR)&xActID, (FTM_VOID_PTR _PTR_)ppActor);
}

FTM_RET	FTNM_ACTOR_getAt(FTM_ULONG ulIndex, FTNM_ACTOR_PTR _PTR_ ppActor)
{
	return	FTM_LIST_getAt(pActorList, ulIndex, (FTM_VOID_PTR _PTR_)ppActor);
}

FTM_RET	FTNM_ACTOR_run(FTM_ACT_ID xActID)
{
	FTM_RET				xRet;
	FTNM_ACTOR_MSG_PTR	pMsg;

	pMsg = (FTNM_ACTOR_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_ACTOR_MSG));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory.\n");	
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType = FTNM_ACTOR_MSG_TYPE_RUN;
	pMsg->xActID = xActID;

	xRet = FTM_MSGQ_push(pCTX->pMsgQ, (FTM_VOID_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);
	}

	return	xRet;
}

FTM_BOOL	FTNM_ACTOR_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTNM_ACTOR_PTR)pElement)->xAct.xID == *((FTM_EVENT_ID_PTR)pIndicator);
}

