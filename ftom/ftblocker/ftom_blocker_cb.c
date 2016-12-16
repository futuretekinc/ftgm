#include "ftom_blocker.h"
#include "ftom_blocker_cb.h"

FTM_RET	FTOM_BLOCKER_EVENT_publishStatusCB
(
	FTM_TIMER_PTR 	pTimer, 
	FTM_VOID_PTR 	pData
)
{
	FTM_RET	xRet;
	FTOM_BLOCKER_PTR	pBlocker = (FTOM_BLOCKER_PTR)pData;
	FTOM_MSG_PTR		pMsg;

    xRet = FTOM_MSG_createReport(pBlocker, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create message!\n");    
		return	xRet;
	}

	xRet = FTOM_BLOCKER_MESSAGE_send(pBlocker, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message!\n");	
		FTOM_MSG_destroy(&pMsg);
	}

	return	xRet;
}

