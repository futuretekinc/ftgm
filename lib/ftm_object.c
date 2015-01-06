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

int	_FTM_EPTypeSeeker(const void *pElement, const void *pKey);

FTM_EP_TYPE_STRING	_pEPTypeString[] =
{
	{	FTM_EP_CLASS_TEMPERATURE,	"TEMPERATURE" 	},
	{	FTM_EP_CLASS_HUMIDITY,		"HUMIDITY"		},
	{	FTM_EP_CLASS_VOLTAGE,		"VOLTAGE"		},
	{	FTM_EP_CLASS_CURRENT,		"CURRENT"		},
	{	FTM_EP_CLASS_DI,			"DIGITAL INPUT"	},
	{	FTM_EP_CLASS_DO,			"DIGITAL OUTPUT"},
	{	FTM_EP_CLASS_GAS,			"GAS"			},
	{	FTM_EP_CLASS_POWER,			"POWER"			},
	{	FTM_EP_CLASS_SRF,			"SOUND RANGE FINDER"},
	{	FTM_EP_CLASS_AI ,			"ANALOG INPUT"	},
	{	FTM_EP_CLASS_MULTI,			"MULTI FUNCTIONS"},
	{	0,							NULL}
};

list_t	_xEPTypeList;

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
