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

FTM_BOOL	FTM_EP_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator);
FTM_BOOL	_FTM_EPTypeSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
FTM_BOOL	_FTM_EPOIDInfoSeeker(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

static FTM_LIST_PTR	pEPList = NULL;

FTM_RET	FTM_EP_init(FTM_VOID)
{
	if (pEPList != NULL)
	{
		ERROR("EP list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	pEPList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pEPList == NULL)
	{
		ERROR("EP list is not allocated.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_LIST_init(pEPList);
	FTM_LIST_setSeeker(pEPList, FTM_EP_seeker);

	TRACE("EP list is initialized.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_final(FTM_VOID)
{
	FTM_EP_PTR	pEP;

	if (pEPList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

 	FTM_LIST_iteratorStart(pEPList);
  	while(FTM_LIST_iteratorNext(pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
   	{   
    	FTM_EP_destroy(pEP);
	}

	FTM_LIST_final(pEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_setDefault(FTM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	memset(pEP, 0, sizeof(FTM_EP));
	pEP->xEPID		=	0;
	pEP->xType		=	0;
	strcpy(pEP->pName, "Unknown");
	pEP->bEnable	=	FTM_FALSE;
	pEP->ulTimeout	=	30;
	pEP->ulInterval	=	10;
	pEP->ulInterval	=	60;

	return	0;
}


FTM_RET	FTM_EP_create(FTM_EP_PTR pSrc, FTM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pSrc != NULL);

	FTM_RET		xRet;
	FTM_EP_PTR	pEP;

	if (pEPList == NULL)
	{
		ERROR("EP list is not initialized.\n");
		FTM_EP_init();
	}

	pEP = (FTM_EP_PTR)FTM_MEM_malloc(sizeof(FTM_EP));
	if (pEP == NULL)
	{
		ERROR("Can't not allocation EP.\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(pEP, pSrc, sizeof(FTM_EP));

	xRet = FTM_LIST_append(pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pEP);
		return	xRet;
	}

	if (ppEP != NULL)
	{
		*ppEP = pEP;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_destroy(FTM_EP_PTR pEP)
{
	ASSERT(pEPList != NULL);
	ASSERT(pEP != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_remove(pEPList, pEP);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pEP);	
	}

	return	xRet;
}

FTM_RET	FTM_EP_append(FTM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	FTM_RET		xRet;

	if (pEPList == NULL)
	{
		ERROR("EP list is not initialized.\n");
		FTM_EP_init();
	}

	xRet = FTM_LIST_append(pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_remove(FTM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	return	FTM_LIST_remove(pEPList, pEP);
}

FTM_RET	FTM_EP_count(FTM_ULONG_PTR pulCount)
{
	ASSERT(pEPList != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pEPList, pulCount);
}

FTM_RET	FTM_EP_get(FTM_EP_ID xEPID, FTM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_get(pEPList, (FTM_VOID_PTR)&xEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTM_EP_getAt(FTM_ULONG ulIndex, FTM_EP_PTR _PTR_ ppEP)
{
	ASSERT(pEPList != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_getAt(pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_BOOL		FTM_EP_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_EP_PTR		pEP = (FTM_EP_PTR)pItem;
	FTM_EP_ID_PTR	pEPID = (FTM_EP_ID_PTR)pIndicator;

	return	(pEP->xEPID == *pEPID);
}

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


FTM_RET	FTM_EP_DATA_create(FTM_EP_DATA_PTR pSrcData, FTM_EP_DATA_PTR _PTR_ ppData)
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

FTM_CHAR_PTR	FTM_EP_typeString(FTM_EP_TYPE xType)
{
	switch(xType)
	{
	case	FTM_EP_TYPE_TEMPERATURE:return	"TEMPERATURE";	
	case	FTM_EP_TYPE_HUMIDITY:	return	"HUMIDITY";	
	case	FTM_EP_TYPE_VOLTAGE:	return	"VOLTAGE";	
	case	FTM_EP_TYPE_CURRENT:	return	"CURRENT";	
	case	FTM_EP_TYPE_DI:			return	"DIGITAL INPUT";	
	case	FTM_EP_TYPE_DO:			return	"DIGITAL OUTPUT";	
	case	FTM_EP_TYPE_GAS:		return	"GAS";	
	case	FTM_EP_TYPE_POWER:		return	"POWER";	
	case	FTM_EP_TYPE_AI:			return	"ANALOG INPUT";	
	case	FTM_EP_TYPE_MULTI:		return	"MULTI-FUNCTION";	
	}

	static FTM_CHAR	pBuff[16];
	sprintf(pBuff, "%08lx", xType);

	return	pBuff;
}
