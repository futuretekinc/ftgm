#include <string.h>
#include <limits.h>
#include "ftm_types.h"
#include "libconfig.h"
#include "ftm_config.h"
#include "ftm_trace.h"
#include "ftm_mem.h"


FTM_RET	FTM_CONFIG_create(FTM_CHAR_PTR pFileName, FTM_CONFIG_PTR _PTR_ ppConfig)
{
	ASSERT(pFileName != NULL);
	ASSERT(ppConfig != NULL);

	FTM_CONFIG_PTR	pConfig;

	pConfig = (FTM_CONFIG_PTR)FTM_MEM_malloc(sizeof(FTM_CONFIG));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;		
	}

	FTM_CONFIG_init(pConfig, pFileName);

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_destroy(FTM_CONFIG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	FTM_CONFIG_final(pConfig);

	FTM_MEM_free(pConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_init(FTM_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	config_init(&pConfig->xLibConfig);
	if (config_read_file(&pConfig->xLibConfig,pFileName) == CONFIG_FALSE)
	{	
		ERROR("config read file error. : %s\n", config_error_text(&pConfig->xLibConfig));
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_final(FTM_CONFIG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	config_destroy(&pConfig->xLibConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_getItem(FTM_CONFIG_PTR pConfig, FTM_CHAR_PTR pName, FTM_CONFIG_ITEM_PTR pItem)
{
	ASSERT(pConfig != NULL);
	ASSERT(pName != NULL);
	ASSERT(pItem != NULL);
	
	config_setting_t	*pSetting;

	pSetting = config_lookup(&pConfig->xLibConfig, pName);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pItem->pSetting = pSetting;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_create(FTM_CONFIG_ITEM_PTR _PTR_ ppItem)
{
	ASSERT(ppItem != NULL);
	
	FTM_CONFIG_ITEM_PTR	pItem;

	pItem = (FTM_CONFIG_ITEM_PTR)FTM_MEM_malloc(sizeof(FTM_CONFIG_ITEM));
	if (pItem == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	memset(pItem, 0, sizeof(FTM_CONFIG_ITEM));

	*ppItem = pItem;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_destroy(FTM_CONFIG_ITEM_PTR pItem)
{
	ASSERT(pItem != NULL);

	FTM_MEM_free(pItem);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getChildItem(FTM_CONFIG_ITEM_PTR pItem, FTM_CHAR_PTR pName, FTM_CONFIG_ITEM_PTR pChildItem)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pChildItem != NULL);
	
	config_setting_t	*pSetting;

	pSetting = config_setting_get_member(pItem->pSetting, pName);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pChildItem->pSetting = pSetting;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_getItemCount(FTM_CONFIG_ITEM_PTR pItem, FTM_ULONG_PTR pulCount)
{
	ASSERT(pItem != NULL);
	ASSERT(pulCount != NULL);
	
	FTM_INT				nType;

	nType = config_setting_type(pItem->pSetting);
	if ((nType != CONFIG_TYPE_ARRAY) && (nType != CONFIG_TYPE_LIST))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	*pulCount = config_setting_length(pItem->pSetting);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_getItemAt(FTM_CONFIG_ITEM_PTR pItem, FTM_ULONG ulIndex, FTM_CONFIG_ITEM_PTR pChildItem)
{
	ASSERT(pItem != NULL);
	ASSERT(pChildItem != NULL);
	
	config_setting_t	*pSetting;
	FTM_INT				nType;

	nType = config_setting_type(pItem->pSetting);
	if ((nType != CONFIG_TYPE_ARRAY) && (nType != CONFIG_TYPE_LIST))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	pSetting = config_setting_get_elem(pItem->pSetting, ulIndex);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pChildItem->pSetting = pSetting;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getItemUSHORT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_USHORT_PTR 		pusValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getUSHORT(&xChildItem, pusValue);
}

FTM_RET	FTM_CONFIG_ITEM_getItemINT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_INT_PTR 		pnValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getINT(&xChildItem, pnValue);
}

FTM_RET	FTM_CONFIG_ITEM_getItemULONG
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_ULONG_PTR 		pulValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getULONG(&xChildItem, pulValue);
}

FTM_RET	FTM_CONFIG_ITEM_getItemFLOAT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_FLOAT_PTR 		pfValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getFLOAT(&xChildItem, pfValue);
}

FTM_RET	FTM_CONFIG_ITEM_getItemBOOL
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_BOOL_PTR 		pbValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getBOOL(&xChildItem, pbValue);
}

FTM_RET	FTM_CONFIG_ITEM_getItemString
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			ulBuffLen
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getString(&xChildItem, pBuff, ulBuffLen);
}

FTM_RET	FTM_CONFIG_ITEM_getItemTime
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_TIME_PTR		pTime
)
{
	ASSERT(pItem != NULL);
	ASSERT(pTime != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getTime(&xChildItem, pTime);
}

FTM_RET	FTM_CONFIG_ITEM_getItemEPData
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_EP_DATA_PTR		pEPData
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getEPData(&xChildItem, pEPData);
}

FTM_RET	FTM_CONFIG_ITEM_getItemValue
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_VALUE_PTR		pValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getValue(&xChildItem, pValue);
}

FTM_RET	FTM_CONFIG_ITEM_getItemEPClass
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_EP_CLASS_PTR	pEPClass
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getEPClass(&xChildItem, pEPClass);
}

FTM_RET	FTM_CONFIG_ITEM_getItemNode
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_NODE_PTR 		pNode
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getNode(&xChildItem, pNode);
}

FTM_RET	FTM_CONFIG_ITEM_getUSHORT(FTM_CONFIG_ITEM_PTR pItem, FTM_USHORT_PTR pusValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pusValue != NULL);

	FTM_INT				nType;
	FTM_INT64			nValue;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch(nType)
	{
	case	CONFIG_TYPE_INT:
		{
			nValue = config_setting_get_int(pItem->pSetting);
			if (nValue < 0 || nValue > (USHRT_MAX))
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}

			*pusValue = (FTM_USHORT)nValue;
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			nValue = config_setting_get_int64(pItem->pSetting);
			if (nValue < 0 || nValue > (USHRT_MAX))
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}

			*pusValue = (FTM_USHORT)nValue;
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getINT(FTM_CONFIG_ITEM_PTR pItem, FTM_INT_PTR pnValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pnValue != NULL);

	FTM_INT				nType;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch(nType)
	{
	case	CONFIG_TYPE_INT:
		{
			*pnValue = config_setting_get_int(pItem->pSetting);
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			*pnValue = config_setting_get_int64(pItem->pSetting);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getULONG(FTM_CONFIG_ITEM_PTR pItem, FTM_ULONG_PTR pulValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pulValue != NULL);

	FTM_INT				nType;
	FTM_INT64			nValue;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch(nType)
	{
	case	CONFIG_TYPE_INT:
		{
			nValue = config_setting_get_int(pItem->pSetting);
			if ((nValue < 0) || (nValue > (ULONG_MAX)))
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}

			*pulValue = (FTM_ULONG)nValue;
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			nValue = config_setting_get_int64(pItem->pSetting);
			if ((nValue < 0) || (nValue > (ULONG_MAX)))
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}

			*pulValue = (FTM_ULONG)nValue;
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getFLOAT(FTM_CONFIG_ITEM_PTR pItem, FTM_FLOAT_PTR pfValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pfValue != NULL);

	FTM_INT				nType;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch (nType)
	{
	case	CONFIG_TYPE_INT:
		{
			*pfValue = config_setting_get_int(pItem->pSetting);
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			*pfValue = config_setting_get_int64(pItem->pSetting);
		}
		break;

	case	CONFIG_TYPE_FLOAT:
		{
			*pfValue = config_setting_get_float(pItem->pSetting);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getBOOL(FTM_CONFIG_ITEM_PTR pItem, FTM_BOOL_PTR pbValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pbValue != NULL);

	FTM_INT				nType;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch (nType)
	{
	case	CONFIG_TYPE_BOOL:
		{
			*pbValue = config_setting_get_bool(pItem->pSetting);
		}
		break;

	case	CONFIG_TYPE_INT:
		{
			*pbValue = (config_setting_get_int(pItem->pSetting) != 0);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getString(FTM_CONFIG_ITEM_PTR pItem, FTM_CHAR_PTR pBuff, FTM_ULONG ulBuffLen)
{
	ASSERT(pItem != NULL);
	ASSERT(pBuff != NULL);

	FTM_INT				nType;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch (nType)
	{
	case	CONFIG_TYPE_STRING:
		{
			char const *pString;

			pString = config_setting_get_string(pItem->pSetting);
			if ((strlen(pString) + 1) > ulBuffLen)
			{
				return	FTM_RET_CONFIG_BUFFER_TOO_SMALL;
			}

			strcpy(pBuff, pString);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;

}

FTM_RET	FTM_CONFIG_ITEM_getTime(FTM_CONFIG_ITEM_PTR pItem, FTM_TIME_PTR pTime)
{
	ASSERT(pItem != NULL);
	ASSERT(pTime != NULL);

	FTM_INT				nType;
	FTM_INT64			nValue;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	nType = config_setting_type(pItem->pSetting) ;
	switch(nType)
	{
	case	CONFIG_TYPE_INT:
		{
			nValue = config_setting_get_int(pItem->pSetting);
			if ((nValue < 0) || (nValue > (ULONG_MAX)))
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}

			pTime->xTimeval.tv_sec = nValue / 1000000;
			pTime->xTimeval.tv_usec= nValue % 1000000;
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			nValue = config_setting_get_int64(pItem->pSetting);
			if ((nValue < 0) || (nValue > (ULONG_MAX)))
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}

			pTime->xTimeval.tv_sec = nValue / 1000000;
			pTime->xTimeval.tv_usec= nValue % 1000000;
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getNode(FTM_CONFIG_ITEM_PTR pItem, FTM_NODE_PTR pNode)
{
	ASSERT(pItem != NULL);
	ASSERT(pNode != NULL);

	FTM_RET			xRet;
	FTM_NODE		xNode;
	FTM_CONFIG_ITEM	xSNMPItem;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
	
	xRet = FTM_CONFIG_ITEM_getItemString(pItem, "id", xNode.pDID, sizeof(xNode.pDID) - 1);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_getItemULONG(pItem, "type", &xNode.xType);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_getItemString(pItem,"location",	xNode.pLocation, sizeof(xNode.pLocation) - 1);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "interval", &xNode.ulInterval);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "timeout",  &xNode.ulTimeout);

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "snmp", &xSNMPItem);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM_getSNMP(&xSNMPItem, &xNode.xOption.xSNMP);
	}

	memcpy(pNode, &xNode, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getSNMP(FTM_CONFIG_ITEM_PTR pItem, FTM_NODE_OPT_SNMP_PTR pSNMP)
{
	ASSERT(pItem != NULL);
	ASSERT(pSNMP != NULL);

	FTM_NODE_OPT_SNMP	xSNMP;

	FTM_CONFIG_ITEM_getItemULONG(pItem, "version", 	&xSNMP.ulVersion);
	FTM_CONFIG_ITEM_getItemString(pItem, "url", 	xSNMP.pURL, sizeof(xSNMP.pURL) - 1);
	FTM_CONFIG_ITEM_getItemString(pItem, "community", xSNMP.pCommunity, sizeof(xSNMP.pCommunity) - 1);
	FTM_CONFIG_ITEM_getItemString(pItem, "mib", 	xSNMP.pMIB, sizeof(xSNMP.pMIB) - 1);

	memcpy(pSNMP, &xSNMP, sizeof(FTM_NODE_OPT_SNMP));
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getEPData(FTM_CONFIG_ITEM_PTR pItem, FTM_EP_DATA_PTR pEPData)
{
	ASSERT(pItem != NULL);
	ASSERT(pEPData != NULL);

	FTM_RET				xRet;
	FTM_EP_DATA			xData;
	FTM_CONFIG_ITEM		xValueItem;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
/*
 	if (config_setting_is_list(pSetting) != CONFIG_TRUE)
  	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
    }
*/
	xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "type", 	&xData.xType);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Type is not exist.\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "time", 	&xData.ulTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Type is not exist.\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "state", (FTM_ULONG_PTR)&xData.xState);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Type is not exist.\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "value", &xValueItem);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Type is not exist.\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	xRet = FTM_CONFIG_ITEM_getValue(&xValueItem, &xData.xValue);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xData.xState = FTM_EP_DATA_STATE_VALID;

	memcpy(pEPData, &xData, sizeof(FTM_EP_DATA));

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getValue
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_VALUE_PTR 		pValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pValue != NULL);

	FTM_RET				xRet;
	FTM_VALUE			xValue;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
/*
 	if (config_setting_is_list(pSetting) != CONFIG_TRUE)
  	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
    }
*/
	xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "type", &xValue.xType);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Type is not exist.\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	switch(xValue.xType)
	{
	case	FTM_VALUE_TYPE_INT:
		{
			xRet = FTM_CONFIG_ITEM_getItemINT(pItem, "value", &xValue.xValue.nValue);
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "value", &xValue.xValue.ulValue);
		}
		break;

	case	FTM_VALUE_TYPE_FLOAT:
		{
			xRet = FTM_CONFIG_ITEM_getItemFLOAT(pItem, "value", &xValue.xValue.fValue);
		}
		break;

	case	FTM_VALUE_TYPE_BOOL:
		{
			xRet = FTM_CONFIG_ITEM_getItemBOOL(pItem, "value", &xValue.xValue.bValue);
		}
		break;
	default:
		{
			ERROR("Type[%d] is not exist.\n", xValue.xType);
			return	FTM_RET_CONFIG_INVALID_OBJECT;
		}
	}

	memcpy(pValue, &xValue, sizeof(FTM_VALUE));

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getEP(FTM_CONFIG_ITEM_PTR pItem, FTM_EP_PTR pEP)
{
	ASSERT(pItem != NULL);
	ASSERT(pEP != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xLimitItem;

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_getItemString(pItem,"epid", 	pEP->pEPID, FTM_EPID_LEN);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "type",	 	(FTM_ULONG_PTR)&pEP->xType);
	FTM_CONFIG_ITEM_getItemString(pItem,"name",		pEP->pName, FTM_NAME_LEN);
	FTM_CONFIG_ITEM_getItemString(pItem,"unit",		pEP->pUnit, FTM_UNIT_LEN);
	FTM_CONFIG_ITEM_getItemBOOL(pItem, 	"enable",	&pEP->bEnable);
	FTM_CONFIG_ITEM_getItemULONG(pItem,	"interval",	&pEP->ulInterval);
	FTM_CONFIG_ITEM_getItemULONG(pItem,	"cycle",	&pEP->ulCycle);
	FTM_CONFIG_ITEM_getItemULONG(pItem,	"timeout",	&pEP->ulTimeout);

	FTM_CONFIG_ITEM_getItemString(pItem,"did",		pEP->pDID, sizeof(pEP->pDID) - 1);

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "limit", &xLimitItem);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM_getEPLimit(&xLimitItem, &pEP->xLimit);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getEPLimit(FTM_CONFIG_ITEM_PTR pItem, FTM_EP_LIMIT_PTR pLimit)
{
	ASSERT(pItem != NULL);
	ASSERT(pLimit!= NULL);

	FTM_RET			xRet;
	FTM_EP_LIMIT	xLimit;
	FTM_CONFIG_ITEM	xTypeItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "type", &xTypeItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	switch(config_setting_type(xTypeItem.pSetting))
	{
	case	CONFIG_TYPE_INT:
		{
			FTM_ULONG	ulValue;

			xRet = FTM_CONFIG_ITEM_getULONG(&xTypeItem, &ulValue);
			if (xRet != FTM_RET_OK)
			{
				return	FTM_RET_CONFIG_INVALID_VALUE;
			}

			switch(ulValue)
			{
			case	FTM_EP_LIMIT_TYPE_COUNT:
			case	FTM_EP_LIMIT_TYPE_TIME:
				{
					xLimit.xType = ulValue;
				}
				break;

			default:
				{
					return	FTM_RET_CONFIG_INVALID_VALUE;
				}
			}
		}
		break;

	case	CONFIG_TYPE_STRING:
		{
			FTM_CHAR	pType[256];

			memset(pType, 0, sizeof(pType));
			xRet = FTM_CONFIG_ITEM_getString(&xTypeItem, pType, sizeof(pType) - 1);
			if (xRet != FTM_RET_OK)
			{
				return	FTM_RET_CONFIG_INVALID_VALUE;
			}

			if (strcasecmp(pType, "count") == 0)
			{
				xLimit.xType = 	FTM_EP_LIMIT_TYPE_COUNT;
			}
			else if (strcasecmp(pType, "time") == 0)
			{
				xLimit.xType = 	FTM_EP_LIMIT_TYPE_TIME;
			}
			else
			{
				return	FTM_RET_CONFIG_INVALID_VALUE;
			}
		}
		break;
	
	default:
		{
			return	FTM_RET_CONFIG_INVALID_OBJECT;
		}
	}

	switch(xLimit.xType)
	{
	case	FTM_EP_LIMIT_TYPE_COUNT:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "count", &xLimit.xParams.ulCount);
		}
		break;

	case	FTM_EP_LIMIT_TYPE_TIME:
		{
			xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "start", &xLimit.xParams.xTime.ulStart);
			xRet = FTM_CONFIG_ITEM_getItemULONG(pItem, "end", &xLimit.xParams.xTime.ulEnd);
		}
		break;
	default:
		break;
	}

	memcpy(pLimit, &xLimit, sizeof(FTM_EP_LIMIT));
	
	return	FTM_RET_OK;
}

FTM_RET FTM_CONFIG_ITEM_getEPClass(FTM_CONFIG_ITEM_PTR pItem, FTM_EP_CLASS_PTR pEPClass)
{
	ASSERT(pItem != NULL);
	ASSERT(pEPClass!= NULL);

	if (pItem->pSetting == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_getItemULONG(pItem, "type",	 	(FTM_ULONG_PTR)&pEPClass->xType);
	FTM_CONFIG_ITEM_getItemString(pItem,"name",	 	pEPClass->pName, sizeof(pEPClass->pName) - 1);

	return	FTM_RET_OK;
}

