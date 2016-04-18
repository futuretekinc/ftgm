#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
#include "ftom_dmc.h"

#define	FTOM_EP_DATA_COUNT	32

typedef	enum FTOM_EP_CMD_ENUM
{
	FTOM_EP_CMD_STOP,
} FTOM_EP_CMD, _PTR_ FTOM_EP_CMD_PTR;

typedef	struct	FTOM_EP_MSG_STRUCT
{
	FTOM_EP_CMD		xCmd;	
} FTOM_EP_MSG, _PTR_ FTOM_EP_MSG_PTR;

static FTM_INT	FTOM_EPM_seekEP
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_INT	FTOM_EPM_compareEP
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

static FTM_INT	FTOM_EPM_seekCLASS
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_INT	FTOM_EPM_compareCLASS
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

FTM_RET	FTOM_EPM_create
(
	FTOM_PTR 	pOM, 
	FTOM_EPM_PTR _PTR_ ppEPM
)
{
	ASSERT(pOM != NULL);
	ASSERT(ppEPM != NULL);

	FTM_RET			xRet;
	FTOM_EPM_PTR	pEPM;

	pEPM = (FTOM_EPM_PTR)FTM_MEM_malloc(sizeof(FTOM_EPM));
	if (pEPM == NULL)
	{
		ERROR("Not enough memory.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_EPM_init(pOM, pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EPM init failed[%08x].\n", xRet);
		FTM_MEM_free(pEPM);
		return	xRet;
	}

	*ppEPM = pEPM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_destroy
(
	FTOM_EPM_PTR _PTR_ ppEPM
)
{
	ASSERT(ppEPM != NULL);
	
	FTM_RET	xRet;

	if (*ppEPM == NULL)
	{
		ERROR("EPM has not been initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTOM_EPM_final(*ppEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EPM finalize was failed.\n");
	}

	FTM_MEM_free(*ppEPM);
	*ppEPM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_init
(
	FTOM_PTR 		pOM, 	
	FTOM_EPM_PTR 	pEPM
)
{
	ASSERT(pOM != NULL);
	ASSERT(pEPM != NULL);

	FTM_RET	xRet;

	if (pEPM->pEPList != NULL)
	{
		ERROR("It has already been initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pEPM->pEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create a list.\n");
		return	xRet;
	}

	xRet = FTM_LIST_create(&pEPM->pClassList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create a list.\n");
		return	xRet;
	}

	pEPM->pOM = pOM;

	FTM_LIST_setSeeker(pEPM->pEPList, FTOM_EPM_seekEP);
	FTM_LIST_setComparator(pEPM->pEPList, FTOM_EPM_compareEP);

	FTM_LIST_setSeeker(pEPM->pClassList, FTOM_EPM_seekCLASS);
	FTM_LIST_setComparator(pEPM->pClassList, FTOM_EPM_compareCLASS);

	TRACE("EP management initialized.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_final
(
	FTOM_EPM_PTR 	pEPM
)
{
	ASSERT(pEPM != NULL);

	if (pEPM->pEPList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTOM_EP_PTR	pEP;

	FTM_LIST_iteratorStart(pEPM->pEPList);
	while(FTM_LIST_iteratorNext(pEPM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EPM_destroyEP(pEPM, pEP);	
	}

	FTM_LIST_destroy(pEPM->pEPList);
	pEPM->pEPList = NULL;

	FTM_EP_CLASS_PTR	pClass;

	FTM_LIST_iteratorStart(pEPM->pClassList);
	while(FTM_LIST_iteratorNext(pEPM->pClassList, (FTM_VOID_PTR _PTR_)&pClass) == FTM_RET_OK)
	{
		FTOM_EPM_destroyClass(pEPM, pClass);
	}

	FTM_LIST_final(pEPM->pClassList);

	TRACE("EP management finished.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_createEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_EP_PTR 		pInfo, 
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppEP != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP = NULL;

	xRet = FTOM_EP_create(pInfo, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("The EP[%s] creation failed.\n", pInfo->pEPID);
		return	xRet;	
	}

	xRet = FTOM_EPM_attachEP(pEPM, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't attach the EP[%s] to the EPM.\n", pInfo->pEPID);
		return	xRet;	
	}

	*ppEP = pEP;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_destroyEP
(
	FTOM_EPM_PTR	pEPM, 
	FTOM_EP_PTR		pEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	FTOM_EP_stop(pEP, TRUE);

	xRet = FTOM_EPM_detachEP(pEPM, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't detach the EP[%s] from the EPM.\n", pEP->xInfo.pEPID);
		return	xRet;	
	}

	FTOM_EP_destroy(&pEP);

	return	xRet;
}

FTM_RET	FTOM_EPM_attachEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTOM_EP_PTR 	pEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_append(pEPM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't attach EP[%s] to EPM.\n", pEP->xInfo.pEPID);
		return	xRet;	
	}

	pEP->pEPM = pEPM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_detachEP
(
	FTOM_EPM_PTR 	pEPM, 
	FTOM_EP_PTR		pEP
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	if (pEP->pEPM != pEPM)
	{
		WARN("EP[%s] is not attached.\n", pEP->xInfo.pEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;
	}

	xRet = FTM_LIST_remove(pEPM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		WARN("EP[%s] is not attached.\n", pEP->xInfo.pEPID);
		return	FTM_RET_EP_IS_NOT_ATTACHED;
	}

	pEP->pEPM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_count
(
	FTOM_EPM_PTR	pEPM,
	FTM_EP_TYPE 	xType, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pulCount != NULL);

	if ((xType == 0XFFFFFFFF) || (xType == 0))
	{
		return	FTM_LIST_count(pEPM->pEPList, pulCount);
	}
	else
	{
		FTM_ULONG	i, ulTotalCount, ulCount = 0;

		FTM_LIST_count(pEPM->pEPList, &ulTotalCount);
		for(i = 0 ; i < ulTotalCount; i++)
		{
			FTOM_EP_PTR	pEP;

			FTM_LIST_getAt(pEPM->pEPList, i,(FTM_VOID_PTR _PTR_)&pEP);
			if (xType == (pEP->xInfo.xType & FTM_EP_TYPE_MASK))
			{
				ulCount++;
			}
		}


		*pulCount = ulCount;
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_EPM_getIDList
(
	FTOM_EPM_PTR	pEPM,
	FTM_EP_TYPE 	xType, 
	FTM_CHAR		pEPIDList[][FTM_EPID_LEN+1], 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEPIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_ULONG	i, ulTotalCount, ulCount = 0;
	
	FTM_LIST_count(pEPM->pEPList, &ulTotalCount);
	for(i = 0 ; i < ulTotalCount && ulCount < ulMaxCount; i++)
	{
		FTOM_EP_PTR	pEP;

		FTM_LIST_getAt(pEPM->pEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
		if ((xType == 0) || (xType == (pEP->xInfo.xType & FTM_EP_TYPE_MASK)))
		{
			strncpy(pEPIDList[ulCount++],pEP->xInfo.pEPID, FTM_EPID_LEN);
		}
	}

	
	*pulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_EPM_getEP
(
	FTOM_EPM_PTR		pEPM,
	FTM_CHAR_PTR		pEPID,
	FTOM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);
	
	return	FTM_LIST_get(pEPM->pEPList, pEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET FTOM_EPM_getEPAt
(
	FTOM_EPM_PTR		pEPM,
	FTM_ULONG 			ulIndex, 
	FTOM_EP_PTR _PTR_ 	ppEP
)
{
	ASSERT(pEPM != NULL);

	return	FTM_LIST_getAt(pEPM->pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_EPM_saveEPData
(
	FTOM_EPM_PTR	pEPM,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pData != NULL);

	return	FTOM_saveEPData(pEPM->pOM, pEPID, pData);
}

FTM_RET	FTOM_EPM_sendEPData
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData,
	FTM_ULONG		ulCount
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pData != NULL);

	return	FTOM_sendEPData(pEPM->pOM, pEPID, pData, ulCount);
}

FTM_RET	FTOM_EPM_createClass
(
	FTOM_EPM_PTR		pEPM,
	FTM_EP_CLASS_PTR 	pInfo
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pInfo != NULL);

	FTM_EP_CLASS_PTR        pEPClass;

	pEPClass = (FTM_EP_CLASS_PTR)FTM_MEM_malloc(sizeof(FTM_EP_CLASS));
	memcpy(pEPClass, pInfo, sizeof(FTM_EP_CLASS));

	FTM_LIST_append(pEPM->pClassList, pEPClass);

	
	return  FTM_RET_OK;
}

FTM_RET	FTOM_EPM_destroyClass
(
	FTOM_EPM_PTR		pEPM,
	FTM_EP_CLASS_PTR  	pEPClass
)
{
	ASSERT(pEPM != NULL);
	ASSERT(pEPClass != NULL);

	FTM_RET nRet;
       
	nRet = FTM_LIST_remove(pEPM->pClassList, pEPClass);
	if (nRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEPClass); 
	}

	return  nRet;
}

FTM_RET	FTOM_EPM_countClass
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTM_LIST_count(pEPM->pClassList, pulCount);
}

FTM_RET FTOM_EPM_getClass
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_EP_TYPE 	xType, 
	FTM_EP_CLASS_PTR _PTR_ ppEPClass
)
{
	return	FTM_LIST_get(pEPM->pClassList, &xType, (FTM_VOID_PTR _PTR_)ppEPClass);
}

FTM_RET FTOM_EPM_getClassAt
(
	FTOM_EPM_PTR 	pEPM, 
	FTM_ULONG 		ulIndex, 
	FTM_EP_CLASS_PTR _PTR_ ppEPClass
)
{
	return	FTM_LIST_getAt(pEPM->pClassList, ulIndex, (FTM_VOID_PTR _PTR_)ppEPClass);
}

FTM_INT	FTOM_EPM_seekEP
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_EP_PTR		pEP = (FTOM_EP_PTR)pElement;
	FTM_CHAR_PTR	pEPID=(FTM_CHAR_PTR)pIndicator;

	return	strcmp(pEP->xInfo.pEPID,pEPID) == 0;
}

FTM_INT	FTOM_EPM_compareEP
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	ASSERT(pElement1 != NULL);
	ASSERT(pElement2 != NULL);

	FTOM_EP_PTR		pEP1 = (FTOM_EP_PTR)pElement1;
	FTOM_EP_PTR		pEP2 = (FTOM_EP_PTR)pElement2;
	
	return	strcmp(pEP1->xInfo.pEPID, pEP2->xInfo.pEPID);
}

FTM_INT	FTOM_EPM_seekCLASS
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	FTM_EP_CLASS_PTR	pEPClassInfo = (FTM_EP_CLASS_PTR)pElement;
	FTM_EP_TYPE_PTR		pEPClass=(FTM_EP_TYPE_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(pEPClassInfo->xType == *pEPClass);
}

FTM_INT	FTOM_EPM_compareCLASS
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	FTM_EP_CLASS_PTR		pEPClassInfo1 = (FTM_EP_CLASS_PTR)pElement1;
	FTM_EP_CLASS_PTR		pEPClassInfo2 = (FTM_EP_CLASS_PTR)pElement2;
	
	return	(pEPClassInfo1->xType - pEPClassInfo2->xType);
}
