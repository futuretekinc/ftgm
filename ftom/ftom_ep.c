#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_ep.h"
#include "ftom_dmc.h"
#include "ftom_message_queue.h"
#include "ftom_node_snmpc.h"
#include "ftom_logger.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_EP

#define	FTOM_EP_CACHED_DATA_COUNT	100

typedef	enum FTOM_EP_CMD_ENUM
{
	FTOM_EP_CMD_STOP,
} FTOM_EP_CMD, _PTR_ FTOM_EP_CMD_PTR;

typedef	struct	FTOM_EP_MSG_STRUCT
{
	FTOM_EP_CMD		xCmd;	
} FTOM_EP_MSG, _PTR_ FTOM_EP_MSG_PTR;

static 
FTM_VOID_PTR	FTOM_EP_threadMain
(
	FTM_VOID_PTR 	pData
);

static
FTM_RET	FTOM_EP_message
(
	FTOM_EP_PTR		pEP,
	FTOM_MSG_PTR	pBaseMsg
);

#if 0
static
FTM_RET	FTOM_EP_reportStatus
(
	FTOM_EP_PTR 	pEP
);

static
FTM_RET	FTOM_EP_reportDataInTime
(
	FTOM_EP_PTR 	pEP,
	FTM_ULONG		ulStartTime,
	FTM_ULONG		ulEndTime
);
#endif

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
		ERROR2(FTM_RET_ALREADY_INITIALIZED, "It has already been initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Can't create a list.\n");
		return	xRet;
	}

	xRet = FTM_LIST_create(&pClassList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Can't create a list.\n");
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
		FTOM_EP_destroy(&pEP, FTM_FALSE);	
	}

	FTM_LIST_destroy(pEPList);
	pEPList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_create
(
	FTM_EP_PTR 	pInfo,
	FTM_BOOL	bNew,
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
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pEP, 0, sizeof(FTOM_EP));
	memcpy(&pEP->xInfo, pInfo, sizeof(FTM_EP));

	pEP->bStop = FTM_TRUE;

	xRet = FTM_LOCK_create(&pEP->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Lock init failed.\n");
		goto error1;
	}

	xRet = FTOM_MSGQ_init(&pEP->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"MsgQ	init failed.\n");
		goto error2;
	}

	xRet = FTM_LIST_init(&pEP->xDataList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Data list init failed.\n");
		goto error3;
	}

	xRet = FTM_LIST_init(&pEP->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger list init failed.\n");
		goto error4;
	}

	xRet = FTM_LIST_append(pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP[%s] failed to add to list.\n", pEP->xInfo.pEPID);
		goto error5;
	}

	if (bNew)
	{
		xRet = FTOM_DB_EP_add(&pEP->xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add EP[%s] to DB!\n", pEP->xInfo.pEPID);
		}
		
		FTOM_addEPCreationLog(&pEP->xInfo);
	}


	*ppEP = pEP;

	return	FTM_RET_OK;

error5:
	FTM_LIST_final(&pEP->xTriggerList);

error4:
	FTM_LIST_final(&pEP->xDataList);

error3:
	FTOM_MSGQ_final(&pEP->xMsgQ);

error2:
	FTM_LOCK_destroy(&pEP->pLock);

error1:

	FTM_MEM_free(pEP);	

	return	xRet;
}

FTM_RET	FTOM_EP_destroy
(
	FTOM_EP_PTR _PTR_ ppEP,
	FTM_BOOL		bIncludeDB
)
{
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pData;

	if ((*ppEP)->pNode != NULL)
	{
		FTOM_NODE_unlinkEP((*ppEP)->pNode, *ppEP);
	}
	else
	{
		FTOM_EP_stop(*ppEP, TRUE);
	}

	if (bIncludeDB)
	{
		xRet = FTOM_DB_EP_remove((*ppEP)->xInfo.pEPID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove EP[%s] from DB.\n", (*ppEP)->xInfo.pEPID);
		}
		FTOM_addEPRemovalLog((*ppEP)->xInfo.pEPID);
	}

	xRet = FTM_LIST_remove(pEPList, *ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to remove EP[%s] from list.\n", (*ppEP)->xInfo.pEPID);
	}

	FTM_LIST_iteratorStart(&(*ppEP)->xDataList);
	while(FTM_LIST_iteratorNext(&(*ppEP)->xDataList, (FTM_VOID_PTR _PTR_)&pData) == FTM_RET_OK)
	{
		FTM_EP_DATA_destroy(pData);	
	}

	xRet = FTM_LIST_final(&(*ppEP)->xTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger list finalize failed.\n");
	}

	xRet = FTM_LIST_final(&(*ppEP)->xDataList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Data list finalize failed.\n");
	}

	xRet = FTOM_MSGQ_final(&(*ppEP)->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"MsgQ finalize failed.\n");
	}

	xRet = FTM_LOCK_destroy(&(*ppEP)->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy lock of EP[%s].\n", (*ppEP)->xInfo.pEPID);	
	}

	xRet = FTM_MEM_free(*ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP failed to release memory!\n");
	}

	*ppEP = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_count
