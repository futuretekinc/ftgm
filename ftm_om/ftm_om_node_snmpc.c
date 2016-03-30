#include <stdlib.h>
#include "ftm_om.h"
#include "ftm_om_node_snmpc.h"
#include "ftm_om_dmc.h"
#include "ftm_om_ep.h"
#include "ftm_om_ep_class.h"

FTM_ULONG		active_hosts = 0;
FTM_RET			FTM_OM_NODE_SNMPC_start(FTM_OM_NODE_SNMPC_PTR pNode);
FTM_RET			FTM_OM_NODE_SNMPC_stop(FTM_OM_NODE_SNMPC_PTR pNode);
FTM_RET			FTM_OM_NODE_SNMPC_getEPData(FTM_OM_NODE_SNMPC_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
FTM_RET			FTM_OM_NODE_SNMPC_setEPData(FTM_OM_NODE_SNMPC_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData);

FTM_RET	FTM_OM_NODE_SNMPC_create(FTM_NODE_PTR pInfo, FTM_OM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_OM_NODE_SNMPC_PTR	pNode;
	
	
	pNode = (FTM_OM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTM_OM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pNode, 0, sizeof(FTM_OM_NODE_SNMPC));

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));
	FTM_LIST_init(&pNode->xCommon.xEPList);
	pthread_mutex_init(&pNode->xCommon.xMutexLock, NULL);
	sem_init(&pNode->xLock, 0, 1);
	
	pNode->xCommon.fStart	= (FTM_OM_NODE_START)FTM_OM_NODE_SNMPC_start;
	pNode->xCommon.fStop 	= (FTM_OM_NODE_STOP)FTM_OM_NODE_SNMPC_stop;
	pNode->xCommon.fGetEPData=(FTM_OM_NODE_GET_EP_DATA)FTM_OM_NODE_SNMPC_getEPData;
	pNode->xCommon.fSetEPData=(FTM_OM_NODE_SET_EP_DATA)FTM_OM_NODE_SNMPC_setEPData;
	*ppNode = (FTM_OM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_NODE_SNMPC_destroy(FTM_OM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_MEM_free(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_NODE_SNMPC_init(FTM_OM_NODE_SNMPC_PTR pNode)
{
	FTM_RET				nRet;
	FTM_ULONG			ulEPCount;

	ASSERT(pNode != NULL);

	nRet = FTM_OM_NODE_getEPCount((FTM_OM_NODE_PTR)pNode, &ulEPCount);
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
			FTM_OM_EP_PTR				pEP;
			FTM_EP_CLASS_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTM_OM_NODE_getEPAt((FTM_OM_NODE_PTR)pNode, i, (FTM_OM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			if (FTM_OM_EP_CLASS_get((pEP->xInfo.xEPID & FTM_EP_TYPE_MASK), &pEPClassInfo) != FTM_RET_OK)
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
	pNode->xCommon.xState = FTM_OM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_NODE_SNMPC_final(FTM_OM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_NODE_SNMPC_start(FTM_OM_NODE_SNMPC_PTR pNode)
{
	return	FTM_RET_OK;
}


FTM_RET	FTM_OM_NODE_SNMPC_stop(FTM_OM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	return	FTM_RET_OK;
}


FTM_RET	FTM_OM_NODE_SNMPC_getEPData(FTM_OM_NODE_SNMPC_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	return	FTM_OM_SNMPC_getEPData(pNode, pEP, pData);
}

FTM_RET	FTM_OM_NODE_SNMPC_setEPData(FTM_OM_NODE_SNMPC_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
{
	return	FTM_OM_SNMPC_setEPData(pNode, pEP, pData);
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

FTM_RET		FTM_OM_NODE_SNMPC_getOID(FTM_OM_NODE_SNMPC_PTR pNode, FTM_ULONG ulType, FTM_ULONG ulIndex, oid *pOID, size_t *pnOIDLen)
{
	ASSERT(pNode != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pnOIDLen != NULL);

	FTM_CHAR	pBuff[1024];

	sprintf(pBuff, "%s:%sValue.%lu", pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pOIDNamePrefix[ulType], ulIndex);

	return	FTM_OM_SNMPC_getOID(pBuff, pOID, pnOIDLen);
}
