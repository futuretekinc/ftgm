#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftnm.h"
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
static FTM_RET		FTNM_EPM_notifyEPDataSaveToDB(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData);
static FTM_RET		FTNM_EP_transmissionData
(
	FTNM_EP_PTR 	pEP,
	FTM_ULONG		ulStartTime,
	FTM_ULONG		ulEndTime
);
static FTM_RET		FTNM_EPM_notifyEPDataTransmissionINT(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_INT nValue, FTM_INT nAverage, FTM_INT nCount, FTM_INT nMax, FTM_INT nMin);
static FTM_RET		FTNM_EPM_notifyEPDataTransmissionULONG(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_ULONG ulValue, FTM_ULONG ulAverage, FTM_INT nCount, FTM_ULONG ulMax, FTM_ULONG ulMin);
static FTM_RET		FTNM_EPM_notifyEPDataTransmissionFLOAT(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_FLOAT fValue, FTM_FLOAT fAverage, FTM_INT nCount, FTM_FLOAT fMax, FTM_FLOAT fMin);
static FTM_RET		FTNM_EPM_notifyEPDataTransmissionBOOL(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_BOOL bValue);

FTM_RET	FTNM_EPM_create(FTNM_CONTEXT_PTR pCTX, FTNM_EPM_PTR _PTR_ ppEPM)
{
	ASSERT(pCTX != NULL);
	ASSERT(ppEPM != NULL);

	FTM_RET		xRet;
	FTNM_EPM_PTR	pEPM;

	pEPM = (FTNM_EPM_PTR)FTM_MEM_malloc(sizeof(FTNM_EPM));
	if (pEPM == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTNM_EPM_init(pCTX, pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EPM init failed[%08x].\n", xRet);
		FTM_MEM_free(pEPM);
		return	xRet;
	}

	*ppEPM = pEPM;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_destroy(FTNM_EPM_PTR _PTR_ ppEPM)
{
	ASSERT(ppEPM != NULL);

	if (*ppEPM == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	FTNM_EPM_final(*ppEPM);

	FTM_MEM_free(*ppEPM);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_init(FTNM_CONTEXT_PTR pCTX, FTNM_EPM_PTR pEPM)
{
	ASSERT(pCTX != NULL);
	ASSERT(pEPM != NULL);

	FTM_RET	xRet;

	if (pEPM->pEPList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pEPM->pEPList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	pEPM->pCTX = pCTX;
	FTM_LIST_setSeeker(pEPM->pEPList, FTNM_EP_seeker);
	FTM_LIST_setComparator(pEPM->pEPList, FTNM_EP_comparator);

	TRACE("EP management initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_final(FTNM_EPM_PTR pEPM)
{
	ASSERT(pEPM != NULL);

	if (pEPM->pEPList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTNM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPM->pEPList);
	while(FTM_LIST_iteratorNext(pEPM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTNM_EPM_destroyEP(pEPM, pEP);	
	}

	FTM_LIST_destroy(pEPM->pEPList);
	pEPM->pEPList = NULL;

	TRACE("EP management finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_createEP(FTNM_EPM_PTR pEPM, FTM_EP_PTR pInfo, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTNM_EP_PTR		pEP = NULL;
	FTM_EP_DATA_PTR	pData;

	TRACE("TYPE = %08x\n", pInfo->xType);
	pEP = (FTNM_EP_PTR)FTM_MEM_malloc(sizeof(FTNM_EP) + sizeof(FTM_EP_DATA) * FTNM_EP_DATA_COUNT);
	if (pEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pEP, 0, sizeof(FTNM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));
	switch(pInfo->xEPID & FTM_EP_TYPE_MASK)
	{
	case	FTM_EP_TYPE_TEMPERATURE:
	case	FTM_EP_TYPE_HUMIDITY:
	case	FTM_EP_TYPE_VOLTAGE:
	case	FTM_EP_TYPE_CURRENT:
	case	FTM_EP_TYPE_GAS:
	case	FTM_EP_TYPE_POWER:
	case	FTM_EP_TYPE_AI:
	case	FTM_EP_TYPE_COUNT:
	case	FTM_EP_TYPE_MULTI:
		{
			xRet = FTM_EP_DATA_createFloat(0, FTM_EP_DATA_STATE_INVALID, 0, &pData);
		}
		break;

	case	FTM_EP_TYPE_DI:
	case	FTM_EP_TYPE_DO:
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

	FTM_LIST_init(&pEP->xTriggerList);
	pEP->pEPM = pEPM;
	
	FTM_LIST_append(pEPM->pEPList, pEP);

	*ppEP = pEP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_destroyEP(FTNM_EPM_PTR pEPM, FTNM_EP_PTR	pEP)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;
	FTM_EP_DATA_PTR	pData;

	xRet = FTM_LIST_remove(pEPM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_iteratorStart(&pEP->xDataList);
	while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		FTM_EP_DATA_destroy(pData);	
	}

	FTM_LIST_final(&pEP->xDataList);

	FTNM_EP_stop(pEP, FTM_TRUE);

	FTM_LIST_final(&pEP->xTriggerList);
	FTM_MSGQ_final(&pEP->xMsgQ);
	sem_destroy(&pEP->xLock);

	FTM_MEM_free(pEP);	

	return	xRet;
}

FTM_RET	FTNM_EPM_count
(
	FTNM_EPM_PTR	pEPM,
	FTM_EP_TYPE 	xType, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pulCount != NULL);

	if ((xType == 0XFFFFFFFF) || (xType == 0))
	{
		return	FTM_LIST_count(pEPM->pEPList, pulCount);
	}
	else
	{
		FTM_ULONG	i, ulTotalCount, ulCount = 0;

		FTM_LIST_count(pEPM->pEPList, &ulTotalCount);
		for(i = 0 ; i < ulTotalCount; i++)
		{
			FTNM_EP_PTR	pEP;

			FTM_LIST_getAt(pEPM->pEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
			if (xType == (pEP->xInfo.xEPID & FTM_EP_TYPE_MASK))
			{
				ulCount++;
			}
		}


		*pulCount = ulCount;
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_EPM_getIDList
(
	FTNM_EPM_PTR		pEPM,
	FTM_EP_TYPE 	xType, 
	FTM_EP_ID_PTR 	pEPIDList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(pEPM->pEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTNM_EP_PTR	pEP;

		FTM_LIST_getAt(pEPM->pEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
		if ((xType == 0) || (xType == (pEP->xInfo.xEPID & FTM_EP_TYPE_MASK)))
		{
			pEPIDList[ulCount++] = pEP->xInfo.xEPID;
		}
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_get
(
	FTNM_EPM_PTR		pEPM,
	FTM_EP_ID 			xEPID, 
	FTNM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);

	return	FTM_LIST_get(pEPM->pEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTNM_EPM_getAt
(
	FTNM_EPM_PTR		pEPM,
	FTM_ULONG 			ulIndex, 
	FTNM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);

	return	FTM_LIST_getAt(pEPM->pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}


FTM_RET	FTNM_EPM_notifyEPUpdated(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEPM != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_UPDATED;
	pMsg->xParams.xEPDataUpdated.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPDataUpdated.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTNM_MSGQ_push(pEPM->pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EPM_notifyEPDataSaveToDB(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEPM != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_SAVE_TO_DB;
	pMsg->xParams.xEPDataUpdated.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPDataUpdated.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTNM_MSGQ_push(pEPM->pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

/***********************************************************************
 * EP object operation
 ***********************************************************************/

FTM_RET	FTNM_EP_attach
(
	FTNM_EP_PTR 	pEP, 
	FTNM_NODE_PTR 	pNode
)
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
	FTM_TIMER		xCollectionTimer;
	FTM_TIMER		xTransmissionTimer;
	FTM_TIME		xCurrentTime, xAlignTime, xNextTime, xInterval, xCycle;
	FTM_BOOL		bStop = FTM_FALSE;

	pEP->xState = FTM_EP_STATE_RUN;

	TRACE("EP[%08x] process start.\n", pEP->xInfo.xEPID);

	FTM_TIME_getCurrent(&xAlignTime);
	switch(pEP->xInfo.ulInterval)
	{
	case       1: FTM_TIME_align(&xAlignTime, FTM_TIME_ALIGN_1S, &xAlignTime); break;
	case      10: FTM_TIME_align(&xAlignTime, FTM_TIME_ALIGN_10S, &xAlignTime); break;
	case      60: FTM_TIME_align(&xAlignTime, FTM_TIME_ALIGN_1M, &xAlignTime); break;
	case     600: FTM_TIME_align(&xAlignTime, FTM_TIME_ALIGN_10M, &xAlignTime); break;
	case    3600: FTM_TIME_align(&xAlignTime, FTM_TIME_ALIGN_1H, &xAlignTime); break;
	case 24*3600: FTM_TIME_align(&xAlignTime, FTM_TIME_ALIGN_1D, &xAlignTime); break;
	};

	FTM_TIME_setSeconds(&xInterval, pEP->xInfo.ulInterval);
	FTM_TIME_add(&xAlignTime, &xInterval, &xNextTime);

	FTM_TIME_getCurrent(&xCurrentTime);
	FTM_TIME_sub(&xNextTime, &xCurrentTime, &xInterval);

	FTM_TIMER_initTime(&xCollectionTimer, &xInterval);

	FTM_TIME_setSeconds(&xCycle, pEP->xInfo.ulCycle);
	FTM_TIME_add(&xAlignTime, &xCycle, &xNextTime);

	FTM_TIME_getCurrent(&xCurrentTime);
	FTM_TIME_sub(&xNextTime, &xCurrentTime, &xCycle);

	FTM_TIMER_initTime(&xTransmissionTimer, &xCycle);

	while(!bStop)
	{
		FTM_ULONG		ulRemainTime = 0;
		FTM_EP_DATA		xData, xReadData;
		FTNM_EP_MSG_PTR	pMsg = NULL;
	
		xData.ulTime = time(NULL);
		if (FTNM_NODE_getEPData(pEP->pNode, pEP, &xReadData) == FTM_RET_OK)
		{
			xData.xState = FTM_EP_DATA_STATE_VALID;
			xData.xType = xReadData.xType;
			memcpy(&xData.xValue, &xReadData.xValue, sizeof(xData.xValue));

			FTNM_EP_setData(pEP, &xData);
		}
		else
		{
			WARN("EP(%08x:%s) - The data import was failed.\n", pEP->xInfo.xEPID, pEP->pNode->xInfo.pDID);
		}

		if (FTM_TIMER_isExpired(&xTransmissionTimer))
		{
			TRACE("TRANSMISSION : %08x\n", pEP->xInfo.xEPID);
			FTM_ULONG	ulPrevTime, ulCurrentTime;

			FTM_TIMER_getTime(&xTransmissionTimer, &ulCurrentTime);
			ulPrevTime = ulCurrentTime - pEP->xInfo.ulCycle;

			FTNM_EP_transmissionData(pEP, ulCurrentTime, ulPrevTime);
			FTM_TIMER_add(&xTransmissionTimer, pEP->xInfo.ulCycle * 1000000);
		}
		
		FTM_TIMER_remain(&xCollectionTimer, &ulRemainTime);
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

			FTM_TIMER_remain(&xCollectionTimer, &ulRemainTime);

			FTM_MEM_free(pMsg);
		}
	
		if (!bStop)
		{
			FTM_TIMER_waitForExpired(&xCollectionTimer);
		}

		FTM_TIMER_add(&xCollectionTimer, pEP->xInfo.ulInterval * 1000000);

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
	FTM_EP_DATA_PTR		pLastData;
	FTM_EP_DATA_PTR		pNewData = NULL;

	xRet = FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pLastData);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_ERROR;
	}

	if (pLastData->xType != pData->xType)
	{
		ERROR("Data type missmatch[%08x:%08x]!\n", pLastData->xType, pData->xType);
		return	FTM_RET_INVALID_ARGUMENTS;
	}


	strcpy(pTimeString, ctime((time_t *)&pData->ulTime));
	pTimeString[strlen(pTimeString) - 1] = 0;

	xRet = FTM_EP_DATA_create(pData, &pNewData);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_LIST_count(&pEP->xDataList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
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
		if (xRet == FTM_RET_OK)
		{
			FTNM_EPM_notifyEPDataSaveToDB(pEP->pEPM, pEP->xInfo.xEPID, pData);
		}
		else
		{
			FTM_EP_DATA_destroy(pNewData);	
		}
	}

	return	xRet;
}

FTM_RET	FTNM_EP_transmissionData
(
	FTNM_EP_PTR 	pEP,
	FTM_ULONG		ulStartTime,
	FTM_ULONG		ulEndTime
)
{
	ASSERT(pEP != NULL);
	FTM_EP_DATA		xData;
	FTM_EP_DATA_PTR	pData;

	FTNM_EP_getData(pEP, &xData);

	switch (xData.xType)
	{
	case	FTM_EP_DATA_TYPE_INT:
		{
			FTM_INT	nMax = 0, nMin = 0, nAverage = 0, nCount = 0;

			FTM_LIST_iteratorStart(&pEP->xDataList);
			while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
			{
				if (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime)
				{
					if (nCount == 0)
					{
						nMax 	= pData->xValue.nValue;	
						nMin	= pData->xValue.nValue;	
						nAverage= pData->xValue.nValue;	
					}
					else
					{
						if (nMax < pData->xValue.nValue)
						{
							nMax = pData->xValue.nValue;	
						}
						else if (nMin > pData->xValue.nValue)
						{
							nMin = pData->xValue.nValue;	
						}

						nAverage += pData->xValue.nValue;
					}

					nCount++;
				}
			}

			if (nCount != 0)
			{
				nAverage = nAverage / nCount;
			}

			FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
			FTNM_EPM_notifyEPDataTransmissionINT(pEP->pEPM, pEP->xInfo.xEPID, pData->xValue.nValue, nAverage, nCount, nMax, nMin);
		}
		break;

	case	FTM_EP_DATA_TYPE_ULONG:
		{
			FTM_ULONG	ulMax = 0, ulMin = 0, ulAverage = 0, nCount = 0;

			FTM_LIST_iteratorStart(&pEP->xDataList);
			while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
			{
				if (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime)
				{
					if (nCount == 0)
					{
						ulMax 	= pData->xValue.nValue;	
						ulMin	= pData->xValue.nValue;	
						ulAverage= pData->xValue.nValue;	
					}
					else
					{
						if (ulMax < pData->xValue.nValue)
						{
							ulMax = pData->xValue.nValue;	
						}
						else if (ulMin > pData->xValue.nValue)
						{
							ulMin = pData->xValue.nValue;	
						}

						ulAverage += pData->xValue.nValue;
					}

					nCount++;
				}
			}

			if (nCount != 0)
			{
				ulAverage = ulAverage / nCount;
			}
			
			FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
			FTNM_EPM_notifyEPDataTransmissionULONG(pEP->pEPM, pEP->xInfo.xEPID, pData->xValue.ulValue, ulAverage, nCount, ulMax, ulMin);
		}
		break;

	case	FTM_EP_DATA_TYPE_FLOAT:
		{
			FTM_FLOAT	fMax = 0, fMin = 0, fAverage = 0, nCount = 0;

			FTM_LIST_iteratorStart(&pEP->xDataList);
			while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
			{
				if (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime)
				{
					if (nCount == 0)
					{
						fMax 	= pData->xValue.nValue;	
						fMin	= pData->xValue.nValue;	
						fAverage= pData->xValue.nValue;	
					}
					else
					{
						if (fMax < pData->xValue.nValue)
						{
							fMax = pData->xValue.nValue;	
						}
						else if (fMin > pData->xValue.nValue)
						{
							fMin = pData->xValue.nValue;	
						}

						fAverage += pData->xValue.nValue;
					}

					nCount++;
				}
			}

			if (nCount != 0)
			{
				fAverage = fAverage / nCount;
			}

			FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
			FTNM_EPM_notifyEPDataTransmissionFLOAT(pEP->pEPM, pEP->xInfo.xEPID, pData->xValue.fValue, fAverage, nCount, fMax, fMin);
		}
		break;

	case	FTM_EP_DATA_TYPE_BOOL:
		{
			FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData);
			FTNM_EPM_notifyEPDataTransmissionBOOL(pEP->pEPM, pEP->xInfo.xEPID, pData->xValue.bValue);
		}
		break;
	}

	
	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_getEventCount(FTNM_EP_PTR pEP, FTM_ULONG_PTR pulCount)
{
	ASSERT(pEP != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pEP->xTriggerList, pulCount);
}
#if 0
FTM_RET	FTNM_EP_getEventAt(FTNM_EP_PTR pEP, FTM_ULONG ulIndex, FTM_TRIGGER_ID_PTR pTriggerID)
{
	ASSERT(pEP != NULL);
	ASSERT(pTriggerID != NULL);
	
	return	FTM_LIST_getAt(&pEP->xTriggerList, ulIndex, (FTM_VOID_PTR _PTR_)&pTriggerID);
}
#endif
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

	return	FTM_RET_OK;
}

FTM_RET		FTNM_EPM_notifyEPDataTransmissionINT
(
	FTNM_EPM_PTR 	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_INT			nValue,
	FTM_INT 		nAverage, 
	FTM_INT 		nCount, 
	FTM_INT 		nMax, 
	FTM_INT 		nMin
)
{
	ASSERT(pEPM != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_INT;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nValue	= nValue;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nAverage = nAverage;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nCount 	= nCount;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nMax 	= nMax;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nMin 	= nMin;

	xRet = FTNM_MSGQ_push(pEPM->pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET		FTNM_EPM_notifyEPDataTransmissionULONG
(
	FTNM_EPM_PTR 	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulValue, 
	FTM_ULONG 		ulAverage, 
	FTM_INT 		nCount, 
	FTM_ULONG 		ulMax, 
	FTM_ULONG 		ulMin
)
{
	ASSERT(pEPM != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_ULONG;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulValue	= ulValue;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulAverage 	= ulAverage;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.nCount 	= nCount;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulMax 		= ulMax;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulMin 		= ulMin;

	xRet = FTNM_MSGQ_push(pEPM->pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET		FTNM_EPM_notifyEPDataTransmissionFLOAT
(
	FTNM_EPM_PTR	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_FLOAT 		fValue, 
	FTM_FLOAT 		fAverage, 
	FTM_INT 		nCount, 
	FTM_FLOAT 		fMax, 
	FTM_FLOAT 		fMin
)
{
	ASSERT(pEPM != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_FLOAT;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fValue		= fValue;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fAverage 	= fAverage;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.nCount 	= nCount;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fMax 		= fMax;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fMin 		= fMin;

	xRet = FTNM_MSGQ_push(pEPM->pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET		FTNM_EPM_notifyEPDataTransmissionBOOL
(
	FTNM_EPM_PTR 	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_BOOL 		bValue
)
{
	ASSERT(pEPM != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_BOOL;
	pMsg->xParams.xEPDataTrans.xValue.xBOOL.bValue	= bValue;

	xRet = FTNM_MSGQ_push(pEPM->pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTNM_EP_PTR		pEP = (FTNM_EP_PTR)pElement;
	FTM_EP_ID_PTR	pEPID=(FTM_EP_ID_PTR)pIndicator;

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

