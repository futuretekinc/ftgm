#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "ftm_rule.h"
#include "cJSON.h"

static
FTM_RET	FTOM_CGI_addRuleToObject
(
	cJSON _PTR_ pObject,
	FTM_CHAR_PTR	pName,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRuleInfo
);

static
FTM_RET	FTOM_CGI_addRuleToArray
(
	cJSON _PTR_ pObject,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRuleInfo
);

static
FTM_RET	FTOM_CGI_createRuleObject
(
	FTM_RULE_PTR	pRuleInfo,
	FTM_RULE_FIELD	xFields,
	cJSON _PTR_ _PTR_ ppObject
);

FTM_RET	FTOM_CGI_getRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	FTM_RULE		xRuleInfo;
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();
	
	xRet = FTOM_CGI_getRuleID(pReq, pRuleID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_RULE_get(pClient, pRuleID, &xRuleInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_addRuleToObject(pRoot, "rule", FTM_RULE_FIELD_ALL, &xRuleInfo);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_addRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_RULE		xRuleInfo;
	FTM_RULE_FIELD	xFields = 0;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();
	
	FTM_RULE_setDefault(&xRuleInfo);

	xRet = FTOM_CGI_getRuleID(pReq, xRuleInfo.pID, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_RULE_FIELD_ID;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}
	
	xRet = FTOM_CGI_getName(pReq, xRuleInfo.pName, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_RULE_FIELD_NAME;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}
	
	for(FTM_INT i = 0 ; i < 8 ; i++)
	{
		xRet = FTOM_CGI_getRuleTrigger(pReq, i, xRuleInfo.xParams.pTriggers[i], FTM_ID_LEN, FTM_FALSE);
		if (xRet == FTM_RET_OK)
		{
			xFields |= FTM_RULE_FIELD_TRIGGERS;	
			xRuleInfo.xParams.ulTriggers++;
		}
		else if (xRet == FTM_RET_OBJECT_NOT_FOUND)
		{
			break;
		}
		else
		{
			goto finish;
		}
	}

	for(FTM_INT i = 0 ; i < 8 ; i++)
	{
		xRet = FTOM_CGI_getRuleAction(pReq, i, xRuleInfo.xParams.pActions[i], FTM_ID_LEN, FTM_FALSE);
		if (xRet == FTM_RET_OK)
		{
			xFields |= FTM_RULE_FIELD_ACTIONS;	
			xRuleInfo.xParams.ulActions++;
		}
		else if (xRet == FTM_RET_OBJECT_NOT_FOUND)
		{
			break;
		}
		else 
		{
			goto finish;
		}
	}

	xRet = FTOM_CLIENT_RULE_add(pClient, &xRuleInfo, pRuleID, FTM_ID_LEN);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_CLIENT_RULE_get(pClient, pRuleID, &xRuleInfo);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_CGI_addRuleToObject(pRoot, "rule", xFields, &xRuleInfo);
		}
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();
	
	xRet = FTOM_CGI_getRuleID(pReq, pRuleID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_RULE_del(pClient, pRuleID);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getRuleList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_INT			i;
	FTM_ULONG		ulCount = 0;
	FTM_CHAR_PTR	pValue = NULL;
	FTM_RULE_FIELD	xFields = FTM_RULE_FIELD_ID;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pRules = NULL;

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
			xFields = FTM_RULE_FIELD_ALL;
			break;
		}
		else if (strcasecmp(pValue, "name") == 0)
		{
			xFields |= FTM_RULE_FIELD_NAME;	
		}
		else if (strcasecmp(pValue, "rule") == 0)
		{
			xFields |= FTM_RULE_FIELD_RULE;	
		}
	}

	xRet = FTOM_CLIENT_RULE_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON_AddItemToObject(pRoot, "rules", pRules = cJSON_CreateArray());

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		FTM_RULE	xRuleInfo;

		xRet = FTOM_CLIENT_RULE_getAt(pClient, i, &xRuleInfo);
		if (xRet != FTM_RET_OK)
		{
			TRACE("RULE INFO GET ERROR\n");
			continue;
		}

		FTOM_CGI_addRuleToArray(pRules, xFields, &xRuleInfo);
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_setRule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_RULE		xRuleInfo;
	FTM_CHAR		pRuleID[FTM_ID_LEN+1];
	FTM_RULE_FIELD	xFields = 0;
	cJSON _PTR_		pRoot = NULL;

	pRoot = cJSON_CreateObject();
	
	FTM_RULE_setDefault(&xRuleInfo);

	xRet = FTOM_CGI_getRuleID(pReq, pRuleID, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CLIENT_RULE_get(pClient, pRuleID, &xRuleInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getName(pReq, xRuleInfo.pName, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_RULE_FIELD_NAME;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	for(FTM_INT i = 0 ; i < 8 ; i++)
	{
		FTM_CHAR		pTitle[32];
		FTM_CHAR_PTR	pValue;

		sprintf(pTitle, "trig%d", i+1);
		pValue = pReq->getstr(pReq, pTitle, false);
		if (pValue == NULL)
		{
			break;	
		}

		if (i == 0)
		{
			xRuleInfo.xParams.ulTriggers = 0;	
		}

		if (strlen(pValue) > FTM_ID_LEN)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
		}

		strncpy(xRuleInfo.xParams.pTriggers[xRuleInfo.xParams.ulTriggers++], pValue, FTM_ID_LEN);
		xFields |= FTM_RULE_FIELD_TRIGGERS;
	}

	for(FTM_INT i = 0 ; i < 8 ; i++)
	{
		FTM_CHAR		pTitle[32];
		FTM_CHAR_PTR	pValue;

		sprintf(pTitle, "act%d", i+1);
		pValue = pReq->getstr(pReq, pTitle, false);
		if (pValue == NULL)
		{
			break;	
		}
		
		if (i == 0)
		{
			xRuleInfo.xParams.ulActions = 0;	
		}

		if (strlen(pValue) > FTM_ID_LEN)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
		}

		strncpy(xRuleInfo.xParams.pActions[xRuleInfo.xParams.ulActions++], pValue, FTM_ID_LEN);
		xFields |= FTM_RULE_FIELD_ACTIONS;
	}

	xRet = FTOM_CLIENT_RULE_set(pClient, pRuleID, xFields, &xRuleInfo);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTOM_CLIENT_RULE_get(pClient, pRuleID, &xRuleInfo);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_CGI_addRuleToObject(pRoot, "rule", FTM_RULE_FIELD_ALL, &xRuleInfo);
		}
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_addRuleToObject
(
	cJSON _PTR_ pObject,
	FTM_CHAR_PTR	pName,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRuleInfo
)
{
	ASSERT(pObject != NULL);
	ASSERT(pRuleInfo != NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createRuleObject(pRuleInfo, xFields, &pNewObject);
	if(xRet == FTM_RET_OK)
	{
		cJSON_AddItemToObject(pObject, pName, pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_addRuleToArray
(
	cJSON _PTR_ pObject,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRuleInfo
)

{
	ASSERT(pObject != NULL);
	ASSERT(pRuleInfo != NULL);

	FTM_RET	xRet;
	cJSON _PTR_ pNewObject;

	xRet = FTOM_CGI_createRuleObject(pRuleInfo, xFields, &pNewObject);
	if(xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_createRuleObject
(
	FTM_RULE_PTR	pRuleInfo,
	FTM_RULE_FIELD	xFields,
	cJSON _PTR_ _PTR_ ppObject
)
{
	ASSERT(pRuleInfo != NULL);
	ASSERT(ppObject != NULL);


	cJSON _PTR_ pObject;
	cJSON _PTR_ pParams;
	cJSON _PTR_ pTriggers;
	cJSON _PTR_ pActions;

	pObject = cJSON_CreateObject();

	if (xFields & FTM_RULE_FIELD_ID)
	{
		cJSON_AddStringToObject(pObject, "id", pRuleInfo->pID);	
	}

	if (xFields & FTM_RULE_FIELD_NAME)
	{
		cJSON_AddStringToObject(pObject, "name", pRuleInfo->pName);
	}

	if (xFields & FTM_RULE_FIELD_STATE)
	{
		if (pRuleInfo->xState == FTM_RULE_STATE_ACTIVATE)
		{
			cJSON_AddStringToObject(pObject, "state", "ACTIVATE");
		}
		else
		{
			cJSON_AddStringToObject(pObject, "state", "DEACTIVATE");
		}
	}

	if (xFields & FTM_RULE_FIELD_RULE)
	{
		cJSON_AddItemToObject(pObject, "params", pParams = cJSON_CreateObject());
		cJSON_AddItemToObject(pParams, "triggers", pTriggers = cJSON_CreateArray());
		for(FTM_INT i = 0 ; i < pRuleInfo->xParams.ulTriggers ; i++)
		{
			cJSON_AddItemToArray(pTriggers, cJSON_CreateString(pRuleInfo->xParams.pTriggers[i]));
		}
	
		cJSON_AddItemToObject(pParams, "actions", pActions = cJSON_CreateArray());
		for(FTM_INT i = 0 ; i < pRuleInfo->xParams.ulActions ; i++)
		{
			cJSON_AddItemToArray(pActions, cJSON_CreateString(pRuleInfo->xParams.pActions[i]));
		}
	}

	*ppObject = pObject;

	return	FTM_RET_OK;
}
