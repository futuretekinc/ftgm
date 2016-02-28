#include <string.h>
#include "ftm_types.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_msg_queue.h"
#include "ftm_mem.h"
#include "ftnm_ep.h"
#include "ftnm_msg.h"

static FTM_MSG_QUEUE_PTR	pMsgQ = NULL;

FTM_RET FTNM_MSG_init(FTM_VOID)
{
	if (pMsgQ != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	return	FTM_MSGQ_create(&pMsgQ);	
}

FTM_RET	FTNM_MSG_final(FTM_VOID)
{
	FTM_MSGQ_destroy(pMsgQ);
	pMsgQ = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_MSG_sendSNMPTrap(FTM_CHAR_PTR pTrapMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pTrapMsg != NULL);
	FTM_RET			xRet;
	FTNM_MSG_PTR 	pMsg;

	pMsg = (FTNM_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_MSG));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	memset(pMsg, 0, sizeof(FTNM_MSG));

	pMsg->xType = FTNM_MSG_TYPE_SNMPTRAP;
	strncpy(pMsg->xParams.xSNMPTrap.pString, pTrapMsg, sizeof(pMsg->xParams.xSNMPTrap.pString) - 1);

	xRet = FTM_MSGQ_push(pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET FTNM_MSG_sendEPChanged(FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pData != NULL);
	FTM_RET			xRet;
	FTNM_MSG_PTR pMsg;

	pMsg = (FTNM_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_MSG));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	memset(pMsg, 0, sizeof(FTNM_MSG));

	pMsg->xType = FTNM_MSG_TYPE_EP_CHANGED;
	pMsg->xParams.xEPChanged.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPChanged.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTM_MSGQ_push(pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET	FTNM_MSG_pop(FTNM_MSG_PTR _PTR_ ppMsg)
{
	return	FTM_MSGQ_pop(pMsgQ, (FTM_VOID_PTR _PTR_)ppMsg);	
}
