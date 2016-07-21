#include <string.h>
#include "ftom.h"
#include "ftom_tp_restapi_gateway.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT


FTM_RET	FTOM_TP_GATEWAY_create
(
	FTOM_TP_GATEWAY_PTR _PTR_ ppGateway
)
{
	ASSERT(ppGateway != NULL);
	FTM_RET	xRet;

	*ppGateway = (FTOM_TP_GATEWAY_PTR)FTM_MEM_malloc(sizeof(FTOM_TP_GATEWAY));
	if (*ppGateway == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_TP_GATEWAY_init(*ppGateway);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(*ppGateway);	
		*ppGateway = NULL;
	}

	return	xRet;
}

FTM_RET	FTOM_TP_GATEWAY_destroy
(
	FTOM_TP_GATEWAY_PTR _PTR_ ppGateway
)
{
	ASSERT(ppGateway != NULL);
	FTM_RET	xRet;

	xRet = FTOM_TP_GATEWAY_final(*ppGateway);
	if (xRet == FTM_RET_OK)
	{
		FTM_MEM_free(*ppGateway);
		*ppGateway = NULL;
	}

	return	FTM_RET_OK;

}

FTM_RET	FTOM_TP_GATEWAY_init
(
	FTOM_TP_GATEWAY_PTR	pGateway
)
{
	ASSERT(pGateway != NULL);
	FTM_RET	xRet;

	memset(pGateway, 0, sizeof(FTOM_TP_GATEWAY));

	xRet = FTM_LIST_create(&pGateway->pDeviceList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can't creation device list!\n");
		return	xRet;	
	}

	xRet = FTM_LIST_create(&pGateway->pSensorList);
	if (xRet != FTM_RET_OK)
	{
		FTM_LIST_destroy(pGateway->pDeviceList);
		pGateway->pDeviceList = NULL;
		ERROR2(xRet, "Can't creation sensor list!\n");
		return	xRet;	
	}

	return	xRet;
}

FTM_RET	FTOM_TP_GATEWAY_final
(
	FTOM_TP_GATEWAY_PTR pGateway
)
{
	ASSERT(pGateway != NULL);
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTM_LIST_count(pGateway->pDeviceList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR_PTR	pDeviceID = NULL;

		xRet =FTM_LIST_getAt(pGateway->pDeviceList, i, (FTM_VOID_PTR _PTR_)&pDeviceID);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pDeviceID);	
		}
	
	}

	FTM_LIST_destroy(pGateway->pDeviceList);
	pGateway->pDeviceList = NULL;

	xRet = FTM_LIST_count(pGateway->pSensorList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR_PTR	pSensorID = NULL;

		xRet =FTM_LIST_getAt(pGateway->pSensorList, i, (FTM_VOID_PTR _PTR_)&pSensorID);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pSensorID);	
		}
	
	}

	FTM_LIST_destroy(pGateway->pSensorList);
	pGateway->pSensorList = NULL;

	return	FTM_RET_OK;
}
