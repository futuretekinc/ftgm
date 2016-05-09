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
	FTM_TRIGGER_FIELD	xFiels,
	FTM_TRIGGER_PTR	pTriggerInfo
);
static
FTM_RET FTOM_CGI_createTriggerObject
(
	FTM_TRIGGER_PTR	pTriggerInfo,
	FTM_TRIGGER_FIELD	xFiels,
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
		cJSON_AddStringToObject(pRoot, "id", xTriggerInfo.pID);	
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
	FTM_CHAR		pTriggerID[FTM_ID_LEN+1];
	cJSON _PTR_	pRoot = NULL;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getTriggerID(pReq, pTriggerID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_TRIGGER_del(pClient, pTriggerID);
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
	FTM_CHAR		pTriggerID[FTM_ID_LEN+1];
	FTM_TRIGGER		xTriggerInfo;
	cJSON _PTR_	pRoot = NULL;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getTriggerID(pReq, pTriggerID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_TRIGGER_get(pClient, pTriggerID, &xTriggerInfo);
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

FTM_RET	FTOM_CGI_setTrigger
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pTriggerID[FTM_ID_LEN+1];
	FTM_TRIGGER		xInfo;
	FTM_TRIGGER_FIELD	xFields = 0;
	cJSON _PTR_	pRoot = NULL;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getTriggerID(pReq, pTriggerID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_TRIGGER_get(pClient, pTriggerID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getName(pReq, xInfo.pName, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_TRIGGER_FIELD_NAME;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet |= FTOM_CGI_getDetectTime(pReq, &xInfo.xParams.xCommon.ulDetectionTime, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_TRIGGER_FIELD_DETECT_TIME;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet |= FTOM_CGI_getHoldTime(pReq, &xInfo.xParams.xCommon.ulHoldingTime, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_TRIGGER_FIELD_HOLD_TIME;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	switch(xInfo.xType)
	{
	case FTM_TRIGGER_TYPE_ABOVE:
	case FTM_TRIGGER_TYPE_BELOW:
		{
			xRet |= FTOM_CGI_getValue(pReq, 
						xInfo.xParams.xAbove.xValue.xType, 
						&xInfo.xParams.xAbove.xValue, FTM_FALSE);
			if (xRet == FTM_RET_OK)
			{
				xFields |= FTM_TRIGGER_FIELD_VALUE;
			}
			else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
			{
				goto finish;
			}
		}
		break;
	
	case FTM_TRIGGER_TYPE_INCLUDE:
	case FTM_TRIGGER_TYPE_EXCEPT:
		{
			xRet |= FTOM_CGI_getLowerValue(pReq, 
						xInfo.xParams.xInclude.xLower.xType, 
						&xInfo.xParams.xInclude.xLower, FTM_FALSE);
			if (xRet == FTM_RET_OK)
			{
				xFields |= FTM_TRIGGER_FIELD_LOWER;
			}
			else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
			{
				goto finish;
			}
		
			xRet |= FTOM_CGI_getUpperValue(pReq, 
						xInfo.xParams.xInclude.xUpper.xType, 
						&xInfo.xParams.xInclude.xUpper, FTM_FALSE);
			if (xRet == FTM_RET_OK)
			{
				xFields |= FTM_TRIGGER_FIELD_UPPER;
			}
			else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
			{
				goto finish;
			}
		}
		break;

		default:
		{
		}
	}

	xRet = FTOM_CLIENT_TRIGGER_set(pClient, pTriggerID, xFields, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_addTriggerToObject(pRoot, "trigger", &xInfo);
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

	FTM_RET				xRet;
	FTM_INT				i;
	FTM_ULONG			ulCount = 0;
	FTM_CHAR_PTR		pValue = NULL;
	FTM_TRIGGER_FIELD	xFields = FTM_TRIGGER_FIELD_ID;
	cJSON _PTR_			pRoot = NULL;
	cJSON _PTR_			pTriggers = NULL;

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
			xFields |= FTM_TRIGGER_FIELD_ALL;
			break;
		}
		else if (strcasecmp(pValue, "type") == 0)
		{
			xFields |= FTM_TRIGGER_FIELD_TYPE;	
		}
		else if (strcasecmp(pValue, "name") == 0)
		{
			xFields |= FTM_TRIGGER_FIELD_NAME;	
		}
		else if (strcasecmp(pValue, "epid") == 0)
		{
			xFields |= FTM_TRIGGER_FIELD_EPID;	
		}
		else if (strcasecmp(pValue, "condition") == 0)
		{
			xFields |= FTM_TRIGGER_FIELD_CONDITION;	
		}
		else
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	xRet = FTOM_CLIENT_TRIGGER_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON_AddItemToObject(pRoot, "triggers", pTriggers = cJSON_CreateArray());

	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER	xTriggerInfo;

		xRet = FTOM_CLIENT_TRIGGER_getAt(pClient, i, &xTriggerInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		FTOM_CGI_addTriggerToArray(pTriggers, xFields, &xTriggerInfo);
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

	xRet = FTOM_CGI_createTriggerObject(pTriggerInfo, FTM_TRIGGER_FIELD_ALL, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToObject(pObject, pName, pNewObject);	
	}

	return	xRet;
}

FTM_RET FTOM_CGI_addTriggerToArray
(
	cJSON _PTR_ pObject,
	FTM_TRIGGER_FIELD xFields,
	FTM_TRIGGER_PTR	pTriggerInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pTriggerInfo != NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createTriggerObject(pTriggerInfo, xFields, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);	
	}

	return	xRet;
}

FTM_RET FTOM_CGI_createTriggerObject
(
	FTM_TRIGGER_PTR	pTriggerInfo,
	FTM_TRIGGER_FIELD	xFields,
	cJSON _PTR_ _PTR_ ppObject
)
{
	ASSERT(ppObject != NULL);
	ASSERT(pTriggerInfo != NULL);

	cJSON _PTR_ pObject;
	cJSON _PTR_ pCondition;

	pObject = cJSON_CreateObject();

	if (xFields & FTM_TRIGGER_FIELD_ID)
	{
		cJSON_AddStringToObject(pObject, "id", pTriggerInfo->pID);	
	}

	if (xFields & FTM_TRIGGER_FIELD_TYPE)
	{
		cJSON_AddStringToObject(pObject, "type", FTM_TRIGGER_typeString(pTriggerInfo->xType));	
	}

	if (xFields & FTM_TRIGGER_FIELD_NAME)
	{
		cJSON_AddStringToObject(pObject, "name", pTriggerInfo->pName);	
	}

	if (xFields & FTM_TRIGGER_FIELD_EPID)
	{
	cJSON_AddStringToObject(pObject, "epid", pTriggerInfo->pEPID);	
	}

	if (xFields & FTM_TRIGGER_FIELD_CONDITION)
	{
		cJSON_AddItemToObject(pObject, "contidion", pCondition = cJSON_CreateObject());
		cJSON_AddNumberToObject(pCondition, "detectTime", pTriggerInfo->xParams.xCommon.ulDetectionTime);
		cJSON_AddNumberToObject(pCondition, "holdTime", pTriggerInfo->xParams.xCommon.ulHoldingTime);
		switch(pTriggerInfo->xType)
		{
		case	FTM_TRIGGER_TYPE_ABOVE:
			{
				cJSON_AddStringToObject(pCondition, "value", FTM_VALUE_print(&pTriggerInfo->xParams.xAbove.xValue));
			}
			break;
	
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
	}

	*ppObject =pObject;

	return	FTM_RET_OK;
}
