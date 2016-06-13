#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ftm_types.h"
#include "libconfig.h"
#include "ftm_config.h"
#include "ftm_trace.h"
#include "ftm_mem.h"


FTM_RET	FTM_CONFIG_create
(
	FTM_CHAR_PTR pFileName, 
	FTM_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(pFileName != NULL);
	ASSERT(ppConfig != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_PTR	pConfig;

	pConfig = (FTM_CONFIG_PTR)FTM_MEM_malloc(sizeof(FTM_CONFIG));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;		
	}

	xRet = FTM_CONFIG_init(pConfig, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Configuration init failed!\n");
		return	xRet;
	}

	*ppConfig = pConfig;

	return	xRet;
}

FTM_RET	FTM_CONFIG_destroy
(
	FTM_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);

	FTM_CONFIG_final(*ppConfig);

	FTM_MEM_free(*ppConfig);

	*ppConfig = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_init
(
	FTM_CONFIG_PTR pConfig, 
	FTM_CHAR_PTR pFileName
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	FILE *fp;
	FTM_ULONG		ulReadSize;

	fp = fopen(pFileName, "rt");
	if (fp == NULL)
	{
		ERROR("Can't open file[%s]\n", pFileName);
		return	FTM_RET_CONFIG_LOAD_FAILED;	
	}

	fseek(fp, 0L, SEEK_END);
	pConfig->ulLen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (pConfig->pData != NULL)
	{
		FTM_MEM_free(pConfig->pData);
		pConfig->pData = NULL;
		pConfig->ulLen = 0;
	}

	pConfig->pData = (FTM_CHAR_PTR)FTM_MEM_malloc(pConfig->ulLen);
	if (pConfig->pData == NULL)
	{
		ERROR("Not enough memory[size = %d]\n", pConfig->ulLen);
		pConfig->ulLen = 0;
		fclose(fp);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	ulReadSize = fread(pConfig->pData, 1, pConfig->ulLen, fp);
	if (ulReadSize != pConfig->ulLen)
	{
		ERROR("Size missmatch[%d:%d]\n", pConfig->ulLen, ulReadSize);
		FTM_MEM_free(pConfig->pData);
		pConfig->pData = NULL;
		pConfig->ulLen = 0;
		fclose(fp);
		return	FTM_RET_CONFIG_LOAD_FAILED;	
	}

	fclose(fp);

	pConfig->pRoot = cJSON_Parse(pConfig->pData);
	if (pConfig->pRoot == NULL)
	{
		printf("File Name : %s", pFileName);
		printf("%s", pConfig->pData);
		FTM_MEM_free(pConfig->pData);
		pConfig->pData = NULL;
		pConfig->ulLen = 0;

		return	FTM_RET_CONFIG_LOAD_FAILED;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_final
(
	FTM_CONFIG_PTR pConfig
)
{
	ASSERT(pConfig != NULL);

	if (pConfig->pRoot != NULL)
	{
		cJSON_Delete(pConfig->pRoot);
		pConfig->pRoot = NULL;
	}

	if (pConfig->pData != NULL)
	{
		FTM_MEM_free(pConfig->pData);
		pConfig->pData = NULL;
		pConfig->ulLen = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_print
(
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	if (pConfig->pRoot != NULL)
	{
		FTM_CHAR_PTR	pPrintout;

		pPrintout = cJSON_Print(pConfig->pRoot);
		if (pPrintout != NULL)
		{
			MESSAGE("%s", pPrintout);
			free(pPrintout);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_getItem
(
	FTM_CONFIG_PTR pConfig, 
	FTM_CHAR_PTR pName, 
	FTM_CONFIG_ITEM_PTR pItem
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pName != NULL);
	ASSERT(pItem != NULL);

	cJSON _PTR_ pObject;

	pObject = cJSON_GetObjectItem(pConfig->pRoot, pName);
	if (pObject == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_addItem
(
	FTM_CONFIG_PTR pConfig, 
	FTM_CHAR_PTR 	pName, 
	FTM_CONFIG_ITEM_PTR pItem
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pItem != NULL);

	cJSON _PTR_ pObject;

	pObject = cJSON_CreateObject();
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToObject(pConfig->pRoot, pName, pObject);

	pItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_destroy
(
	FTM_CONFIG_ITEM_PTR pItem
)
{
	ASSERT(pItem != NULL);

	FTM_MEM_free(pItem);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getChildItem
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR pName, 
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pChildItem != NULL);

	cJSON _PTR_ pObject;

	pObject = cJSON_GetObjectItem(pItem->pObject, pName);
	if (pObject == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pChildItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_createChildItem
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR pName, 
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pChildItem != NULL);

	cJSON _PTR_ pObject;

	pObject = cJSON_CreateObject();
	if (pObject == NULL)
	{
		ERROR("JSON object creation failed!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToObject(pItem->pObject, pName, pObject);

	pChildItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_createChildList
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR pName, 
	FTM_CONFIG_ITEM_PTR pChildList
)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pChildList != NULL);

	cJSON _PTR_ pObject;

	pObject = cJSON_CreateArray();
	if (pObject == NULL)
	{
		ERROR("JSON object creation failed!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToObject(pItem->pObject, pName, pObject);

	pChildList->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_getItemCount
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_ULONG_PTR pulCount
)
{
	ASSERT(pItem != NULL);
	ASSERT(pulCount != NULL);
	
	if (pItem->pObject->type != cJSON_Array)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	*pulCount = cJSON_GetArraySize(pItem->pObject);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_getItemAt
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_ULONG ulIndex, 
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	ASSERT(pItem != NULL);
	ASSERT(pChildItem != NULL);
	
	cJSON	*pObject;

	if (pItem->pObject->type != cJSON_Array)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	pObject = cJSON_GetArrayItem(pItem->pObject, ulIndex);
	if (pObject == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pChildItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_deleteItemAt
(
	FTM_CONFIG_ITEM_PTR	pItem,
	FTM_ULONG			ulIndex
)
{
	ASSERT(pItem != NULL);

	if (pItem->pObject->type != cJSON_Array)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	cJSON_DeleteItemFromArray(pItem->pObject, ulIndex);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_addItem
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	ASSERT(pItem != NULL);
	ASSERT(pChildItem != NULL);
	
	cJSON	*pObject;

	if (pItem->pObject->type != cJSON_Array)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	pObject = cJSON_CreateObject();
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToArray(pItem->pObject, pObject);

	pChildItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_addItemNumber
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_FLOAT			fValue,
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	ASSERT(pItem != NULL);
	ASSERT(pChildItem != NULL);
	
	cJSON	*pObject;

	if (pItem->pObject->type != cJSON_Array)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	pObject = cJSON_CreateNumber(fValue);
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToArray(pItem->pObject, pObject);

	pChildItem->pObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_LIST_addItemINT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_INT				nValue,
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	return	FTM_CONFIG_LIST_addItemNumber(pItem, nValue, pChildItem);
}

FTM_RET	FTM_CONFIG_LIST_addItemUSHORT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_USHORT			usValue,
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	return	FTM_CONFIG_LIST_addItemNumber(pItem, usValue, pChildItem);
}

FTM_RET	FTM_CONFIG_LIST_addItemULONG
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_ULONG			ulValue,
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	return	FTM_CONFIG_LIST_addItemNumber(pItem, ulValue, pChildItem);
}

FTM_RET	FTM_CONFIG_LIST_addItemFLOAT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_FLOAT			fValue,
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	return	FTM_CONFIG_LIST_addItemNumber(pItem, fValue, pChildItem);
}

FTM_RET	FTM_CONFIG_LIST_addItemString
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR		pValue,
	FTM_CONFIG_ITEM_PTR pChildItem
)
{
	ASSERT(pItem != NULL);
	ASSERT(pChildItem != NULL);
	
	cJSON	*pObject;

	if (pItem->pObject->type != cJSON_Array)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	pObject = cJSON_CreateString(pValue);
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToArray(pItem->pObject, pObject);

	pChildItem->pObject = pObject;

	return	FTM_RET_OK;
}

/****************************************************************
 * get item
 ****************************************************************/
FTM_RET	FTM_CONFIG_ITEM_getItemNumber
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_FLOAT_PTR		pfValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_CONFIG_ITEM_getNumber(&xChildItem, pfValue);
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

/***********************************************************
 * set item
 ***********************************************************/
FTM_RET	FTM_CONFIG_ITEM_setItemNumber
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_FLOAT			fValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_CONFIG_ITEM_addItemNumber(pItem, pName, fValue);
	}

	if (xChildItem.pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xChildItem.pObject->valueint	= (int)fValue;
	xChildItem.pObject->valuedouble = (double)fValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setItemBOOL
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_BOOL			bValue
)
{
	return	FTM_CONFIG_ITEM_setItemNumber(pItem, pName, bValue);
}

FTM_RET	FTM_CONFIG_ITEM_setItemUSHORT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_USHORT	 		usValue
)
{
	return	FTM_CONFIG_ITEM_setItemNumber(pItem, pName, usValue);
}

FTM_RET	FTM_CONFIG_ITEM_setItemINT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_INT				nValue
)
{
	return	FTM_CONFIG_ITEM_setItemNumber(pItem, pName, nValue);
}

FTM_RET	FTM_CONFIG_ITEM_setItemULONG
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_ULONG			ulValue
)
{
	return	FTM_CONFIG_ITEM_setItemNumber(pItem, pName, ulValue);
}

FTM_RET	FTM_CONFIG_ITEM_setItemFLOAT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_FLOAT			fValue
)
{
	return	FTM_CONFIG_ITEM_setItemNumber(pItem, pName, fValue);
}

FTM_RET	FTM_CONFIG_ITEM_setItemString
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_CHAR_PTR		pValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_CONFIG_ITEM_addItemString(pItem, pName, pValue);
	}

	if (xChildItem.pObject->type != cJSON_String)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (strlen(xChildItem.pObject->valuestring) < strlen(pValue))
	{
		FTM_CHAR_PTR	pBuff;
		
		pBuff = (FTM_CHAR_PTR)malloc(strlen(pValue) + 1);
		if (pBuff == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		free(xChildItem.pObject->valuestring);
		xChildItem.pObject->valuestring = pBuff;
	}

	strcpy(xChildItem.pObject->valuestring, pValue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setItemTime
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
		return	FTM_CONFIG_ITEM_addItemTime(pItem, pName, pTime);	
	}

	return	FTM_CONFIG_ITEM_setTime(&xChildItem, pTime);
}


FTM_RET	FTM_CONFIG_ITEM_setItemValue
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

	return	FTM_CONFIG_ITEM_setValue(&xChildItem, pValue);
}

FTM_RET	FTM_CONFIG_ITEM_setItemEPData
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

	return	FTM_CONFIG_ITEM_setEPData(&xChildItem, pEPData);
}

