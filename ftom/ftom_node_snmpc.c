#include <stdlib.h>
#include "ftom.h"
#include "ftom_node_snmpc.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

FTM_ULONG		active_hosts = 0;

static FTM_RET	FTOM_NODE_SNMPC_getEPData
(
	FTOM_NODE_SNMPC_PTR	pSNMPC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

static FTM_RET	FTOM_NODE_SNMPC_setEPData
(
	FTOM_NODE_SNMPC_PTR	pSNMPC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

FTM_RET	FTOM_NODE_SNMPC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTOM_NODE_SNMPC_PTR	pNode;
	
	
	pNode = (FTOM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.xDescript.xType		= FTOM_NODE_TYPE_SNMPC;
	pNode->xCommon.xDescript.fInit		= (FTOM_NODE_INIT)FTOM_NODE_SNMPC_init;
	pNode->xCommon.xDescript.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_SNMPC_final;
	pNode->xCommon.xDescript.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_SNMPC_getEPData;
	pNode->xCommon.xDescript.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_SNMPC_setEPData;
	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_destroy
(
	FTOM_NODE_SNMPC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_LIST_final(&(*ppNode)->xCommon.xEPList);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_init
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	FTM_RET				nRet;
	FTM_ULONG			ulEPCount;

	ASSERT(pNode != NULL);

	sem_init(&pNode->xLock, 0, 1);

	nRet = FTOM_NODE_getEPCount((FTOM_NODE_PTR)pNode, &ulEPCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	TRACE("NODE(%08x)[%s] has %d EPs\n", pNode, pNode->xCommon.xInfo.pDID, ulEPCount);
	if (ulEPCount != 0)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTOM_EP_PTR				pEP;
			FTOM_EP_CLASS_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTOM_NODE_getEPAt((FTOM_NODE_PTR)pNode, i, (FTOM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			if (FTOM_EP_CLASS_get((pEP->xInfo.xType & FTM_EP_TYPE_MASK), &pEPClassInfo) != FTM_RET_OK)
			{
				TRACE("EP CLASS[%s] information not found\n", pEP->xInfo.pEPID);
				continue;
			}

			snprintf(pOIDName, sizeof(pOIDName) - 1, "%s::%s", 
				pNode->xCommon.xInfo.xOption.xSNMP.pMIB, 
				pEPClassInfo->xInfo.pValue);
			pEP->xOption.xSNMP.nOIDLen = MAX_OID_LEN;
			if (read_objid(pOIDName, pEP->xOption.xSNMP.pOID, &pEP->xOption.xSNMP.nOIDLen) == 0)
			{
				TRACE("Can't find MIB\n");
				continue;
			}

			FTM_INT	nIndex;
			nIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 3], 0, 16);
			pEP->xOption.xSNMP.pOID[pEP->xOption.xSNMP.nOIDLen++] = nIndex & 0xFF;
			FTM_LIST_append(&pNode->xCommon.xEPList, pEP);
		}
	}
	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_final
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_getEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	return	FTOM_SNMPC_getEPData(pNode, pEP, pData);
}

FTM_RET	FTOM_NODE_SNMPC_setEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	return	FTOM_SNMPC_setEPData(pNode, pEP, pData);
}

#if 0
FTM_VOID_PTR FTOM_NODE_SNMPC_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTOM_NODE_SNMPC_PTR	pSNMPC = (FTOM_NODE_SNMPC_PTR)pData;
	FTM_TIMER		xLoopTimer;
	FTOM_MSG_PTR	pMsg;

	TRACE("Node[%s] start.\n", pSNMPC->xCommon.xInfo.pDID);
	FTM_TIMER_init(&xLoopTimer, pSNMPC->xCommon.xInfo.ulInterval * 1000000);

	pSNMPC->xCommon.bStop = FTM_FALSE;
	while(!pSNMPC->xCommon.bStop)
	{
		FTM_ULONG		ulRemainTime = 0;
		
		FTM_TIMER_remain(&xLoopTimer, &ulRemainTime);
		while (!pSNMPC->xCommon.bStop && (FTOM_MSGQ_timedPop(&pSNMPC->xCommon.xMsgQ, ulRemainTime, &pMsg) == FTM_RET_OK))
		{
			FTM_TIMER_remain(&xLoopTimer, &ulRemainTime);

			FTM_MEM_free(pMsg);
		}
	
		if (!pSNMPC->xCommon.bStop)
		{
			FTM_TIMER_waitForExpired(&xLoopTimer);
		}

		FTM_TIMER_add(&xLoopTimer, pSNMPC->xCommon.xInfo.ulInterval * 1000000);

	} 
	TRACE("SNMPC stopped.\n");

	return	FTM_RET_OK;
}
#endif

static FTM_CHAR_PTR	pOIDNamePrefix[] =
{
	"",
	"temp",
	"humi",
	"vlt",
	"curr",
	"di",
	"do",
	"cnt",
	"prs",
	"dsc",
	"dev"
};

FTM_RET		FTOM_NODE_SNMPC_getOID
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	oid 				*pOID, 
	size_t 				*pnOIDLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pnOIDLen != NULL);

	FTM_CHAR	pBuff[1024];

	sprintf(pBuff, "%s:%sValue.%lu", pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pOIDNamePrefix[ulType], ulIndex);

	return	FTOM_SNMPC_getOID(pBuff, pOID, pnOIDLen);
}
