#include "ftom_cgi.h"
#include "ftom_client.h"
#include "cJSON.h"

FTM_RET	FTOM_CGI_addNodeInfoToObject
(
	cJSON _PTR_ 	pRoot, 
	FTM_NODE_PTR 	pNodeInfo, 
	FTM_NODE_FIELD 	xFields
);

FTM_RET	FTOM_CGI_addNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	FTM_NODE	xInfo;
	FTM_CHAR	pDID[FTM_ID_LEN+1];
	FTM_NODE_FIELD	xFields = 0;
	cJSON _PTR_ pRoot;

	pRoot= cJSON_CreateObject();

	FTM_NODE_setDefault(&xInfo);

	xRet = FTOM_CGI_getDID(pReq, xInfo.pDID, FTM_TRUE);
	xRet |= FTOM_CGI_getNodeType(pReq, &xInfo.xType, FTM_FALSE);
	xRet |= FTOM_CGI_getLocation(pReq, xInfo.pLocation, FTM_TRUE);
	xRet |= FTOM_CGI_getInterval(pReq, &xInfo.ulInterval, FTM_TRUE);
	xRet |= FTOM_CGI_getTimeout(pReq, &xInfo.ulTimeout, FTM_TRUE);
	
	switch(xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
		}
		break;

	default:
		goto finish;
	}

	xRet = FTOM_CLIENT_NODE_create(pClient, &xInfo, pDID);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

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
	FTM_CHAR	pDID[FTM_ID_LEN+1];
	cJSON _PTR_ pRoot;

	pRoot= cJSON_CreateObject();

	xRet = FTOM_CGI_getDID(pReq, pDID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_NODE_get(pClient, pDID, &xNodeInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_addNodeInfoToObject(pRoot, &xNodeInfo, FTM_NODE_FIELD_ALL);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
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
		cJSON _PTR_ pObject;

		xRet = FTOM_CLIENT_NODE_getAt(pClient, i, &xNodeInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;	
		}
		
		cJSON_AddItemToArray(pRoot, pObject = cJSON_CreateObject());
		FTOM_CGI_addNodeInfoToObject(pObject, &xNodeInfo, FTM_NODE_FIELD_ALL);
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

FTM_RET	FTOM_CGI_setNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	FTM_NODE	xInfo;
	FTM_CHAR	pDID[FTM_ID_LEN+1];
	FTM_NODE_FIELD	xFields = 0;
	cJSON _PTR_ pRoot;

	pRoot= cJSON_CreateObject();

	xRet = FTOM_CGI_getDID(pReq, pDID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	xRet = FTOM_CLIENT_NODE_get(pClient, pDID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getLocation(pReq, xInfo.pLocation, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_LOCATION;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getInterval(pReq, &xInfo.ulInterval, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_INTERVAL;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getTimeout(pReq, &xInfo.ulTimeout, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_TIMEOUT;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CLIENT_NODE_set(pClient, pDID, xFields, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_addNodeInfoToObject
(
	cJSON _PTR_ 	pRoot, 
	FTM_NODE_PTR 	pNodeInfo, 
	FTM_NODE_FIELD 	xFields
)
{
	ASSERT(pRoot != NULL);
	ASSERT(pNodeInfo != NULL);

	cJSON _PTR_ pFlags;
	
	if (xFields & FTM_NODE_FIELD_DID )
	{
		cJSON_AddStringToObject(pRoot,	"did", 		pNodeInfo->pDID);
	}
	
	if (xFields & FTM_NODE_FIELD_TYPE )
	{
		cJSON_AddStringToObject(pRoot,	"type", 	FTM_NODE_typeString(pNodeInfo->xType));
	}
	
	if (xFields & FTM_NODE_FIELD_LOCATION )
	{
		cJSON_AddStringToObject(pRoot,	"location", pNodeInfo->pLocation);
	}
	
	if (xFields & FTM_NODE_FIELD_INTERVAL )
	{
		cJSON_AddNumberToObject(pRoot,	"interval", pNodeInfo->ulInterval);
	}
	
	if (xFields & FTM_NODE_FIELD_TIMEOUT )
	{
		cJSON_AddNumberToObject(pRoot,	"timeout", 	pNodeInfo->ulTimeout);
	}
	
	if (xFields & FTM_NODE_FIELD_FLAGS )
	{
		cJSON_AddItemToObject(pRoot, 	"flags", 	pFlags = cJSON_CreateArray());
	
		for(FTM_INT j = 0; FTM_NODE_FLAG_LAST < (1 << j) ; j++)
		{
			if (pNodeInfo->xFlags & (1 << j))
			{
				cJSON_AddItemToArray(pFlags, cJSON_CreateString(FTM_NODE_flagString(1 << j)));	
			}
		}
	
	}
	
	if (xFields & FTM_NODE_FIELD_OPTION)
	{
		switch(pNodeInfo->xType)
		{
		case	FTM_NODE_TYPE_SNMP:
			{
				cJSON _PTR_ pSNMP;
	
				cJSON_AddItemToObject(pRoot, "snmp", pSNMP = cJSON_CreateObject());
				cJSON_AddStringToObject(pSNMP, "version", 	FTM_SNMP_versionString(pNodeInfo->xOption.xSNMP.ulVersion));
				cJSON_AddStringToObject(pSNMP, "url", 		pNodeInfo->xOption.xSNMP.pURL);
				cJSON_AddStringToObject(pSNMP, "community", pNodeInfo->xOption.xSNMP.pCommunity);
				cJSON_AddStringToObject(pSNMP, "mib", 		pNodeInfo->xOption.xSNMP.pMIB);
				cJSON_AddNumberToObject(pSNMP, "retry",	pNodeInfo->xOption.xSNMP.ulMaxRetryCount);
			}
			break;
	
		default:
			{
			}
		}
	}

	return	FTM_RET_OK;
}

