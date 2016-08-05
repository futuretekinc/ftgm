#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftm_list.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_OBJECT

typedef struct 
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pTypeString;
} FTM_EP_TYPE_STRING, _PTR_ FTM_EP_TYPE_STRING_PTR;

typedef struct 
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pUnit;
} FTM_EP_TYPE_UNIT, _PTR_ FTM_EP_TYPE_UNIT_PTR;

FTM_BOOL	FTM_EP_seeker
(
	const FTM_VOID_PTR pItem, 
	const FTM_VOID_PTR pIndicator
);

FTM_BOOL	_FTM_EPTypeSeeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_BOOL	_FTM_EPOIDInfoSeeker
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

static
FTM_EP			xDefaultEP =
{
	.xType					= 0,
	.pName 					= {"Unknown"},
	.bEnable				= FTM_TRUE,
	.ulTimeout				= 5,
	.ulUpdateInterval		= 10,
	.ulReportInterval		= 60,
	.xLimit.xType 			= FTM_EP_LIMIT_TYPE_MONTHS,
	.xLimit.xParams.ulMonths= 1
};

static 
FTM_LIST_PTR	pEPList = NULL;

static
FTM_EP_TYPE_UNIT	pEPUnits[] =
{
	{	.xType = FTM_EP_TYPE_TEMPERATURE,.pUnit = "C"	},
	{	.xType = FTM_EP_TYPE_HUMIDITY,	.pUnit = "%RH"	},
	{	.xType = FTM_EP_TYPE_VOLTAGE,	.pUnit = "V"	},
	{	.xType = FTM_EP_TYPE_CURRENT,	.pUnit = "A"	},
	{	.xType = FTM_EP_TYPE_DI	,		.pUnit = ""	},
	{	.xType = FTM_EP_TYPE_DO	,		.pUnit = ""	},
	{	.xType = FTM_EP_TYPE_GAS,		.pUnit = "ppm"	},
	{	.xType = FTM_EP_TYPE_POWER,		.pUnit = "W"	},
	{	.xType = FTM_EP_TYPE_AI	,		.pUnit = ""	},
	{	.xType = FTM_EP_TYPE_COUNT	,	.pUnit = ""	},
	{	.xType = FTM_EP_TYPE_PRESSURE,	.pUnit = ""	},
	{	.xType = FTM_EP_TYPE_MULTI,		.pUnit = ""	}
};

