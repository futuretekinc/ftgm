#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_ep_class.h"
#include "ftdm_sqlite.h"

static FTM_RET	FTDM_EP_CLASS_LIST_append
(
	FTM_EP_CLASS_PTR	pEPClassInfo
);

static FTM_RET	FTDM_EP_CLASS_LIST_del
(
 	FTM_EP_CLASS_PTR	pEPClassInfo
);

static FTM_RET	FTDM_EP_CLASS_LIST_isExist
(
 	FTM_EP_TYPE			xType,
	FTM_BOOL_PTR			pExist
);

static FTM_RET	FTDM_EP_CLASS_LIST_get
(
 	FTM_EP_TYPE				xType,
	FTM_EP_CLASS_PTR _PTR_	ppEPClassInfo
);

static FTM_INT	FTDM_EP_CLASS_seeker
(
	const void *pElement, 
	const void *pKey)
;

static FTM_LIST	xEPClassInfoList;

FTM_RET	FTDM_EP_CLASS_init
(
	FTM_VOID
)
{
	FTM_RET		xRet;

	xRet = FTM_LIST_init(&xEPClassInfoList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(&xEPClassInfoList, FTDM_EP_CLASS_seeker);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_loadConfig
(
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "ep", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xTypeItemList;

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "types", &xTypeItemList);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG		ulItemCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xTypeItemList, &ulItemCount);	
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG		i;
				FTM_CONFIG_ITEM	xTypeItem;

				for(i = 0 ; i < ulItemCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xTypeItemList, i, &xTypeItem);
					if (xRet == FTM_RET_OK)
					{
						FTM_BOOL		bExist;
						FTM_EP_CLASS	xEPClass;
						xRet = FTM_CONFIG_ITEM_getEPClass(&xTypeItem, &xEPClass);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
			
						FTDM_EP_CLASS_LIST_isExist(xEPClass.xType, &bExist);
						if (!bExist)
						{
							FTDM_EP_CLASS_add(&xEPClass);	
						}
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_EP_CLASS_final
(
	FTM_VOID
)
{
	FTM_EP_CLASS_PTR pEPClassInfo;
	
	FTM_LIST_iteratorStart(&xEPClassInfoList);
	while(FTM_LIST_iteratorNext(&xEPClassInfoList, (FTM_VOID_PTR _PTR_)&pEPClassInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pEPClassInfo);	
	}	

	FTM_LIST_final(&xEPClassInfoList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_add
(
	FTM_EP_CLASS_PTR 	pEPClassInfo
)
{
	FTM_EP_CLASS_PTR	pTempInfo;
	FTM_EP_CLASS_PTR	pNewInfo;
	FTM_RET	nRet;

	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet = FTDM_EP_CLASS_LIST_get(pEPClassInfo->xType, &pTempInfo);
	if ((nRet == FTM_RET_OK) && (pTempInfo != NULL))
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;	
	}

	pNewInfo = (FTM_EP_CLASS_PTR)FTM_MEM_malloc(sizeof(FTM_EP_CLASS));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pEPClassInfo, sizeof(FTM_EP_CLASS));

	nRet = FTDM_EP_CLASS_LIST_append(pNewInfo);
	
	if (nRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNewInfo);
		return	nRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_del
(
	FTM_EP_TYPE		xType
)
{
	FTM_RET	nRet;
	FTM_EP_CLASS_PTR	pEPClassInfo = NULL;

	nRet = FTDM_EP_CLASS_LIST_get(xType, &pEPClassInfo);
	if (nRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	FTDM_EP_CLASS_LIST_del(pEPClassInfo);
	FTM_MEM_free(pEPClassInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_count
(
	FTM_ULONG_PTR	pnCount
)
{
	return	FTM_LIST_count(&xEPClassInfoList, pnCount);
}

FTM_RET	FTDM_EP_CLASS_get
(
	FTM_EP_TYPE			xType,
	FTM_EP_CLASS_PTR	pEPClassInfo
)
{
	ASSERT (pEPClassInfo != NULL);

	FTM_EP_CLASS_PTR 	pItem;
	FTM_RET	nRet;

	nRet = FTDM_EP_CLASS_LIST_get(xType, &pItem);
	if (nRet == FTM_RET_OK)
	{
		memcpy(pEPClassInfo, pItem, sizeof(FTM_EP_CLASS));	
	}

	return	nRet;
}

FTM_RET	FTDM_EP_CLASS_getAt
(
	FTM_ULONG				nIndex,
	FTM_EP_CLASS_PTR	pEPClassInfo
)
{
	ASSERT (pEPClassInfo != NULL);

	return	FTM_LIST_getAt(&xEPClassInfoList, nIndex, (FTM_VOID_PTR _PTR_)&pEPClassInfo);
}

FTM_INT	FTDM_EP_CLASS_seeker(const void *pElement, const void *pKey)
{
	FTM_EP_CLASS_PTR	pEPClassInfo = (FTM_EP_CLASS_PTR)pElement;
	FTM_EP_TYPE_PTR	pClass = (FTM_EP_TYPE_PTR)pKey;

	if (pEPClassInfo->xType == *pClass)
	{
		return	1;	
	}

	return	0;
}

FTM_RET	FTDM_EP_CLASS_LIST_append
(
	FTM_EP_CLASS_PTR	pEPClassInfo
)
{
	ASSERT(pEPClassInfo != NULL);

	FTM_LIST_append(&xEPClassInfoList, pEPClassInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_EP_CLASS_LIST_del
(
 	FTM_EP_CLASS_PTR	pEPClassInfo
)
{
	ASSERT(pEPClassInfo != NULL);

	return	FTM_LIST_remove(&xEPClassInfoList, pEPClassInfo);
}

FTM_RET	FTDM_EP_CLASS_LIST_get
(
 	FTM_EP_TYPE				xType,
	FTM_EP_CLASS_PTR _PTR_	ppEPClassInfo
)
{
	ASSERT (ppEPClassInfo != NULL);

	return	FTM_LIST_get(&xEPClassInfoList, &xType, (FTM_VOID_PTR _PTR_)ppEPClassInfo);
}

FTM_RET	FTDM_EP_CLASS_LIST_isExist
(
 	FTM_EP_TYPE			xType,
	FTM_BOOL_PTR			pExist
)
{
	ASSERT(pExist != NULL);

	if(FTM_LIST_seek(&xEPClassInfoList, &xType) == FTM_RET_OK)
	{
		*pExist = FTM_TRUE;

		return	FTM_RET_OK;
	}

	*pExist = FTM_FALSE;

	return	FTM_RET_OBJECT_NOT_FOUND;
}