(
	FTM_EP_TYPE			xType,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pulCount != NULL);

	FTM_ULONG	ulCount = 0;

	if ((xType == 0) && ((pDID == NULL) || (strlen(pDID) == 0)))
	{
		return	FTM_LIST_count(pEPList, pulCount);
	}

	FTOM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPList);
	while(FTM_LIST_iteratorNext(pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		if ((xType != 0) && (xType != pEP->xInfo.xType))
		{
			continue;	
		}

		if ((pDID != NULL) && (strcmp(pDID, pEP->xInfo.pDID) != 0))
		{
			continue;	
		}

		ulCount++;
	}

	*pulCount = ulCount;

	return	FTM_RET_OK;
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
		ERROR2(xRet,"EP[%s] failed to set info to DB.\n", pEP->xInfo.pEPID);
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

	if (xFields & FTM_EP_FIELD_TIMEOUT)
	{
		pEP->xInfo.ulTimeout = pInfo->ulTimeout;
	}

	if (xFields & FTM_EP_FIELD_INTERVAL)
	{
		pEP->xInfo.ulUpdateInterval = pInfo->ulUpdateInterval;
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

	if ((xFields & FTM_EP_FIELD_ENABLE) && (pEP->xInfo.bEnable != pInfo->bEnable))
	{
		pEP->xInfo.bEnable = pInfo->bEnable;
		if (pEP->xInfo.bEnable)
		{
			FTOM_EP_start(pEP);	
		}
		else 
		{
			FTOM_EP_stop(pEP, FTM_FALSE);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_EP_start
(
	FTOM_EP_PTR pEP
)
{
	ASSERT(pEP != NULL);

	FTM_RET	xRet;
	FTM_INT	nRet;

	if (pEP->pNode == NULL)
	{
		FTOM_NODE_PTR	pNode;


		xRet = FTOM_NODE_get(pEP->xInfo.pDID, &pNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Node[%s] is not found!\n", pEP->xInfo.pDID);
			return	xRet;
		}

		xRet = FTOM_NODE_linkEP(pNode, pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "The EP[%s] can not attached to The Node[%s].\n", pEP->xInfo.pEPID, pEP->xInfo.pDID);
			return	xRet;	
		}

		xRet = FTM_RET_EP_IS_NOT_ATTACHED;	
		ERROR2(xRet, "EP[%s] is not attached.\n", pEP->xInfo.pEPID);
		return	xRet;	
	}

	if (!pEP->xInfo.bEnable)
	{
		INFO("EP[%s] is disabled.\n", pEP->xInfo.pEPID);
		return	FTM_RET_OBJECT_IS_DISABLED;
	}

	if (!pEP->bStop)
	{
		WARN("EP[%s] already started.\n", pEP->xInfo.pEPID);
		return	FTM_RET_ALREADY_STARTED;
	}

	nRet = pthread_create(&pEP->xPThread, NULL, FTOM_EP_threadMain, (FTM_VOID_PTR)pEP);
	if (nRet != 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "Failed to create thread.\n");	
		return	xRet;
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
		return	FTM_RET_NOT_START;	
	}

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg = NULL;

	xRet = FTOM_MSG_createQuit(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] : Can't create quit message!\n", pEP->xInfo.pEPID);
	}
	else
	{
		xRet = FTOM_MSGQ_push(&pEP->xMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Message push failed.!\n");
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

	TRACE("EP[%s] is stopped!\n", pEP->xInfo.pEPID);
	return	FTM_RET_OK;
}

FTM_RET FTOM_EP_isRun
(
	FTOM_EP_PTR 		pEP, 
	FTM_BOOL_PTR		pbRun
)
{
	ASSERT(pEP != NULL);
	ASSERT(pbRun != NULL);

	*pbRun = !pEP->bStop;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_EP_threadMain
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	FTOM_EP_PTR	pEP = (FTOM_EP_PTR)pData;
	FTM_TIMER	xLoopTimer;


	TRACE("EP[%s] started.\n", pEP->xInfo.pEPID);

	TRACE("Update Interval : %lu ms\n",pEP->xInfo.ulUpdateInterval);
	TRACE("Report Interval : %lu ms\n",pEP->xInfo.ulReportInterval);

	pEP->bStop = FTM_FALSE;
	FTM_TIMER_initS(&pEP->xUpdateTimer, 0);
	FTM_TIMER_initS(&pEP->xReportTimer, 0);

	while(!pEP->bStop)
	{
		FTM_EP_DATA		xData;
		FTOM_MSG_PTR	pBaseMsg = NULL;

		if (FTM_TIMER_isExpired(&pEP->xUpdateTimer))
		{
			//if (FTOM_EP_isAsyncMode(pEP) != FTM_RET_TRUE)
			{
				xRet = FTOM_EP_remoteGetData(pEP, &xData);
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
			}
#if 0
			else
			{
				FTOM_EP_remoteGetDataAsync(pEP);
			}
#endif
			if (pEP->xInfo.ulUpdateInterval > 0)
			{
				FTM_TIMER_addMS(&pEP->xUpdateTimer, pEP->xInfo.ulUpdateInterval);
			}
			else
			{
				FTM_ULONG	ulUpdateInterval = 1;

				xRet = FTOM_getDefaultUpdateInterval(&ulUpdateInterval);
				if (xRet == FTM_RET_OK)
				{
					FTM_TIMER_addMS(&pEP->xUpdateTimer, ulUpdateInterval);
				}
			}
		}

		if (FTM_TIMER_isExpired(&pEP->xReportTimer))
		{
#if 0
			FTM_ULONG	ulPrevTime, ulCurrentTime;

			FTM_TIMER_getTime(&pEP->xReportTimer, &ulCurrentTime);
			ulPrevTime = ulCurrentTime - pEP->xInfo.ulReportInterval;

			FTOM_EP_reportStatus(pEP);
			FTOM_EP_reportDataInTime(pEP, ulPrevTime, ulCurrentTime);
#endif
			FTM_TIMER_addMS(&pEP->xReportTimer, pEP->xInfo.ulReportInterval);
		}
	
		FTM_ULONG		ulRemainTime1 = 0;
		FTM_ULONG		ulRemainTime2 = 0;
		FTM_ULONG		ulRemainTime  = 0;
	
		FTM_TIMER_remainMS(&pEP->xUpdateTimer, &ulRemainTime1);
		FTM_TIMER_remainMS(&pEP->xReportTimer, &ulRemainTime2);
		if (ulRemainTime1 < ulRemainTime2)
		{
			ulRemainTime = ulRemainTime1;	
		}
		else
		{
			ulRemainTime = ulRemainTime2;	
		}


		if (ulRemainTime == 0)
		{
			ulRemainTime = 100;	
		}

		FTM_TIMER_initMS(&xLoopTimer, ulRemainTime);

		while (!pEP->bStop)
		{
			xRet = FTOM_MSGQ_timedPop(&pEP->xMsgQ, ulRemainTime, &pBaseMsg);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			FTOM_EP_message(pEP, pBaseMsg);
			
			FTOM_MSG_destroy(&pBaseMsg);

			FTM_TIMER_remainMS(&xLoopTimer, &ulRemainTime);
		}
	} 

	TRACE("EP[%s] stopped.\n", pEP->xInfo.pEPID);

	return	0;
}

FTM_RET	FTOM_EP_message
(
	FTOM_EP_PTR		pEP,
	FTOM_MSG_PTR	pBaseMsg
)
{
	ASSERT(pEP != NULL);
	ASSERT(pBaseMsg != NULL);

	FTM_RET	xRet;

	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_EP_DATA:
		{
			FTM_INT	i;
			FTOM_MSG_EP_DATA_PTR	pMsg = (FTOM_MSG_EP_DATA_PTR)pBaseMsg;

			for(i = 0 ; i < pMsg->ulCount ; i++)
			{
				if (pMsg->pData[i].xState == FTM_EP_DATA_STATE_VALID)
				{
					xRet = FTOM_EP_setData(pEP, &pMsg->pData[i]);
				}
			}
		}
		break;

	case	FTOM_MSG_TYPE_QUIT:
		{
			pEP->bStop = FTM_TRUE;

			xRet = FTM_RET_OK;
		}
		break;

	default:
		{	
			xRet = FTM_RET_INVALID_MESSAGE_TYPE;
			ERROR2(xRet, "Invalid message[%08x]\n", pBaseMsg->xType);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_EP_getDataType
(
	FTOM_EP_PTR 		pEP, 
	FTM_VALUE_TYPE_PTR	pType
)
{
	ASSERT(pEP != NULL);
	ASSERT(pType != NULL);
	
	return	FTM_EP_getDataType(&pEP->xInfo, pType);
}

FTM_RET	FTOM_EP_remoteSetData
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
		ERROR2(xRet,"EP[%s] data push error!\n", pEP->xInfo.pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_remoteGetData
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
		ERROR2(xRet,"EP[%s] data pull error!\n", pEP->xInfo.pEPID);
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

FTM_RET	FTOM_EP_getDataCountWithTime
(
	FTOM_EP_PTR			pEP,
	FTM_ULONG			ulStart,
	FTM_ULONG			ulEnd,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulCount != NULL);

	return	FTOM_DB_EP_getDataCountWithTime(pEP->xInfo.pEPID, ulStart, ulEnd, pulCount);
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

	xRet = FTM_LIST_getFirst(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pTempData);
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
	FTM_ULONG_PTR	pulCount,
	FTM_BOOL_PTR	pbRemain
)
{
	ASSERT(pEP != NULL);
	ASSERT(pDatas != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulCachedDataCount;
	FTM_ULONG	ulDataCount = 0;

	xRet = FTM_LIST_count(&pEP->xDataList, &ulCachedDataCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to get EP[%s] data count.\n", pEP->xInfo.pEPID);
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
			else
			{
				ERROR2(xRet, "Failed to get EP data from cache! - Cached Data[%d]\n", ulIndex+i);	
			}
		}

		*pulCount = ulDataCount;
		*pbRemain = FTM_FALSE;
	}
	else
	{
		MESSAGE("%lu, %lu -> ", ulIndex, ulMaxCount);
		xRet = FTOM_DB_EP_getDataList(pEP->xInfo.pEPID, ulIndex, pDatas, ulMaxCount, pulCount, pbRemain);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get EP data from DB!\n");	
		}
		MESSAGE("%lu, %d\n", *pulCount, *pbRemain);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_getDataListWithTime
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR	pDatas,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount,
	FTM_BOOL_PTR	pbRemain
)
{
	ASSERT(pEP != NULL);
	ASSERT(pDatas != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;

	xRet = FTOM_DB_EP_getDataListWithTime(pEP->xInfo.pEPID, ulBegin, ulEnd, bAscending, pDatas, ulMaxCount, pulCount, pbRemain);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get EP data from DB!\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_EP_setServerTime
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulTime
)
{
	ASSERT(pEP != NULL);

	return	FTM_TIME_setSeconds(&pEP->xServerTime, ulTime);
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
	FTM_VALUE_TYPE		xDataType;
	FTM_EP_DATA_PTR		pNewData = NULL;

	xRet = FTM_EP_getDataType(&pEP->xInfo, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] data type is unknown.\n", pEP->xInfo.pEPID);
		return	FTM_RET_ERROR;
	}

	if (xDataType != pData->xValue.xType)
	{
		ERROR2(xRet,"EP[%s] data type missmatch[%08x:%08x]!\n", pEP->xInfo.pEPID, xDataType, pData->xValue.xType);
		return	FTM_RET_INVALID_ARGUMENTS;
	}


	xRet = FTM_EP_DATA_create(pData, &pNewData);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Data creation failed[%08x]!\n", pData->xValue.xType);
		return	xRet;	
	}

	xRet = FTOM_DB_EP_addData(pEP->xInfo.pEPID, pData);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"DB store failed!\n");	
	}

	xRet = FTM_LIST_count(&pEP->xDataList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		while (ulCount >= FTOM_EP_CACHED_DATA_COUNT)
		{
			FTM_EP_DATA_PTR	pTempData = NULL;

			xRet = FTM_LIST_getLast(&pEP->xDataList, (FTM_VOID_PTR _PTR_)&pTempData);	
			if (xRet != FTM_RET_OK)
			{
				FTM_EP_DATA_destroy(pNewData);
				return	xRet;	
			}

			xRet = FTM_LIST_remove(&pEP->xDataList, pTempData);	
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet,"Data remove failed.\n");	
				break;
			}

			FTM_EP_DATA_destroy(pTempData);	
			ulCount--;
		}
	}
	
	xRet = FTM_LIST_insert(&pEP->xDataList, pNewData, 0);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Data append failed.\n");
		FTM_EP_DATA_destroy(pNewData);	
	}

	return	xRet;
}

