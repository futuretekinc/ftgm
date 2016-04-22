#include <string.h>
#include <stdlib.h>
#include "ftm.h"
#include "ftm_json.h"

FTM_RET	FTM_JSON_createString
(
	FTM_CHAR_PTR	pString,
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_STRING_PTR pItem = NULL;
    
    pItem = (FTM_JSON_STRING_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_STRING));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_STRING;
    pItem->pValue = FTM_MEM_malloc(strlen(pString) + 1);
    if (pItem->pValue == NULL)
    {
        FTM_MEM_free(pItem);
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    strcpy(pItem->pValue, pString);

	*ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createNumber
(
	FTM_INT	nValue,
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_NUMBER_PTR pItem = NULL;
    
    pItem = (FTM_JSON_NUMBER_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_NUMBER));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_NUMBER;
    pItem->nValue = nValue;
   
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createHex
(
	FTM_INT	nValue,
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_NUMBER_PTR pItem = NULL;
    
    pItem = (FTM_JSON_NUMBER_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_HEX));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_HEX;
    pItem->nValue = nValue;
    
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createFloat
(
	FTM_FLOAT	fValue,
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)	
{
    FTM_JSON_FLOAT_PTR pItem = NULL;
    
    pItem = (FTM_JSON_FLOAT_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_FLOAT));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_FLOAT;
    pItem->fValue = fValue;
    
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createPair
(
	FTM_CHAR_PTR	pString, 
	FTM_JSON_VALUE_PTR pValue,
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_PAIR_PTR pItem = NULL;
    
    pItem = (FTM_JSON_PAIR_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_PAIR));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_PAIR;
    pItem->pString = FTM_MEM_malloc(strlen(pString) + 1);
    if (pItem->pString== NULL)
    {
        FTM_MEM_free(pItem);
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    strcpy(pItem->pString, pString);
    pItem->pValue = pValue;
    
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}


FTM_RET  FTM_JSON_createObject
(
	FTM_ULONG	ulMaxCount,
	FTM_JSON_OBJECT_PTR	_PTR_ ppItem
)
{
    FTM_JSON_OBJECT_PTR pItem = NULL;
    
    pItem = (FTM_JSON_OBJECT_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_OBJECT) + sizeof(FTM_JSON_PAIR_PTR)*ulMaxCount);
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_OBJECT;
    pItem->nMaxCount = ulMaxCount;
    
    *ppItem = pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createArray
(
	FTM_ULONG	ulMaxCount,
	FTM_JSON_ARRAY_PTR	_PTR_ ppItem
)
{
    FTM_JSON_ARRAY_PTR pItem = NULL;
    
    pItem = (FTM_JSON_ARRAY_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_ARRAY) + sizeof(FTM_JSON_VALUE_PTR)*ulMaxCount);
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_ARRAY;
    pItem->nMaxCount = ulMaxCount;
    
    *ppItem = pItem;

	return	FTM_RET_OK;
}


FTM_RET FTM_JSON_OBJECT_setPair
(
	FTM_JSON_OBJECT_PTR pObject, 
	FTM_CHAR_PTR		pString, 
	FTM_JSON_VALUE_PTR 	pValue
)
{
    ASSERT((pObject != NULL) && (pValue != NULL));
   
   	FTM_RET	xRet;

    if (pObject->nCount < pObject->nMaxCount)
    {
        int i;
        
        for(i = 0; i < pObject->nMaxCount ; i++)
        {
            if (pObject->pPairs[i] == 0)
            {
                FTM_JSON_PAIR_PTR   pPair;
				
				xRet = FTM_JSON_createPair(pString, pValue, (FTM_JSON_VALUE_PTR _PTR_)&pPair);
                if (xRet != FTM_RET_OK)
                {
                    return  xRet;
                }
                
                pObject->pPairs[i] = pPair;
                pObject->nCount++;

                return  FTM_RET_OK;
            }
        }
    }
    
    return  FTM_RET_ERROR;
}

