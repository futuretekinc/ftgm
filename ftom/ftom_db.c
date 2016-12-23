#include "ftom.h"
#include "ftom_db.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_FTOM

/******************************************************************
 * Node management interface
 ******************************************************************/
FTM_RET	FTOM_DB_NODE_add
(
	FTM_NODE_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_add(pService->pData, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add Node[%s] to DB.\n", pInfo->pDID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_remove
(
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pDID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_remove(pService->pData, pDID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove Node[%s] from DB.\n", pDID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_count
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_count(pService->pData, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get node count from DB.\n");
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET FTOM_DB_NODE_getDIDList
(
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDIDs != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_getDIDList(pService->pData, pDIDs, ulIndex, ulMaxCount, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get node id list from DB.\n");			
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}
	
	return	xRet;
}

FTM_RET	FTOM_DB_NODE_getInfo
(
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_get(pService->pData, pDID, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get Node[%s] info from DB!\n", pDID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_getAt(pService->pData, ulIndex, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get Node at[%d]!\n", ulIndex);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_setInfo
(
	FTM_CHAR_PTR	pDID,
	FTM_ULONG		xFields,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_NODE_set(pService->pData, pDID, xFields, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get Node[%s]!\n", pDID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

/******************************************************************
 * EP management interface
 ******************************************************************/

FTM_RET	FTOM_DB_EP_add
(
	FTM_EP_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_add(pService->pData, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add EP[%s] to DB!\n", pInfo->pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_remove
(
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pEPID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_remove(pService->pData, pEPID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove EP[%s] from DB!\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET FTOM_DB_EP_getEPIDList
(
	FTM_EPID_PTR	pEPIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEPIDs != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_getEPIDList(pService->pData, pEPIDs, ulIndex, ulMaxCount, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get rule id list from DB.\n");			
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}
	
	return	xRet;
}

FTM_RET	FTOM_DB_EP_getInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_get(pService->pData, pEPID, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"EP[%s] failed to get info from DB.\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_setInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_set(pService->pData, pEPID, xFields, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"EP[%s] failed to set info to DB.\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getDataList
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulStart, 
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount,
	FTM_BOOL_PTR	pbRemain
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pDataList != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_get(pService->pData, pEPID, ulStart, pDataList, ulMaxCount, pulCount, pbRemain);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to EP[%s] data.\n", pEPID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getDataListWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulBegin, 
	FTM_ULONG 		ulEnd, 
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount,
	FTM_BOOL_PTR	pbRemain
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pDataList != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_getWithTime(pService->pData, pEPID, ulBegin, ulEnd, bAscending, pDataList, ulMaxCount, pulCount, pbRemain);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to EP[%s] data.\n", pEPID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getDataInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_info(pService->pData, pEPID, pulBeginTime, pulEndTime, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to EP[%s] data info.\n", pEPID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_setDataLimit
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_LIMIT_PTR	pLimit
)
{
	ASSERT(pEPID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_setLimit(pService->pData, pEPID, pLimit);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to set EP[%s] data limit.\n", pEPID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_callback
(
	FTOM_SERVICE_ID 	xID, 
	FTOM_MSG_TYPE 		xMsg, 
	FTM_VOID_PTR 		pData
)
{
	switch(xID)
	{
	case	FTOM_SERVICE_SERVER:
		{
		}
		break;

	case	FTOM_SERVICE_SNMP_CLIENT:
		{
		}
		break;

	case	FTOM_SERVICE_SNMPTRAPD:
		{
		}
		break;

	case	FTOM_SERVICE_DMC:
		{
		}
		break;

	default:
		{
			ERROR2(FTM_RET_INVALID_TYPE,"Invalid service ID[%08x] received.\n", xID);
		}
	}

	return	FTM_RET_OK;
}

/************************************************************
 *	Node management 
 ************************************************************/

FTM_RET	FTOM_DB_EP_addData
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createEPData(NULL, pEPID, pData, 1, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_SERVICE_sendMessage(FTOM_SERVICE_DMC, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy(&pMsg);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getDataCount
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_count(pService->pData, pEPID, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Failed to EP[%s] data count from DMC!\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getDataCountWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_countWithTime(pService->pData, pEPID, ulStart, ulEnd, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Failed to EP[%s] data count from DMC!\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_removeData
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_remove(pService->pData, pEPID, ulIndex, ulCount, pulDeletedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove EP[%s] data!\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_removeDataWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EP_DATA_removeWithTime(pService->pData, pEPID, ulBegin, ulEnd, pulDeletedCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove EP[%s] data with time.\n", pEPID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EVENT_add
(
	FTM_EVENT_PTR		pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EVENT_add(pService->pData, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Event[%s] failed to add to DB.\n", pInfo->pID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}
	
	return	xRet;
}

FTM_RET	FTOM_DB_EVENT_remove
(
	FTM_CHAR_PTR	pEventID
)
{
	ASSERT(pEventID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EVENT_remove(pService->pData, pEventID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Event[%s] failed to remove from DB.\n", pEventID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET FTOM_DB_EVENT_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EVENT_getIDList(pService->pData, pIDs, ulIndex, ulMaxCount, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get trigger id list from DB.\n");			
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}
	
	return	xRet;
}

FTM_RET	FTOM_DB_EVENT_getInfo
(
	FTM_CHAR_PTR	pEventID,
	FTM_EVENT_PTR	pInfo
)
{
	ASSERT(pEventID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EVENT_get(pService->pData, pEventID, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Event[%s] failed to get information.\n", pEventID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_TRIgger_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_EVENT_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EVENT_getAt(pService->pData, ulIndex, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Event[%d] failed to get information.\n", ulIndex);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EVENT_setInfo
(
	FTM_CHAR_PTR		pEventID,
	FTM_EVENT_FIELD	xFields,
	FTM_EVENT_PTR		pInfo
)
{
	ASSERT(pEventID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_EVENT_set(pService->pData, pEventID, xFields, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Event[%s] DB update failed.\n", pEventID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_add
(
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_ACTION_add(pService->pData, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Action[%s] failed to add to DB.\n", pInfo->pID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_remove
(
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pActionID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_ACTION_remove(pService->pData, pActionID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Action[%s] failed to remove from DB.\n", pActionID);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;	
}

FTM_RET FTOM_DB_ACTION_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_ACTION_getIDList(pService->pData, pIDs, ulIndex, ulMaxCount, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get action id list from DB.\n");			
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}
	
	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_getInfo
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_ACTION_get(pService->pData, pActionID, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Action[%s] info failed to get from DB.\n", pActionID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_ACTION_getAt(pService->pData, ulIndex, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Action[%lu] info failed to get from DB.", ulIndex);
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;	
}

FTM_RET	FTOM_DB_ACTION_setInfo
(
	FTM_CHAR_PTR		pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pActionID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_ACTION_set(pService->pData, pActionID, xFields, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Action[%s] DB update failed.\n", pActionID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_RULE_add
(
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_RULE_add(pService->pData, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Failed to add rule[%s].\n", pInfo->pID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_RULE_remove
(
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_RULE_remove(pService->pData, pRuleID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Failed to remove rule[%s].\n", pRuleID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}

FTM_RET FTOM_DB_RULE_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_RULE_getIDList(pService->pData, pIDs, ulIndex, ulMaxCount, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get rule id list from DB.\n");			
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}
	
	return	xRet;
}

FTM_RET	FTOM_DB_RULE_getInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_RULE_get(pService->pData, pRuleID, pInfo);
}

FTM_RET	FTOM_DB_RULE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_RULE_getAt(pService->pData, ulIndex, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Failed to get rule at[%d].\n", ulIndex);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;	
}

FTM_RET	FTOM_DB_RULE_setInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_DMC_RULE_set(pService->pData, pRuleID, xFields, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"Rule[%s] DB update failed.\n", pRuleID);	
		}
	}
	else
	{
		ERROR2(xRet, "Data management service not found!\n");
	}

	return	xRet;
}


