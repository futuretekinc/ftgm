#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftm_list.h"
#include "ftnm_ep_class.h"

static FTM_LIST	xEPList;

FTM_INT	FTNM_EP_CLASS_INFO_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_INT	FTNM_EP_CLASS_INFO_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

FTM_RET	FTNM_EP_CLASS_INFO_init(FTM_VOID)
{
	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&xEPList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&xEPList, FTNM_EP_CLASS_INFO_seeker);
	FTM_LIST_setComparator(&xEPList, FTNM_EP_CLASS_INFO_comparator);

	return	nRet;
}

FTM_RET FTNM_EP_CLASS_INFO_final(FTM_VOID)
{
	FTM_EP_CLASS_INFO_PTR	pEP;

	while(FTM_LIST_getAt(&xEPList, 0, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTNM_EP_CLASS_INFO_destroy(pEP);
	}
	
	FTM_LIST_destroy(&xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_CLASS_INFO_create(FTM_EP_CLASS_INFO_PTR pInfo)
{
	FTM_EP_CLASS_INFO_PTR	pNewEP;

	ASSERT(pInfo != NULL);

	pNewEP = (FTM_EP_CLASS_INFO_PTR)FTM_MEM_calloc(1, sizeof(FTM_EP_CLASS_INFO));
	memcpy(pNewEP, pInfo, sizeof(FTM_EP_CLASS_INFO));

	FTM_LIST_append(&xEPList, pNewEP);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_CLASS_INFO_destroy(FTM_EP_CLASS_INFO_PTR	pEPClass)
{
	FTM_RET	nRet;
	
	ASSERT(pEPClass != NULL);
	nRet = FTM_LIST_remove(&xEPList, pEPClass);
	if (nRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEPClass);	
	}

	return	nRet;
}

FTM_RET	FTNM_EP_CLASS_INFO_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(&xEPList, pulCount);
}

FTM_RET FTNM_EP_CLASS_INFO_get(FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR _PTR_ ppEP)
{
	return	FTM_LIST_get(&xEPList, &xClass, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTNM_EP_CLASS_INFO_getAt(FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR _PTR_ ppEP)
{
	return	FTM_LIST_getAt(&xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_INT	FTNM_EP_CLASS_INFO_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)pElement;
	FTM_EP_CLASS_PTR		pEPClass=(FTM_EP_CLASS_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(pEPClassInfo->xClass == *pEPClass);
}

FTM_INT	FTNM_EP_CLASS_INFO_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	FTM_EP_CLASS_INFO_PTR		pEPClassInfo1 = (FTM_EP_CLASS_INFO_PTR)pElement1;
	FTM_EP_CLASS_INFO_PTR		pEPClassInfo2 = (FTM_EP_CLASS_INFO_PTR)pElement2;
	
	return	(pEPClassInfo1->xClass - pEPClassInfo2->xClass);
}
