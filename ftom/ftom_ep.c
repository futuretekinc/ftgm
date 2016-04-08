#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
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

/***********************************************************************
 * EP object operation
 ***********************************************************************/

FTM_RET	FTOM_EP_create
(
	FTM_EP_PTR pInfo,
	FTOM_EP_PTR _PTR_ ppEP
)
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

FTM_RET	FTOM_EP_destroy
(
	FTOM_EP_PTR _PTR_ ppEP
)
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

FTM_RET	FTOM_EP_init
(
	FTOM_EP_PTR pEP, 
	FTM_EP_PTR pInfo
)
{
	ASSERT(pEP != NULL);

	FTM_RET		xRet;

	memset(pEP, 0, sizeof(FTOM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	pEP->bStop = FTM_TRUE;
	sem_init(&pEP->xLock, 0, 1);
	xRet = FTOM_MSGQ_init(&pEP->xMsgQ);
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

FTM_RET	FTOM_EP_final
(
	FTOM_EP_PTR pEP
)
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

	xRet = FTOM_MSGQ_final(&pEP->xMsgQ);
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

FTM_RET	FTOM_EP_detach
(
	FTOM_EP_PTR pEP
)
{
	ASSERT(pEP != NULL);

	pEP->pNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_EP_start
(
	FTOM_EP_PTR pEP
)
{
	ASSERT(pEP != NULL);
	
	FTM_INT	nRet;

	if (pEP->pNode == NULL)
	{
		ERROR("EP[%08x] is not attached.\n", pEP->xInfo.xEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	if (!pEP->xInfo.bEnable)
	{
		INFO("EP[%08x] is disabled.\n", pEP->xInfo.xEPID);
		return	FTM_RET_OK;
	}

	if (!pEP->bStop)
	{
		WARN("EP[%08x] already started.\n", pEP->xInfo.xEPID);
		return	FTM_RET_ALREADY_STARTED;
	}

	nRet = pthread_create(&pEP->xPThread, NULL, FTOM_EP_process, (FTM_VOID_PTR)pEP);
	if (nRet != 0)
	{
		ERROR("Can't create thread.\n");	
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_stop
(
	FTOM_EP_PTR pEP, 
	FTM_BOOL 	bWaitForStop
)
{
	ASSERT(pEP != NULL);

	if (pEP->bStop)
	{
		WARN("EP[%08x] is stopped.\n", pEP->xInfo.xEPID);
		return	FTM_RET_NOT_START;	
	}

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg = NULL;

	xRet = FTOM_MSG_createQuit(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create quit message!\n");
	}
	else
	{
		xRet = FTOM_MSGQ_push(&pEP->xMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Message push failed[%08x].!\n", xRet);
			FTOM_MSG_destroy(&pMsg);
		}
	}

	if (pMsg == NULL)
	{
		pthread_cancel(pEP->xPThread);
	}

	if (bWaitForStop)
	{
		pthread_join(pEP->xPThread, NULL);
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_EP_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP = (FTOM_EP_PTR)pData;
	FTM_TIMER	xCollectionTimer;
	FTM_TIMER	xTransTimer;
	FTM_TIME	xCurrentTime, xAlignTime, xNextTime, xInterval, xCycle;

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
		FTOM_MSG_PTR	pMsg = NULL;

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
	
		do
		{
			FTM_TIMER_remain(&xCollectionTimer, &ulRemainTime);
		
			if (FTOM_MSGQ_timedPop(&pEP->xMsgQ, ulRemainTime, &pMsg) == FTM_RET_OK)
			{
				TRACE("Receive Message : EP[%08x], MSG[%08x]\n", pEP->xInfo.xEPID, pMsg->xType);
				switch(pMsg->xType)
				{
				case	FTOM_MSG_TYPE_QUIT:
					{
						pEP->bStop = FTM_TRUE;
					}
					break;

				default:
					{	
						WARN("Invalid message[%08x]\n", pMsg->xType);
					}
				}

				FTM_MEM_free(pMsg);
			}
		}
		while (!pEP->bStop && (FTM_TIMER_isExpired(&xCollectionTimer) != FTM_TRUE));
	
		FTM_TIMER_add(&xCollectionTimer, pEP->xInfo.ulInterval * 1000000);
	} 

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

FTM_RET	FTOM_EP_pushData
(
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
)
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

FTM_RET	FTOM_EP_pullData
(
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
)
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

FTM_RET	FTOM_EP_getData
(
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
)
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

FTM_RET	FTOM_EP_setData
(
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
)
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

	TRACE("Send data in time[%d ~ %d]\n", ulStartTime, ulEndTime);
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

	i = 0;
	FTM_LIST_iteratorStart(&pEP->xDataList);
	while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		if ((i < nCount) && (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime))
		{
			memcpy(&pDataList[i++], pData, sizeof(FTM_EP_DATA));
		}
	}

	xRet = FTOM_EPM_sendEPData(pEP->pEPM, pEP->xInfo.xEPID, pDataList, nCount);
	FTM_MEM_free(pDataList);

	return	xRet;
}

FTM_RET	FTOM_EP_getEventCount
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pEP->xTriggerList, pulCount);
}

