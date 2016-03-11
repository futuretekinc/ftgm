#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftnm_action.h"
#include "ftnm_msg.h"
#include "libconfig.h"

#define	FTNM_ACTION_LOOP_INTERVAL	1000	// 1000 us

typedef enum
{
	FTNM_ACTION_MSG_TYPE_RUN = 0
}	FTNM_ACTION_MSG_TYPE, _PTR_ FTNM_ACTION_MSG_TYPE_PTR;

typedef	struct
{
	FTNM_ACTION_MSG_TYPE	xType;
	FTM_ACTION_ID			xActID;
}	FTNM_ACTION_MSG, _PTR_ FTNM_ACTION_MSG_PTR;

static FTM_VOID_PTR FTNM_ACTION_process(FTM_VOID_PTR pData);
static FTM_BOOL		FTNM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTNM_ACTION_MANAGER_PTR	pCTX = NULL;
static FTM_LIST_PTR				pActionList = NULL;

FTM_RET	FTNM_ACTION_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pCTX != NULL)
	{
		ERROR("The event manager is  not initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pCTX = (FTNM_ACTION_MANAGER_PTR)FTM_MEM_malloc(sizeof(FTNM_ACTION_MANAGER));
	if (pCTX == NULL)
	{
		ERROR("Can't allocated memory[%d].\n", sizeof(FTNM_ACTION_MANAGER));
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pCTX, 0, sizeof(FTNM_ACTION_MANAGER));

	xRet = FTM_MSGQ_create(&pCTX->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message Queue creation failed[%08x].\n", xRet);
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pActionList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor list creation failed[%08x].\n", xRet);
		FTM_MSGQ_destroy(pCTX->pMsgQ);
		FTM_MEM_free(pCTX);
		return	xRet;	
	}

	FTM_LIST_setSeeker(pActionList, FTNM_ACTION_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_ACTION_final(FTM_VOID)
{
	FTM_RET			xRet;
	FTNM_ACTION_PTR	pAction;

	if (pCTX == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_MSGQ_destroy(pCTX->pMsgQ);
	pCTX->pMsgQ = NULL;


	FTM_LIST_iteratorStart(pActionList);
	while(FTM_LIST_iteratorNext(pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pActionList, pAction);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pAction);	
		}
	}

	FTM_MEM_free(pCTX);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_ACTION_loadConfig(FTM_CHAR_PTR pFileName)
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

FTM_RET	FTNM_ACTION_start(FTM_VOID)
{
	ASSERT(pCTX != NULL);
	
	FTM_INT	nRet;

	nRet = pthread_create(&pCTX->xPThread, NULL, FTNM_ACTION_process, pCTX);
	if (nRet < 0)
	{
		ERROR("Can't start Act Manager!\n");
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_ACTION_stop(FTM_VOID)
{
	ASSERT(pCTX != NULL);

	pCTX->bStop = FTM_TRUE;
	pthread_join(pCTX->xPThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_ACTION_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);
	FTNM_ACTION_MANAGER_PTR	pCTX = (FTNM_ACTION_MANAGER_PTR)pData;
	FTM_RET					xRet;
	FTNM_ACTION_MSG_PTR		pMsg;
	FTM_TIMER				xTimer;
	
	FTM_TIMER_init(&xTimer, 0);

	while(!pCTX->bStop)
	{
		FTM_TIMER_add(&xTimer, FTNM_ACTION_LOOP_INTERVAL);

		xRet = FTM_MSGQ_timedPop(pCTX->pMsgQ, FTNM_ACTION_LOOP_INTERVAL, (FTM_VOID_PTR _PTR_)&pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTNM_ACTION_MSG_TYPE_RUN:
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

FTM_RET FTNM_ACTION_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(pActionList, pulCount);
}

FTM_RET	FTNM_ACTION_create(FTM_ACTION_PTR pInfo)
{
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTNM_ACTION_PTR	pAction;

	pAction = (FTNM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTNM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));

	xRet = FTM_LIST_append(pActionList, pAction);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pAction);	
	}

	return	xRet;
}

FTM_RET	FTNM_ACTION_del(FTM_ACTION_ID  xActID)
{
	FTM_RET			xRet;
	FTNM_ACTION_PTR	pAction;

	xRet = FTM_LIST_get(pActionList, (FTM_VOID_PTR)&xActID, (FTM_VOID_PTR _PTR_)&pAction);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_remove(pActionList, pAction);
		FTM_MEM_free(pAction);
	}

	return	xRet;	
}

FTM_RET	FTNM_ACTION_get(FTM_ACTION_ID xActID, FTNM_ACTION_PTR _PTR_ ppAction)
{
	return	FTM_LIST_get(pActionList, (FTM_VOID_PTR)&xActID, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTNM_ACTION_getAt(FTM_ULONG ulIndex, FTNM_ACTION_PTR _PTR_ ppAction)
{
	return	FTM_LIST_getAt(pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);
}

FTM_RET	FTNM_ACTION_run(FTM_ACTION_ID xActID)
{
	FTM_RET				xRet;
	FTNM_ACTION_MSG_PTR	pMsg;

	pMsg = (FTNM_ACTION_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_ACTION_MSG));
	if (pMsg == NULL)
	{
		ERROR("Not enough memory.\n");	
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pMsg->xType = FTNM_ACTION_MSG_TYPE_RUN;
	pMsg->xActID = xActID;

	xRet = FTM_MSGQ_push(pCTX->pMsgQ, (FTM_VOID_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);
	}

	return	xRet;
}

FTM_BOOL	FTNM_ACTION_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	((FTNM_ACTION_PTR)pElement)->xInfo.xID == *((FTM_ACTION_ID_PTR)pIndicator);
}

