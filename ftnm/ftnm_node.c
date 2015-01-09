#include <stdlib.h>
#include <string.h>
#include "ftnm_node.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"

static FTM_LIST	xNodeList;

FTM_VOID_PTR 	FTNM_NODE_process(FTM_VOID_PTR pData);
FTM_INT			FTNM_NODE_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET FTNM_NODE_MNGR_init(FTM_VOID)
{
	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&xNodeList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&xNodeList, FTNM_NODE_seek);

	return	nRet;
}

FTM_RET FTNM_NODE_MNGR_final(FTM_VOID)
{
	FTNM_NODE_PTR	pNode;

	while(FTM_LIST_getAt(&xNodeList, 0, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		FTNM_NODE_destroy(pNode);
	}
	
	FTM_LIST_destroy(&xNodeList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_create(FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTM_RET			nRet;
	FTNM_NODE_PTR	pNewNode;

	ASSERT((pInfo != NULL) && (ppNode != NULL));

	switch(pInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			pNewNode = (FTNM_NODE_PTR)calloc(1, sizeof(FTNM_NODE_SNMP));
			if (pNewNode == NULL)
			{
				ERROR("Not enough memory!\n");
				return	FTM_RET_NOT_ENOUGH_MEMORY;
			}
		}
		break;

	default:
		{
			ERROR("pInfo->xType = %08lx", pInfo->xType);
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	memcpy(&pNewNode->xInfo, pInfo, sizeof(FTM_NODE_INFO));
	pNewNode->xState = FTNM_NODE_STATE_STOP;
	pthread_mutex_init(&pNewNode->xMutexLock, NULL);

	nRet = FTM_LIST_append(&xNodeList, pNewNode);
	if (nRet != FTM_RET_OK)
	{
		free(pNewNode);	
	}

	*ppNode = pNewNode;

	return	nRet;
}

FTM_RET	FTNM_NODE_destroy(FTNM_NODE_PTR	pNode)
{
	FTM_RET			nRet;

	nRet = FTM_LIST_remove(&xNodeList, (FTM_VOID_PTR _PTR_)pNode->xInfo.pDID);
	if (nRet == FTM_RET_OK)
	{
		free(pNode);
	}

	return	nRet;
}

FTM_RET FTNM_NODE_get(FTM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTM_RET			nRet;
	FTNM_NODE_PTR	pNode;
	
	nRet = FTM_LIST_get(&xNodeList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (nRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	nRet;
}

FTM_RET	FTNM_NODE_linkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP)
{
	FTNM_EP_PTR	pTempEP;

	ASSERT((pNOde != NULL) && (pEP != NULL));

	pthread_mutex_lock(&pNode->xMutexLock);

	if (FTM_LIST_get(&pNode->xEPList, &pEP->xInfo.xEPID, &pTempEP) == FTM_RET_OK)
	{
		pthread_mutex_unlock(&pNode->xMutexLock);
		
		return	FTM_RET_ALREADY_EXISTS;
	}

	FTM_LIST_append(&pNode->xEPList, pEP);
	FTNM_EP_setNode(pEP, pNode);

	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_unlinkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP)
{
	ASSERT((pNode != NULL) && (pEP != NULL));

	pthread_mutex_lock(&pNode->xMutexLock);

	FTNM_EP_setNode(pEP, NULL);
	FTM_LIST_remove(&pNode->xEPList, &pEP->xInfo.xEPID);
	
	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_start(FTNM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xState != FTNM_NODE_STATE_STOP)
	{
		return	FTM_RET_ALREADY_RUNNING;
	}

	pthread_create(&pNode->xPThread, NULL, FTNM_NODE_process, pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_stop(FTNM_NODE_PTR pNode)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_restart(FTNM_NODE_PTR pNode)
{
	return	FTM_RET_OK;
}

FTM_VOID_PTR FTNM_NODE_process(FTM_VOID_PTR pData)
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pData;

	while(1)
	{
	
	}
}

FTM_INT	FTNM_NODE_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(strcmp(pNode->xInfo.pDID, pDID) == 0);
}

