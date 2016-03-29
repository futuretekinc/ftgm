#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_ep_class.h"

static FTM_LIST	xEPList;

FTM_INT	FTOM_EP_CLASS_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_INT	FTOM_EP_CLASS_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

FTM_RET	FTOM_EP_CLASS_init(FTM_VOID)
{
	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&xEPList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&xEPList, FTOM_EP_CLASS_seeker);
	FTM_LIST_setComparator(&xEPList, FTOM_EP_CLASS_comparator);

	return	nRet;
}

FTM_RET FTOM_EP_CLASS_final(FTM_VOID)
{
	FTM_EP_CLASS_PTR	pEP;

	while(FTM_LIST_getAt(&xEPList, 0, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EP_CLASS_destroy(pEP);
	}
	
	FTM_LIST_final(&xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_CLASS_create(FTM_EP_CLASS_PTR pInfo)
{
	FTM_EP_CLASS_PTR	pNewEP;

	ASSERT(pInfo != NULL);

	pNewEP = (FTM_EP_CLASS_PTR)FTM_MEM_malloc(sizeof(FTM_EP_CLASS));
	memcpy(pNewEP, pInfo, sizeof(FTM_EP_CLASS));

	FTM_LIST_append(&xEPList, pNewEP);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EP_CLASS_destroy(FTM_EP_CLASS_PTR	pEPClass)
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

FTM_RET	FTOM_EP_CLASS_count(FTM_ULONG_PTR pulCount)
{
	return	FTM_LIST_count(&xEPList, pulCount);
}

FTM_RET FTOM_EP_CLASS_get(FTM_EP_TYPE xType, FTM_EP_CLASS_PTR _PTR_ ppEP)
{
	return	FTM_LIST_get(&xEPList, &xType, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTOM_EP_CLASS_getAt(FTM_ULONG ulIndex, FTM_EP_CLASS_PTR _PTR_ ppEP)
{
	return	FTM_LIST_getAt(&xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_INT	FTOM_EP_CLASS_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTM_EP_CLASS_PTR	pEPClassInfo = (FTM_EP_CLASS_PTR)pElement;
	FTM_EP_TYPE_PTR		pEPClass=(FTM_EP_TYPE_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(pEPClassInfo->xType == *pEPClass);
}

FTM_INT	FTOM_EP_CLASS_comparator(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2)
{
	FTM_EP_CLASS_PTR		pEPClassInfo1 = (FTM_EP_CLASS_PTR)pElement1;
	FTM_EP_CLASS_PTR		pEPClassInfo2 = (FTM_EP_CLASS_PTR)pElement2;
	
	return	(pEPClassInfo1->xType - pEPClassInfo2->xType);
}