FTM_RET	FTM_CONFIG_ITEM_setItemSNMP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR		pName,
	FTM_NODE_OPT_SNMP_PTR pSNMP
)
{
	ASSERT(pItem != NULL);
	ASSERT(pSNMP != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildItem(pItem, pName, &xChildItem);	
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	FTM_CONFIG_ITEM_setItemULONG(&xChildItem, 	"version", 	pSNMP->ulVersion);
	FTM_CONFIG_ITEM_setItemString(&xChildItem, 	"url", 		pSNMP->pURL);
	FTM_CONFIG_ITEM_setItemString(&xChildItem, 	"community",pSNMP->pCommunity);
	FTM_CONFIG_ITEM_setItemString(&xChildItem, 	"mib", 		pSNMP->pMIB);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setItemMBTCP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR		pName,
	FTM_NODE_OPT_MODBUS_OVER_TCP_PTR pMB
)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pMB != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildItem(pItem, pName, &xChildItem);	
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	return	FTM_CONFIG_ITEM_setMBTCP(&xChildItem, pMB);

}

FTM_RET	FTM_CONFIG_ITEM_setItemFINS
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR		pName,
	FTM_NODE_OPT_FINS_PTR	pFINS
)
{
	ASSERT(pItem != NULL);
	ASSERT(pName != NULL);
	ASSERT(pFINS != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, pName, &xChildItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildItem(pItem, pName, &xChildItem);	
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	return	FTM_CONFIG_ITEM_setFINS(&xChildItem, pFINS);
}


/***********************************************************
 * add item
 ***********************************************************/
FTM_RET	FTM_CONFIG_ITEM_addItemNumber
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_FLOAT			fValue
)
{
	cJSON _PTR_		pObject;

	pObject = cJSON_CreateNumber(fValue);
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToObject(pItem->pObject, pName, pObject);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_addItemBOOL
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_BOOL			bValue
)
{
	return	FTM_CONFIG_ITEM_addItemNumber(pItem, pName, bValue);
}

