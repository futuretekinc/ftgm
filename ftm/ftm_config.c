#include <string.h>
#include "ftm_types.h"
#include "ftm_error.h"
#include "libconfig.h"
#include "ftm_config.h"
#include "ftm_debug.h"
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
	if (config_read_file(&pConfig->xLibConfig, pFileName) == CONFIG_FALSE)
	{
		FTM_MEM_free(pConfig);	

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

FTM_RET	FTM_CONFIG_ITEM_getInt(FTM_CONFIG_ITEM_PTR pItem, FTM_CHAR_PTR pName, FTM_INT_PTR pnValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pnValue != NULL);

	config_setting_t	*pSetting;
	FTM_INT				nType;

	pSetting = config_setting_get_member(pItem->pSetting, pName);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	nType = config_setting_type(pSetting) ;
	switch(nType)
	{
	case	CONFIG_TYPE_INT:
		{
			*pnValue = config_setting_get_int(pSetting);
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			*pnValue = config_setting_get_int64(pSetting);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getFloat(FTM_CONFIG_ITEM_PTR pItem, FTM_CHAR_PTR pName, FTM_FLOAT_PTR pfValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pfValue != NULL);

	config_setting_t	*pSetting;
	FTM_INT				nType;

	pSetting = config_setting_get_member(pItem->pSetting, pName);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	nType = config_setting_type(pSetting) ;
	switch (nType)
	{
	case	CONFIG_TYPE_INT:
		{
			*pfValue = config_setting_get_int(pSetting);
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			*pfValue = config_setting_get_int64(pSetting);
		}
		break;

	case	CONFIG_TYPE_FLOAT:
		{
			*pfValue = config_setting_get_float(pSetting);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getBool(FTM_CONFIG_ITEM_PTR pItem, FTM_CHAR_PTR pName, FTM_BOOL_PTR pbValue)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pbValue != NULL);

	config_setting_t	*pSetting;
	FTM_INT				nType;

	pSetting = config_setting_get_member(pItem->pSetting, pName);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	nType = config_setting_type(pSetting) ;
	switch (nType)
	{
	case	CONFIG_TYPE_BOOL:
		{
			*pbValue = config_setting_get_bool(pSetting);
		}
		break;

	case	CONFIG_TYPE_INT:
		{
			*pbValue = (config_setting_get_int(pSetting) != 0);
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getData(FTM_CONFIG_ITEM_PTR pItem, FTM_CHAR_PTR pName, FTM_EP_DATA_PTR pData)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pData != NULL);

	config_setting_t	*pSetting;
	config_setting_t	*pField;
	FTM_EP_DATA			xData;

	pSetting = config_setting_get_member(pItem->pSetting, pName);
	if (pSetting == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

 	if (config_setting_is_list(pSetting) != CONFIG_TRUE)
  	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
    }

	pField = config_setting_get_member(pSetting, "type");
	if (pField == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	xData.xType = config_setting_get_int(pField);

	pField = config_setting_get_member(pSetting, "value");
	if (pField == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	switch(config_setting_type(pField))
	{
	case	CONFIG_TYPE_INT:
		{
			xData.xType = FTM_EP_DATA_TYPE_INT;
			xData.xValue.nValue = config_setting_get_int(pField);
		}
		break;

	case	CONFIG_TYPE_INT64:
		{
			xData.xType = FTM_EP_DATA_TYPE_INT;
			xData.xValue.nValue = config_setting_get_int64(pField);
		}
		break;

	case	CONFIG_TYPE_FLOAT:
		{
			xData.xType = FTM_EP_DATA_TYPE_FLOAT;
			xData.xValue.nValue = config_setting_get_float(pField);
		}
		break;

	case	CONFIG_TYPE_BOOL:
		{
			xData.xType = FTM_EP_DATA_TYPE_BOOL;
			xData.xValue.nValue = config_setting_get_bool(pField);
		}
		break;
	default:
		{
			return	FTM_RET_CONFIG_INVALID_OBJECT;
		}
	}

	xData.xState = FTM_EP_DATA_STATE_VALID;

	memcpy(pData, &xData, sizeof(FTM_EP_DATA));

	return	FTM_RET_OK;
}
