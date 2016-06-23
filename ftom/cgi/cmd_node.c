#include "ftom_cgi.h"
#include "ftom_client.h"
#include "cJSON.h"

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
	cJSON _PTR_ pRoot;

	pRoot= cJSON_CreateObject();

	FTM_NODE_setDefault(&xInfo);

	xRet = FTOM_CGI_getDID(pReq, xInfo.pDID, FTM_TRUE);
	xRet |= FTOM_CGI_getNodeType(pReq, &xInfo.xType, FTM_FALSE);
	xRet |= FTOM_CGI_getLocation(pReq, xInfo.pLocation, FTM_TRUE);
	xRet |= FTOM_CGI_getReportInterval(pReq, &xInfo.ulReportInterval, FTM_TRUE);
	xRet |= FTOM_CGI_getTimeout(pReq, &xInfo.ulTimeout, FTM_TRUE);
	
	switch(xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			xRet |= FTOM_CGI_getNodeOptSNMP(pReq, &xInfo.xOption.xSNMP, FTM_TRUE);
		}
		break;

	default:
		ERROR("Node type[%s] not supported!\n", FTM_NODE_typeString(xInfo.xType));
		goto finish;
	}

	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid arguments!\n");
		goto finish;	
	}

	xRet = FTOM_CLIENT_NODE_create(pClient, &xInfo, pDID, sizeof(pDID));
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node creation failed.\n");
		goto finish;	
	}

	xRet = FTOM_CLIENT_NODE_get(pClient, pDID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] not found!\n", pDID);
		goto finish;
	}

	xRet = FTOM_CGI_addNodeInfoToObject(pRoot, &xInfo, FTM_NODE_FIELD_ALL);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delNode
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	FTM_CHAR	pDID[FTM_ID_LEN+1];
	cJSON _PTR_ pRoot;

	pRoot= cJSON_CreateObject();

	xRet = FTOM_CGI_getDID(pReq, pDID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_NODE_destroy(pClient, pDID);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddStringToObject(pRoot, "did", pDID);
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
	FTM_NODE		xNodeInfo;
	FTM_CHAR_PTR	pValue;
	FTM_NODE_FIELD	xFields = FTM_NODE_FIELD_DID;
	FTM_INT			i;
	cJSON _PTR_ pRoot; 
	cJSON _PTR_ pNodes;

	pRoot = cJSON_CreateObject();

	cJSON_AddItemToObject(pRoot, "nodes", pNodes = cJSON_CreateArray());

	for(i = 0; i < 10 ; i++)
	{
		FTM_CHAR	pTitle[32];

		sprintf(pTitle, "field%d", (i+1)); 

		pValue = pReq->getstr(pReq, pTitle, false);
		if (pValue == NULL)
		{
			break;	
		}

		if (strcasecmp(pValue, "all") == 0)
		{
			xFields |= FTM_NODE_FIELD_ALL;
		}
		else if (strcasecmp(pValue, "type") == 0)
		{
			xFields |= FTM_NODE_FIELD_TYPE;
		}
		else if (strcasecmp(pValue, "model") == 0)
		{
			xFields |= FTM_NODE_FIELD_MODEL;
		}
		else if (strcasecmp(pValue, "name") == 0)
		{
			xFields |= FTM_NODE_FIELD_NAME;
		}
		else if (strcasecmp(pValue, "flags") == 0)
		{
			xFields |= FTM_NODE_FIELD_FLAGS;
		}
		else if (strcasecmp(pValue, "location") == 0)
		{
			xFields |= FTM_NODE_FIELD_LOCATION;
		}
		else if (strcasecmp(pValue, "timeout") == 0)
		{
			xFields |= FTM_NODE_FIELD_TIMEOUT;
		}
		else if (strcasecmp(pValue, "interval") == 0)
		{
			xFields |= FTM_NODE_FIELD_INTERVAL;
		}
		else if (strcasecmp(pValue, "snmp") == 0)
		{
			xFields |= FTM_NODE_FIELD_SNMP;
		}
	}

	xRet = FTOM_CLIENT_NODE_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject;

		xRet = FTOM_CLIENT_NODE_getAt(pClient, i, &xNodeInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;	
		}
		
		cJSON_AddItemToArray(pNodes, pObject = cJSON_CreateObject());
		FTOM_CGI_addNodeInfoToObject(pObject, &xNodeInfo, xFields);
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
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

	xRet = FTOM_CGI_getModel(pReq, xInfo.pModel, FTM_TRUE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_MODEL;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getName(pReq, xInfo.pName, FTM_TRUE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_NAME;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getLocation(pReq, xInfo.pLocation, FTM_TRUE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_LOCATION;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getReportInterval(pReq, &xInfo.ulReportInterval, FTM_TRUE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_NODE_FIELD_INTERVAL;
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getTimeout(pReq, &xInfo.ulTimeout, FTM_TRUE);
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
	
	xRet = FTOM_CLIENT_NODE_get(pClient, pDID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	FTOM_CGI_addNodeInfoToObject(pRoot, &xInfo, FTM_NODE_FIELD_ALL);

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
	
	if (xFields & FTM_NODE_FIELD_MODEL)
	{
		cJSON_AddStringToObject(pRoot,	"model", pNodeInfo->pModel);
	}
	
	if (xFields & FTM_NODE_FIELD_NAME)
	{
		cJSON_AddStringToObject(pRoot,	"name", pNodeInfo->pName);
	}
	
	if (xFields & FTM_NODE_FIELD_LOCATION )
	{
		cJSON_AddStringToObject(pRoot,	"location", pNodeInfo->pLocation);
	}
	
	if (xFields & FTM_NODE_FIELD_INTERVAL )
	{
		cJSON_AddNumberToObject(pRoot,	"report_interval", pNodeInfo->ulReportInterval);
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
	
	switch(pNodeInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{
			if (xFields & FTM_NODE_FIELD_SNMP) 
			{
				cJSON _PTR_ pSNMP;
	
				cJSON_AddItemToObject(pRoot, "snmp", pSNMP = cJSON_CreateObject());
				if (xFields & FTM_NODE_FIELD_SNMP_VERSION)
				{
					//cJSON_AddNumberToObject(pSNMP, "version", 	pNodeInfo->xOption.xSNMP.ulVersion);
					cJSON_AddStringToObject(pSNMP, "version", 	FTM_SNMP_versionString(pNodeInfo->xOption.xSNMP.ulVersion));
				}

				if (xFields & FTM_NODE_FIELD_SNMP_URL)
				{
					cJSON_AddStringToObject(pSNMP, "url", 		pNodeInfo->xOption.xSNMP.pURL);
				}

				if (xFields & FTM_NODE_FIELD_SNMP_COMMUNITY)
				{
					cJSON_AddStringToObject(pSNMP, "community", pNodeInfo->xOption.xSNMP.pCommunity);
				}

				if (xFields & FTM_NODE_FIELD_SNMP_MIB)
				{
					cJSON_AddStringToObject(pSNMP, "mib", 		pNodeInfo->xOption.xSNMP.pMIB);
				}

				if (xFields & FTM_NODE_FIELD_SNMP_MAX_RETRY)
				{
					cJSON_AddNumberToObject(pSNMP, "retry",	pNodeInfo->xOption.xSNMP.ulMaxRetryCount);
				}
			}
		}
		break;
	
	default:
		{
		}
	}

	return	FTM_RET_OK;
}