FTM_RET	FTM_CONFIG_ITEM_addItemUSHORT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_USHORT	 		usValue
)
{
	return	FTM_CONFIG_ITEM_addItemNumber(pItem, pName, usValue);
}

FTM_RET	FTM_CONFIG_ITEM_addItemINT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_INT				nValue
)
{
	return	FTM_CONFIG_ITEM_addItemNumber(pItem, pName, nValue);
}

FTM_RET	FTM_CONFIG_ITEM_addItemULONG
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_ULONG			ulValue
)
{
	return	FTM_CONFIG_ITEM_addItemNumber(pItem, pName, ulValue);
}

FTM_RET	FTM_CONFIG_ITEM_addItemFLOAT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_FLOAT			fValue
)
{
	return	FTM_CONFIG_ITEM_addItemNumber(pItem, pName, fValue);
}

FTM_RET	FTM_CONFIG_ITEM_addItemString
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_CHAR_PTR		pValue
)
{
	cJSON _PTR_		pObject;

	pObject = cJSON_CreateString(pValue);
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToObject(pItem->pObject, pName, pObject);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_addItemTime
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_TIME_PTR		pTime
)
{
	ASSERT(pItem != NULL);
	ASSERT(pTime != NULL);

	FTM_ULONG		ulValue;
	cJSON _PTR_		pObject;

	ulValue = pTime->xTimeval.tv_sec * 1000 + pTime->xTimeval.tv_usec / 1000;

	pObject = cJSON_CreateNumber(ulValue);
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	cJSON_AddItemToObject(pItem->pObject, pName, pObject);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_addItemValue
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR 		pName, 
	FTM_VALUE_PTR		pValue
)
{
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xChildItem;

	xChildItem.pObject = cJSON_CreateObject();
	if (xChildItem.pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_CONFIG_ITEM_setValue(&xChildItem, pValue);
	if (xRet != FTM_RET_OK)
	{
		cJSON_Delete(xChildItem.pObject);
		return	xRet;
	}

	cJSON_AddItemToObject(pItem->pObject, pName, xChildItem.pObject);

	return	FTM_RET_OK;
}
/*******************************************************
 * get value
 *******************************************************/
FTM_RET	FTM_CONFIG_ITEM_getNumber
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_FLOAT_PTR pfValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pfValue != NULL);

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	*pfValue = pItem->pObject->valuedouble;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getBOOL
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_BOOL_PTR pbValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pbValue != NULL);

	if ((pItem->pObject == NULL) || (pItem->pObject->type != cJSON_Number))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	*pbValue = (pItem->pObject->valueint != 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getUSHORT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_USHORT_PTR pusValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pusValue != NULL);

	FTM_USHORT	usValue;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	usValue = pItem->pObject->valueint;
	if (usValue < 0 || usValue > (USHRT_MAX))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	*pusValue = (FTM_USHORT)usValue;

	return	FTM_RET_OK;
}


