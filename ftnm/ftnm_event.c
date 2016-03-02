#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "ftm_timer.h"
#include "ftnm_ep.h"
#include "ftnm_event.h"
#include "ftnm_msg.h"
#include "libconfig.h"

#define	FTNM_EVENTM_LOOP_INTERVAL	1000	// 1000 us

static FTM_VOID_PTR FTNM_EVENTM_process(FTM_VOID_PTR pData);

FTM_RET	FTNM_EVENTM_init(FTM_VOID)
{
	FTM_EVENT_init();
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_final(FTM_VOID)
{
	FTM_EVENT_final();
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_create(FTNM_EVENT_MANAGER_PTR _PTR_ ppCTX)
{
	ASSERT(ppCTX != NULL);
	FTNM_EVENT_MANAGER_PTR	pCTX;
	FTM_RET	xRet;

	pCTX = (FTNM_EVENT_MANAGER_PTR)FTM_MEM_malloc(sizeof(FTNM_EVENT_MANAGER));
	if (pCTX == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pCTX, 0, sizeof(FTNM_EVENT_MANAGER));

	xRet = FTM_MSGQ_create(&pCTX->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	*ppCTX = pCTX;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_destroy(FTNM_EVENT_MANAGER_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	
	FTM_MSGQ_destroy(pCTX->pMsgQ);
	pCTX->pMsgQ = NULL;

	FTM_MEM_free(pCTX);

	return	FTM_RET_OK;
}


FTM_RET	FTNM_EVENTM_loadConfig(FTNM_EVENT_MANAGER_PTR pCTX, FTM_CHAR_PTR pFileName)
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

FTM_RET	FTNM_EVENTM_start(FTNM_EVENT_MANAGER_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	pthread_create(&pCTX->xPThread, NULL, FTNM_EVENTM_process, pCTX);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_stop(FTNM_EVENT_MANAGER_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	pthread_cancel(pCTX->xPThread);
	pthread_join(pCTX->xPThread, NULL);

	return	FTM_RET_OK;
}

#if 0
FTM_RET	FTNM_EVENTM_notify(FTNM_EVENT_MANAGER_PTR pCTX, FTNM_MSG_PTR pMsg)
{
	ASSERT(pCTX != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET		xRet;
	FTNM_EP_PTR	pEP;

	xRet = FTNM_EP_get(pMsg->xEPID, &pEP);
	if (xRet != RTM_RET_OK)
	{
		ERROR("EP[%08x] not found\n", pMsg->xEPID);
		return	xRet;
	}


	return	FTM_RET_OK;
}
#endif

FTM_VOID_PTR FTNM_EVENTM_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_EVENT_MANAGER_PTR	pCTX = (FTNM_EVENT_MANAGER_PTR)pData;
	FTM_RET			xRet;
	FTM_VOID_PTR	pMsg;
	FTM_TIMER		xTimer;
	
	pCTX->bRun = FTM_TRUE;

	FTM_TIMER_init(&xTimer, 0);

	while(pCTX->bRun)
	{
		FTM_TIMER_add(&xTimer, FTNM_EVENTM_LOOP_INTERVAL);

		xRet = FTM_MSGQ_timedPop(pCTX->pMsgQ, FTNM_EVENTM_LOOP_INTERVAL, &pMsg);
		if (xRet == FTM_RET_OK)
		{
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

