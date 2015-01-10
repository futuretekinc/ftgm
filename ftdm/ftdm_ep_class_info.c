#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_ep_class_info.h"
#include "ftdm_sqlite.h"
#include "simclist.h"

static FTM_RET	FTDM_EP_CLASS_INFO_LIST_append
(
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
);

static FTM_RET	FTDM_EP_CLASS_INFO_LIST_del
(
 	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
);

static FTM_RET	FTDM_EP_CLASS_INFO_LIST_isExist
(
 	FTM_EP_CLASS			xClass,
	FTM_BOOL_PTR			pExist
);

static FTM_RET	FTDM_EP_CLASS_INFO_LIST_get
(
 	FTM_EP_CLASS				xClass,
	FTM_EP_CLASS_INFO_PTR _PTR_	ppEPClassInfo
);

static FTM_INT	FTDM_EP_CLASS_INFO_seeker
(
	const void *pElement, 
	const void *pKey)
;

static list_t	xEPClassInfoList;

FTM_RET	FTDM_EP_CLASS_INFO_init
(
	FTDM_CFG_EP_PTR	pConfig
)
{
	FTM_ULONG	nMaxEPCount = 0;

	if (list_init(&xEPClassInfoList) < 0)
	{
		return	FTM_RET_INTERNAL_ERROR;	
	}

	list_attributes_seeker(&xEPClassInfoList, FTDM_EP_CLASS_INFO_seeker);

	if (FTDM_CFG_EP_CLASS_INFO_count(pConfig, &nMaxEPCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < nMaxEPCount ; i++)
		{
			FTM_EP_CLASS_INFO	xEPClassInfo;

			if (FTDM_CFG_EP_CLASS_INFO_getAt(pConfig, i, &xEPClassInfo) == FTM_RET_OK)
			{
				FTM_BOOL	bExist;

				FTDM_EP_CLASS_INFO_LIST_isExist(xEPClassInfo.xClass, &bExist);
				if (!bExist)
				{
					FTDM_EP_CLASS_INFO_add(&xEPClassInfo);	
				}
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET FTDM_EP_CLASS_INFO_final
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

FTM_RET	FTDM_EP_CLASS_INFO_add
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

	nRet = FTDM_EP_CLASS_INFO_LIST_get(pEPClassInfo->xClass, &pTempInfo);
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

	nRet = FTDM_EP_CLASS_INFO_LIST_append(pNewInfo);
	
	if (nRet != FTM_RET_OK)
	{
		free(pNewInfo);
		return	nRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_INFO_del
(
	FTM_EP_CLASS		xClass
)
{
	FTM_RET	nRet;
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = NULL;

	nRet = FTDM_EP_CLASS_INFO_LIST_get(xClass, &pEPClassInfo);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	FTDM_EP_CLASS_INFO_LIST_del(pEPClassInfo);
	free(pEPClassInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_INFO_count
(
	FTM_ULONG_PTR	pnCount
)
{
	*pnCount = list_size(&xEPClassInfoList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_INFO_get
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

	nRet = FTDM_EP_CLASS_INFO_LIST_get(xClass, &pItem);
	if (nRet == FTM_RET_OK)
	{
		memcpy(pEPClassInfo, pItem, sizeof(FTM_EP_CLASS_INFO));	
	}

	return	nRet;
}

FTM_RET	FTDM_EP_CLASS_INFO_getAt
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

FTM_INT	FTDM_EP_CLASS_INFO_seeker(const void *pElement, const void *pKey)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)pElement;
	FTM_EP_CLASS_PTR	pClass = (FTM_EP_CLASS_PTR)pKey;

	if (pEPClassInfo->xClass == *pClass)
	{
		return	1;	
	}

	return	0;
}

FTM_RET	FTDM_EP_CLASS_INFO_LIST_append
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

FTM_RET	FTDM_EP_CLASS_INFO_LIST_del
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

FTM_RET	FTDM_EP_CLASS_INFO_LIST_get
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

FTM_RET	FTDM_EP_CLASS_INFO_LIST_isExist
(
 	FTM_EP_CLASS			xClass,
	FTM_BOOL_PTR			pExist
)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo = NULL;

	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)list_seek(&xEPClassInfoList, &xClass);
	if (pEPClassInfo != NULL)
	{
		*pExist = FTM_BOOL_TRUE;

		return	FTM_RET_OK;
	}

	*pExist = FTM_BOOL_FALSE;

	return	FTM_RET_OBJECT_NOT_FOUND;
}