FTM_RET FTM_JSON_ARRAY_setElement
(
	FTM_JSON_ARRAY_PTR pArray, 
	FTM_JSON_VALUE_PTR pElement
)
{
    ASSERT((pArray != NULL) && (pElement != NULL));
    
    if (pArray->nCount < pArray->nMaxCount)
    {
        int i;
        
        for(i = 0; i < pArray->nMaxCount ; i++)
        {
            if (pArray->pElements[i] == 0)
            {
                pArray->pElements[i] = pElement;
                pArray->nCount++;
        
                return  FTM_RET_OK;
            }
        }
    }
    
    return  FTM_RET_ERROR;
}

FTM_RET  FTM_JSON_createTrue
(
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_VALUE_PTR pItem = NULL;
    
    pItem = (FTM_JSON_VALUE_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_VALUE));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_TRUE;
   
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createFalse
(
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_VALUE_PTR pItem = NULL;
    
    pItem = (FTM_JSON_VALUE_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_VALUE));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_FALSE;
    
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET  FTM_JSON_createNull
(
	FTM_JSON_VALUE_PTR	_PTR_ ppItem
)
{
    FTM_JSON_VALUE_PTR pItem = NULL;
    
    pItem = (FTM_JSON_VALUE_PTR)FTM_MEM_malloc(sizeof(FTM_JSON_VALUE));
    if (pItem == NULL)
    {
        return  FTM_RET_NOT_ENOUGH_MEMORY;
    }
    
    pItem->xType = FTM_JSON_TYPE_NULL;
    
    *ppItem = (FTM_JSON_VALUE_PTR)pItem;

	return	FTM_RET_OK;
}

FTM_RET FTM_JSON_destroy
(
	FTM_JSON_VALUE_PTR _PTR_ ppItem
)
{
    ASSERT(ppItem != NULL);
    
    switch((*ppItem)->xType)
    {
    case    FTM_JSON_TYPE_STRING:
        {           
            if (((FTM_JSON_STRING_PTR)(*ppItem))->pValue != NULL)
            {
                FTM_MEM_free(((FTM_JSON_STRING_PTR)(*ppItem))->pValue);
                ((FTM_JSON_STRING_PTR)(*ppItem))->pValue = NULL;
            }
        }
        break;
        
    case    FTM_JSON_TYPE_PAIR:   
        {
            if (((FTM_JSON_PAIR_PTR)(*ppItem))->pString != NULL)
            {
                FTM_MEM_free(((FTM_JSON_PAIR_PTR)(*ppItem))->pString);
                ((FTM_JSON_PAIR_PTR)(*ppItem))->pString = NULL;
            }
            
            if (((FTM_JSON_PAIR_PTR)(*ppItem))->pValue != NULL)
            {
                FTM_JSON_destroy(&((FTM_JSON_PAIR_PTR)(*ppItem))->pValue);
                ((FTM_JSON_PAIR_PTR)(*ppItem))->pValue = NULL;
            }
        }
        break;

    case    FTM_JSON_TYPE_OBJECT:
        {
            int i;
            
            for(i = 0 ; i < ((FTM_JSON_OBJECT_PTR)(*ppItem))->nMaxCount ; i++)
            {
                if (((FTM_JSON_OBJECT_PTR)(*ppItem))->pPairs[i] != NULL)
                {
                    FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&((FTM_JSON_OBJECT_PTR)(*ppItem))->pPairs[i]);
                    ((FTM_JSON_OBJECT_PTR)(*ppItem))->pPairs[i] = NULL;
                }
            }
            ((FTM_JSON_OBJECT_PTR)(*ppItem))->nCount = 0;
        }
        break;
        
    case    FTM_JSON_TYPE_ARRAY:
        {
            int i;
            
            for(i = 0 ; i < ((FTM_JSON_ARRAY_PTR)(*ppItem))->nMaxCount ; i++)
            {
                if (((FTM_JSON_ARRAY_PTR)(*ppItem))->pElements[i] != NULL)
                {
                    FTM_JSON_destroy(&((FTM_JSON_ARRAY_PTR)(*ppItem))->pElements[i]);
                    ((FTM_JSON_ARRAY_PTR)(*ppItem))->pElements[i] = NULL;
                }
            }
            ((FTM_JSON_ARRAY_PTR)(*ppItem))->nCount = 0;
        }
        break;
    case    FTM_JSON_TYPE_NUMBER:
    case    FTM_JSON_TYPE_HEX:
    case    FTM_JSON_TYPE_FLOAT:
    case    FTM_JSON_TYPE_TRUE:
    case    FTM_JSON_TYPE_FALSE:
    case    FTM_JSON_TYPE_NULL:
        break;
        
    default:
        return  FTM_RET_ERROR;
    }
    
    FTM_MEM_free(*ppItem);
   
   	*ppItem = NULL;

    return  FTM_RET_OK;
}

