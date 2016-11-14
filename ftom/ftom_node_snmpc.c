#include <string.h>
#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_snmpc.h"
#include "ftom_node_fte.h"
#include "ftom_node_ftm.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_NODE

FTOM_NODE_CLASS	xNodeClassGeneralSNMP = 
{
	.pModel 		= "general",
	.xType			= FTM_NODE_TYPE_SNMP,
	.fCreate		= (FTOM_NODE_CREATE)FTOM_NODE_SNMPC_create,
	.fDestroy		= (FTOM_NODE_DESTROY)FTOM_NODE_SNMPC_destroy,
	.fInit			= (FTOM_NODE_INIT)FTOM_NODE_SNMPC_init,
	.fFinal			= (FTOM_NODE_FINAL)FTOM_NODE_SNMPC_final,
	.fPrestart		= (FTOM_NODE_PRESTART)FTOM_NODE_SNMPC_prestart,
	.fPrestop		= (FTOM_NODE_PRESTOP)FTOM_NODE_SNMPC_prestop,

	.fGetEPCount	= (FTOM_NODE_GET_EP_COUNT)FTOM_NODE_SNMPC_getEPCount,
	.fGetEPID		= (FTOM_NODE_GET_EP_ID)FTOM_NODE_SNMPC_getEPID,
	.fGetEPName		= (FTOM_NODE_GET_EP_NAME)FTOM_NODE_SNMPC_getEPName,
	.fGetEPState	= (FTOM_NODE_GET_EP_STATE)FTOM_NODE_SNMPC_getEPState,
	.fGetEPInterval	= (FTOM_NODE_GET_EP_INTERVAL)FTOM_NODE_SNMPC_getEPUpdateInterval,
	.fGetEPData		= (FTOM_NODE_GET_EP_DATA)FTOM_NODE_SNMPC_getEPData,
	.fGetEPDataAsync= (FTOM_NODE_GET_EP_DATA_ASYNC)FTOM_NODE_SNMPC_getEPDataAsync,

	.fSet			= (FTOM_NODE_SET)FTOM_NODE_SNMPC_set,
	.fSetEPData		= (FTOM_NODE_SET_EP_DATA)FTOM_NODE_SNMPC_setEPData,
	.fSetEPDataAsync= (FTOM_NODE_SET_EP_DATA_ASYNC)FTOM_NODE_SNMPC_setEPDataAsync,

	.fAttachEP		= (FTOM_NODE_ATTACH_EP)FTOM_NODE_SNMPC_attachEP,
	.fDetachEP		= (FTOM_NODE_DETACH_EP)FTOM_NODE_SNMPC_detachEP,

	.fPrintOpts		= (FTOM_NODE_PRINT_OPTS)FTOM_NODE_SNMPC_printOpts
};

static 
FTOM_NODE_CLASS_PTR	pClasses[] =
{
	&xNodeClassGeneralSNMP,
	&xNodeClassFTE,
	&xNodeClassFTM
};

