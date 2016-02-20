#include <stdlib.h>
#include "ftnm.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_dmc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

FTM_ULONG		active_hosts = 0;
FTM_RET			FTNM_NODE_SNMPC_start(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET			FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET			FTNM_NODE_SNMPC_updateEP(FTNM_NODE_SNMPC_PTR pNode, FTNM_EP_PTR pEP);

FTM_RET	FTNM_NODE_SNMPC_create(FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTNM_NODE_SNMPC_PTR	pNode;
	
	
	pNode = (FTNM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTNM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pNode, 0, sizeof(FTNM_NODE_SNMPC));

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE_INFO));
	FTM_LIST_init(&pNode->xCommon.xEPList);
	pthread_mutex_init(&pNode->xCommon.xMutexLock, NULL);
	sem_init(&pNode->xLock, 0, 1);
	
	pNode->xCommon.fStart	= (FTNM_NODE_START)FTNM_NODE_SNMPC_start;
	pNode->xCommon.fStop 	= (FTNM_NODE_STOP)FTNM_NODE_SNMPC_stop;
	pNode->xCommon.fUpdateEP= (FTNM_NODE_EP_UPDATE)FTNM_NODE_SNMPC_updateEP;
	*ppNode = (FTNM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_destroy(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_MEM_free(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_init(FTNM_NODE_SNMPC_PTR pNode)
{
	FTM_RET				nRet;
	FTM_ULONG			ulEPCount;

	ASSERT(pNode != NULL);

	nRet = FTNM_NODE_EP_count((FTNM_NODE_PTR)pNode, &ulEPCount);
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
			FTNM_EP_PTR				pEP;
			FTM_EP_CLASS_INFO_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTNM_NODE_EP_getAt((FTNM_NODE_PTR)pNode, i, (FTNM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			if (FTNM_EP_CLASS_INFO_get((pEP->xInfo.xEPID & FTM_EP_CLASS_MASK), &pEPClassInfo) != FTM_RET_OK)
			{
				TRACE("EP CLASS[%08lx] information not found\n", pEP->xInfo.xEPID);
				continue;
			}

			snprintf(pOIDName, sizeof(pOIDName) - 1, "%s::%s", 
				pNode->xCommon.xInfo.xOption.xSNMP.pMIB, 
				pEPClassInfo->xOIDs.pValue);
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
	pNode->xCommon.xState = FTNM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_final(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_start(FTNM_NODE_SNMPC_PTR pNode)
{
	return	FTM_RET_OK;
}


FTM_RET	FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	return	FTM_RET_OK;
}


FTM_RET	FTNM_NODE_SNMPC_updateEP(FTNM_NODE_SNMPC_PTR pNode, FTNM_EP_PTR pEP)
{
	return	FTNM_SNMPC_updateEP(pNode, pEP);
}
