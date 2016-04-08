#include "ftom_discovery.h"
#include "ftm_list.h"

static 
FTM_RET	FTOM_DISCOVERY_messageCB
(
	FTOM_PTR	pOM,
	FTOM_MSG_DISCOVERY_PTR	pMsg,
	FTM_VOID_PTR	pObj
);

FTM_RET	FTOM_DISCOVERY_init
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTOM_PTR			pOM
)
{
	ASSERT(pOM != NULL);

	FTM_LIST_init(&pDiscovery->xNodeList);
	pDiscovery->pOM = pOM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_final
(
	FTOM_DISCOVERY_PTR	pDiscovery
)
{
	FTOM_DISCOVERY_NODE_PTR pNodeInfo = NULL;

	FTM_LIST_iteratorStart(&pDiscovery->xNodeList);
	while(FTM_LIST_iteratorNext(&pDiscovery->xNodeList, (FTM_VOID_PTR _PTR_)&pNodeInfo) == FTM_RET_OK)
	{
		FTM_LIST_remove(&pDiscovery->xNodeList, pNodeInfo);
		FTM_MEM_free(pNodeInfo);
	}

	FTM_LIST_final(&pDiscovery->xNodeList);
	pDiscovery->pOM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_run
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG			ulTimeout
)
{
	FTM_RET	xRet;
	FTOM_ON_MESSAGE_CALLBACK fOldCB;
	FTM_VOID_PTR	pOldData;

	TRACE("call FTOM_setMessageCallback\n");
	FTOM_setMessageCallback(pDiscovery->pOM, FTOM_MSG_TYPE_DISCOVERY, (FTOM_ON_MESSAGE_CALLBACK)FTOM_DISCOVERY_messageCB, (FTM_VOID_PTR)pDiscovery, &fOldCB, &pOldData);
	xRet = FTOM_nodeDiscovery(pDiscovery->pOM, "255.255.255.255", 1234);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	MESSAGE("Waiting for information gathering.\n");
	usleep(ulTimeout);

	FTOM_setMessageCallback(pDiscovery->pOM, FTOM_MSG_TYPE_DISCOVERY, fOldCB, pOldData, NULL, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_getNodeInfoCount
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pDiscovery != NULL);
	ASSERT(pulCount != NULL);

	FTM_LIST_count(&pDiscovery->xNodeList, pulCount);	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_getNodeInfoAt
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG			i,
	FTOM_DISCOVERY_NODE_PTR	pNodeInfo
)
{
	ASSERT(pDiscovery != NULL);
	ASSERT(pNodeInfo != NULL);

	FTM_RET	xRet;	
	FTOM_DISCOVERY_NODE_PTR	pItem;

	xRet = FTM_LIST_getAt(&pDiscovery->xNodeList, i, (FTM_VOID_PTR _PTR_)&pItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(pNodeInfo, pItem, sizeof(FTOM_DISCOVERY_NODE));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_messageCB
(
	FTOM_PTR	pOM,
	FTOM_MSG_DISCOVERY_PTR	pMsg,
	FTM_VOID_PTR	pObj
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);
	ASSERT(pObj != NULL);
	
	FTM_INT	i;
	FTOM_DISCOVERY_PTR	pDiscovery = (FTOM_DISCOVERY_PTR)pObj;
	FTOM_DISCOVERY_NODE_PTR pNodeInfo;

	pNodeInfo = (FTOM_DISCOVERY_NODE_PTR)FTM_MEM_malloc(sizeof(FTOM_DISCOVERY_NODE));
	if (pNodeInfo == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pNodeInfo->pDID, pMsg->pDID);
	for(i = 0 ; (i < pMsg->ulCount) && (i < (sizeof(pNodeInfo->pEPTypes) / sizeof(pNodeInfo->pEPTypes[i]))) ; i++)
	{
		pNodeInfo->pEPTypes[i] = pMsg->pTypes[i];	
	}

	FTM_LIST_append(&pDiscovery->xNodeList, pNodeInfo);

	return	FTM_RET_OK;		
}