FTM_RET FTM_JSON_snprint
(
	FTM_CHAR_PTR	pBuff, 
	FTM_ULONG		ulBuffLen, 
	FTM_JSON_VALUE_PTR pValue
)
{
    FTM_INT	nLen = 0;
    
    ASSERT((pBuff != NULL) && (pValue != NULL));
    
    switch(pValue->xType)
    {
    case    FTM_JSON_TYPE_STRING:
        {
            nLen = snprintf(pBuff, ulBuffLen, "\"%s\"", ((FTM_JSON_STRING_PTR)pValue)->pValue);
        }
        break;
        
    case    FTM_JSON_TYPE_PAIR:   
        {
            nLen = snprintf(pBuff, ulBuffLen, "\"%s\":", ((FTM_JSON_PAIR_PTR)pValue)->pString);
            nLen += FTM_JSON_snprint(&pBuff[nLen], ulBuffLen - nLen, ((FTM_JSON_PAIR_PTR)pValue)->pValue);
        }
        break;
        
    case    FTM_JSON_TYPE_OBJECT:
        {            
            FTM_INT	i;
            FTM_BOOL	bFirst = FTM_TRUE;
            
            nLen = snprintf(pBuff, ulBuffLen, "{");
            for(i = 0 ; i < ((FTM_JSON_OBJECT_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTM_JSON_OBJECT_PTR)pValue)->pPairs[i] != NULL)
                {
                    if (!bFirst)
                    {
                        nLen += snprintf(&pBuff[nLen], ulBuffLen - nLen, ",");
                    }
                    nLen += FTM_JSON_snprint(&pBuff[nLen], ulBuffLen - nLen, (FTM_JSON_VALUE_PTR)((FTM_JSON_OBJECT_PTR)pValue)->pPairs[i]);
                    bFirst = FTM_FALSE;
                }
            }
            nLen += snprintf(&pBuff[nLen], ulBuffLen - nLen, "}");
        }
        break;

    case    FTM_JSON_TYPE_ARRAY:
        {            
            FTM_INT	i;
            FTM_BOOL	bFirst = FTM_TRUE;
             
            nLen = snprintf(pBuff, ulBuffLen, "[");
            for(i = 0 ; i < ((FTM_JSON_ARRAY_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTM_JSON_ARRAY_PTR)pValue)->pElements[i] != NULL)
                {
                    if (!bFirst)
                    {
                        nLen += snprintf(&pBuff[nLen], ulBuffLen - nLen, ",");
                    }
                    nLen += FTM_JSON_snprint(&pBuff[nLen], ulBuffLen - nLen, ((FTM_JSON_ARRAY_PTR)pValue)->pElements[i]);
                    bFirst = FTM_FALSE;
                }
            }
            nLen += snprintf(&pBuff[nLen], ulBuffLen - nLen, "]");
        }
        break;

    case    FTM_JSON_TYPE_NUMBER:
        {
            nLen = snprintf(pBuff, ulBuffLen, "%d", ((FTM_JSON_NUMBER_PTR)pValue)->nValue);
       }
        break;
        
    case    FTM_JSON_TYPE_HEX:
        {
            nLen = snprintf(pBuff, ulBuffLen, "0x%08x", ((FTM_JSON_HEX_PTR)pValue)->nValue);
       }
        break;
        
    case    FTM_JSON_TYPE_FLOAT:
        {
            nLen = snprintf(pBuff, ulBuffLen, "%5.2f", ((FTM_JSON_FLOAT_PTR)pValue)->fValue);
       }
        break;
    case    FTM_JSON_TYPE_TRUE:
        {
             nLen = snprintf(pBuff, ulBuffLen, "true");
        }
        break;
    case    FTM_JSON_TYPE_FALSE:
        {
             nLen = snprintf(pBuff, ulBuffLen, "false");
        }
        break;
    case    FTM_JSON_TYPE_NULL:
        {
             nLen = snprintf(pBuff, ulBuffLen, "null");
        }
        break;
        
    default:
        return  0;
    }
    
    return  nLen;
}

