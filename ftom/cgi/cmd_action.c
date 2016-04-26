#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"

static
FTM_RET	FTOM_CGI_addActionToObject
(
	cJSON _PTR_ pObject,
	FTM_CHAR_PTR	pName,
	FTM_ACTION_PTR pActionInfo
);

static
FTM_RET	FTOM_CGI_addActionToArray
(
	cJSON _PTR_ pObject,
	FTM_ACTION_PTR pActionInfo
);

static
FTM_RET	FTOM_CGI_createActionObject
(
	FTM_ACTION_PTR pActionInfo,
	cJSON _PTR_ _PTR_ ppObject
);

FTM_RET	FTOM_CGI_addAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ACTION		xActionInfo;
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();

	FTM_ACTION_setDefault(&xActionInfo);

	xRet = FTOM_CGI_getActionID(pReq, &xActionInfo.xID, FTM_TRUE);
	xRet |= FTOM_CGI_getActionType(pReq, &xActionInfo.xType, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	switch(xActionInfo.xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			FTM_EP_DATA_TYPE	xType;

			xRet = FTOM_CGI_getEPID(pReq, xActionInfo.xParams.xSet.pEPID, FTM_FALSE);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}

			xRet = FTOM_CLIENT_EP_DATA_type(pClient, xActionInfo.xParams.xSet.pEPID, &xType);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}

			xRet = FTOM_CGI_getValue(pReq, xType, &xActionInfo.xParams.xSet.xValue, FTM_FALSE);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}


	xRet = FTOM_CLIENT_ACTION_add(pClient, &xActionInfo);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddNumberToObject(pRoot, "id", xActionInfo.xID);
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	cJSON _PTR_		pRoot = NULL;
	FTM_ACTION_ID	xID;

	pRoot = cJSON_CreateObject();

	xRet =FTOM_CGI_getActionID(pReq, &xID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	FTOM_CLIENT_ACTION_del(pClient, xID);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ACTION_ID	xID;
	FTM_ACTION		xActionInfo;
	cJSON _PTR_		pRoot = NULL;
	
	pRoot = cJSON_CreateObject();
	
	xRet = FTOM_CGI_getActionID(pReq, &xID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_ACTION_get(pClient, xID, &xActionInfo);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_CGI_addActionToObject(pRoot, "action", &xActionInfo);	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getActionList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount = 0;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pActionList = NULL;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CLIENT_ACTION_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON_AddItemToObject(pRoot, "actions", pActionList = cJSON_CreateArray());

	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_ACTION	xActionInfo;

		xRet = FTOM_CLIENT_ACTION_get(pClient, i, &xActionInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		FTOM_CGI_addActionToArray(pActionList, &xActionInfo);
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_addActionToObject
(
	cJSON _PTR_ pObject,
	FTM_CHAR_PTR	pName,
	FTM_ACTION_PTR pActionInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pName != NULL);
	ASSERT(pActionInfo != NULL);
	
	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createActionObject(pActionInfo, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToObject(pObject, pName, pNewObject);
	}

	return	xRet;
}
FTM_RET	FTOM_CGI_addActionToArray
(
	cJSON _PTR_ pObject,
	FTM_ACTION_PTR pActionInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pActionInfo != NULL);
	
	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createActionObject(pActionInfo, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_createActionObject
(
	FTM_ACTION_PTR pActionInfo,
	cJSON _PTR_ _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);
	ASSERT(pActionInfo != NULL);

	cJSON _PTR_ pObject;
	cJSON _PTR_ pAction ;
	
	pObject = cJSON_CreateObject();

	cJSON_AddNumberToObject(pObject, "id", pActionInfo->xID);	
	cJSON_AddStringToObject(pObject, "type", FTM_ACTION_typeString(pActionInfo->xType));	
	cJSON_AddItemToObject(pObject, "action", pAction = cJSON_CreateObject());
	switch(pActionInfo->xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			cJSON_AddStringToObject(pAction, "epid", pActionInfo->xParams.xSet.pEPID);
			cJSON_AddStringToObject(pAction, "value", FTM_VALUE_print(&pActionInfo->xParams.xSet.xValue));
		}
		break;
	
	default:
	break;
	};

	*ppObject = pObject;

	return	FTM_RET_OK;
}
