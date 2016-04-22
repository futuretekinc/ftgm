#include "ftom_cgi.h"
#include "ftom_client.h"
#include "ftm_json.h"


static 
FTM_RET	FTOM_CGI_makeEPInfo
(
	FTM_EP_PTR		pEPInfo,
	FTM_JSON_VALUE_PTR _PTR_ ppObject
);

FTM_RET	FTOM_CGI_getEP(FTOM_CLIENT_PTR pClient, qentry_t *pReq)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_EP			xEPInfo;
	FTM_ULONG		ulBuffLen;
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_JSON_VALUE_PTR	pObject = NULL;


	FTM_CHAR_PTR	pEPID = pReq->getstr(pReq, "id", false);

	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTOM_CGI_makeEPInfo(&xEPInfo, &pObject);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_buffSize((FTM_JSON_VALUE_PTR)pObject, &ulBuffLen);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(ulBuffLen + 1);
	if (pBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		goto finish;
	}

	FTM_JSON_snprint(pBuff, ulBuffLen + 1, (FTM_JSON_VALUE_PTR)pObject);

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);
	TRACE("%s", pBuff);

	xRet = FTM_RET_OK;

finish:
	if (pBuff != NULL)
	{
		FTM_MEM_free(pBuff);	
	}

	if (pObject != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pObject);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_getEPList(FTOM_CLIENT_PTR pClient, qentry_t *pReq)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_JSON_ARRAY_PTR	pJSON;
	FTM_JSON_VALUE_PTR	pJSONObject;
	FTM_RET			xRet;
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_ULONG		ulBuffLen = 0;
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

	xRet = FTM_JSON_createArray(ulCount, &pJSON);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_EP	xEPInfo;

		xRet = FTOM_CLIENT_EP_get(pClient, pEPIDList[i], &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		xRet = FTOM_CGI_makeEPInfo(&xEPInfo, &pJSONObject);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_JSON_ARRAY_setElement(pJSON, pJSONObject);	
			if (xRet != FTM_RET_OK)
			{
				FTM_JSON_destroy(&pJSONObject);	
			}
			pJSONObject = NULL;	
		}
	}

	xRet = FTM_JSON_buffSize((FTM_JSON_VALUE_PTR)pJSON, &ulBuffLen);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(ulBuffLen + 1);
	if (pBuff == NULL)
	{
		goto finish;	
	}

	xRet = FTM_JSON_snprint(pBuff, ulBuffLen + 1, (FTM_JSON_VALUE_PTR)pJSON);
	if (pBuff == NULL)
	{
		goto finish;	
	}


	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);
	TRACE("%s", pBuff);
	xRet = FTM_RET_OK;