FTM_RET	FTOM_NODE_SNMPC_getClass
(
	FTM_CHAR_PTR	pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
)
{
	FTM_INT	i;

	if (strlen(pModel) == 0)
	{
		*ppClass = &xNodeClassGeneralSNMP;	
		return	FTM_RET_OK;
	}

	for(i = 0 ; i < sizeof(pClasses) / sizeof(FTOM_NODE_CLASS_PTR) ; i++)
	{
		if(strcasecmp(pModel, pClasses[i]->pModel) == 0)
		{
			*ppClass = pClasses[i];	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTOM_NODE_SNMPC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);
	
	FTM_RET	xRet;
	FTOM_NODE_SNMPC_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	xRet = FTOM_NODE_SNMPC_getClass(pInfo->pModel, &pClass);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Class[%s] not found!\n", pInfo->pModel);
		return	xRet;
	}

	pNode = (FTOM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR2(xRet,"Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.pClass = pClass;
	if (strlen(pNode->xCommon.xInfo.pName) == 0)
	{
		snprintf(pNode->xCommon.xInfo.pName, FTM_NAME_LEN, "node%s", pNode->xCommon.xInfo.pDID);
	}

	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_destroy
(
	FTOM_NODE_SNMPC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_init
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_RET				xRet;
	FTM_CHAR			pModel[FTM_NAME_LEN+1];
	FTM_VALUE			xValue;
	FTM_BOOL			bValid = FTM_TRUE;
	FTM_SNMP_OID		xOID;
 	FTM_CHAR			pBuff[64];
	FTOM_SERVICE_PTR 	pService;

	TRACE("called\n");
	if (strlen(pNode->pIP) == 0)
	{
		if (strlen(pNode->xCommon.xInfo.xOption.xSNMP.pURL) == 0)
		{
			xRet = FTM_RET_NOT_INITIALIZED;
			ERROR2(xRet, "The node[%s] is not initialized.\n", pNode->xCommon.xInfo.pDID);
			return	xRet;
		}

		strncpy(pNode->pIP, pNode->xCommon.xInfo.xOption.xSNMP.pURL, FTM_URL_LEN);
	}

	TRACE("NODE[%s] IP : %s\n", pNode->xCommon.xInfo.pDID, pNode->pIP);
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "SNNP Client not supported!\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize value!\n");
		return	xRet;	
	}

	sprintf(pBuff, "%s:prodModel.0", pNode->xCommon.xInfo.xOption.xSNMP.pMIB);
	xRet = FTOM_SNMPC_getOID(pBuff, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID[%s]\n", pBuff);	
		goto finish;
	}

	xRet = FTOM_SNMPC_get( 
				pService->pData,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_VALUE_getSTRING(&xValue, pModel, FTM_NAME_LEN);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to read model from value!\n");
		goto finish;	
	}

	if (strlen(pNode->xCommon.xInfo.pModel) == 0)
	{
		strncpy(pNode->xCommon.xInfo.pModel, pModel, FTM_NAME_LEN);
	}

	if (pNode->pSNMPC == NULL)
	{
#if 0
		FTOM_SNMPC_PTR		pSNMPC = NULL;

		xRet = FTOM_SNMPC_create(&pSNMPC);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to create SNMP Client!\n");
			return	xRet;	
		}

		FTOM_SNMPC_setConfig(pSNMPC, &((FTOM_SNMPC_PTR)pService->pData)->xConfig);
		sprintf(pSNMPC->xConfig.pName, "ftom:%s", pNode->xCommon.xInfo.pDID);

		pNode->pSNMPC = pSNMPC;
#else
		pNode->pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
#endif
	}

finish:

	FTM_VALUE_final(&xValue);

	return	xRet;
}

FTM_RET	FTOM_NODE_SNMPC_prestart
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_set(&pNode->xCommon.xLock);

	FTM_LOCK_reset(&pNode->xCommon.xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_prestop
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_set(&pNode->xCommon.xLock);
	snmp_timeout();
	FTM_LOCK_reset(&pNode->xCommon.xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_final
(
	FTOM_NODE_SNMPC_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_LOCK_set(&pNode->xCommon.xLock);

	if (pNode->pSNMPC != NULL)
	{
		pNode->pSNMPC = NULL;
#if 0
		FTM_RET	xRet;
		xRet = FTOM_SNMPC_destroy(&pNode->pSNMPC);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy SNMPC!\n");	
		}
#endif
	}

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_LOCK_reset(&pNode->xCommon.xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_getModel
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_CHAR_PTR		pModel,
	FTM_ULONG			ulMaxLen
)
{
	FTM_RET	xRet;
	FTM_VALUE			xValue;
	FTM_BOOL			bValid = FTM_TRUE;
	FTOM_SNMPC_PTR		pSNMPC = NULL;
	FTM_SNMP_OID		xOID;
 	FTM_CHAR			pBuff[64];

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		return	FTM_RET_SNMP_TIMEOUT;
	} 

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize value!\n");
		return	xRet;	
	}

	sprintf(pBuff, "%s:prodModel.0", pNode->xCommon.xInfo.xOption.xSNMP.pMIB);

	xRet = FTOM_SNMPC_getOID(pBuff, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID[%s]\n", pBuff);	
		return	xRet;
	}

	xRet = FTOM_SNMPC_get( 
				pSNMPC,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_getSTRING(&xValue, pModel, ulMaxLen);	
	}

	FTM_VALUE_final(&xValue);

	return	xRet;
}


