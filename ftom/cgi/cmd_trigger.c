#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"

static
FTM_RET FTOM_CGI_addTriggerToObject
(
	cJSON _PTR_ pObject,
	FTM_CHAR_PTR	pName,
	FTM_TRIGGER_PTR	pTriggerInfo
);
static
FTM_RET FTOM_CGI_addTriggerToArray
(
	cJSON _PTR_ pObject,
	FTM_TRIGGER_PTR	pTriggerInfo
);
static
FTM_RET FTOM_CGI_createTriggerObject
(
	FTM_TRIGGER_PTR	pTriggerInfo,
	cJSON _PTR_ _PTR_ ppObject
);

FTM_RET	FTOM_CGI_addTrigger
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET	xRet;
	FTM_TRIGGER	xTriggerInfo;	
	FTM_EP_DATA_TYPE	xDataType;
	cJSON _PTR_	pRoot = NULL;

	pRoot = cJSON_CreateObject();

	FTM_TRIGGER_setDefault(&xTriggerInfo);

	xRet = FTOM_CGI_getTriggerType(pReq, &xTriggerInfo.xType, FTM_FALSE);
	xRet |= FTOM_CGI_getEPID(pReq, xTriggerInfo.pEPID, FTM_FALSE);
	xRet |= FTOM_CGI_getDetectTime(pReq, &xTriggerInfo.xParams.xCommon.ulDetectionTime, FTM_TRUE);
	xRet |= FTOM_CGI_getHoldTime(pReq, &xTriggerInfo.xParams.xCommon.ulHoldingTime, FTM_TRUE);

	xRet = FTOM_CLIENT_EP_DATA_type(pClient, xTriggerInfo.pEPID, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	switch(	xTriggerInfo.xType)
	{
	case	FTM_TRIGGER_TYPE_ABOVE:
	case	FTM_TRIGGER_TYPE_BELOW:
		{
			xRet = FTOM_CGI_getValue(pReq, xDataType, &xTriggerInfo.xParams.xAbove.xValue, FTM_FALSE);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}
		}
		break;

	case	FTM_TRIGGER_TYPE_INCLUDE:
	case	FTM_TRIGGER_TYPE_EXCEPT:
		{
			xRet = FTOM_CGI_getLowerValue(pReq, xDataType, &xTriggerInfo.xParams.xInclude.xLower, FTM_FALSE);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}

			xRet = FTOM_CGI_getUpperValue(pReq, xDataType, &xTriggerInfo.xParams.xInclude.xUpper, FTM_FALSE);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}
		}
		break;

	default:
		break;
	}

	xRet = FTOM_CLIENT_TRIGGER_add(pClient, &xTriggerInfo);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddNumberToObject(pRoot, "id", xTriggerInfo.xID);	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delTrigger
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_TRIGGER_ID	xTriggerID;
	cJSON _PTR_	pRoot = NULL;

	xRet = FTOM_CGI_getTriggerID(pReq, &xTriggerID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_TRIGGER_del(pClient, xTriggerID);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getTrigger
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_TRIGGER_ID	xTriggerID;
	FTM_TRIGGER		xTriggerInfo;
	cJSON _PTR_	pRoot = NULL;

	xRet = FTOM_CGI_getTriggerID(pReq, &xTriggerID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_TRIGGER_get(pClient, xTriggerID, &xTriggerInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_addTriggerToObject(pRoot, "trigger", &xTriggerInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getTriggerList
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
	cJSON _PTR_		pTriggers = NULL;

	xRet = FTOM_CLIENT_TRIGGER_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	pRoot = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, "triggers", pTriggers = cJSON_CreateArray());

	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER	xTriggerInfo;

		xRet = FTOM_CLIENT_TRIGGER_get(pClient, i, &xTriggerInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		FTOM_CGI_addTriggerToArray(pTriggers, &xTriggerInfo);
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET FTOM_CGI_addTriggerToObject
(
	cJSON _PTR_ pObject,
	FTM_CHAR_PTR	pName,
	FTM_TRIGGER_PTR	pTriggerInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pName != NULL);
	ASSERT(pTriggerInfo != NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createTriggerObject(pTriggerInfo, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToObject(pObject, pName, pNewObject);	
	}

	return	xRet;
}

FTM_RET FTOM_CGI_addTriggerToArray
(
	cJSON _PTR_ pObject,
	FTM_TRIGGER_PTR	pTriggerInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pTriggerInfo != NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createTriggerObject(pTriggerInfo, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);	
	}

	return	xRet;
}

FTM_RET FTOM_CGI_createTriggerObject
(
	FTM_TRIGGER_PTR	pTriggerInfo,
	cJSON _PTR_ _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);
	ASSERT(pTriggerInfo != NULL);

	cJSON _PTR_ pObject;
	cJSON _PTR_ pCondition;

	pObject = cJSON_CreateObject();

	cJSON_AddNumberToObject(pObject, "id", pTriggerInfo->xID);	
	cJSON_AddStringToObject(pObject, "type", FTM_TRIGGER_typeString(pTriggerInfo->xType));	
	cJSON_AddStringToObject(pObject, "epid", pTriggerInfo->pEPID);	
	cJSON_AddItemToObject(pObject, "contidion", pCondition = cJSON_CreateObject());
	cJSON_AddNumberToObject(pCondition, "detectTime", pTriggerInfo->xParams.xCommon.ulDetectionTime);
	cJSON_AddNumberToObject(pCondition, "holdTime", pTriggerInfo->xParams.xCommon.ulHoldingTime);
	switch(pTriggerInfo->xType)
	{
	case	FTM_TRIGGER_TYPE_ABOVE:
	case	FTM_TRIGGER_TYPE_BELOW:
		{
			cJSON_AddStringToObject(pCondition, "value", FTM_VALUE_print(&pTriggerInfo->xParams.xAbove.xValue));
		}
		break;

	case	FTM_TRIGGER_TYPE_INCLUDE:
	case	FTM_TRIGGER_TYPE_EXCEPT:
		{
			cJSON_AddStringToObject(pCondition, "upper", FTM_VALUE_print(&pTriggerInfo->xParams.xInclude.xUpper));
			cJSON_AddStringToObject(pCondition, "lower", FTM_VALUE_print(&pTriggerInfo->xParams.xInclude.xLower));
		}
		break;

	default:
		break;
	};

	*ppObject =pObject;

	return	FTM_RET_OK;
}
