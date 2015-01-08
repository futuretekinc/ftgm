#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_ep_class_info.h"
#include "ftdm_sqlite.h"
#include "simclist.h"

static FTM_RET	FTDM_LIST_insertEPClassInfo
(
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
);

static FTM_RET	FTDM_LIST_delEPClassInfo
(
 	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
);

FTM_RET	FTDM_LIST_getEPClassInfo
(
 	FTM_EP_CLASS				xClass,
	FTM_EP_CLASS_INFO_PTR _PTR_	ppEPClassInfo
);

static FTM_INT	FTDM_EPSeeker
(
	const void *pElement, 
	const void *pKey)
;

static list_t	xEPClassInfoList;

FTM_RET	FTDM_initEPClassInfo
(
	FTM_VOID
)
{
	if (list_init(&xEPClassInfoList) < 0)
	{
		return	FTM_RET_INTERNAL_ERROR;	
	}

	list_attributes_seeker(&xEPClassInfoList, FTDM_EPSeeker);

	return	FTM_RET_OK;
}

FTM_RET FTDM_finalEPClassInfo
(
	FTM_VOID
)
{
	FTM_EP_CLASS_INFO_PTR pEPClassInfo;

	list_iterator_start(&xEPClassInfoList);
	while((pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)list_iterator_next(&xEPClassInfoList)) != NULL)
	{
		free(pEPClassInfo);	
	}	

	list_destroy(&xEPClassInfoList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_addEPClassInfo
(
	FTM_EP_CLASS_INFO_PTR 	pEPClassInfo
)
{
	FTM_EP_CLASS_INFO_PTR	pTempInfo;
	FTM_EP_CLASS_INFO_PTR	pNewInfo;
	FTM_RET	nRet;

	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_LIST_getEPClassInfo(pEPClassInfo->xClass, &pTempInfo);
	if ((nRet == FTM_RET_OK) && (pTempInfo != NULL))
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;	
	}

	pNewInfo = (FTM_EP_CLASS_INFO_PTR)malloc(sizeof(FTM_EP_CLASS_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pEPClassInfo, sizeof(FTM_EP_CLASS_INFO));

	nRet = FTDM_LIST_insertEPClassInfo(pNewInfo);
	
	if (nRet != FTM_RET_OK)
	{
		free(pNewInfo);
		return	nRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_delEPClassInfo
(
	FTM_EP_CLASS		xClass
)
{
	FTM_RET	nRet;
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = NULL;

	nRet = FTDM_LIST_getEPClassInfo(xClass, &pEPClassInfo);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	FTDM_LIST_delEPClassInfo(pEPClassInfo);
	free(pEPClassInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getEPClassInfoCount
(
	FTM_ULONG_PTR	pnCount
)
{
	*pnCount = list_size(&xEPClassInfoList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getEPClassInfo
(
	FTM_EP_CLASS			xClass,
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
)
{
	FTM_EP_CLASS_INFO_PTR 	pItem;
	FTM_RET	nRet;
	
	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_LIST_getEPClassInfo(xClass, &pItem);
	if (nRet == FTM_RET_OK)
	{
		memcpy(pEPClassInfo, pItem, sizeof(FTM_EP_CLASS_INFO));	
	}

	return	nRet;
}

FTM_RET	FTDM_getEPClassInfoByIndex
(
	FTM_ULONG				nIndex,
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
)
{
	FTM_EP_CLASS_INFO_PTR	pItem;

	
	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = (FTM_EP_CLASS_INFO_PTR)list_get_at(&xEPClassInfoList, nIndex);
	if (pItem != NULL)
	{
		memcpy(pEPClassInfo, pItem, sizeof(FTM_EP_CLASS_INFO));

		return	FTM_RET_OK;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_INT	FTDM_EPSeeker(const void *pElement, const void *pKey)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)pElement;
	FTM_EP_CLASS_PTR	pClass = (FTM_EP_CLASS_PTR)pKey;

	if (pEPClassInfo->xClass == *pClass)
	{
		return	1;	
	}

	return	0;
}

FTM_RET	FTDM_LIST_insertEPClassInfo
(
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
)
{
	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	list_append(&xEPClassInfoList, pEPClassInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_LIST_delEPClassInfo
(
 	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
)
{
	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (list_delete(&xEPClassInfoList, pEPClassInfo) == 0)
	{
		return	FTM_RET_OK;
	}
	else
	{
		return	FTM_RET_INTERNAL_ERROR;		
	}
}

FTM_RET	FTDM_LIST_getEPClassInfo
(
 	FTM_EP_CLASS				xClass,
	FTM_EP_CLASS_INFO_PTR _PTR_	ppEPClassInfo
)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = NULL;

	if (ppEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)list_seek(&xEPClassInfoList, &xClass);
	if (pEPClassInfo != NULL)
	{
		*ppEPClassInfo = pEPClassInfo;	

		return	FTM_RET_OK;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

