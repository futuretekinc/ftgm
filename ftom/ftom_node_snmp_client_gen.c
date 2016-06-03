#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_snmp_client.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

FTM_RET	FTOM_NODE_SNMPC_GEN_init
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

	xRet = FTOM_NODE_getEPCount((FTOM_NODE_PTR)pNode, &ulEPCount);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Node[%s] get EP count failed.!\n", pNode->xCommon.xInfo.pDID);
		return	xRet;	
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

FTM_RET	FTOM_NODE_SNMPC_GEN_final
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_LOCK_destroy(&pNode->pLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_GEN_getEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	return	FTOM_SNMPC_getEPData(pNode, pEP, pData);
}

FTM_RET	FTOM_NODE_SNMPC_GEN_setEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	return	FTOM_SNMPC_setEPData(pNode, pEP, pData);
}

FTOM_NODE_CLASS	xGeneralSNMP = 
{
	.pModel = "general",
	.xType		= FTOM_NODE_TYPE_SNMPC,
	.fCreate	= (FTOM_NODE_CREATE)FTOM_NODE_SNMPC_create,
	.fDestroy	= (FTOM_NODE_DESTROY)FTOM_NODE_SNMPC_destroy,
	.fInit		= (FTOM_NODE_INIT)FTOM_NODE_SNMPC_GEN_init,
	.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_SNMPC_GEN_final,
	.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_SNMPC_GEN_getEPData,
	.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_SNMPC_GEN_setEPData,
};
