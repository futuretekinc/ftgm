#include <stdlib.h>
#include "ftom.h"
#include "ftom_node_snmpc.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_ep_class.h"

FTM_ULONG		active_hosts = 0;
FTM_RET			FTOM_NODE_SNMPC_start(FTOM_NODE_SNMPC_PTR pNode);
FTM_RET			FTOM_NODE_SNMPC_stop(FTOM_NODE_SNMPC_PTR pNode);
FTM_RET			FTOM_NODE_SNMPC_getEPData(FTOM_NODE_SNMPC_PTR pNode, FTOM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
FTM_RET			FTOM_NODE_SNMPC_setEPData(FTOM_NODE_SNMPC_PTR pNode, FTOM_EP_PTR pEP, FTM_EP_DATA_PTR pData);

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

	memset(pNode, 0, sizeof(FTOM_NODE_SNMPC));

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));
	FTM_LIST_init(&pNode->xCommon.xEPList);
	pthread_mutex_init(&pNode->xCommon.xMutexLock, NULL);
	sem_init(&pNode->xLock, 0, 1);
	
	pNode->xCommon.fStart	= (FTOM_NODE_START)FTOM_NODE_SNMPC_start;
	pNode->xCommon.fStop 	= (FTOM_NODE_STOP)FTOM_NODE_SNMPC_stop;
	pNode->xCommon.fGetEPData=(FTOM_NODE_GET_EP_DATA)FTOM_NODE_SNMPC_getEPData;
	pNode->xCommon.fSetEPData=(FTOM_NODE_SET_EP_DATA)FTOM_NODE_SNMPC_setEPData;
	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_destroy
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_MEM_free(pNode);

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
			FTM_EP_CLASS_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTOM_NODE_getEPAt((FTOM_NODE_PTR)pNode, i, (FTOM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			if (FTOM_EP_CLASS_get((pEP->xInfo.xEPID & FTM_EP_TYPE_MASK), &pEPClassInfo) != FTM_RET_OK)
			{
				TRACE("EP CLASS[%08lx] information not found\n", pEP->xInfo.xEPID);
				continue;
			}

			snprintf(pOIDName, sizeof(pOIDName) - 1, "%s::%s", 
				pNode->xCommon.xInfo.xOption.xSNMP.pMIB, 
				pEPClassInfo->pValue);
			pEP->xOption.xSNMP.nOIDLen = MAX_OID_LEN;
			if (read_objid(pOIDName, pEP->xOption.xSNMP.pOID, &pEP->xOption.xSNMP.nOIDLen) == 0)
			{
				TRACE("Can't find MIB\n");
				continue;
			}
			pEP->xOption.xSNMP.pOID[pEP->xOption.xSNMP.nOIDLen++] = pEP->xInfo.xDEPID & 0xFF;
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

FTM_RET	FTOM_NODE_SNMPC_start
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	return	FTM_RET_OK;
}


FTM_RET	FTOM_NODE_SNMPC_stop
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

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
