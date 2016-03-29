#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_ep.h"
#include "ftom_dmc.h"

#define	FTOM_EP_DATA_COUNT	32

typedef	enum FTOM_EP_CMD_ENUM
{
	FTOM_EP_CMD_STOP,
} FTOM_EP_CMD, _PTR_ FTOM_EP_CMD_PTR;

typedef	struct	FTOM_EP_MSG_STRUCT
{
	FTOM_EP_CMD		xCmd;	
} FTOM_EP_MSG, _PTR_ FTOM_EP_MSG_PTR;

static FTM_VOID_PTR	FTOM_EP_process
(
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_EP_sendDataInTime
(
	FTOM_EP_PTR 	pEP,
	FTM_ULONG		ulStartTime,
	FTM_ULONG		ulEndTime
);

static FTM_INT	FTOM_EP_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_INT	FTOM_EP_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

FTM_RET	FTOM_EPM_create(FTOM_PTR pOM, FTOM_EPM_PTR _PTR_ ppEPM)
{
	ASSERT(pOM != NULL);
	ASSERT(ppEPM != NULL);

	FTM_RET			xRet;
	FTOM_EPM_PTR	pEPM;

	pEPM = (FTOM_EPM_PTR)FTM_MEM_malloc(sizeof(FTOM_EPM));
	if (pEPM == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_EPM_init(pOM, pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EPM init failed[%08x].\n", xRet);
		FTM_MEM_free(pEPM);
		return	xRet;
	}

	*ppEPM = pEPM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_destroy(FTOM_EPM_PTR _PTR_ ppEPM)
{
	ASSERT(ppEPM != NULL);
	
	FTM_RET	xRet;

	if (*ppEPM == NULL)
	{
		ERROR("EPM has not been initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTOM_EPM_final(*ppEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EPM finalize was failed.\n");
	}

	FTM_MEM_free(*ppEPM);
	*ppEPM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_init(FTOM_PTR pOM, FTOM_EPM_PTR pEPM)
{
	ASSERT(pOM != NULL);
	ASSERT(pEPM != NULL);

	FTM_RET	xRet;

	if (pEPM->pEPList != NULL)
	{
		ERROR("It has already been initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pEPM->pEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create a list.\n");
		return	xRet;
	}

	pEPM->pOM = pOM;
	FTM_LIST_setSeeker(pEPM->pEPList, FTOM_EP_seeker);
	FTM_LIST_setComparator(pEPM->pEPList, FTOM_EP_comparator);

	TRACE("EP management initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_final(FTOM_EPM_PTR pEPM)
{
	ASSERT(pEPM != NULL);

	if (pEPM->pEPList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTOM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPM->pEPList);
	while(FTM_LIST_iteratorNext(pEPM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EPM_destroyEP(pEPM, pEP);	
	}

	FTM_LIST_destroy(pEPM->pEPList);
	pEPM->pEPList = NULL;

	TRACE("EP management finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_createEP(FTOM_EPM_PTR pEPM, FTM_EP_PTR pInfo, FTOM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP = NULL;

	xRet = FTOM_EP_create(pInfo, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("The EP[%08x] creation failed.\n", pInfo->xEPID);
		return	xRet;	
	}

	xRet = FTOM_EPM_attachEP(pEPM, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't attach the EP[%08x] to the EPM.\n", pInfo->xEPID);
		return	xRet;	
	}

	*ppEP = pEP;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_destroyEP(FTOM_EPM_PTR pEPM, FTOM_EP_PTR	pEP)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	FTOM_EP_stop(pEP, TRUE);

	xRet = FTOM_EPM_detachEP(pEPM, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't detach the EP[%08x] from the EPM.\n", pEP->xInfo.xEPID);
		return	xRet;	
	}

	FTOM_EP_destroy(&pEP);

	return	xRet;
}

FTM_RET	FTOM_EPM_attachEP(FTOM_EPM_PTR pEPM, FTOM_EP_PTR pEP)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_append(pEPM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't attach EP[%08x] to EPM.\n", pEP->xInfo.xEPID);
		return	xRet;	
	}

	pEP->pEPM = pEPM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_detachEP(FTOM_EPM_PTR pEPM, FTOM_EP_PTR pEP)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	if (pEP->pEPM != pEPM)
	{
		WARN("EP[%08x] is not attached.\n", pEP->xInfo.xEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;
	}

	xRet = FTM_LIST_remove(pEPM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		WARN("EP[%08x] is not attached.\n", pEP->xInfo.xEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;
	}

	pEP->pEPM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_count
(
	FTOM_EPM_PTR	pEPM,
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
			FTOM_EP_PTR	pEP;

			FTM_LIST_getAt(pEPM->pEPList, i,(FTM_VOID_PTR _PTR_)&pEP);
			if (xType == (pEP->xInfo.xEPID & FTM_EP_TYPE_MASK))
			{
				ulCount++;
			}
		}


		*pulCount = ulCount;
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_EPM_getIDList
(
	FTOM_EPM_PTR		pEPM,
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
		FTOM_EP_PTR	pEP;

		FTM_LIST_getAt(pEPM->pEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
		if ((xType == 0) || (xType == (pEP->xInfo.xEPID & FTM_EP_TYPE_MASK)))
		{
			pEPIDList[ulCount++] = pEP->xInfo.xEPID;
		}
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_get
(
	FTOM_EPM_PTR		pEPM,
	FTM_EP_ID 			xEPID, 
	FTOM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);
	
	return	FTM_LIST_get(pEPM->pEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTOM_EPM_getAt
(
	FTOM_EPM_PTR		pEPM,
	FTM_ULONG 			ulIndex, 
	FTOM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);

	return	FTM_LIST_getAt(pEPM->pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_EPM_saveEPData
(
	FTOM_EPM_PTR	pEPM,
	FTM_EP_ID		xEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pData != NULL);

	return	FTOM_saveEPData(pEPM->pOM, xEPID, pData);
}

FTM_RET	FTOM_EPM_sendEPData
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR pData,
	FTM_ULONG		ulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pData != NULL);

	return	FTOM_sendEPData(pEPM->pOM, xEPID, pData, ulCount);
}
/***********************************************************************
 * EP object operation
 ***********************************************************************/

FTM_RET	FTOM_EP_create(FTM_EP_PTR pInfo,FTOM_EP_PTR _PTR_ ppEP)
{
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR	pEP;

	pEP = (FTOM_EP_PTR)FTM_MEM_malloc(sizeof(FTOM_EP));
	if (pEP == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTOM_EP_init(pEP, pInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		return	xRet;
	}

	*ppEP = pEP;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_destroy(FTOM_EP_PTR _PTR_ ppEP)
{
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;

	xRet = FTOM_EP_final(*ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP finailize failed.\n");
	}

	xRet = FTM_MEM_free(*ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP failed to release memory!\n");
	}

	*ppEP = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_init(FTOM_EP_PTR pEP, FTM_EP_PTR pInfo)
{
	ASSERT(pEP != NULL);

	FTM_RET		xRet;

	memset(pEP, 0, sizeof(FTOM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	pEP->bStop = FTM_TRUE;
	sem_init(&pEP->xLock, 0, 1);
	xRet = FTM_MSGQ_init(&pEP->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MsgQ	init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_init(&pEP->xDataList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Data list init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_init(&pEP->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list init failed.\n");
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_final(FTOM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pData;

	FTM_LIST_iteratorStart(&pEP->xDataList);
	while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		FTM_EP_DATA_destroy(pData);	
	}

	xRet = FTM_LIST_final(&pEP->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list finalize failed.\n");
	}

	xRet = FTM_LIST_final(&pEP->xDataList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Data list finalize failed.\n");
	}

	xRet = FTM_MSGQ_final(&pEP->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MsgQ finalize failed.\n");
	}

	sem_destroy(&pEP->xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_attach
(
	FTOM_EP_PTR 	pEP, 
	FTOM_NODE_PTR 	pNode
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

			FTOM_NODE_SNMPC_getOID((FTOM_NODE_SNMPC_PTR)pNode, 
					(pEP->xInfo.xEPID >> 24) & 0xFF, 
					(pEP->xInfo.xEPID & 0xFF), 
					pEP->xOption.xSNMP.pOID, 
					&pEP->xOption.xSNMP.nOIDLen);
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_detach(FTOM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	pEP->pNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_EP_start(FTOM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	if (pEP->pNode == NULL)
	{
		ERROR("EP[%08x] is not attached.\n", pEP->xInfo.xEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	if (!pEP->bStop)
	{
		ERROR("EP[%08x] already started.\n", pEP->xInfo.xEPID);
		return	FTM_RET_ALREADY_STARTED;
	}

	pthread_create(&pEP->xPThread, NULL, FTOM_EP_process, (FTM_VOID_PTR)pEP);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_stop(FTOM_EP_PTR pEP, FTM_BOOL bWaitForStop)
{
	pEP->bStop = FTM_TRUE;

	if (bWaitForStop)
	{
		pthread_join(pEP->xPThread, NULL);
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_EP_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR	pEP = (FTOM_EP_PTR)pData;
	FTM_TIMER		xCollectionTimer;
	FTM_TIMER		xTransTimer;
	FTM_TIME		xCurrentTime, xAlignTime, xNextTime, xInterval, xCycle;

	pEP->bStop = FTM_FALSE;

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

	FTM_TIMER_initTime(&xTransTimer, &xCycle);

	while(!pEP->bStop)
	{
		FTM_ULONG		ulRemainTime = 0;
		FTM_EP_DATA		xData;
		FTOM_EP_MSG_PTR	pMsg = NULL;
	
		xRet = FTOM_EP_pullData(pEP, &xData);
		if (xRet == FTM_RET_OK)
		{
			xData.xState = FTM_EP_DATA_STATE_VALID;
			xData.ulTime = time(NULL);
			FTOM_EP_setData(pEP, &xData);
		}
		else
		{
			WARN("It failed to import data from EP[%s-%08x].\n", pEP->pNode->xInfo.pDID, pEP->xInfo.xEPID);
		}

		if (FTM_TIMER_isExpired(&xTransTimer))
		{
			FTM_ULONG	ulPrevTime, ulCurrentTime;

			FTM_TIMER_getTime(&xTransTimer, &ulCurrentTime);
			ulPrevTime = ulCurrentTime - pEP->xInfo.ulCycle;

			FTOM_EP_sendDataInTime(pEP, ulPrevTime, ulCurrentTime);
			FTM_TIMER_add(&xTransTimer, pEP->xInfo.ulCycle * 1000000);
		}
		
		FTM_TIMER_remain(&xCollectionTimer, &ulRemainTime);
		while (!pEP->bStop && (FTM_MSGQ_timedPop(&pEP->xMsgQ, ulRemainTime, (FTM_VOID_PTR _PTR_)&pMsg) == FTM_RET_OK))
		{
			FTM_TIMER_remain(&xCollectionTimer, &ulRemainTime);

			FTM_MEM_free(pMsg);
		}
	
		if (!pEP->bStop)
		{
			FTM_TIMER_waitForExpired(&xCollectionTimer);
		}

		FTM_TIMER_add(&xCollectionTimer, pEP->xInfo.ulInterval * 1000000);

	} 

	pEP->bStop = FTM_TRUE;

	TRACE_EXIT();

	return	0;
}

FTM_RET	FTOM_EP_getDataType
(
	FTOM_EP_PTR 			pEP, 
	FTM_EP_DATA_TYPE_PTR 	pType
)
{
	ASSERT(pEP != NULL);
	ASSERT(pType != NULL);
	
	return	FTM_EP_getDataType(&pEP->xInfo, pType);
}

FTM_RET	FTOM_EP_pushData(FTOM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;

	xRet = FTOM_NODE_setEPData(pEP->pNode, pEP, pData);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] data push error!\n", pEP->xInfo.xEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_pullData(FTOM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;

	xRet = FTOM_NODE_getEPData(pEP->pNode, pEP, pData);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] data pull error!\n", pEP->xInfo.xEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_getData(FTOM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
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

FTM_RET	FTOM_EP_setData(FTOM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_CHAR			pTimeString[64];
	FTM_EP_DATA_TYPE	xDataType;
	FTM_EP_DATA_PTR		pNewData = NULL;

	xRet = FTM_EP_getDataType(&pEP->xInfo, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] data type is unknown.\n", pEP->xInfo.xEPID);
		return	FTM_RET_ERROR;
	}

	if (xDataType != pData->xType)
	{
		ERROR("Data type missmatch[%08x:%08x]!\n", xDataType, pData->xType);
		return	FTM_RET_INVALID_ARGUMENTS;
	}


	strcpy(pTimeString, ctime((time_t *)&pData->ulTime));
	pTimeString[strlen(pTimeString) - 1] = 0;

	xRet = FTM_EP_DATA_create(pData, &pNewData);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Data creation failed[%08x]!\n", pData->xType);
		return	xRet;	
	}

	xRet = FTM_LIST_count(&pEP->xDataList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		if (ulCount >= FTOM_EP_DATA_COUNT)
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
			FTOM_EPM_saveEPData(pEP->pEPM, pEP->xInfo.xEPID, pData);
		}
		else
		{
			FTM_EP_DATA_destroy(pNewData);	
		}
	}

	return	xRet;
}

FTM_RET	FTOM_EP_sendDataInTime
(
	FTOM_EP_PTR 	pEP,
	FTM_ULONG		ulStartTime,
	FTM_ULONG		ulEndTime
)
{
	ASSERT(pEP != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pData;
	FTM_EP_DATA_PTR	pDataList;
	FTM_INT			i, nCount = 0;

	FTM_LIST_iteratorStart(&pEP->xDataList);
	while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		if (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime)
		{
			nCount++;
		}
	}

	if (nCount == 0)
	{
		return	FTM_RET_OK;
	}

	pDataList = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * nCount);
	if (pDataList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_LIST_iteratorStart(&pEP->xDataList);
	while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		if ((i < nCount) && (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime))
		{
			memcpy(&pDataList[i++], pData, sizeof(FTM_EP_DATA));
			nCount++;
		}
	}

	xRet = FTOM_EPM_sendEPData(pEP->pEPM, pEP->xInfo.xEPID, pDataList, nCount);
	FTM_MEM_free(pDataList);

	return	xRet;
}

FTM_RET	FTOM_EP_getEventCount(FTOM_EP_PTR pEP, FTM_ULONG_PTR pulCount)
{
	ASSERT(pEP != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pEP->xTriggerList, pulCount);
}

FTM_INT	FTOM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_EP_PTR		pEP = (FTOM_EP_PTR)pElement;
	FTM_EP_ID_PTR	pEPID=(FTM_EP_ID_PTR)pIndicator;

	return	(pEP->xInfo.xEPID == *pEPID);
}

FTM_INT	FTOM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTOM_EP_PTR		pEP1 = (FTOM_EP_PTR)pElement1;
	FTOM_EP_PTR		pEP2 = (FTOM_EP_PTR)pElement2;
	
	return	(pEP1->xInfo.xEPID - pEP2->xInfo.xEPID);
}

