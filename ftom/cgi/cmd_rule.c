#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"

FTM_RET	FTOM_CGI_getRuleList
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

	xRet = FTOM_CLIENT_RULE_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	pRoot = cJSON_CreateObject();
	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		FTM_RULE	xRuleInfo;
		cJSON 	_PTR_ pParams, _PTR_ pTriggers, _PTR_ pActions;

		xRet = FTOM_CLIENT_RULE_getAt(pClient, i, &xRuleInfo);
		if (xRet != FTM_RET_OK)
		{
			TRACE("RULE INFO GET ERROR\n");
			continue;
		}

		cJSON_AddNumberToObject(pRoot, "ID", xRuleInfo.xID);	
		if (xRuleInfo.xState == FTM_RULE_STATE_ACTIVATE)
		{
			cJSON_AddStringToObject(pRoot, "STATE", "ACTIVATE");
		}
		else
		{
			cJSON_AddStringToObject(pRoot, "STATE", "DEACTIVATE");
		}
		cJSON_AddItemToObject(pRoot, "PARAMS", pParams = cJSON_CreateObject());
		cJSON_AddItemToObject(pParams, "TRIGGERS", pTriggers = cJSON_CreateArray());
		for(FTM_INT j = 0 ; j < xRuleInfo.xParams.ulTriggers ; j++)
		{
			cJSON_AddItemToArray(pTriggers, cJSON_CreateNumber(xRuleInfo.xParams.pTriggers[i]));
		}

		cJSON_AddItemToObject(pParams, "ACTIONS", pActions = cJSON_CreateArray());
		for(FTM_INT j = 0 ; j < xRuleInfo.xParams.ulActions ; j++)
		{
			cJSON_AddItemToArray(pActions, cJSON_CreateNumber(xRuleInfo.xParams.pActions[i]));
		}
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