/***************************************************************
 *
 ***************************************************************/

FTM_RET	FTOM_NODE_SNMPC_getEPCount
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulCount != NULL);
	FTOM_SNMPC_PTR		pSNMPC = NULL;

	FTM_RET	xRet;

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTOM_SNMPC_getEPCount(pSNMPC, pNode->pIP, xType, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get SNMP client count.\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_NODE_SNMPC_getEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	FTM_RET	xRet;
	FTM_VALUE_TYPE		xDataType;
	FTM_VALUE			xValue;
	FTM_BOOL			bValid = FTM_TRUE;
	FTOM_SNMPC_PTR		pSNMPC = NULL;

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		xRet = FTM_RET_SNMP_TIMEOUT;
		ERROR2(xRet, "The node[%s] is not connected!\n", pNode->xCommon.xInfo.pDID);
		return	xRet;
	} 

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTOM_EP_getDataType(pEP, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get EP data type!\n");
		return	xRet;
	}

	if (pEP->pOpts == NULL)
	{
		ERROR2(FTM_RET_EP_IS_NOT_ATTACHED, "EP[%s] is not attached.\n", pEP->xInfo.pEPID);	
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	FTM_LOCK_set(&pNode->xCommon.xLock);
	FTM_LOCK_set(pEP->pLock);

	FTM_VALUE_init(&xValue, xDataType);

	xRet = FTOM_SNMPC_get( 
				pSNMPC,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&((FTOM_NODE_SNMPC_EP_OPTS_PTR)pEP->pOpts)->xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_EP_DATA_initVALUE(pData, &xValue);
	}
	else
	{
		ERROR2(xRet, "SNMP get failed.\n");
	}

	FTM_VALUE_final(&xValue);

	FTM_LOCK_reset(pEP->pLock);
	FTM_LOCK_reset(&pNode->xCommon.xLock);

	return	xRet;
}

FTM_RET	FTOM_NODE_SNMPC_setEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	FTM_RET	xRet;
	FTOM_SNMPC_PTR		pSNMPC = NULL;
	
	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		xRet = FTM_RET_NOT_CONNECTED;
		ERROR2(xRet, "Node[%s] is not connected.\n", pNode->xCommon.xInfo.pDID);	
		return	xRet;
	} 

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	if (pEP->pOpts == NULL)
	{
		ERROR2(FTM_RET_EP_IS_NOT_ATTACHED, "EP[%s] is not attached.\n", pEP->xInfo.pEPID);	
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	FTM_LOCK_set(&pNode->xCommon.xLock);
	FTM_LOCK_set(pEP->pLock);

	xRet = FTOM_SNMPC_set(
				pSNMPC,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&((FTOM_NODE_SNMPC_EP_OPTS_PTR)pEP->pOpts)->xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&pData->xValue);
			

	FTM_LOCK_reset(pEP->pLock);
	FTM_LOCK_reset(&pNode->xCommon.xLock);

	return	xRet;
}

FTM_RET	FTOM_NODE_SNMPC_getEPDataAsync
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP
)
{
	FTM_RET	xRet;
	FTM_VALUE_TYPE		xDataType;
	FTOM_MSG_PTR		pMsg = NULL;

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		xRet = FTM_RET_NOT_CONNECTED;
		ERROR2(xRet, "Node[%s] is not connected.\n", pNode->xCommon.xInfo.pDID);	
		return	xRet;
	} 

	xRet = FTOM_EP_getDataType(pEP, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get EP data type.\n");
		return	xRet;
	}
	
	if (pEP->pOpts == NULL)
	{
		ERROR2(FTM_RET_EP_IS_NOT_ATTACHED, "EP[%s] is not attached.\n", pEP->xInfo.pEPID);	
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	xRet = FTOM_MSG_SNMPC_createGetEPData(
				pNode->xCommon.xInfo.pDID, 
				pEP->xInfo.pEPID,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&((FTOM_NODE_SNMPC_EP_OPTS_PTR)pEP->pOpts)->xOID,
				pNode->xCommon.xInfo.ulTimeout,
				xDataType,
				&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet ,"Failed to create message\n");
		return	xRet;
	}

	xRet = FTOM_SERVICE_sendMessage(FTOM_SERVICE_SNMP_CLIENT, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message\n");
		FTOM_MSG_destroy(&pMsg);	
	}

	return	xRet;
}

