#include "ftom_service.h"
#include "ftm_mem.h"
#include "ftm_list.h"

static FTM_BOOL FTOM_SERVICE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_LIST_PTR	pServiceList = NULL;

FTM_RET	FTOM_SERVICE_init
(
	FTOM_SERVICE_PTR pServices, 
	FTM_ULONG ulServices
)
{
	FTM_RET				xRet;
	FTM_ULONG			i;

	if (pServiceList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;
	}

	xRet = FTM_LIST_create(&pServiceList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(pServiceList, FTOM_SERVICE_seeker);	
	}

	for(i = 0 ; i < ulServices ; i++)
	{
		if (pServices[i].fCreate != NULL)
		{
			pServices[i].xRet = pServices[i].fCreate(&pServices[i].pData);
		}
		else if (pServices[i].fInit != NULL)
		{
			pServices[i].xRet = pServices[i].fInit(pServices[i].pData);
		}

		if ((pServices[i].fSetCallback != NULL) && (pServices[i].fCallback != NULL))
		{
			pServices[i].fSetCallback(pServices[i].pData, pServices[i].xID, pServices[i].fCallback);	
		}

		FTOM_SERVICE_register(&pServices[i]);
	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_final
(
	FTM_VOID
)
{
	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;

	if (pServiceList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	FTM_LIST_iteratorStart(pServiceList);
	while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
	{
		if (pService->fDestroy != NULL)
		{
			if (pService->pData != NULL)
			{
				pService->fDestroy(&pService->pData);
			}
		}
		else if (pService->fFinal != NULL)
		{
			pService->fFinal(pService->pData);
		}
	}

	xRet = FTM_LIST_destroy(pServiceList);
	if (xRet == FTM_RET_OK)
	{
		pServiceList = NULL;	
	}

	return	FTM_RET_OK;
}


FTM_RET	FTOM_SERVICE_register
(
	FTOM_SERVICE_PTR pService
)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pService != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pTempService;

	if (FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&pService->xType, (FTM_VOID_PTR _PTR_)&pTempService) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXISTS;
	}

	xRet = FTM_LIST_append(pServiceList, pService);
	if (xRet == FTM_RET_OK)
	{
		TRACE("Service[%s] registeration is complete.\n", pService->pName);	
	}
	else
	{
		ERROR2(xRet, "Service[%s] registration failed.\n", pService->pName);	
	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_unregister
(
	FTOM_SERVICE_TYPE xType
)
{
	ASSERT(pServiceList != NULL);
	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;

	if (FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService) != FTM_RET_OK)
	{
		return	FTM_RET_NOT_EXISTS;
	}

	xRet = FTM_LIST_remove(pServiceList, pService);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(pService);
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_SERVICE_count
(
	FTM_ULONG_PTR pulCount
)
{
	ASSERT(pulCount != NULL);

	if (pServiceList == NULL)
	{
		*pulCount = 0;

		return	FTM_RET_OK;
	}
	
	return	FTM_LIST_count(pServiceList, pulCount);
}

FTM_RET FTOM_SERVICE_get
(
	FTOM_SERVICE_TYPE xType, 
	FTOM_SERVICE_PTR _PTR_ ppService
)
{
	ASSERT(ppService != NULL);

	if (pServiceList == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}
	
	return	FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)ppService);
}

FTM_RET FTOM_SERVICE_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_SERVICE_PTR _PTR_ ppService
)
{
	ASSERT(pServiceList != NULL);
	ASSERT(ppService != NULL);

	return	FTM_LIST_getAt(pServiceList, ulIndex, (FTM_VOID_PTR _PTR_)ppService);
}

FTM_RET	FTOM_SERVICE_loadConfig
(
	FTOM_SERVICE_TYPE 	xType,
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fLoadConfig != NULL)
			{
				pService->xRet = pService->fLoadConfig(pService->pData, pConfig);
			}
		}

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService);		
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		if (pService->fLoadConfig == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fLoadConfig(pService->pData, pConfig);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_loadConfigFromFile
(
	FTOM_SERVICE_TYPE xType, 
	FTM_CHAR_PTR pFileName
)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet =FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "SERVER configuration file[%s] load failed\n", pFileName);
		return	xRet;	
	}

	xRet = FTOM_SERVICE_loadConfig(xType, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTOM_SERVICE_saveConfig
(
	FTOM_SERVICE_TYPE 	xType,
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pServiceList != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;

	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fLoadConfig != NULL)
			{
				pService->xRet = pService->fSaveConfig(pService->pData, pConfig);
			}
		}

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService);		
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		if (pService->fLoadConfig == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fSaveConfig(pService->pData, pConfig);

		xRet = pService->xRet;
	}

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTOM_SERVICE_saveConfigToFile
(
	FTOM_SERVICE_TYPE xType, 
	FTM_CHAR_PTR pFileName
)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet =FTM_CONFIG_create(pFileName, &pConfig, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create configuration file[%s].\n", pFileName);
		return	xRet;	
	}

	xRet = FTOM_SERVICE_saveConfig(xType, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTOM_SERVICE_showConfig
(
	FTOM_SERVICE_TYPE xType
)
{
	ASSERT(pServiceList != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fShowConfig != NULL)
			{
				pService->xRet = pService->fShowConfig(pService->pData);
			}
		}

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService);		
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		if (pService->fShowConfig == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fShowConfig(pService->pData);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_start
(
	FTOM_SERVICE_TYPE xType
)
{
	ASSERT(pServiceList != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fStart != NULL)
			{
				pService->xRet = pService->fStart(pService->pData);
			}
		}

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService);		
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		if (pService->fStart == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fStart(pService->pData);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_stop
(
	FTOM_SERVICE_TYPE xType
)
{
	ASSERT(pServiceList != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fStop != NULL)
			{
				TRACE("Service[%s] stop request.\n", pService->pName);
				pService->xRet = pService->fStop(pService->pData);
				TRACE("Service[%s] stopped.\n", pService->pName);
			}
		}

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService);		
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		if (pService->fStop == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fStop(pService->pData);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_sendMessage
(
	FTOM_SERVICE_TYPE 	xType, 
	FTOM_MSG_PTR 		pMsg
)
{
	ASSERT(pServiceList != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fSendMessage != NULL)
			{
				FTOM_MSG_PTR	pNewMsg;

				xRet = FTOM_MSG_copy(pMsg, &pNewMsg);
				if (xRet == FTM_RET_OK)
				{
					pService->xRet = pService->fSendMessage(pService->pData, pNewMsg);
				}
				else
				{
					WARN("Message copy error!\n");	
				}
			}
		}

		FTOM_MSG_destroy(&pMsg);

		xRet = FTM_RET_OK;
	}
	else
	{
		xRet = FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)&pService);		
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		if (pService->fSendMessage == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fSendMessage(pService->pData, pMsg);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_BOOL FTOM_SERVICE_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	
	return	((FTOM_SERVICE_PTR)pElement)->xType == *((FTOM_SERVICE_TYPE_PTR)pIndicator);
}