FTM_RET	FTOM_EP_removeDataList
(
	FTOM_EP_PTR		pEP,
	FTM_INT			nIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulDeletedCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulListCount, ulDataCount;

	xRet = FTM_LIST_count(&pEP->xDataList, &ulListCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP Data count unknown[%08x]\n", ulListCount);
		return	xRet;
	}

	if (nIndex >= 0)
	{
		if (nIndex < ulListCount)
		{
			FTM_INT	i;
	
			if (nIndex + ulCount > ulListCount)
			{
				ulCount = ulListCount  - nIndex;
			}
	
			for(i = 0 ; i < ulCount ; i++)
			{
				FTM_EP_DATA_PTR	pData;

				xRet = FTM_LIST_getAt(&pEP->xDataList, nIndex, (FTM_VOID_PTR _PTR_)&pData);	
				if (xRet == FTM_RET_OK)
				{
					FTM_LIST_removeAt(&pEP->xDataList, nIndex);
					FTM_EP_DATA_destroy(pData);
				}
			}
		}
	
		xRet = FTOM_DB_EP_removeData(pEP->xInfo.pEPID, nIndex, ulCount, pulDeletedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"EP[%s] data failed to remove from DB.\n", pEP->xInfo.pEPID);	
		}
	}
	else
	{
		xRet = FTOM_DB_EP_removeData(pEP->xInfo.pEPID, -1, ulCount, pulDeletedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"EP[%s] data failed to remove from DB.\n", pEP->xInfo.pEPID);	
		}

		xRet = FTOM_DB_EP_getDataCount(pEP->xInfo.pEPID, &ulDataCount);
		if (xRet == FTM_RET_OK)
		{
			if (ulDataCount < ulListCount)
			{
				FTM_INT	i;

				for(i = 0; i < ulListCount - ulDataCount ; i++)
				{
					FTM_EP_DATA_PTR	pData;

					xRet = FTM_LIST_getAt(&pEP->xDataList, ulDataCount, (FTM_VOID_PTR _PTR_)&pData);	
					if (xRet == FTM_RET_OK)
					{
						FTM_LIST_removeAt(&pEP->xDataList, ulDataCount);	
						FTM_EP_DATA_destroy(pData);
					}

				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_removeDataListWithTime
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
		ERROR2(xRet,"EP[%s] data failed to remove from DB.\n", pEP->xInfo.pEPID);	
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

FTM_RET	FTOM_EP_setDataLimit
(
	FTOM_EP_PTR			pEP,
	FTM_EP_LIMIT_PTR	pLimit
)
{
	ASSERT(pEP != NULL);
	ASSERT(pLimit != NULL);
	
	FTM_RET	xRet;

	xRet = FTOM_DB_EP_setDataLimit(pEP->xInfo.pEPID, pLimit);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set EP data limit!\n");
	}
	else
	{
		memcpy(&pEP->xInfo.xLimit, pLimit, sizeof(FTM_EP_LIMIT));	
	}

	return	xRet;
}

FTM_RET	FTOM_EP_remoteGetDataAsync
(
	FTOM_EP_PTR 	pEP 
)
{
	ASSERT(pEP != NULL);

	FTM_RET			xRet;

	xRet = FTOM_NODE_getEPDataAsync(pEP->pNode, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] data pull error!\n", pEP->xInfo.pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_remoteSetDataAsync
(
	FTOM_EP_PTR 	pEP,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pEP != NULL);

	FTM_RET			xRet;

	xRet = FTOM_NODE_setEPDataAsync(pEP->pNode, pEP, pData);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] data push error!\n", pEP->xInfo.pEPID);
	}

	return	xRet;
}

