#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftm_list.h"
#include "ftnm_ep.h"
#include "ftnm_dmc.h"

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_INT	FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

static FTM_LIST_PTR	pEPList = NULL;

FTM_RET	FTNM_EP_init(void)
{
	FTM_RET	xRet;

	pEPList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pEPList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_LIST_init(pEPList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(pEPList, FTNM_EP_seeker);
	FTM_LIST_setComparator(pEPList, FTNM_EP_comparator);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_final(void)
{
	ASSERT(pEPList != NULL);

	FTNM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPList);
	while(FTM_LIST_iteratorNext(pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTNM_EP_destroy(pEP);	
	}

	FTM_MEM_free(pEPList);
	pEPList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_create(FTM_EP_INFO_PTR pInfo, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(ppEP != NULL);

	FTNM_EP_PTR	pNewEP = NULL;

	pNewEP = (FTNM_EP_PTR)FTM_MEM_malloc(sizeof(FTNM_EP));
	if (pNewEP == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNewEP->xInfo, pInfo, sizeof(FTM_EP_INFO));

	FTM_LIST_append(pEPList, pNewEP);

	*ppEP = pNewEP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_destroy(FTNM_EP_PTR	pEP)
{
	ASSERT(pEP != NULL);
	ASSERT(pEPList != NULL);

	FTM_RET	xRet;
	
	xRet = FTM_LIST_remove(pEPList, pEP);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEP);	
	}

	return	xRet;
}

FTM_RET	FTNM_EP_count
(
	FTM_EP_CLASS 	xClass, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPList != NULL);
	ASSERT(pulCount != NULL);

	if ((xClass == 0XFFFFFFFF) || (xClass == 0))
{
		return	FTM_LIST_count(pEPList, pulCount);
	}
	else
	{
		FTM_ULONG	i, ulTotalCount, ulCount = 0;

		FTM_LIST_count(pEPList, &ulTotalCount);
		for(i = 0 ; i < ulTotalCount; i++)
		{
			FTNM_EP_PTR	pEP;

			FTM_LIST_getAt(pEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
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
	FTM_EP_CLASS 	xClass, 
	FTM_EPID_PTR 	pEPIDList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPList != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(pEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTNM_EP_PTR	pEP;

		FTM_LIST_getAt(pEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
		if ((xClass == 0) || (xClass == (pEP->xInfo.xEPID & FTM_EP_CLASS_MASK)))
		{
			pEPIDList[ulCount++] = pEP->xInfo.xEPID;
		}
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_get(FTM_EPID xEPID, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);

	return	FTM_LIST_get(pEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTNM_EP_getAt(FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);

	return	FTM_LIST_getAt(pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTNM_EP_attach(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode)
{
	ASSERT(pEP != NULL);

	pEP->pNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_detach(FTNM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	pEP->pNode = NULL;

	return	FTM_RET_OK;
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
