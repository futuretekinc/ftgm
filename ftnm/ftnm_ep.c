#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftm_list.h"
#include "ftnm_ep.h"

static FTM_LIST	xEPList;

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_INT	FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

FTM_RET	FTNM_EP_init(FTM_VOID)
{
	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&xEPList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&xEPList, FTNM_EP_seeker);
	FTM_LIST_setComparator(&xEPList, FTNM_EP_comparator);

	return	nRet;
}

FTM_RET FTNM_EP_final(FTM_VOID)
{
	FTNM_EP_PTR	pEP;

	while(FTM_LIST_getAt(&xEPList, 0, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTNM_EP_destroy(pEP);
	}
	
	FTM_LIST_destroy(&xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_create(FTM_EP_INFO_PTR pInfo, FTNM_EP_PTR _PTR_ ppEP)
{
	FTNM_EP_PTR	pNewEP;

	ASSERT((pInfo != NULL) && (ppEP != NULL));

	pNewEP = (FTNM_EP_PTR)FTM_MEM_calloc(1, sizeof(FTNM_EP));
	memcpy(&pNewEP->xInfo, pInfo, sizeof(FTM_EP_INFO));

	FTM_LIST_append(&xEPList, pNewEP);

	*ppEP = pNewEP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_destroy(FTNM_EP_PTR	pEP)
{
	FTM_RET	nRet;
	
	ASSERT(pEP != NULL);
	nRet = FTM_LIST_remove(&xEPList, pEP);
	if (nRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEP);	
	}

	return	nRet;
}

FTM_RET	FTNM_EP_count
(
	FTM_EP_CLASS 	xClass, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pulCount != NULL);

	if ((xClass == 0XFFFFFFFF) || (xClass == 0))
{
		return	FTM_LIST_count(&xEPList, pulCount);
	}
	else
	{
		FTM_ULONG	i, ulTotalCount, ulCount = 0;

		FTM_LIST_count(&xEPList, &ulTotalCount);
		for(i = 0 ; i < ulTotalCount; i++)
		{
			FTNM_EP_PTR	pEP;

			FTM_LIST_getAt(&xEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
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
	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	ASSERT((pEPIDList != NULL) && (pulCount != NULL));

	FTM_LIST_count(&xEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTNM_EP_PTR	pEP;

		FTM_LIST_getAt(&xEPList, i,	(FTM_VOID_PTR _PTR_)&pEP);
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

	return	FTM_LIST_get(&xEPList, &xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTNM_EP_getAt(FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP)
{
	return	FTM_LIST_getAt(&xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTNM_EP_setNode(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode)
{
	ASSERT(pEP != NULL);

	pEP->pNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_DATA_info(FTM_EPID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount)
{
	return	FTNM_DMC_EP_DATA_info(xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTNM_EP_DATA_count(FTM_EPID xEPID, FTM_ULONG_PTR pulCount)
{
	return	FTNM_DMC_EP_DATA_count(xEPID, pulCount);
}

FTM_INT	FTNM_EP_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTNM_EP_PTR		pEP = (FTNM_EP_PTR)pElement;
	FTM_EPID_PTR	pEPID=(FTM_EPID_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(pEP->xInfo.xEPID == *pEPID);
}

FTM_INT	FTNM_EP_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	FTNM_EP_PTR		pEP1 = (FTNM_EP_PTR)pElement1;
	FTNM_EP_PTR		pEP2 = (FTNM_EP_PTR)pElement2;
	
	return	(pEP1->xInfo.xEPID - pEP2->xInfo.xEPID);
}
