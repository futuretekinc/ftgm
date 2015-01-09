#include <stdlib.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"
#include "ftnm_ep.h"

static FTM_LIST	xEPList;

FTM_INT	FTNM_EP_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

FTM_RET	FTNM_EP_init(FTM_VOID)
{
	FTM_RET	nRet;
	
	nRet = FTM_LIST_init(&xEPList);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	FTM_LIST_setSeeker(&xEPList, FTNM_EP_seek);

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

	pNewEP = (FTNM_EP_PTR)calloc(1, sizeof(FTNM_EP));
	memcpy(&pNewEP->xInfo, pInfo, sizeof(FTM_EP_INFO));

	*ppEP = pNewEP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_EP_destroy(FTNM_EP_PTR	pEP)
{
	FTM_RET	nRet;
	
	ASSERT(pEP != NULL);
	nRet = FTM_LIST_remove(&xEPList, (FTM_VOID_PTR)&pEP->xInfo.xEPID);
	if (nRet == FTM_RET_OK)
	{
		free(pEP);	
	}

	return	nRet;
}

FTM_RET	FTNM_EP_setNode(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode)
{
	ASSERT(pEP != NULL);

	pEP->pNode = pNode;

	return	FTM_RET_OK;
}

FTM_INT	FTNM_EP_seek(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTNM_EP_PTR		pEP = (FTNM_EP_PTR)pElement;
	FTM_EPID_PTR	pEPID=(FTM_EPID_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(pEP->xInfo.xEPID == *pEPID);
}
