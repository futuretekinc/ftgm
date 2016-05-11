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

static 
FTM_INT	FTOM_EP_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static 
FTM_INT	FTOM_EP_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

static 
FTM_INT	FTOM_EP_CLASS_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static 
FTM_INT	FTOM_EP_CLASS_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

static
FTM_LIST_PTR	pEPList = NULL;

static
FTM_LIST_PTR	pClassList = NULL;

/***********************************************************************
 * EP object operation
 ***********************************************************************/
FTM_RET	FTOM_EP_init
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	if (pEPList != NULL)
	{
		ERROR("It has already been initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create a list.\n");
		return	xRet;
	}

	xRet = FTM_LIST_create(&pClassList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create a list.\n");
		return	xRet;
	}

	FTM_LIST_setSeeker(pEPList, FTOM_EP_seeker);
	FTM_LIST_setComparator(pEPList, FTOM_EP_comparator);

	FTM_LIST_setSeeker(pClassList, FTOM_EP_CLASS_seeker);
	FTM_LIST_setComparator(pClassList, FTOM_EP_CLASS_comparator);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_final
(
	FTM_VOID
)
{
	if ((pEPList == NULL) || (pClassList == NULL))
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTOM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPList);
	while(FTM_LIST_iteratorNext(pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EP_destroy(&pEP);	
	}

	FTM_LIST_destroy(pEPList);
	pEPList = NULL;
/*
	FTM_EP_CLASS_PTR	pClass;

	FTM_LIST_iteratorStart(pClassList);
	while(FTM_LIST_iteratorNext(pClassList, (FTM_VOID_PTR _PTR_)&pClass) == FTM_RET_OK)
	{
		FTOM_EP_CLASS_destroy(&pClass);
	}

	FTM_LIST_final(pClassList);
*/
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_create
(
	FTM_EP_PTR 	pInfo,
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pInfo->pEPID, &pEP);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXISTS;
	}

	pEP = (FTOM_EP_PTR)FTM_MEM_malloc(sizeof(FTOM_EP));
	if (pEP == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pEP, 0, sizeof(FTOM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	pEP->bStop = FTM_TRUE;
	sem_init(&pEP->xLock, 0, 1);
	xRet = FTOM_MSGQ_init(&pEP->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("MsgQ	init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_init(&pEP->xDataList);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("Data list init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_init(&pEP->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("Trigger list init failed.\n");
		return	xRet;
	}

	xRet = FTOM_DB_EP_add(&pEP->xInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("EP[%s] failed to add to DB[%08x].\n", pEP->xInfo.pEPID, xRet);
		return	xRet;	
	}

	xRet = FTM_LIST_append(pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("EP[%s] failed to add to list[%08x].\n", pEP->xInfo.pEPID, xRet);
	}
	else
	{
		*ppEP = pEP;
	}

	return	xRet;
}

FTM_RET	FTOM_EP_createFromDB
(
	FTM_CHAR_PTR	pEPID,
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(ppEP != NULL);

	FTM_RET		xRet;
	FTM_EP		xInfo;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_DB_EP_getInfo(pEPID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	pEP = (FTOM_EP_PTR)FTM_MEM_malloc(sizeof(FTOM_EP));
	if (pEP == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pEP, 0, sizeof(FTOM_EP));
	memcpy(&pEP->xInfo, &xInfo, sizeof(FTM_EP));

	pEP->bStop = FTM_TRUE;
	sem_init(&pEP->xLock, 0, 1);
	xRet = FTOM_MSGQ_init(&pEP->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("MsgQ	init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_init(&pEP->xDataList);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("Data list init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_init(&pEP->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("Trigger list init failed.\n");
		return	xRet;
	}

	xRet = FTM_LIST_append(pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		ERROR("EP[%s] failed to add to list[%08x].\n", pEP->xInfo.pEPID, xRet);
	}
	else
	{
		*ppEP = pEP;
	}

	return	xRet;
}

FTM_RET	FTOM_EP_destroy
(
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pData;

	FTOM_EP_stop(*ppEP, TRUE);

	FTM_LIST_remove(pEPList, *ppEP);

	FTM_LIST_iteratorStart(&(*ppEP)->xDataList);
	while(FTM_LIST_iteratorNext(&(*ppEP)->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		FTM_EP_DATA_destroy(pData);	
	}

	xRet = FTM_LIST_final(&(*ppEP)->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger list finalize failed.\n");
	}

	xRet = FTM_LIST_final(&(*ppEP)->xDataList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Data list finalize failed.\n");
	}

	xRet = FTOM_MSGQ_final(&(*ppEP)->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("MsgQ finalize failed.\n");
	}

	sem_destroy(&(*ppEP)->xLock);

	xRet = FTM_MEM_free(*ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP failed to release memory!\n");
	}

	*ppEP = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_count
(
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pEPList, pulCount);
}


FTM_RET	FTOM_EP_get
(
	FTM_CHAR_PTR		pEPID,
	FTOM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPID != NULL);
	ASSERT(ppEP != NULL);
	
	return	FTM_LIST_get(pEPList, pEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTOM_EP_getAt
(
	FTM_ULONG 			ulIndex, 
	FTOM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(ppEP != NULL);

	return	FTM_LIST_getAt(pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_EP_setInfo
(
	FTOM_EP_PTR		pEP,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEP != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;

	xRet = FTOM_DB_EP_setInfo(pEP->xInfo.pEPID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xFields & FTM_EP_FIELD_FLAGS)
	{
		pEP->xInfo.xFlags = pInfo->xFlags;
	}

	if (xFields & FTM_EP_FIELD_NAME)
	{
		strcpy(pEP->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_EP_FIELD_UNIT)
	{
		strcpy(pEP->xInfo.pUnit, pInfo->pUnit);
	}

	if (xFields & FTM_EP_FIELD_ENABLE)
	{
		pEP->xInfo.bEnable = pInfo->bEnable;
	}

	if (xFields & FTM_EP_FIELD_TIMEOUT)
	{
		pEP->xInfo.ulTimeout = pInfo->ulTimeout;
	}

	if (xFields & FTM_EP_FIELD_INTERVAL)
	{
		pEP->xInfo.ulInterval = pInfo->ulInterval;
	}

	if (xFields & FTM_EP_FIELD_DID)
	{
		strcpy(pEP->xInfo.pDID, pInfo->pDID);
	}

	if (xFields & FTM_EP_FIELD_LIMIT)
	{
		if (pEP->xInfo.xLimit.xType != pInfo->xLimit.xType)
		{
			memset(&pEP->xInfo.xLimit, 0, sizeof(FTM_EP_LIMIT));
		}
		pEP->xInfo.xLimit.xType = pInfo->xLimit.xType;

		switch(pEP->xInfo.xLimit.xType)
		{
		case	FTM_EP_LIMIT_TYPE_COUNT:	
			{
				pEP->xInfo.xLimit.xParams.ulCount = pInfo->xLimit.xParams.ulCount;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_TIME:	
			{
				pEP->xInfo.xLimit.xParams.xTime.ulStart = pInfo->xLimit.xParams.xTime.ulStart;
				pEP->xInfo.xLimit.xParams.xTime.ulEnd = pInfo->xLimit.xParams.xTime.ulEnd;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_HOURS:	
			{
				pEP->xInfo.xLimit.xParams.ulHours= pInfo->xLimit.xParams.ulCount;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_DAYS:	
			{
				pEP->xInfo.xLimit.xParams.ulDays = pInfo->xLimit.xParams.ulCount;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_MONTHS:	
			{
				pEP->xInfo.xLimit.xParams.ulMonths = pInfo->xLimit.xParams.ulCount;
			}
			break;
		}
	}

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
			FTM_INT	nIndex;

			nIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 2], 0, 16);
			pEP->xOption.xSNMP.nOIDLen = MAX_OID_LEN;

			FTOM_NODE_SNMPC_getOID((FTOM_NODE_SNMPC_PTR)pNode, 
					(pEP->xInfo.xType >> 24) & 0xFF, 
					nIndex,
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
		ERROR("EP[%s] is not attached.\n", pEP->xInfo.pEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	if (!pEP->xInfo.bEnable)
	{
		INFO("EP[%s] is disabled.\n", pEP->xInfo.pEPID);
		return	FTM_RET_OK;
	}

	if (!pEP->bStop)
	{
		WARN("EP[%s] already started.\n", pEP->xInfo.pEPID);
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
		WARN("EP[%s] is stopped.\n", pEP->xInfo.pEPID);
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

	TRACE("EP[%s] process start.\n", pEP->xInfo.pEPID);

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
			WARN("It failed to import data from EP[%s].\n", pEP->xInfo.pEPID);
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
				TRACE("Receive Message : EP[%s], MSG[%08x]\n", pEP->xInfo.pEPID, pMsg->xType);
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
		ERROR("EP[%s] data push error!\n", pEP->xInfo.pEPID);
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
		ERROR("EP[%s] data pull error!\n", pEP->xInfo.pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_getDataCount
(
	FTOM_EP_PTR			pEP,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulCount != NULL);

	return	FTOM_DB_EP_getDataCount(pEP->xInfo.pEPID, pulCount);
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

FTM_RET	FTOM_EP_getDataList
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulIndex,
	FTM_EP_DATA_PTR	pDatas,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pDatas != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulCachedDataCount;
	FTM_ULONG	ulDataCount = 0;

	xRet = FTOM_EP_getDataCount(pEP, &ulCachedDataCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	if (ulIndex + ulMaxCount <  ulCachedDataCount)
	{
		FTM_INT	i;
		FTM_EP_DATA_PTR	pData;

		for(i = 0 ; i < ulMaxCount ; i++)
		{
			xRet = FTM_LIST_getAt(&pEP->xDataList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pData);
			if (xRet == FTM_RET_OK)
			{
				memcpy(&pDatas[ulDataCount++], pData, sizeof(FTM_EP_DATA));
			}
		}

		*pulCount = ulDataCount;
	}
	else
	{
		xRet = FTOM_DB_EP_getDataList(pEP->xInfo.pEPID, ulIndex, pDatas, ulMaxCount, pulCount);
	}

	return	FTM_RET_OK;
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
		ERROR("EP[%s] data type is unknown.\n", pEP->xInfo.pEPID);
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
	
		xRet = FTM_LIST_insert(&pEP->xDataList, pNewData, 0);
		if (xRet == FTM_RET_OK)
		{
			FTOM_DB_EP_addData(pEP->xInfo.pEPID, pData);
		}
		else
		{
			ERROR("Data append failed.\n");
			FTM_EP_DATA_destroy(pNewData);	
		}
	}
	else
	{
		ERROR("List count get failed.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_EP_removeData
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulDeletedCount != NULL);

	FTM_RET	xRet;

	xRet = FTOM_DB_EP_removeData(pEP->xInfo.pEPID, ulIndex, ulCount, pulDeletedCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%s] data failed to remove from DB[%08x].\n", pEP->xInfo.pEPID, xRet);	
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_removeDataWithTime
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulDeletedCount != NULL);

	FTM_RET	xRet;

	xRet = FTOM_DB_EP_removeDataWithTime(pEP->xInfo.pEPID, ulBegin, ulEnd, pulDeletedCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%s] data failed to remove from DB[%08x].\n", pEP->xInfo.pEPID, xRet);	
	}
	else
	{
		FTM_EP_DATA_PTR	pData;

		FTM_LIST_iteratorStart(&pEP->xDataList);
		while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
		{
			if(ulBegin <= pData->ulTime && pData->ulTime <= ulEnd)
			{
				FTM_LIST_remove(&pEP->xDataList, pData);
				FTM_EP_DATA_destroy(pData);
			}
			else if (ulBegin > pData->ulTime)
			{
				break;	
			}
		}
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_getDataInfo
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG_PTR	pulBegin,
	FTM_ULONG_PTR	pulEnd,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulBegin != NULL);
	ASSERT(pulEnd != NULL);
	ASSERT(pulCount != NULL);

	return	FTOM_DB_EP_getDataInfo(pEP->xInfo.pEPID, pulBegin, pulEnd, pulCount);
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

	i = 0;
	FTM_LIST_iteratorStart(&pEP->xDataList);
	while(FTM_LIST_iteratorNext(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		if ((i < nCount) && (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime))
		{
			memcpy(&pDataList[i++], pData, sizeof(FTM_EP_DATA));
		}
	}

	xRet = FTOM_SYS_EP_publishData(pEP->xInfo.pEPID, pDataList, nCount);
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

FTM_RET FTOM_EP_getIDList
(
	FTM_CHAR		pEPIDList[][FTM_EPID_LEN+1], 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(pEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTOM_EP_PTR	pEP;

		FTM_LIST_getAt(pEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
		strncpy(pEPIDList[ulCount++], pEP->xInfo.pEPID, FTM_EPID_LEN);
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}
FTM_INT	FTOM_EP_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_EP_PTR		pEP = (FTOM_EP_PTR)pElement;
	FTM_CHAR_PTR	pEPID=(FTM_CHAR_PTR)pIndicator;

	return	strcmp(pEP->xInfo.pEPID,pEPID) == 0;
}

FTM_INT	FTOM_EP_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTOM_EP_PTR		pEP1 = (FTOM_EP_PTR)pElement1;
	FTOM_EP_PTR		pEP2 = (FTOM_EP_PTR)pElement2;
	
	return	strcmp(pEP1->xInfo.pEPID, pEP2->xInfo.pEPID);
}

FTM_INT	FTOM_EP_CLASS_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	FTM_EP_CLASS_PTR	pEPClassInfo = (FTM_EP_CLASS_PTR)pElement;
	FTM_EP_TYPE_PTR		pEPClass=(FTM_EP_TYPE_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(pEPClassInfo->xType == *pEPClass);
}

FTM_INT	FTOM_EP_CLASS_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	FTM_EP_CLASS_PTR		pEPClassInfo1 = (FTM_EP_CLASS_PTR)pElement1;
	FTM_EP_CLASS_PTR		pEPClassInfo2 = (FTM_EP_CLASS_PTR)pElement2;
	
	return	(pEPClassInfo1->xType - pEPClassInfo2->xType);
}

FTM_RET	FTOM_EP_CLASS_create
(
	FTM_EP_CLASS_PTR 	pInfo,
	FTOM_EP_CLASS_PTR _PTR_	ppEPClass
)
{
	ASSERT(pInfo != NULL);

	FTOM_EP_CLASS_PTR        pEPClass;

	pEPClass = (FTOM_EP_CLASS_PTR)FTM_MEM_malloc(sizeof(FTOM_EP_CLASS));
	if(pEPClass == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pEPClass->xInfo, pInfo, sizeof(FTM_EP_CLASS));

	FTM_LIST_append(pClassList, pEPClass);
	
	return  FTM_RET_OK;
}

FTM_RET	FTOM_EP_CLASS_destroy
(
	FTOM_EP_CLASS_PTR _PTR_	ppEPClass
)
{
	ASSERT(ppEPClass != NULL);

	FTM_RET nRet;
       
	nRet = FTM_LIST_remove(pClassList, *ppEPClass);
	if (nRet == FTM_RET_OK)
	{
		FTM_MEM_free(ppEPClass); 

		*ppEPClass = NULL;
	}

	return  nRet;
}

FTM_RET	FTOM_EP_CLASS_count
(
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTM_LIST_count(pClassList, pulCount);
}

FTM_RET FTOM_EP_CLASS_get
(
	FTM_EP_TYPE 	xType, 
	FTOM_EP_CLASS_PTR _PTR_ ppEPClass
)
{
	return	FTM_LIST_get(pClassList, &xType, (FTM_VOID_PTR _PTR_)ppEPClass);
}

FTM_RET FTOM_EP_CLASS_getAt
(
	FTM_ULONG 		ulIndex, 
	FTOM_EP_CLASS_PTR _PTR_ ppEPClass
)
{
	return	FTM_LIST_getAt(pClassList, ulIndex, (FTM_VOID_PTR _PTR_)ppEPClass);
}

FTM_RET	FTOM_EP_printList
(
	FTM_VOID
)
{
	FTM_INT		i;
	FTM_ULONG	ulCount;
	FTOM_EP_PTR	pEP;
	
	MESSAGE("\n# EP Information\n");
	MESSAGE("%16s %16s %16s %16s %8s %24s\n", "EPID", "TYPE", "DID", "STATE", "VALUE", "TIME");
	FTOM_EP_count(&ulCount);
	for(i = 0; i < ulCount ; i++)
	{
		if (FTOM_EP_getAt(i, &pEP) == FTM_RET_OK)
		{
			FTM_CHAR	pTimeString[64];
			FTM_EP_DATA	xData;
		
			FTOM_EP_getData(pEP, &xData);

			ctime_r((time_t *)&xData.ulTime, pTimeString);
			if (strlen(pTimeString) != 0)
			{
				pTimeString[strlen(pTimeString) - 1] = '\0';
			}
			
			MESSAGE("%16s ", pEP->xInfo.pEPID);
			MESSAGE("%16s ", FTM_EP_typeString(pEP->xInfo.xType));
			if (pEP->pNode != NULL)
			{
				MESSAGE("%16s ", pEP->pNode->xInfo.pDID);
			}
	
			MESSAGE("%16s ", (!pEP->bStop)?"RUN":"STOP");
			MESSAGE("%8s ", FTM_VALUE_print(&xData.xValue));
			MESSAGE("%24s\n", pTimeString);
		}
	}
	
	return	FTM_RET_OK;
}