FTM_RET	FTM_CONFIG_ITEM_getINT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_INT_PTR pnValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pnValue != NULL);

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	*pnValue = pItem->pObject->valueint;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getULONG
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_ULONG_PTR pulValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pulValue != NULL);

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	*pulValue = pItem->pObject->valueint;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getFLOAT
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_FLOAT_PTR pfValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pfValue != NULL);

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	*pfValue = pItem->pObject->valuedouble;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getString
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR pBuff, 
	FTM_ULONG ulBuffLen
)
{
	ASSERT(pItem != NULL);
	ASSERT(pBuff != NULL);

	if ((pItem->pObject == NULL) || (pItem->pObject->type != cJSON_String))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (strlen(pItem->pObject->valuestring) >= ulBuffLen)
	{
		return	FTM_RET_CONFIG_BUFFER_TOO_SMALL;
	}

	strcpy(pBuff, pItem->pObject->valuestring);

	return	FTM_RET_OK;

}

FTM_RET	FTM_CONFIG_ITEM_getTime
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_TIME_PTR pTime
)
{
	ASSERT(pItem != NULL);
	ASSERT(pTime != NULL);

	FTM_ULONG	ulValue;

	if ((pItem->pObject == NULL) || (pItem->pObject->type != cJSON_Number))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	ulValue = pItem->pObject->valueint;
	if ((ulValue < 0) || (ulValue > (ULONG_MAX)))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	pTime->xTimeval.tv_sec = ulValue / 1000;
	pTime->xTimeval.tv_usec= ulValue % 1000 * 1000000;

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

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
/*
 	if (config_setting_is_list(pObject) != CONFIG_TRUE)
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

FTM_RET	FTM_CONFIG_ITEM_getNode
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_NODE_PTR pNode
)
{
	ASSERT(pItem != NULL);
	ASSERT(pNode != NULL);

	FTM_RET			xRet;
	FTM_NODE		xNode;
	FTM_CHAR		pTypeString[128];
	FTM_CONFIG_ITEM	xSNMPItem;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	memset(&xNode, 0, sizeof(xNode));

	xRet = FTM_CONFIG_ITEM_getItemString(pItem, "id", xNode.pDID, sizeof(xNode.pDID) - 1);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node id object invalid!\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xRet = FTM_CONFIG_ITEM_getItemString(pItem, "type", pTypeString, sizeof(pTypeString));
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node type object invalid!\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xRet = FTM_CONFIG_ITEM_getItemString(pItem,"model", xNode.pModel, sizeof(xNode.pModel) - 1);
	if (xRet == FTM_RET_OBJECT_NOT_FOUND)	
	{
		ERROR("Node model object not found!\n");
		strcpy(xNode.pModel, "general");	
	}
	else if (xRet != FTM_RET_OK)
	{
		ERROR("Node model object invalid!\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xRet = FTM_NODE_strToType(pTypeString, &xNode.xType);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node type invalid!\n");
		return	xRet;	
	}
	
	FTM_CONFIG_ITEM_getItemString(pItem,"name",		xNode.pName, sizeof(xNode.pName) - 1);
	FTM_CONFIG_ITEM_getItemString(pItem,"location",	xNode.pLocation, sizeof(xNode.pLocation) - 1);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "report_interval", &xNode.ulReportInterval);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "timeout",  &xNode.ulTimeout);

	if (strcmp(pTypeString, "snmp") == 0)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "snmp", &xSNMPItem);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node type object invalid!\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;	
		}
		
		xRet = FTM_CONFIG_ITEM_getSNMP(&xSNMPItem, &xNode.xOption.xSNMP);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node type object invalid!\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;	
		}
	}
	else if (strcmp(pTypeString, "modbustcp") == 0)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "modbustcp", &xSNMPItem);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node type object invalid!\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;	
		}
		
		xRet = FTM_CONFIG_ITEM_getMBTCP(&xSNMPItem, &xNode.xOption.xMB);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node type object invalid!\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;	
		}
	}
	else if (strcmp(pTypeString, "fins") == 0)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "fins", &xSNMPItem);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node type object invalid!\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;	
		}
		
		xRet = FTM_CONFIG_ITEM_getFINS(&xSNMPItem, &xNode.xOption.xFINS);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node type object invalid!\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;	
		}
	}

	memcpy(pNode, &xNode, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getEP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_PTR pEP
)
{
	ASSERT(pItem != NULL);
	ASSERT(pEP != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pBuff[1024];
	FTM_CONFIG_ITEM	xLimitItem;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_getItemString(pItem,"epid", 	pEP->pEPID, FTM_EPID_LEN);
	FTM_CONFIG_ITEM_getItemString(pItem, "type",	pBuff, 1023);
	pEP->xType = strtoul(pBuff, 0 , 16);

	FTM_CONFIG_ITEM_getItemString(pItem,"name",		pEP->pName, FTM_NAME_LEN);
	FTM_CONFIG_ITEM_getItemString(pItem,"unit",		pEP->pUnit, FTM_UNIT_LEN);
	FTM_CONFIG_ITEM_getItemBOOL(pItem, 	"enable",	&pEP->bEnable);
	FTM_CONFIG_ITEM_getItemULONG(pItem,	"update_interval",	&pEP->ulUpdateInterval);
	FTM_CONFIG_ITEM_getItemULONG(pItem,	"report_interval",	&pEP->ulReportInterval);
	FTM_CONFIG_ITEM_getItemULONG(pItem,	"timeout",	&pEP->ulTimeout);

	FTM_CONFIG_ITEM_getItemString(pItem,"did",		pEP->pDID, sizeof(pEP->pDID) - 1);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "depid",	(FTM_ULONG_PTR)&pEP->xDEPID);

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "limit", &xLimitItem);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM_getEPLimit(&xLimitItem, &pEP->xLimit);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getEPData
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_DATA_PTR pEPData
)
{
	ASSERT(pItem != NULL);
	ASSERT(pEPData != NULL);

	FTM_RET				xRet;
	FTM_EP_DATA			xData;
	FTM_CHAR			pBuff[1024];
	FTM_CONFIG_ITEM		xValueItem;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
/*
 	if (config_setting_is_list(pObject) != CONFIG_TRUE)
  	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
    }
*/
	xRet = FTM_CONFIG_ITEM_getItemString(pItem, "type", pBuff, 1023);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Type is not exist.\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;
	}

	xData.xType = strtoul(pBuff, 0, 16);	

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

