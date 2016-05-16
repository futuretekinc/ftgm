#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"


static 
FTM_RET	FTOM_CGI_addEPInfoToArray
(
	cJSON _PTR_ 		pObject,
	FTM_EP_PTR			pEPInfo,
	FTM_EP_FIELD		xFields
);
static
FTM_RET	FTOM_CGI_createEPInfoObject
(
	FTM_EP_PTR			pEPInfo,
	FTM_EP_FIELD		xFields,
	cJSON _PTR_ _PTR_	ppObject
);

FTM_RET	FTOM_CGI_addEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_EP			xEPInfo;
	FTM_EP			xNewEPInfo;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	FTM_EP_setDefault(&xEPInfo);

	xRet = FTOM_CGI_getEPID(pReq, xEPInfo.pEPID, FTM_TRUE);
	xRet |= FTOM_CGI_getEPType(pReq, &xEPInfo.xType, FTM_FALSE);
	xRet |= FTOM_CGI_getDID(pReq, xEPInfo.pDID, FTM_FALSE);
	xRet |= FTOM_CGI_getEPFlags(pReq, &xEPInfo.xFlags, FTM_TRUE);
	xRet |= FTOM_CGI_getName(pReq, xEPInfo.pName, FTM_TRUE);
	xRet |= FTOM_CGI_getUnit(pReq, xEPInfo.pUnit, FTM_TRUE);
	xRet |= FTOM_CGI_getEnable(pReq, &xEPInfo.bEnable, FTM_TRUE);
	xRet |= FTOM_CGI_getTimeout(pReq,&xEPInfo.ulTimeout, FTM_TRUE);
	xRet |= FTOM_CGI_getInterval(pReq, &xEPInfo.ulInterval, FTM_TRUE);
	xRet |= FTOM_CGI_getLimit(pReq, &xEPInfo.xLimit, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CLIENT_EP_create(pClient, &xEPInfo, &xNewEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}


	FTOM_CGI_addEPInfoToObject(pRoot, &xNewEPInfo, FTM_EP_FIELD_ALL);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_destroy(pClient, pEPID);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_EP		xEPInfo;
	FTM_CHAR	pEPID[FTM_EPID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_addEPInfoToObject(pRoot, &xEPInfo, FTM_EP_FIELD_ALL);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_setEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_EP			xEPInfo;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_FIELD	xFields = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	
	xRet = FTOM_CGI_getEPFlags(pReq, &xEPInfo.xFlags, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_FLAGS;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getName(pReq, xEPInfo.pName, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_NAME;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getUnit(pReq, xEPInfo.pUnit, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_UNIT;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getEnable(pReq, &xEPInfo.bEnable, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_ENABLE;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getTimeout(pReq, &xEPInfo.ulTimeout, FTM_FALSE); 
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_TIMEOUT;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getInterval(pReq, &xEPInfo.ulInterval, FTM_FALSE); 
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_INTERVAL;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getLimit(pReq, &xEPInfo.xLimit, FTM_FALSE); 
	if (xRet == FTM_RET_OK)
	{
		xFields |= FTM_EP_FIELD_INTERVAL;	
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_set(pClient, pEPID, xFields, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_addEPInfoToObject(pRoot, &xEPInfo, FTM_EP_FIELD_ALL);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getEPCount
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulCount = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON_AddNumberToObject(pRoot, "count", ulCount);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getEPList
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_INT			i;
	FTM_CHAR_PTR	pValue = NULL;
	FTM_ULONG		ulCount = 0;
	FTM_EP_FIELD	xFields = FTM_EP_FIELD_EPID;
	FTM_EPID		*pEPIDList = NULL;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

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
			xFields |= FTM_EP_FIELD_ALL;
		}
		else if (strcasecmp(pValue, "type") == 0)
		{
			xFields |= FTM_EP_FIELD_EPTYPE;
		}
		else if (strcasecmp(pValue, "flags") == 0)
		{
			xFields |= FTM_EP_FIELD_FLAGS;
		}
		else if (strcasecmp(pValue, "name") == 0)
		{
			xFields |= FTM_EP_FIELD_NAME;
		}
		else if (strcasecmp(pValue, "unit") == 0)
		{
			xFields |= FTM_EP_FIELD_UNIT;
		}
		else if (strcasecmp(pValue, "enable") == 0)
		{
			xFields |= FTM_EP_FIELD_ENABLE;
		}
		else if (strcasecmp(pValue, "timeout") == 0)
		{
			xFields |= FTM_EP_FIELD_TIMEOUT;
		}
		else if (strcasecmp(pValue, "interval") == 0)
		{
			xFields |= FTM_EP_FIELD_INTERVAL;
		}
		else if (strcasecmp(pValue, "did") == 0)
		{
			xFields |= FTM_EP_FIELD_DID;
		}
		else if (strcasecmp(pValue, "limit") == 0)
		{
			xFields |= FTM_EP_FIELD_LIMIT;
		}
	}

	xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	pEPIDList = (FTM_EPID_PTR)FTM_MEM_malloc((FTM_EPID_LEN + 1) * ulCount);
	if (pEPIDList == NULL)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_getList(pClient, 0, pEPIDList, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pEPList = cJSON_CreateArray();

	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_EP	xEPInfo;

		xRet = FTOM_CLIENT_EP_get(pClient, pEPIDList[i], &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		FTOM_CGI_addEPInfoToArray(pEPList, &xEPInfo, xFields);
	}
	
	cJSON_AddItemToObject(pRoot, "eps", pEPList);
	xRet = FTM_RET_OK;

finish:
	if (pEPIDList != NULL)
	{
		FTM_MEM_free(pEPIDList);
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getEPDataInfo
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulBegin = 0;
	FTM_ULONG		ulEnd = 0;
	FTM_ULONG		ulCount = 0;
	cJSON _PTR_		pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_DATA_info(pClient, pEPID, &ulBegin, &ulEnd, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP Data info get failed[%08x]\n", xRet);
		goto finish;
	}

	cJSON_AddStringToObject(pRoot, "epid", pEPID);
	cJSON_AddNumberToObject(pRoot, "begin", ulBegin);
	cJSON_AddNumberToObject(pRoot, "end", ulEnd);
	cJSON_AddNumberToObject(pRoot, "count", ulCount);

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getEPData
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pData;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulCount = 20;
	cJSON _PTR_		pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	xRet |= FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finish;	
	}

	if (ulCount > 100)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finish;
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulCount);
	if (pData == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		goto finish;
	}

	xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, 0, pData, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP Data list get failed[%08x]\n", xRet);
		goto finish;
	}

	cJSON _PTR_ pDataList;

	cJSON_AddStringToObject(pRoot, "epid", pEPID);
	cJSON_AddItemToObject(pRoot, "data", pDataList = cJSON_CreateArray());

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject;

		cJSON_AddItemToArray(pDataList, pObject = cJSON_CreateObject());

		FTM_CHAR	pValueString[64];
		FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &pData[i]);

		cJSON_AddStringToObject(pObject, "value", pValueString);
		cJSON_AddNumberToObject(pObject, "time", pData[i].ulTime);
	}

finish:
	if (pData != NULL)
	{
		FTM_MEM_free(pData);
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_getEPDataLast
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount = 0;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EPID		*pEPIDList = NULL;
	cJSON _PTR_	pRoot, _PTR_ pDataArray;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		FTM_EP		xInfo;
		FTM_EP_DATA	xEPData;
		FTM_CHAR	pValueString[64];

		xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			goto finish;
		}

		xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pEPID, &xEPData);
		if (xRet != FTM_RET_OK)
		{
			goto finish;
		}

		xRet = FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &xEPData);
		if (xRet != FTM_RET_OK)
		{
			goto finish;
		}

		cJSON_AddStringToObject(pRoot, "epid", pEPID);
		cJSON_AddStringToObject(pRoot, "value", pValueString);
		cJSON_AddNumberToObject(pRoot, "time", xEPData.ulTime);
		if (xInfo.bEnable)
		{
			cJSON_AddStringToObject(pRoot, "state", "run");
		}
		else
		{
			cJSON_AddStringToObject(pRoot, "state", "stop");
		}
	}
	else if (xRet != FTM_RET_OBJECT_NOT_FOUND)
	{
		xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			goto finish;
		}

		pEPIDList = (FTM_EPID_PTR)FTM_MEM_malloc((FTM_EPID_LEN + 1) * ulCount);
		if (pEPIDList == NULL)
		{
			goto finish;	
		}

		xRet = FTOM_CLIENT_EP_getList(pClient, 0, pEPIDList, ulCount, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			goto finish;
		}

		cJSON_AddItemToObject(pRoot, "datas", pDataArray = cJSON_CreateArray());
		for(int i = 0 ; i < ulCount ; i++)
		{
			FTM_EP		xInfo;
			FTM_EP_DATA	xEPData;
			FTM_CHAR	pValueString[64];
			cJSON _PTR_ pObject;
	
			xRet = FTOM_CLIENT_EP_get(pClient, pEPIDList[i], &xInfo);
			if (xRet != FTM_RET_OK)
			{
				goto finish;
			}

			xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pEPIDList[i], &xEPData);
			if (xRet != FTM_RET_OK)
			{
				continue;
			}
	
			xRet = FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &xEPData);
			if (xRet != FTM_RET_OK)
			{
				continue;
			}
	
			cJSON_AddItemToArray(pDataArray, pObject = cJSON_CreateObject());
			cJSON_AddStringToObject(pObject, "epid", pEPIDList[i]);
			cJSON_AddStringToObject(pObject, "value", pValueString);
			cJSON_AddNumberToObject(pObject, "time", xEPData.ulTime);
			if (xInfo.bEnable)
			{
				cJSON_AddStringToObject(pRoot, "state", "run");
			}
			else 
			{
				cJSON_AddStringToObject(pRoot, "state", "stop");
			}
		}

	}
	else
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finish;	
	}

