#include "ftom_cgi.h"
#include "ftom_client.h"
#include "cJSON.h"

FTM_RET	FTOM_CGI_getNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	FTM_NODE	xNodeInfo;
	cJSON _PTR_ pRoot, _PTR_ pFlags;
	FTM_CHAR_PTR	pDID = pReq->getstr(pReq, "id", false);
	if (pDID == NULL)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_NODE_get(pClient, pDID, &xNodeInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot,	"DID", 		xNodeInfo.pDID);
	cJSON_AddStringToObject(pRoot,	"TYPE", 	FTM_NODE_typeString(xNodeInfo.xType));
	cJSON_AddStringToObject(pRoot,	"LOCATION", xNodeInfo.pLocation);
	cJSON_AddNumberToObject(pRoot,	"INTERVAL", xNodeInfo.ulInterval);
	cJSON_AddNumberToObject(pRoot,	"TIMEOUT", 	xNodeInfo.ulTimeout);
	cJSON_AddItemToObject(pRoot, 	"FLAGS", 	pFlags = cJSON_CreateArray());

	for(FTM_INT j = 0; FTM_NODE_FLAG_LAST < (1 << j) ; j++)
	{
		if (xNodeInfo.xFlags & (1 << j))
		{
			cJSON_AddItemToArray(pFlags, cJSON_CreateString(FTM_NODE_flagString(1 << j)));	
		}
	}

finish:

	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getNodeList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount = 0;
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_NODE		xNodeInfo;
	cJSON _PTR_ pRoot;

	xRet = FTOM_CLIENT_NODE_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	pRoot = cJSON_CreateArray();

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject, _PTR_ pFlags;

		xRet = FTOM_CLIENT_NODE_getAt(pClient, i, &xNodeInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;	
		}
		
		cJSON_AddItemToArray(pRoot, pObject = cJSON_CreateObject());
		cJSON_AddStringToObject(pObject,"DID", 		xNodeInfo.pDID);
		cJSON_AddStringToObject(pObject,"TYPE", 	FTM_NODE_typeString(xNodeInfo.xType));
		cJSON_AddStringToObject(pObject,"LOCATION", xNodeInfo.pLocation);
		cJSON_AddNumberToObject(pObject,"INTERVAL", xNodeInfo.ulInterval);
		cJSON_AddNumberToObject(pObject,"TIMEOUT", 	xNodeInfo.ulTimeout);
		cJSON_AddItemToObject(pObject, 	"FLAGS", 	pFlags = cJSON_CreateArray());

		for(FTM_INT j = 0; FTM_NODE_FLAG_LAST < (1 << j) ; j++)
		{
			if (xNodeInfo.xFlags & (1 << j))
			{
				cJSON_AddItemToArray(pFlags, cJSON_CreateString(FTM_NODE_flagString(1 << j)));	
			}
		}
	}

	pBuff = cJSON_Print(pRoot);

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);

	xRet = FTM_RET_OK;

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	xRet;
}