FTM_RET	FTOM_NODE_SNMPC_setEPDataAsync
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	FTM_RET	xRet;
	FTOM_MSG_PTR		pMsg = NULL;
	
	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		return	FTM_RET_SNMP_TIMEOUT;
	} 

	if (pEP->pOpts == NULL)
	{
		ERROR2(FTM_RET_EP_IS_NOT_ATTACHED, "EP[%s] is not attached.\n", pEP->xInfo.pEPID);	
		return	FTM_RET_EP_IS_NOT_ATTACHED;	
	}

	xRet = FTOM_MSG_SNMPC_createSetEPData(
				pNode->xCommon.xInfo.pDID, 
				pEP->xInfo.pEPID,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&((FTOM_NODE_SNMPC_EP_OPTS_PTR)pEP->pOpts)->xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&pData->xValue,
				&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTOM_SERVICE_sendMessage(FTOM_SERVICE_SNMP_CLIENT, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message\n");
		FTOM_MSG_destroy(&pMsg);	
	}

	return	xRet;
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForID
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "ID", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForType
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "Type", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForName
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "Name", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForSerialNumber
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "SN", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForState
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "State", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForUpdateInterval
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "UpdateInterval", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForTime
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "LastTime", ulIndex, pOID);
}

FTM_RET		FTOM_NODE_SNMPC_getOIDForValue
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	return	FTOM_NODE_SNMPC_getOID(pNode, ulType, "Value", ulIndex, pOID);
}

typedef struct	
{
	FTM_EP_TYPE		xType;
	FTM_CHAR_PTR	pName;
}	FTOM_NODE_SNMPC_OID_PREFIX, _PTR_ FTOM_NODE_SNMPC_OID_PREFIX_PTR;

static
FTOM_NODE_SNMPC_OID_PREFIX	pOIDPrefixes[] =
{	
	{	FTM_EP_TYPE_TEMPERATURE,"temp"},
	{	FTM_EP_TYPE_HUMIDITY, 	"humi"},
	{	FTM_EP_TYPE_VOLTAGE,	"vlt"},
	{	FTM_EP_TYPE_CURRENT, 	"curr"},
	{	FTM_EP_TYPE_DI,			"di"},
	{	FTM_EP_TYPE_DO, 		"do"},
	{	FTM_EP_TYPE_GAS, 		"gas"},
	{	FTM_EP_TYPE_POWER, 		"pwr"},
	{	FTM_EP_TYPE_AI, 		"ai"},
	{	FTM_EP_TYPE_COUNT, 		"cnt"},
	{	FTM_EP_TYPE_PRESSURE, 	"prs"},
	{	FTM_EP_TYPE_DISCRETE, 	"dsc"},
	{	FTM_EP_TYPE_DEVICE, 	"dev"},
	{	FTM_EP_TYPE_MULTI, 		"multi"},
	{	FTM_EP_TYPE_CTRL, 		"ctrl"},
	{	0, 						NULL	}
};

FTM_RET		FTOM_NODE_SNMPC_getOID
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_CHAR_PTR		pFieldName,
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
)
{
	ASSERT(pNode != NULL);
	ASSERT(pOID != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pBuff[1024];
	FTOM_NODE_SNMPC_OID_PREFIX_PTR	pPrefix = pOIDPrefixes;

	while(pPrefix->xType != 0)
	{
		if (pPrefix->xType == ulType)
		{
			break;
		}

		pPrefix++;
	}
	
	if (pPrefix->xType == 0)
	{
		xRet = FTM_RET_INVALID_TYPE; 
		ERROR2(xRet, "Invalid EP type!\n");
		return	xRet;
	}

	sprintf(pBuff, "%s:%s%s.%lu", pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pPrefix->pName, pFieldName, ulIndex + 1);

	xRet = FTOM_SNMPC_getOID(pBuff, pOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID[%s]\n", pBuff);	
	}

	return	xRet;
}

FTM_RET	FTOM_NODE_SNMPC_getEPID
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				ulMaxLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET			xRet;
	FTM_SNMP_OID	xOID;
	FTOM_SNMPC_PTR		pSNMPC = NULL;
	FTM_VALUE		xValue;
	FTM_BOOL		bValid = FTM_TRUE;

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		return	FTM_RET_SNMP_TIMEOUT;
	} 

	if (ulMaxLen < 2)
	{
		xRet = FTM_RET_BUFFER_TOO_SMALL;
		ERROR2(xRet, "Buffer too small[%d].\n", ulMaxLen);
		return	xRet;
	}

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTOM_NODE_SNMPC_getOIDForID(pNode, ulEPType, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID for ID[Type = %08x].\n", ulEPType);
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to init value!\n");
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pSNMPC, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet == FTM_RET_OK)
	{
		memset(pEPID, 0, ulMaxLen);
		xRet = FTM_VALUE_getSTRING(&xValue, pEPID, ulMaxLen);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;

}

