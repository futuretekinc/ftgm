#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "simclist.h"

typedef struct 
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pTypeString;
} FTM_EP_TYPE_STRING, _PTR_ FTM_EP_TYPE_STRING_PTR;

static int	_FTM_EPTypeSeeker(const void *pElement, const void *pKey);
static int	_FTM_EPOIDInfoSeeker(const void *pElement, const void *pKey);

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

static list_t	_xEPTypeList;

FTM_RET	FTM_initEPTypeString(void)
{
	list_init(&_xEPTypeList);
	list_attributes_seeker(&_xEPTypeList, _FTM_EPTypeSeeker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_finalEPTypeString(void)
{
	FTM_EP_TYPE_STRING_PTR	pItem;

	list_iterator_start(&_xEPTypeList);
	while((pItem = list_iterator_next(&_xEPTypeList)) != NULL)
	{
		free(pItem->pTypeString);	
		free(pItem);	
	}

	list_destroy(&_xEPTypeList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_appendEPTypeString(FTM_EP_TYPE xType, FTM_CHAR_PTR pTypeString)
{
	FTM_EP_TYPE_STRING_PTR	pItem;

	pItem = list_seek(&_xEPTypeList, &xType);
	if (pItem != NULL)
	{
		return	FTM_RET_ALREADY_EXISTS;	
	}

	pItem = (FTM_EP_TYPE_STRING_PTR)calloc(1, sizeof(FTM_EP_TYPE_STRING));
	if (pItem == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pItem->xType = xType;
	pItem->pTypeString = strdup(pTypeString);

	if (list_append(&_xEPTypeList, pItem) < 0)
	{
		free(pItem->pTypeString);
		free(pItem);

		return	FTM_RET_INTERNAL_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR FTM_getEPTypeString(FTM_EP_TYPE xType)
{
	FTM_EP_TYPE_STRING_PTR pItem;

	pItem = list_seek(&_xEPTypeList, &xType);
	if (pItem != NULL)
	{
		return	pItem->pTypeString;
	}

	pItem = _pEPTypeString;
	while(pItem->xType != 0)
	{
		if ((xType & FTM_EP_CLASS_MASK) == pItem->xType)
		{
			return	pItem->pTypeString;	
		}
		pItem ++;	
	}

	return	"UNKNOWN";
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


static list_t	xEPOIDInfoList;

FTM_RET	FTM_initEPOIDInfo(void)
{
	list_init(&xEPOIDInfoList);
	list_attributes_seeker(&xEPOIDInfoList, _FTM_EPOIDInfoSeeker);

	return	FTM_RET_OK;
}

FTM_RET	FTM_finalEPOIDInfo(void)
{
	FTM_EP_OID_INFO_PTR	pItem;

	list_iterator_start(&xEPOIDInfoList);
	while((pItem = list_iterator_next(&xEPOIDInfoList)) != NULL)
	{
		free(pItem);	
	}

	list_destroy(&xEPOIDInfoList);

	return	FTM_RET_OK;
}


FTM_RET	FTM_addEPOIDInfo
(
	FTM_EP_OID_INFO_PTR	pOIDInfo		
)
{
	FTM_EP_OID_INFO_PTR	pNewOIDInfo;

	if (list_seek(&xEPOIDInfoList, (FTM_VOID_PTR)&pOIDInfo->ulClass) != NULL)
	{
		return	FTM_RET_ALREADY_EXISTS;	
	}
	
	pNewOIDInfo = (FTM_EP_OID_INFO_PTR)calloc(1, sizeof(FTM_EP_OID_INFO));
	if (pNewOIDInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewOIDInfo, pOIDInfo, sizeof(FTM_EP_OID_INFO));

	list_append(&xEPOIDInfoList, pNewOIDInfo);

	return	FTM_RET_OK;
}

FTM_RET	FTM_getEPOIDInfo(FTM_EPID xEPID, FTM_EP_OID_INFO_PTR _PTR_ ppOIDInfo)
{
	FTM_EP_OID_INFO_PTR	pOIDInfo;

	pOIDInfo = (FTM_EP_OID_INFO_PTR)list_seek(&xEPOIDInfoList, (FTM_VOID_PTR)&xEPID);

	if (pOIDInfo == NULL)
	{
		return	FTM_RET_NOT_EXISTS;	
	}

	*ppOIDInfo = pOIDInfo;

	return	FTM_RET_OK;
}

int	_FTM_EPOIDInfoSeeker(const void *pElement, const void *pKey)
{
	FTM_EP_OID_INFO_PTR 	pOIDInfo = (FTM_EP_OID_INFO_PTR)pElement;
	FTM_EPID_PTR			pEPID = (FTM_EPID_PTR)pKey;

	if (pOIDInfo->ulClass == (FTM_EP_CLASS_MASK & (*pEPID)))
	{
		return	1;	
	}

	return	0;
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
