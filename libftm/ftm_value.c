#include <ctype.h>
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

	xRet = FTM_VALUE_init(pValue, xType);
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

	FTM_VALUE_final(pValue);

	FTM_MEM_free(pValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_init
(
	FTM_VALUE_PTR	pObject,
	FTM_VALUE_TYPE	xType
)
{
	ASSERT(pObject != NULL);

	FTM_RET	xRet;

	switch(xType)
	{
	case	FTM_VALUE_TYPE_INT:		xRet = FTM_VALUE_initINT(pObject, 0); 			break;
	case	FTM_VALUE_TYPE_ULONG:	xRet = FTM_VALUE_initULONG(pObject, 0); 		break;
	case	FTM_VALUE_TYPE_FLOAT:	xRet = FTM_VALUE_initFLOAT(pObject, 0); 		break;
	case	FTM_VALUE_TYPE_BOOL:	xRet = FTM_VALUE_initBOOL(pObject, FTM_FALSE); 	break;
	case	FTM_VALUE_TYPE_STRING:	xRet = FTM_VALUE_initSTRING(pObject, "");		break;
	default:
		{
			xRet = FTM_RET_INVALID_TYPE;
		}
	}

	return	xRet;
}
	

FTM_RET	FTM_VALUE_final
(
	FTM_VALUE_PTR	pObject
)
{
	ASSERT(pObject != NULL);

	if (pObject->xType == FTM_VALUE_TYPE_STRING)
	{
		if (pObject->xValue.pValue != NULL)
		{
			FTM_MEM_free(pObject->xValue.pValue);
			pObject->xValue.pValue = NULL;
			pObject->ulLen = 0;
		}
	}

	pObject->xType = FTM_VALUE_TYPE_UNKNOWN;

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

FTM_RET	FTM_VALUE_initSTRING
(
	FTM_VALUE_PTR	pObject,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pObject != NULL);
	ASSERT(pValue != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_UNKNOWN)
	{
		FTM_VALUE_final(pObject);
	}

	pObject->xType = FTM_VALUE_TYPE_STRING;
	pObject->ulLen = strlen(pValue) + 1;
	pObject->xValue.pValue = FTM_MEM_malloc(pObject->ulLen);
	if (pObject->xValue.pValue == NULL)
	{
		pObject->ulLen = 0;
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pObject->xValue.pValue, pValue);

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


FTM_RET	FTM_VALUE_setINT
(
	FTM_VALUE_PTR 	pObject, 
	FTM_INT			nValue
)
{
	ASSERT(pObject != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_INT)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	pObject->xValue.nValue = nValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_setULONG
(
	FTM_VALUE_PTR 	pObject, 
	FTM_ULONG 		ulValue
)
{
	ASSERT(pObject != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_ULONG)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	pObject->xValue.ulValue = ulValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_setFLOAT
(
	FTM_VALUE_PTR 	pObject, 
	FTM_FLOAT		fValue
)
{
	ASSERT(pObject != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_FLOAT)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	pObject->xValue.fValue = fValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_setBOOL
(
	FTM_VALUE_PTR 	pObject, 
	FTM_BOOL		bValue
)
{
	ASSERT(pObject != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_BOOL)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	pObject->xValue.bValue = bValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_getBOOL
(
	FTM_VALUE_PTR 	pObject, 
	FTM_BOOL_PTR	pbValue
)
{
	ASSERT(pObject != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_BOOL)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	*pbValue = pObject->xValue.bValue;

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

FTM_RET	FTM_VALUE_setSTRING
(
	FTM_VALUE_PTR	pObject,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pObject != NULL);
	ASSERT(pValue != NULL);

	if (pObject->xType != FTM_VALUE_TYPE_STRING)
	{
		return	FTM_RET_INVALID_TYPE;
	}

	if (pObject->xValue.pValue != NULL)
	{
		FTM_MEM_free(pObject->xValue.pValue);
		pObject->xValue.pValue = NULL;
		pObject->ulLen = 0;
	}

	pObject->ulLen = strlen(pValue) + 1;
	pObject->xValue.pValue = FTM_MEM_malloc(pObject->ulLen);
	if (pObject->xValue.pValue == NULL)
	{
		pObject->ulLen = 0;
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pObject->xValue.pValue, pValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_getSTRING
(
	FTM_VALUE_PTR	pObject,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pObject != NULL);
	ASSERT(pBuff != NULL);

	FTM_CHAR_PTR	pPrintOut;

	pPrintOut = FTM_VALUE_print(pObject);
	if (strlen(pPrintOut) + 1 > ulBuffLen)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strcpy(pBuff, pPrintOut);

	return	FTM_RET_OK;
}

FTM_RET	FTM_VALUE_setFromString
(
	FTM_VALUE_PTR	pObject,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pObject != NULL);
	ASSERT(pValue != NULL);

	switch(pObject->xType)
	{
	case	FTM_VALUE_TYPE_INT:
		{
			FTM_INT	ulLen = strlen(pValue);
			FTM_INT	i;
			FTM_INT	nValue = 0;

			if ((ulLen > 1) && (pValue[0] == '0') && (toupper(pValue[1]) == 'X'))
			{
				nValue = strtol(pValue, 0, 16);
			}
			else if ((ulLen > 1) && (pValue[0] == '0'))
			{
				nValue = strtol(pValue, 0, 8);
			}
			else if (pValue[0] == '-')
			{
				for(i = 1 ; i < ulLen ; i++)
				{
					if (isdigit(pValue[i]) == 0)
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}
				}

				nValue = strtol(pValue, 0, 10);
			}
			else 
			{
				for(i = 0 ; i < ulLen ; i++)
				{
					if (isdigit(pValue[i]) == 0)
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}
				}

				nValue = strtol(pValue, 0, 10);
			}

			FTM_VALUE_initINT(pObject, nValue);
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:
		{

			FTM_INT	ulLen = strlen(pValue);
			FTM_INT	i;
			FTM_ULONG	ulValue = 0;

			if ((ulLen > 1) && (pValue[0] == '0') && (toupper(pValue[1]) == 'X'))
			{
				ulValue = strtoul(pValue, 0, 16);
			}
			else if ((ulLen > 1) && (pValue[0] == '0'))
			{
				ulValue = strtoul(pValue, 0, 8);
			}
			else 
			{
				for(i = 0 ; i < ulLen ; i++)
				{
					if (isdigit(pValue[i]) == 0)
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}
				}

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

	case	FTM_VALUE_TYPE_STRING:
		{
			FTM_VALUE_initSTRING(pObject, pValue);
		}
		break;

	default:
		return	FTM_RET_INVALID_TYPE;
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

FTM_RET	FTM_VALUE_setVALUE
(
	FTM_VALUE_PTR	pValue1,
	FTM_VALUE_PTR	pValue2
)
{
	ASSERT(pValue1 != NULL);
	ASSERT(pValue2 != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	switch(pValue1->xType)
	{
	case	FTM_VALUE_TYPE_INT:
		{
			switch(pValue2->xType)
			{
			case	FTM_VALUE_TYPE_INT:
				{
					pValue1->xValue.nValue = pValue2->xValue.nValue;
				}
				break;

			case	FTM_VALUE_TYPE_ULONG:
				{
					pValue1->xValue.nValue = (FTM_INT)pValue2->xValue.ulValue;
				}
				break;

			case	FTM_VALUE_TYPE_FLOAT:
				{
					pValue1->xValue.nValue = (FTM_INT)pValue2->xValue.fValue;
				}
				break;

			case	FTM_VALUE_TYPE_BOOL:
				{
					pValue1->xValue.nValue = (FTM_INT)pValue2->xValue.bValue;
				}
				break;

			case	FTM_VALUE_TYPE_STRING:
				{
					pValue1->xValue.nValue = strtol(pValue2->xValue.pValue, NULL, 10);
				}
				break;

			default:
				return	FTM_RET_INVALID_TYPE;
			}
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:
		{
			switch(pValue2->xType)
			{
			case	FTM_VALUE_TYPE_INT:
				{
					pValue1->xValue.ulValue = (FTM_ULONG)pValue2->xValue.nValue;
				}
				break;

			case	FTM_VALUE_TYPE_ULONG:
				{
					pValue1->xValue.ulValue = pValue2->xValue.ulValue;
				}
				break;

			case	FTM_VALUE_TYPE_FLOAT:
				{
					pValue1->xValue.ulValue = (FTM_ULONG)pValue2->xValue.fValue;
				}
				break;

			case	FTM_VALUE_TYPE_BOOL:
				{
					pValue1->xValue.ulValue = (FTM_ULONG)pValue2->xValue.bValue;
				}
				break;

			case	FTM_VALUE_TYPE_STRING:
				{
					pValue1->xValue.ulValue = strtoul(pValue2->xValue.pValue, NULL, 10);
				}
				break;

			default:
				return	FTM_RET_INVALID_TYPE;
			}
		}
		break;

	case	FTM_VALUE_TYPE_FLOAT:
		{
			switch(pValue2->xType)
			{
			case	FTM_VALUE_TYPE_INT:
				{
					pValue1->xValue.fValue = (FTM_FLOAT)pValue2->xValue.nValue;
				}
				break;

			case	FTM_VALUE_TYPE_ULONG:
				{
					pValue1->xValue.fValue = (FTM_FLOAT)pValue2->xValue.ulValue;
				}
				break;

			case	FTM_VALUE_TYPE_FLOAT:
				{
					pValue1->xValue.fValue = pValue2->xValue.fValue;
				}
				break;

			case	FTM_VALUE_TYPE_BOOL:
				{
					pValue1->xValue.fValue = (FTM_FLOAT)pValue2->xValue.bValue;
				}
				break;

			case	FTM_VALUE_TYPE_STRING:
				{
					pValue1->xValue.fValue = strtod(pValue2->xValue.pValue, NULL);
				}
				break;

			default:
				return	FTM_RET_INVALID_TYPE;
			}
		}
		break;

	case	FTM_VALUE_TYPE_BOOL:
		{
			switch(pValue2->xType)
			{
			case	FTM_VALUE_TYPE_INT:
				{
					pValue1->xValue.bValue = (FTM_BOOL)pValue2->xValue.nValue;
				}
				break;

			case	FTM_VALUE_TYPE_ULONG:
				{
					pValue1->xValue.bValue = (FTM_BOOL)pValue2->xValue.ulValue;
				}
				break;

			case	FTM_VALUE_TYPE_FLOAT:
				{
					pValue1->xValue.bValue = (FTM_BOOL)pValue2->xValue.fValue;
				}
				break;

			case	FTM_VALUE_TYPE_BOOL:
				{
					pValue1->xValue.bValue = pValue2->xValue.bValue;
				}
				break;

			case	FTM_VALUE_TYPE_STRING:
				{
					pValue1->xValue.bValue = (FTM_BOOL)strtoul(pValue2->xValue.pValue, NULL, 10);
				}
				break;

			default:
				return	FTM_RET_INVALID_TYPE;
			}
		}
		break;

	case	FTM_VALUE_TYPE_STRING:
		{
			switch(pValue2->xType)
			{
			case	FTM_VALUE_TYPE_INT:
			case	FTM_VALUE_TYPE_ULONG:
			case	FTM_VALUE_TYPE_FLOAT:
			case	FTM_VALUE_TYPE_BOOL:
				{
					xRet = FTM_VALUE_setSTRING(pValue1, FTM_VALUE_print(pValue2));
				}
				break;

			case	FTM_VALUE_TYPE_STRING:
				{
					xRet = FTM_VALUE_setSTRING(pValue1, pValue2->xValue.pValue);
				}
				break;

			default:
				return	FTM_RET_INVALID_TYPE;
			}
		}
		break;
	}

	return	xRet;
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