finish:
	if (pBuff != NULL)
	{
		free(pBuff);	
	}

	if (pEPIDList != NULL)
	{
		FTM_MEM_free(pEPIDList);
	}

	if (pJSON != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSON);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_getEPData(FTOM_CLIENT_PTR pClient, qentry_t *pReq)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_EP_DATA_PTR	pData;
	FTM_CHAR_PTR	pBuff;
	FTM_CHAR_PTR	pEPID = pReq->getstr(pReq, "id", false);
	FTM_ULONG		ulCount = pReq->getint(pReq, "cnt");

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

	FTM_JSON_OBJECT_PTR	pJSON = NULL;
	FTM_JSON_ARRAY_PTR	pJSONArray = NULL;
	FTM_JSON_VALUE_PTR	pJSONValue = NULL;
	FTM_JSON_OBJECT_PTR	pJSONData = NULL;

	xRet = FTM_JSON_createObject(2, &pJSON);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_JSON_createString(pEPID, &pJSONValue);	
	if (xRet != FTM_RET_OK)
	{
		goto finish;			
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "ID", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;			
	}
	pJSONValue = NULL;

	xRet = FTM_JSON_createArray(ulCount, &pJSONArray);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	for(FTM_INT i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_JSON_createObject(2, &pJSONData);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		FTM_CHAR	pValueString[64];
		FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &pData[i]);
	
		xRet = FTM_JSON_createString(pValueString, &pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);
			continue;
		}


		xRet = FTM_JSON_OBJECT_setPair(pJSONData, "VALUE", pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);
			FTM_JSON_destroy(&pJSONValue);
			continue;
		}
		pJSONValue = NULL;
		

		xRet = FTM_JSON_createNumber(pData[i].ulTime, &pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);
			continue;
		}

		xRet = FTM_JSON_OBJECT_setPair(pJSONData, "TIME", pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);
			FTM_JSON_destroy(&pJSONValue);
			continue;
		}
		pJSONValue = NULL;

		xRet = FTM_JSON_ARRAY_setElement(pJSONArray, (FTM_JSON_VALUE_PTR)pJSONData);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);
			continue;
		}
		pJSONData = NULL;
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "DATA", (FTM_JSON_VALUE_PTR)pJSONArray);
	if (xRet != FTM_RET_OK)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONArray);
		goto finish;	
	}
	pJSONArray = NULL;

	FTM_ULONG	ulBuffLen;

	xRet = FTM_JSON_buffSize((FTM_JSON_VALUE_PTR)pJSON, &ulBuffLen);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	
	pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(ulBuffLen + 1);
	if (pBuff == NULL)
	{
		goto finish;	
	}

	FTM_JSON_snprint(pBuff, ulBuffLen + 1, (FTM_JSON_VALUE_PTR)pJSON);

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);
	TRACE("%s", pBuff);

	xRet = FTM_RET_OK;

finish:
	if (pJSON != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSON);	
	}

	if (pJSONArray != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONArray);	
	}

	if (pJSONValue != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONValue);	
	}

	if (pJSONData != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);	
	}

	if (pBuff != NULL)
	{
		FTM_MEM_free(pBuff);
	}

	if (pData != NULL)
	{
		FTM_MEM_free(pData);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_getEPDataLast(FTOM_CLIENT_PTR pClient, qentry_t *pReq)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET			xRet;
	FTM_CHAR_PTR	pBuff = NULL;
	FTM_ULONG		ulBuffLen;
	FTM_ULONG		ulCount = 0;
	FTM_EPID		*pEPIDList = NULL;
	FTM_JSON_ARRAY_PTR	pJSON = NULL;
	FTM_JSON_VALUE_PTR	pJSONObject = NULL;
	FTM_JSON_VALUE_PTR	pJSONValue = NULL;

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

	xRet = FTM_JSON_createArray(ulCount, &pJSON);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}
	
	for(int i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_DATA	xEPData;
		FTM_CHAR	pValueString[64];
		
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

		xRet = FTM_JSON_createObject(3, (FTM_JSON_OBJECT_PTR _PTR_)&pJSONObject);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		xRet = FTM_JSON_createString(pEPIDList[i], &pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			continue;
		}
		
		xRet = FTM_JSON_OBJECT_setPair((FTM_JSON_OBJECT_PTR)pJSONObject, "ID", pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONValue);
			continue;
		}

		xRet = FTM_JSON_createString(pValueString, &pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			continue;
		}
		
		xRet = FTM_JSON_OBJECT_setPair((FTM_JSON_OBJECT_PTR)pJSONObject, "VALUE", pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONValue);
			continue;
		}

		xRet = FTM_JSON_createNumber(xEPData.ulTime, &pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			continue;
		}
		
		xRet = FTM_JSON_OBJECT_setPair((FTM_JSON_OBJECT_PTR)pJSONObject, "TIME", pJSONValue);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONValue);
			continue;
		}
		
		xRet = FTM_JSON_ARRAY_setElement(pJSON, pJSONObject);
		if (xRet != FTM_RET_OK)
		{
			FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONObject);
			continue;
		}
	}

	xRet = FTM_JSON_buffSize((FTM_JSON_VALUE_PTR)pJSON, &ulBuffLen);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(ulBuffLen + 1);
	if (pBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		goto finish;
	}

	xRet = FTM_JSON_snprint(pBuff, ulBuffLen + 1, (FTM_JSON_VALUE_PTR)pJSON);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);
	TRACE("%s", pBuff);
	xRet = FTM_RET_OK;

