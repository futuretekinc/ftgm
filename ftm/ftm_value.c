#include <stdlib.h>
#include <string.h>
#include "ftm_value.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

FTM_RET	FTM_VALUE_create
(
	FTM_VALUE_PTR _PTR_ ppValue, 
	FTM_VALUE_TYPE xType
)

{
	FTM_RET			xRet;
	FTM_VALUE_PTR	pValue;

	ASSERT(ppValue != NULL);
	
	pValue = (FTM_VALUE_PTR)FTM_MEM_calloc(1, sizeof(FTM_VALUE));	
	if (pValue == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_VALUE_init(pValue, xType, NULL);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pValue);
		return	xRet;
	}

	*ppValue = pValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_destroy
(	
	FTM_VALUE_PTR pValue
)
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

FTM_RET	FTM_VALUE_init
(
	FTM_VALUE_PTR	pObject,
	FTM_VALUE_TYPE	xType,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pObject != NULL);

	switch(xType)
	{
	case	FTM_VALUE_TYPE_INT:
		{
			FTM_INT	nValue = 0;

			if (pValue != NULL)
			{
				nValue = strtol(pValue, 0, 10);
			}
			FTM_VALUE_initINT(pObject, nValue);
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:
		{
			FTM_ULONG	ulValue = 0;

			if (pValue != NULL)
			{
				ulValue = strtoul(pValue, 0, 10);
			}
			FTM_VALUE_initULONG(pObject, ulValue);
		}
		break;

	case	FTM_VALUE_TYPE_FLOAT:
		{
			FTM_FLOAT	fValue = 0;

			if (pValue != NULL)
			{
				fValue = atof(pValue);
			}
			FTM_VALUE_initFLOAT(pObject, fValue);
		}
		break;

	case	FTM_VALUE_TYPE_BOOL:
		{
			FTM_BOOL	bValue = FTM_FALSE;

			if (pValue != NULL)
			{
				if (strcasecmp(pValue, "true") == 0)
				{
					bValue = FTM_TRUE;	
				}
				else if (strcasecmp(pValue, "false") == 0)
				{
					bValue = FTM_FALSE;	
				}
				else
				{
					FTM_INT	nValue;
					nValue = strtol(pValue, 0, 10);
					if ((nValue != 0) && (nValue != 1))
					{
						return	FTM_RET_INVALID_DATA;	
					}
	
					bValue = nValue;
				}
			}

			FTM_VALUE_initBOOL(pObject, bValue);
		}
		break;

	default:
		return	FTM_RET_INVALID_TYPE;
	}

	return	FTM_RET_OK;
}	

