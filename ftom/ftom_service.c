#include "ftom_service.h"
#include "ftm_mem.h"
#include "ftm_list.h"

static FTM_BOOL FTOM_SERVICE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_LIST_PTR	pServiceList = NULL;

FTM_RET	FTOM_SERVICE_init(FTOM_PTR pOM, FTOM_SERVICE_PTR pServices, FTM_ULONG ulServices)
{
	FTM_RET				xRet;
	FTM_ULONG			i;
	FTOM_SERVICE_PTR	pService;

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
		FTOM_SERVICE_register(&pServices[i]);	
	}

	FTM_LIST_iteratorStart(pServiceList);
	while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
	{
		if (pService->fInit != NULL)
		{
			pService->xRet = pService->fInit(pService->pData, pOM);
		}

		if ((pService->fSetCallback != NULL) && (pService->fCallback != NULL))
		{
			pService->fSetCallback(pService->pData, pService->xID, pService->fCallback);	
		}
	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_final(FTM_VOID)
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
		if (pService->fFinal != NULL)
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


FTM_RET	FTOM_SERVICE_register(FTOM_SERVICE_PTR pService)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pService != NULL);

	FTOM_SERVICE_PTR pTempService;

	if (FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&pService->xType, (FTM_VOID_PTR _PTR_)&pTempService) == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXISTS;
	}

	return	FTM_LIST_append(pServiceList, pService);
}

FTM_RET	FTOM_SERVICE_unregister(FTOM_SERVICE_TYPE xType)
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

FTM_RET FTOM_SERVICE_count(FTM_ULONG_PTR pulCount)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pServiceList, pulCount);
}

FTM_RET FTOM_SERVICE_get(FTOM_SERVICE_TYPE xType, FTOM_SERVICE_PTR _PTR_ ppService)
{
	ASSERT(pServiceList != NULL);
	ASSERT(ppService != NULL);

	return	FTM_LIST_get(pServiceList, (FTM_VOID_PTR)&xType, (FTM_VOID_PTR _PTR_)ppService);
}

FTM_RET FTOM_SERVICE_getAt(FTM_ULONG ulIndex, FTOM_SERVICE_PTR _PTR_ ppService)
{
	ASSERT(pServiceList != NULL);
	ASSERT(ppService != NULL);

	return	FTM_LIST_getAt(pServiceList, ulIndex, (FTM_VOID_PTR _PTR_)ppService);
}

FTM_RET	FTOM_SERVICE_loadFromFile(FTOM_SERVICE_TYPE xType, FTM_CHAR_PTR pFileName)
{
	ASSERT(pServiceList != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fLoadFromFile != NULL)
			{
				pService->xRet = pService->fLoadFromFile(pService->pData, pFileName);
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

		if (pService->fLoadFromFile == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fLoadFromFile(pService->pData, pFileName);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_RET	FTOM_SERVICE_showConfig(FTOM_SERVICE_TYPE xType)
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

FTM_RET	FTOM_SERVICE_start(FTOM_SERVICE_TYPE xType)
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

FTM_RET	FTOM_SERVICE_stop(FTOM_SERVICE_TYPE xType)
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

FTM_RET	FTOM_SERVICE_notify(FTOM_SERVICE_TYPE xType, FTOM_MSG_PTR pMsg)
{
	ASSERT(pServiceList != NULL);

	FTM_RET				xRet;
	FTOM_SERVICE_PTR	pService;
	
	if (xType == FTOM_SERVICE_ALL)
	{
		FTM_LIST_iteratorStart(pServiceList);
		while(FTM_LIST_iteratorNext(pServiceList, (FTM_VOID_PTR _PTR_)&pService) == FTM_RET_OK)
		{
			if (pService->fNotify != NULL)
			{
				pService->xRet = pService->fNotify(pService->pData, pMsg);
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

		if (pService->fNotify == NULL)
		{
			return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
		}

		pService->xRet = pService->fNotify(pService->pData, pMsg);
		xRet = pService->xRet;

	}

	return	xRet;
}

FTM_BOOL FTOM_SERVICE_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);
	
	return	((FTOM_SERVICE_PTR)pElement)->xType == *((FTOM_SERVICE_TYPE_PTR)pIndicator);
}