finish:
	if (pEPIDList != NULL)
	{
		FTM_MEM_free(pEPIDList);
	}

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_delEPData
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulIndex = 0;
	FTM_ULONG		ulCount = 0;
	FTM_ULONG		ulBegin = 0;
	FTM_ULONG		ulEnd = 0;
	cJSON _PTR_		pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

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

	xRet = FTOM_CGI_getBeginTime(pReq, &ulBegin, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CGI_getEndTime(pReq, &ulEnd, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	if (ulCount != 0)
	{
		if ((ulBegin != 0) || (ulEnd != 0))
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;	
			goto finish;
		}

		xRet = FTOM_CLIENT_EP_DATA_del(pClient, pEPID, ulIndex, ulCount, &ulCount);
	}
	else if ((ulBegin != 0) || (ulEnd != 0))
	{
		xRet = FTOM_CLIENT_EP_DATA_delWithTime(pClient,	pEPID, ulBegin, ulEnd, &ulCount);
	}
	else
	{
		xRet = FTOM_CLIENT_EP_DATA_del(pClient, pEPID, 0, 0, &ulCount);
	}

	if (xRet != FTM_RET_OK)
	{
		ERROR("EP Data delete failed[%08x]\n", xRet);
	}
	else
	{
		TRACE("%lu data have been deleted.\n", ulCount);
		cJSON_AddNumberToObject(pRoot, "count", ulCount);

	}

finish:

	return	FTOM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTOM_CGI_addEPInfoToObject
(
	cJSON _PTR_	pObject,
	FTM_EP_PTR			pEPInfo,
	FTM_EP_FIELD		xFields

)
{
	ASSERT(pObject != NULL);
	ASSERT(pEPInfo != NULL);
	
	FTM_RET	xRet;
	cJSON _PTR_ pNewObject = NULL;

	xRet = FTOM_CGI_createEPInfoObject(pEPInfo, xFields, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToObject(pObject, "ep", pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_addEPInfoToArray
(
	cJSON _PTR_		pObject,
	FTM_EP_PTR		pEPInfo,
	FTM_EP_FIELD	xFields

)
{
	ASSERT(pObject != NULL);
	ASSERT(pEPInfo != NULL);
	
	FTM_RET	xRet;
	cJSON _PTR_ pNewObject = NULL;

	xRet = FTOM_CGI_createEPInfoObject(pEPInfo, xFields, &pNewObject);
	if (xRet == FTM_RET_OK)
	{
		cJSON_AddItemToArray(pObject, pNewObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_createEPInfoObject
(
	FTM_EP_PTR		pEPInfo,
	FTM_EP_FIELD	xFields,
	cJSON _PTR_ _PTR_	ppObject
)
{
	cJSON _PTR_	pObject, _PTR_ pLimit;


	pObject = cJSON_CreateObject();
	if (pObject == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	if (xFields & FTM_EP_FIELD_EPID)
	{
		cJSON_AddStringToObject(pObject, "epid", pEPInfo->pEPID);
	}

	if (xFields & FTM_EP_FIELD_EPTYPE)
	{
		cJSON_AddStringToObject(pObject, "type", FTM_EP_typeString(pEPInfo->xType));
	}

	if (xFields & FTM_EP_FIELD_NAME)
	{
		cJSON_AddStringToObject(pObject, "name", pEPInfo->pName);
	}

	if (xFields & FTM_EP_FIELD_UNIT)
	{
		cJSON_AddStringToObject(pObject, "unit", pEPInfo->pUnit);
	}

	if (xFields & FTM_EP_FIELD_ENABLE)
	{
		if (pEPInfo->bEnable)
		{
			cJSON_AddStringToObject(pObject, "state", "run");
		}
		else
		{
			cJSON_AddStringToObject(pObject, "state", "stop");
		}
	}

	if (xFields & FTM_EP_FIELD_INTERVAL)
	{
		cJSON_AddNumberToObject(pObject, "interval", pEPInfo->ulInterval);
	}

	if (xFields & FTM_EP_FIELD_DID)
	{
		cJSON_AddStringToObject(pObject, "did", pEPInfo->pDID);
	}

	if (xFields & FTM_EP_FIELD_LIMIT)
	{
		cJSON_AddItemToObject(pObject, "limit", pLimit = cJSON_CreateObject());
		
		switch(pEPInfo->xLimit.xType)
		{
		case	FTM_EP_LIMIT_TYPE_COUNT:
			{
				cJSON_AddStringToObject(pLimit, "type", "count");
				cJSON_AddNumberToObject(pLimit, "count", pEPInfo->xLimit.xParams.ulCount);
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_TIME:
			{
				cJSON _PTR_ pTime;
	
				cJSON_AddStringToObject(pLimit, "type", "time");
				cJSON_AddItemToObject(pLimit, "time", pTime = cJSON_CreateObject());
				cJSON_AddNumberToObject(pTime, "start", pEPInfo->xLimit.xParams.xTime.ulStart);
				cJSON_AddNumberToObject(pTime, "end", pEPInfo->xLimit.xParams.xTime.ulEnd);
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_HOURS:
			{
				cJSON_AddStringToObject(pLimit, "type", "hours");
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_DAYS:
			{
				cJSON_AddStringToObject(pLimit, "type", "days");
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_MONTHS:
			{
				cJSON_AddStringToObject(pLimit, "type", "months");
			}
			break;
		}
	}

	*ppObject = pObject;

	return	FTM_RET_OK;
}

