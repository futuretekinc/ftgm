#include <stdlib.h>
#include "ftom.h"
#include "ftom_node_snmpc.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

static 
FTM_RET	FTOM_NODE_MBC_TEST_get
(
	FTOM_NODE_MBC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_INT		nRet;
	FTM_USHORT	usValue = 0;

	if (pNode->pMB == NULL)
	{
		return	FTM_RET_NOT_CONNECTED;	
	}

	nRet = modbus_read_input_registers(pNode->pMB, 0, 1, &usValue);
	if (nRet < 0)
	{
		ERROR("MODBUS read input registers failed[%d]!\n", nRet);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	TRACE("MODBUS Read value : %d\n", usValue);
	switch(pData->xValue.xType)
	{
	case	FTM_VALUE_TYPE_INT:
		FTM_VALUE_setINT(&pData->xValue, (FTM_INT)usValue);
		break;

	case	FTM_VALUE_TYPE_ULONG:
		FTM_VALUE_setINT(&pData->xValue, (FTM_ULONG)usValue);
		break;

	case	FTM_VALUE_TYPE_FLOAT:
		FTM_VALUE_setFLOAT(&pData->xValue, (FTM_FLOAT)usValue);
		break;

	case	FTM_VALUE_TYPE_BOOL:
		FTM_VALUE_setBOOL(&pData->xValue, (FTM_BOOL)usValue);
		break;

	default:	
		return	FTM_RET_INVALID_TYPE;
	}

	return	FTM_RET_OK;
}


FTM_RET	FTOM_NODE_MBC_TEST_set
(
	FTOM_NODE_MBC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);


	FTM_ULONG	ulValue;

	if (pNode->pMB == NULL)
	{
		return	FTM_RET_NOT_CONNECTED;	
	}

	FTM_VALUE_getULONG(&pData->xValue, &ulValue);

	modbus_write_register(pNode->pMB, 0, ulValue);

	return	FTM_RET_OK;
}

FTOM_NODE_MBC_DESCRIPT	pMODBUSClient[] =
{
	{
		.pModel	= "test",
		.fGet	= FTOM_NODE_MBC_TEST_get,
		.fSet	= FTOM_NODE_MBC_TEST_set
	}
};