FTM_RET	FTOM_NODE_SNMPC_getEPName
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_CHAR_PTR			pName,
	FTM_ULONG				ulMaxLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pName != NULL);

	FTM_RET			xRet;
	FTM_SNMP_OID	xOID;
	FTOM_SNMPC_PTR		pSNMPC = NULL;
	FTM_VALUE		xValue;
	FTM_BOOL		bValid = FTM_TRUE;

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		return	FTM_RET_SNMP_TIMEOUT;
	} 

	if (ulMaxLen < 2)
	{
		xRet = FTM_RET_BUFFER_TOO_SMALL;
		ERROR2(xRet, "Buffer too small[%d].\n", ulMaxLen);
		return	xRet;
	}

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTOM_NODE_SNMPC_getOIDForName(pNode, ulEPType, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID for name.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to init value!\n");
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pSNMPC, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet == FTM_RET_OK)
	{
		memset(pName, 0, ulMaxLen);
		xRet = FTM_VALUE_getSTRING(&xValue, pName, ulMaxLen);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;

}

FTM_RET	FTOM_NODE_SNMPC_getEPState
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_BOOL_PTR			pbEnable
)
{
	ASSERT(pNode != NULL);
	ASSERT(pbEnable != NULL);

	FTM_RET			xRet;
	FTM_SNMP_OID	xOID;
	FTM_VALUE		xValue;
	FTM_BOOL		bValid = FTM_TRUE;
	FTOM_SNMPC_PTR	pSNMPC;

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		return	FTM_RET_SNMP_TIMEOUT;
	} 

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTOM_NODE_SNMPC_getOIDForState(pNode, ulEPType, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID for State.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_BOOL);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to init value!\n");
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pSNMPC, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_getBOOL(&xValue, pbEnable);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;

}

FTM_RET	FTOM_NODE_SNMPC_getEPUpdateInterval
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_ULONG				ulEPType,
	FTM_ULONG				ulIndex,
	FTM_ULONG_PTR			pulInterval
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulInterval != NULL);

	FTM_RET			xRet;
	FTM_SNMP_OID	xOID;
	FTM_VALUE		xValue;
	FTM_BOOL		bValid = FTM_TRUE;
	FTOM_SNMPC_PTR	pSNMPC;

	if (FTOM_NODE_isConnected((FTOM_NODE_PTR)pNode) != FTM_TRUE)
	{
		return	FTM_RET_SNMP_TIMEOUT;
	} 

	if (pNode->pSNMPC == NULL)
	{
		FTOM_SERVICE_PTR 	pService;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "SNNP Client not supported!\n");
			return	xRet;	
		}

		pSNMPC = (FTOM_SNMPC_PTR)pService->pData;
	}
	else
	{
		pSNMPC = pNode->pSNMPC;	
	}

	xRet = FTOM_NODE_SNMPC_getOIDForUpdateInterval(pNode, ulEPType, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to get OID for update interval.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_ULONG);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to init value!\n");
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pSNMPC, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->pIP,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue,
				&bValid);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_getULONG(&xValue, pulInterval);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;

}

