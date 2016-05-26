#include <stdlib.h>
#include <semaphore.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftm_timer.h"
#include "ftom_node_class.h"
#include "modbus/modbus-tcp.h"
#include "ftm_lock.h"

static
FTM_RET	FTOM_NODE_MBC_HHTW_init
(
	FTOM_NODE_MBC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_init(&pNode->xLock);
	FTM_LIST_init(&pNode->xCommon.xEPList);

	pNode->xCommon.xState = FTOM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

static
FTM_RET	FTOM_NODE_MBC_HHTW_final
(
	FTOM_NODE_MBC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);
	FTM_LOCK_final(&pNode->xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_HHTW_prestart
(
	FTOM_NODE_MBC_PTR	pNode
)
{
	ASSERT(pNode != NULL);
	FTM_INT	nRet;

	pNode->pMB = modbus_new_tcp(pNode->xCommon.xInfo.xOption.xMB.pURL, pNode->xCommon.xInfo.xOption.xMB.ulPort);
	if (pNode->pMB == NULL)
	{
		ERROR("Can't creation MB object!\n");
		return	FTM_RET_MODBUS_ERROR;
	}

	struct timeval timeout;

	timeout.tv_sec = pNode->xCommon.xInfo.ulTimeout;
	timeout.tv_usec = 0;

	modbus_set_debug(pNode->pMB,1);
	modbus_set_response_timeout(pNode->pMB,&timeout);
	modbus_set_slave(pNode->pMB, pNode->xCommon.xInfo.xOption.xMB.ulSlaveID);

	nRet = modbus_connect(pNode->pMB);
	if (nRet < 0)
	{
		ERROR("Node[%s] failed connection to %s:%d!\n", pNode->xCommon.xInfo.pDID, pNode->xCommon.xInfo.xOption.xMB.pURL, pNode->xCommon.xInfo.xOption.xMB.ulPort);
		return	FTM_RET_NOT_CONNECTED;
	}

	TRACE("Node[%s] connected : pMB = %08x!\n",pNode->xCommon.xInfo.pDID, pNode->pMB);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_HHTW_poststop
(
	FTOM_NODE_MBC_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	if (pNode->pMB != NULL)
	{
		modbus_close(pNode->pMB);
		modbus_free(pNode->pMB);
		pNode->pMB = NULL;
	}

	return	FTM_RET_OK;
}

static 
FTM_RET	FTOM_NODE_MBC_HHTW_get
(
	FTOM_NODE_MBC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	FTM_INT		nRet;
	FTM_USHORT	usValue = 0;
	FTM_USHORT	usRegID = 0;
	FTM_EP_DATA_TYPE	xDataType;

	if (pNode->pMB == NULL)
	{
		return	FTM_RET_NOT_CONNECTED;	
	}

	FTM_LOCK_set(&pNode->xLock);

	switch(pEP->xInfo.xDEPID)
	{
	case	FTM_EP_TYPE_PRESSURE | 1: usRegID = 9300; break;
	case	FTM_EP_TYPE_PRESSURE | 2: usRegID = 9302; break;
	case	FTM_EP_TYPE_PRESSURE | 3: usRegID = 9304; break;
	case	FTM_EP_TYPE_PRESSURE | 4: usRegID = 9310; break;
	case	FTM_EP_TYPE_PRESSURE | 5: usRegID = 9312; break;
	case	FTM_EP_TYPE_AI | 1: usRegID = 9306; break;
	case	FTM_EP_TYPE_AI | 2: usRegID = 9308; break;
	}

	TRACE("MODBUS get(%d)\n", usRegID);
	nRet = modbus_read_registers(pNode->pMB, usRegID, 1, &usValue);
	if (nRet < 0)
	{
		ERROR("MODBUS read input registers failed[%d]!\n", nRet);
		xRet = FTM_RET_OBJECT_NOT_FOUND;
		goto finish;
	}

	xRet = FTOM_EP_getDataType(pEP, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Invalid data type\n");
		xRet = FTM_RET_INVALID_TYPE;
		goto finish;
	}

	pData->ulTime = time(NULL);
	pData->xState = FTM_EP_DATA_STATE_VALID;
	pData->xType  = FTM_VALUE_TYPE_FLOAT;
	xRet = FTM_VALUE_initFLOAT(&pData->xValue, (FTM_FLOAT)usValue / 100);

finish:
	
	FTM_LOCK_reset(&pNode->xLock);

	return	xRet;
}

static
FTM_RET	FTOM_NODE_MBC_HHTW_set
(
	FTOM_NODE_MBC_PTR	pNode,
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	if (pNode->pMB == NULL)
	{
		return	FTM_RET_NOT_CONNECTED;	
	}

	return	FTM_RET_OK;
}

FTOM_NODE_CLASS	xNodeModbusClientHHTW = 
{
	.pModel = "hhtw comp",
	.xType		= FTOM_NODE_TYPE_MBC,
	.fCreate	= (FTOM_NODE_CREATE)FTOM_NODE_MBC_create,
	.fDestroy	= (FTOM_NODE_DESTROY)FTOM_NODE_MBC_destroy,
	.fInit		= (FTOM_NODE_INIT)FTOM_NODE_MBC_HHTW_init,
	.fFinal		= (FTOM_NODE_FINAL)FTOM_NODE_MBC_HHTW_final,
	.fPrestart	= (FTOM_NODE_PRESTART)FTOM_NODE_MBC_HHTW_prestart,
	.fPoststop	= (FTOM_NODE_POSTSTOP)FTOM_NODE_MBC_HHTW_poststop,
	.fGetEPData	= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_MBC_HHTW_get,
	.fSetEPData	= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_MBC_HHTW_set
};

