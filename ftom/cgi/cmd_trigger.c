#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"

FTM_RET	FTOM_CGI_addTrigger
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET	xRet;
	FTM_CHAR_PTR pParam;
	FTM_TRIGGER	xTriggerInfo;	
	FTM_EP_DATA_TYPE	xDataType;

	pParam = pReq->getstr(pReq, "type", false);
	if (pParam == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;			
	}
	xTriggerInfo.xType = strtoul(pParam, 0, 10);

	pParam = pReq->getstr(pReq, "epid", false);
	if ((pParam == NULL) || (strlen(pParam) > FTM_EPID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;			
	}
	strncpy(xTriggerInfo.pEPID, pParam, FTM_EPID_LEN);

	xRet = FTOM_CLIENT_EP_DATA_type(pClient, xTriggerInfo.pEPID, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	pParam = pReq->getstr(pReq, "detect", false);
	if (pParam != NULL)
	{
		xTriggerInfo.xParams.xCommon.ulDetectionTime = strtoul(pParam, 0, 10);
	}

	pParam = pReq->getstr(pReq, "hold", false);
	if (pParam != NULL)
	{
		xTriggerInfo.xParams.xCommon.ulHoldingTime = strtoul(pParam, 0, 10);
	}

	switch(	xTriggerInfo.xType)
	{
	case	FTM_TRIGGER_TYPE_ABOVE:
	case	FTM_TRIGGER_TYPE_BELOW:
		{
			pParam = pReq->getstr(pReq, "value", false);
			if (pParam == NULL)
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}

			FTM_VALUE_init(&xTriggerInfo.xParams.xAbove.xValue, xDataType, pParam);
		}
		break;

	case	FTM_TRIGGER_TYPE_INCLUDE:
	case	FTM_TRIGGER_TYPE_EXCEPT:
		{
			FTM_CHAR_PTR	pLower, pUpper;
			pLower = pReq->getstr(pReq, "lower", false);
			pUpper = pReq->getstr(pReq, "upper", false);
		
			if ((pLower == NULL) || (pUpper == NULL))
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}

			FTM_VALUE_init(&xTriggerInfo.xParams.xInclude.xLower, xDataType, pLower);
			FTM_VALUE_init(&xTriggerInfo.xParams.xInclude.xUpper, xDataType, pUpper);
		}
		break;

	default:
		break;
	}

	pParam = pReq->getstr(pReq, "id", false);
	if (pParam != NULL)
	{
		xTriggerInfo.xID = strtoul(pParam, 0, 10);		
	}


	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_delTrigger
(
	FTOM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	return	FTM_RET_OK;
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
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_ULONG		ulCount = 0;
	cJSON _PTR_		pRoot = NULL;

	xRet = FTOM_CLIENT_TRIGGER_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	pRoot = cJSON_CreateObject();
	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER	xTriggerInfo;
		cJSON 	_PTR_ pCondition;

		xRet = FTOM_CLIENT_TRIGGER_get(pClient, i, &xTriggerInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		cJSON_AddNumberToObject(pRoot, "ID", xTriggerInfo.xID);	
		cJSON_AddStringToObject(pRoot, "TYPE", FTM_TRIGGER_typeString(xTriggerInfo.xType));	
		cJSON_AddStringToObject(pRoot, "EPID", xTriggerInfo.pEPID);	
		cJSON_AddItemToObject(pRoot, "CONTIDION", pCondition = cJSON_CreateObject());
		cJSON_AddNumberToObject(pCondition, "DETECT_TIME", xTriggerInfo.xParams.xCommon.ulDetectionTime);
		cJSON_AddNumberToObject(pCondition, "HOLD_TIME", xTriggerInfo.xParams.xCommon.ulHoldingTime);
		switch(xTriggerInfo.xType)
		{
		case	FTM_TRIGGER_TYPE_ABOVE:
		case	FTM_TRIGGER_TYPE_BELOW:
			{
				cJSON_AddStringToObject(pCondition, "VALUE", FTM_VALUE_print(&xTriggerInfo.xParams.xAbove.xValue));
			}
			break;

		case	FTM_TRIGGER_TYPE_INCLUDE:
		case	FTM_TRIGGER_TYPE_EXCEPT:
			{
				cJSON_AddStringToObject(pCondition, "UPPER", FTM_VALUE_print(&xTriggerInfo.xParams.xInclude.xUpper));
				cJSON_AddStringToObject(pCondition, "LOWER", FTM_VALUE_print(&xTriggerInfo.xParams.xInclude.xLower));
			}
			break;

		case	FTM_TRIGGER_TYPE_CHANGE:
			break;
		};
	}

	pBuff = cJSON_Print(pRoot);

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);
	TRACE("%s", pBuff);
	xRet = FTM_RET_OK;

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	xRet;
}
