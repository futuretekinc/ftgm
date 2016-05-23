#include <stdlib.h>
#include "ftom.h"
#include "ftom_node_snmpc.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

static FTM_RET	FTOM_NODE_MBC_getEPData
(
	FTOM_NODE_MBC_PTR	pMBC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

static FTM_RET	FTOM_NODE_MBC_setEPData
(
	FTOM_NODE_MBC_PTR	pMBC,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	PData
);

FTM_RET	FTOM_NODE_MBC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTOM_NODE_MBC_PTR	pNode;
	

	pNode = (FTOM_NODE_MBC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_MBC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.xDescript.xType		= FTOM_NODE_TYPE_MBC;
	pNode->xCommon.xDescript.fInit		= (FTOM_NODE_INIT)FTOM_NODE_MBC_init;
	pNode->xCommon.xDescript.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_MBC_final;
	pNode->xCommon.xDescript.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_MBC_getEPData;
	pNode->xCommon.xDescript.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_MBC_setEPData;
	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_destroy
(
	FTOM_NODE_MBC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_LIST_final(&(*ppNode)->xCommon.xEPList);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_init
(
	FTOM_NODE_MBC_PTR pNode
)
{
	ASSERT(pNode != NULL);
	
	sem_init(&pNode->xLock, 0, 1);

	FTM_LIST_final(&pNode->xCommon.xEPList);
	pNode->pMB = modbus_new_tcp(pNode->xCommon.xInfo.xOption.xMB.pURL, pNode->xCommon.xInfo.xOption.xMB.ulPort);
	if (pNode->pMB == NULL)
	{
		ERROR("Can't creation MB object!\n");
		return	FTM_RET_MODBUS_ERROR;
	}

	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_final
(
	FTOM_NODE_MBC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->pMB != NULL)
	{
		modbus_free(pNode->pMB);
		pNode->pMB = NULL;
	}

	FTM_LIST_final(&pNode->xCommon.xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_getEPData
(
	FTOM_NODE_MBC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET	nRet;
	FTM_USHORT	usValue;

	if (pNode->pMB == NULL)
	{
		return	FTM_RET_NOT_CONNECTED;
	}

	nRet = modbus_connect(pNode->pMB);
	if (nRet < 0)
	{
		return	FTM_RET_NOT_CONNECTED;
	}

	modbus_read_registers(pNode->pMB, 0x1000, 1, &usValue);

	modbus_close(pNode->pMB);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_setEPData
(
	FTOM_NODE_MBC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET	nRet;
	FTM_USHORT	usValue = 0;

	if (pNode->pMB == NULL)
	{
		return	FTM_RET_NOT_CONNECTED;
	}

	nRet = modbus_connect(pNode->pMB);
	if (nRet < 0)
	{
		return	FTM_RET_NOT_CONNECTED;
	}

	modbus_write_register(pNode->pMB, 0x1000, usValue);

	modbus_close(pNode->pMB);

	return	FTM_RET_OK;
}

