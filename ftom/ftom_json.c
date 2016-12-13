#include <string.h>
#include <stdlib.h>
#include "ftm.h"
#include "ftom_json.h"

FTM_RET	FTOM_JSON_createObject
(
	FTOM_JSON_PTR _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);

	cJSON _PTR_ pObject;
	
	pObject = cJSON_CreateObject();

	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	*ppObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_createArray
(
	FTOM_JSON_PTR _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);

	cJSON _PTR_ pObject;
	
	pObject = cJSON_CreateArray();

	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	*ppObject = pObject;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_destroy
(
	FTOM_JSON_PTR _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);

	cJSON_Delete(*ppObject);

	*ppObject = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_addStringToObject
(
	FTOM_JSON_PTR	pObject,
	FTM_CHAR_PTR 	pTitle,
	FTM_CHAR_PTR	pString
)
{
	ASSERT(pObject != NULL);
	ASSERT(pTitle != NULL);
	ASSERT(pString != NULL);

	cJSON_AddStringToObject(pObject, pTitle, pString);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_addNumberToObject
(
	FTOM_JSON_PTR	pObject,
	FTM_CHAR_PTR 	pTitle,
	FTM_FLOAT		fValue
)
{
	ASSERT(pObject != NULL);
	ASSERT(pTitle != NULL);

	cJSON_AddNumberToObject(pObject, pTitle, fValue);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_addItemToObject
(
	FTOM_JSON_PTR	pObject,
	FTM_CHAR_PTR 	pTitle,
	FTOM_JSON_PTR	pItem
)
{
	ASSERT(pObject != NULL);
	ASSERT(pTitle != NULL);
	ASSERT(pItem != NULL);

	cJSON_AddItemToObject(pObject, pTitle, pItem);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_addItemToArray
(
	FTOM_JSON_PTR	pObject,
	FTOM_JSON_PTR	pItem
)
{
	ASSERT(pObject != NULL);
	ASSERT(pItem != NULL);

	cJSON_AddItemToArray(pObject, pItem);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_JSON_getBufferSize
(
	FTOM_JSON_PTR	pObject,
	FTM_ULONG_PTR	pulSize
)
{
	ASSERT(pObject != NULL);
	FTM_CHAR_PTR	pPrint;

	pPrint = cJSON_Print(pObject);

	*pulSize = strlen(pPrint) + 1;

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTOM_JSON_print
(
	FTOM_JSON_PTR	pObject
)
{
	ASSERT(pObject != NULL);

	return	cJSON_Print(pObject);
}

FTM_RET	FTOM_JSON_createEPData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount,
	FTOM_JSON_PTR _PTR_	ppObject
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);
	ASSERT(ppObject != NULL);

	FTM_RET         xRet;
	FTOM_JSON_PTR	pRoot;
	FTOM_JSON_PTR	pDataArray;
	FTM_INT         i;  

	xRet = FTOM_JSON_createObject(&pRoot);
	if (xRet != FTM_RET_OK)
	{   
		return  xRet;  
	}   

	xRet = FTOM_JSON_addStringToObject(pRoot, "epid", pEPID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}

	xRet = FTOM_JSON_createArray(&pDataArray);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}


	xRet = FTOM_JSON_addItemToObject(pRoot, "data", pDataArray);
	if (xRet != FTM_RET_OK)
	{
		FTOM_JSON_destroy(&pDataArray);

		goto finished;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{   
		FTOM_JSON_PTR	pObject;
		FTM_CHAR_PTR	pValueString;

		xRet = FTOM_JSON_createObject(&pObject);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to create object!\n");
			goto finished;	
		}

		xRet = FTOM_JSON_addItemToArray(pDataArray, pObject);
		if (xRet != FTM_RET_OK)
		{
			FTOM_JSON_destroy(&pObject);
			ERROR2(xRet, "Failed to add object to array!\n");
			goto finished;	
		}

		pValueString = FTM_EP_DATA_print(&pData[i]);

		xRet = FTOM_JSON_addStringToObject(pObject, "value", pValueString);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add value string!\n");
			goto finished;	
		}

		xRet = FTOM_JSON_addNumberToObject(pObject, "time", pData[i].ulTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add time!\n");
			goto finished;	
		}
	}   

	*ppObject = pRoot;

	return	FTM_RET_OK;
finished:

	cJSON_Delete(pRoot);

	return	xRet;
}
