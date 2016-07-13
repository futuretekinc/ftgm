#include <string.h>
#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_snmp_client.h"
#include "ftom_node_snmp_client_gen.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

FTM_ULONG		active_hosts = 0;

static
FTM_RET		FTOM_NODE_SNMPC_getOID
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_CHAR_PTR		pFieldName,
	FTM_ULONG 			ulIndex, 
	FTM_SNMP_OID_PTR	pOID
);

static 
FTOM_NODE_CLASS_PTR	pClasses[] =
{
	&xGeneralSNMP
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
		*ppClass = &xGeneralSNMP;	
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
		ERROR("Class[%s] not found!\n", pInfo->pModel);
		return	xRet;
	}

	pNode = (FTOM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.pClass = pClass;
	if (strlen(pNode->xCommon.xInfo.pName) == 0)
	{
		snprintf(pNode->xCommon.xInfo.pName, FTM_NAME_LEN, "node_%s", pNode->xCommon.xInfo.pDID);
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

static FTM_CHAR_PTR	pOIDNamePrefix[] =
{
	"temp",
	"humi",
	"vlt",
	"curr",
	"di",
	"do",
	"gas",
	"pwr",
	"cnt",
	"prs",
	"dsc",
	"dev"
};

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

	if ((ulType == 0) || (ulType > sizeof(pOIDNamePrefix) / sizeof(FTM_CHAR_PTR)))
	{
		ERROR("Invalid type : %08x\n", ulType);
		return	FTM_RET_INVALID_TYPE;

	}

	sprintf(pBuff, "%s:%s%s.%lu", pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pOIDNamePrefix[ulType - 1], pFieldName, ulIndex + 1);

	xRet = FTOM_SNMPC_getOID(pBuff, pOID);
	if (xRet != FTM_RET_OK)
	{
		WARN("Failed to get OID[%s]\n", pBuff);	
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
	FTOM_SERVICE_PTR pService;
	FTM_VALUE		xValue;

	if (ulMaxLen < 2)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;
	}

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR("SNMP Client not supported!\n");
		return  xRet;   
	}

	xRet = FTOM_NODE_SNMPC_getOIDForID(pNode, ulEPType >> 24, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to get OID for ID.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init value[%08x]!\n", xRet);
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pService->pData, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue);
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
	FTOM_SERVICE_PTR pService;
	FTM_VALUE		xValue;

	if (ulMaxLen < 2)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;
	}

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR("SNMP Client not supported!\n");
		return  xRet;   
	}

	xRet = FTOM_NODE_SNMPC_getOIDForName(pNode, ulEPType >> 24, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to get OID for ID.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init value[%08x]!\n", xRet);
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pService->pData, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue);
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
	FTOM_SERVICE_PTR pService;
	FTM_VALUE		xValue;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR("SNMP Client not supported!\n");
		return  xRet;   
	}

	xRet = FTOM_NODE_SNMPC_getOIDForState(pNode, ulEPType >> 24, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to get OID for ID.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_BOOL);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init value[%08x]!\n", xRet);
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pService->pData, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_getBOOL(&xValue, pbEnable);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;

}

FTM_RET	FTOM_NODE_SNMPC_getEPInterval
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
	FTOM_SERVICE_PTR pService;
	FTM_VALUE		xValue;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SNMP_CLIENT, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR("SNMP Client not supported!\n");
		return  xRet;   
	}

	xRet = FTOM_NODE_SNMPC_getOIDForUpdateInterval(pNode, ulEPType >> 24, ulIndex, &xOID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to get OID for ID.\n");
		return	xRet;	
	}

	xRet = FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_ULONG);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init value[%08x]!\n", xRet);
		return	xRet;
	}

	xRet = FTOM_SNMPC_get(
				pService->pData, 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion, 
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xValue);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_VALUE_getULONG(&xValue, pulInterval);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;

}
