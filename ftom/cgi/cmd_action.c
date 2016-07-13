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
	FTM_ACTION_FIELD xFields,
	FTM_ACTION_PTR pActionInfo
);

static
FTM_RET	FTOM_CGI_createActionObject
(
	FTM_ACTION_PTR pActionInfo,
	FTM_ACTION_FIELD xFields,
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
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();

	FTM_ACTION_setDefault(&xActionInfo);

	xRet = FTOM_CGI_getActionID(pReq, xActionInfo.pID, FTM_TRUE);
	xRet |= FTOM_CGI_getActionType(pReq, &xActionInfo.xType, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getName(pReq, xActionInfo.pName, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	switch(xActionInfo.xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			FTM_VALUE_TYPE	xType;

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


	memset(pActionID, 0, sizeof(pActionID));
	xRet = FTOM_CLIENT_ACTION_add(pClient, &xActionInfo, pActionID, FTM_ID_LEN);
	TRACE("Action ID : %s\n", pActionID);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_CLIENT_ACTION_get(pClient, pActionID, &xActionInfo);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_CGI_addActionToObject(pRoot, "action", &xActionInfo);	
		}
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
	FTM_CHAR		pActionID[FTM_ID_LEN+1];

	pRoot = cJSON_CreateObject();

	xRet =FTOM_CGI_getActionID(pReq, pActionID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	FTOM_CLIENT_ACTION_del(pClient, pActionID);

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
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	FTM_ACTION		xActionInfo;
	cJSON _PTR_		pRoot = NULL;
	
	pRoot = cJSON_CreateObject();
	
	xRet = FTOM_CGI_getActionID(pReq, pActionID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_ACTION_get(pClient, pActionID, &xActionInfo);
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

	FTM_RET				xRet;
	FTM_INT				i;
	FTM_ULONG			ulCount = 0;
	FTM_CHAR_PTR		pValue = NULL;
	FTM_ACTION_FIELD	xFields = FTM_ACTION_FIELD_ID;
	cJSON _PTR_			pRoot = NULL;
	cJSON _PTR_			pActionList = NULL;

	pRoot = cJSON_CreateObject();

	for(i = 0 ; ; i++)
	{
		FTM_CHAR	pTitle[32];

		sprintf(pTitle,"field%d", i+1);

		pValue = pReq->getstr(pReq, pTitle, false);
		if (pValue == NULL)
		{
			break;	
		}

		if (strcasecmp(pValue, "all") == 0)
		{
			xFields = FTM_ACTION_FIELD_ALL;
			break;
		}
		else if (strcasecmp(pValue, "type") == 0)
		{
			xFields |= FTM_ACTION_FIELD_TYPE;	
		}
		else if (strcasecmp(pValue, "name") == 0)
		{
			xFields |= FTM_ACTION_FIELD_NAME;	
		}
		else if (strcasecmp(pValue, "action") == 0)
		{
			xFields |= FTM_ACTION_FIELD_ACTION;	
		}
		else
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	xRet = FTOM_CLIENT_ACTION_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON_AddItemToObject(pRoot, "actions", pActionList = cJSON_CreateArray());

	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_ACTION	xActionInfo;

		xRet = FTOM_CLIENT_ACTION_getAt(pClient, i, &xActionInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		FTOM_CGI_addActionToArray(pActionList, xFields, &xActionInfo);
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_setAction
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pActionID[FTM_ID_LEN+1];
	FTM_ACTION		xActionInfo;
	FTM_ACTION_FIELD xFields = 0;
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();


	xRet = FTOM_CGI_getActionID(pReq, pActionID, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_ACTION_get(pClient, pActionID, &xActionInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getName(pReq, xActionInfo.pName, FTM_TRUE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_ACTION_FIELD_NAME;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	switch(xActionInfo.xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			FTM_VALUE_TYPE	xType;

			xRet = FTOM_CLIENT_EP_DATA_type(pClient, xActionInfo.xParams.xSet.pEPID, &xType);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}

			xRet = FTOM_CGI_getValue(pReq, xType, &xActionInfo.xParams.xSet.xValue, FTM_TRUE);
			if (xRet == FTM_RET_OK)
			{
				xFields |= FTM_ACTION_FIELD_VALUE;
			}
			else if (xRet == FTM_RET_OBJECT_NOT_FOUND)
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


	xRet = FTOM_CLIENT_ACTION_set(pClient, pActionID, xFields, &xActionInfo);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_CGI_addActionToObject(pRoot, "action", &xActionInfo);	
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

	xRet = FTOM_CGI_createActionObject(pActionInfo, FTM_ACTION_FIELD_ALL,  &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToObject(pObject, pName, pNewObject);
	}

	return	xRet;
}
FTM_RET	FTOM_CGI_addActionToArray
(
	cJSON _PTR_ pObject,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR pActionInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pActionInfo != NULL);
	
	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createActionObject(pActionInfo, xFields, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_createActionObject
(
	FTM_ACTION_PTR pActionInfo,
	FTM_ACTION_FIELD xFields,
	cJSON _PTR_ _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);
	ASSERT(pActionInfo != NULL);

	cJSON _PTR_ pObject;
	cJSON _PTR_ pAction ;
	
	pObject = cJSON_CreateObject();

	if (xFields & FTM_ACTION_FIELD_ID)
	{
		cJSON_AddStringToObject(pObject, "id", pActionInfo->pID);	
	}

	if (xFields & FTM_ACTION_FIELD_TYPE)
	{
		cJSON_AddStringToObject(pObject, "type", FTM_ACTION_typeString(pActionInfo->xType));	
	}

	if (xFields & FTM_ACTION_FIELD_NAME)
	{
		cJSON_AddStringToObject(pObject, "name", pActionInfo->pName);
	}

	if (xFields & FTM_ACTION_FIELD_ACTION)
	{
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
	}

	*ppObject = pObject;

	return	FTM_RET_OK;
}