FTM_RET	FTM_VALUE_initINT
(
	FTM_VALUE_PTR	pObject,
	FTM_INT			nValue
)
{
	ASSERT(pObject != NULL);
	
	pObject->xType = FTM_VALUE_TYPE_INT;
	pObject->xValue.nValue = nValue;
	pObject->ulLen = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_initULONG
(
	FTM_VALUE_PTR	pObject,
	FTM_ULONG		ulValue
)
{
	ASSERT(pObject != NULL);
	
	pObject->xType = FTM_VALUE_TYPE_ULONG;
	pObject->xValue.ulValue = ulValue;
	pObject->ulLen = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_initFLOAT
(
	FTM_VALUE_PTR	pObject,
	FTM_FLOAT		fValue
)
{
	ASSERT(pObject != NULL);
	
	pObject->xType = FTM_VALUE_TYPE_FLOAT;
	pObject->xValue.fValue = fValue;
	pObject->ulLen = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_initBOOL
(
	FTM_VALUE_PTR	pObject,
	FTM_BOOL		bValue
)
{
	ASSERT(pObject != NULL);
	
	pObject->xType = FTM_VALUE_TYPE_BOOL;
	pObject->xValue.bValue = bValue;
	pObject->ulLen = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_final
(
	FTM_VALUE_PTR pValue
)
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

FTM_BOOL	FTM_VALUE_isUSHORT
(
	FTM_VALUE_PTR pObject
)
{
	return	(pObject->xType == FTM_VALUE_TYPE_USHORT);
}

FTM_BOOL 	FTM_VALUE_isULONG
(
	FTM_VALUE_PTR pObject
)
{
	return	(pObject->xType == FTM_VALUE_TYPE_ULONG);
}

FTM_BOOL	FTM_VALUE_isSTRING
(
	FTM_VALUE_PTR pObject
)
{
	return	(pObject->xType == FTM_VALUE_TYPE_STRING);
}


FTM_RET	FTM_VALUE_setULONG
(
	FTM_VALUE_PTR 	pObject, 
	FTM_ULONG 		ulValue
)
{
	ASSERT(pObject != NULL);

	pObject->xType = FTM_VALUE_TYPE_ULONG;
	pObject->ulLen = 4;
	pObject->xValue.ulValue = ulValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_getULONG
(
	FTM_VALUE_PTR 	pObject, 
	FTM_ULONG_PTR 	pulValue
)
{
	ASSERT((pObject != NULL) && (pulValue != NULL));

	switch(pObject->xType)
	{
	case	FTM_VALUE_TYPE_USHORT:	
		*pulValue = (FTM_ULONG)pObject->xValue.usValue;
		break;

	case	FTM_VALUE_TYPE_ULONG:	
		*pulValue = pObject->xValue.ulValue;
		break;

	case	FTM_VALUE_TYPE_STRING:	
		if (pObject->ulLen != 0)
		{
			*pulValue = strtoul(pObject->xValue.pValue, 0, 10);
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

FTM_RET	FTM_VALUE_compare
(
	FTM_VALUE_PTR pValue1, 
	FTM_VALUE_PTR pValue2, 
	FTM_INT_PTR pResult
)
{
	ASSERT(pValue1 != NULL);
	ASSERT(pValue2 != NULL);
	ASSERT(pResult != NULL);

	if (pValue1->xType != pValue2->xType)
	{
		FTM_FLOAT	fValue1;
		FTM_FLOAT	fValue2;


		switch(pValue1->xType)
		{
		case	FTM_VALUE_TYPE_INT:
			{	
				fValue1 = pValue1->xValue.nValue;
			}
			break;

		case	FTM_VALUE_TYPE_ULONG:
			{	
				fValue1 = pValue1->xValue.ulValue;
			}
			break;

		case	FTM_VALUE_TYPE_FLOAT:
			{	
				fValue1 = pValue1->xValue.fValue;
			}
			break;

		case	FTM_VALUE_TYPE_BOOL:
			{	
				fValue1 = pValue1->xValue.bValue;
			}
			break;

		default:
			return	FTM_RET_ERROR;
		}

		switch(pValue2->xType)
		{
		case	FTM_VALUE_TYPE_INT:
			{	
				fValue2 = pValue2->xValue.nValue;
			}
			break;

		case	FTM_VALUE_TYPE_ULONG:
			{	
				fValue2 = pValue2->xValue.ulValue;
			}
			break;

		case	FTM_VALUE_TYPE_FLOAT:
			{	
				fValue2 = pValue2->xValue.fValue;
			}
			break;

		case	FTM_VALUE_TYPE_BOOL:
			{	
				fValue2 = pValue2->xValue.bValue;
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
		switch(pValue1->xType)
		{
		case	FTM_VALUE_TYPE_INT:
			{	
				if (pValue1->xValue.nValue > pValue2->xValue.nValue)
				{
					*pResult = 1;
				}
				else if (pValue1->xValue.nValue < pValue2->xValue.nValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		case	FTM_VALUE_TYPE_ULONG:
			{	
				if (pValue1->xValue.ulValue > pValue2->xValue.ulValue)
				{
					*pResult = 1;
				}
				else if (pValue1->xValue.ulValue < pValue2->xValue.ulValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		case	FTM_VALUE_TYPE_FLOAT:
			{	
				if (pValue1->xValue.fValue > pValue2->xValue.fValue)
				{
					*pResult = 1;
				}
				else if (pValue1->xValue.fValue < pValue2->xValue.fValue)
				{
					*pResult = -1;	
				}
				else
				{
					*pResult = 0;	
				}
			}
			break;

		case	FTM_VALUE_TYPE_BOOL:
			{	
				if (pValue1->xValue.fValue > pValue2->xValue.fValue)
				{
					*pResult = 1;
				}
				else if (pValue1->xValue.fValue < pValue2->xValue.fValue)
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

FTM_RET	FTM_VALUE_snprint
(
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulMaxLen, 
	FTM_VALUE_PTR	pObject
)
{
	ASSERT(pBuff != NULL);
	ASSERT(pObject != NULL);

	switch(pObject->xType)
	{
	case	FTM_VALUE_TYPE_INT:		
		{
			snprintf(pBuff, ulMaxLen, "%d", pObject->xValue.nValue); 
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:	
		{
			snprintf(pBuff, ulMaxLen, "%lu", pObject->xValue.ulValue); 
		}
		break;

	case	FTM_VALUE_TYPE_FLOAT:	
		{
			snprintf(pBuff, ulMaxLen, "%4.2lf", pObject->xValue.fValue); 
		}
		break;

	case	FTM_VALUE_TYPE_BOOL:	
		{
			if (pObject->xValue.bValue)
			{
				snprintf(pBuff, ulMaxLen, "true");
			}
			else
			{
				snprintf(pBuff, ulMaxLen, "false");
			}
		}
		break;

	default:	
		pBuff[0] = '\0'; break;
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_VALUE_print
(	
	FTM_VALUE_PTR 	pObject
)
{
	static FTM_CHAR	pBuff[64];

	if(pObject->xType == FTM_VALUE_TYPE_STRING)
	{
		return	pObject->xValue.pValue;	
	}

	FTM_VALUE_snprint(pBuff, sizeof(pBuff), pObject);

	return	pBuff;
}
