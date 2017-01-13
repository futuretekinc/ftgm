#include "ftm_sim.h"

FTM_RET	FTM_SIM_copy
(
	FTM_SIM_PTR	pSrc,
	FTM_SIM_PTR	_PTR_ ppDest
)
{
	ASSERT(pSrc != NULL);
	ASSERT(ppDest != NULL);

	FTM_SIM_PTR	pNew;

	pNew = (FTM_SIM_PTR)FTM_MEM_malloc(pSrc->ulLen);
	if(pNew == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNew, pSrc, pSrc->ulLen);

	*ppDest = pNew;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIM_destroy
(
	FTM_SIM_PTR	_PTR_ ppSIM
)
{
	ASSERT(*ppSIM != NULL);
	
	FTM_MEM_free(*ppSIM);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIM_createConnectionStatus
(
	FTM_ULONG	xSenderID,
	FTM_BOOL	bConnected,
	FTM_SIM_PTR _PTR_ ppSIM
)
{
	ASSERT(ppSIM != NULL);
	FTM_RET	xRet;
	FTM_SIM_NOTIFY_CONNECTION_STATUS_PTR	pSIM;

	pSIM = (FTM_SIM_NOTIFY_CONNECTION_STATUS_PTR)FTM_MEM_malloc(sizeof(FTM_SIM_NOTIFY_CONNECTION_STATUS));
	if (pSIM == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Failed to create service interface message!\n");
		return	xRet;
	}

	pSIM->xType     = FTM_SIM_TYPE_CONNECTION_STATUS;
	pSIM->ulLen     = sizeof(FTM_SIM_NOTIFY_CONNECTION_STATUS);
   	pSIM->xSenderID = xSenderID;
	pSIM->bConnected= bConnected;
	
	*ppSIM = (FTM_SIM_PTR)pSIM;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SIM_createNetStat
(
	FTM_ULONG	xSenderID,
	FTM_BOOL	bConnected,
	FTM_SIM_PTR _PTR_ ppSIM
)
{
	ASSERT(ppSIM != NULL);
	FTM_RET	xRet;
	FTM_SIM_NOTIFY_NET_STAT_PTR	pSIM;

	pSIM = (FTM_SIM_NOTIFY_NET_STAT_PTR)FTM_MEM_malloc(sizeof(FTM_SIM_NOTIFY_NET_STAT));
	if (pSIM == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Failed to create service interface message!\n");
		return	xRet;
	}

	pSIM->xType     = FTM_SIM_TYPE_NET_STAT;
	pSIM->ulLen     = sizeof(FTM_SIM_NOTIFY_NET_STAT);
   	pSIM->xSenderID = xSenderID;
	pSIM->bConnected= bConnected;
	
	*ppSIM = (FTM_SIM_PTR)pSIM;

	return	FTM_RET_OK;
}
		