finish:
	if (pBuff != NULL)
	{
		free(pBuff);	
	}

	if (pEPIDList != NULL)
	{
		FTM_MEM_free(pEPIDList);
	}

	if (pJSON != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSON);
	}

	return	xRet;
}

FTM_RET	FTOM_CGI_makeEPInfo
(
	FTM_EP_PTR		pEPInfo,
	FTM_JSON_VALUE_PTR _PTR_ ppObject
)
{
	FTM_RET	xRet;
	FTM_JSON_OBJECT_PTR	pJSON = NULL;
	FTM_JSON_OBJECT_PTR	pJSONObject = NULL;
	FTM_JSON_VALUE_PTR	pJSONValue = NULL;
	FTM_JSON_OBJECT_PTR	pJSONData = NULL;

	xRet = FTM_JSON_createObject(7, &pJSON);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_createString(pEPInfo->pEPID, &pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "ID", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;

	xRet = FTM_JSON_createString(FTM_EP_typeString(pEPInfo->xType), &pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "TYPE", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;

	xRet = FTM_JSON_createString(pEPInfo->pName, &pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "NAME", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;

	xRet = FTM_JSON_createString(pEPInfo->pUnit, &pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "UNIT", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;


	xRet = FTM_JSON_createNumber(pEPInfo->ulInterval, &pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "INTERVAL", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;

	xRet = FTM_JSON_createString(pEPInfo->pDID, &pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "DID", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;

	xRet = FTM_JSON_createObject(4, &pJSONObject);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	switch(pEPInfo->xLimit.xType)
	{
	case	FTM_EP_LIMIT_TYPE_COUNT:
		{
			xRet = FTM_JSON_createString("COUNT", &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_TIME:
		{
			xRet = FTM_JSON_createString("TIME", &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_HOURS:
		{
			xRet = FTM_JSON_createString("HOURS", &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_DAYS:
		{
			xRet = FTM_JSON_createString("DAYS", &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
		}
		break;

	case	FTM_EP_LIMIT_TYPE_MONTHS:
		{
			xRet = FTM_JSON_createString("MONTHS", &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
		}
		break;
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSONObject, "TYPE", pJSONValue);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONValue = NULL;

	switch(pEPInfo->xLimit.xType)
	{
	case	FTM_EP_LIMIT_TYPE_COUNT:
		{
			xRet = FTM_JSON_createNumber(pEPInfo->xLimit.xParams.ulCount, &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}

			xRet = FTM_JSON_OBJECT_setPair(pJSONObject, "COUNT", pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
			pJSONValue = NULL;

		}
		break;

	case	FTM_EP_LIMIT_TYPE_TIME:
		{
			xRet = FTM_JSON_createNumber(pEPInfo->xLimit.xParams.xTime.ulStart, &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}

			xRet = FTM_JSON_OBJECT_setPair(pJSONObject, "START", pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
			pJSONValue = NULL;

			xRet = FTM_JSON_createNumber(pEPInfo->xLimit.xParams.xTime.ulEnd, &pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}

			xRet = FTM_JSON_OBJECT_setPair(pJSONObject, "END", pJSONValue);
			if (xRet != FTM_RET_OK)
			{
				goto finish;	
			}
			pJSONValue = NULL;
		}
		break;

	default:
		break;
	}

	xRet = FTM_JSON_OBJECT_setPair(pJSON, "LIMIT", (FTM_JSON_VALUE_PTR)pJSONObject);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}
	pJSONObject = NULL;

	*ppObject = (FTM_JSON_VALUE_PTR)pJSON;

	return	FTM_RET_OK;

finish:
	if (pJSON != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSON);
	}

	if (pJSONData != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONData);
	}

	if (pJSONValue != NULL)
	{
		FTM_JSON_destroy((FTM_JSON_VALUE_PTR _PTR_)&pJSONValue);
	}

	return	xRet;
}

