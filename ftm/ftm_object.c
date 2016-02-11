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

static FTM_EP_TYPE_STRING	_pEPTypeString[] =
{
	{	FTM_EP_CLASS_TEMPERATURE,	"TEMPERATURE" 	},
	{	FTM_EP_CLASS_HUMIDITY,		"HUMIDITY"		},
	{	FTM_EP_CLASS_VOLTAGE,		"VOLTAGE"		},
	{	FTM_EP_CLASS_CURRENT,		"CURRENT"		},
	{	FTM_EP_CLASS_DI,			"DIGITAL INPUT"	},
	{	FTM_EP_CLASS_DO,			"DIGITAL OUTPUT"},
	{	FTM_EP_CLASS_GAS,			"GAS"			},
	{	FTM_EP_CLASS_POWER,			"POWER"			},
	{	FTM_EP_CLASS_AI ,			"ANALOG INPUT"	},
	{	FTM_EP_CLASS_MULTI,			"MULTI FUNCTIONS"},
	{	0,							NULL}
};

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

FTM_CHAR_PTR FTM_nodeTypeString(FTM_NODE_TYPE nType)
{
	static FTM_CHAR	pBuff[256];
	switch(nType)
	{
	case	FTM_NODE_TYPE_SNMP: return	"SNMP";
	case	FTM_NODE_TYPE_MODBUS_OVER_TCP: return	"MODBUS/TCP";
	case	FTM_NODE_TYPE_MODBUS_OVER_SERIAL: return	"MODBUS/SERIAL";
	default:	sprintf(pBuff, "UNKNOWN(%08lx)", nType);
	}

	return	pBuff;
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

static FTM_LIST_PTR	_pEPOIDInfoList = NULL;

FTM_RET	FTM_initEPOIDInfo(void)
{
	FTM_LIST_create(&_pEPOIDInfoList);
	FTM_LIST_setSeeker(_pEPOIDInfoList, _FTM_EPOIDInfoSeeker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_finalEPOIDInfo(void)
{
	FTM_EP_OID_INFO_PTR	pItem;

	FTM_LIST_iteratorStart(_pEPOIDInfoList);
	while(FTM_LIST_iteratorNext(_pEPOIDInfoList, (FTM_VOID_PTR _PTR_)&pItem) == FTM_RET_OK)
	{
		FTM_MEM_free(pItem);	
	}

	FTM_LIST_destroy(_pEPOIDInfoList);

	return	FTM_RET_OK;
}


FTM_RET	FTM_addEPOIDInfo
(
	FTM_EP_OID_INFO_PTR	pOIDInfo		
)
{
	ASSERT(pOIDInfo != NULL);

	FTM_EP_OID_INFO_PTR	pNewOIDInfo;

	if (FTM_LIST_get(_pEPOIDInfoList, (FTM_VOID_PTR)&pOIDInfo->ulClass, (FTM_VOID_PTR _PTR_)&pNewOIDInfo) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXISTS;	
	}
	
	pNewOIDInfo = (FTM_EP_OID_INFO_PTR)FTM_MEM_calloc(1, sizeof(FTM_EP_OID_INFO));
	if (pNewOIDInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewOIDInfo, pOIDInfo, sizeof(FTM_EP_OID_INFO));

	return FTM_LIST_append(_pEPOIDInfoList, pNewOIDInfo);
}

FTM_RET	FTM_getEPOIDInfo(FTM_EPID xEPID, FTM_EP_OID_INFO_PTR _PTR_ ppOIDInfo)
{
	ASSERT(ppOIDInfo != NULL);

	FTM_EP_OID_INFO_PTR	pOIDInfo;

	if (FTM_LIST_get(_pEPOIDInfoList, (FTM_VOID_PTR)&pOIDInfo->ulClass, (FTM_VOID_PTR _PTR_)&pOIDInfo) == FTM_RET_OK)
	{
		*ppOIDInfo = pOIDInfo;

		return	FTM_RET_OK;
	}

	return	FTM_RET_NOT_EXISTS;	
}

FTM_BOOL _FTM_EPOIDInfoSeeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pKey)
{
	ASSERT(pElement != NULL);
	ASSERT(pKey != NULL);

	FTM_EP_OID_INFO_PTR 	pOIDInfo = (FTM_EP_OID_INFO_PTR)pElement;
	FTM_EPID_PTR			pEPID = (FTM_EPID_PTR)pKey;

	return (pOIDInfo->ulClass == (FTM_EP_CLASS_MASK & (*pEPID)));
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
