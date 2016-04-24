#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"
#include "cJSON.h"

static 
FTM_RET	FTOM_CGI_makeEPInfo
(
	FTM_EP_PTR			pEPInfo,
	cJSON _PTR_ _PTR_ 	ppObject,
	FTM_EP_FIELD_TYPE	xFields
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

	FTM_EP_setDefault(&xEPInfo);

	xRet = FTOM_CGI_getEPID(pReq, xEPInfo.pEPID, FTM_FALSE);
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
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTOM_CLIENT_EP_create(pClient, &xEPInfo);
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

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_CLIENT_EP_destroy(pClient, pEPID);
}

FTM_RET	FTOM_CGI_getEP
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_EP			xEPInfo;
	FTM_CHAR_PTR	pBuff = NULL;
	cJSON _PTR_ 	pRoot = NULL;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];

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

	xRet = FTOM_CGI_makeEPInfo(&xEPInfo, &pRoot, FTM_EP_FIELD_ALL);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
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
	FTM_CHAR_PTR	pBuff = NULL;
	cJSON _PTR_ 	pRoot = NULL;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_TYPE		xEPType;
	FTM_CHAR		pDID[FTM_DID_LEN+1];

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getEPType(pReq, &xEPType, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_getDID(pReq, pDID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	
	xRet =	FTOM_CGI_getEPFlags(pReq, &xEPInfo.xFlags, FTM_TRUE);
	xRet |=	FTOM_CGI_getName(pReq, xEPInfo.pName, FTM_TRUE);
	xRet |=	FTOM_CGI_getUnit(pReq, xEPInfo.pUnit, FTM_TRUE);
	xRet |=	FTOM_CGI_getEnable(pReq, &xEPInfo.bEnable, FTM_TRUE);
	xRet |= FTOM_CGI_getTimeout(pReq, &xEPInfo.ulTimeout, FTM_TRUE); 
	xRet |= FTOM_CGI_getInterval(pReq, &xEPInfo.ulInterval, FTM_TRUE); 
	xRet |= FTOM_CGI_getLimit(pReq, &xEPInfo.xLimit, FTM_TRUE); 
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTOM_CLIENT_EP_set(pClient, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
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
	FTM_EP_TYPE		xEPType;
	FTM_CHAR_PTR	pBuff = NULL;
	cJSON _PTR_		pRoot = NULL;

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
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_ULONG		ulCount = 0;
	FTM_EP_FIELD_TYPE	xFields = FTM_EP_FIELD_EPID;
	FTM_EPID		*pEPIDList = NULL;
	cJSON _PTR_		pRoot = NULL;

	for(i = 0; i < 10 ; i++)
	{
		FTM_CHAR	pTitle[32];

		sprintf(pTitle, "field%d", (i+1)); 

		pValue = pReq->getstr(pReq, pTitle, false);
		if (pValue == NULL)
		{
			break;	
		}

		if (strcasecmp(pValue, "eptype") == 0)
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
	
	pRoot = cJSON_CreateArray();
	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_EP	xEPInfo;
		cJSON _PTR_ pObject;

		xRet = FTOM_CLIENT_EP_get(pClient, pEPIDList[i], &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		xRet = FTOM_CGI_makeEPInfo(&xEPInfo, &pObject, xFields);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToArray(pRoot, pObject);
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

	if (pEPIDList != NULL)
	{
		FTM_MEM_free(pEPIDList);
	}

	return	xRet;
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
	FTM_CHAR_PTR	pBuff;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_ULONG		ulCount = pReq->getint(pReq, "cnt");

	xRet = FTOM_CGI_getEPID(pReq, pEPID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finish;	
	}

	if (ulCount > 100)
	{
		return	FTM_RET_ERROR;	
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulCount);
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, 0, pData, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pRoot, _PTR_ pDataList;

	pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, "ID", pEPID);
	cJSON_AddItemToObject(pRoot, "DATA", pDataList = cJSON_CreateArray());

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject;

		cJSON_AddItemToArray(pDataList, pObject = cJSON_CreateObject());

		FTM_CHAR	pValueString[64];
		FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &pData[i]);

		cJSON_AddStringToObject(pObject, "VALUE", pValueString);
		cJSON_AddNumberToObject(pObject, "TIME", pData[i].ulTime);
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

	if (pData != NULL)
	{
		FTM_MEM_free(pData);
	}

	return	xRet;
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
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_ULONG		ulCount = 0;
	FTM_EPID		*pEPIDList = NULL;

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

	cJSON _PTR_ pRoot;

	pRoot = cJSON_CreateArray();
	
	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_DATA	xEPData;
		FTM_CHAR	pValueString[64];
		cJSON _PTR_ pObject;

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

		cJSON_AddItemToArray(pRoot, pObject = cJSON_CreateObject());
		cJSON_AddStringToObject(pObject, "ID", pEPIDList[i]);
		cJSON_AddStringToObject(pObject, "VALUE", pValueString);
		cJSON_AddNumberToObject(pObject, "TIME", xEPData.ulTime);
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

	if (pEPIDList != NULL)
	{
		FTM_MEM_free(pEPIDList);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_makeEPInfo
(
	FTM_EP_PTR			pEPInfo,
	cJSON _PTR_ _PTR_ 	ppObject,
	FTM_EP_FIELD_TYPE	xFields
)
{
	FTM_RET	xRet;
	cJSON _PTR_	pRoot, _PTR_ pLimit;

	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		goto finish;
	}

	if (xFields & FTM_EP_FIELD_EPID)
	{
		cJSON_AddStringToObject(pRoot, "EPID", pEPInfo->pEPID);
	}

	if (xFields & FTM_EP_FIELD_EPTYPE)
	{
		cJSON_AddStringToObject(pRoot, "TYPE", FTM_EP_typeString(pEPInfo->xType));
	}

	if (xFields & FTM_EP_FIELD_NAME)
	{
		cJSON_AddStringToObject(pRoot, "NAME", pEPInfo->pName);
	}

	if (xFields & FTM_EP_FIELD_UNIT)
	{
		cJSON_AddStringToObject(pRoot, "UNIT", pEPInfo->pUnit);
	}

	if (xFields & FTM_EP_FIELD_INTERVAL)
	{
		cJSON_AddNumberToObject(pRoot, "INTERVAL", pEPInfo->ulInterval);
	}

	if (xFields & FTM_EP_FIELD_DID)
	{
		cJSON_AddStringToObject(pRoot, "DID", pEPInfo->pDID);
	}

	if (xFields & FTM_EP_FIELD_LIMIT)
	{
		cJSON_AddItemToObject(pRoot, "LIMIT", pLimit = cJSON_CreateObject());
		
		switch(pEPInfo->xLimit.xType)
		{
		case	FTM_EP_LIMIT_TYPE_COUNT:
			{
				cJSON_AddStringToObject(pLimit, "TYPE", "COUNT");
				cJSON_AddNumberToObject(pLimit, "COUNT", pEPInfo->xLimit.xParams.ulCount);
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_TIME:
			{
				cJSON _PTR_ pTime;
	
				cJSON_AddStringToObject(pLimit, "TYPE", "TIME");
				cJSON_AddItemToObject(pLimit, "TIME", pTime = cJSON_CreateObject());
				cJSON_AddNumberToObject(pTime, "START", pEPInfo->xLimit.xParams.xTime.ulStart);
				cJSON_AddNumberToObject(pTime, "END", pEPInfo->xLimit.xParams.xTime.ulEnd);
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_HOURS:
			{
				cJSON_AddStringToObject(pLimit, "TYPE", "HOURS");
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_DAYS:
			{
				cJSON_AddStringToObject(pLimit, "TYPE", "DAYS");
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_MONTHS:
			{
				cJSON_AddStringToObject(pLimit, "TYPE", "MONTHS");
			}
			break;
		}
	}

	*ppObject = pRoot;

	return	FTM_RET_OK;

finish:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	xRet;
}

