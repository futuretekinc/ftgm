#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftm_list.h"
#include "ftnm_ep.h"
#include "ftnm_dmc.h"

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_INT	FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

FTM_RET	FTNM_EP_init(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&pCTX->xEPList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&pCTX->xEPList, FTNM_EP_seeker);
	FTM_LIST_setComparator(&pCTX->xEPList, FTNM_EP_comparator);

	return	nRet;
}

FTM_RET FTNM_EP_final(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_EP_PTR	pEP;

	while(FTM_LIST_getAt(&pCTX->xEPList, 0, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTNM_EP_destroy(pCTX, pEP);
	}
	
	FTM_LIST_destroy(&pCTX->xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_create(FTNM_CONTEXT_PTR pCTX, FTM_EP_INFO_PTR pInfo, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pCTX != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(ppEP != NULL);

	FTNM_EP_PTR	pNewEP;

	pNewEP = (FTNM_EP_PTR)FTM_MEM_calloc(1, sizeof(FTNM_EP));
	if (pNewEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNewEP->xInfo, pInfo, sizeof(FTM_EP_INFO));

	FTM_LIST_append(&pCTX->xEPList, pNewEP);

	*ppEP = pNewEP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_destroy(FTNM_CONTEXT_PTR pCTX, FTNM_EP_PTR	pEP)
{
	ASSERT(pCTX != NULL);

	FTM_RET	nRet;
	
	ASSERT(pEP != NULL);
	nRet = FTM_LIST_remove(&pCTX->xEPList, pEP);
	if (nRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEP);	
	}

	return	nRet;
}

FTM_RET	FTNM_EP_count
(
	FTNM_CONTEXT_PTR pCTX, 
	FTM_EP_CLASS 	xClass, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pCTX != NULL);
	ASSERT(pulCount != NULL);

	if ((xClass == 0XFFFFFFFF) || (xClass == 0))
{
		return	FTM_LIST_count(&pCTX->xEPList, pulCount);
	}
	else
	{
		FTM_ULONG	i, ulTotalCount, ulCount = 0;

		FTM_LIST_count(&pCTX->xEPList, &ulTotalCount);
		for(i = 0 ; i < ulTotalCount; i++)
		{
			FTNM_EP_PTR	pEP;

			FTM_LIST_getAt(&pCTX->xEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
			if (xClass == (pEP->xInfo.xEPID & FTM_EP_CLASS_MASK))
			{
				ulCount++;
			}
		}


		*pulCount = ulCount;
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_EP_getList
(
	FTNM_CONTEXT_PTR pCTX, 
	FTM_EP_CLASS 	xClass, 
	FTM_EPID_PTR 	pEPIDList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pCTX != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(&pCTX->xEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTNM_EP_PTR	pEP;

		FTM_LIST_getAt(&pCTX->xEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
		if ((xClass == 0) || (xClass == (pEP->xInfo.xEPID & FTM_EP_CLASS_MASK)))
		{
			pEPIDList[ulCount++] = pEP->xInfo.xEPID;
		}
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_get(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pCTX != NULL);

	return	FTM_LIST_get(&pCTX->xEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTNM_EP_getAt(FTNM_CONTEXT_PTR pCTX, FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pCTX != NULL);
	return	FTM_LIST_getAt(&pCTX->xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTNM_EP_attach(FTNM_CONTEXT_PTR pCTX, FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode)
{
	ASSERT(pCTX != NULL);
	ASSERT(pEP != NULL);

	pEP->pNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_detach(FTNM_CONTEXT_PTR pCTX, FTNM_EP_PTR pEP)
{
	ASSERT(pCTX != NULL);
	ASSERT(pEP != NULL);

	pEP->pNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_DATA_info(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount)
{
	ASSERT(pCTX != NULL);
	return	FTNM_DMC_EP_DATA_info(&pCTX->xDMC, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTNM_EP_DATA_count(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTM_ULONG_PTR pulCount)
{
	ASSERT(pCTX != NULL);
	return	FTNM_DMC_EP_DATA_count(&pCTX->xDMC, xEPID, pulCount);
}

FTM_RET	FTNM_EP_DATA_set(FTNM_CONTEXT_PTR pCTX, FTM_EPID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pCTX != NULL);

	return	FTNM_DMC_EP_DATA_set(&pCTX->xDMC, xEPID, pData);
}

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTNM_EP_PTR		pEP = (FTNM_EP_PTR)pElement;
	FTM_EPID_PTR	pEPID=(FTM_EPID_PTR)pIndicator;

	return	(pEP->xInfo.xEPID == *pEPID);
}

FTM_INT	FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTNM_EP_PTR		pEP1 = (FTNM_EP_PTR)pElement1;
	FTNM_EP_PTR		pEP2 = (FTNM_EP_PTR)pElement2;
	
	return	(pEP1->xInfo.xEPID - pEP2->xInfo.xEPID);
}