FTM_RET	FTOM_NODE_SNMPC_set
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTM_NODE_FIELD			xFields,
	FTM_NODE_PTR			pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	if (xFields & FTM_NODE_FIELD_FLAGS)
	{
		pNode->xCommon.xInfo.xFlags = pInfo->xFlags;
	}

	if (xFields & FTM_NODE_FIELD_NAME)
	{
		strcpy(pNode->xCommon.xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_NODE_FIELD_LOCATION)
	{
		strcpy(pNode->xCommon.xInfo.pLocation, pInfo->pLocation);
	}

	if (xFields & FTM_NODE_FIELD_INTERVAL)
	{
		pNode->xCommon.xInfo.ulReportInterval = pInfo->ulReportInterval;
	}

	if (xFields & FTM_NODE_FIELD_TIMEOUT)
	{
		pNode->xCommon.xInfo.ulTimeout = pInfo->ulTimeout;
	}

	if (xFields & FTM_NODE_FIELD_SNMP_VERSION)
	{
		pNode->xCommon.xInfo.xOption.xSNMP.ulVersion = pInfo->xOption.xSNMP.ulVersion ;
	}

	if (xFields & FTM_NODE_FIELD_SNMP_URL)
	{
		strcpy(pNode->xCommon.xInfo.xOption.xSNMP.pURL, pInfo->xOption.xSNMP.pURL);
	}

	if (xFields & FTM_NODE_FIELD_SNMP_COMMUNITY)
	{
		strcpy(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity, pInfo->xOption.xSNMP.pCommunity);
	}

	if (xFields & FTM_NODE_FIELD_SNMP_MIB)
	{
		strcpy(pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pInfo->xOption.xSNMP.pMIB);
	}

	if (xFields & FTM_NODE_FIELD_SNMP_MAX_RETRY)
	{
		pNode->xCommon.xInfo.xOption.xSNMP.ulMaxRetryCount = pInfo->xOption.xSNMP.ulMaxRetryCount;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_attachEP
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTOM_EP_PTR				pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	FTM_RET	xRet;
	FTM_INT	nIndex;
	FTOM_NODE_SNMPC_EP_OPTS	xOpts;

	nIndex = strtoul(&pEP->xInfo.pEPID[strlen(pEP->xInfo.pEPID) - 2], 0, 16);

	xRet = FTOM_NODE_SNMPC_getOIDForValue(pNode, pEP->xInfo.xType, nIndex - 1, &xOpts.xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get OID for EP[%s] of node[%s]\n", pEP->xInfo.pEPID, pNode->xCommon.xInfo.pDID);
		ERROR2(xRet, "EP Info[Type = %08x, nIndex = %d]\n", pEP->xInfo.xType, nIndex - 1);
		return	xRet;
	}

	pEP->pOpts = (FTM_VOID_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_SNMPC_EP_OPTS));
	if (pEP->pOpts == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Not enough memory[size = %d]\n", sizeof(FTOM_NODE_SNMPC_EP_OPTS));
		return	xRet;	
	}

	memcpy(pEP->pOpts, &xOpts, sizeof(FTOM_NODE_SNMPC_EP_OPTS));
	pEP->pNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_detachEP
(
	FTOM_NODE_SNMPC_PTR		pNode,
	FTOM_EP_PTR				pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	
	if ((FTM_VOID_PTR)pEP->pNode != (FTM_VOID_PTR)pNode)
	{
		return	FTM_RET_OWNER_MISMATCH;
	}

	if (pEP->pOpts != NULL)
	{
		FTM_MEM_free(pEP->pOpts);	
		pEP->pOpts = NULL;	
	}

	pEP->pNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_printOpts
(
	FTOM_NODE_SNMPC_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	MESSAGE("%16s   %10s - %s\n", "", "Version", 	FTM_SNMP_versionString(pNode->xCommon.xInfo.xOption.xSNMP.ulVersion));	
	MESSAGE("%16s   %10s - %s\n", "", "URL", 		pNode->xCommon.xInfo.xOption.xSNMP.pURL);
	MESSAGE("%16s   %10s - %s\n", "", "Community", 	pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);
	MESSAGE("%16s   %10s - %s\n", "", "MIB", 		pNode->xCommon.xInfo.xOption.xSNMP.pMIB);
	MESSAGE("%16s   %10s - %lu\n","", "Retry", 		pNode->xCommon.xInfo.xOption.xSNMP.ulMaxRetryCount);

	return	FTM_RET_OK;
}
