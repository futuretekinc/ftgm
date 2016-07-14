#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_snmp_client.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

FTM_RET	FTOM_NODE_FTM_init
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	FTM_RET				xRet;
	FTM_ULONG			ulEPCount;

	ASSERT(pNode != NULL);

	xRet = FTM_LOCK_create(&pNode->pLock);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Lock init failed!\n");
		return	xRet;	
	}

	xRet = FTOM_NODE_getEPCount((FTOM_NODE_PTR)pNode, 0, &ulEPCount);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Node[%s] get EP count failed.!\n", pNode->xCommon.xInfo.pDID);
		return	xRet;	
	}

	TRACE("NODE[%s] has %d EPs\n", pNode->xCommon.xInfo.pDID, ulEPCount);
	if (ulEPCount != 0)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTOM_EP_PTR				pEP;

			xRet = FTOM_NODE_getEPAt((FTOM_NODE_PTR)pNode, i, (FTOM_EP_PTR _PTR_)&pEP);
			if (xRet != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			FTM_INT	nIndex;
			nIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 3], 0, 16);

			xRet = FTOM_NODE_SNMPC_getOIDForValue(pNode, pEP->xInfo.xType, nIndex, &pEP->xOption.xSNMP.xOID);
			if (xRet != FTM_RET_OK)
			{
				WARN("Failed to get OID for EP[%08x].\n", pEP->xInfo.pEPID);
			}
		}
	}
	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FTM_prestart
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_set(pNode->pLock);

	FTM_LOCK_reset(pNode->pLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FTM_prestop
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_set(pNode->pLock);
	snmp_timeout();
	FTM_LOCK_reset(pNode->pLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FTM_final
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_LOCK_destroy(&pNode->pLock);

	return	FTM_RET_OK;
}

FTOM_NODE_CLASS	xNodeClassFTM = 
{
	.pModel 	= "ftm",
	.xType		= FTOM_NODE_TYPE_SNMPC,
	.fCreate	= (FTOM_NODE_CREATE)FTOM_NODE_SNMPC_create,
	.fDestroy	= (FTOM_NODE_DESTROY)FTOM_NODE_SNMPC_destroy,
	.fInit		= (FTOM_NODE_INIT)FTOM_NODE_FTM_init,
	.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_FTM_final,
	.fPrestart	= (FTOM_NODE_PRESTART)FTOM_NODE_FTM_prestart,
	.fPrestop	= (FTOM_NODE_PRESTOP)FTOM_NODE_FTM_prestop,
	.fGetEPCount= (FTOM_NODE_GET_EP_COUNT)FTOM_NODE_SNMPC_getEPCount,
	.fGetEPID	= (FTOM_NODE_GET_EP_ID)FTOM_NODE_SNMPC_getEPID,
	.fGetEPName	= (FTOM_NODE_GET_EP_NAME)FTOM_NODE_SNMPC_getEPName,
	.fGetEPState= (FTOM_NODE_GET_EP_STATE)FTOM_NODE_SNMPC_getEPState,
	.fGetEPUpdateInterval= (FTOM_NODE_GET_EP_UPDATE_INTERVAL)FTOM_NODE_SNMPC_getEPUpdateInterval,
	.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_SNMPC_getEPData,
	.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_SNMPC_setEPData,
	.fGetEPDataAsync	= (FTOM_NODE_GET_EP_DATA_ASYNC)FTOM_NODE_SNMPC_getEPDataAsync,
	.fSetEPDataAsync	= (FTOM_NODE_SET_EP_DATA_ASYNC)FTOM_NODE_SNMPC_setEPDataAsync,
};