#if 0
FTM_RET	FTOM_EP_reportStatus
(
	FTOM_EP_PTR 	pEP
)
{
	ASSERT(pEP != NULL);

	return	FTOM_SYS_EP_publishStatus(pEP->xInfo.pEPID, !pEP->bStop, pEP->xInfo.ulReportInterval);
}

FTM_RET	FTOM_EP_reportDataInTime
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
	FTM_INT			nIndex, nDataCount = 0, nCount = 0;
	FTM_ULONG		ulListCount = 0;

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
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY,"Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_LIST_count(&pEP->xDataList, &ulListCount);
	for(nIndex = ulListCount - 1 ; nIndex >= 0 ; nIndex--)
	{
		xRet = FTM_LIST_getAt(&pEP->xDataList, nIndex, (FTM_VOID_PTR _PTR_)&pData);
		if ((xRet == FTM_RET_OK) && (nDataCount < nCount))
		{
			if (ulStartTime < pData->ulTime && pData->ulTime <= ulEndTime)
			{
				memcpy(&pDataList[nDataCount++], pData, sizeof(FTM_EP_DATA));
			}
		}
	}

	xRet = FTOM_SYS_EP_publishData(pEP->xInfo.pEPID, pDataList, nDataCount);
	FTM_MEM_free(pDataList);

	return	xRet;
}
#endif

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
	FTM_EP_TYPE		xType,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR		pEPIDList[][FTM_EPID_LEN+1], 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET		xRet;
	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(pEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTOM_EP_PTR	pEP;

		xRet = FTM_LIST_getAt(pEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		if ((xType != 0) && (pEP->xInfo.xType != xType))
		{
			continue;	
		}

		if ((pDID != NULL) && (strlen(pDID) != 0) && (strcmp(pDID, pEP->xInfo.pDID) != 0))
		{
			continue;	
		}

		strncpy(pEPIDList[ulCount++], pEP->xInfo.pEPID, FTM_EPID_LEN);
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_setReportInterval
(
	FTOM_EP_PTR	pEP,
	FTM_ULONG		ulInterval
)
{
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	xRet = FTM_isValidInterval(ulInterval);
	if (xRet == FTM_RET_OK)
	{
		pEP->xInfo.ulReportInterval = ulInterval;
		if ((!pEP->bStop) && (!FTM_TIMER_isExpired(&pEP->xReportTimer)))
		{
			FTM_TIMER_initS(&pEP->xReportTimer, 0);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_EP_getReportInterval
(
	FTOM_EP_PTR	pEP,
	FTM_ULONG_PTR	pulInterval
)
{
	ASSERT(pEP != NULL);
	ASSERT(pulInterval != NULL);
		
	*pulInterval = pEP->xInfo.ulReportInterval;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_sendMessage
(
	FTOM_EP_PTR		pEP,
	FTOM_MSG_PTR	pMsg
)
{
	ASSERT(pEP != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	if (pEP->bStop)
	{
		xRet = FTOM_EP_message(pEP, pMsg);
		if (xRet == FTM_RET_OK)
		{
			FTOM_MSG_destroy(&pMsg);
		}
	}
	else
	{
		xRet = FTOM_MSGQ_push(&pEP->xMsgQ, pMsg);
	}

	return	xRet;
}

FTM_RET	FTOM_EP_isAsyncMode
(
	FTOM_EP_PTR		pEP
)
{
	ASSERT(pEP != NULL);

	return	FTM_EP_isAsyncMode(&pEP->xInfo);
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

FTM_RET	FTOM_EP_print
(
	FTOM_EP_PTR	pEP
)
{
	ASSERT(pEP != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pTimeString[64];
	FTM_EP_DATA	xData;
	FTM_ULONG	ulBegin, ulEnd, ulCount;

	MESSAGE("\n# EP Information\n");
	FTM_EP_print(&pEP->xInfo);
	MESSAGE("%16s : %s\n", 	"State",(!pEP->bStop)?"RUN":"STOP");
	xRet = FTOM_EP_getDataInfo(pEP, &ulBegin, &ulEnd, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %lu\n", "COUNT", ulCount);
	}
		
	xRet = FTOM_EP_getData(pEP, &xData);
	if (xRet == FTM_RET_OK)
	{
		ctime_r((time_t *)&xData.ulTime, pTimeString);
		if (strlen(pTimeString) != 0)
		{
			pTimeString[strlen(pTimeString) - 1] = '\0';
		}
		MESSAGE("%16s : %s\n", 	"Time", pTimeString);
		MESSAGE("%16s : %s\n", "VALUE", FTM_VALUE_print(&xData.xValue));	
	}
	else
	{
		MESSAGE("%16s : NOT EXISTS\n", "VALUE");
	}
	
	MESSAGE("%16s : %s\n", "Server Time", FTM_TIME_printf(&pEP->xServerTime, NULL));	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_printList
(
	FTM_VOID
)
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_ULONG	ulCount;
	FTOM_EP_PTR	pEP;
	FTM_EP_DATA	xData;
	
	MESSAGE("\n# EP Information\n");
	MESSAGE("%16s %16s %16s %16s %8s %8s %8s %8s %8s %24s\n", "EPID", "TYPE", "NAME", "DID", "STATE", "VALUE", "UNIT", "UPDATE", "REPORT", "TIME");
	FTOM_EP_count(0, NULL, &ulCount);
	for(i = 0; i < ulCount ; i++)
	{
		xRet = FTOM_EP_getAt(i, &pEP);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}
		
		xRet = FTOM_EP_getData(pEP, &xData);
		if (xRet != FTM_RET_OK)
		{
			xData.ulTime = 0;
			xData.xState = FTM_EP_DATA_STATE_INVALID;
		}

		MESSAGE("%16s ", pEP->xInfo.pEPID);
		MESSAGE("%16s ", FTM_EP_typeString(pEP->xInfo.xType));
		MESSAGE("%16s ", pEP->xInfo.pName);
		if (pEP->pNode != NULL)
		{
			MESSAGE("%16s ", pEP->pNode->xInfo.pDID);
		}
		else
		{
			MESSAGE("%16s ", "");
		}

		MESSAGE("%8s ", (!pEP->bStop)?"RUN":"STOP");
		if (xData.xState == FTM_EP_DATA_STATE_INVALID)
		{
			MESSAGE("%8s ", "N/A");
		}
		else
		{
			MESSAGE("%8s ", FTM_VALUE_print(&xData.xValue));
		}
		MESSAGE("%8s ", pEP->xInfo.pUnit);
		MESSAGE("%8lu ", pEP->xInfo.ulUpdateInterval);
		MESSAGE("%8lu ", pEP->xInfo.ulReportInterval);
		if (xData.xState == FTM_EP_DATA_STATE_INVALID)
		{
			MESSAGE("%24s\n", "N/A");
		}
		else
		{
			MESSAGE("%24s\n", FTM_TIME_printf2(xData.ulTime, NULL));
		}
	}
	
	return	FTM_RET_OK;
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

FTM_RET	FTOM_EP_CLASS_print
(
	FTOM_EP_CLASS_PTR	pClass
)
{
	ASSERT(pClass != NULL);

	MESSAGE("%8s : %s\n", "Type", 	FTM_EP_typeString(pClass->xInfo.xType));
	MESSAGE("%8s : %s\n", "ID",		pClass->xInfo.pID);
	MESSAGE("%8s : %s\n", "Name",	pClass->xInfo.pName);
	MESSAGE("%8s : %s\n", "SN",		pClass->xInfo.pSN);
	MESSAGE("%8s : %s\n", "State",	pClass->xInfo.pState);
	MESSAGE("%8s : %s\n", "Value",	pClass->xInfo.pValue);
	MESSAGE("%8s : %s\n", "Time", 	pClass->xInfo.pTime);

	return	FTM_RET_OK;
}
