#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftm_list.h"
#include "ftm_timer.h"
#include "ftnm_ep.h"
#include "ftnm_dmc.h"

#define	FTNM_EP_DATA_COUNT	32

typedef	enum FTNM_EP_CMD_ENUM
{
	FTNM_EP_CMD_STOP,
} FTNM_EP_CMD, _PTR_ FTNM_EP_CMD_PTR;

typedef	struct	FTNM_EP_MSG_STRUCT
{
	FTNM_EP_CMD		xCmd;	
} FTNM_EP_MSG, _PTR_ FTNM_EP_MSG_PTR;

static FTM_VOID_PTR	FTNM_EP_process(FTM_VOID_PTR pData);
static FTM_INT		FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT		FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

static FTM_LIST_PTR	pEPList = NULL;

FTM_RET	FTNM_EP_init(void)
{
	FTM_RET	xRet;

	pEPList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pEPList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_LIST_init(pEPList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(pEPList, FTNM_EP_seeker);
	FTM_LIST_setComparator(pEPList, FTNM_EP_comparator);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_final(void)
{
	ASSERT(pEPList != NULL);

	FTNM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPList);
	while(FTM_LIST_iteratorNext(pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTNM_EP_destroy(pEP);	
	}

	FTM_MEM_free(pEPList);
	pEPList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_create(FTM_EP_INFO_PTR pInfo, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTNM_EP_PTR		pEP = NULL;
	FTM_EP_DATA_PTR	pData;

	pEP = (FTNM_EP_PTR)FTM_MEM_malloc(sizeof(FTNM_EP) + sizeof(FTM_EP_DATA) * FTNM_EP_DATA_COUNT);
	if (pEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pEP, 0, sizeof(FTNM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP_INFO));
	switch(pInfo->xEPID & FTM_EP_CLASS_MASK)
	{
	case	FTM_EP_CLASS_TEMPERATURE:
	case	FTM_EP_CLASS_HUMIDITY:
	case	FTM_EP_CLASS_VOLTAGE:
	case	FTM_EP_CLASS_CURRENT:
	case	FTM_EP_CLASS_GAS:
	case	FTM_EP_CLASS_POWER:
	case	FTM_EP_CLASS_AI:
	case	FTM_EP_CLASS_COUNT:
	case	FTM_EP_CLASS_MULTI:
		{
			xRet = FTM_EP_DATA_createFloat(0, FTM_EP_DATA_STATE_INVALID, 0, &pData);
		}
		break;

	case	FTM_EP_CLASS_DI:
	case	FTM_EP_CLASS_DO:
		{	
			xRet = FTM_EP_DATA_createInt(0, FTM_EP_DATA_STATE_INVALID, 0, &pData);
		}
		break;

	default:
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		return	xRet;
	}

	sem_init(&pEP->xLock, 0, 1);
	FTM_MSGQ_init(&pEP->xMsgQ);

	FTM_LIST_init(&pEP->xDataList);
	FTM_LIST_append(&pEP->xDataList, pData);

	FTM_LIST_init(&pEP->xEventList);

	FTM_LIST_append(pEPList, pEP);

	*ppEP = pEP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_destroy(FTNM_EP_PTR	pEP)
{
	ASSERT(pEP != NULL);
	ASSERT(pEPList != NULL);

	FTM_RET	xRet;
	
	xRet = FTM_LIST_remove(pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}


	FTNM_EP_stop(pEP, FTM_TRUE);

	FTM_LIST_final(&pEP->xEventList);
	FTM_MSGQ_final(&pEP->xMsgQ);
	sem_destroy(&pEP->xLock);

	FTM_MEM_free(pEP);	

	return	xRet;
}

FTM_RET	FTNM_EP_count
(
	FTM_EP_CLASS 	xClass, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPList != NULL);
	ASSERT(pulCount != NULL);

	if ((xClass == 0XFFFFFFFF) || (xClass == 0))
{
		return	FTM_LIST_count(pEPList, pulCount);
	}
	else
	{
		FTM_ULONG	i, ulTotalCount, ulCount = 0;

		FTM_LIST_count(pEPList, &ulTotalCount);
		for(i = 0 ; i < ulTotalCount; i++)
		{
			FTNM_EP_PTR	pEP;

			FTM_LIST_getAt(pEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
			if (xClass == (pEP->xInfo.xEPID & FTM_EP_CLASS_MASK))
			{
				ulCount++;
			}
		}


		*pulCount = ulCount;
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_EP_getIDList
(
	FTM_EP_CLASS 	xClass, 
	FTM_EPID_PTR 	pEPIDList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPList != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(pEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTNM_EP_PTR	pEP;

		FTM_LIST_getAt(pEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
		if ((xClass == 0) || (xClass == (pEP->xInfo.xEPID & FTM_EP_CLASS_MASK)))
		{
			pEPIDList[ulCount++] = pEP->xInfo.xEPID;
		}
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_get(FTM_EPID xEPID, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);

	return	FTM_LIST_get(pEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTNM_EP_getAt(FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);

	return	FTM_LIST_getAt(pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTNM_EP_attach(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode)
{
	ASSERT(pEP != NULL);
	ASSERT(pNode != NULL);

	pEP->pNode = pNode;

	switch (pNode->xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			pEP->xOption.xSNMP.nOIDLen = MAX_OID_LEN;

			FTNM_NODE_SNMPC_getOID((FTNM_NODE_SNMPC_PTR)pNode, 
					(pEP->xInfo.xEPID >> 24) & 0xFF, 
					(pEP->xInfo.xEPID & 0xFF), 
					pEP->xOption.xSNMP.pOID, 
					&pEP->xOption.xSNMP.nOIDLen);
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_detach(FTNM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	pEP->pNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTNM_EP_start(FTNM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	if (pEP->pNode == NULL)
	{
		ERROR("EP[%08x] is not attached.\n", pEP->xInfo.xEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	if (pEP->xState == FTM_EP_STATE_RUN)
	{
		ERROR("EP[%08x] already started.\n", pEP->xInfo.xEPID);
		return	FTM_RET_ALREADY_STARTED;
	}

	pthread_create(&pEP->xPThread, NULL, FTNM_EP_process, (FTM_VOID_PTR)pEP);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_stop(FTNM_EP_PTR pEP, FTM_BOOL bWaitForStop)
{
	FTM_VOID_PTR	xRet;
	FTNM_EP_MSG_PTR	pMsg;
	if (pEP->xState == FTM_EP_STATE_STOP)
	{
		return	FTM_RET_EP_DID_NOT_START;
	}

	pMsg = (FTNM_EP_MSG_PTR)FTM_MEM_malloc(sizeof(FTNM_EP_MSG));
	if (pMsg == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;		
	}

	pMsg->xCmd = FTNM_EP_CMD_STOP;
	FTM_MSGQ_push(&pEP->xMsgQ, pMsg);

	if (bWaitForStop)
	{
		pthread_join(pEP->xPThread, (void **)&xRet);
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_EP_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTNM_EP_PTR		pEP = (FTNM_EP_PTR)pData;
	FTM_TIMER		xInterval;
	FTM_BOOL		bStop = FTM_FALSE;

	pEP->xState = FTM_EP_STATE_RUN;

	TRACE("EP[%08x] process start.\n", pEP->xInfo.xEPID);

	FTM_TIMER_init(&xInterval, 0);

	while(!bStop)
	{
		FTM_ULONG		ulRemainTime = 0;
		FTM_EP_DATA		xData, xReadData;
		FTNM_EP_MSG_PTR	pMsg = NULL;
		
		FTM_TIMER_add(&xInterval, pEP->xInfo.ulInterval * 1000000);
		FTM_TIMER_remain(&xInterval, &ulRemainTime);

		xData.ulTime = time(NULL);
		if (FTNM_NODE_getEPData(pEP->pNode, pEP, &xReadData) == FTM_RET_OK)
		{
			TRACE("EP(%08x:%s) - The data import was successful.\n", pEP->xInfo.xEPID, pEP->pNode->xInfo.pDID);
			xData.xState = FTM_EP_DATA_STATE_VALID;
			xData.xType = xReadData.xType;
			memcpy(&xData.xValue, &xReadData.xValue, sizeof(xData.xValue));

			FTNM_EP_setData(pEP, &xData);
		}
		else
		{
			TRACE("EP(%08x:%s) - The data import was failed.\n", pEP->xInfo.xEPID, pEP->pNode->xInfo.pDID);
		}

		
		FTM_TIMER_remain(&xInterval, &ulRemainTime);
		while (!bStop && (FTM_MSGQ_timedPop(&pEP->xMsgQ, ulRemainTime, (FTM_VOID_PTR _PTR_)&pMsg) == FTM_RET_OK))
		{
			switch(pMsg->xCmd)
			{
			case	FTNM_EP_CMD_STOP:
				{
					bStop = FTM_TRUE;
				}	
				break;
			}

			FTM_TIMER_remain(&xInterval, &ulRemainTime);

			FTM_MEM_free(pMsg);
		}
	
		if (!bStop)
		{
			FTM_TIMER_waitForExpired(&xInterval);
		}
	} 

	pEP->xState = FTM_EP_STATE_STOP;

	return	0;
}

FTM_RET	FTNM_EP_getData(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pTempData;

	xRet = FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pTempData);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pData, pTempData, sizeof(FTM_EP_DATA));
	}

	return	xRet;
}

FTM_RET	FTNM_EP_setData(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_CHAR			pTimeString[64];
	FTM_EP_DATA_PTR		pTempData;
	FTM_EP_DATA_PTR		pNewData = NULL;

	xRet = FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pTempData);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;
	}

	if (pTempData->xType != pData->xType)
	{
		ERROR("Data type missmatch[%08x:%08x]!\n", pTempData->xType, pData->xType);
		return	FTM_RET_INVALID_ARGUMENTS;
	}


	strcpy(pTimeString, ctime((time_t *)&pData->ulTime));
	pTimeString[strlen(pTimeString) - 1] = 0;

	TRACE("%6s : %s\n", "TIME", pTimeString);
	xRet = FTM_EP_DATA_createCopy(pData, &pNewData);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

#if 0
	FTM_EVENT_ID		xEventID;

	xRet = FTM_LIST_count(pEP->xEventList, &ulEventCount);
	if (xRet  == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0; i < ulEventCount ; i++)
		{
			xRet = FTM_LIST_getAt(&pEP->xEventList, i, &xEventID);
			if (xRet == FTM_RET_OK)
			{
			}
		}
	}
#endif

	xRet = FTM_LIST_count(&pEP->xDataList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		FTM_EP_DATA_destroy(pNewData);
		return	xRet;	
	}
	
	if (ulCount >= FTNM_EP_DATA_COUNT)
	{
		FTM_EP_DATA_PTR	pTempData;

		xRet = FTM_LIST_getFirst(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pTempData);	
		if (xRet != FTM_RET_OK)
		{
			FTM_EP_DATA_destroy(pNewData);
			return	xRet;	
		}

		FTM_LIST_remove(&pEP->xDataList, pTempData);	
	}
	
	xRet = FTM_LIST_append(&pEP->xDataList, pNewData);
	if (xRet != FTM_RET_OK)
	{
		FTM_EP_DATA_destroy(pNewData);	
	}

	FTNM_setEPData(pEP->xInfo.xEPID, pNewData);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_getEventCount(FTNM_EP_PTR pEP, FTM_ULONG_PTR pulCount)
{
	ASSERT(pEP != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pEP->xEventList, pulCount);
}

FTM_RET	FTNM_EP_getEventAt(FTNM_EP_PTR pEP, FTM_ULONG ulIndex, FTM_EVENT_ID_PTR pEventID)
{
	ASSERT(pEP != NULL);
	ASSERT(pEventID != NULL);
	
	return	FTM_LIST_getAt(&pEP->xEventList, ulIndex, (FTM_VOID_PTR _PTR_)&pEventID);
}

FTM_RET FTNM_EP_trap(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;

	TRACE("The event occurred in EP[%08x].\n", pEP->xInfo.xEPID);
	TRACE("%6s : %d\n", "STATE", pData->xState);

	xRet = FTNM_EP_setData(pEP, pData);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTNM_MSG_sendEPChanged(pEP->xInfo.xEPID, pData);

	return	FTM_RET_OK;
}

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTNM_EP_PTR		pEP = (FTNM_EP_PTR)pElement;
	FTM_EPID_PTR	pEPID=(FTM_EPID_PTR)pIndicator;

	return	(pEP->xInfo.xEPID == *pEPID);
}

FTM_INT	FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTNM_EP_PTR		pEP1 = (FTNM_EP_PTR)pElement1;
	FTNM_EP_PTR		pEP2 = (FTNM_EP_PTR)pElement2;
	
	return	(pEP1->xInfo.xEPID - pEP2->xInfo.xEPID);
}