FTM_RET FTM_JSON_buffSize
(
	FTM_JSON_VALUE_PTR 	pValue,
	FTM_ULONG_PTR		pulLen
)
{
    FTM_INT	nLen = 0;
    static FTM_CHAR	pBuff[128];    
    
    ASSERT((pBuff != NULL) && (pValue != NULL));
    
    switch(pValue->xType)
    {
    case    FTM_JSON_TYPE_STRING:
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "\"%s\"", ((FTM_JSON_STRING_PTR)pValue)->pValue);
        }
        break;
        
    case    FTM_JSON_TYPE_PAIR:   
        {
			FTM_ULONG	ulPartLen;

            nLen = snprintf(pBuff, sizeof(pBuff), "\"%s\":", ((FTM_JSON_PAIR_PTR)pValue)->pString);
            FTM_JSON_buffSize(((FTM_JSON_PAIR_PTR)pValue)->pValue, &ulPartLen);
			nLen += ulPartLen;
        }
        break;
        
    case    FTM_JSON_TYPE_OBJECT:
        {            
            FTM_INT	i;
            FTM_BOOL	bFirst = FTM_TRUE;
            
            nLen = snprintf(pBuff, sizeof(pBuff), "{");
            for(i = 0 ; i < ((FTM_JSON_OBJECT_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTM_JSON_OBJECT_PTR)pValue)->pPairs[i] != NULL)
                {
					FTM_ULONG	ulPartLen;

                    if (!bFirst)
                    {
                        nLen += snprintf(pBuff, sizeof(pBuff), ",");
                    }
                    FTM_JSON_buffSize((FTM_JSON_VALUE_PTR)((FTM_JSON_OBJECT_PTR)pValue)->pPairs[i], &ulPartLen);
					nLen += ulPartLen;
                    bFirst = FTM_FALSE;
                }
            }
            nLen += snprintf(pBuff, sizeof(pBuff), "}");
        }
        break;

    case    FTM_JSON_TYPE_ARRAY:
        {            
            FTM_INT	i;
            FTM_BOOL	bFirst = FTM_TRUE;
            
            nLen = snprintf(pBuff, sizeof(pBuff), "[");
            for(i = 0 ; i < ((FTM_JSON_ARRAY_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTM_JSON_ARRAY_PTR)pValue)->pElements[i] != NULL)
                {
					FTM_ULONG	ulPartLen;

                    if (!bFirst)
                    {
                        nLen += snprintf(pBuff, sizeof(pBuff), ",");
                    }
                    FTM_JSON_buffSize(((FTM_JSON_ARRAY_PTR)pValue)->pElements[i], &ulPartLen);
					nLen += ulPartLen;
                    bFirst = FTM_FALSE;
                }
            }
            nLen += snprintf(pBuff, sizeof(pBuff), "]");
        }
        break;

    case    FTM_JSON_TYPE_NUMBER:
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "%d", ((FTM_JSON_NUMBER_PTR)pValue)->nValue);
       }
        break;
        
    case    FTM_JSON_TYPE_HEX:
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "0x%08x", ((FTM_JSON_HEX_PTR)pValue)->nValue);
       }
        break;
        
    case    FTM_JSON_TYPE_FLOAT:
		{
            nLen = snprintf(pBuff, sizeof(pBuff), "%5.2f", ((FTM_JSON_FLOAT_PTR)pValue)->fValue);
       	}
        break;
    case    FTM_JSON_TYPE_TRUE:
        {
             nLen = snprintf(pBuff, sizeof(pBuff), "true");
        }
        break;
    case    FTM_JSON_TYPE_FALSE:
        {
             nLen = snprintf(pBuff, sizeof(pBuff), "false");
        }
        break;
    case    FTM_JSON_TYPE_NULL:
        {
             nLen = snprintf(pBuff, sizeof(pBuff), "null");
        }
        break;
        
    default:
        return  FTM_RET_ERROR;
    }
    
    *pulLen = nLen;

	return	FTM_RET_OK;
}