FTM_RET	FTM_CONFIG_ITEM_getEPLimit
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_LIMIT_PTR pLimit
)
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

	switch(xTypeItem.pObject->type)
	{
	case	cJSON_Number:
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

	case	cJSON_String:
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

FTM_RET FTM_CONFIG_ITEM_getEPClass
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_CLASS_PTR pEPClass
)
{
	ASSERT(pItem != NULL);
	ASSERT(pEPClass!= NULL);

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_getItemULONG(pItem, "type",	 	(FTM_ULONG_PTR)&pEPClass->xType);
	FTM_CONFIG_ITEM_getItemString(pItem,"name",	 	pEPClass->pName, sizeof(pEPClass->pName) - 1);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getSNMP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_NODE_OPT_SNMP_PTR pSNMP
)
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

FTM_RET	FTM_CONFIG_ITEM_getMBTCP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_NODE_OPT_MODBUS_OVER_TCP_PTR pMB
)
{
	ASSERT(pItem != NULL);
	ASSERT(pMB != NULL);

	FTM_NODE_OPT_MODBUS_OVER_TCP	xMB;

	FTM_CONFIG_ITEM_getItemULONG(pItem,	"version", 	&xMB.ulVersion);
	FTM_CONFIG_ITEM_getItemString(pItem,"url", 		xMB.pURL, sizeof(xMB.pURL) - 1);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "port", 	&xMB.ulPort);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "slave_id", 		&xMB.ulSlaveID);

	memcpy(pMB, &xMB, sizeof(FTM_NODE_OPT_MODBUS_OVER_TCP));
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_getFINS
(
	FTM_CONFIG_ITEM_PTR 	pItem, 
	FTM_NODE_OPT_FINS_PTR	pFINS
)
{
	ASSERT(pItem != NULL);
	ASSERT(pFINS != NULL);

	FTM_NODE_OPT_FINS	xFINS;

	FTM_CONFIG_ITEM_getItemULONG(pItem,	"version", 	&xFINS.ulVersion);
	FTM_CONFIG_ITEM_getItemString(pItem,"dip", 		xFINS.pDIP, sizeof(xFINS.pDIP) - 1);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "dport", 	&xFINS.ulDP);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "sport", 	&xFINS.ulSP);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "da", 		&xFINS.ulDA);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "sa", 		&xFINS.ulSA);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "server_id",&xFINS.ulServerID);
	FTM_CONFIG_ITEM_getItemULONG(pItem, "retry", 	&xFINS.ulRetryCount);

	memcpy(pFINS, &xFINS, sizeof(FTM_NODE_OPT_FINS));
	
	return	FTM_RET_OK;
}


