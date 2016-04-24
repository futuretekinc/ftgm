#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"

FTM_RET	FTOM_CGI_getActionList
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

	xRet = FTOM_CLIENT_ACTION_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	pRoot = cJSON_CreateObject();
	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_ACTION	xActionInfo;
		cJSON 	_PTR_ pAction;

		xRet = FTOM_CLIENT_ACTION_get(pClient, i, &xActionInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		cJSON_AddNumberToObject(pRoot, "ID", xActionInfo.xID);	
		cJSON_AddStringToObject(pRoot, "TYPE", FTM_ACTION_typeString(xActionInfo.xType));	
		cJSON_AddItemToObject(pRoot, "ACTION", pAction= cJSON_CreateObject());
		switch(xActionInfo.xType)
		{
		case	FTM_ACTION_TYPE_SET:
			{
				FTM_CHAR	pValueString[64];

				cJSON_AddStringToObject(pAction, "EPID", xActionInfo.xParams.xSet.pEPID);
				FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &xActionInfo.xParams.xSet.xValue);
				cJSON_AddStringToObject(pAction, "VALUE", pValueString);
			}
			break;

		case	FTM_ACTION_TYPE_SMS:
		case	FTM_ACTION_TYPE_PUSH:
		case	FTM_ACTION_TYPE_MAIL:
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
