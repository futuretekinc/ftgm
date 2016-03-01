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

FTM_RET	FTNM_EVENTM_create(FTNM_EVENT_MANAGER_PTR _PTR_ ppManager)
{
	ASSERT(ppManager != NULL);
	FTNM_EVENT_MANAGER_PTR	pManager;
	FTM_RET	xRet;

	pManager = (FTNM_EVENT_MANAGER_PTR)FTM_MEM_malloc(sizeof(FTNM_EVENT_MANAGER));
	if (pManager == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(pManager, 0, sizeof(FTNM_EVENT_MANAGER));

	xRet = FTM_MSGQ_create(&pManager->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pManager);
		return	xRet;	
	}

	*ppManager = pManager;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_destroy(FTNM_EVENT_MANAGER_PTR pManager)
{
	ASSERT(pManager != NULL);
	
	FTM_MSGQ_destroy(pManager->pMsgQ);
	pManager->pMsgQ = NULL;

	FTM_MEM_free(pManager);

	return	FTM_RET_OK;
}


FTM_RET	FTNM_EVENTM_start(FTNM_EVENT_MANAGER_PTR pManager)
{
	ASSERT(pManager != NULL);

	pthread_create(&pManager->xPThread, NULL, FTNM_EVENTM_process, pManager);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EVENTM_stop(FTNM_EVENT_MANAGER_PTR pManager)
{
	ASSERT(pManager != NULL);

	pthread_cancel(pManager->xPThread);
	pthread_join(pManager->xPThread, NULL);

	return	FTM_RET_OK;
}

#if 0
FTM_RET	FTNM_EVENTM_notify(FTNM_EVENT_MANAGER_PTR pManager, FTNM_MSG_PTR pMsg)
{
	ASSERT(pManager != NULL);
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
	FTNM_EVENT_MANAGER_PTR	pManager = (FTNM_EVENT_MANAGER_PTR)pData;
	FTM_RET			xRet;
	FTM_VOID_PTR	pMsg;
	FTM_TIMER		xTimer;
	
	pManager->bRun = FTM_TRUE;

	FTM_TIMER_init(&xTimer, 0);

	while(pManager->bRun)
	{
		FTM_TIMER_add(&xTimer, FTNM_EVENTM_LOOP_INTERVAL);

		xRet = FTM_MSGQ_timedPop(pManager->pMsgQ, FTNM_EVENTM_LOOP_INTERVAL, &pMsg);
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