/*******************************************************
 * set value
 *******************************************************/

FTM_RET	FTM_CONFIG_ITEM_setNumber
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_FLOAT			fValue
)
{
	ASSERT(pItem != NULL);

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (pItem->pObject->type != cJSON_Number)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	pItem->pObject->valueint = fValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setUSHORT
(
	FTM_CONFIG_ITEM_PTR	pItem,
	FTM_USHORT			usValue
)
{
	return	FTM_CONFIG_ITEM_setNumber(pItem, usValue);
}

FTM_RET	FTM_CONFIG_ITEM_setINT
(
	FTM_CONFIG_ITEM_PTR	pItem,
	FTM_INT				nValue
)
{
	return	FTM_CONFIG_ITEM_setNumber(pItem, nValue);
}

FTM_RET	FTM_CONFIG_ITEM_setULONG
(
	FTM_CONFIG_ITEM_PTR	pItem,
	FTM_ULONG			ulValue
)
{
	return	FTM_CONFIG_ITEM_setNumber(pItem, ulValue);
}

FTM_RET	FTM_CONFIG_ITEM_setFLOAT
(
	FTM_CONFIG_ITEM_PTR	pItem,
	FTM_FLOAT			fValue
)
{
	return	FTM_CONFIG_ITEM_setNumber(pItem, fValue);
}

FTM_RET	FTM_CONFIG_ITEM_setBOOL
(
	FTM_CONFIG_ITEM_PTR	pItem,
	FTM_BOOL			bValue
)
{
	return	FTM_CONFIG_ITEM_setNumber(pItem, bValue);
}

FTM_RET	FTM_CONFIG_ITEM_setString
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_CHAR_PTR pBuff
)
{
	ASSERT(pItem != NULL);
	ASSERT(pBuff != NULL);

	if ((pItem->pObject == NULL) || (pItem->pObject->type != cJSON_String))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	if (strlen(pItem->pObject->valuestring) < strlen(pBuff))
	{
		FTM_CHAR_PTR pNewValue = malloc(strlen(pBuff) + 1);
		if (pNewValue == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
		
		free(pItem->pObject->valuestring);
		pItem->pObject->valuestring = pNewValue;
	}

	strcpy(pItem->pObject->valuestring, pBuff);

	return	FTM_RET_OK;

}

FTM_RET	FTM_CONFIG_ITEM_setTime
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_TIME_PTR pTime
)
{
	ASSERT(pItem != NULL);
	ASSERT(pTime != NULL);

	FTM_ULONG	ulValue;

	if ((pItem->pObject == NULL) || (pItem->pObject->type != cJSON_Number))
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	ulValue = pTime->xTimeval.tv_sec * 1000 + pTime->xTimeval.tv_usec / 1000;

	pItem->pObject->valueint = ulValue;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setValue
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_VALUE_PTR 		pValue
)
{
	ASSERT(pItem != NULL);
	ASSERT(pValue != NULL);

	FTM_RET				xRet;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
/*
 	if (config_setting_is_list(pObject) != CONFIG_TRUE)
  	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
    }
*/
	xRet = FTM_CONFIG_ITEM_setItemULONG(pItem, "type", pValue->xType);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_addItemULONG(pItem, "type", pValue->xType);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can't add type object.\n");
			return	FTM_RET_CONFIG_INVALID_OBJECT;
		}
	}

	switch(pValue->xType)
	{
	case	FTM_VALUE_TYPE_INT:
		{
			xRet = FTM_CONFIG_ITEM_setItemINT(pItem, "value", pValue->xValue.nValue);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTM_CONFIG_ITEM_addItemINT(pItem, "value", pValue->xValue.nValue);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't add type object.\n");
					return	FTM_RET_CONFIG_INVALID_OBJECT;
				}
			}
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:
		{
			xRet = FTM_CONFIG_ITEM_setItemULONG(pItem, "value", pValue->xValue.ulValue);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTM_CONFIG_ITEM_addItemULONG(pItem, "value", pValue->xValue.ulValue);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't add type object.\n");
					return	FTM_RET_CONFIG_INVALID_OBJECT;
				}
			}
		}
		break;

	case	FTM_VALUE_TYPE_FLOAT:
		{
			xRet = FTM_CONFIG_ITEM_setItemFLOAT(pItem, "value", pValue->xValue.fValue);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTM_CONFIG_ITEM_addItemFLOAT(pItem, "value", pValue->xValue.fValue);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't add type object.\n");
					return	FTM_RET_CONFIG_INVALID_OBJECT;
				}
			}
		}
		break;

	case	FTM_VALUE_TYPE_BOOL:
		{
			xRet = FTM_CONFIG_ITEM_setItemBOOL(pItem, "value", pValue->xValue.bValue);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTM_CONFIG_ITEM_addItemBOOL(pItem, "value", pValue->xValue.bValue);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Can't add type object.\n");
					return	FTM_RET_CONFIG_INVALID_OBJECT;
				}
			}
		}
		break;
	default:
		{
			ERROR("Type[%d] is not exist.\n", pValue->xType);
			return	FTM_RET_CONFIG_INVALID_OBJECT;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setNode
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_NODE_PTR pNode
)
{
	ASSERT(pItem != NULL);
	ASSERT(pNode != NULL);

	FTM_RET			xRet;
	FTM_NODE		xNode;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xRet = FTM_CONFIG_ITEM_setItemString(pItem, "id", pNode->pDID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node id object invalid!\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xRet = FTM_CONFIG_ITEM_setItemString(pItem, "type", FTM_NODE_typeString(pNode->xType));
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node type object invalid!\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	xRet = FTM_CONFIG_ITEM_setItemString(pItem,"model", xNode.pModel);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node model object invalid!\n");
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_setItemString(pItem,"name",		xNode.pName);
	FTM_CONFIG_ITEM_setItemString(pItem,"location",	xNode.pLocation);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "report_interval", pNode->ulReportInterval);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "timeout",  pNode->ulTimeout);

	switch(pNode->xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			xRet = FTM_CONFIG_ITEM_setItemSNMP(pItem, "snmp", &pNode->xOption.xSNMP);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Node type object invalid!\n");
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}
		}
		break;

	case	FTM_NODE_TYPE_MODBUS_OVER_TCP:	
		{
			xRet = FTM_CONFIG_ITEM_setItemMBTCP(pItem, "modbustcp", &pNode->xOption.xMB);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Node type object invalid!\n");
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}
		}
		break;

	case	FTM_NODE_TYPE_FINS:
		{
			xRet = FTM_CONFIG_ITEM_setItemFINS(pItem, "fins", &pNode->xOption.xFINS);
			if (xRet != FTM_RET_OK)
			{
				ERROR("Node type object invalid!\n");
				return	FTM_RET_CONFIG_INVALID_OBJECT;	
			}
		}
		break;

	default:
		return	FTM_RET_CONFIG_INVALID_OBJECT;	

	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setEP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_PTR pEP
)
{
	ASSERT(pItem != NULL);
	ASSERT(pEP != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pBuff[1024];
	FTM_CONFIG_ITEM	xLimitItem;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}

	FTM_CONFIG_ITEM_setItemString(pItem,"epid", 	pEP->pEPID);
	sprintf(pBuff, "0x%08lx", pEP->xType);
	FTM_CONFIG_ITEM_setItemString(pItem, "type",	pBuff);
	FTM_CONFIG_ITEM_setItemString(pItem,"name",		pEP->pName);
	FTM_CONFIG_ITEM_setItemString(pItem,"unit",		pEP->pUnit);
	FTM_CONFIG_ITEM_setItemBOOL(pItem, 	"enable",	pEP->bEnable);
	FTM_CONFIG_ITEM_setItemULONG(pItem,	"update_interval",	pEP->ulUpdateInterval);
	FTM_CONFIG_ITEM_setItemULONG(pItem,	"report_interval",	pEP->ulReportInterval);
	FTM_CONFIG_ITEM_setItemULONG(pItem,	"timeout",	pEP->ulTimeout);

	FTM_CONFIG_ITEM_setItemString(pItem,"did",		pEP->pDID);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "depid",	pEP->xDEPID);

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "limit", &xLimitItem);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM_setEPLimit(&xLimitItem, &pEP->xLimit);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setEPData
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_DATA_PTR pEPData
)
{
	ASSERT(pItem != NULL);
	ASSERT(pEPData != NULL);

	FTM_RET				xRet;
	FTM_CHAR			pBuff[1024];
	FTM_CONFIG_ITEM		xValueItem;

	if (pItem->pObject == NULL)
	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;	
	}
