#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftm_list.h"
#include "ftm_mem.h"

typedef struct 
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pTypeString;
} FTM_EP_TYPE_STRING, _PTR_ FTM_EP_TYPE_STRING_PTR;

FTM_BOOL	_FTM_EPTypeSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_BOOL	_FTM_EPOIDInfoSeeker(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

static FTM_LIST_PTR _pEPTypeList = NULL;

FTM_RET	FTM_initEPTypeString(void)
{
	FTM_RET	xRet;
	
	xRet = FTM_LIST_create(&_pEPTypeList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(_pEPTypeList, _FTM_EPTypeSeeker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_finalEPTypeString(void)
{
	FTM_EP_TYPE_STRING_PTR	pItem;

	FTM_LIST_iteratorStart(_pEPTypeList);
	while(FTM_LIST_iteratorNext(_pEPTypeList, (FTM_VOID_PTR _PTR_)&pItem) == FTM_RET_OK)
	{
		FTM_MEM_free(pItem->pTypeString);	
		FTM_MEM_free(pItem);	
	}

	FTM_LIST_destroy(_pEPTypeList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_appendEPTypeString(FTM_EP_TYPE xType, FTM_CHAR_PTR pTypeString)
{
	ASSERT(pTypeString != NULL);

	FTM_RET					xRet;
	FTM_EP_TYPE_STRING_PTR	pItem;
	
	xRet = FTM_LIST_get(_pEPTypeList, &xType, (FTM_VOID_PTR _PTR_)&pItem);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	pItem = (FTM_EP_TYPE_STRING_PTR)FTM_MEM_calloc(1, sizeof(FTM_EP_TYPE_STRING));
	if (pItem == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pItem->xType = xType;
	pItem->pTypeString = strdup(pTypeString);

	if (FTM_LIST_append(_pEPTypeList, pItem) < 0)
	{
		FTM_MEM_free(pItem->pTypeString);
		FTM_MEM_free(pItem);

		return	FTM_RET_INTERNAL_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR FTM_getEPTypeString(FTM_EP_TYPE xType)
{
	FTM_RET	xRet;
	FTM_EP_TYPE_STRING_PTR pItem;

	xRet = FTM_LIST_get(_pEPTypeList, &xType, (FTM_VOID_PTR _PTR_)&pItem);
	if (xRet != FTM_RET_OK)
	{
		return	"UNKNOWN";
	}

	return	pItem->pTypeString;	
}

int	_FTM_EPTypeSeeker(const void *pElement, const void *pKey)
{
	FTM_EP_TYPE_STRING_PTR 	pTypeString = (FTM_EP_TYPE_STRING_PTR)pElement;
	FTM_EP_TYPE_PTR			pType = (FTM_EP_TYPE_PTR)pKey;

	if (pTypeString->xType == *pType)	
	{
		return	1;	
	}

	return	0;
}


FTM_RET	FTM_EP_DATA_createCopy(FTM_EP_DATA_PTR pSrcData, FTM_EP_DATA_PTR _PTR_ ppData)
{
	ASSERT(pSrcData != NULL);
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pData, pSrcData, sizeof(FTM_EP_DATA));

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createInt(FTM_INT nValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData)
{
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pData->xType = FTM_EP_DATA_TYPE_INT;
	pData->xState = xState;
	pData->ulTime = ulTime;
	pData->xValue.nValue = nValue;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createUlong(FTM_ULONG ulValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData)
{
	ASSERT(ppData != NULL);
	
	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pData->xType = FTM_EP_DATA_TYPE_ULONG;
	pData->xState = xState;
	pData->ulTime = ulTime;
	pData->xValue.ulValue = ulValue;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createFloat(FTM_DOUBLE fValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData)
{
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pData->xType = FTM_EP_DATA_TYPE_FLOAT;
	pData->xState = xState;
	pData->ulTime = ulTime;
	pData->xValue.fValue = fValue;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createBool(FTM_BOOL bValue, FTM_EP_DATA_STATE xState, FTM_ULONG ulTime, FTM_EP_DATA_PTR _PTR_ ppData)
{
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	pData->xType = FTM_EP_DATA_TYPE_BOOL;
	pData->xState = xState;
	pData->ulTime = ulTime;
	pData->xValue.bValue = bValue;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_destroy(FTM_EP_DATA_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_MEM_free(pData);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_compare(FTM_EP_DATA_PTR pData1, FTM_EP_DATA_PTR pData2, FTM_INT_PTR pResult)
{
	ASSERT(pData1 != NULL);
	ASSERT(pData2 != NULL);
	ASSERT(pResult != NULL);

	if (pData1->xType != pData2->xType)
	{
		FTM_FLOAT	fValue1;
		FTM_FLOAT	fValue2;


		switch(pData1->xType)
		{
		case	FTM_EP_DATA_TYPE_INT:
			{	
				fValue1 = pData1->xValue.nValue;
			}
			break;

		case	FTM_EP_DATA_TYPE_ULONG:
			{	
				fValue1 = pData1->xValue.ulValue;
			}
			break;

		case	FTM_EP_DATA_TYPE_FLOAT:
			{	
				fValue1 = pData1->xValue.fValue;
			}
			break;

		case	FTM_EP_DATA_TYPE_BOOL:
			{	
				fValue1 = pData1->xValue.bValue;
			}
			break;

		default:
			return	FTM_RET_ERROR;
		}

		switch(pData2->xType)
		{
		case	FTM_EP_DATA_TYPE_INT:
			{	
				fValue2 = pData2->xValue.nValue;
			}
			break;

		case	FTM_EP_DATA_TYPE_ULONG:
			{	
				fValue2 = pData2->xValue.ulValue;
			}
			break;

		case	FTM_EP_DATA_TYPE_FLOAT:
			{	
				fValue2 = pData2->xValue.fValue;
			}
			break;

		case	FTM_EP_DATA_TYPE_BOOL:
			{	
				fValue2 = pData2->xValue.bValue;
			}
			break;

		default:
			return	FTM_RET_ERROR;
		}

		if (fValue1 > fValue2)
		{
			*pResult = 1;
		}
		else if (fValue1 < fValue2)
		{
			*pResult = -1;	
		}
		else
		{
			*pResult = 0;	
		}

	}
	else
	{
		switch(pData1->xType)
		{
		case	FTM_EP_DATA_TYPE_INT:
			{	
				if (pData1->xValue.nValue > pData2->xValue.nValue)
				{
					*pResult = 1;
				}
				else if (pData1->xValue.nValue < pData2->xValue.nValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		case	FTM_EP_DATA_TYPE_ULONG:
			{	
				if (pData1->xValue.ulValue > pData2->xValue.ulValue)
				{
					*pResult = 1;
				}
				else if (pData1->xValue.ulValue < pData2->xValue.ulValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		case	FTM_EP_DATA_TYPE_FLOAT:
			{	
				if (pData1->xValue.fValue > pData2->xValue.fValue)
				{
					*pResult = 1;
				}
				else if (pData1->xValue.fValue < pData2->xValue.fValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		case	FTM_EP_DATA_TYPE_BOOL:
			{	
				if (pData1->xValue.fValue > pData2->xValue.fValue)
				{
					*pResult = 1;
				}
				else if (pData1->xValue.fValue < pData2->xValue.fValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		default:
			return	FTM_RET_INVALID_DATA;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_snprint(FTM_CHAR_PTR pBuff, FTM_ULONG ulMaxLen, FTM_EP_DATA_PTR pData)
{
	if ((pBuff == NULL) || (pData == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	switch(pData->xType)
	{
	case	FTM_EP_DATA_TYPE_INT:	snprintf(pBuff, ulMaxLen, "%d", pData->xValue.nValue); break;
	case	FTM_EP_DATA_TYPE_ULONG:	snprintf(pBuff, ulMaxLen, "%lu", pData->xValue.ulValue); break;
	case	FTM_EP_DATA_TYPE_FLOAT:	snprintf(pBuff, ulMaxLen, "%4.2lf", pData->xValue.fValue); break;
	}

	return	FTM_RET_OK;
}
