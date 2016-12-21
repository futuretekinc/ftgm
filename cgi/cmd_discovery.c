#include "ftom_cgi.h"
#include "ftom_client.h"
#include "cJSON.h"

FTM_RET	FTOM_CGI_startDiscovery
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	cJSON _PTR_ pRoot;
	FTM_CHAR	pIP[256] = "255.255.255.255";
	FTM_USHORT	usPort = 1234;
	FTM_ULONG	ulRetryCount = 3;

	pRoot= cJSON_CreateObject();

	xRet = FTOM_CGI_getIPString(pReq, pIP, sizeof(pIP), FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getPort(pReq, &usPort, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getRetry(pReq, &ulRetryCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_DISCOVERY_start(pClient, pIP, usPort, ulRetryCount);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getDiscoveryInfo
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	FTM_BOOL	bFinished = FTM_FALSE;
	FTM_ULONG	ulNodeInfoCount = 0;
	FTM_ULONG	ulEPInfoCount = 0;
	cJSON _PTR_ pRoot;


	pRoot= cJSON_CreateObject();

	xRet = FTOM_CLIENT_DISCOVERY_getInfo(pClient, &bFinished, &ulNodeInfoCount, &ulEPInfoCount);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddStringToObject(pRoot,	"finished", (bFinished)?"true":"false");
		if (bFinished == FTM_TRUE)
		{
			cJSON_AddNumberToObject(pRoot,	"node count", ulNodeInfoCount);
			cJSON_AddNumberToObject(pRoot,	"ep count", ulEPInfoCount);
		}
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}


FTM_RET	FTOM_CGI_getDiscoveryNodeList
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	cJSON _PTR_ pRoot;
	cJSON _PTR_ pNodes;
	FTM_ULONG	ulIndex = 0;
	FTM_ULONG	ulCount = 10;
	FTM_NODE_PTR	pNodeList = NULL;

	pRoot= cJSON_CreateObject();

	xRet = FTOM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	if (ulCount == 0)
	{
		goto finish;	
	}
	
	pNodeList = (FTM_NODE_PTR)FTM_MEM_malloc(sizeof(FTM_NODE) * ulCount);
	if (pNodeList == NULL)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_DISCOVERY_getNodeList(pClient, ulIndex, ulCount, pNodeList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	cJSON_AddNumberToObject(pRoot, "index", ulIndex);
	cJSON_AddNumberToObject(pRoot, "count", ulCount);
	cJSON_AddItemToObject(pRoot, "nodes", pNodes = cJSON_CreateArray());

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject;

		cJSON_AddItemToArray(pNodes, pObject = cJSON_CreateObject());
		FTOM_CGI_addNodeInfoToObject(pObject, &pNodeList[i], FTM_NODE_FIELD_ALL);
	}

finish:

	if (pNodeList != NULL)
	{
		FTM_MEM_free(pNodeList);	
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getDiscoveryEPList
(
	FTOM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);
	
	FTM_RET		xRet;
	cJSON _PTR_ pRoot;
	cJSON _PTR_ pEPs;
	FTM_ULONG	ulIndex = 0;
	FTM_ULONG	ulCount = 10;
	FTM_EP_PTR	pEPList = NULL;

	pRoot= cJSON_CreateObject();

	xRet = FTOM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	if (ulCount == 0)
	{
		goto finish;	
	}
	
	pEPList = (FTM_EP_PTR)FTM_MEM_malloc(sizeof(FTM_EP) * ulCount);
	if (pEPList == NULL)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_DISCOVERY_getEPList(pClient, ulIndex, ulCount, pEPList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		TRACE("FTOM_CLIENT_DISCOVERY_getEPList error!\n");
		goto finish;	
	}

	cJSON_AddNumberToObject(pRoot, "index", ulIndex);
	cJSON_AddNumberToObject(pRoot, "count", ulCount);
	cJSON_AddItemToObject(pRoot, "eps", pEPs = cJSON_CreateArray());

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject;

		cJSON_AddItemToArray(pEPs, pObject = cJSON_CreateObject());
		FTOM_CGI_addEPInfoToObject(pObject, &pEPList[i], FTM_EP_FIELD_ALL);
	}

finish:

	if (pEPList != NULL)
	{
		FTM_MEM_free(pEPList);	
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}
