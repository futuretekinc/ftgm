#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "ftm_rule.h"
#include "cJSON.h"

static
FTM_RET	FTOM_CGI_addLogToArray
(
	cJSON _PTR_ 	pObject,
	FTM_LOG_FIELD	xFields,
	FTM_LOG_PTR		pLog
);

static
FTM_RET	FTOM_CGI_createLogObject
(
	FTM_LOG_PTR		pLog,
	FTM_LOG_FIELD	xFields,
	cJSON _PTR_ _PTR_ ppObject
);

FTM_RET	FTOM_CGI_getLog
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulIndex = 0;
	FTM_ULONG		ulCount = 0;
	FTM_ULONG		ulGetCount = 0;
	FTM_ULONG		i;
	FTM_LOG_PTR		pLogs = NULL;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pLogArray = NULL;

	pRoot = cJSON_CreateObject();
	
	xRet = FTOM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	
	if (ulCount > 100)
	{
		xRet = FTM_RET_DATA_IS_TOO_LARGE;
		goto finish;
	}
	else if (ulCount == 0)
	{
		goto finish;	
	}

	pLogs = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulCount);
	if (pLogs == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		goto finish;
	}

	xRet = FTOM_CLIENT_LOG_getList(pClient, ulIndex, ulCount , pLogs, &ulGetCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	cJSON_AddItemToObject(pRoot, "logs", pLogArray = cJSON_CreateArray());
	for(i = 0 ; i < ulGetCount ; i++)
	{
		xRet = FTOM_CGI_addLogToArray(pLogArray, FTM_LOG_FIELD_ALL, &pLogs[i]);
	}

finish:

	if (pLogs !=NULL)
	{
		FTM_MEM_free(pLogs);
		pLogs = NULL;
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delLog
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulIndex = 0;
	FTM_ULONG		ulCount = 0;
	FTM_ULONG		ulDeletedCount = 0;
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();
	
	xRet = FTOM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_LOG_del(pClient, ulIndex, ulCount, &ulDeletedCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	cJSON_AddItemToObject(pRoot, "count", cJSON_CreateNumber(ulDeletedCount));
finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_addLogToArray
(
	cJSON _PTR_ pObject,
	FTM_LOG_FIELD	xFields,
	FTM_LOG_PTR	pLogInfo
)

{
	ASSERT(pObject != NULL);
	ASSERT(pLogInfo != NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createLogObject(pLogInfo, xFields, &pNewObject);
	if(xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_createLogObject
(
	FTM_LOG_PTR	pLog,
	FTM_LOG_FIELD	xFields,
	cJSON _PTR_ _PTR_ ppObject
)
{
	ASSERT(pLog != NULL);
	ASSERT(ppObject != NULL);

	cJSON _PTR_ pObject;

	pObject = cJSON_CreateObject();

	if (xFields & FTM_LOG_FIELD_ID)
	{
		cJSON_AddNumberToObject(pObject, "id", pLog->ullID);	
	}

	if (xFields & FTM_LOG_FIELD_TIME)
	{
		cJSON_AddNumberToObject(pObject, "time", pLog->ulTime);
	}

	if (xFields & FTM_LOG_FIELD_VALUE)
	{
		cJSON_AddStringToObject(pObject, "value", FTM_LOG_print(pLog));

	}

	*ppObject = pObject;

	return	FTM_RET_OK;
}
