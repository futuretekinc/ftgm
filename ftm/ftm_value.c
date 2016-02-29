#include <stdlib.h>
#include "ftm_value.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

FTM_RET	FTM_VALUE_create(FTM_VALUE_PTR _PTR_ ppValue, FTM_VALUE_TYPE xType)

{
	FTM_VALUE_PTR	pValue;

	ASSERT(ppValue != NULL);
	
	pValue = (FTM_VALUE_PTR)FTM_MEM_calloc(1, sizeof(FTM_VALUE));	
	switch(xType)
	{
	case	FTM_VALUE_TYPE_BYTE:	
		pValue->xType = xType;
		pValue->ulLen = 1;
		break;

	case	FTM_VALUE_TYPE_USHORT:	
		pValue->xType = xType;
		pValue->ulLen = 2;
		break;

	case	FTM_VALUE_TYPE_ULONG:	
		pValue->xType = xType;
		pValue->ulLen = 4;
		break;

	case	FTM_VALUE_TYPE_STRING:	
		pValue->xType = xType;
		pValue->ulLen = 0;
		break;

	default:
		FTM_MEM_free(pValue);
		return	FTM_RET_ERROR;
	}
	
	pValue->xMagic = FTM_VALUE_DYNAMIC_MAGIC;

	*ppValue = pValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_destroy(FTM_VALUE_PTR pValue)
{
	ASSERT(pValue != NULL);

	if (pValue->xMagic != FTM_VALUE_DYNAMIC_MAGIC)
	{
		ERROR("pValue is not dynamic object!\n");
		return	FTM_RET_ERROR;	
	}

	if (pValue->xType == FTM_VALUE_TYPE_STRING)
	{
		if (pValue->ulLen != 0)
		{
			FTM_MEM_free(pValue->xValue.pValue);
			pValue->xValue.pValue = NULL;
			pValue->ulLen = 0;
		}
	}

	FTM_MEM_free(pValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_init(FTM_VALUE_PTR pValue, FTM_VALUE_TYPE xType)
{
	ASSERT(pValue != NULL);

	switch(xType)
	{
	case	FTM_VALUE_TYPE_BYTE:	
		pValue->xType = xType;
		pValue->ulLen = 1;
		break;

	case	FTM_VALUE_TYPE_USHORT:	
		pValue->xType = xType;
		pValue->ulLen = 2;
		break;

	case	FTM_VALUE_TYPE_ULONG:	
		pValue->xType = xType;
		pValue->ulLen = 4;
		break;

	case	FTM_VALUE_TYPE_STRING:	
		pValue->xType = xType;
		pValue->ulLen = 0;
		break;
	
	default:
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_final(FTM_VALUE_PTR pValue)
{
	ASSERT(pValue != NULL);

	if (pValue->xType == FTM_VALUE_TYPE_STRING)
	{
		if (pValue->ulLen != 0)
		{
			FTM_MEM_free(pValue->xValue.pValue);
			pValue->xValue.pValue = NULL;
			pValue->ulLen = 0;
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_VALUE_isBYTE(FTM_VALUE_PTR pValue)
{
	return	(pValue->xType == FTM_VALUE_TYPE_BYTE);
}

FTM_BOOL	FTM_VALUE_isUsHORT(FTM_VALUE_PTR pValue)
{
	return	(pValue->xType == FTM_VALUE_TYPE_USHORT);
}

FTM_BOOL 	FTM_VALUE_isULONG(FTM_VALUE_PTR pValue)
{
	return	(pValue->xType == FTM_VALUE_TYPE_ULONG);
}

FTM_BOOL	FTM_VALUE_isSTRING(FTM_VALUE_PTR pValue)
{
	return	(pValue->xType == FTM_VALUE_TYPE_STRING);
}


FTM_RET	FTM_VALUE_setULONG(FTM_VALUE_PTR pValue, FTM_ULONG ulValue)
{
	ASSERT(pValue != NULL);

	pValue->xType = FTM_VALUE_TYPE_ULONG;
	pValue->ulLen = 4;
	pValue->xValue.ulValue = ulValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_getULONG(FTM_VALUE_PTR pValue, FTM_ULONG_PTR pulValue)
{
	ASSERT((pValue != NULL) && (pulValue != NULL));

	switch(pValue->xType)
	{
	case	FTM_VALUE_TYPE_BYTE:	
		*pulValue = (FTM_ULONG)pValue->xValue.bValue;
		break;

	case	FTM_VALUE_TYPE_USHORT:	
		*pulValue = (FTM_ULONG)pValue->xValue.usValue;
		break;

	case	FTM_VALUE_TYPE_ULONG:	
		*pulValue = pValue->xValue.ulValue;
		break;

	case	FTM_VALUE_TYPE_STRING:	
		if (pValue->ulLen != 0)
		{
			*pulValue = strtoul(pValue->xValue.pValue, 0, 10);
		}
		else
		{
			*pulValue = 0;
		}
		break;

	default:
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