/*
 	if (config_setting_is_list(pObject) != CONFIG_TRUE)
  	{
		return	FTM_RET_CONFIG_INVALID_OBJECT;
    }
*/
	sprintf(pBuff, "0x%08lx", pEPData->xType);
	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "type", &xValueItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_addItemString(pItem, "type", pBuff);
		if (xRet != FTM_RET_OK)
		{
			ERROR("New item creation failed!\n");	
		}
	}
	else
	{
		FTM_CONFIG_ITEM_setString(&xValueItem, pBuff);
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem,  "time", &xValueItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_addItemULONG(pItem, "time", pEPData->ulTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR("New item creation failed!\n");	
		}
	}
	else
	{
		FTM_CONFIG_ITEM_setULONG(&xValueItem, pEPData->ulTime);
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem,  "state", &xValueItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_addItemULONG(pItem, "state", pEPData->xState);
		if (xRet != FTM_RET_OK)
		{
			ERROR("New item creation failed!\n");	
		}
	}
	else
	{
		FTM_CONFIG_ITEM_setULONG(&xValueItem, pEPData->xState);
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem,  "value", &xValueItem);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_addItemValue(pItem, "value", &pEPData->xValue);
		if (xRet != FTM_RET_OK)
		{
			ERROR("New item creation failed!\n");	
		}
	}
	else
	{
		FTM_CONFIG_ITEM_setValue(&xValueItem, &pEPData->xValue);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setEPLimit
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_EP_LIMIT_PTR pLimit
)
{
	ASSERT(pItem != NULL);
	ASSERT(pLimit!= NULL);

	FTM_RET			xRet;
	FTM_EP_LIMIT	xLimit;
	FTM_CONFIG_ITEM	xTypeItem;

	xRet = FTM_CONFIG_ITEM_getChildItem(pItem, "type", &xTypeItem);
	if (xRet != FTM_RET_OK)
	{
		if (pLimit->xType == FTM_EP_LIMIT_TYPE_COUNT)
		{
			FTM_CONFIG_ITEM_setItemString(pItem, "type", "count");
		}
		else if (pLimit->xType == FTM_EP_LIMIT_TYPE_TIME)
		{
			FTM_CONFIG_ITEM_setItemString(pItem, "type", "time");
		}
		else
		{
			return	FTM_RET_CONFIG_INVALID_OBJECT;
		}
	}
	else
	{
		switch(xTypeItem.pObject->type)
		{
		case	cJSON_Number:
			{
				xTypeItem.pObject->valueint = pLimit->xType;
			}
			break;

		case	cJSON_String:
			{
				FTM_CHAR	pValue[32];

				switch(pLimit->xType)
				{
				case	FTM_EP_LIMIT_TYPE_COUNT:
					{
						strcpy(pValue, "count");
					}
					break;

				case	FTM_EP_LIMIT_TYPE_TIME:
					{
						strcpy(pValue, "time");
					}
					break;

				default:
					{
						return	FTM_RET_CONFIG_INVALID_OBJECT;
					}
				}

				if (strlen(xTypeItem.pObject->valuestring) < strlen(pValue))
				{
					FTM_CHAR_PTR	pBuff = (FTM_CHAR_PTR)malloc(strlen(pValue) + 1);
					if (pBuff == NULL)
					{
						return	FTM_RET_NOT_ENOUGH_MEMORY;	
					}

					free(xTypeItem.pObject->valuestring) ;
					xTypeItem.pObject->valuestring = pBuff;

				}

				strcpy(xTypeItem.pObject->valuestring, pValue);
			}
			break;
		
		default:
			{
				return	FTM_RET_CONFIG_INVALID_OBJECT;
			}
		}
	}

	switch(xLimit.xType)
	{
	case	FTM_EP_LIMIT_TYPE_COUNT:
		{
			xRet = FTM_CONFIG_ITEM_setItemULONG(pItem, "count", pLimit->xParams.ulCount);
		}
		break;

	case	FTM_EP_LIMIT_TYPE_TIME:
		{
			xRet = FTM_CONFIG_ITEM_setItemULONG(pItem, "start", pLimit->xParams.xTime.ulStart);
			xRet = FTM_CONFIG_ITEM_setItemULONG(pItem, "end", 	pLimit->xParams.xTime.ulEnd);
		}
		break;

	default:
		break;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setSNMP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_NODE_OPT_SNMP_PTR pSNMP
)
{
	ASSERT(pItem != NULL);
	ASSERT(pSNMP != NULL);

	FTM_CONFIG_ITEM_setItemULONG(pItem, "version", 	pSNMP->ulVersion);
	FTM_CONFIG_ITEM_setItemString(pItem, "url", 	pSNMP->pURL);
	FTM_CONFIG_ITEM_setItemString(pItem, "community", pSNMP->pCommunity);
	FTM_CONFIG_ITEM_setItemString(pItem, "mib", 	pSNMP->pMIB);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setMBTCP
(
	FTM_CONFIG_ITEM_PTR pItem, 
	FTM_NODE_OPT_MODBUS_OVER_TCP_PTR pMB
)
{
	ASSERT(pItem != NULL);
	ASSERT(pMB != NULL);

	FTM_CONFIG_ITEM_setItemULONG(pItem,	"version", 	pMB->ulVersion);
	FTM_CONFIG_ITEM_setItemString(pItem,"url", 		pMB->pURL);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "port", 	pMB->ulPort);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "slave_id", pMB->ulSlaveID);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_ITEM_setFINS
(
	FTM_CONFIG_ITEM_PTR 	pItem, 
	FTM_NODE_OPT_FINS_PTR	pFINS
)
{
	ASSERT(pItem != NULL);
	ASSERT(pFINS != NULL);

	FTM_CONFIG_ITEM_setItemULONG(pItem,	"version", 	pFINS->ulVersion);
	FTM_CONFIG_ITEM_setItemString(pItem,"dip", 		pFINS->pDIP);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "dport", 	pFINS->ulDP);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "sport", 	pFINS->ulSP);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "da", 		pFINS->ulDA);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "sa", 		pFINS->ulSA);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "server_id",pFINS->ulServerID);
	FTM_CONFIG_ITEM_setItemULONG(pItem, "retry", 	pFINS->ulRetryCount);

	return	FTM_RET_OK;
}