FTM_RET	FTM_EP_init(FTM_VOID)
{
	FTM_RET	xRet;

	if (pEPList != NULL)
	{
		ERROR("EP list is already initialized.\n");
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pEPList);	
	if (xRet != FTM_RET_OK)
	{
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

	FTM_LIST_destroy(pEPList);
	pEPList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_setDefault
(
	FTM_EP_PTR pEP
)
{
	ASSERT(pEP != NULL);
	struct timeval	tv;

	gettimeofday(&tv, NULL);

	memcpy(pEP, &xDefaultEP, sizeof(FTM_EP));
	sprintf(pEP->pEPID, "%lu%lu", tv.tv_sec, tv.tv_usec);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_create
(
	FTM_EP_PTR pSrc, 
	FTM_EP_PTR _PTR_ ppEP
)
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

FTM_RET	FTM_EP_destroy
(
	FTM_EP_PTR pEP
)
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

FTM_RET	FTM_EP_append
(
	FTM_EP_PTR pEP
)
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

FTM_RET	FTM_EP_remove
(
	FTM_EP_PTR pEP
)
{
	ASSERT(pEP != NULL);

	if (pEPList == NULL)
	{
		ERROR("EP list is not initialized.\n");
		FTM_EP_init();
	}

	return	FTM_LIST_remove(pEPList, pEP);
}

FTM_RET	FTM_EP_count
(
	FTM_ULONG_PTR pulCount
)
{
	ASSERT(pulCount != NULL);

	if (pEPList == NULL)
	{
		ERROR("EP list is not initialized.\n");
		FTM_EP_init();
	}

	return	FTM_LIST_count(pEPList, pulCount);
}

FTM_RET	FTM_EP_get
(
	FTM_CHAR_PTR	pEPID, 
	FTM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pEPID != NULL);
	ASSERT(ppEP != NULL);

	if (pEPList == NULL)
	{
		ERROR("EP list is not initialized.\n");
		FTM_EP_init();
	}

	return	FTM_LIST_get(pEPList, pEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTM_EP_getAt
(
	FTM_ULONG ulIndex, 
	FTM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(ppEP != NULL);

	if (pEPList == NULL)
	{
		ERROR("EP list is not initialized.\n");
		FTM_EP_init();
	}

	return	FTM_LIST_getAt(pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTM_EP_getDataType
(
	FTM_EP_PTR 			pEP, 
	FTM_VALUE_TYPE_PTR	pType
)
{
	ASSERT(pEP != NULL);
	ASSERT(pType != NULL);

	switch(pEP->xType)
	{
	case	FTM_EP_TYPE_TEMPERATURE:
	case	FTM_EP_TYPE_HUMIDITY:
	case	FTM_EP_TYPE_VOLTAGE:
	case	FTM_EP_TYPE_CURRENT:
	case	FTM_EP_TYPE_GAS:
	case	FTM_EP_TYPE_POWER:
	case	FTM_EP_TYPE_AI:
	case	FTM_EP_TYPE_PRESSURE:
		{
			*pType = FTM_VALUE_TYPE_FLOAT;
		}
		break;

	case	FTM_EP_TYPE_DI:
	case	FTM_EP_TYPE_DO:
	case	FTM_EP_TYPE_COUNT:
	case	FTM_EP_TYPE_MULTI:
	default:
		{
			*pType = FTM_VALUE_TYPE_INT;
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_isValid(FTM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);
	FTM_RET		xRet;
	FTM_INT		i;

	for(i = 0 ; i < FTM_EPID_LEN+1 ; i++)
	{
		if (pEP->pEPID[i] == 0)
		{
			break;	
		}
	}

	if ((i == 0) || (i == (FTM_EPID_LEN+1)))
	{
		ERROR("Invalid EP ID[%s]\n", pEP->pEPID);
		return	FTM_RET_INVALID_ID;
	}

	if (pEP->xType == 0)
	{
		ERROR("Invalid EP Type[%d]\n", pEP->xType);
		return	FTM_RET_INVALID_TYPE;	
	}

	xRet = FTM_isValidName(pEP->pName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid EP Name[%s]\n", pEP->pName);
		return	xRet;
	}

	xRet = FTM_isValidUnit(pEP->pUnit);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid EP Unit[%s]\n", pEP->pUnit);
		return	xRet;
	}
	
	xRet = FTM_isValidInterval(pEP->ulUpdateInterval);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid EP UpdateInterval[%lu]\n", pEP->ulUpdateInterval);
		return	xRet;
	}

#if 0
	xRet = FTM_EP_isValidTimeout(pEP, pEP->ulTimeout);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid EP Timeout[%lu]\n", pEP->ulTimeout);
		return	xRet;
	}
#endif

	return	xRet;
}

FTM_RET	FTM_EP_isValidTimeout(FTM_EP_PTR pEP, FTM_ULONG	ulTimeout)
{
	ASSERT(pEP != NULL);

	if (pEP->ulUpdateInterval < ulTimeout)
	{
		return	FTM_RET_INVALID_TIMEOUT;	
	}

	return	FTM_RET_OK;
}


FTM_RET	FTM_EP_isStatic(FTM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	if (pEP->xFlags & FTM_EP_FLAG_STATIC)
	{
		return	FTM_RET_TRUE;
	}
	else
	{
		return	FTM_RET_FALSE;	
	}
}

FTM_RET	FTM_EP_isAsyncMode(FTM_EP_PTR pEP)
{
	ASSERT(pEP != NULL);

	if (pEP->xFlags & FTM_EP_FLAG_SYNC)
	{
		return	FTM_RET_FALSE;
	}
	else
	{
		return	FTM_RET_TRUE;	
	}
}

FTM_BOOL		FTM_EP_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pItem != NULL);
	ASSERT(pIndicator != NULL);

	FTM_EP_PTR		pEP = (FTM_EP_PTR)pItem;
	FTM_CHAR_PTR	pEPID = (FTM_CHAR_PTR)pIndicator;

	return	strncpy(pEP->pEPID, pEPID, FTM_EPID_LEN) == 0;
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

FTM_RET	FTM_EP_getDefaultUnit
(
	FTM_EP_TYPE		xType,
	FTM_CHAR_PTR	pUnit,
	FTM_ULONG		ulLen
)
{
	FTM_INT	i;
	for(i = 0 ; i < sizeof(pEPUnits) / sizeof(FTM_EP_TYPE_UNIT) ; i++)
	{
		if (pEPUnits[i].xType == (xType & FTM_EP_TYPE_MASK))
		{
			strncpy(pUnit, pEPUnits[i].pUnit, ulLen);	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_INVALID_TYPE;
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

FTM_RET	FTM_EP_DATA_createInt
(
	FTM_INT 	nValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG 	ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
)
{
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_VALUE_initINT(&pData->xValue, nValue);
	pData->xState = xState;
	pData->ulTime = ulTime;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createUlong
(
	FTM_ULONG 	ulValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG 	ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
)
{
	ASSERT(ppData != NULL);
	
	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_VALUE_initULONG(&pData->xValue, ulValue);
	pData->xState = xState;
	pData->ulTime = ulTime;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createFloat
(
	FTM_DOUBLE 	fValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
)
{
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_VALUE_initULONG(&pData->xValue, fValue);
	pData->xState = xState;
	pData->ulTime = ulTime;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_createBool
(
	FTM_BOOL 	bValue, 
	FTM_EP_DATA_STATE xState, 
	FTM_ULONG ulTime, 
	FTM_EP_DATA_PTR _PTR_ ppData
)
{
	ASSERT(ppData != NULL);

	FTM_EP_DATA_PTR	pData;

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA));
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	FTM_VALUE_initBOOL(&pData->xValue, bValue);
	pData->xState = xState;
	pData->ulTime = ulTime;

	*ppData = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_init
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_TYPE	xType,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pData != NULL);
	FTM_RET	xRet;

	xRet = FTM_VALUE_init(&pData->xValue, xType);
	if (xRet == FTM_RET_OK)
	{
		struct timeval	xTimeval;

		if (pValue != NULL)
		{
			xRet = FTM_VALUE_setFromString(&pData->xValue, pValue);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;
			}
		}

		gettimeofday(&xTimeval, NULL);

		pData->xState = FTM_EP_DATA_STATE_VALID;
		pData->ulTime = xTimeval.tv_sec;

	}

	return	xRet;
}

FTM_RET	FTM_EP_DATA_initINT
(
	FTM_EP_DATA_PTR pData,
	FTM_INT 		nValue
)
{
	ASSERT(pData != NULL);

	struct timeval	xTimeval;
	gettimeofday(&xTimeval, NULL);

	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->ulTime = xTimeval.tv_sec;
	FTM_VALUE_initINT(&pData->xValue, nValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_initULONG
(
	FTM_EP_DATA_PTR pData,
	FTM_ULONG 		ulValue
)
{
	ASSERT(pData != NULL);
	
	struct timeval	xTimeval;
	gettimeofday(&xTimeval, NULL);

	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->ulTime = xTimeval.tv_sec;
	FTM_VALUE_initULONG(&pData->xValue, ulValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_initFLOAT
(
	FTM_EP_DATA_PTR pData,
	FTM_DOUBLE 	fValue
)
{
	ASSERT(pData != NULL);

	struct timeval	xTimeval;
	gettimeofday(&xTimeval, NULL);

	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->ulTime = xTimeval.tv_sec;
	FTM_VALUE_initULONG(&pData->xValue, fValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_initBOOL
(
	FTM_EP_DATA_PTR pData,
	FTM_BOOL 		bValue
)
{
	ASSERT(pData != NULL);
	
	struct timeval	xTimeval;
	gettimeofday(&xTimeval, NULL);

	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->ulTime = xTimeval.tv_sec;
	FTM_VALUE_initBOOL(&pData->xValue, bValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_initVALUE
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_PTR	pValue
)
{
	ASSERT(pData != NULL);
	ASSERT(pValue != NULL);
	
	FTM_RET	xRet;
	struct timeval	xTimeval;

	gettimeofday(&xTimeval, NULL);
	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->ulTime = xTimeval.tv_sec;
	xRet = FTM_VALUE_init(&pData->xValue, pValue->xType);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_setVALUE(&pData->xValue, pValue);
	}
	
	return	xRet;

}

FTM_RET	FTM_EP_DATA_final
(
	FTM_EP_DATA_PTR		pData
)
{
	ASSERT(pData != NULL);

	return	FTM_VALUE_final(&pData->xValue);
}

FTM_RET	FTM_EP_DATA_initValueFromString
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_TYPE	xType,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pData != NULL);
	ASSERT(pValue != NULL);

	FTM_RET	xRet;
	struct timeval	xTimeval;

	gettimeofday(&xTimeval, NULL);
	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->ulTime = xTimeval.tv_sec;
	xRet = FTM_VALUE_init(&pData->xValue, xType);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_setFromString(&pData->xValue, pValue);
	}

	return	xRet;
}

FTM_RET	FTM_EP_DATA_setValueFromString
(
	FTM_EP_DATA_PTR	pData,
	FTM_CHAR_PTR	pValue
)
{
	ASSERT(pData != NULL);
	ASSERT(pValue != NULL);

	FTM_RET	xRet;

	xRet = FTM_VALUE_setFromString(&pData->xValue, pValue);
	if (xRet == FTM_RET_OK)
	{
		pData->ulTime = time(NULL);
		pData->xState = FTM_EP_DATA_STATE_VALID;
	}

	return	xRet;
}

FTM_RET	FTM_EP_DATA_setValue
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_PTR	pValue
)
{
	ASSERT(pData != NULL);
	ASSERT(pValue != NULL);

	FTM_RET	xRet;

	xRet = FTM_VALUE_setVALUE(&pData->xValue, pValue);
	if (xRet == FTM_RET_OK)
	{
		pData->ulTime = time(NULL);
		pData->xState = FTM_EP_DATA_STATE_VALID;
	}
	
	return	xRet;
}

FTM_RET	FTM_EP_DATA_destroy
(
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_MEM_free(pData);

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_compare
(
	FTM_EP_DATA_PTR pData1, 
	FTM_EP_DATA_PTR pData2, 
	FTM_INT_PTR pResult
)
{
	ASSERT(pData1 != NULL);
	ASSERT(pData2 != NULL);
	ASSERT(pResult != NULL);

	return	FTM_VALUE_compare(&pData1->xValue, &pData2->xValue, pResult);
}

FTM_RET	FTM_EP_DATA_toValue
(
	FTM_EP_DATA_PTR	pData,
	FTM_VALUE_PTR	pValue
)
{
	ASSERT(pData != NULL);
	ASSERT(pValue != NULL);
	
	memcpy(pValue, &pData->xValue, sizeof(FTM_VALUE));

	return	FTM_RET_OK;
}

FTM_RET	FTM_EP_DATA_snprint
(
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulMaxLen, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pBuff != NULL);
	ASSERT(pData != NULL);

	if (pData->xState == FTM_EP_DATA_STATE_VALID)
	{
		return	FTM_VALUE_snprint(pBuff, ulMaxLen, &pData->xValue);
	}
	
	snprintf(pBuff, ulMaxLen, "N/A");

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_EP_DATA_print
(
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pData != NULL);

	return	FTM_VALUE_print(&pData->xValue);
}

static
FTM_EP_TYPE_STRING	_typeString[] =
{
	{
		.xType = FTM_EP_TYPE_TEMPERATURE,
		.pTypeString = "TEMPERATURE"
	},	
	{
		.xType = FTM_EP_TYPE_HUMIDITY,
		.pTypeString = "HUMIDITY"	
	},
	{
		.xType = FTM_EP_TYPE_VOLTAGE,
		.pTypeString = "VOLTAGE"
	},
	{
		.xType = FTM_EP_TYPE_CURRENT,
		.pTypeString = "CURRENT"	
	},
	{
		.xType = FTM_EP_TYPE_DI,
		.pTypeString = "DIGITAL INPUT"
	},
	{
		.xType = FTM_EP_TYPE_DI,
		.pTypeString = "ONOFF"
	},
	{	
		.xType = FTM_EP_TYPE_DO,
		.pTypeString = "DIGITAL OUTPUT"
	},
	{	
		.xType = FTM_EP_TYPE_GAS,
		.pTypeString = "GAS"
	},
	{	
		.xType = FTM_EP_TYPE_GAS,
		.pTypeString = "CO2"
	},
	{	
		.xType = FTM_EP_TYPE_POWER,
		.pTypeString = "POWER"
	},
	{
		.xType = FTM_EP_TYPE_AI,
		.pTypeString = "ANALOG INPUT"
	},
	{	
		.xType = FTM_EP_TYPE_PRESSURE,
		.pTypeString = "PRESSURE"
	},
	{	
		.xType = FTM_EP_TYPE_MULTI,
		.pTypeString = "MULTI-FUNCTION"
	},
};

FTM_CHAR_PTR	FTM_EP_typeString
(
	FTM_EP_TYPE xType
)
{
	FTM_INT	i;

	for(i = 0 ; i < sizeof(_typeString) / sizeof(FTM_EP_TYPE_STRING) ; i++)
	{
		if (_typeString[i].xType == xType)
		{
			return	_typeString[i].pTypeString;	
		}
	}

	static FTM_CHAR	pBuff[16];
	sprintf(pBuff, "%08lx", xType);

	return	pBuff;
}

FTM_RET	FTM_EP_strToType
(
	FTM_CHAR_PTR	pString,
	FTM_EP_TYPE_PTR	pType
)
{
	ASSERT(pString != NULL);
	ASSERT(pType != NULL);

	FTM_INT	i;

	for(i = 0 ; i < sizeof(_typeString) / sizeof(FTM_EP_TYPE_STRING) ; i++)
	{
		if (strcasecmp(_typeString[i].pTypeString, pString) == 0)
		{
			*pType = _typeString[i].xType;	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;

}

FTM_RET	FTM_EP_print
(
	FTM_EP_PTR	pEP
)
{
	ASSERT(pEP != NULL);

	MESSAGE("%16s : %s\n", 	"EPID", pEP->pEPID);
	MESSAGE("%16s : %s\n", 	"Type", FTM_EP_typeString(pEP->xType));
	MESSAGE("%16s : %s\n", 	"Name", pEP->pName);
	MESSAGE("%16s : %s\n", 	"Unit", pEP->pUnit);
	MESSAGE("%16s : ", 		"Enabled");
	if (pEP->bEnable)
	{
		MESSAGE("%s\n", "Yes");
	}
	else
	{
		MESSAGE("%s\n", "No");
	}

	MESSAGE("%16s : %lu\n", "Update interval", pEP->ulUpdateInterval);
	MESSAGE("%16s : %lu\n", "Report interval", pEP->ulReportInterval);
	MESSAGE("%16s : %lu\n", "Timeout", 	pEP->ulTimeout);
	MESSAGE("%16s : %s\n", 	"DID", 		pEP->pDID);

	switch(pEP->xLimit.xType)
	{
	case	FTM_EP_LIMIT_TYPE_TIME:
		{
			MESSAGE("%16s : %s(%lu ~ %lu)\n", "Limit","Time", pEP->xLimit.xParams.xTime.ulStart, pEP->xLimit.xParams.xTime.ulEnd);
		}
		break;

	case	FTM_EP_LIMIT_TYPE_MONTHS:
		{
			MESSAGE("%16s : %lu months\n",	"Limit", pEP->xLimit.xParams.ulMonths);
		}
		break;

	case	FTM_EP_LIMIT_TYPE_DAYS:
		{
			MESSAGE("%16s : %lu days\n",	"Limit", pEP->xLimit.xParams.ulCount);
		}
		break;

	case	FTM_EP_LIMIT_TYPE_HOURS:
		{
			MESSAGE("%16s : %lu hours\n",	"Limit", pEP->xLimit.xParams.ulCount);
		}
		break;

	case	FTM_EP_LIMIT_TYPE_COUNT:
		{
			MESSAGE("%16s : %s(%lu)\n",	"Limit","Count", pEP->xLimit.xParams.ulCount);
		}
		break;
	}
	return	FTM_RET_OK;
}
